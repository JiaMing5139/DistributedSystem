//
// Created by parallels on 6/15/20.
//

#ifndef DISTRIBUTED_LAB_RPCSERVER_H
#define DISTRIBUTED_LAB_RPCSERVER_H

#include <net/InetAddress.h>
#include <google/protobuf/service.h>
#include "TcpConnection.h"
#include "RpcChannel.h"
#include "TcpServer.h"


class EventLoop;
class RpcServer
{
public:


    RpcServer(EventLoop* loop,
              const InetAddress& listenAddr):
            server_(listenAddr,loop)
    {
    }

    void registerService(::google::protobuf::Service* service){
        const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
        services_[desc->full_name()] = service;
    }

    void onAppendEntryMessage(){

    }

    void start(){
        server_.setOnConnectionCallback(bind(&RpcServer::onConnection,this,std::placeholders::_1));
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn){
        RpcChannel::RpcChannelPtr channel(new RpcChannel(conn));
        channel->setServices(&services_);
        conn->setOnMessageCallback(
                std::bind(&RpcChannel::onMessage, channel.get(),std::placeholders::_1,std::placeholders::_2));
        conn->setRpcChannel(channel);
    }


    TcpServer server_;
    std::map<std::string, ::google::protobuf::Service*> services_;
};


#endif //DISTRIBUTED_LAB_RPCSERVER_H
