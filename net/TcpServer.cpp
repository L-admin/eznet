#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "../log/AsyncLog.h"
#include "TcpConnection.h"

namespace net
{
    TcpServer::TcpServer(EventLoop *loop,
                         const InetAddress &listenAddr)
        : listenAddr_(listenAddr),
          loop_(loop),
          acceptor_(new Acceptor(loop, listenAddr)),
          started_(0),
          nextConnId_(1)
    {
        acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                      this,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2));
    }

    TcpServer::~TcpServer()
    {
    }

    void TcpServer::start()
    {
        if (started_ == 0)
        {
            acceptor_->listen();
            started_ = 1;
        }
    }

    void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
    {
        loop_->assertInLoopThread();

        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "%s#%d", listenAddr_.toIpPort().c_str(), nextConnId_);
        ++nextConnId_;
        string connName = buf;

        LOGD("TcpServer::new connection [%s] from %s",
             connName.c_str(), peerAddr.toIpPort().c_str());

        TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, listenAddr_, peerAddr));

        connections_[connName] = conn;

        conn->setConnectionCallback(connectionCallback_);
        conn->setMessageCallback(messageCallback_);
        conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
        conn->connectEstablished();
    }

    void TcpServer::removeConnection(const TcpConnectionPtr &conn)
    {
        loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
    }

    void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
    {
        loop_->assertInLoopThread();
        LOGD("TcpServer::removeConnectionInLoop connection: %s", conn->name().c_str());
        size_t n = connections_.erase(conn->name());
        if (n != 1)
        {
            // 出现这种情况，是TcpConneaction对象在创建过程中，对方就断开连接了。
            LOGD("TcpServer::removeConnectionInLoop connection %s, connection does not exist.", conn->name().c_str());
            return;
        }

        loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
} // namespace net
