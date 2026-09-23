#include "Common/tcp_server.h"

namespace Common
{
    explicit TCPServer::TCPServer(Logger& logger) : listenerSocket_(logger), logger_(logger)
    {
        recvCallback_ = [this](auto socket, auto rxTime) { defaultRecvCallback(socket, rxTime); };
        recvFinishedCallback_ = [this](auto socket, auto rxTime)
        {
            defaultRecvFinishCallback(socket, rxTime);
        };
    }

    void TCPServer::defaultRecvCallback(TCPSocket* socket, Nanos rxTime) noexcept
    {}

    void TCPServer::defaultRecvFinishedCallback() noexcept
    {}

    void TCPServer::destroy()
    {
        close(efd_);
        efd_ = -1;
        listenerSocket_.destroy();
    }

    void TCPServer::listen(const std::string& iface, int port)
    {
        // clear the old unrelated to sure safe operations
        destroy();

        // creates the epoll instance
        efd_ = epoll_create(1);
        ASSERT(efd_ >= 0, "epoll_create() failed error" + std::string(std::strerror(errno)));

        // creates the TCP listener socket instance
        ASSERT(
            listenerSocket_.connect("", iface, port, true) >= 0,
            "Listener socket failed to connect."
        );

        /*
            packet hits NIC -> kernel updates listenerSocket_ status
        */
        ASSERT(epollAdd(&listenerSocket_), "BRUH");
    }

    bool TCPServer::epollAdd(TCPSocket* socket)
    {
        epoll_event ev{};
        ev.events = EPOLLET | EPOLLIN;
        ev.data.ptr = reinterpret_cast<void*>(socket);
        return (epoll_ctl(efd_, EPOLL_CTL_ADD, socket->fd_, &ev) != -1);
    }

    bool TCPServer::epollDel(TCPSocket* socket)
    {
        return (epoll_ctl(efd_, EPOLL_CTL_DEL, socket->fd_, nullptr) != -1);
    }

    void TCPServer::del(TCPSocket* socket)
    {
        // removes the entire socket from all the queues
        epollDel(socket);
        sockets_.erase(std::remove(sockets_.begin(), sockets_.end(), socket), sockets_.end());
        receiveSockets_.erase(std::remove(
            receiveSockets_.begin(), receiveSockets_.end(), socket), receiveSockets_.end()
        );
        sendSockets_.erase(std::remove(
            sendSockets_.begin(), sendSockets_.end(), socket),
            sendSockets_.end()
        );
    }

    // critical function for waiting for signal from epoll_wait()
    // and then construct TCPSocket and add them into the queue for furhter processing
    void TCPServer::poll() noexcept
    {
        const int maxEvents = 1 + sockets_.size();

        // delete all the sockets from the disconnected sockets queue from all other connecting sockets from the whole struct
        for (auto socket : disconnectedSockets_) {
            del(socket);
        }

        const int n = epoll_wait(efd_, events_, maxEvents, 0);
        bool haveNewConnection = false;
        for (int i = 0; i < n; i++) {
            epoll_event& event = events_[i];
            auto socket = reinterpret_cast<TCPSocket*>(event.data.ptr);
            
            if (event.events & EPOLLIN) {
                if (socket == &listenerSocket_) {
                    haveNewConnection = true;
                    continue;
                }
            }

            // insert into the queue if it isnt already has been
            if (std::find(receiveSockets_.begin(), receiveSockets_.end(), socket) == receiveSockets_.end())
                receiveSockets_.push_back(socket);
            
            // if the either of the two flags are set, it means an error has occurred or the socket was closed
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                if (std::find(disconnectedSockets_.begin(), disconnectedSockets_.end(), socket) == disconnectedSockets_.end()) {
                    disconnectedSockets_.push_back(socket);
                }
            }
        }

        while (haveNewConnection) {
            sockaddr_storage addr;
            socklen_t addrLen = sizeof(addr);
            int fd = accept(listenerSocket_.fd_, reinterpret_cast<sockaddr*>(&addr), &addrLen);
            if (fd == -1) break;
        
            ASSERT(setNonBlocking(fd) && setNoDelay(fd), "Failed to set non-blocking or no-delay on socket");

            TCPSocket* socket = new TCPSocket(logger_);
            socket->fd_ = fd;
            socket->recvCallback_ = recvCallback_;
            ASSERT(epollAdd(socket), "Unable to add socket");

            if (std::find(sockets_.begin(), sockets_.end(), socket) == sockets_.end()) {
                sockets_.push_back(socket);
            }

            if (std::find(receiveSockets_.begin(), receiveSockets_.end(), socket) == receiveSockets_.end()) {
                receiveSockets_.push_back(socket);
            }
        }

    }
}