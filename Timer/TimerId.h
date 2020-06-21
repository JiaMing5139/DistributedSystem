//
// Created by parallels on 6/3/20.
//

#ifndef MUDUO_STUDY_TIMERID_H
#define MUDUO_STUDY_TIMERID_H


#include <memory>
#include "Timestamp.h"
#include "Timer.h"

class TimerId {
public:
    TimerId() = default;
    typedef std::weak_ptr<Timer> TimerId_base;
    typedef std::shared_ptr<Timer> Timerptr;
    typedef std::pair<Timestamp,Timerptr> entry;
    explicit TimerId(const Timestamp& timestamp,const Timerptr& timer):
    timerId_(timer),
    timetamp_(timestamp),
    timerPair_(timestamp,timer)
    {

    }
    Timerptr getTimer() const{
       return timerId_.lock();
    }
    entry getPair() const {
        entry ret(0,nullptr);
        if(auto timer = timerId_.lock()) {
            ret.first = timer->expiration();
            ret.second = std::move(timer);
            return ret;
        }
        return ret;

    }
    

private:
    Timestamp timetamp_;
    TimerId_base timerId_;
    std::pair<Timestamp,Timerptr> timerPair_;
};


#endif //MUDUO_STUDY_TIMERID_H
