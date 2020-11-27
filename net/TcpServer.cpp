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
    conn->connectEstablished();
}
} // namespace net
