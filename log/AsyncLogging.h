//
// Created by parallels on 4/14/20.
//

#ifndef LOG_ASYNCLOGGING_H
#define LOG_ASYNCLOGGING_H


#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include "base/noncopyable.h"
#include "FixedBuffer.h"
#include <mutex>
#include <condition_variable>
#define BUFFER_SIZE

class AsyncLogging :noncopyable{
public:
    explicit AsyncLogging(std::string name);
    void append(const char * msg, size_t len);
    void start(){
        std::thread t(std::bind(&AsyncLogging::threadFunc,this));
        thread_ = std::move(t);
        isRunning_ =true;
    }
    ~AsyncLogging(){
       if(isRunning_){
        cond_.notify_all();
        thread_.join();
        isRunning_ =false;
       }
    }
    std::mutex& Mutex(){return mutex_;}
private:
    void threadFunc();

    std::string name_;
    std::atomic<bool> isRunning_;
    typedef FixedBuffer<4000000> Buffer;
    typedef std::unique_ptr<Buffer> Bufferptr;
    typedef std::vector<Bufferptr> Buffers;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    Bufferptr currentBuffer_;
    Bufferptr nextBuffer_;
    Buffers buffersptr_;

};


#endif //LOG_ASYNCLOGGING_H
