#ifndef MUDUO_STUDY_EVENTLOOP_H
#define MUDUO_STUDY_EVENTLOOP_H

#include "base/noncopyable.h"
#include "base/CallBack.h"
#include "Timer/TimerQueue.h"
#include <cstdlib>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include "myeventFd.h"
class Poller;
class Channel;
class EventLoop: noncopyable{
public:
    EventLoop();
    typedef std::shared_ptr<Channel> Channelptr;
    typedef std::vector<Channelptr> Channelptrlist;
    void assertInLoopThread(){
        if(!runInthread()){

            abortInthread();
        }
    }


    ~EventLoop();
    void loop();
    void update(Channelptr channel);
    void cancel(Channelptr channel);
    void quit();


    // TimerId用于指定定时器操作（像取消操作) 尚未实现
    TimerId runAt(Timestamp timerstamp, TimerCallback cb);
    TimerId runAfter(double delay, TimerCallback cb);
    TimerId runEvery(double interval, TimerCallback cb);
    void cancleTimer(const TimerId & id);


    void runInLoop( funcCallback  cb);

private:
    void cancleTimerInloop(const TimerId &id);
    bool runInthread();
    void abortInthread();

    const pid_t threadId_;
    const std::unique_ptr<Poller> poll_; // can't be copied and moved,replace scoped_ptr
    Channelptrlist activedChannls;

    TimerQueue timerQueue;
    std::atomic<bool> looping_;
    std::atomic<bool> quit_;


    void queueInloop(funcCallback );
    void wakeup();
    void doPendingFunctors();
    std::atomic<bool> doingFunctors;
    std::atomic<bool> callingPendingFunctors_;
    //int wakeUpfd_;
    Jimmy::myeventFd wakeUpfd_;
    std::shared_ptr<Channel> wakeupChannel_;
    std::vector<funcCallback> funcList_;
    std::mutex mutex_;
};


#endif