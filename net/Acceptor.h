//
// Created by parallels on 5/13/20.
//

#ifndef MUDUO_STUDY_ACCEPTOR_H
#define MUDUO_STUDY_ACCEPTOR_H


#include "InetAddress.h"
#include <memory>
#include "Socket.h"
#include "base/noncopyable.h"
class Channel;
class EventLoop;
class TcpConnection;

class Acceptor: public noncopyable {
public:
    typedef std::shared_ptr<Channel> Channelptr;
    typedef std::shared_ptr<TcpConnection> TcpConnectionptr;
    typedef std::function<void (int fd,const InetAddress &)> NewConnectionCallback;

    explicit Acceptor(const InetAddress &,EventLoop * loop,NewConnectionCallback);
     Acceptor(const InetAddress &,EventLoop * loop);
     ~Acceptor();
    void setNewConnectionCallback(NewConnectionCallback cb){newConnectionCallback=cb;}
    void listen();

private:
    NewConnectionCallback newConnectionCallback;
    void handleReadEvent();
    InetAddress localAddr_;
    EventLoop* loop_;
    Socket listenfd_;
    Channelptr channel_;


};


#endif //MUDUO_STUDY_ACCEPTOR_H
