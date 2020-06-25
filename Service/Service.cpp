//
// Created by parallels on 6/23/20.
//

#include "Service.h"

Service::Service(EventLoop *eventLoop, const InetAddress &addr, const std::vector<InetAddress> &clientAddrs) :
        baseloop_(eventLoop),
        clients_(clientAddrs),
        rpcServer_(eventLoop,addr)
        {

    EventLoop *RaftLoop = eventLoopThread_.startLoop();
    auto local = clientAddrs.front();
    std::vector<InetAddress> nodes(clientAddrs.begin()+1,clientAddrs.end());
    raft_ = new Raft(RaftLoop, local, nodes, this);


    kvService1_.setonKvCommandMessge(std::bind(&Service::onClientMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4));
    rpcServer_.registerService(&kvService1_);
    status_ = kClosed;

}

void Service::start() {
    if (status_ == kClosed) {
    raft_->start();
    rpcServer_.start();
    }

}

void Service::appendLog(const std::string &operation, const std::string &command, int64_t id) {
    raft_->appendLog(operation, command);
}


bool Service::isLeader() {
    return false;
}

Raft::status Service::getState() {
    return Raft::kClosed;
}


void Service::applyCommand(int64_t id, bool commandVaild, const std::string& operation, const std::string& commandName) {

    auto it = waitngResponse_.find(id);
    if (it != waitngResponse_.end()) {
        auto out = it->second;
        waitngResponse_.erase(it);
        auto *response = out.response;
        // set true or false
        // response
        out.done->Run();
    }

}

void Service::onClientMessage(google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                              ::google::protobuf::Message *response, ::google::protobuf::Closure *done) {
    //get request command
    auto *kvReponse = static_cast< kvService::kvReponse * >(response);
    auto *request_tmp = const_cast< ::google::protobuf::Message *>(request);
    auto *kvRequest = static_cast< kvService::kvRequest * >(request_tmp);
    //get a id from
    //commandId is for syns the client and service
    //commind = reequest.Id

    LOG_INFO << kvRequest->operation();
    LOG_INFO << kvRequest->key();
    LOG_INFO << kvRequest->value();

    if(raft_->getStatus() ==Raft::kLeader){

        int commandId_ = 0;
        auto it = waitngResponse_.find(commandId_);
        if (it != waitngResponse_.end()) {   // already in

        } else {
            appendLog( kvRequest->operation(), kvRequest->key(), commandId_);
            waitngResponse waitngResponse = {response, done};
            waitngResponse_[commandId_] = waitngResponse;
        }

    }else{
        kvReponse->set_success(false);
        raft_->getLeader();
        std::ostringstream os;
        os << raft_->getLeader();
        kvReponse->set_leader(os.str());
        done->Run();
    }





}

