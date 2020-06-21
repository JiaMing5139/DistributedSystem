//
// Created by parallels on 5/25/20.
//

#include "TcpClient.h"
#include "log/logger.h"
#include "TcpConnection.h"
TcpClient::TcpClient(EventLoop *loop, const InetAddress &addr):
connector_(loop,addr),
baseloop_(loop),
status_(closed)
{

}
void TcpClient::disconnect(){
    status_ = closing;
    if(tcpConnectionptr_){
        tcpConnectionptr_->shutdown();
    }
}

void TcpClient::start() {
    if(status_ == closed){
        status_ = connecting;
        connector_.setNewConnectionCallback(bind(&TcpClient::newConnection,this,std::placeholders::_1));
        connector_.start();

    }

}

void TcpClient::newConnection(int fd) {
  //  LOG_TRACE << "TcpClient: newConnection status:"<< status_ ;
    if(status_ == connecting) {

        TcpConnectionptr tcpConnectionptr(new TcpConnection(baseloop_, fd), [](TcpConnection *conn) {
            LOG_TRACE << "Deleter of  Tcpconnection in Client: " << conn->localAddr() ;
            delete conn;
        });
        InetAddress localaddr;
        tcpConnectionptr->setInetAddress(connector_.serverAddress(),localaddr);

        LOG_TRACE << "Tcpclient NewConnection connected:" << tcpConnectionptr->localAddr() << "->"
                  << tcpConnectionptr->peerAddr() << " assigned fd:" << fd;
        tcpConnectionptr->setOnMessageCallback(onMessage_);
        tcpConnectionptr->setOnConnectionCallback(onConnection_);
        tcpConnectionptr->setOnClosedCallback(std::bind(&TcpClient::removeTcpConnection, this));
        tcpConnectionptr->buildConnection(baseloop_);
       // onConnection_(tcpConnectionptr_);
        tcpConnectionptr_ = tcpConnectionptr;
        status_ = connected;
       
    }
}

void TcpClient::removeTcpConnection() {
    if(status_ == closing)
    {
        LOG_TRACE << "TcpClient tcpconnection closed" ;
        tcpConnectionptr_.reset();
        status_ =closed;
    }



}

void TcpClient::send(const std::string & msg) {
    tcpConnectionptr_->send(msg);
}
