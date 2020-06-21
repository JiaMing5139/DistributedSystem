//
// Created by parallels on 5/13/20.
//

#include "TcpConnection.h"
#include "Channel.h"
#include "log/logger.h"
#include "EventLoop.h"
#include <unistd.h>
#include <assert.h>

void defaultCompleteCallback(TcpConnectionPtr ptr){
    LOG_TRACE <<"defaultCompleteCallback";
}

TcpConnection::TcpConnection(EventLoop *loop,int fd):
sockfd_(fd),
channel_(new Channel(loop,fd),[](Channel * chanptr)
{
    delete chanptr;
    LOG_TRACE<< "Deleter of Channel";
}),
kStatus_(closed),
loop_(loop)
{  // refcount of channel =  1
//    channel_->enableWrite();
    //sockfd_.setTcpNoDelay(true);
  //  sockfd_.setReuseAddr(true);
   // sockfd_.setKeepAlive(true);
    writeCompleteCallback  = defaultCompleteCallback;
}

void TcpConnection::buildConnection(EventLoop * ioloop) {
    assert(kStatus_ == closed);
    kStatus_ = kConnected;
//    channel_->setWriteCallBack(bind(&TcpConnection::handleWriteEvent,shared_from_this()));
//    channel_->setReadCallBack(bind(&TcpConnection::handReadEvent,shared_from_this()));
    channel_->setWriteCallBack(bind(&TcpConnection::handleWriteEvent,this));
    channel_->setReadCallBack(bind(&TcpConnection::handReadEvent,this));
    channel_->setCloseCallBack(bind(&TcpConnection::handleCloseEvent,this));
    channel_->setErrorCallBack(bind(&TcpConnection::handleError,this));
  //  channel_->enableRead();//refcount of channel =  2 channelMap +1
    assert(onConnection_);
    onConnection_(shared_from_this());
    ioloop->runInLoop(std::bind(&Channel::enableRead,channel_));

}

void TcpConnection::cancel() {
    if(kStatus_ == kConnected) {
        channel_->removeself();
    }
}

void TcpConnection::handReadEvent() {
    //refcount of channel =  4  | one is in channelMap of Poll, and another is in activedChannels;
//    LOG_TRACE<<"TcpConnection: read event:" <<peerAddr()<<"->"
//             << localAddr() <<" status:" << this->kStatus_  ;

    if(kStatus_ != closed){
        assert(onMessage_);
        int savedErrno;
        int n = inputBuffer_.readFd(sockfd_.fd(),&savedErrno);
        if(n == 0){

            handleCloseEvent();
        }else if (n <0){

            if (savedErrno != 11 && savedErrno != 104)
                LOG_SYSFATAL <<" readv";
            if(savedErrno == 104) {
                handleCloseEvent();
            }

        }else{
            LOG_TRACE<<"Tcpserver: newReadEvent:" <<peerAddr()<<"->"
                     <<localAddr();
            onMessage_(&inputBuffer_,shared_from_this());
        }
    }

}

void TcpConnection::handleCloseEvent() {
    loop_->assertInLoopThread();
    LOG_TRACE<<"TcpConnection: close event:" <<peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
    if(kStatus_ == kConnected || kStatus_ == closing) {
        kStatus_ = closed;

        //remove channel in Eventloop
  //      channel_->disableAll();
        channel_->removeself();//
//        LOG_TRACE << "refcount of connection before closecallback  fd: " << sockfd_.fd() << "t his addr " << this;
//        LOG_TRACE << "addr of shared_from_this before closecallback  " << shared_from_this();
//        LOG_TRACE << "fd of connection before closecallback from shared_from_this " << shared_from_this()->sockfd_.fd();
        closeCallback(shared_from_this()); //remove connnection in Tcpserver
        //closecallback中 调用map.eraser TcpConnection 引用计数 - 1 ，仅剩shared_from_this维持的引用计数，函数接受后，参数释放引用计数清零，TcpConnection释放
        // 后面不能再操作，coredump +未定义操作

        // 函数结束后 actieved中的引用被释放，channel析构

        //  LOG_TRACE << "refcount of connection after closecallback  fd: " << sockfd_ << " this addr :" << this; // ok
        //   LOG_TRACE << "refcount of channel accessed with this after closecallback:" << this->channel_.use_count();
//    LOG_TRACE << "fd of connection after closecallback from shared_from_this "<< shared_from_this()->sockfd_; //core dump
    }
}

void TcpConnection::send(const std::string &msg) {
    LOG_TRACE<<"TcpConnection: send(const std::string &msg):" << peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
    if(kStatus_ == kConnected){
        loop_->runInLoop(bind(&TcpConnection::sendInLoop,shared_from_this(),msg));
    }

}

