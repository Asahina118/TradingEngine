#pragma once

#include "Common/types.h"
#include "Common/constants.h"

using namespace Common;
namespace Trading
{
    struct MarketOrder
    {
        OrderId orderId_ = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Priority_INVALID;

        MarketOrder* prevOrder_ = nullptr;
        MarketOrder* nextOrder_ = nullptr;

        // only for memory pool
        MarketOrder() = default;

        MarketOrder(OrderId, Side, Price, Qty, Priority, MarketOrder* prevOrder, MarketOrder* nextOrder) noexcept;
        std::string toString() const;

    };

    struct MarketOrdersAtPrice
    {
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;

        MarketOrder* firstMktOrder_ = nullptr;

        MarketOrdersAtPrice* prevEntry_ = nullptr;
        MarketOrdersAtPrice* nextEntry_ = nullptr;

        MarketOrdersAtPrice() = default;

        MarketOrdersAtPrice(Side, Price, MarketOrder* firstMktOrder, MarketOrdersAtPrice* prevEntry, MarketOrdersAtPrice* nextEntry);

    };

    // Best Bid Offer : represents the total quantity available at the best bid and ask prices
    // used for components that do not require the entire OrderBook information
    struct BBO
    {
        Price bidPrice_ = Price_INVALID;
        Price askPrice_ = Price_INVALID;
        Qty bidQty_ = Qty_INVALID;
        Qty askQty_ = Qty_INVALID;
    };

    using OrderHashMap = std::array<MarketOrder*, ME_MAX_ORDER_IDS>;
    using OrdersAtPriceHashMap = std::array<MarketOrdersAtPrice*, ME_MAX_PRICE_LEVELS>;
}