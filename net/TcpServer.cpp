//
// Created by parallels on 5/13/20.
//

#include "TcpServer.h"
#include "log/logger.h"
#include "TcpConnection.h"
#include <assert.h>
#include <signal.h>
#include "EventLoop.h"
namespace {
    void handleSigpiete(int parm){
        LOG_TRACE<<"SigPipe:"<<parm ;
    }
    class IgnoreSigPipe {
    public:
            IgnoreSigPipe() {
            LOG_TRACE<<"Ignore SigPipe" ;
            if(signal(SIGPIPE,handleSigpiete) == SIG_ERR){
                LOG_SYSFATAL<<"signal";
            }
        }
    };

    IgnoreSigPipe initObj;
}
TcpServer::TcpServer(const InetAddress &addr,EventLoop * loop):
acceptor_(addr,loop),
loop_(loop),
localaddr_(addr),
eventLoopThreadPool_(loop,0)
{

}



void TcpServer::newConnectionCallback( int fd, const InetAddress & peerAddress) {
    EventLoop * subloop = eventLoopThreadPool_.getLoop();
    TcpConnectionptr tcpConnectionptr(new TcpConnection(subloop,fd),[](TcpConnection * conn){
        LOG_TRACE<< "Deleter of Tcpconnection";
        delete conn;
    });
    tcpConnectionptr->setInetAddress(peerAddress,localaddr_);

    LOG_TRACE<<"Tcpserver: NewConnectionacallback:" <<tcpConnectionptr->peerAddr()<<"->"
             << tcpConnectionptr->localAddr() << " assigned fd:" << fd ;
    tcpConnectionptr->setOnMessageCallback(onMessage_);
    tcpConnectionptr->setOnConnectionCallback(onConnection_);
    tcpConnectionptr->setOnClosedCallback(bind(&TcpServer::removeTcpConnection,this,std::placeholders::_1));
    tcpConnectionptr->buildConnection(subloop);
    tcpConnectionMap_[fd] = std::move(tcpConnectionptr);
 //   LOG_TRACE<<"refcout of connection" <<tcpConnectionMap_[fd].use_count();

}

void TcpServer::removeTcpConnection(const TcpServer::TcpConnectionptr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeInLoop,this,conn));
    //TcpConnection dtor here
}

void TcpServer::removeInLoop(const TcpConnectionptr & conn) {
    loop_->assertInLoopThread();
   // LOG_TRACE <<"try to remove TcpConnection from TcpServer current :refcount of conn is " << conn.use_count();
    int ret = tcpConnectionMap_.erase(conn->fd());
    assert(ret != 0);

}

void TcpServer::start() {
    acceptor_.setNewConnectionCallback(std::bind(&TcpServer::newConnectionCallback,this,std::placeholders::_1,std::placeholders::_2));
    loop_ ->runInLoop(std::bind(&Acceptor::listen,&acceptor_));
    eventLoopThreadPool_.start();
}


