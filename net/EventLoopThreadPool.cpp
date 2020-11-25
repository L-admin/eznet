#include "EventLoopThreadPool.h"
#include <stdio.h>
#include <assert.h>
#include <sstream>
#include <string>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Callbacks.h"

namespace net
{
    EventLoopThreadPool::EventLoopThreadPool() : baseLoop_(NULL),
                                                 started_(false),
                                                 numThreads_(0),
                                                 next_(0)
    {
    }

    EventLoopThreadPool::~EventLoopThreadPool()
    {
        // Don't delete loop, it's stack variable
    }

    void EventLoopThreadPool::init(EventLoop *baseLoop, int numThreads)
    {
        numThreads_ = numThreads;
        baseLoop_ = baseLoop;
    }

    void EventLoopThreadPool::start(const ThreadInitCallback &cb = ThreadInitCallback())
    {
        // assert(baseLoop_);
        if (baseLoop_ == NULL)
        {
            return;
        }

        // assert(!started_);
        if (started_)
        {
            return;
        }

        for (int i = 0; i < numThreads_; ++i)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);

            std::unique_ptr<EventLoopThread> t(new EventLoopThread(cb, buf));
            loops_.push_back(t->startLoop());
            threads_.push_back(std::move(t));
        }

        if (numThreads_ == 0 && cb)
        {
            cb(baseLoop_);
        }
    }

    void EventLoopThreadPool::stop()
    {
        for (auto &it : threads_)
        {
            it->stopLoop();
        }
    }

    EventLoop *EventLoopThreadPool::getNextLoop()
    {
        // TODO
    }

    EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode)
    {
        // TODO
    }

    std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
    {
        // TODO
    }

    const std::string EventLoopThreadPool::info() const
    {
        std::stringstream ss;

        ss << "print threads id info " << endl;
        for (size_t i = 0; i < loops_.size(); i++)
        {
            ss << i << ": id = " << loops_[i]->getThreadID() << endl;
        }

        return ss.str();
    }
} // namespace net