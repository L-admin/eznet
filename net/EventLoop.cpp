#include "EventLoop.h"
#include "EpollPoller.h"
#include "Channel.h"
#include <sstream>
#include "../log/AsyncLog.h"
#include <sys/eventfd.h>
#include <string.h>

namespace net
{
    thread_local EventLoop *t_loopInThisThread = 0;
    const int kPollTimeMs = 1;

    EventLoop::EventLoop() : looping_(false),
                             quit_(false),
                             threadId_(std::this_thread::get_id()),
                             timerQueue_(new TimerQueue(this))
    {
        createWakeupfd();

        wakeupChannel_.reset(new Channel(this, wakeupFd_));

        poller_.reset(new EpollPoller(this));

        if (t_loopInThisThread)
        {
            LOGF("Another EventLoop  exists in this thread ");
        }
        else
        {
            t_loopInThisThread = this;
        }

        wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
        wakeupChannel_->enableReading(); // we are always reading the wakeupfd
    }

    EventLoop::~EventLoop()
    {
        assertInLoopThread();
        LOGD("EventLoop 0x%x destructs.", this);
        wakeupChannel_->disableAll();
        wakeupChannel_->remove();
        ::close(wakeupFd_);
        t_loopInThisThread = NULL;
    }

    void EventLoop::loop()
    {
        assertInLoopThread();
        looping_ = true;
        quit_ = false;

        while (!quit_)
        {
            timerQueue_->doTimer();

            activeChannels_.clear();
            pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
            for (auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
            {
                (*it)->handleEvent(pollReturnTime_);
            }

            doPendingFunctors();
        }

        looping_ = false;
    }

    void EventLoop::quit()
    {
        quit_ = true;
    }

    TimerId EventLoop::runAt(const Timestamp &time, const TimerCallback &cb)
    {
        // 只执执行一次
        return timerQueue_->addTimer(cb, time, 0, 1);
    }

    TimerId EventLoop::runAt(const Timestamp &time, TimerCallback &&cb)
    {
        // 只执执行一次
        return timerQueue_->addTimer(std::move(cb), time, 0, 1);
    }

    TimerId EventLoop::runAfter(int64_t delay, const TimerCallback &cb)
    {
        Timestamp time(addTime(Timestamp::now(), delay));
        return runAt(time, cb);
    }

    TimerId EventLoop::runAfter(int64_t delay, TimerCallback &&cb)
    {
        Timestamp time(addTime(Timestamp::now(), delay));
        return runAt(time, std::move(cb));
    }

    TimerId EventLoop::runEvery(int64_t interval, const TimerCallback &cb)
    {
        Timestamp time(addTime(Timestamp::now(), interval));
        return timerQueue_->addTimer(cb, time, interval, -1); // -1表示一直重复下去
    }

    TimerId EventLoop::runEvery(int64_t interval, TimerCallback &&cb)
    {
        Timestamp time(addTime(Timestamp::now(), interval));
        return timerQueue_->addTimer(std::move(cb), time, interval, -1); // -1表示一直重复下去
    }

    void EventLoop::cancel(TimerId timerId, bool off)
    {
        return timerQueue_->cancel(timerId, off);
    }

    void EventLoop::remove(TimerId timerId)
    {
        return timerQueue_->removeTimer(timerId);
    }

    void EventLoop::runInLoop(const Functor &cb)
    {
        if (isInLoopThread())
        {
            cb();
        }
        else
        {
            queueInLoop(cb);
        }
    }

    void EventLoop::queueInLoop(const Functor &cb)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            pendingFunctors_.push_back(cb);
        }

        if (!isInLoopThread() || callingPendingFunctors_)
        {
            wakeup();
        }
    }

    bool EventLoop::hasChannel(Channel *channel)
    {
        assertInLoopThread();
        return poller_->hasChannel(channel);
    }

    void EventLoop::updateChannel(Channel *channel)
    {
        assertInLoopThread();
        poller_->updateChannel(channel);
    }

    void EventLoop::removeChannel(Channel *channel)
    {
        if (channel->ownerLoop() != this)
        {
            return;
        }

        assertInLoopThread();

        LOGD("Remove channel, channel = 0x%x, fd = %d", channel, channel->fd());
        poller_->removeChannel(channel);
    }

    void EventLoop::abortNotInLoopThread()
    {
        std::stringstream ss;
        ss << "threadid_ = " << threadId_ << " this_thread::get_id() = " << std::this_thread::get_id();
        LOGF("EventLoop::abortNotInLoopThread - EventLoop %s", ss.str().c_str());
    }

    bool EventLoop::createWakeupfd()
    {
        wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (wakeupFd_ < 0)
        {
            LOGF("Unable to create wakeup eventfd, EventLoop: 0x%x", this);
            return false;
        }

        return true;
    }

    bool EventLoop::wakeup()
    {
        uint64_t one = 1;
        int32_t n = ::write(wakeupFd_, &one, sizeof(one));
        if (n != sizeof(one))
        {
            int error = errno;
            LOGSYSE("EventLoop::wakeup() writes %d bytes instead of 8, fd: %d, error : %d, errorinfo : %s",
                    n, wakeupFd_, error, strerror(error));
            return false;
        }

        return true;
    }

    bool EventLoop::handleRead()
    {
        uint64_t one = 1;
        int32_t n = ::read(wakeupFd_, &one, sizeof(one));
        if (n != sizeof(one))
        {
            int error = errno;
            LOGSYSE("EventLoop::wakeup() read %d bytes instead of 8, fd: %d, error: %d, errorinfo: %s",
                    n, wakeupFd_, error, strerror(error));
            return false;
        }

        return true;
    }

    void EventLoop::doPendingFunctors()
    {
        std::vector<Functor> functors;
        callingPendingFunctors_ = true;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            functors.swap(pendingFunctors_);
        }

        for (size_t i = 0; i < functors.size(); ++i)
        {
            functors[i]();
        }

        callingPendingFunctors_ = false;
    }
} // namespace net