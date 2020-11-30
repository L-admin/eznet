#pragma once

#include "InetAddress.h"
#include <netinet/in.h>

namespace net
{
    class Socket
    {
    public:
        explicit Socket(int sockfd) : sockfd_(sockfd)
        {
        }

        ~Socket();

        int fd() const { return sockfd_; }

        void setReuseAddr(bool on);

        void setReusePort(bool on);

        void bindAddress(const InetAddress &addr);

        void listen();

        int accept(InetAddress *peeraddr);

    private:
        int sockfd_;
    };

    namespace sockets
    {
        int createNonblockingOrDie();

        void setReuseAddr(int sockfd, bool on);

        void setReusePort(int sockfd, bool on);

        void bindOrDie(int sockfd, const struct sockaddr_in &addr);

        void listenOrDie(int sockfd);

        int accept(int sockfd, struct sockaddr_in *addr);

        int getSocketError(int sockfd);
    } // namespace sockets
} // namespace net