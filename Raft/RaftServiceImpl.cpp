//
// Created by parallels on 6/15/20.
//

#include "RaftServiceImpl.h"
#include "log/logger.h"
void RaftServiceImpl::AppendEntries(google::protobuf::RpcController *controller,
                                    const ::rpcService::AppendEntriesRequest *request,
                                    ::rpcService::AppendEntriesResponse *response, ::google::protobuf::Closure *done) {
    assert(onAppendEntriesCallback);
    onAppendEntriesCallback(controller,request,response,done);
}

void RaftServiceImpl::Vote(google::protobuf::RpcController *controller, const ::rpcService::RequestVoteRequest *request,
                           ::rpcService::RequestVoteResponse *response, ::google::protobuf::Closure *done) {
    assert(onVoteRequestCallback);
    onVoteRequestCallback(controller,request,response,done);
}
