//
// Created by parallels on 5/11/20.
//

#include "EventLoopThread.h"
#include "log/logger.h"


EventLoop * EventLoopThread::startLoop() {
     if(!running_) {
         std::thread t([this]() {
             EventLoop loop;
             {
                 std::lock_guard<std::mutex>(this->mutex_);
                 this->loop_ = &loop;
                 this->cond_.notify_all();
             }

             loop.loop();
             {
                     std::lock_guard<std::mutex>(this->mutex_);
                     loop_ = NULL;
             }
         });
         thread_ = std::move(t);


         std::unique_lock<std::mutex> lk(mutex_);
         while(loop_ == NULL){
             cond_.wait(lk);
         }
         lk.unlock();

         running_ = true;
         return loop_;
     }
}

EventLoopThread::EventLoopThread():
running_(false) ,
loop_(nullptr)
{

}
EventLoopThread::~EventLoopThread(){
    LOG_TRACE <<"~EventLoopThread id:";
    running_=false;
        if(loop_){
           // loop_->quit();
            thread_.join();
        }


}
