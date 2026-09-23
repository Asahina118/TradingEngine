#include "mcast_socket.h"

namespace Common 
{
    int McastSocket::init(const std::string& ip, const std::string& iface, int port, bool isListening)
    {
        const SocketCfg socketCfg = 
        {
            ip, iface, port, isListening
        };
        return createSocket(logger_, socketCfg);
    }

    bool McastSocket::join(const std::string& ip)
    {
        return Common::join(socketFd_, ip);
    }

    void McastSocket::leave(const std::string&, int)
    {
        close(socketFd_);
        socketFd_ = -1;
    }

    // in a low latency system, we generally avoid separating send and receive into different functions as that is slow, so both send and receive functionality are written into the same function
    /*
        1. this function checks the NIC if any new mcast packets are coming in

        2. if it has, inboundData_ should be populated. Next, this function updates nextRcvValidIndex_, and business logic is executed in recvCallback_() (which should be making use of the inboundData_ and nextRcvValidIndex_ updated in this function)

        3. it checks if this class's member field nextSendValidIndex_ has been updated

        4. if so, it sends the data in outboundData_ to outside
    */
    bool McastSocket::sendAndRecv() noexcept
    {
        const ssize_t n_rcv = recv(socketFd_, inboundData_.data() + nextRcvValidIndex_, McastBufferSize - nextRcvValidIndex_, MSG_DONTWAIT);

        if (n_rcv > 0) {
            nextRcvValidIndex_ += n_rcv;
            // the callback function below has this pointer because in actual use case,
            // other classes define their own versions of recvCallback and assign to this socket class
            // so passing "this" pointer into the recvCallback_ can allow other classes to communicate(read data) from this exact socket that is invoking the callback function
            recvCallback_(this);
        }

        // Publish market data in the send buffer to the multicast stream.
        if (nextSendValidIndex_ > 0) {
            ssize_t n = ::send(socketFd_, outboundData_.data(), nextSendValidIndex_, MSG_DONTWAIT | MSG_NOSIGNAL);
        }
        nextSendValidIndex_ = 0;

        // returns true if new data is detected after checking
        return (n_rcv > 0);
    }
}