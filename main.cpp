#include <sys/timerfd.h>
#include "./net/EventLoop.h"
#include "./net/Channel.h"
#include "./base/Timestamp.h"
#include <unistd.h>
#include <strings.h>


using namespace net;

EventLoop *g_loop;

void timeout(Timestamp ms)
{
    printf("Timeout!\n");
    g_loop->quit();
}

int main()
{
    EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();

    ::close(timerfd);
}