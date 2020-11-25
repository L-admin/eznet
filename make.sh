#!/bin/bash

g++ -g ./log/AsyncLog.cpp \
    ./net/Timestamp.cpp ./net/Timer.cpp  ./net/TimerQueue.cpp \
    ./net/EventLoop.cpp ./net/EpollPoller.cpp ./net/Channel.cpp \
    ./net/EventLoopThread.cpp \
    ./net/InetAddress.cpp ./net/Sockets.cpp \
    ./net/Acceptor.cpp \
    ./net/TcpConnection.cpp \
    ./net/TcpServer.cpp \
    ./main.cpp  -o main -lpthread