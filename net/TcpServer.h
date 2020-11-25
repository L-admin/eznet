#pragma once

#include <string>
#include "Callback.h"
#include "InetAddress.h"
#include <atomic>
#include <map>

namespace net
{
    class Acceptor;
    class EventLoop;
    class EventLoopThreadPool;

    class TcpServer
    {
    public:
        TcpServer(EventLoop *loop,
                  const InetAddress &listenAddr,
                  const std::string &nameArg);

        ~TcpServer();

        const InetAddress& listenAddr() const { return listenAddr_; }

        const std::string &name() const { return name_; }

        EventLoop *getLoop() const { return loop_; }

        void start();

        void removeConnection(const TcpConnectionPtr &conn);

        void setConnectionCallback(const ConnectionCallback &cb)
        {
            connectionCallback_ = cb;
        }

        void setWriteCompleteCallback(const WriteCompleteCallback &cb)
        {
            writeCompleteCallback_ = cb;
        }

        void setMessageCallback(const MessageCallback &cb)
        {
            messageCallback_ = cb;
        }

    private:
        /// Not thread safe, but in loop
        void newConnection(int sockfd, const InetAddress &peerAddr);

    private:
        InetAddress listenAddr_;
        const std::string name_;

        EventLoop *loop_; // the acceptor loop

        std::unique_ptr<Acceptor> acceptor_;

        std::atomic<int> started_; // TODO: 为什么不用bool

        std::map<std::string, TcpConnectionPtr> connections_;

        int nextConnId_; // always in loop thread

        ConnectionCallback connectionCallback_;
        MessageCallback messageCallback_;
        WriteCompleteCallback writeCompleteCallback_;
    };

} // namespace net
