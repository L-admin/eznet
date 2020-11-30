#include "Sockets.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../log/AsyncLog.h"
#include <string.h>

namespace net
{
    Socket::~Socket()
    {
        ::close(sockfd_);
    }

    void Socket::setReuseAddr(bool on)
    {
        sockets::setReuseAddr(sockfd_, on);
    }

    void Socket::setReusePort(bool on)
    {
        sockets::setReusePort(sockfd_, on);
    }

    void Socket::bindAddress(const InetAddress &addr)
    {
        sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
    }

    void Socket::listen()
    {
        sockets::listenOrDie(sockfd_);
    }

    int Socket::accept(InetAddress *peeraddr)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        int connfd = sockets::accept(sockfd_, &addr);
        if (connfd >= 0)
        {
            peeraddr->setSockAddrInet(addr);
        }

        return connfd;
    }

    namespace sockets
    {
        int createNonblockingOrDie()
        {
            int sockfd = ::socket(AF_INET,
                                  SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                                  IPPROTO_TCP);
            if (sockfd < 0)
            {
                LOGF("sockets::createOrDie");
            }

            return sockfd;
        }

        void setReuseAddr(int sockfd, bool on)
        {
            int optval = on ? 1 : 0;
            ::setsockopt(sockfd,
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &optval,
                         static_cast<socklen_t>(sizeof(optval)));
            // FIXME CHECK
        }

        void setReusePort(int sockfd, bool on)
        {
            int optval = on ? 1 : 0;
            int ret = ::setsockopt(sockfd,
                                   SOL_SOCKET,
                                   SO_REUSEPORT,
                                   &optval,
                                   static_cast<socklen_t>(sizeof(optval)));
            if (ret < 0 && on)
            {
                LOGSYSE("SO_REUSEPORT failed.");
            }
        }

        void bindOrDie(int sockfd, const struct sockaddr_in &addr)
        {
            int ret = ::bind(sockfd,
                             (struct sockaddr *)(&addr),
                             static_cast<socklen_t>(sizeof(addr)));
            if (ret < 0)
            {
                LOGF("sockets::bindOrDie");
            }
        }

        void listenOrDie(int sockfd)
        {
            int ret = ::listen(sockfd, SOMAXCONN);
            if (ret < 0)
            {
                LOGF("sockets::listenOrDie");
            }
        }

        int accept(int sockfd, struct sockaddr_in *addr)
        {
            socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
            int connfd = ::accept4(sockfd,
                                   (struct sockaddr *)(addr),
                                   &addrlen,
                                   SOCK_NONBLOCK | SOCK_CLOEXEC);
            if (connfd < 0)
            {
                int savedErrno = errno;
                LOGSYSE("Socket::accept");
                switch (savedErrno)
                {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO: // ???
                case EPERM:
                case EMFILE: // per-process lmit of open file desctiptor ???
                    // expected errors
                    errno = savedErrno;
                    break;
                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                    // unexpected errors
                    LOGF("unexpected error of ::accept %d", savedErrno);
                    break;
                default:
                    LOGF("unknown error of ::accept %d", savedErrno);
                    break;
                }
            }

            return connfd;
        }

        int getSocketError(int sockfd)
        {
            int optval;
            socklen_t optlen = static_cast<socklen_t>(sizeof optval);
            if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
            {
                return errno;
            }

            return optval;
        }
    } // namespace sockets
} // namespace net
