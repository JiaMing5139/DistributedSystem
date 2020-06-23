//
// Created by parallels on 5/25/20.
//

#ifndef MUDUO_STUDY_TCPCLIENT_H
#define MUDUO_STUDY_TCPCLIENT_H

#include "base/CallBack.h"
#include "base/noncopyable.h"
#include "Connector.h"
#include <atomic>
#include <memory>
class InetAddress;
class EventLoop;
class TcpConnection;
class TcpClient : public noncopyable {
public:
    enum status{
        connecting,
        connected,
        closing,
        closed,
    };

    typedef std::shared_ptr<TcpConnection> TcpConnectionptr;
    TcpClient(EventLoop * loop,const InetAddress & addr );
    void start() ;
    void disconnect();
    void send(const std::string & msg);
    TcpConnectionptr connection(){return tcpConnectionptr_;}
    void setOnMessageCallback(readTcpEventCallback cb){onMessage_ =cb;}
    void setOnConnectionCallback(TcpEventCallback cb){onConnection_ =cb;}
    void setOnWriteCompleteCallback(TcpEventCallback cb){writeCompleteCallback =cb;}
private:
    readTcpEventCallback onMessage_;
    InetAddress peerAddr;
    TcpEventCallback onConnection_;
    TcpEventCallback closeCallback;
    TcpEventCallback writeCompleteCallback;
    void removeTcpConnection();
    void connectionFaild(int);


    std::atomic<status> status_;
    void newConnection(int fd);
   TcpConnectionptr tcpConnectionptr_;
   Connector connector_;
   EventLoop * baseloop_;
};


#endif //MUDUO_STUDY_TCPCLIENT_H
