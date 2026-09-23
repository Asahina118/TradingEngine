#pragma once

#include "Common/thread_utils.h"
#include "Common/lf_queue.h"
#include "Common/macros.h"
#include "Common/logging.h"

#include "order_server/client_request.h"
#include "order_server/client_response.h"
#include "exchange/market_data/market_update.h"
#include "exchange/matcher/me_order.h"
#include "exchange/matcher/me_order_book.h"

using namespace Common;
namespace Exchange
{
    class MatchingEngine final
    {
    public:
        MatchingEngine(
            ClientRequestLFQueue* clientRequests,
            ClientResponseLFQueue* clientResponses,
            MEMarketUpdateLFQueue* marketUpdates
        );

        ~MatchingEngine();

        MatchingEngine(const MatchingEngine&) = delete;
        MatchingEngine(MatchingEngine&&) = delete;

        MatchingEngine& operator=(const MatchingEngine&) = delete;
        MatchingEngine& operator=(MatchingEngine&&) = delete;

        void start();
        void stop();
        void run();

        void processClientRequest(const MEClientRequest*) noexcept;

        // note: this function moves the MEClientResponse out
        void sendClientResponse(MEClientResponse*) noexcept;

        void sendMarketUpdate(const MEMarketUpdate*) noexcept;

    private:
        OrderBookHashMap tickerOrderBook_;

        ClientRequestLFQueue* incomingRequests_ = nullptr;
        ClientResponseLFQueue* outgoingOGWResponses_ = nullptr;
        MEMarketUpdateLFQueue* outgoingMDUpdates_ = nullptr;

        volatile bool run_ = false;

        std::string timeStr_;
        Logger logger_;
    };
}