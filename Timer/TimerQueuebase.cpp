//
// Created by parallels on 5/4/20.
//

#include "TimerQueuebase.h"
#include "Timestamp.h"
#include "net/EventLoop.h"

#ifdef __linux__

#include <sys/timerfd.h>

#endif

#include "log/logger.h"
#include "net/Channel.h"
#include <string.h>
#include <unistd.h>
#include "base/CallBack.h"

TimerQueuebase::TimerQueuebase(EventLoop *loop) :
        timerFd_(createTimerFd()),
        loop_(loop),
        channel_(new Channel(loop, timerFd_)) {
    channel_->setReadCallBack(std::bind(&TimerQueuebase::handleEvent, this));
    channel_->enableRead();

}

void TimerQueuebase::addTimerInLoop(std::shared_ptr<Timer> timer) {
    loop_->assertInLoopThread();

    if (insert(timer)) {
        //true : saying that this is the earlist timer in the set,so we need reset the time of timerfd, make it equal to the earlist timer
        reset(timer->expiration());
    }
    debugTimerQueue();
}

TimerId TimerQueuebase::addTimer(Timestamp timestamp, Timer::TimerCallback cb, double interval) {
    std::shared_ptr<Timer> timer(new Timer(cb, timestamp, interval));
    TimerId ret(timestamp, timer);
    loop_->runInLoop(std::bind(&TimerQueuebase::addTimerInLoop, this, timer));
    return ret;

}

void TimerQueuebase::cancel(const TimerId &id) {
    auto timerpair = id.getPair();
    debugTimerQueue();
    if (timerpair.second) {
        timerpair.second->setRepeat(false);
        timerList_.erase(timerpair);
    }



}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.mircoSecond()
                           - Timestamp::now().mircoSecond();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPersecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPersecond) * 1000);
    return ts;
}

void readTimerfd(int fd) {
    uint64_t howmany = 0;
    ssize_t n = ::read(fd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany;
    if (n != sizeof howmany) {
        LOG_SYSFATAL << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}


void TimerQueuebase::reset(Timestamp timestamp) {
#ifdef  __linux__
    // LOG_TRACE<<"reset the time of timequeue" << ":" << timestamp ;
    struct itimerspec new_time;
    struct itimerspec old_time;
    bzero(&new_time, sizeof new_time);
    bzero(&old_time, sizeof old_time);
    Timestamp now(Timestamp::now());
    new_time.it_value = howMuchTimeFromNow(timestamp);
    int ret = timerfd_settime(timerFd_, 0, &new_time, &old_time);
    if (ret == -1) {
        LOG_SYSFATAL << "timerfd_set";
    }
#endif

}

int TimerQueuebase::createTimerFd() {
    int ret;
#ifdef  __linux__
    ret = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (ret == -1) {
        LOG_SYSFATAL << "timerfd_create";
    }
#endif
    return ret;
}

void TimerQueuebase::handleEvent() {

    readTimerfd(this->timerFd_);
    Timestamp now(Timestamp::now());
    std::shared_ptr<Timer> MAX(reinterpret_cast<Timer *>(UINTPTR_MAX), [](Timer *) {});
    auto sentry = entry(now, std::move(MAX));
    auto bound = timerList_.lower_bound(sentry);
//    std::cout<<"now time:" <<  now<< std:: endl;
//    std::cout<<"begin time:" <<  (*timerList_.begin()).second->expiration() << std:: endl;
    copy(timerList_.begin(), bound, std::back_inserter(activedTimerList_));

    timerList_.erase(timerList_.begin(), bound);

    // run the callback , restart
    for (const auto &timer: activedTimerList_) {
        timer.second->run();

        if (timer.second->repeat()) {
            timer.second->restart(now);
            insert(timer.second);
        }
    }

    activedTimerList_.clear();

    if (!timerList_.empty()) {
        Timestamp nextExpired = timerList_.begin()->second->expiration();
        reset(nextExpired);
    }
//


}


bool TimerQueuebase::insert(std::shared_ptr<Timer> timer) {
    auto ret = timerList_.emplace(timer->expiration(), std::move(timer));
    return ret.second && ret.first == timerList_.begin();
}

void TimerQueuebase::debugTimerQueue() {


}



