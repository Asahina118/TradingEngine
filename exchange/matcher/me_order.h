#pragma once

#include <array>
#include <sstream>
#include "Common/types.h"
#include "Common/constants.h"

using namespace Common;
namespace Exchange
{
    struct MEOrder
    {
        TickerId tickerId_ = TickerId_INVALID;
        ClientId clientId_ = ClientId_INVALID;
        OrderId clientOrderId_ = OrderId_INVALID;
        OrderId marketOrderId_ = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Priority_INVALID;

        MEOrder* prevOrder_ = nullptr;
        MEOrder* nextOrder_ = nullptr;

        // MemPool initialization with the {T(), true} initialization for the MemPool struct for a DataBlock {T data_; bool isFree_;}
        // so in reality we only include this for the memory pool, we normally have no reasons to use the default ctor
        MEOrder() = default;

        MEOrder(TickerId tickerId, ClientId clientId, OrderId clientOrderId, OrderId marketOrderId, Side side, Price price, Qty qty, Priority priority, MEOrder* prevOrder, MEOrder* nextOrder) :
        tickerId_(tickerId),
        clientId_(clientId),
        clientOrderId_(clientOrderId),
        marketOrderId_(marketOrderId),
        side_(side),
        price_(price),
        qty_(qty),
        priority_(priority),
        prevOrder_(prevOrder),
        nextOrder_(nextOrder)
        {}

        std::string toString() const;
    };

    using OrderHashMap = std::array<MEOrder*, ME_MAX_ORDER_IDS>;

    // clientId -> this client's order (also stored in hash map)
    using ClientOrderHashMap = std::array<OrderHashMap, ME_MAX_NUM_CLIENTS>;

    struct MEOrdersAtPrice
    {
        Price price_ = Price_INVALID;
        Side side_ = Side::INVALID;

        MEOrder* firstMeOrder_ = nullptr;

        MEOrdersAtPrice* next_ = nullptr;
        MEOrdersAtPrice* prev_ = nullptr;
    
        MEOrdersAtPrice() = default;
        MEOrdersAtPrice(Side side, Price price, MEOrder* firstMeOrder, MEOrdersAtPrice* prev, MEOrdersAtPrice* next) :
        price_(price),
        side_(side),
        firstMeOrder_(firstMeOrder),
        next_(next),
        prev_(prev)
        {}

        std::string toString() const;
    };

    using OrdersAtPriceHashMap = std::array<MEOrdersAtPrice*, ME_MAX_PRICE_LEVELS>;
}