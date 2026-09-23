#include "trading/market_data/market_data_consumer.h"

namespace Trading
{
    MarketDataConsumer::MarketDataConsumer(
        ClientId clientId,
        Exchange::MEMarketUpdateLFQueue* marketUpdates,
        const std::string& iface,
        const std::string& snapshotIp,
        int snapshotPort,
        const std::string& incrementalIp,
        int incrementalPort
    ) :
    incomingMDUpdates_(marketUpdates),
    run_(false),
    logger_("trading_market_data_consumer_" + std::to_string(clientId) + ".log"),
    incrementalMcastSocket_(logger_),
    snapshotMcastSocket_(logger_),
    iface_(iface),
    snapshotIp_(snapshotIp),
    snapshotPort_(snapshotPort)
    {
        auto recvCallbackInvoke = [this](auto socket) 
        {
            // defined below
            recvCallback(socket);
        };
        incrementalMcastSocket_.recvCallback_ = recvCallbackInvoke;

        ASSERT(
            incrementalMcastSocket_.init(incrementalIp, iface, incrementalPort, true) >= 0,
            "Unable to create incremental mcast socket. error: " + std::string(std::strerror(errno))
        );

        ASSERT(
            incrementalMcastSocket_.join(incrementalIp, iface, incrementalPort),
            "Join failed on: " + std::to_string(incrementalMcastSocket_.socketFd_) + " error: " + std::string(std::strerror(errno))
        );

        snapshotMcastSocket_.recvCallback_ = recvCallbackInvoke;
    }

    MarketDataConsumer::~MarketDataConsumer()
    {
        stop();

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(5s);
    }

    void MarketDataConsumer::start()
    {
        run_ = true;
        ASSERT(createAndStartThread(-1, "Trading/MarketDataConsumer", [this]() { run(); }) != nullptr,
        "Failed to start MarketData thread"
        );
    }

    void MarketDataConsumer::stop()
    {
        run_ = false;
    }

    void MarketDataConsumer::run() noexcept
    {
        logger_.log("%:% %() %\n", __FILE__, __LINE__, __FUNCTION__, Common::getCurrentTimeStr(&timeStr_));

        // constantly checking if any new data is coming in the NIC
        while (run_) {
            incrementalMcastSocket_.sendAndRecv();
            snapshotMcastSocket_.sendAndRecv();
        }
    }

    // this callback is invoked in the main run_ loop above, and both snapshot and incremental mcast sockets use the same recvCallback function
    void MarketDataConsumer::recvCallback(McastSocket* socket) noexcept
    {
        /*
        checking if it is snapshot, otherwise, we proceed to fill in our buffer for updating the orderbook
        */
        const bool isSnapshot = (socket->socketFd_ == snapshotMcastSocket_.socketFd_);

        // unlikely situation when we are not in recovery mode but receiving snapshot data
        if (UNLIKELY(isSnapshot && !inRecovery_)) {
            socket->nextRcvValidIndex_ = 0;
            return;
        }

        // real processing of the data
        if (socket->nextRcvValidIndex_ >= sizeof(Exchange::MEMarketUpdate)) {
            size_t i = 0;

            for (; i + sizeof(Exchange::MDPMarketUpdate) <= socket->nextRcvValidIndex_; i+= sizeof(Exchange::MDPMarketUpdate)) {
                const Exchange::MDPMarketUpdate* request = reinterpret_cast<const Exchange::MDPMarketUpdate*>(socket->inboundData_.data() + i);
                const bool alreadyInRecovery = inRecovery_;

                // compare expected sequence number to the new received package
                inRecovery_ = (alreadyInRecovery || request->seqNum_ != nextExpIncSeqNum_);

                if (UNLIKELY(inRecovery_)) {

                    // first time in recovery
                    if (UNLIKELY(!alreadyInRecovery)) {
                        // initializes the Mcast snapshot socket
                        startSnapshotSync();
                    }

                    queueMessage(isSnapshot, request);
                } else if (!isSnapshot) {

                    // incremental data branch, where we will just output to the trading engine directly

                    nextExpIncSeqNum_++;
                    auto nextWrite = incomingMDUpdates_->getNextWritePtr();
                    *nextWrite = std::move(request->meMarketUpdate_);
                    incomingMDUpdates_->updateWriteNext();
                }

                // recall i is a multiple of MDPMarketUpdate size
                memcpy(socket->inboundData_.data(), socket->inboundData_.data() + i, socket->nextRcvValidIndex_ - i);
                socket->nextRcvValidIndex_ -= i;
            }

        }
    }

    // in recovery mode for the first time
    void MarketDataConsumer::startSnapshotSync()
    {
        snapshotQueuedMsgs_.clear();
        incrementalQueuedMsgs_.clear();

        // initialize the snapshot socket
        ASSERT(snapshotMcastSocket_.init(snapshotIp_, iface_, snapshotPort_, true) >= 0,
        "Unable to create snapshot mcast socket");


        ASSERT(snapshotMcastSocket_.join(snapshotIp_, iface_, snapshotPort_),
        "Join failed");
    }

    // this function receives MDPMarketUpdate message
    void MarketDataConsumer::queueMessage(bool isSnapshot, const Exchange::MDPMarketUpdate* request)
    {
        if (isSnapshot) {
            // if the entry for the sequence number already exists in our hashmap, then this means we are receiving a new snapshot message cycle and we were not able to recover from the previous snapshot message cycle
            if (snapshotQueuedMsgs_.find(request->seqNum_) != snapshotQueuedMsgs_.end()) {
                snapshotQueuedMsgs_.clear();
            }
            snapshotQueuedMsgs_[request->seqNum_] = request->meMarketUpdate_;
        } else {
            incrementalQueuedMsgs_[request->seqNum_] = request->meMarketUpdate_;
        }
        // basically business logic for checking the snapshot and incremental market data
        // check out page 288 for the book
        checkSnapshotSync();
    }

}