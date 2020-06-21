//
// Created by parallels on 4/15/20.
//

#ifndef LOG_FIXEDBUFFER_H
#define LOG_FIXEDBUFFER_H

#include <printf.h>
#include <string>
#include <string.h>

template <int size>
class FixedBuffer {
public:
    const char* data() const { return buff_; }
    int length() const { return static_cast<int>(cur_ - buff_);}

    void append(const char * msg,size_t len){
        if(static_cast<size_t>(avail()) >= len){
            memcpy(cur_,msg,len);
            cur_ += len;
        }
    }
    // write to data_ directly
    char* current() { return cur_; }
    int avail()  { return static_cast<int>(end() - cur_);}
    void add(size_t len) { cur_ += len;}
    char * begin() { return buff_;}
    char * end() {return buff_ + sizeof buff_;}
    void reset() { cur_ = buff_; }
    void bzero() {  memset(buff_, 0, sizeof buff_);  cur_ = buff_; }
    void flush(){  }
    std::string toString() const { return std::string(buff_);}
    FixedBuffer():cur_(buff_){bzero();}

private:
    char* cur_;
    char buff_[size];
};


#endif //LOG_FIXEDBUFFER_H
