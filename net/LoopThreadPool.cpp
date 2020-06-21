//
// Created by parallels on 5/22/20.
//

#include "LoopThreadPool.h"
#include "log/logger.h"
#include "assert.h"
LoopThreadPool::LoopThreadPool(EventLoop *baseloop ,int size):
baseloop_(baseloop),
loopThreadList_(size),
currentIndex_(0)
{

}

void LoopThreadPool::start() {
   for(EventLoopThreadPack & pack:loopThreadList_){
       pack.eventptr_ = pack.eventLoopThread_.startLoop();
   }


}

EventLoop * LoopThreadPool::getLoop() {
    if( loopThreadList_.size() == 0)
        return baseloop_;

    size_t avaliableIndex=  currentIndex_ % loopThreadList_.size();
    EventLoop * ret = loopThreadList_[avaliableIndex].eventptr_;
    loopThreadList_[avaliableIndex].usedtimes_ ++;
    currentIndex_++;
    assert(ret != nullptr);
    return ret;
}

LoopThreadPool::~LoopThreadPool() {
    LOG_TRACE <<"LoopThreadPool";
}
