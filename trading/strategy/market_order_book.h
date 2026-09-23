#pragma once

#include <array>

#include "Common/types.h"
#include "Common/mem_pool.h"
#include "Common/logging.h"

#include "trading/strategy/market_order.h"
#include "exchange/market_data/market_update.h"

namespace Trading
{
    class TradingEngine;

    // has all the functions to manage the order book
    class MarketOrderBook final
    {
    public:
        MarketOrderBook(TickerId tickerId, Logger* logger);
        ~MarketOrderBook();

        void setTradingEngine(TradingEngine*);
        void onMarketUpdate(const Exchange::MEMarketUpdate*) noexcept;
        void updateBBO(bool bidUpdated, bool askUpdated);

        auto priceToIndex(Price) const noexcept;
        MarketOrdersAtPrice getOrdersAtPrice(Price) const noexcept;

        void addOrder(MarketOrder*) noexcept;
        void addOrdersAtPrice(Side, Price) noexcept;

        void removeOrder(MarketOrder*);
        void removeOrdersAtPrice(Side, Price);
    private:
        const TickerId tickerId_;
        TradingEngine* tradingEngine_ = nullptr;
        OrderHashMap oidToOrder_;

        MemPool<MarketOrdersAtPrice> ordersAtPricePool_;
        MarketOrdersAtPrice* bidsByPrice_ = nullptr;
        MarketOrdersAtPrice* asksByPrice_ = nullptr;

        OrdersAtPriceHashMap priceOrdersAtPrice_;
        MemPool<MarketOrder> orderPool_;

        BBO bbo_;
        Logger logger_;
    };
    using MarketOrderBookHashMap = std::array<MarketOrderBook*, ME_MAX_TICKERS>;
}