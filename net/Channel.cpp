//
// Created by parallels on 4/29/20.
//

#include "Channel.h"
#include "EventLoop.h"
#include <sys/poll.h>
#include "log/logger.h"
#include <assert.h>
#include <unistd.h>
const int Channel::NoneEvent = 0;
const int Channel::ReadEvent = POLLIN | POLLPRI;
const int Channel::WriteEvent = POLLOUT;


Channel::Channel(EventLoop *loop, int fd) :
fd_(fd),
index_(-1),
loop_(loop),
revents_(0),
events_(0) {
}

void Channel::handleEvent() {
    if(revents_ & POLLHUP && !(revents_ & POLLIN) ){
        LOG_TRACE<<"do revents_ & POLLHUP && !(revents_ & POLLIN) : fd" << fd_;
        assert(closeCallback);
        closeCallback();
    }

    if(revents_&POLLNVAL){
    }

    if(revents_ & (POLLIN|POLLPRI|POLLHUP)) {
        assert(readEventCallback);
        readEventCallback(1);
    }


    if(revents_ & POLLOUT) {
        assert(writeEventCallback);
        writeEventCallback();
    }
    if(revents_ & (POLLERR|POLLNVAL)) {
        assert(errorCallBack);
        errorCallBack();
    }

}

void Channel::update() {
    loop_->update(shared_from_this());
}

void Channel::removeself() {
    loop_->cancel(shared_from_this());
}

Channel::~Channel() {
    status = false;
}
