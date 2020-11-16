#include "Acceptor.h"
#include <unistd.h>
#include "EventLoop.h"
#include "../log/AsyncLog.h"

namespace net
{
    Acceptor::Acceptor(EventLoop *loop,
                       const InetAddress &listenAddr)
        : loop_(loop),
          acceptSocket_(sockets::createNonblockingOrDie()),
          acceptChannel_(loop, acceptSocket_.fd()),
          listenning_(false)
    {
        acceptSocket_.setReuseAddr(true);
        acceptSocket_.setReusePort(true);
        acceptSocket_.bindAddress(listenAddr);
        acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
    }

    Acceptor::~Acceptor()
    {
        acceptChannel_.disableAll();
        acceptChannel_.remove();
    }

    void Acceptor::listen()
    {
        loop_->assertInLoopThread();
        listenning_ = true;
        acceptSocket_.listen();
        acceptChannel_.enableReading();
    }

    void Acceptor::handleRead()
    {
        loop_->assertInLoopThread();
        InetAddress peerAddr;
        int connfd = acceptSocket_.accept(&peerAddr);
        // TODO: 这里没有考虑描述符用尽的情况
        if (connfd >= 0)
        {
            string hostport = peerAddr.toIpPort();
            LOGD("Accepts of %s", hostport.c_str());
            // newConnectionCallback_实际指向
            // TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
            if (newConnectionCallback_)
            {
                newConnectionCallback_(connfd, peerAddr);
            }
            else
            {
                ::close(connfd);
            }
        }
        else
        {
            LOGSYSE("in Acceptor::handleRead");
        }
    }
} // namespace net
