#include "me_order.h"

using namespace Common;
namespace Exchange
{
    std::string MEOrder::toString() const
    {
        std::stringstream ss;
        ss  << "MEOrder" << "["
            << "ticker:" << tickerIdToString(tickerId_) << " "
            << "cid:" << clientIdToString(clientId_) << " "
            << "oid:" << orderIdToString(clientOrderId_) << " "
            << "moid:" << orderIdToString(marketOrderId_) << " "
            << "side:" << sideToString(side_) << " "
            << "price:" << priceToString(price_) << " "
            << "qty:" << qtyToString(qty_) << " "
            << "prio:" << priorityToString(priority_) << " "
            << "prev:" << orderIdToString(prevOrder_ ?
            prevOrder_->marketOrderId_ :
            OrderId_INVALID) << " "
            << "next:" << orderIdToString(nextOrder_ ?
            nextOrder_->marketOrderId_ :
            OrderId_INVALID) << "]";
        return ss.str();    
    }
}