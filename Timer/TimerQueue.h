//
// Created by parallels on 5/4/20.
//

#ifndef MUDUO_STUDY_TIMERQUEUE_H
#define MUDUO_STUDY_TIMERQUEUE_H
#include "base/noncopyable.h"
#include "Timer.h"
#include "TimerId.h"
class Timestamp;
class TimerQueuebase;
class EventLoop;

class TimerQueue: public noncopyable {
public:
    explicit TimerQueue(EventLoop*);
    ~TimerQueue();
    TimerId addTimer(Timestamp timestamp,Timer::TimerCallback cb,double interval);
    void cancel(const TimerId & id);


private:
    TimerQueuebase * timerimpl_;
};


#endif //MUDUO_STUDY_TIMERQUEUE_H
