#pragma once

#include "Channel.h"
#include "InetAddress.h"
#include "Sockets.h"

namespace net
{
    class EventLoop;
    class InetAddress;

    class Acceptor
    {
    public:
        typedef std::function<void(int sockfd, const InetAddress &)> NewConnectionCallback;

    public:
        Acceptor(EventLoop *loop, const InetAddress &listenAddr);
        Acceptor(const Acceptor &rhs) = delete;
        Acceptor operator=(const Acceptor &rhs) = delete;
        ~Acceptor();

        // 设置新连接到来的回调函数
        void setNewConnectionCallback(const NewConnectionCallback &cb)
        {
            newConnectionCallback_ = cb;
        }

        bool listenning() const { return listenning_; }

        void listen();

    private:
        void handleRead();

    private:
        EventLoop *loop_;
        Socket  acceptSocket_;
        Channel acceptChannel_;
        NewConnectionCallback newConnectionCallback_;
        bool listenning_;
    };
} // namespace net
