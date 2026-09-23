#pragma once

#include <iostream>
#include <string>
#include <unordered_set>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "macros.h"
#include "logging.h"

namespace Common
{
    constexpr int MaxTCPServerBacklog = 1024;

    struct SocketCfg
    {
        std::string ip_;
        std::string iface_;
        int port_ = -1;
        bool isUDP_ = false;
        bool isListening_ = false;
        bool needsSOTimestamp_ = false;
    };

    std::string getIfaceIP(const std::string& iface);
    bool setNonBlocking(int fd);
    bool setNoDelay(int fd);
    bool setSOTimestamp(int fd);
    bool wouldBlock();
    bool setMcastTTL(int fd, int ttl) noexcept;
    bool setTTL(int fd, int ttl);
    bool join(int fd, const std::string& ip, const std::string& iface, int port);
    int createSocket(Logger& logger, const std::string& tIp, const std::string& iface, int port, bool isUDP, bool isBlocking, bool isListening, int ttl, bool needsSoTimestamp);

    [[nodiscard]] inline auto createSocket(Logger &logger, const SocketCfg& socket_cfg) -> int 
    {
      std::string time_str;

      const auto ip = socket_cfg.ip_.empty() ? getIfaceIP(socket_cfg.iface_) : socket_cfg.ip_;

      const int input_flags = (socket_cfg.isListening_ ? AI_PASSIVE : 0) | (AI_NUMERICHOST | AI_NUMERICSERV);
      const addrinfo hints{input_flags, AF_INET, socket_cfg.isUDP_? SOCK_DGRAM : SOCK_STREAM,
                          socket_cfg.isUDP_ ? IPPROTO_UDP : IPPROTO_TCP, 0, 0, nullptr, nullptr};

      addrinfo *result = nullptr;
      const auto rc = getaddrinfo(ip.c_str(), std::to_string(socket_cfg.port_).c_str(), &hints, &result);
      ASSERT(!rc, "getaddrinfo() failed. error:" + std::string(gai_strerror(rc)) + "errno:" + strerror(errno));

      int socket_fd = -1;
      int one = 1;
      for (addrinfo *rp = result; rp; rp = rp->ai_next) {
        ASSERT((socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) != -1, "socket() failed. errno:" + std::string(strerror(errno)));

        ASSERT(setNonBlocking(socket_fd), "setNonBlocking() failed. errno:" + std::string(strerror(errno)));

        if (!socket_cfg.isListening_) { // establish connection to specified address.
          ASSERT(connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != 1, "connect() failed. errno:" + std::string(strerror(errno)));
        }

        if (socket_cfg.isListening_) { // allow re-using the address in the call to bind()
          ASSERT(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&one), sizeof(one)) == 0, "setsockopt() SO_REUSEADDR failed. errno:" + std::string(strerror(errno)));
        }

        if (socket_cfg.isListening_) {
          // bind to the specified port number.
          const sockaddr_in addr{AF_INET, htons(socket_cfg.port_), {htonl(INADDR_ANY)}, {}};
          ASSERT(bind(socket_fd, socket_cfg.isUDP_ ? reinterpret_cast<const struct sockaddr *>(&addr) : rp->ai_addr, sizeof(addr)) == 0, "bind() failed. errno:%" + std::string(strerror(errno)));
        }

        if (!socket_cfg.isUDP_ && socket_cfg.isListening_) { // listen for incoming TCP connections.
          ASSERT(listen(socket_fd, MaxTCPServerBacklog) == 0, "listen() failed. errno:" + std::string(strerror(errno)));
        }

        if (socket_cfg.needsSOTimestamp_) { // enable software receive timestamps.
          ASSERT(setSOTimestamp(socket_fd), "setSOTimestamp() failed. errno:" + std::string(strerror(errno)));
        }
    }

    return socket_fd;
  }
  
    inline auto join(int fd, const std::string &ip) -> bool {
    const ip_mreq mreq{{inet_addr(ip.c_str())}, {htonl(INADDR_ANY)}};
    return (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) != -1);
  }
}