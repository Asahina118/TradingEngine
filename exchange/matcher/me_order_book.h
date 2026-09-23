#pragma once

#include "Common/types.h"
#include "Common/mem_pool.h"
#include "Common/logging.h"
#include "order_server/client_response.h"
#include "exchange/market_data/market_update.h"
#include "exchange/matcher/me_order.h"

using namespace Common;
namespace Exchange
{
    class MatchingEngine;

    class MEOrderBook final
    {
    public:
        MEOrderBook(TickerId tickerId, Logger* logger, MatchingEngine* matchingEngine);

        ~MEOrderBook();

        MEOrderBook() = delete;

        MEOrderBook(const MEOrderBook&) = delete;
        MEOrderBook(MEOrderBook&&) = delete;

        MEOrderBook& operator=(const MEOrderBook&) = delete;
        MEOrderBook& operator=(MEOrderBook&&) = delete;

        void add(
            ClientId clientId,
            OrderId orderId,
            TickerId tickerId,
            Side side,
            Price price,
            Qty qty
        );

        void cancel(
            ClientId clientId,
            OrderId orderId,
            TickerId tickerId
        );

    private:
        TickerId tickerId_ = TickerId_INVALID;

        MatchingEngine* matchingEngine_ = nullptr;
        ClientOrderHashMap cidOidToOrder_;

        MemPool<MEOrdersAtPrice> ordersAtPricePool_;
        MEOrdersAtPrice* bidsByPrice_ = nullptr;
        MEOrdersAtPrice* asksByPrice_ = nullptr;

        OrdersAtPriceHashMap priceOrdersAtPrice_;

        MemPool<MEOrder> orderPool_;
        MEClientResponse clientResponse_;
        MEMarketUpdate marketUpdate_;

        std::string timeStr_;
        Logger* logger_ = nullptr;

        OrderId nextMarketOrderId_ = OrderId_INVALID;
        OrderId generateNewMarketOrderId() noexcept
        {
            return nextMarketOrderId_++;
        }

        Price priceToIndex(Price price) const noexcept
        {
            return (price % ME_MAX_PRICE_LEVELS);
        }

        /*
            priceOrdersAtPrice_ save both sides in one hashmap, as both sides in logic should not have the same price level (otherwise a buy would instantly be executed by some participants in the market)
        */
        MEOrdersAtPrice* getOrdersAtPrice(Price price) const noexcept
        {
            return priceOrdersAtPrice_.at(priceToIndex(price));
        }

        Qty checkForMatch(ClientId, OrderId, TickerId, Side, Price, Qty, Qty) noexcept;
        Priority getNextPriority(Price price) noexcept;

        void addOrder(MEOrder*) noexcept;
        void addOrdersAtPrice(MEOrdersAtPrice*) noexcept;

        void removeOrder(MEOrder*) noexcept;
        void removeOrdersAtPrice(Side, Price) noexcept;

        void match(TickerId, ClientId, Side, OrderId clientOrderId, OrderId newMarketOrderId, MEOrder* askItr, Qty* leavesQty) noexcept;
    };

    using OrderBookHashMap = std::array<MEOrderBook*, ME_MAX_TICKERS>;
}