#include "me_order_book.h"
#include "me_order.h"
#include "Common/mem_pool.h"
#include "exchange/matcher/matching_engine.h"
#include <algorithm>

namespace Exchange
{
    MEOrderBook::MEOrderBook(TickerId tickerId, Logger* logger, MatchingEngine* matchingEngine) : 
    tickerId_(tickerId),
    matchingEngine_(matchingEngine),
    ordersAtPricePool_(ME_MAX_PRICE_LEVELS),
    orderPool_(ME_MAX_ORDER_IDS),
    logger_(logger)
    {}

    MEOrderBook::~MEOrderBook()
    {
        matchingEngine_ = nullptr;
        bidsByPrice_ = asksByPrice_ = nullptr;
        for (auto& itr : cidOidToOrder_)
            itr.fill(nullptr);
    }

    void MEOrderBook::add(
        ClientId clientId,
        OrderId clientOrderId,
        TickerId tickerId,
        Side side,
        Price price,
        Qty qty
    )
    {
        const OrderId newMarketOrderId = generateNewMarketOrderId();
        clientResponse_ = 
        {
            ClientResponseType::ACCEPTED,
            clientId,
            tickerId,
            clientOrderId,
            newMarketOrderId,
            side,
            price,
            0,
            qty
        };
        matchingEngine_->sendClientResponse(&clientResponse_);

        const Qty leavesQty = checkForMatch();

        // if there is remaining leavesQty (either because some sell orders are matched and not enough to consume all the buy request quantity, or no sell orders have a price for which the buy order requested), we add this buy to the order book
        if (LIKELY(leavesQty)) {
            const Priority priority = getNextPriority(price);
            MEOrder* order = orderPool_.allocate(
                tickerId,
                clientId,
                clientOrderId,
                newMarketOrderId,
                side,
                price,
                qty,
                priority,
                nullptr,
                nullptr
            );

            addOrder(order);

            marketUpdate_ = 
            {
                MarketUpdateType::ADD,
                newMarketOrderId,
                tickerId,
                side,
                price,
                leavesQty,
                priority
            };
            matchingEngine_->sendMarketUpdate(&marketUpdate_);
        }



    }

    void MEOrderBook::cancel(
        ClientId clientId,
        OrderId orderId,
        TickerId tickerId
    )
    {
        bool isCancelable = (clientId < cidOidToOrder_.size());
        MEOrder* exchangeOrder = nullptr;
        if (LIKELY(isCancelable)) {
            // OrderHashMap& coItr = cidOidToOrder_.at(clientId);
            // exchangeOrder = coItr.at(orderId);
            exchangeOrder = cidOidToOrder_.at(clientId).at(orderId);
            isCancelable = (exchangeOrder != nullptr);
        }

        if (UNLIKELY(!isCancelable)) {
            clientResponse_ =
            {
                ClientResponseType::CANCEL_REJECTED,
                clientId,
                tickerId,
                orderId,
                OrderId_INVALID,
                Side::INVALID,
                Price_INVALID,
                Qty_INVALID,
                Qty_INVALID
            };
        } else {
            clientResponse_ =
            {
                ClientResponseType::CANCELED,
                clientId,
                tickerId,
                orderId,
                exchangeOrder->marketOrderId_,
                exchangeOrder->side_,
                exchangeOrder->price_,
                Qty_INVALID,
                exchangeOrder->qty_
            };

            marketUpdate_ =
            {
                MarketUpdateType::CANCEL,
                exchangeOrder->marketOrderId_,
                tickerId,
                exchangeOrder->side_,
                exchangeOrder->price_,
                0,
                exchangeOrder->priority_
            };

            removeOrder(exchangeOrder);
            matchingEngine_->sendMarketUpdate(&marketUpdate_);
        }

        matchingEngine_->sendClientResponse(&clientResponse_);
    }

    Priority MEOrderBook::getNextPriority(Price price) noexcept
    {
        const MEOrdersAtPrice* ordersAtPrice = getOrdersAtPrice(price);
        if (!ordersAtPrice) {
            return 1lu;
        }

        return ordersAtPrice->firstMeOrder_->prevOrder_->priority_ + 1;
    }

