//
// Created by parallels on 5/13/20.
//

#ifndef MUDUO_STUDY_TCPCONNECTION_H
#define MUDUO_STUDY_TCPCONNECTION_H

#include "Timer/TimeWheel.h"
#include <memory>
#include <atomic>
#include "base/CallBack.h"
#include "InetAddress.h"
#include "base/noncopyable.h"
#include "Buffer.h"
#include "Socket.h"
class Channel;
class EventLoop;
class RpcChannel;
class TcpConnection: public std::enable_shared_from_this<TcpConnection>, public noncopyable  {
public:
     enum status{
         kConnected,
         closed,
         closing,
         overed,
     };
    typedef std::shared_ptr<RpcChannel> RpchannelPtr;
    typedef std::shared_ptr<Channel> Channelptr;
    typedef std::shared_ptr<TcpConnection> TcpConnectionptr;
    explicit TcpConnection(EventLoop *, int fd);
    int fd() const {return sockfd_.fd();}
    void buildConnection(EventLoop *);
    ~TcpConnection();

    void setInetAddress(const InetAddress &peerAddr,const InetAddress &localAddr){
        peerAddr_ = peerAddr;
        localAddr_ = localAddr;
    }
    InetAddress localAddr() const { return localAddr_;}
    InetAddress peerAddr() const { return peerAddr_;}
    void cancel();

    void handReadEvent();
    void handleCloseEvent();
    void handleWriteEvent();
    void handleError();

    bool connected(){ return kStatus_ == kConnected;}
    void send(const std::string & msg);
    void send(Buffer * buffer);
    void shutdown();

    void setOnMessageCallback(readTcpEventCallback cb){onMessage_ =cb;}
    void setOnConnectionCallback(TcpEventCallback cb){onConnection_ =cb;}
    void setOnClosedCallback(TcpEventCallback cb){closeCallback = cb;}
    void setOnWriteCompleteCallback(TcpEventCallback cb){writeCompleteCallback =cb;}
    void setTimeWheelEntry(const TimeWheel::ConnEntryPtr& connEntryPtr){
        connEntryWeakPtr_ = connEntryPtr;
    }

     TimeWheel:: ConnEntryWeakPtr getTimeWheelEntry(){
         return connEntryWeakPtr_;
    }
    void setRpcChannel(RpchannelPtr rpchannelPtr){
         rpchannelPtr_ = rpchannelPtr;
    }



private:
    TimeWheel::ConnEntryWeakPtr connEntryWeakPtr_;

    void destoryChannel();
    readTcpEventCallback onMessage_;

    TcpEventCallback onConnection_;
    TcpEventCallback closeCallback;
    TcpEventCallback writeCompleteCallback;

    std::atomic<status>  kStatus_ ;

    InetAddress peerAddr_;
    InetAddress localAddr_;



    void sendInLoop(const std::string & msg);
    void shutdownInLoop();
    Buffer outputBuffer_;
    Buffer inputBuffer_;

    RpchannelPtr rpchannelPtr_;
    Channelptr channel_;
    Socket sockfd_;
    EventLoop * loop_;
};


#endif //MUDUO_STUDY_TCPCONNECTION_H
