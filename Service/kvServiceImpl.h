//
// Created by parallels on 6/24/20.
//

#ifndef DISTRIBUTED_LAB_KVSERVICEIMPL_H
#define DISTRIBUTED_LAB_KVSERVICEIMPL_H

#include "serviceMessage.pb.h"
#include <functional>
class kvServiceImpl: public  kvService::kvService{
public:
    typedef std::function<void (google::protobuf::RpcController* controller,
                                const ::google::protobuf::Message* request,
                                ::google::protobuf::Message* response,
                                ::google::protobuf::Closure* done)>  RpcMessageCallback;


    void setonKvCommandMessge(const RpcMessageCallback& cb){

        onKvCommand_ = cb;
    }
private:
    void kvCommand(google::protobuf::RpcController* controller,
                   const ::kvService::kvRequest* request,
                   ::kvService::kvReponse* response,
                   ::google::protobuf::Closure* done) override {
        assert(onKvCommand_);
        onKvCommand_(controller,request,response,done);
    }
    RpcMessageCallback onKvCommand_;

};


#endif //DISTRIBUTED_LAB_KVSERVICEIMPL_H
