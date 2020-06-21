//
// Created by parallels on 5/1/20.
//
#ifdef __linux__
#include "Epoll.h"
#include <string.h>
#include <assert.h>
#include <vector>
#include "base/Types.h"
#define NDEBUG
namespace {
    const int kNew = -1;
    const int kDeleted = 0;
    const int kAdded = 1;
    std::vector<std::string> epollOperation= {"add","delete","modifiy"};
}


void Epoll::removeChannel(Poller::Channelptr channel) {
assertInLoopThread();
assert(channel->index() == kAdded);
    if(channel->index() == kAdded){
        update(EPOLL_CTL_DEL, channel);
        channel->setIndex(kDeleted);
        channels_.erase(channel->fd());
    }
}

void Epoll::updateChannel(Poller::Channelptr channel) {
    assertInLoopThread();
    //add

    if(channel->index() == kNew || channel->index() == kDeleted){

        update(EPOLL_CTL_ADD, channel);
        channel->setIndex(kAdded);
        Poller::channels_[channel->fd()] = channel;

#ifdef  NDEBUG
        LOG_TRACE << "fd:"<< Poller::channels_[channel->fd()]->fd()<< " has been added";
#endif


    }else{

        update(EPOLL_CTL_MOD, channel);

    }
    //update
}

bool Epoll::hasChannel(Poller::Channelptr) {
    assertInLoopThread();
    return false;
}

Epoll::~Epoll() {

}

void Epoll::poller_wait(int timeOut, Poller::Channelptrlist *activedChannels) {
    //epoll wait

    int  numEvents = ::epoll_wait(this->epollfd_,&*events_.begin(),events_.size(),timeOut);

    if (numEvents > 0) {
        // LOG_TRACE << "Epoll: epoll:" <<numEvents << " happened" ;
         if(implicit_cast<size_t>(numEvents) == events_.size()){
             events_.resize(events_.size() * 2);
         }
         fillActiveChannels( numEvents,  activedChannels);

    } else if(numEvents == 0){
       // LOG_TRACE << "noting happened" ;
    }else{
        if(errno != EINTR){
            perror("poller_wait");
            LOG_TRACE << "Epoller::poll()";
            abort();
        }
    }

}

void Epoll::update(int operation, Poller::Channelptr channel) {
    struct epoll_event ev;
    bzero(&ev,sizeof ev);
    ev.data.fd = channel->fd();
    ev.events = channel->events();
    LOG_TRACE << " epoll_ctl op = " << epollOperation[operation] <<" fd:" << channel->fd();
    if(::epoll_ctl(epollfd_,operation, channel->fd(),&ev)){
        perror("epoll_ctl_add");
        LOG_TRACE << "error epoll_ctl op = " << operation;
        LOG_TRACE << "error fd = " << channel->fd();
        LOG_TRACE << "error epollfd = " << epollfd_;
        LOG_TRACE << "error &ev = " << ev.events;
    }
}

Epoll::Epoll(EventLoop *loop) :
Poller(loop),
events_(kInitEventListSize) ,
epollfd_(createFd()){

}

void Epoll::fillActiveChannels(int numEvents, Poller::Channelptrlist *actived) {

#ifdef NDEBUG
    assert( actived );
    assert( numEvents > 0) ;
#endif

    for(int index = 0 ; index < numEvents ; index ++){

        Channelptr channel =  Poller::channels_[events_[index].data.fd];
        if(channel){
          //  LOG_TRACE << "activdchannel fd found:" << channel->fd() << "type of activedEvent:" << events_[index].events;
            channel->setRevents(events_[index].events);
            actived->push_back(channel);
        }else{
            LOG_TRACE << "channel not found:";
            abort();
        }

    }


}
#endif