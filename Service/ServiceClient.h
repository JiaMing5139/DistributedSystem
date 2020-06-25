//
// Created by parallels on 6/24/20.
//

#ifndef DISTRIBUTED_LAB_SERVICECLIENT_H
#define DISTRIBUTED_LAB_SERVICECLIENT_H

#include "net/Channel.h"
#include "log/logger.h"
#include "serviceMessage.pb.h"
#include "Rpc/RpcChannel.h"
#include <utility>
#include <vector>
#include "net/InetAddress.h"
#include <atomic>
#include <unistd.h>
#include "net/TcpClient.h"
#include "Util.cpp"
#include "EventLoop.h"
class ServiceClient {
public:
    typedef std::function<void(google::protobuf::Message *response)> RpcResponseCallback;
    ServiceClient(EventLoop* loop,std::vector<InetAddress>  addrs):
    rpcChannelPtr_(new RpcChannel),
    kvServiceStub_(rpcChannelPtr_.get()),
    loop_(loop),
    serverAddrs_(std::move(addrs)),
    leaderAddr_(serverAddrs_.back()),
    client_(loop,leaderAddr_),
    inputChannel_(new Channel(loop,STDIN_FILENO)),
    msgId_(0)
    {

    }
    void SendRquest(const kvService::kvRequest& request,RpcResponseCallback cb){
        client_.setServerAdder(leaderAddr_);
        responseCallback_ = std::move(cb);
        client_.setOnConnectionCallback(std::bind(&ServiceClient::onConnectionSendRpc,this,request,std::placeholders::_1));
        client_.setOnMessageCallback(std::bind(&RpcChannel::onMessage,rpcChannelPtr_.get(),std::placeholders::_1,std::placeholders::_2));
        client_.start();

    }

    void start(){
        inputChannel_->setReadCallBack(std::bind(&ServiceClient::onInputFromCTerminal,this));
        inputChannel_->enableRead();
    }

    void onConnectionSendRpc(const kvService::kvRequest& request,const TcpConnectionPtr &conn){
        auto *response = new kvService::kvReponse;
        rpcChannelPtr_->setConnection(const_cast<TcpConnectionPtr &>(conn));
        google::protobuf::Message *response_t = response;
        kvServiceStub_.kvCommand(nullptr,&request,response,NewCallback(this, &ServiceClient::solved, response_t));
    }


private:

    void onInputFromCTerminal(){
        char buff[2048] ={0};
        kvService::kvRequest request;
        int nread = read(STDIN_FILENO,buff,sizeof buff);
        if(nread < 0){
            LOG_SYSFATAL   << "read";
        }

        auto commands = Utils::splitString(buff,strlen(buff)+1);
        msgId_ ++ ;
        if(commands[0] =="get" ) {
            request.set_operation(commands[0]);
            request.set_key(commands[1]);

        }else if(commands[0] =="set"){
            request.set_operation(commands[0]);
            request.set_key(commands[1]);
            request.set_operation(commands[0]);
            request.set_value(commands[2]);
        }else{
            std::cout << "opeartion should be get or set" << "\n";
            return ;
        }

        auto retry = loop_->runEvery(5,[&](){
            client_.resetConnection();
           // client_.start();
        });
        LOG_INFO << "request:" << request.operation() << " " << request.key() << " " << request.value() ;
        SendRquest(request,[&](google::protobuf::Message *response){
            auto *kvReponse = static_cast<kvService::kvReponse *>(response);
            if(kvReponse->success()){
                //apply it to memory
                LOG_INFO<<" Leader:" << kvReponse->leader();
            }else{
                LOG_INFO<<" Leader:" << kvReponse->leader();
            }
        });



    }

    void solved(google::protobuf::Message *response) {
        responseCallback_(response);
        delete response;
        client_.disconnect();
    }


    RpcResponseCallback responseCallback_;

    std::vector<InetAddress> serverAddrs_;
    std::atomic<int64_t> msgId_;
    std::shared_ptr<Channel> inputChannel_;
    std::shared_ptr<RpcChannel> rpcChannelPtr_;
    InetAddress leaderAddr_;
    kvService::kvService_Stub kvServiceStub_;
    EventLoop * loop_;
    TcpClient client_;
};



#endif //DISTRIBUTED_LAB_SERVICECLIENT_H
