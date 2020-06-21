//
// Created by parallels on 6/9/20.
//

#ifndef PRCPROTO_RPCCHANNEL_H
#define PRCPROTO_RPCCHANNEL_H

#include <atomic>
#include <google/protobuf/service.h>
#include <map>
#include "Codec.h"

class TcpConnection;
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
class RpcChannel: public google::protobuf::RpcChannel {
public:
    typedef std::shared_ptr<RpcChannel> RpcChannelPtr;

    RpcChannel();
    explicit  RpcChannel(const TcpConnectionPtr & );
    ~RpcChannel() override;

    void setServices(  std::map<std::string, ::google::protobuf::Service*>* services){
        services_ = services;
    }
    void setConnection( TcpConnectionPtr&conn)
    {
        this->conn_= conn;
    }
    void CallMethod(const ::google::protobuf::MethodDescriptor* method,
              ::google::protobuf::RpcController* controller,
              const ::google::protobuf::Message* request,
              ::google::protobuf::Message* response,
              ::google::protobuf::Closure* done) override;

    void onMessage(Buffer *,TcpConnectionPtr conn);
    void doneService(::google::protobuf::Message* response, int64_t id);

private:
    void onRpcMessage(Codec::MessagePtr message, std::shared_ptr<TcpConnection> connptr);
    struct waitngResponse{
        google::protobuf::Message* response;
        ::google::protobuf::Closure* done;
    };
    Codec codec_;
    std::map<std::string, ::google::protobuf::Service*>* services_;
    std::map<int64_t, waitngResponse> waitingResponseMap_;
    std::atomic<int64_t > incrementId_;
    std::weak_ptr<TcpConnection> conn_;
};


#endif //PRCPROTO_RPCCHANNEL_H