    void MEOrderBook::addOrder(MEOrder* order) noexcept
    {
        const MEOrdersAtPrice* ordersAtPrice = getOrdersAtPrice(order->price_);

        if (!ordersAtPrice) {
            order->nextOrder_ = order->prevOrder_ = order;

            MEOrdersAtPrice* newOrdersAtPrice = ordersAtPricePool_.allocate(
                order->side_,
                order->price_,
                order,
                nullptr,
                nullptr
            );

            addOrdersAtPrice(newOrdersAtPrice);

        } else {
            // remember circular doubly linked list, so we can do this fast trick to add the MEOrder at the back of the list
            MEOrder* firstOrder = ordersAtPrice->firstMeOrder_;

            order->nextOrder_ = firstOrder;
            order->prevOrder_ = firstOrder->prevOrder_;

            firstOrder->prevOrder_->nextOrder_ = order;
            firstOrder->prevOrder_ = order;

            cidOidToOrder_.at(order->clientId_).at(order->clientOrderId_) = order;
        }
    }

    void MEOrderBook::addOrdersAtPrice(MEOrdersAtPrice* ordersAtPrice) noexcept
    {
        /*
        check ordersAtPrice.side_

            if (side_ == Side::BUY)
                insert into bidsByPrice_ 
            else
                insert into asksByPrice_

        either way (be it bidsByPrice_ or asksByPrice_):
            1. set the pointer of prev and next of ordersAtPrice
            2. set the original link list node pointers
            3. update hash map

        */

        priceOrdersAtPrice_.at(priceToIndex(ordersAtPrice->price_)) = ordersAtPrice;

        MEOrdersAtPrice* bestOrdersByPrice = ordersAtPrice->side_ == Side::BUY ? bidsByPrice_ : asksByPrice_;

        if (UNLIKELY(!bestOrdersByPrice)) {
            (ordersAtPrice->side_ == Side::BUY ? bidsByPrice_ : asksByPrice_) = ordersAtPrice;
            ordersAtPrice->next_ = ordersAtPrice->prev_ = ordersAtPrice;
        } 
        else {
            auto target = bestOrdersByPrice;
            bool addAfter = (
                (ordersAtPrice->side_ == Side::SELL
                && ordersAtPrice->price_ > target->price_)
                ||
                (ordersAtPrice->side_ == Side::BUY && ordersAtPrice->price_ < target->price_)
            );
            if (addAfter) {
                target = target->next_;
                addAfter = (
                    (ordersAtPrice->side_ == Side::SELL
                    && ordersAtPrice->price_ > target->price_)
                    ||
                    (ordersAtPrice->side_ == Side::BUY && ordersAtPrice->price_ < target->price_)
                );
                while (addAfter && target != bestOrdersByPrice) {
                    addAfter = (
                        (ordersAtPrice->side_ == Side::SELL
                        && ordersAtPrice->price_ > target->price_)
                        ||
                        (ordersAtPrice->side_ == Side::BUY && ordersAtPrice->price_ < target->price_)
                    );
                    if (addAfter)
                        target = target->next_;
                }
                if (addAfter) {
                    if (target == bestOrdersByPrice) {
                        target = bestOrdersByPrice->prev_;
                    }
                }
                ordersAtPrice->prev_ = target;
                target->next_->prev_ = ordersAtPrice;
                ordersAtPrice->next_ = target->next_;
                target->next_ = ordersAtPrice;
            } else {
                ordersAtPrice->prev_ = target->prev_;
                ordersAtPrice->next_ = target->next_;
                target->prev_->next_ = ordersAtPrice;
                target->prev_ = ordersAtPrice;
            }

            if (
                (ordersAtPrice->side_ == Side::BUY && ordersAtPrice->price_ > bestOrdersByPrice->price_)
                ||
                (ordersAtPrice->side_ == Side::SELL &&
                ordersAtPrice->price_ < bestOrdersByPrice->price_)
            ) {
                target->next_ = 
                (target->next_ == bestOrdersByPrice ? ordersAtPrice : target->next_);
                (ordersAtPrice->side_ == Side::BUY ? bidsByPrice_ : asksByPrice_) = ordersAtPrice;
            }
        }
    }

