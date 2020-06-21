//
// Created by parallels on 5/4/20.
//

#include "Timer.h"
#include "Timestamp.h"

void Timer::restart(Timestamp now) {
    expiration_ = addTime(now,this->interval_);
}
