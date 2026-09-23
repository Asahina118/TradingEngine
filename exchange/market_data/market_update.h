#pragma once

#include <sstream>

#include "Common/types.h"
#include "Common/lf_queue.h"

// market data updates from the matching engine
using namespace Common;
namespace Exchange
{
    #pragma pack(push, 1)
    enum class MarketUpdateType : uint8_t
    {
        INVALID = 0,
        CLEAR = 1,
        ADD = 2,
        MODIFY = 3,
        CANCEL = 4,
        TRADE = 5,
        SNAPSHOT_START = 6,
        SNAPSHOT_END = 7
    };

    inline std::string marketUpdateTypeToString(MarketUpdateType type)
    {
        switch (type) {
            case MarketUpdateType::INVALID : return "INVALID";
            case MarketUpdateType::ADD : return "ADD";
            case MarketUpdateType::MODIFY : return "MODIFY";
            case MarketUpdateType::CANCEL : return "CANCEL";
            case MarketUpdateType::TRADE : return "TRADE";
        }
        return "UNKNOWN";
    }

    struct MEMarketUpdate
    {
        MarketUpdateType type_ = MarketUpdateType::INVALID;
        OrderId orderId_ = OrderId_INVALID;
        TickerId tickerId_ = TickerId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Priority_INVALID;

        std::string toString() const
        {
            std::stringstream ss;
            ss << "MEMarketUpdate"
            << " ["
            << " type:" << marketUpdateTypeToString(type_)
            << " ticker:" << tickerIdToString(tickerId_)
            << " oid:" << orderIdToString(orderId_)
            << " side:" << sideToString(side_)
            << " qty:" << qtyToString(qty_)
            << " price:" << priceToString(price_)
            << " priority:" << priorityToString(priority_)
            << "]";
            return ss.str();
        }
    };

    struct MDPMarketUpdate
    {
        size_t seqNum_ = 0;
        MEMarketUpdate meMarketUpdate_;

        std::string toString() const 
        {
            std::stringstream ss;
            ss << "MDPMarketUpdate"
            << " ["
            << " seq:" << seqNum_
            << " " << meMarketUpdate_.toString()
            << "]";
            return ss.str();
        }
    };

    #pragma pack(pop)
    using MEMarketUpdateLFQueue = LFQueue<MEMarketUpdate>;
    using MDPMarketUpdateLFQueue = LFQueue<MDPMarketUpdate>;
}