#pragma once

#include <functional>
#include <memory>
#include "Timestamp.h"

namespace net
{
    class TcpConnection;
    class Buffer;
    typedef std::function<void()> TimerCallback;
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
    typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
    typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;

    // the data has been read to (buf, len)
    typedef std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)> MessageCallback;
} // namespace net