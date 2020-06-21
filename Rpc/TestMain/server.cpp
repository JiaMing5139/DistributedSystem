//
// Created by parallels on 6/11/20.
//
#include "Rpc/RpcChannel.h"
#include "Rpc/message.pb.h"
#include "TcpServer.h"
#include <functional>
#include "log/logger.h"
typedef std::function<void (google::protobuf::RpcController *controller,
                            const ::rpcService::AppendEntriesRequest *request,
                            ::rpcService::AppendEntriesResponse *response,
                            ::google::protobuf::Closure *done)> onServiceCallback;

class AppendEntryServiceImpl : public rpcService::AppendEntriesService {
    void AppendEntries(google::protobuf::RpcController *controller,
                       const ::rpcService::AppendEntriesRequest *request,
                       ::rpcService::AppendEntriesResponse *response,
                       ::google::protobuf::Closure *done) override {
            //cb_(controller,request,response,done);
            LOG_TRACE<<"AppendEntryServiceImpl";
            response->set_term(1);
            done->Run();
       }

       void setAppendEntryServiceImpl(onServiceCallback cb){

    };

private:
    onServiceCallback cb_;

};

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
int main(){
    EventLoop loop;
    InetAddress inetAddress(2333);
    RpcServer prc(&loop,inetAddress);
    AppendEntryServiceImpl appendEntryService;
    prc.registerService(&appendEntryService);
    prc.start();
    loop.loop();

}