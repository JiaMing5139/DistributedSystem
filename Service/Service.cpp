//
// Created by parallels on 6/23/20.
//

#include "Service.h"

Service::Service(EventLoop *eventLoop, const InetAddress &addr, const std::vector<InetAddress> &clientAddrs) :
        baseloop_(eventLoop),
        clients_(clientAddrs),
        tcpserver_(addr, eventLoop) {
    EventLoop *RaftLoop = eventLoopThread_.startLoop();
    raft_ = new Raft(RaftLoop, addr, clientAddrs, this);
    status_ = kClosed;
}

void Service::start() {
    if (status_ == kClosed) {
        raft_->start();
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


void Service::applyCommand(int64_t id, bool commandVaild, std::string operation, std::string commandName) {

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

    //get a id from
    //commandId is for syns the client and service
    //commind = reequest.Id

    int commandId_ = 0;
    auto it = waitngResponse_.find(commandId_);
    if (it != waitngResponse_.end()) {   // already in
    } else {
        appendLog("get", "5", commandId_);
        waitngResponse waitngResponse = {response, done};
        waitngResponse_[commandId_] = waitngResponse;
    }

}

