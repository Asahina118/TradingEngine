#include "me_order.h"
#include "me_order_book.h"
#include "matching_engine.h"
#include "order_server/client_request.h"

using namespace Common;
namespace Exchange
{
    MatchingEngine::MatchingEngine(ClientRequestLFQueue* clientRequests, ClientResponseLFQueue* clientResponses, MEMarketUpdateLFQueue* marketUpdates) 
    : 
    incomingRequests_(clientRequests),
    outgoingOGWResponses_(clientResponses),
    outgoingMDUpdates_(marketUpdates),
    logger_("exchange_matching_engine.log")
    {
        for (size_t i = 0; i < tickerOrderBook_.size(); i++) {
            tickerOrderBook_[i] = new MEOrderBook(i, &logger_, this);
        }
    }

    MatchingEngine::~MatchingEngine()
    {
        run_ = false;

        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);

        incomingRequests_ = nullptr;
        outgoingOGWResponses_ = nullptr;
        outgoingMDUpdates_ = nullptr;

        for (MEOrderBook*& orderBook : tickerOrderBook_) {
            delete orderBook;
            orderBook = nullptr;
        }
    }

    void MatchingEngine::start()
    {
        run_ = true;
        ASSERT(Common::createAndStartThread(-1, "Exchange/MatchingEngine", [this]() { run(); }) != nullptr, "Failed to start MatchingEngine thread.");
    }

    void MatchingEngine::stop()
    {
        run_ = false;
    }

    void MatchingEngine::run()
    {
        logger_.log("Matching engine is running at: %", __FILE__);
        std::cout << "Matching engine is running at: " << __FILE__ << std::endl;

        while (run_)
        {
            // remark returns nullptr if no elements are left to read
            std::cout << "Getting request from clientRequests..." << std::endl;
            const MEClientRequest* clientRequest = incomingRequests_->get();
            if (LIKELY(clientRequest)) {
                processClientRequest(clientRequest);
            }
            incomingRequests_->updateReadNext();
        }
    }

    void MatchingEngine::processClientRequest(const MEClientRequest* clientRequest) noexcept
    {
        MEOrderBook* orderBook = tickerOrderBook_[clientRequest->tickerId_];

        switch (clientRequest->type_) {
        case ClientRequestType::NEW :
            orderBook->add(
                clientRequest->clientId_,
                clientRequest->orderId_,
                clientRequest->tickerId_,
                clientRequest->side_,
                clientRequest->price_,
                clientRequest->qty_
            );
            break;

        case ClientRequestType::CANCEL :
            orderBook->cancel(
                clientRequest->clientId_,
                clientRequest->orderId_,
                clientRequest->tickerId_
            );
            break;

        default:
            FATAL("Received invalid client-request-type: " + clientRequestTypeToString(clientRequest->type_));
            break;
        }
    }

    void MatchingEngine::sendClientResponse(MEClientResponse* response) noexcept
    {
        MEClientResponse* nextWrite = outgoingOGWResponses_->getNextWritePtr();
        *nextWrite = std::move(*response);
        outgoingOGWResponses_->updateWriteNext();
    }

    void MatchingEngine::sendMarketUpdate(const MEMarketUpdate* marketUpdate) noexcept
    {
        MEMarketUpdate* nextWrite = outgoingMDUpdates_->getNextWritePtr();
        *nextWrite = *marketUpdate;
        outgoingMDUpdates_->updateWriteNext();
    }
}