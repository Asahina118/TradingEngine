#pragma once

#include "Common/tcp_socket.h"

namespace Common
{
    struct TCPServer
    {
        explicit TCPServer(Logger& logger);

        TCPServer() = delete;
        TCPServer(const TCPServer&) = delete;
        TCPServer(TCPServer&&) = delete;
        TCPServer& operator=(const TCPServer&) = delete;
        TCPServer& operator=(TCPServer&&) = delete;

        // file descriptor for the socket data management
        int efd_ = -1;
        TCPSocket listenerSocket_;

        epoll_event events_[1024];
        std::vector<TCPSocket*> sockets_, receiveSockets_, sendSockets_, disconnectedSockets_;

        std::function<void(TCPSocket* s, Nanos rxTime)> recvCallback_;
        std::function<void()> recvFinishedCallback_;

        std::string timeStr_;
        Logger& logger_;

        void defaultRecvCallback(TCPSocket* socket, Nanos rxTime) noexcept;
        void defaultRecvFinishedCallback() noexcept;

        void destroy();
        void listen(const std::string& iface, int port);
    
        // add new TCPSocket into the queue
        bool epollAdd(TCPSocket*);
        //delete TCPSockets from the queue
        bool epollDel(TCPSocket*);

        void del(TCPSocket* socket);

        void poll() noexcept;
    };
    
}