#include "trading/strategy/market_order.h"

namespace Trading
{
    MarketOrder::MarketOrder(OrderId orderId, Side side, Price price, Qty qty, Priority priority, MarketOrder* prevOrder, MarketOrder* nextOrder) :
    orderId_(orderId),
    side_(side),
    price_(price),
    qty_(qty),
    priority_(priority),
    prevOrder_(prevOrder),
    nextOrder_(nextOrder)
    {}

    MarketOrdersAtPrice::MarketOrdersAtPrice(Side side, Price price, MarketOrder* firstMktOrder, MarketOrdersAtPrice* prevEntry, MarketOrdersAtPrice* nextEntry_) :
    side_(side),
    price_(price),
    firstMktOrder_(firstMktOrder),
    prevEntry_(prevEntry),
    nextEntry_(nextEntry_)
    {}

}