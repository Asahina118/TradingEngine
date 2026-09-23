#pragma once

#include <sstream>

#include "Common/types.h"
#include "Common/lf_queue.h"

using namespace Common;
namespace Exchange
{
    #pragma pack(push, 1)
    enum class ClientResponseType : uint8_t
    {
        INVALID = 0,
        ACCEPTED = 1,
        CANCELED = 2,
        FILLED = 3,
        CANCEL_REJECTED = 4
    };

    inline std::string clientResponseTypeToString(ClientResponseType type)
    {
        switch (type) {
            case ClientResponseType::INVALID : return "INVALID";
            case ClientResponseType::ACCEPTED : return "ACCEPTED";
            case ClientResponseType::CANCELED : return "CANCELED";
            case ClientResponseType::FILLED : return "FILLED";
            case ClientResponseType::CANCEL_REJECTED : return "CANCEL_REJECTED";
        }
        return "UNKNOWN";
    }
    
    struct MEClientResponse
    {
        ClientResponseType type_ = ClientResponseType::INVALID;
        ClientId clientId_ = ClientId_INVALID;
        TickerId tickerId_ = TickerId_INVALID;
        OrderId clientOrderId_ = OrderId_INVALID;
        OrderId marketOrderId_ = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty execQty_ = Qty_INVALID;
        Qty leavesQty_ = Qty_INVALID;

        std::string toString() const {
            std::stringstream ss;
            ss << "MEClientResponse"
            << " ["
            << "type:" << clientResponseTypeToString(type_)
            << " client:" << clientIdToString(clientId_)
            << " ticker:" << tickerIdToString(tickerId_)
            << " coid:" << orderIdToString(clientOrderId_)
            << " moid:" << orderIdToString(marketOrderId_)
            << " side:" << sideToString(side_)
            << " exec_qty:" << qtyToString(execQty_)
            << " leaves_qty:" << qtyToString(leavesQty_)
            << " price:" << priceToString(price_)
            << "]";
            return ss.str();
        }
    };

    using ClientResponseLFQueue = LFQueue<MEClientResponse>;
}