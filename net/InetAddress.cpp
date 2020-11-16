#include "InetAddress.h"
#include "Endian.h"
#include <arpa/inet.h>
#include <string.h>
#include "../log/AsyncLog.h"

namespace net
{
    static const in_addr_t kInaddrAny = INADDR_ANY;
    static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

    InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = sockets::hostToNetwork16(port);
        in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
        addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
    }

    InetAddress::InetAddress(const std::string &ip, uint16_t port)
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = sockets::hostToNetwork16(port);
        if (::inet_pton(AF_INET, ip.c_str(), &(addr_.sin_addr)) <= 0)
        {
            LOGSYSE("sockets::fromIpPort");
        }
    }

    std::string InetAddress::toIp() const
    {
        char buf[32] = {0};
        ::inet_ntop(AF_INET,
                    &(addr_.sin_addr),
                    buf,
                    static_cast<socklen_t>(sizeof(buf)));
        return buf;
    }

    uint16_t InetAddress::toPort() const
    {
        return sockets::networkToHost16(addr_.sin_port);
    }

    std::string InetAddress::toIpPort() const
    {
        char buf[32] = {0};
        ::inet_ntop(AF_INET,
                    &(addr_.sin_addr),
                    buf,
                    static_cast<socklen_t>(sizeof(buf)));
        uint16_t port = sockets::networkToHost16(addr_.sin_port);
        size_t end = ::strlen(buf);
        snprintf(buf + end, sizeof(buf) - end, ":%u", port);
        return buf;
    }
} // namespace net