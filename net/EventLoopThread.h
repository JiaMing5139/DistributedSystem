//
// Created by parallels on 5/11/20.
//

#ifndef MUDUO_STUDY_EVENTLOOPTHREAD_H
#define MUDUO_STUDY_EVENTLOOPTHREAD_H

#include <thread>
#include "EventLoop.h"
#include "base/noncopyable.h"
#include <mutex>
#include <condition_variable>
class EventLoopThread: noncopyable  {
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop * startLoop();
private:
     bool running_;
     //must be created in thread
     std::mutex mutex_;
     std::condition_variable cond_;
     EventLoop* loop_;
     std::thread thread_;
};


#endif //MUDUO_STUDY_EVENTLOOPTHREAD_H
