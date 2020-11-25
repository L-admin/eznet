#include "TcpConnection.h"
#include "Sockets.h"
#include "Channel.h"
#include "../log/AsyncLog.h"
#include "EventLoop.h"

namespace net
{
    TcpConnection::TcpConnection(EventLoop *loop,
                                 const string &name,
                                 int sockfd,
                                 const InetAddress &localAddr,
                                 const InetAddress &peerAddr) : loop_(loop),
                                                                name_(name),
                                                                state_(StateE::kConnecting),
                                                                socket_(new Socket(sockfd)),
                                                                channel_(new Channel(loop, sockfd)),
                                                                localAddr_(localAddr),
                                                                peerAddr_(peerAddr)
    {
        channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    }

    TcpConnection::~TcpConnection()
    {
        LOGD("TcpConnection::dtor[%s] at 0x%x fd=%d state=%s",
             name_.c_str(), this, channel_->fd(), stateToString());
    }

    void TcpConnection::handleRead(Timestamp receiveTime)
    {
        char buf[65536];
        ssize_t n = ::read(channel_->fd(), buf, sizeof(buf));
        messageCallback_(shared_from_this(), nullptr, receiveTime);
    }

    void TcpConnection::connectEstablished()
    {
        loop_->assertInLoopThread();
        if (state_ != StateE::kConnecting)
        {
            return;
        }

        setState(StateE::kConnected);
        connectionCallback_(shared_from_this());
        channel_->enableReading();
    }

    const char *TcpConnection::stateToString() const
    {
        switch (state_)
        {
        case StateE::kDisconnected:
            return "kDisconnected";
        case StateE::kConnecting:
            return "kConnecting";
        case StateE::kConnected:
            return "kConnected";
        case StateE::kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
        }
    }
} // namespace net