//
// Created by parallels on 5/1/20.
//

#ifndef MUDUO_STUDY_EPOLL_H
#define MUDUO_STUDY_EPOLL_H
#ifdef __linux__
#include "Poller.h"
#include "log/logger.h"
#include <sys/epoll.h>

class Epoll: public Poller {
public:

    explicit Epoll(EventLoop* loop);
     void poller_wait(int timeOut, Channelptrlist* activedChannels) ;
     void updateChannel(Channelptr) override ;
     void removeChannel(Channelptr) override ;
     bool hasChannel(Channelptr) override ;

    ~Epoll() override;

private:
    void fillActiveChannels(int numEvents, Channelptrlist * actived);
    void update(int operation,  Channelptr channelptr);

    int createFd(){
        int ret = epoll_create(10);
        if(ret < 0 ){
            LOG_TRACE<<" epoll_create error";
            abort();
        } else
            return ret;
    }
    const int kInitEventListSize = 16;

    typedef std::vector<struct epoll_event> EventList;

    int epollfd_;
    EventList events_;

};


#endif //MUDUO_STUDY_EPOLL_H
#endif