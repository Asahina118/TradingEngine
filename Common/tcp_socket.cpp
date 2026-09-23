#include "Common/tcp_socket.h"

namespace Common
{
    void TCPSocket::destroy()
    {
        close(fd_);
        fd_ = -1;
    }

    int TCPSocket::connect(const std::string& ip, const std::string& iface, int port, bool isListening) noexcept
    {
        destroy();
        fd_ = createSocket(logger_, ip, iface, port, false, false, isListening, 0, true);

        inInAddr.sin_addr.s_addr = INADDR_ANY;
        inInAddr.sin_port = htons(port);
        inInAddr.sin_family = AF_INET;

        return fd_;
    }

    void TCPSocket::send(const void* data, size_t len) noexcept
    {
        if (len > 0) {
            memcpy(sendBuffer_ + nextSendValidIndex_, data, len);
            nextSendValidIndex_ += len;
        }
    }

    bool TCPSocket::sendAndRecv() noexcept
    {
        char ctrl[CMSG_SPACE(sizeof(struct timeval))];
        struct cmsghdr* cmsg = (struct cmsghdr*)& ctrl;
        struct iovec iov;
        iov.iov_base = rcvBuffer_ + nextRcvValidIndex_;
        iov.iov_len = TCPBufferSize - nextRcvValidIndex_;

        msghdr msg;
        msg.msg_control = ctrl;
        msg.msg_controllen = sizeof(ctrl);
        msg.msg_name = &inInAddr;
        msg.msg_namelen = sizeof(inInAddr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        const auto nRcv = recvmsg(fd_, &msg, MSG_DONTWAIT);
        if (nRcv > 0) {
            nextRcvValidIndex_ += nRcv;

            Nanos kernelTime = 0;
            struct timeval timeKernel;
            if (
                cmsg->cmsg_level == SOL_SOCKET &&
                cmsg->cmsg_type == SCM_TIMESTAMP &&
                cmsg->cmsg_len == CMSG_LEN(sizeof(timeKernel))
            ) {
                memcpy(&timeKernel, CMSG_DATA(cmsg), sizeof(timeKernel));
                kernelTime = timeKernel.tv_sec * NANOS_TO_SECONDS + timeKernel.tv_usec * NANOS_TO_MICROS;
            }

            const auto userTime = getCurrentNanos();
            recvCallback_(this, kernelTime);
        }

        ssize_t nSend = std::min(TCPBufferSize, nextSendValidIndex_);
        while (nSend > 0) {
            auto nSendThisMsg = std::min(static_cast<ssize_t>(nextSendValidIndex_), nSend);
            const int flags = MSG_DONTWAIT | MSG_NOSIGNAL | (nSendThisMsg < nSend ? MSG_MORE : 0);
            auto n = ::send(fd_, sendBuffer_, nSendThisMsg, flags);
            if (UNLIKELY(n < 0)) {
                if (!wouldBlock()) {
                    sendDisconnected_ = true;
                }
                break;
            }

            nSend -= n;
        }
        nextSendValidIndex_ = 0;

        return (nRcv > 0);
    }

}
