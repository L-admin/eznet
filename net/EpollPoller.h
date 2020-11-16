#pragma once
#include <map>
#include <vector>
#include "Timestamp.h"
#include <sys/epoll.h>

namespace net
{
class EventLoop;
class Channel;

class EpollPoller
{
public:
    typedef std::vector<Channel *> ChannelList;

public:
    EpollPoller(EventLoop *loop);
    EpollPoller(const EpollPoller &rhs) = delete;
    EpollPoller operator=(const EpollPoller &rhs) = delete;
    ~EpollPoller();

    Timestamp poll(int timeoutMs, ChannelList *activeChannels);
    bool updateChannel(Channel *channel);
    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel) const;
    void assertInLoopThread() const;

private:
    static const int kInitEventListSize;

    void fillActiveChannels(int numEvents,
                            ChannelList *activeChannels) const;
    bool update(int operation, Channel *channel);

private:
    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel *> ChannelMap; // key 是fd

    int epollfd_;
    EventList events_;
    ChannelMap channels_;
    EventLoop *ownerLoop_;
};
} // namespace net