#pragma once

#include <functional>

#include "socket_utils.h"

#include "logging.h"

namespace Common
{
    constexpr size_t McastBufferSize = 64 * 1024 * 1024;

    struct McastSocket
    {
        McastSocket(Logger& logger) : logger_(logger) 
        {
            outboundData_.resize(McastBufferSize);
            inboundData_.resize(McastBufferSize);
        }

        int init(const std::string& ip, const std::string& iface, int port, bool isListening);

        bool join(const std::string& ip);
        void leave(const std::string& ip, int port);
        bool sendAndRecv() noexcept;

        void send(const void* data, size_t len) noexcept;

        int socketFd_ = -1;

        std::vector<char> outboundData_;
        size_t nextSendValidIndex_ = 0;
        
        // buffer for storing incoming data, logically populated by kernel
        std::vector<char> inboundData_;
        size_t nextRcvValidIndex_ = 0;
        std::function<void(McastSocket* s)> recvCallback_ = nullptr;

        std::string timeStr_;
        Logger& logger_;
    };
}