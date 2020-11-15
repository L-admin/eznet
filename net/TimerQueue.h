#pragma once

#include "Timer.h"
#include "TimerId.h"
#include "Channel.h"
#include <map>
#include <set>

namespace net
{

class EventLoop;

class TimerQueue
{
public:
    explicit TimerQueue(EventLoop *loop);
    TimerQueue(const TimerQueue &rhs) = delete;
    TimerQueue operator=(const TimerQueue &rhs) = delete;
    ~TimerQueue();

    TimerId addTimer(const TimerCallback &cb,
                     Timestamp when,
                     int64_t interval,
                     int64_t repeatCount);

    TimerId addTimer(TimerCallback &&cb,
                     Timestamp when,
                     int64_t interval,
                     int64_t repeatCount);

    void removeTimer(TimerId timerId);

    void cancel(TimerId timerId, bool off);

    void doTimer();

private:
    typedef std::pair<Timestamp, Timer *> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer *, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

private:
    void addTimerInLoop(Timer *timer);
    void removeTimerInLoop(TimerId timerId);
    void cancelTimerInLoop(TimerId timerId, bool off);
    void insert(Timer *timer);

private:
    EventLoop *loop_;
    // Timer list sorted by expiration
    TimerList timers_;
};
} // namespace net