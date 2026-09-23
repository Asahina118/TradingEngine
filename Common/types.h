#pragma once

#include <cstdint>
#include <limits>
#include "macros.h"

namespace Common
{
    using OrderId = uint64_t;
    using TickerId = uint32_t;
    using ClientId = uint32_t;
    using Price = int64_t;
    using Qty = uint32_t;
    using Priority = uint64_t;

    enum class Side : int8_t
    {
        INVALID = 0,
        BUY = 1,
        SELL = -1
    };

    constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max();
    constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max();
    constexpr auto ClientId_INVALID = std::numeric_limits<ClientId>::max();
    constexpr auto Price_INVALID = std::numeric_limits<Price>::max();
    constexpr auto Qty_INVALID = std::numeric_limits<Qty>::max();
    constexpr auto Priority_INVALID = std::numeric_limits<Priority>::max();

    inline std::string orderIdToString(OrderId orderId)
    {
        if (UNLIKELY(orderId == OrderId_INVALID)) {
            return "INVALID";
        }

        return std::to_string(orderId);
    }

    inline std::string tickerIdToString(TickerId tickerId)
    {
        if (UNLIKELY(tickerId == TickerId_INVALID)) {
            return "INVALID";
        }

        return std::to_string(tickerId);
    }

    inline std::string clientIdToString(ClientId clientId)
    {
        if (UNLIKELY(clientId == ClientId_INVALID)) {
            return "INVALID";
        }

        return std::to_string(clientId);
    }

    inline std::string priceToString(Price price)
    {
        if (UNLIKELY(price == Price_INVALID)) {
            return "INVALID";
        }

        return std::to_string(price);
    }

    inline std::string qtyToString(Qty qty)
    {
        if (UNLIKELY(qty == Qty_INVALID)) {
            return "INVALID";
        }

        return std::to_string(qty);
    }

    inline std::string priorityToString(Priority priority)
    {
        if (UNLIKELY(priority == Priority_INVALID)) {
            return "INVALID";
        }

        return std::to_string(priority);
    }

    inline std::string sideToString(Side side)
    {
        switch (side)
        {
            case Side::BUY : return "BUY";
            case Side::SELL : return "SELL";
            case Side::INVALID : return "INVALID";
        }

        return "UNKNOWN";
    }

}