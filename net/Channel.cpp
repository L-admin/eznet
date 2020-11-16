#include "Channel.h"
#include "EventLoop.h"
#include <sys/poll.h>
#include "../log/AsyncLog.h"
#include <sstream>

namespace net
{
    const int Channel::kNoneEvent = 0;
    const int Channel::kReadEvent = POLLIN | POLLPRI;
    const int Channel::kWriteEvent = POLLOUT;

    Channel::Channel(EventLoop *loop, int fd) : loop_(loop),
                                                fd_(fd),
                                                events_(0),
                                                revents_(0),
                                                index_(-1) /* -1 默认表示创建 */
    {
    }

    void Channel::remove()
    {
        if (!isNoneEvent())
        {
            return;
        }
        loop_->removeChannel(this);
    }

    void Channel::update()
    {
        loop_->updateChannel(this);
    }

    void Channel::handleEvent(Timestamp receiveTime)
    {
        /*
        POLLIN, 读事件
        POLLPRI, 读事件，但表示紧急数据，例如tcp socket的带外数据
        POLLRDNORM, 读事件，表示有普通数据可读
        POLLRDBAND, 读事件，表示有优先数据可读　　　
        POLLOUT, 写事件
        POLLWRNORM, 写事件，表示有普通数据可写
        POLLWRBAND, 写事件，表示有优先数据可写　　　   　　　
        POLLRDHUP(since Linux 2.6.17) ,
            Stream socket的一端关闭了连接（注意是stream socket，我们知道还有raw socket,
            dgram socket），或者是写端关闭了连接，如果要使用这个事件，必须定义_GNU_SOURCE 宏。
            这个事件可以用来判断链路是否发生异常（当然更通用的方法是使用心跳机制）。
            要使用这个事件，得这样包含头文件：
    　　    #define _GNU_SOURCE
      　　      #include<poll.h>
        POLLERR, 仅用于内核设置传出参数revents，表示设备发生错误
        POLLHUP, 仅用于内核设置传出参数revents，表示设备被挂起，
            如果poll监听的fd是socket，表示这个socket并没有在网络上建立连接,
            比如说只调用了socket() 函数，但是没有进行connect。
        POLLNVAL，仅用于内核设置传出参数revents，表示非法请求文件描述符fd没有打开
    */
        if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
        {
            LOGW("Channel::handle_event() POLLHUP");
            if (closeCallback_)
            {
                closeCallback_();
            }
        }

        if (revents_ & POLLNVAL)
        {
            LOGW("Channel::handle_event() XPOLLNVAL");
        }

        if (revents_ & (POLLERR | POLLNVAL))
        {
            if (errorCallback_)
            {
                errorCallback_();
            }
        }

        if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
        {
            // 当是侦听socket时，readCallback_指向Acceptor::handleRead
            // 当是客户端socket时，调用TcpConnection::handleRead
            if (readCallback_)
            {
                readCallback_(receiveTime);
            }
        }

        if (revents_ & POLLOUT)
        {
            // 如果是连接状态服的socket，则writeCallback_指向Connector::handleWrite()
            if (writeCallback_)
            {
                writeCallback_();
            }
        }
    }

    std::string Channel::eventsToString(int fd, int ev)
    {
        std::ostringstream oss;
        oss << fd << ": ";
        if (ev & POLLIN)
            oss << "IN ";
        if (ev & POLLPRI)
            oss << "PRI ";
        if (ev & POLLOUT)
            oss << "OUT ";
        if (ev & POLLHUP)
            oss << "HUP ";
        if (ev & POLLRDHUP)
            oss << "RDHUP ";
        if (ev & POLLERR)
            oss << "ERR ";
        if (ev & POLLNVAL)
            oss << "NVAL ";

        return oss.str().c_str();
    }

} // namespace net