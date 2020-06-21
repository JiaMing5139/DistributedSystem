//
// Created by parallels on 5/25/20.
//

#include "Connector.h"
#include "Socket.h"
#include "SocketsOps.h"
#include "log/logger.h"
#include "Channel.h"
Connector::Connector(EventLoop *loop, const InetAddress &serverAddr):
loop_(loop),
serverAddr_(serverAddr)
{

}

Connector::~Connector() {

}

void Connector::start() {
    connect();
}

void Connector::restart() {

}

void Connector::stop() {

}

void Connector::connect() {
    int fd = sockets::createblockingOrDie(AF_INET);
    int ret = sockets::connect(fd, serverAddr_.getSockaddr());
    if (ret < 0 && errno != EINTR ) {
        LOG_TRACE << "connect failed: " <<  serverAddr_;
    }else{
        newConnectionCallback_(fd);
    }

}


void Connector::handleWrite() {

}