    void MEOrderBook::removeOrder(MEOrder* order) noexcept
    {
        MEOrdersAtPrice* ordersAtPrice = getOrdersAtPrice(order->price_);

        // only this order at the price level
        if (order->prevOrder_ == order) {
            removeOrdersAtPrice(order->side_, order->price_);
        } else {
            MEOrder* orderBefore = order->prevOrder_;
            MEOrder* orderAfter = order->nextOrder_;

            orderBefore->nextOrder_ = orderAfter;
            orderAfter->prevOrder_ = orderBefore;

            if (ordersAtPrice->firstMeOrder_ == order) {
                ordersAtPrice->firstMeOrder_ = orderAfter;
            }

            order->prevOrder_ = order->nextOrder_ = nullptr;
        }

        cidOidToOrder_.at(order->clientId_).at(order->clientOrderId_) = nullptr;
        orderPool_.deallocate(order);
    }

    void MEOrderBook::removeOrdersAtPrice(Side side, Price price) noexcept
    {
        MEOrdersAtPrice* bestOrdersByPrice = (side == Side::BUY) ? bidsByPrice_ : asksByPrice_;
        MEOrdersAtPrice* ordersAtPrice = getOrdersAtPrice(price);

        // only one element left in the doubly linked list
        if (UNLIKELY(bestOrdersByPrice->prev_ == bestOrdersByPrice)) {
            (side == Side::BUY ? bidsByPrice_ : asksByPrice_) = nullptr;

        } else {
            ordersAtPrice->next_->prev_ = ordersAtPrice->prev_;
            ordersAtPrice->prev_->next_ = ordersAtPrice->next_;

            // if the search by price retuns the head of either side of the best price
            if (ordersAtPrice == bestOrdersByPrice) {
                // but note that bestOrdersByPrice is no the member field so we have to do the retarded tertiary again to get the correct side
                (side == Side::BUY ? bidsByPrice_ : asksByPrice_) = ordersAtPrice->next_;
            }

            ordersAtPrice->next_ = ordersAtPrice->prev_ = nullptr;
        }

        priceOrdersAtPrice_.at(priceToIndex(price)) = nullptr;
        ordersAtPricePool_.deallocate(bestOrdersByPrice);
    }

    Qty MEOrderBook::checkForMatch(ClientId clientId, OrderId clientOrderId, TickerId tickerId, Side side, Price price, Qty qty, Qty newMarketOrderId) noexcept
    {
        Qty leavesQty = qty;
        if (side == Side::BUY) {
            while (leavesQty && asksByPrice_) {
                MEOrder* askItr = asksByPrice_->firstMeOrder_;

                // if the new buy request has no price matching sell order
                if (LIKELY(askItr->price_ > price)) {
                    break;
                }

                // add new bid order into the order book? Yep, should be it
                match(tickerId, clientId, side, clientOrderId, newMarketOrderId, askItr, &leavesQty);
            }
        }
        return leavesQty;
    }

    // this is called when the new aggressive order matches the passive order on the order book
    void MEOrderBook::match(TickerId tickerId, ClientId clientId, Side side, OrderId clientOrderId, OrderId newMarketOrderId, MEOrder* askItr, Qty* leavesQty)
    {
        /*
            1. need to generate two execution responses and sends them to the matching engine:
            one client is the aggressive order sender
            another one is the passive order

            2. need to publish a marketUpdate with MarketUpdateType::TRADE

            3. if the trade completely trades out the pass order, then another marketUpdate of type MarketUpdateType::CANCEL should be sent to the client that the passive order is removed

            Otherwise, a MarketUpdateType::UPDATE is instead sent to the participants
            
            4. returns the appropriate leavesQty
        */
       Qty& orderQty = askItr->qty_;
        Qty fillQty = std::min(askItr->qty_, *leavesQty);

        orderQty -= fillQty;
        *leavesQty -= fillQty;

        if (orderQty == 0) {
            removeOrder(askItr);
            clientResponse_ =
            {

            };
            marketUpdate_ =
            {

            };

            matchingEngine_->sendClientResponse(&clientResponse_);
            matchingEngine_->sendMarketUpdate(&marketUpdate_);
            
        }
    }
}