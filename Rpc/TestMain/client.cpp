
//Created by parallels on 6/11/20.

#include "TcpClient.h"
#include "Rpc/message.pb.h"
#include "Rpc/RpcChannel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <google/protobuf/service.h>
#include "log/logger.h"

class RpcClient{
public:
    RpcClient(EventLoop* loop, const InetAddress& serverAddr)
            : loop_(loop),
              client_(loop, serverAddr),
              channel_(new RpcChannel),
              AppendEntriesServiceStub_(channel_.get())
    {

    }


    void AppendEntry(){
        client_.setOnConnectionCallback(std::bind(&RpcClient::onConnection,this,std::placeholders::_1));
        client_.setOnMessageCallback(std::bind(&RpcChannel::onMessage,channel_.get(),std::placeholders::_1,std::placeholders::_2));
        client_.start();
    }

    void requestVote(){

    }



private:
    void onConnection(const TcpConnectionPtr& conn)
    {
            channel_->setConnection(const_cast<TcpConnectionPtr &>(conn));
            rpcService::AppendEntriesRequest request;
            rpcService::AppendEntriesResponse *response = new  rpcService::AppendEntriesResponse;
            AppendEntriesServiceStub_.AppendEntries(NULL, &request, response, NewCallback(this, &RpcClient::solved, response));
    }


    void solved( rpcService::AppendEntriesResponse *response)
    {
        LOG_TRACE<<"receive AppendEntriesResponse term:" << response->term();
        delete response;
        client_.disconnect();
    }



    EventLoop* loop_;
    TcpClient client_;
    RpcChannel::RpcChannelPtr channel_;
    rpcService::AppendEntriesService::Stub AppendEntriesServiceStub_;

};


int main(){
    EventLoop loop;
    InetAddress inetAddress(2333);
    RpcClient client(&loop,inetAddress);
    client.AppendEntry();
    loop.loop();
}