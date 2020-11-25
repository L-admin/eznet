#pragma once

#include "Callback.h"
#include "InetAddress.h"
#include "Timestamp.h"
#include <memory>
namespace net
{
    class EventLoop;
    class Channel;
    class Socket;

    class TcpConnection : public std::enable_shared_from_this<TcpConnection>
    {
    public:
        TcpConnection(EventLoop *loop,
                      const string &name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);

        ~TcpConnection();

        bool connected() const
        {
            return state_ == StateE::kConnected;
        }

        const std::string &name() const { return name_; }

        const InetAddress &localAddress() const
        {
            return localAddr_;
        }

        const InetAddress &peerAddress() const
        {
            return peerAddr_;
        }

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }

        void connectEstablished();

    private:
        enum class StateE
        {
            kConnecting,
            kConnected,
            kDisconnecting,
            kDisconnected,
        };

    private:
        void setState(StateE s)
        {
            state_ = s;
        }

        void handleRead(Timestamp receiveTime);
        const char *stateToString() const;

    private:
        EventLoop *loop_;
        std::string name_;
        StateE state_;
        std::unique_ptr<Socket> socket_;
        std::unique_ptr<Channel> channel_;
        InetAddress localAddr_;
        InetAddress peerAddr_;
        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
    };
} // namespace net