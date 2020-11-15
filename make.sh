#!/bin/bash

g++ -g  ./base/AsyncLog.cpp ./base/Timestamp.cpp  \
    ./net/EpollPoller.cpp ./net/Channel.cpp  \
    ./net/Timer.cpp  ./net/TimerQueue.cpp ./net/EventLoop.cpp \
    ./main.cpp  -o main -lpthread