//
// Created by parallels on 5/24/20.
//
#include "kqueue.h"

#ifdef __APPLE__
#include "kqueue.h"
kqueue::kqueue(EventLoop *loop) : Poller(loop) {

}

void kqueue::poller_wait(int timeOut, Poller::Channelptrlist *activedChannels) {

}

void kqueue::updateChannel(Poller::Channelptr) {

}

void kqueue::removeChannel(Poller::Channelptr) {

}

bool kqueue::hasChannel(Poller::Channelptr) {
    return false;
}

kqueue::~kqueue() {

}

#endif


