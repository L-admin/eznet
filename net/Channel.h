#pragma once

#include <functional>
#include <string>
#include "../base/Timestamp.h"

namespace net
{
    class EventLoop;

    class Channel
    {
    public:
        typedef std::function<void()> EventCallback;
        typedef std::function<void(Timestamp)> ReadEventCallback;

    public:
        Channel(EventLoop *loop, int fd);

        void handleEvent(Timestamp receiveTime);

        void setReadCallback(const ReadEventCallback &cb)
        {
            readCallback_ = cb;
        }

        void setWriteCallback(const EventCallback &cb)
        {
            writeCallback_ = cb;
        }

        void setErrorCallback(const EventCallback &cb)
        {
            errorCallback_ = cb;
        }

        int fd() const { return fd_; }
        int events() const { return events_; }
        void set_revents(int revt) { revents_ = revt; }
        bool isNoneEvent() const { return events_ == kNoneEvent; }

        int index() { return index_; }
        void set_index(int idx) { index_ = idx; }
        EventLoop *ownerLoop() { return loop_; }

        void enableReading()
        {
            events_ |= kReadEvent;
            update();
        }

        void enableWriting()
        {
            events_ |= kWriteEvent;
            update();
        }

        void disableWriting()
        {
            events_ &= ~kWriteEvent;
            update();
        }

        void disableAll()
        {
            events_ = kNoneEvent;
            update();
        }

    private:
        void update();
        std::string eventsToString(int fd, int ev);

    private:
        static const int kNoneEvent;
        static const int kReadEvent;
        static const int kWriteEvent;

        EventLoop *loop_;
        const int fd_;
        int events_;
        int revents_;
        int index_;
        ReadEventCallback readCallback_;
        EventCallback writeCallback_;
        EventCallback closeCallback_;
        EventCallback errorCallback_;
    }; // namespace net
} // namespace net