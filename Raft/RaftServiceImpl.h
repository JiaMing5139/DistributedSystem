//
// Created by parallels on 6/15/20.
//

#ifndef DISTRIBUTED_LAB_RAFTSERVICEIMPL_H
#define DISTRIBUTED_LAB_RAFTSERVICEIMPL_H

#include "Raft.pb.h"
#include <google/protobuf/service.h>

#include <utility>
class RaftServiceImpl: public rpcService::RaftService {
public:
     typedef std::function<void (google::protobuf::RpcController* controller,
                                 const ::google::protobuf::Message* request,
                                 ::google::protobuf::Message* response,
                                 ::google::protobuf::Closure* done)>  RpcMessageCallback;

     void AppendEntries(google::protobuf::RpcController* controller,
                               const ::rpcService::AppendEntriesRequest* request,
                               ::rpcService::AppendEntriesResponse* response,
                               ::google::protobuf::Closure* done) override ;

     void Vote(google::protobuf::RpcController* controller,
                      const ::rpcService::RequestVoteRequest* request,
                      ::rpcService::RequestVoteResponse* response,
                      ::google::protobuf::Closure* done) override;

      void setOnAppendEntriesCallback(RpcMessageCallback cb) {onAppendEntriesCallback = std::move(cb);}
      void setoOVoteRequestCallback(RpcMessageCallback cb){onVoteRequestCallback = std::move(cb);}

private:
    RpcMessageCallback onAppendEntriesCallback;
    RpcMessageCallback onVoteRequestCallback;


};


#endif //DISTRIBUTED_LAB_RAFTSERVICEIMPL_H
