#pragma once

#include <functional>
#include <map>

#include "Common/thread_utils.h"
#include "Common/lf_queue.h"
#include "Common/macros.h"
#include "Common/mcast_socket.h"

#include "exchange/market_data/market_update.h"

using namespace Common;
namespace Trading
{
    class MarketDataConsumer
    {
    public:
        MarketDataConsumer() = delete;
        MarketDataConsumer(
            ClientId, Exchange::MEMarketUpdateLFQueue* marketUpdates, const std::string& iface, 
            const std::string& snapshotIp, int snapshotPort, 
            const std::string& incrementalIp, int incrementalPort
        );

        void start();
        void stop();
        void run() noexcept;
        void recvCallback(McastSocket*) noexcept;
        void startSnapshotSync();
        void queueMessage(bool isSnapshot, const Exchange::MDPMarketUpdate* request);
        void checkSnapshotSync();
    private:

        // track the sequence number from the MDPMarketUpdate to check for correctness of order and potential packet drops
        size_t nextExpIncSeqNum_ = 1;
        // connection to real trading engine
        Exchange::MEMarketUpdateLFQueue* incomingMDUpdates_ = nullptr;

        volatile bool run_ = false;

        // tells us if this class is currently in recovery mode and using the snapshot and incremental messages to rebuild the order book
        bool inRecovery_ = false;

        // necessary information for joining the snapshot multicast stream
        const std::string iface_;
        const std::string snapshotIp_;
        const int snapshotPort_;

        Logger logger_;
        std::string timeStr_;

        Common::McastSocket incrementalMcastSocket_;
        Common::McastSocket snapshotMcastSocket_;

        // sequence number -> struct
        QueuedMarketUpdates snapshotQueuedMsgs_;
        QueuedMarketUpdates incrementalQueuedMsgs_;
    };
    using QueuedMarketUpdates = std::map<size_t, Exchange::MEMarketUpdate>;

}