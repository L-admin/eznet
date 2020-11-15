#include "Timer.h"

namespace net
{
std::atomic<int64_t> Timer::s_numCreated_;

Timer::Timer(const TimerCallback &cb,
             Timestamp when,
             int64_t interval,
             int64_t repeatCount) : callback_(cb),
                                    expiration_(when),
                                    interval_(interval),
                                    repeatCount_(repeatCount),
                                    sequence_(++s_numCreated_),
                                    canceled_(false)
{
}

Timer::Timer(TimerCallback &&cb,
             Timestamp when,
             int64_t interval) : callback_(std::move(cb)),
                                 expiration_(when),
                                 interval_(interval),
                                 repeatCount_(-1),
                                 sequence_(++s_numCreated_),
                                 canceled_(false)
{
}

void Timer::run()
{
    if (canceled_)
    {
        return;
    }

    callback_();

    if (repeatCount_ != -1)
    {
        --repeatCount_;
        if (repeatCount_ == 0)
        {
            return;
        }
    }

    expiration_ += interval_;
}
} // namespace net