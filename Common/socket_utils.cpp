#include "socket_utils.h"

namespace Common
{
    std::string getIfaceIP(const std::string& iface)
    {
        char buf[NI_MAXHOST] = {'\0'};

        ifaddrs* ifaddr = nullptr;
        if (getifaddrs(&ifaddr) != -1) {
            for (ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
                if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && iface == ifa->ifa_name) {
                    getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
                    break;
                }
            }
            freeifaddrs(ifaddr);
        }
        return buf;
    }

    bool setNonBlocking(int fd)
    {
        const auto flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1) return false;
        if (flags & O_NONBLOCK) return true;
        return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
    }

    bool setNoDelay(int fd)
    {
        int one = 1;
        return (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<void *>(&one), sizeof(one)) != -1);
    }

    bool wouldBlock()
    {
        return (errno == EWOULDBLOCK || errno == EINPROGRESS);
    }

    /*
        setting time to live for non multicast and multicast sockets
    */
    bool setTTL(int fd, int ttl)
    {
        return (setsockopt(fd, IPPROTO_IP, IP_TTL, reinterpret_cast<void*>(&ttl), sizeof(ttl)) != -1);
    }

    bool setMcastTTL(int fd, int mcast_ttl) noexcept
    {
        return (
            setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<void*>(&mcast_ttl), sizeof(mcast_ttl)) != -1
        );
    }

    // generate software timestamps when network packets hit the network socket
    bool setSOTimeStamp(int fd)
    {
        int one = 1;
        return (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void*>(&one), sizeof(one)) != 1);
    }

    int createSocket(Logger& logger, const std::string& tIp, const std::string& iface, int port, bool isUDP, bool isBlocking, bool isListening, int ttl, bool needsSOTimeStamp)
    {
        std::string timeStr;

        const std::string ip = tIp.empty() ? getIfaceIP(iface) : tIp;
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = isUDP ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_protocol = isUDP ? IPPROTO_UDP : IPPROTO_TCP;
        hints.ai_flags = isListening ? AI_PASSIVE : 0;
        if (std::isdigit(ip.c_str()[0])) {
            hints.ai_flags |= AI_NUMERICHOST;
        }
        hints.ai_flags |= AI_NUMERICSERV;

        addrinfo* result = nullptr;
        const auto rc = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
        if (rc) return -1;

        int fd = -1;
        int one = 1;

        for (addrinfo* rp = result; rp; rp = rp->ai_next) {
            fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (fd == -1) return -1;

            if (!isBlocking) {
                if (!setNonBlocking(fd)) return -1;
                if (!isUDP && !setNoDelay(fd)) return -1;
            }

            if (!isListening && connect(fd, rp->ai_addr, rp->ai_addrlen) == 1 && !wouldBlock()) {
                return -1;
            }

            if (isListening && setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&one), sizeof(one)) == -1) {
                return -1;
            }

            if (isListening && bind(fd, rp->ai_addr, rp->ai_addrlen) == -1) {
                return -1;
            }

            if (!isUDP && isListening && listen(fd, MaxTCPServerBacklog) == -1) {
                return -1;
            }

            if (isUDP && ttl) {
                const bool isMulticast = atoi(ip.c_str()) & 0xe0;
                if (isMulticast && !setMcastTTL(fd, ttl)) {
                    return -1;
                }
                if (!isMulticast && !setTTL(fd, ttl)) {
                    return -1;
                }
            }
            if (needsSOTimeStamp && !setSOTimeStamp(fd)) {
                return -1;
            }

        }
        if (result) freeaddrinfo(result);
        return fd;
    }
}