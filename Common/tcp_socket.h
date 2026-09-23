#pragma once

#include <functional>

#include "Common/socket_utils.h"
#include "Common/logging.h"

namespace Common
{
    constexpr size_t TCPBufferSize = 64 * 1024 * 1024;
    void defaultRecvCallback(auto socket, auto rxTime)
    {
        // should be logging the invoke of such function
        return;
    }
    void defaultRecvFinishCallback(auto socket, auto rxTime)
    {
    }

    // used to send and receive data on the sendBuffer_ and rcvBuffer_ respectively
    struct TCPSocket
    {
        explicit TCPSocket(Logger& logger) : logger_(logger)
        {
            sendBuffer_ = new char[TCPBufferSize];
            rcvBuffer_ = new char[TCPBufferSize];
            recvCallback_ = [this](auto socket, auto rxTime) {
                defaultRecvCallback(socket, rxTime);
            };
        }

        TCPSocket() = delete;
        TCPSocket(const TCPSocket&) = delete;
        TCPSocket(TCPSocket&&) = delete;
        TCPSocket& operator=(const TCPSocket&) = delete;
        TCPSocket& operator=(TCPSocket&&) = delete;

        ~TCPSocket()
        {
            destroy();
            delete[] sendBuffer_; sendBuffer_ = nullptr;
            delete[] rcvBuffer_; rcvBuffer_ = nullptr;
        }

        int connect(const std::string& ip, const std::string& iface, int port, bool isListening) noexcept;
        void send(const void* data, size_t len) noexcept;
        bool sendAndRecv() noexcept;


        int fd_ = -1;

        // buffer of data that is waiting to be sent out
        char* sendBuffer_ = nullptr;
        size_t nextSendValidIndex_ = 0;

        // buffer of data that this socket has just read
        char* rcvBuffer_ = nullptr;
        size_t nextRcvValidIndex_ = 0;

        struct sockaddr_in inInAddr;

        // keeps track of whether the two buffers are connected
        bool sendDisconnected_ = false;
        bool recvDisconnected_ = false;

        // a callback function invoked when other components want to read data from this TCP Sockket
        std::function<void(TCPSocket* s, Nanos rxTime)> recvCallback_;
        std::string timeStr_;
        Logger& logger_;

        void destroy();
    };
}