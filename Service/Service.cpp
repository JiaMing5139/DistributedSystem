//
// Created by parallels on 6/23/20.
//

#include "Service.h"

#include "Util.h"
Service::Service(EventLoop *eventLoop, const InetAddress &addr, const std::vector<InetAddress> &clientAddrs) :
        baseloop_(eventLoop),
        clients_(clientAddrs),
        rpcServer_(eventLoop,addr)
        {

    EventLoop *RaftLoop = eventLoopThread_.startLoop();
    auto local = clientAddrs.front();
    std::vector<InetAddress> nodes(clientAddrs.begin()+1,clientAddrs.end());
    raft_ = new Raft(RaftLoop, local, nodes, this,"snapShot");


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
    raft_->appendLog(operation, command,id);
}


bool Service::isLeader() {
    return false;
}

Raft::status Service::getState() {
    return Raft::kClosed;
}


void Service::applyCommand(int64_t id, bool commandVaild, const std::string& log) {

    auto it = waitngResponse_.find(id);
    if (it != waitngResponse_.end()) {
        auto out = it->second;
        waitngResponse_.erase(it);
        auto *response = out.response;

        auto opt =  Utils::splitString(log);
        assert(opt.size() == 3);
        if(commandVaild){ //raft apply success
            if(opt[0] =="set"){
                data_[atoi(opt[1].c_str())] =  atoi(opt[2].c_str());
            }
        }

        auto *kvReponse = static_cast< kvService::kvReponse * >(response);
        kvReponse->set_operation(opt[0]);
        kvReponse->set_success(true);
        kvReponse->set_id(id);

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
    int key = atoi(kvRequest->key().c_str());
    int value =  atoi(kvRequest->value().c_str());
    if(raft_->getStatus() ==Raft::kLeader){

        if(kvRequest->operation() == "get"){ //FIXME for dirty data 1. check Leader itself in raft and get data in eventloop
            if(raft_->getStatus()== Raft::kLeader)
                kvReponse->set_id(kvRequest->id());
            kvReponse->set_success(true);
            kvReponse->set_operation("get");
            auto it  = data_.find(key);
            if(it!=data_.end()){
                int v = it->second;
                kvReponse->set_value(v);
            }
            done->Run();
        }else{
            int commandId_ = kvRequest->id();
            auto it = waitngResponse_.find(commandId_);
            LOG_INFO << "-================kvrequest:================";
            LOG_INFO <<"id:" << kvRequest->id();
            if (it != waitngResponse_.end()) {   // already in

            } else {
                std::string command = kvRequest->key() + " " + kvRequest->value();
                appendLog( kvRequest->operation(),command, commandId_);
                waitngResponse waitngResponse = {response, done};
                waitngResponse_[commandId_] = waitngResponse;
                kvReponse->set_id(commandId_);
            }
        }



    }else{
        kvReponse->set_id(kvRequest->id());
        kvReponse->set_success(false);
        auto Leaderaddr  = raft_->getLeader();
        std::ostringstream os;
        os << Leaderaddr;
        kvReponse->set_leader(os.str());

        done->Run();
    }





}

