#pragma once

#include "Common/logging.h"
#include "Common/socket_utils.h"
#include "Common/tcp_socket.h"

#include "order_server/client_request.h"
#include "order_server/client_response.h"
#include <string>

using namespace Common;
namespace Exchange
{
    const std::string iface_;
    const int port_ = 0;

    ClientResponseLFQueue* outgoingResponses_ = nullptr;

    volatile bool run_ = false;

    std::string timeStr_;
    Logger logger_;

    std::array<size_t, ME_MAX_NUM_CLIENTS> cidNextOutgoingSeqNum_;

    std::array<size_t, ME_MAX_NUM_CLIENTS> cidNextExpSeqNum_;

    std::array<Common::TCPSocket*, ME_MAX_CLIENT_UPDATES> cidTcpSocket_;

}