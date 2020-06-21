//
// Created by parallels on 5/4/20.
//

#include "TimerQueue.h"
#include "TimerQueuebase.h"
TimerId TimerQueue::addTimer(Timestamp timestamp,Timer::TimerCallback cb,double interval) {
    return timerimpl_->addTimer(timestamp,cb,interval);
}



TimerQueue::TimerQueue(EventLoop * loop):timerimpl_(new TimerQueuebase(loop)) {

}

TimerQueue::~TimerQueue() {
    delete timerimpl_;
}

void TimerQueue::cancel(const TimerId &id) {
     timerimpl_->cancel(id);
}
