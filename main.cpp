#include <iostream>
#include "log/logger.h"
#include "EventLoop.h"
#include <string>
#include <InetAddress.h>
#include <TcpServer.h>
#include "Rpc/RpcServer.h"

#include "Raft/Raft.h"
void testEchoserver() {
    EventLoop *loop= new EventLoop;

    InetAddress addr(2333);
    std::cout << addr << std::endl;
    TcpServer tcpServer(addr, loop);
    tcpServer.start();
    tcpServer.setOnMessageCallback([](Buffer *buf, TcpServer::TcpConnectionptr conn) {
        std::string s= buf->retrieveAllAsString();
        for(int i =0 ;i <1000 ;i++){
            conn->send(s);
        }
    });
    tcpServer.setOnConnectionCallback([](TcpServer::TcpConnectionptr conn) {

    });
    loop->loop();
}

void testRaft() {

}

int main(int argc,char ** argv) {
    if (argc<2) {
        perror("local port peer port1 port2 port3..");
        return 0;
    }
    Jimmy::Logger::setLevel(Jimmy::Logger::INFO);
    int16_t port = atoi( argv[1]);
    EventLoop *loop= new EventLoop;
    InetAddress localaddr(port);
    std::vector<InetAddress> points;
    for(int i = 2 ; i < argc ; i ++){

      points.push_back(std::move(InetAddress(atoi( argv[i]))));
    }

    Raft rpcServer(loop,localaddr,points);
    rpcServer.start();

    loop->loop();
    return 0;
}