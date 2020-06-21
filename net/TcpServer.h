//
// Created by parallels on 5/13/20.
//

#ifndef MUDUO_STUDY_TCPSERVER_H
#define MUDUO_STUDY_TCPSERVER_H


#include <map>

#include <vector>
#include "base/CallBack.h"
#include "TcpConnection.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "LoopThreadPool.h"
class EventLoop;
class TcpServer {
public:

    typedef std::shared_ptr<TcpConnection> TcpConnectionptr;
    typedef std::map<int,TcpConnectionptr> TcpConnectionMap;

    TcpServer(const InetAddress &addr,EventLoop *loop);

    void newConnectionCallback(int fd, const InetAddress & inetAddress);

    void setOnMessageCallback(readTcpEventCallback cb){onMessage_ =cb;}
    void setOnConnectionCallback(TcpEventCallback cb){onConnection_ =cb;}
    void setOnWriteCompleteCallback(TcpEventCallback cb){writeCompleteCallback =cb;}
    void start();

private:
    void removeInLoop(const TcpConnectionptr &);
    void removeTcpConnection(const TcpConnectionptr &);

    readTcpEventCallback onMessage_;

    TcpEventCallback onConnection_;
    TcpEventCallback closeCallback;
    TcpEventCallback writeCompleteCallback;

    std::vector<TcpConnectionptr> tcpConnectionList_;
    TcpConnectionMap tcpConnectionMap_;
    Acceptor acceptor_;
    EventLoop* loop_;
    InetAddress localaddr_;
    LoopThreadPool eventLoopThreadPool_;

};




#endif //MUDUO_STUDY_TCPSERVER_H
