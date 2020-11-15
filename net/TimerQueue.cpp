#include "TimerQueue.h"
#include "EventLoop.h"

namespace net
{
TimerQueue::TimerQueue(EventLoop *loop) : loop_(loop)
{
}

TimerQueue::~TimerQueue()
{
    for (auto it = timers_.begin(); it != timers_.end(); ++it)
    {
        delete it->second;
    }
}

TimerId TimerQueue:: addTimer(const TimerCallback &cb,
                             Timestamp when,
                             int64_t interval,
                             int64_t repeatCount)
{
    Timer *timer = new Timer(cb, when, interval, repeatCount);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

TimerId TimerQueue::addTimer(TimerCallback &&cb,
                             Timestamp when,
                             int64_t interval,
                             int64_t repeatCount)
{
    Timer *timer = new Timer(std::move(cb), when, interval, repeatCount);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::removeTimer(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::removeTimerInLoop, this, timerId));
}

void TimerQueue::cancel(TimerId timerId, bool off)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timerId, off));
}

void TimerQueue::doTimer()
{
    loop_->assertInLoopThread();

    Timestamp now(Timestamp::now());

    for (auto it = timers_.begin(); it != timers_.end();)
    {
        if (it->second->expiration() <= now)
        {
            it->second->run();
            if (it->second->getRepeatCount() == 0)
            {
                it = timers_.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            break;
        }
    }
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    loop_->assertInLoopThread();
    insert(timer);
}

void TimerQueue::removeTimerInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    Timer *timer = timerId.timer_;
    for (auto it = timers_.begin(); it != timers_.end(); ++it)
    {
        if (it->second == timer)
        {
            timers_.erase(it);
            break;
        }
    }
}

void TimerQueue::cancelTimerInLoop(TimerId timerId, bool off)
{
    loop_->assertInLoopThread();

    Timer *timer = timerId.timer_;
    for (auto it = timers_.begin(); it != timers_.end(); ++it)
    {
        if (it->second == timer)
        {
            it->second->cancel(off);
            break;
        }
    }
}

void TimerQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();
    Timestamp when = timer->expiration();
    timers_.insert(Entry(when, timer));
}
} // namespace net