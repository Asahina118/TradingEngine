#include "market_order_book.h"


namespace Trading
{
    MarketOrderBook::MarketOrderBook(TickerId tickerId, Logger* logger) :
    tickerId_(tickerId),
    ordersAtPricePool_(ME_MAX_PRICE_LEVELS),
    orderPool_(ME_MAX_ORDER_IDS),
    logger_(logger)
    {}

    MarketOrderBook::~MarketOrderBook()
    {
        tradingEngine_ = nullptr;
        bidsByPrice_ = nullptr;
        asksByPrice_ = nullptr;
        oidToOrder_.fill(nullptr);
    }

    void MarketOrderBook::setTradingEngine(TradingEngine* tradingEngine)
    {
        tradingEngine_ = tradingEngine;
    }

    // handles the actual market update
    void MarketOrderBook::onMarketUpdate(const Exchange::MEMarketUpdate* marketUpdate) noexcept
    {
        const bool bidUpdated = (bidsByPrice_ && marketUpdate->side_ == Side::BUY && marketUpdate->price_ >= bidsByPrice_->price_);
        const bool askUpdated = (asksByPrice_ && marketUpdate->side_ == Side::SELL && marketUpdate->price_ <= asksByPrice_->price_);

        switch (marketUpdate->type_) {
            case Exchange::MarketUpdateType::ADD:
                MarketOrder* order = orderPool_.allocate(marketUpdate->orderId_, marketUpdate->side_, marketUpdate->price_, marketUpdate->qty_, marketUpdate->priority_, nullptr, nullptr);

                // updates the orderBook record and BBO if needed
                addOrder(order);

                break;
            
            case Exchange::MarketUpdateType::MODIFY:
                MarketOrder* order = oidToOrder_.at(marketUpdate->orderId_);
                order->qty_ = marketUpdate->qty_;

                break;
            
            case Exchange::MarketUpdateType::CANCEL:
                MarketOrder* order = oidToOrder_.at(marketUpdate->orderId_);

                // handles the order deletion in the orderbook, BBO will be updated separately in this function
                removeOrder(order);

                break;

            case Exchange::MarketUpdateType::TRADE:
                tradingEngine_->onTradeUpdate(marketUpdate, this);
                break;

            // happens when we detect any packet drops
            case Exchange::MarketUpdateType::CLEAR:
                for (auto& order : oidToOrder_) {
                    if (order)
                        orderPool_.deallocate(order);
                }
                oidToOrder_.fill(nullptr);

                if (bidsByPrice_) {
                    for (auto bid = bidsByPrice_->nextEntry_; bid != bidsByPrice_; bid = bid->nextEntry_) {
                        ordersAtPricePool_.deallocate(bid);
                    }
                    ordersAtPricePool_.deallocate(bidsByPrice_);
                }

                if (asksByPrice_) {
                    for (auto ask = asksByPrice_->nextEntry_; ask != asksByPrice_; ask = ask->nextEntry) {
                        ordersAtPricePool_.deallocate(ask);
                    }
                    ordersAtPricePool_.deallocate(asksByPrice_);
                }
                bidsByPrice_ = asksByPrice_ = nullptr;

                break;

            case Exchange::MarketUpdateType::INVALID:
            case Exchange::MarketUpdateType::SNAPSHOT_START:
            case Exchange::MarketUpdateType::SNAPSHOT_END:
            break;
        }

        updateBBO(bidUpdated, askUpdated);
        // tradingEngine_->onOrderBookUpdate();
    }

    // function is only called at the end of the above function, which means all data members in the same class have finisehd updating, so we can use them directly
    void MarketOrderBook::updateBBO(bool bidUpdated, bool askUpdated)
    {
        if (bidUpdated) {
            if (bidsByPrice_) {
                bbo_.bidPrice_ = bidsByPrice_->price_;
                bbo_.bidQty_ = bidsByPrice_->firstMktOrder_->qty_;
                for (auto bid = bidsByPrice_->firstMktOrder_->nextOrder_; bid != bidsByPrice_->firstMktOrder_; bid = bid->nextOrder_) {
                    bbo_.bidQty_ += bid->qty_;
                }
            } else {
                bbo_.bidPrice_ = Price_INVALID;
                bbo_.bidQty_ = Qty_INVALID;
            }
        }

        if (askUpdated) {
            if (asksByPrice_) {
                bbo_.askPrice_ = asksByPrice_->price_;
                bbo_.askQty_ = asksByPrice_->firstMktOrder_->qty_;
                for (auto order = asksByPrice_->firstMktOrder_->nextOrder_; order != asksByPrice_->firstMktOrder_; order = order->nextOrder_) {
                    bbo_.askQty_ += order->qty_;
                }
            } else {
                bbo_.askPrice_ = Price_INVALID;
                bbo_.askQty_ = Qty_INVALID;
            }
        }

    }
}