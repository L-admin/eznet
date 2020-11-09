#include "EventLoop.h"
#include "EpollPoller.h"
#include "Channel.h"
#include <sstream>
#include "../base/AsyncLog.h"

namespace net
{
    thread_local EventLoop *t_loopInThisThread = 0;
    const int kPollTimeMs = 1;

    EventLoop::EventLoop() : looping_(false),
                             quit_(false),
                             threadId_(std::this_thread::get_id())
    {
        poller_.reset(new EpollPoller(this));

        if (t_loopInThisThread)
        {
            LOGF("Another EventLoop  exists in this thread ");
        }
        else
        {
            t_loopInThisThread = this;
        }
    }

    EventLoop::~EventLoop()
    {
        assertInLoopThread();
        LOGD("EventLoop 0x%x destructs.", this);
        t_loopInThisThread = NULL;
    }

    void EventLoop::loop()
    {
        assertInLoopThread();
        looping_ = true;
        quit_ = false;

        while (!quit_)
        {
            activeChannels_.clear();
            pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
            for (ChannelList::iterator it = activeChannels_.begin();
                 it != activeChannels_.end();
                 ++it)
            {
                (*it)->handleEvent(pollReturnTime_);
            }
        }

        looping_ = false;
    }

    void EventLoop::quit()
    {
        quit_ = true;
    }

    void EventLoop::updateChannel(Channel *channel)
    {
        assertInLoopThread();
        poller_->updateChannel(channel);
    }

    void EventLoop::abortNotInLoopThread()
    {
        std::stringstream ss;
        ss << "threadid_ = " << threadId_ << " this_thread::get_id() = " << std::this_thread::get_id();
        LOGF("EventLoop::abortNotInLoopThread - EventLoop %s", ss.str().c_str());
    }
} // namespace net