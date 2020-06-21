//
// Created by parallels on 5/17/20.
//

#ifndef MUDUO_STUDY_BUFFER_H
#define MUDUO_STUDY_BUFFER_H
#include "base/Copyable.h"
#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <sstream>
#include <string.h>
class Buffer : public Copyable{
public:

    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    typedef std::vector<char> Buffer_base;



    Buffer(size_t size = kInitialSize):buffer_(size){

    }



    size_t readableBytes() const{
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const{
        return buffer_.size() - writerIndex_;
    }
    const char * peek() {
        return begin() + readerIndex_;
    };

     int32_t peekInt32(){
         int32_t be32 = 0;
         ::memcpy(&be32, peek(), sizeof(be32));
         //FIXME net to nost
         return be32;
    }

    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    void retrieve(size_t len){
        assert(len <= readableBytes());
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }
    std::string retrieveAsString(size_t len){
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std:: string peekAllAsString(){
        std::string result(peek(), readableBytes());
        return result;
    }

    std::string retrieveAllAsString() {
        std::string result(peek(), readableBytes());
        retrieveAll();
        return result;
    }

    void append(const char * data,int len){
        ensureSpace(len);
        std::copy(data, data + len , &buffer_[writerIndex_]);
        writerIndex_ += len;
    }
    void append(const std::string & msg){
        ensureSpace(msg.length());
        std::copy(msg.c_str(), msg.c_str() + msg.length()  , &buffer_[writerIndex_]);
        writerIndex_ += msg.length() ;
    }
    void ensureSpace(int len){

        if( len < writableBytes() ){

            return;
        }

        if(readerIndex_ - kCheapPrepend >= len){
            moveContentToBegin();
        }else{
            buffer_.resize(buffer_.size() + len);
        }

    }
    void moveContentToBegin(){
        int readable = readableBytes();
        for(int i  = 0; i < readableBytes() ; i ++) {
            buffer_[i + kCheapPrepend] = std::move(buffer_[readerIndex_ + i]);
        }
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
    }
    void prepend(const void *,int);
    void swap(Buffer &);

    char * begin(){
        return &(*buffer_.begin());
    }

    std::string debugInfo(){
        std::ostringstream os;
        os << "readerIndex_:" << readerIndex_ <<" writerIndex_:" <<writerIndex_;
        return  os.str();
    }
    int readFd(int,int*);

private:


    Buffer_base buffer_;
    int readerIndex_ = kCheapPrepend;
    int writerIndex_ = kCheapPrepend;

    static const char kCRLF[];


};


#endif //MUDUO_STUDY_BUFFER_H
