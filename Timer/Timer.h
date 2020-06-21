//
// Created by parallels on 5/4/20.
//

#ifndef MUDUO_STUDY_TIMER_H
#define MUDUO_STUDY_TIMER_H

#include <memory>
#include <functional>
#include "base/noncopyable.h"
#include "Timestamp.h"
class Timestamp;
class Channel;
class EventLoop;
class Timer : noncopyable
{

public:
    typedef std::function<void()> TimerCallback;
    Timer(TimerCallback cb, Timestamp when, double interval)
            : callback_(std::move(cb)),
              expiration_(when),
              interval_(interval),
              repeat_(interval > 0.0)
    { }

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart(Timestamp now);
    bool setRepeat(bool flag) {repeat_ = flag;}


private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    bool repeat_;

};



#endif //MUDUO_STUDY_TIMER_H
