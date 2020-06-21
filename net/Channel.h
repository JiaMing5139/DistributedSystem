//
// Created by parallels on 4/29/20.
//

#ifndef MUDUO_STUDY_CHANNEL_H
#define MUDUO_STUDY_CHANNEL_H

#include <functional>
#include "base/noncopyable.h"
#include <memory>
class EventLoop;
class Channel: public noncopyable, public std::enable_shared_from_this<Channel> {
    typedef std::function<void ()> EventCallback;
    typedef std::function<void (int)> readCallback;

public:
    typedef std::shared_ptr<Channel> Channelptr;
    Channel() = delete;
    Channel(EventLoop* loop, int fd);
    ~Channel();
    void enableWrite(){events_ |= WriteEvent; update();}
    void disableWriting(){events_ &= ~ WriteEvent;update();}
    bool isWtriting(){return  events_& WriteEvent;}

    void enableRead() {events_ |= ReadEvent; update();}
    void disableAll(){ events_ = 0 ; update();}


    int index(){return this->index_;}
    int fd(){ return  this->fd_;}
    void setIndex(int index){this->index_ = index;}
    int events() { return events_; }
    int setEvents(int events) {this->events_ = events;}

    int setRevents(int events){this->revents_ = events;}
    void handleEvent();
    void setReadCallBack(readCallback cb){
        readEventCallback = cb;
    }
    void setWriteCallBack(EventCallback cb){
        writeEventCallback = cb;
    }
    void setCloseCallBack(EventCallback cb){
        closeCallback = cb;
    }
    void setErrorCallBack(EventCallback cb){
        errorCallBack = cb;
    }
    void removeself();

private:
    static const int NoneEvent;
    static const int ReadEvent;
    static const int WriteEvent;
    void update();
    const int fd_;

    int index_;
    int events_;
    int revents_;

    bool status = true;
    EventCallback closeCallback;
    EventCallback writeEventCallback;
    readCallback readEventCallback;
    EventCallback errorCallBack;

    EventLoop * loop_;


};


#endif //MUDUO_STUDY_CHANNEL_H
