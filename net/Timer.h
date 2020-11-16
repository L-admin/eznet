#pragma once

#include "Timestamp.h"
#include "Callback.h"
#include <atomic>
#include <stdint.h>

namespace net
{
class Timer
{
public:
    Timer(const TimerCallback &cb,
          Timestamp when,
          int64_t interval,
          int64_t repeatCount = -1);
    Timer(TimerCallback &&cb, Timestamp when, int64_t interval);

    Timer(const Timer &rhs) = delete;
    Timer &operator=(const Timer &rhs) = delete;

    void run();

    bool isCanceled() const
    {
        return canceled_;
    }

    void cancel(bool off)
    {
        canceled_ = off;
    }

    Timestamp expiration() const { return expiration_; }
    int64_t getRepeatCount() const { return repeatCount_; }
    int64_t sequence() const { return sequence_; }

    static int64_t numCreated() { return s_numCreated_; }

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const int64_t interval_;
    int64_t repeatCount_;       // 重复次数，-1 表示一直重复下去
    const int64_t sequence_;
    bool canceled_;             // 是否处于取消状态

    static std::atomic<int64_t> s_numCreated_;
};
} // namespace net