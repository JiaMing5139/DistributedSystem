//
// Created by parallels on 5/22/20.
//

#ifndef MUDUO_STUDY_LOOPTHREADPOOL_H
#define MUDUO_STUDY_LOOPTHREADPOOL_H

#include <vector>
#include <mutex>
#include "EventLoopThread.h"
class EventLoopThread;
class EventLoop;
class LoopThreadPool {
   struct EventLoopThreadPack{
       EventLoop * eventptr_;
       EventLoopThread  eventLoopThread_;
       int usedtimes_;
   };
    typedef std::vector<struct EventLoopThreadPack> LoopThreadList;
public:
    explicit  LoopThreadPool(EventLoop* baseloop, int size = 0);
    ~LoopThreadPool();
    void start();
    EventLoop * getLoop();

private:
    EventLoop * baseloop_;
    LoopThreadList loopThreadList_;
    std::mutex lock_;
    size_t currentIndex_;
};


#endif //MUDUO_STUDY_LOOPTHREADPOOL_H
