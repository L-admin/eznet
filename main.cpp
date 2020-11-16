#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "net/Acceptor.h"
#include <unistd.h>
using namespace net;

void newConnection(int sockfd, const InetAddress& peerAddr)
{
    printf("newConnection from %s\n", peerAddr.toIpPort().c_str());
    write(sockfd, "how are you?\n", 13);
    close(sockfd);
}

int main()
{
    InetAddress listenAddr(9981);

    EventLoop loop;

    Acceptor acceptor(&loop, listenAddr);

    acceptor.setNewConnectionCallback(newConnection);
    acceptor.listen();

    loop.loop();
}