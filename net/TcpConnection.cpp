#include "TcpConnection.h"
#include "Sockets.h"
#include "Channel.h"
#include "../log/AsyncLog.h"
#include "EventLoop.h"
#include <cassert>
#include <string.h>

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
        if (n > 0)
        {
            messageCallback_(shared_from_this(), nullptr, receiveTime);
        }
        else if (n == 0)
        {
            handleClose();
        }
        else
        {
            handleError();
        }
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

    void TcpConnection::handleClose()
    {
        loop_->assertInLoopThread();
        LOGD("fd = %d  state = %s", channel_->fd(), stateToString());
        assert(state_ == StateE::kConnected);
        channel_->disableAll();
        closeCallback_(shared_from_this());
    }

    void TcpConnection::handleError()
    {
        int err = sockets::getSocketError(channel_->fd());
        LOGE("TcpConnection::%s handleError [%d] - SO_ERROR = %s", name_.c_str(), err, strerror(err));
    }

    void TcpConnection::connectDestroyed()
    {
        loop_->assertInLoopThread();
        if (state_ == StateE::kConnected)
        {
            setState(StateE::kDisconnected);
            channel_->disableAll();

            connectionCallback_(shared_from_this());
        }
        channel_->remove();
    }

    void TcpConnection::handleWrite()
    {
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