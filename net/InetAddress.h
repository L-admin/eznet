#pragma once
#include <string>
#include <netinet/in.h>

namespace net
{
    class InetAddress
    {
    public:
        InetAddress(uint16_t port = 0, bool loopbackOnly = false);

        InetAddress(const std::string &ip, uint16_t port);

        InetAddress(const struct sockaddr_in &addr) : addr_(addr)
        {
        }

        const struct sockaddr_in &getSockAddrInet() const
        {
            return addr_;
        }

        void setSockAddrInet(const struct sockaddr_in &addr)
        {
            addr_ = addr;
        }

        std::string toIp() const;

        uint16_t toPort() const;

        std::string toIpPort() const;

    private:
        struct sockaddr_in addr_;
    };
} // namespace net