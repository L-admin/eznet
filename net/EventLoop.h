#pragma once
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include "../base/Timestamp.h"

namespace net
{
    class EpollPoller;
    class Channel;

    class EventLoop
    {
    public:
        EventLoop();
        EventLoop(const EventLoop &rhs) = delete;
        EventLoop operator=(const EventLoop &rhs) = delete;
        ~EventLoop();

        void loop();

        void quit();

        void updateChannel(Channel *channel);

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

    private:
        typedef std::vector<Channel *> ChannelList;
        Timestamp pollReturnTime_;
        const std::thread::id threadId_;
        bool looping_; /* atomic */
        bool quit_;
        ChannelList activeChannels_;
        std::unique_ptr<EpollPoller> poller_;
    };
} // namespace net
