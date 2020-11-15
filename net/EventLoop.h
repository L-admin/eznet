#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include "../base/Timestamp.h"
#include "Callback.h"
#include <mutex>
#include "TimerId.h"
#include "TimerQueue.h"

namespace net
{
    class EpollPoller;
    class Channel;

    class EventLoop
    {
    public:
        typedef std::function<void()> Functor;

    public:
        EventLoop();
        EventLoop(const EventLoop &rhs) = delete;
        EventLoop operator=(const EventLoop &rhs) = delete;
        ~EventLoop();

        void loop();

        void quit();

        TimerId runAt(const Timestamp &time, const TimerCallback &cb);
        TimerId runAt(const Timestamp &time, TimerCallback &&cb);

        TimerId runAfter(int64_t delay /* micro second*/, const TimerCallback &cb);
        TimerId runAfter(int64_t delay /* micro second*/, TimerCallback &&cb);

        TimerId runEvery(int64_t interval /* micro second*/, const TimerCallback &cb);
        TimerId runEvery(int64_t interval /* micro second*/, TimerCallback &&cb);

        void cancel(TimerId timerId, bool off);

        void remove(TimerId timerId);

        void runInLoop(const Functor &cb);

        void queueInLoop(const Functor &cb);

        bool hasChannel(Channel *channel);
        void updateChannel(Channel *channel);
        void removeChannel(Channel *channel);

        void assertInLoopThread()
        {
            if (!isInLoopThread())
            {
                abortNotInLoopThread();
            }
        }

        bool isInLoopThread() const
        {
            return threadId_ == std::this_thread::get_id();
        }

    private:
        void abortNotInLoopThread();
        bool wakeup();
        bool handleRead();
        bool createWakeupfd();
        void doPendingFunctors();

    private:
        typedef std::vector<Channel *> ChannelList;

    private:
        Timestamp pollReturnTime_;

        const std::thread::id threadId_;

        bool looping_; /* atomic */

        bool quit_;

        ChannelList activeChannels_;

        std::unique_ptr<EpollPoller> poller_;

        std::unique_ptr<TimerQueue> timerQueue_;

        int wakeupFd_;
        std::unique_ptr<Channel> wakeupChannel_;
        bool callingPendingFunctors_; /* atomic */
        std::mutex mutex_;
        std::vector<Functor> pendingFunctors_; // Guarded by mutex_

        Functor frameFunctor_;
    };
} // namespace net
