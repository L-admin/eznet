#include "EpollPoller.h"
#include "Channel.h"
#include "EventLoop.h"
#include <string.h>
#include <unistd.h>
#include "../log/AsyncLog.h"

namespace net
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
    const int EpollPoller::kInitEventListSize = 16;

    EpollPoller::EpollPoller(EventLoop *loop) : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
                                                events_(kInitEventListSize),
                                                ownerLoop_(loop)
    {
        if (epollfd_ < 0)
        {
            LOGF("EPollPoller::EPollPoller");
        }
    }

    EpollPoller::~EpollPoller()
    {
        ::close(epollfd_);
    }

    Timestamp EpollPoller::poll(int timeoutMs, ChannelList *activeChannels)
    {
        int numEvents = ::epoll_wait(epollfd_,
                                     &(*events_.begin()),
                                     static_cast<int>(events_.size()),
                                     timeoutMs);
        int savedError = errno;
        Timestamp now(Timestamp::now());
        if (numEvents > 0)
        {
            fillActiveChannels(numEvents, activeChannels);
            if (static_cast<size_t>(numEvents) == events_.size())
            {
                events_.resize(events_.size() * 2);
            }
        }
        else if (numEvents == 0)
        {
        }
        else
        {
            // error happens, log uncommon ones
            if (savedError != EINTR)
            {
                errno = savedError; // FIXME: 为什么需要？
                LOGSYSE("EPollPoller::poll()");
            }
        }

        return now;
    }

    bool EpollPoller::updateChannel(Channel *channel)
    {
        assertInLoopThread();
        LOGD("fd = %d  events = %d", channel->fd(), channel->events());
        const int index = channel->index();
        if (index == kNew || index == kDeleted)
        {
            int fd = channel->fd();
            if (index == kNew)
            {
                if (channels_.find(fd) != channels_.end())
                {
                    LOGE("fd = %d  must not exist in channels_", fd);
                    return false;
                }

                channels_[fd] = channel;
            }
            else
            {
                if (channels_.find(fd) == channels_.end())
                {
                    LOGE("fd = %d  must exist in channels_", fd);
                    return false;
                }
                if (channels_[fd] != channel)
                {
                    LOGE("current channel is not matched current fd, fd = %d", fd);
                    return false;
                }
            }

            channel->set_index(kAdded);

            return update(EPOLL_CTL_ADD, channel);
        }
        else
        {
            int fd = channel->fd();
            if (channels_.find(fd) == channels_.end() ||
                channels_[fd] != channel ||
                index != kAdded)
            {
                LOGE("current channel is not matched current fd, fd = %d, channel = 0x%x", fd, channel);
                return false;
            }

            if (channel->isNoneEvent())
            {
                if (update(EPOLL_CTL_DEL, channel))
                {
                    channel->set_index(kDeleted);
                    return true;
                }
                return false;
            }
            else
            {
                return update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EpollPoller::removeChannel(Channel *channel)
    {
        assertInLoopThread();
        int fd = channel->fd();

        //assert(channels_.find(fd) != channels_.end());
        //assert(channels_[fd] == channel);
        //assert(channel->isNoneEvent());
        if (channels_.find(fd) == channels_.end() || channels_[fd] != channel || !channel->isNoneEvent())
            return;

        int index = channel->index();
        //assert(index == kAdded || index == kDeleted);
        if (index != kAdded && index != kDeleted)
            return;

        size_t n = channels_.erase(fd);
        if (n != 1)
        {
            return;
        }

        if (index == kAdded)
        {
            update(EPOLL_CTL_DEL, channel);
        }
        channel->set_index(kNew);
    }

    bool EpollPoller::hasChannel(Channel *channel) const
    {
        assertInLoopThread();
        ChannelMap::const_iterator it = channels_.find(channel->fd());
        return it != channels_.end() && it->second == channel;
    }

    void EpollPoller::assertInLoopThread() const
    {
        ownerLoop_->assertInLoopThread();
    }

    void EpollPoller::fillActiveChannels(int numEvents,
                                         ChannelList *activeChannels) const
    {
        for (int i = 0; i < numEvents; ++i)
        {
            Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
            int fd = channel->fd();
            ChannelMap::const_iterator it = channels_.find(fd);
            if (it == channels_.end() || it->second != channel)
            {
                return;
            }
            channel->set_revents(events_[i].events);
            activeChannels->push_back(channel);
        }
    }

    bool EpollPoller::update(int operation, Channel *channel)
    {
        /*
    struct epoll_event {
        __uint32_t events;
        epoll_data_t data;
    }

    typedef union epoll_data {
        void        *ptr;
        int          fd;
        __uint32_t   u32;
        __uint64_t   u64;
    } epoll_data_t;
    */

        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = channel->events();
        event.data.ptr = channel;
        int fd = channel->fd();
        if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
        {
            if (operation == EPOLL_CTL_DEL)
            {
                LOGE("epoll_ctl op=%d fd=%d, epollfd=%d, errno=%d, errorInfo: %s", operation, fd, epollfd_, errno, strerror(errno));
            }
            else
            {
                LOGE("epoll_ctl op=%d fd=%d, epollfd=%d, errno=%d, errorInfo: %s", operation, fd, epollfd_, errno, strerror(errno));
            }

            return false;
        }

        return true;
    }
} // namespace net