//
// Created by parallels on 5/13/20.
//

#include "Acceptor.h"

#include <utility>
#include <unistd.h>
#include "SocketsOps.h"
#include "Channel.h"
#include "TcpConnection.h"
#include "log/logger.h"

Acceptor::Acceptor(const InetAddress & addr,EventLoop * loop,NewConnectionCallback cb):
loop_(loop),
listenfd_(sockets::createNonblockingOrDie(AF_INET)),
channel_(new Channel(loop_,listenfd_.fd())),
newConnectionCallback(std::move(cb)),
localAddr_(addr)
{
    LOG_TRACE <<"Accepotr created listenfd:" << channel_->fd() ;
    listenfd_.setReuseAddr(true);
    listenfd_.setReusePort(true);
    listenfd_.bindAddress(addr);

}

Acceptor::Acceptor(const InetAddress &addr,EventLoop * loop):
        loop_(loop),
        listenfd_(sockets::createNonblockingOrDie(AF_INET)),
        channel_(new Channel(loop_,listenfd_.fd())),
        localAddr_(addr)
{
    LOG_TRACE <<"Accepotr created listenfd:" << channel_->fd() ;
    listenfd_.setReuseAddr(true);
    listenfd_.setReusePort(true);
    listenfd_.bindAddress(addr);

}



void Acceptor::handleReadEvent() {
    InetAddress peerAddr;
    int fd = listenfd_.accept(&peerAddr);
    if(newConnectionCallback){
        newConnectionCallback(fd,peerAddr);
    }

}

void Acceptor::listen() {
    printf("acceptor listen\n");
    listenfd_.listen();
    channel_->setReadCallBack(std::bind(&Acceptor::handleReadEvent,this));
    channel_->enableRead();
}

Acceptor::~Acceptor() {
    LOG_TRACE <<"dtor of Accepotr fd:" << channel_->fd() ;
}