void TcpConnection::send(Buffer *buffer) {
    LOG_TRACE<<"TcpConnection: send(Buffer *buffer):" <<peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
    if (kStatus_ == kConnected) {
        loop_->runInLoop(bind(&TcpConnection::sendInLoop, shared_from_this(), buffer->retrieveAllAsString()));
    }
}


void TcpConnection::sendInLoop(const std::string &msg) {
     loop_->assertInLoopThread();
//   // LOG_TRACE<<"TcpConnection: sendInLoop:" << peerAddr()<<"->"
//             << localAddr() <<" status:" << this->kStatus_  ;
        ssize_t nwrite;
        if(outputBuffer_.readableBytes() == 0 && !channel_->isWtriting()){
            nwrite = write(sockfd_.fd(),msg.c_str(),msg.length());
            if(nwrite >= 0){

                if(nwrite < msg.length()){
                    outputBuffer_.append(msg.c_str() + nwrite,msg.length() - nwrite);
                    channel_->enableWrite();
                }else{
                    writeCompleteCallback(shared_from_this());
                }

            }else{
                if(errno != EAGAIN && errno != 32){
                    LOG_SYSFATAL <<"write";
                }
            }

        }else{
             outputBuffer_.append(msg.c_str() ,msg.length());
        }

}

void TcpConnection::handleWriteEvent() {
        loop_->assertInLoopThread();
       LOG_TRACE<<"TcpConnection: writeable Event:" <<peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
        if(kStatus_ == closing){
            if(outputBuffer_.readableBytes() == 0 ){
                assert(channel_->isWtriting());
                channel_->disableWriting();
                writeCompleteCallback(shared_from_this());
                shutdownInLoop();

            }else{
                std::string msg =  outputBuffer_.retrieveAllAsString();
                int nwrite = write(sockfd_.fd(),msg.c_str(),msg.length());
                if(nwrite >= 0) {
                    if (nwrite < msg.length()) {
                   //     LOG_TRACE << "write blocked waiting for writeable" <<"total:" <<msg.length() <<" writed:"<<nwrite;
                        outputBuffer_.append(msg.c_str() + nwrite, msg.length() - nwrite);
                     //   LOG_TRACE << "outpubuffer size:"<< outputBuffer_.readableBytes();
                        assert(channel_->isWtriting());
                    }else{
                        assert(channel_->isWtriting());
                        channel_->disableWriting();
                        writeCompleteCallback(shared_from_this());
                        shutdownInLoop();
                    }
                } else{
                    if(errno != EAGAIN){
                        LOG_SYSFATAL <<"write";
                    }
                }
            }

        }

        if(kStatus_ == kConnected){
            if(outputBuffer_.readableBytes() == 0 ){
            //    LOG_TRACE << "send over1";
                assert(channel_->isWtriting());
                channel_->disableWriting();
                writeCompleteCallback(shared_from_this());
            }else{
                std::string msg =  outputBuffer_.retrieveAllAsString();
                int nwrite = write(sockfd_.fd(),msg.c_str(),msg.length());
                if(nwrite >= 0) {
                    if (nwrite < msg.length()) {
                   //     LOG_TRACE << "write blocked waiting for writeable" <<"total:" <<msg.length() <<" writed:"<<nwrite;
                        outputBuffer_.append(msg.c_str() + nwrite, msg.length() - nwrite);
                   //     LOG_TRACE << "outpubuffer size:"<< outputBuffer_.readableBytes();
                        assert(channel_->isWtriting());
                    }else{
                    //    LOG_TRACE << "send over2";
                        assert(channel_->isWtriting());
                        channel_->disableWriting();
                        writeCompleteCallback(shared_from_this());
                    }
                } else{
                    if(errno != EAGAIN){
                        LOG_SYSFATAL <<"write";
                    }
                }
            }

        }


}
void TcpConnection::shutdown() {
    LOG_TRACE<<"TcpConnection: shutdown:" <<peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
    if(kStatus_ == kConnected){
        if(outputBuffer_.readableBytes() != 0){
            kStatus_ = closing;
        }else{
            loop_->runInLoop(bind(&TcpConnection::shutdownInLoop,shared_from_this()));
        }
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    LOG_TRACE<<"TcpConnection: shutdownInLoop:" <<peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
    if (!channel_->isWtriting())
    {
        LOG_TRACE << "try to shutdown";
        sockfd_.shutdownWrite();
    }
}

void TcpConnection::destoryChannel() {
    channel_->disableAll();
    channel_->removeself();
}

TcpConnection::~TcpConnection() {
    LOG_TRACE<<"TcpConnection: ~TcpConnection:" <<peerAddr()<<"->"
             << localAddr() <<" status:" << this->kStatus_  ;
        kStatus_ = overed;
}

void TcpConnection::handleError() {

}








