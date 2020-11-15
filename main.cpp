#include "./net/EventLoop.h"
#include "./net/Channel.h"
#include "./base/Timestamp.h"

using namespace net;

void timeout()
{
    printf("timer cb\n");
}

int main()
{
    EventLoop loop;

    loop.runEvery(2000000, timeout);

    loop.loop();
}