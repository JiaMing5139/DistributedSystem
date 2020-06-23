//
// Created by parallels on 6/23/20.
//

#ifndef DISTRIBUTED_LAB_SERVICE_H
#define DISTRIBUTED_LAB_SERVICE_H

#include "EventLoopThread.h"
#include <google/protobuf/message.h>
#include <map>
#include <Raft/Raft.h>

class EventLoop;
class Service {
public:
    enum   status{
        kClosed,
        kRunning
    };
    Service(EventLoop *eventLoop, const InetAddress &addr, const std::vector<InetAddress> &clientAddrs);
    void start();

    //start to append a log to raft
    void appendLog(const std::string& operation,const std::string& command,int64_t id);

    //is a Leader or not
    bool isLeader();

    //get state of raft
    Raft::status getState();

    // applyaLog to Service
    void applyCommand(int64_t id,bool commandVaild,std::string operation,std::string commandName);

private:
    struct waitngResponse{
        google::protobuf::Message* response;
        ::google::protobuf::Closure* done;
    };

    void onClientMessage(google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                          ::google::protobuf::Message *response, ::google::protobuf::Closure *done);

    status  status_;
    std::map<int,int> map_;
    EventLoopThread eventLoopThread_;
    EventLoop * baseloop_;
    Raft * raft_;
    std::vector<InetAddress> clients_;
    TcpServer tcpserver_;

    std::map<int64_t , waitngResponse > waitngResponse_;


};


#endif //DISTRIBUTED_LAB_SERVICE_H
