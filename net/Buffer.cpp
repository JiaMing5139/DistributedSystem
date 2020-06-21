//
// Created by parallels on 5/17/20.
//
#include <sys/uio.h>
#include "Buffer.h"
#include "unistd.h"
#include "log/logger.h"




void Buffer::prepend(const void *, int) {

}

void Buffer::swap(Buffer &rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);

}
#define READV  1
int Buffer::readFd(int fd,int* savedErrno) {
    char buff[65535];
    struct iovec vec[2] ;
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = buff;
    vec[1].iov_len = sizeof buff;
#if READV
    int n =  readv( fd, vec, 2);
#else
    int n = read(fd,begin() + writerIndex_,writable);
#endif
    if(n < 0 ){
       *savedErrno = errno;
    }else if ( n <= writable){
        writerIndex_ += n;
    }else{
//        LOG_TRACE<<"buit-in buffer:" << &buffer_[readerIndex_];
//        LOG_TRACE<<"extra buffer:" << buff;
//
//        LOG_TRACE<<"buffer capacity: " << buffer_.capacity();
        writerIndex_ = buffer_.size();
        append(buff, n - writable );

//        LOG_TRACE<<"buffer capacity:" << buffer_.capacity();
//   //     LOG_TRACE<<"buit-in buffer:" << &buffer_[readerIndex_];
    }

    return  n;

}
