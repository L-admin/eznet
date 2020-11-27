#include "net/EventLoop.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include <stdio.h>

using namespace net;

void onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toIpPort().c_str());
    }
}

void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp receiveTime)
{
    printf("onMessage(): recceived\n");
}

int main()
{
    InetAddress listenAddr(9981);
    EventLoop loop;
    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();
}