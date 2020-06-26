//
// Created by parallels on 6/9/20.
//

#include "RpcChannel.h"
#include "rpc.pb.h"
#include "TcpConnection.h"
#include "log/logger.h"

void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method,
                            ::google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                            ::google::protobuf::Message *response, ::google::protobuf::Closure *done) {

    Rpc::RpcMessage rpcMessage;
    rpcMessage.set_type(Rpc::REQUEST);
    rpcMessage.set_id(incrementId_);
    rpcMessage.set_service(method->service()->full_name());
    rpcMessage.set_method(method->name());
    rpcMessage.set_request(request->SerializeAsString());
    waitngResponse out = {response, done};

    waitingResponseMap_[incrementId_] = out;
    incrementId_++;
    //serializeAsString all
    char message[65535];
    int len;
    codec_.FillBuffer(rpcMessage, message, &len);
    std::string msg(message, len + 4); //FIXME

    auto conn = conn_.lock();
    if (conn)
        conn->send(msg);
    //send;
}

void RpcChannel::onRpcMessage(Codec::MessagePtr message, std::shared_ptr<TcpConnection> connptr) {
   // assert(connptr == conn_);
    std::shared_ptr<Rpc::RpcMessage> rpcmessage = std::static_pointer_cast<Rpc::RpcMessage>(message);
    if (rpcmessage->type() == Rpc::RESPONSE) {
        int64_t id = rpcmessage->id();
        waitngResponse out = {NULL, NULL};

        auto it = waitingResponseMap_.find(id);
        if (it != waitingResponseMap_.end()) {
            out = it->second;
            waitingResponseMap_.erase(it);
        }

        if (out.response) {
            if (rpcmessage->has_response()) {
                out.response->ParseFromString(rpcmessage->response());
            }
            if (out.done) {
                out.done->Run();
            }
        }
    } else if (rpcmessage->type() == Rpc::REQUEST) {

//        LOG_TRACE << "get a request: " << rpcmessage->descriptor()->full_name();
//        LOG_TRACE << "get a request: " << rpcmessage->method();
//        LOG_TRACE << "get a request: " << rpcmessage->service();
        if (services_) {

            auto target_service = services_->find(rpcmessage->service());
            if (target_service != services_->end()) {
                google::protobuf::Service *service = target_service->second;
                const google::protobuf::ServiceDescriptor *desc = service->GetDescriptor();
//                LOG_TRACE<<"method :"<<rpcmessage->method();
                const google::protobuf::MethodDescriptor *method
                        = desc->FindMethodByName(rpcmessage->method());
                if (method) {
                    std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
                    if(request->ParseFromString(rpcmessage->request())){
                        google::protobuf::Message* response = service->GetResponsePrototype(method).New();
                        int64_t id = rpcmessage->id();
                        service->CallMethod(method, nullptr,request.get(),response, NewCallback(this, &RpcChannel::doneService, response, id));
                    } else{

                    }

                }

            } else {

                // not found service
            }

        } else {

        }


    }


}


void RpcChannel::onMessage(Buffer *buffer, TcpConnectionPtr conn) {
    codec_.onMessage(buffer, conn);
}

RpcChannel::RpcChannel() {
    codec_.setOnMessageCompleteCallback(
            bind(&RpcChannel::onRpcMessage, this, std::placeholders::_1, std::placeholders::_2));
}

RpcChannel::RpcChannel(const TcpConnectionPtr &conn) : conn_(conn) {
    codec_.setOnMessageCompleteCallback(
            bind(&RpcChannel::onRpcMessage, this, std::placeholders::_1, std::placeholders::_2));
}

RpcChannel::~RpcChannel() {

}

void RpcChannel::doneService(::google::protobuf::Message *response, int64_t id) {
    std::unique_ptr<google::protobuf::Message> d(response);
    Rpc::RpcMessage rpcMessage;
    rpcMessage.set_type(Rpc::RESPONSE);
    rpcMessage.set_id(id);
    rpcMessage.set_response(response->SerializeAsString()); // FIXME: error check
    //delete response;
    char message[65535];
    int len;
    codec_.FillBuffer(rpcMessage, message, &len);
    std::string msg(message, len + 4); //FIXME


    auto conn = conn_.lock();
    if (conn)
        conn->send(msg);
}
