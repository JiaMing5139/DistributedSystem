//
// Created by parallels on 6/15/20.
//

#ifndef DISTRIBUTED_LAB_RAFT_H
#define DISTRIBUTED_LAB_RAFT_H


#include <zconf.h>
#include <string>
#include <vector>
#include <atomic>
#include "Rpc/RpcServer.h"
#include "RaftServiceImpl.h"
#include "log/logger.h"
#include "TcpClient.h"
#include "Timer/TimerId.h"

class RpcClient : public noncopyable {
public:
    typedef std::function<void(google::protobuf::Message *response)> RpcResponseCallback;

    RpcClient(EventLoop *loop, const InetAddress &serverAddr)
            : loop_(loop),
              client_(loop, serverAddr),
              channel_(new RpcChannel),
              RaftServiceStub_(channel_.get()),
              addr(serverAddr) {
        std::ostringstream os;
        os << addr;
        peerId_ = os.str();
    }


    void AppendEntry(rpcService::AppendEntriesRequest request, RpcResponseCallback cb) {
        LOG_INFO << "prepare to  AppendEntry ";
        responseCallback_ = cb;
        client_.setOnConnectionCallback(std::bind(&RpcClient::onSendAppendEntry, this, request, std::placeholders::_1));
        client_.setOnMessageCallback(
                std::bind(&RpcChannel::onMessage, channel_.get(), std::placeholders::_1, std::placeholders::_2));
        client_.start();
    }


    void requestVote(rpcService::RequestVoteRequest request, RpcResponseCallback cb) {
        LOG_INFO << "prepare to  requestVote ";
        responseCallback_ = cb;
        client_.setOnConnectionCallback(std::bind(&RpcClient::onSendRquestVote, this, request, std::placeholders::_1));
       // client_.setOnConnectionCallback(std::bind(&RpcClient::testonConnection, this, std::placeholders::_1));
        client_.setOnMessageCallback(
                std::bind(&RpcChannel::onMessage, channel_.get(), std::placeholders::_1, std::placeholders::_2));
        client_.start();
    }

    bool voteGrante() {
        return voteGranted_;
    }

    void setvoteGranted(bool flag) {
        voteGranted_ = flag;
    }

    void setNextInedex(int64_t index) {
        nextIndex_ = index;
    }

    void setMatchIndex(int64_t index) {
        matchIndex_ = index;
    }

    std::string clientName() {
        return peerId_;
    }

    int64_t nextIndex() {
        return nextIndex_;
    }

    int64_t matchIndex() {
        return matchIndex_;
    }


private:
    void testonConnection(const TcpConnectionPtr &conn) {
        LOG_INFO <<"conn use count:" << conn.use_count();
        assert(conn.use_count());
        LOG_INFO << conn->localAddr() <<":" << conn->peerAddr();
     }

    void onSendAppendEntry(rpcService::AppendEntriesRequest request, const TcpConnectionPtr &conn) {
        assert(conn);
        LOG_INFO << conn->localAddr() << "->" << conn->peerAddr() << "connection , start to send AppendEntriesRequest";
        channel_->setConnection(const_cast<TcpConnectionPtr &>(conn));
        rpcService::AppendEntriesResponse *response = new rpcService::AppendEntriesResponse;
        google::protobuf::Message *response_t = response;
        RaftServiceStub_.AppendEntries(NULL, &request, response, NewCallback(this, &RpcClient::solved, response_t));
    }

    void onSendRquestVote(rpcService::RequestVoteRequest request, const TcpConnectionPtr &conn) {
        assert(conn);
        channel_->setConnection(const_cast<TcpConnectionPtr &>(conn));
        LOG_INFO << conn->localAddr() << "->" << conn->peerAddr() << "connection , start to send RequestVoteRequest";
        rpcService::RequestVoteResponse *response = new rpcService::RequestVoteResponse;
        google::protobuf::Message *response_t = response;
        RaftServiceStub_.Vote(NULL, &request, response, NewCallback(this, &RpcClient::solved, response_t));
    }


    void solved(google::protobuf::Message *response) {
        responseCallback_(response);
        delete response;
        client_.disconnect();
    }


    int64_t nextIndex_ = 1;
    int64_t matchIndex_ = 0;
    bool voteGranted_ = false;
    std::string peerId_;


    RpcResponseCallback responseCallback_;
    InetAddress addr;
    EventLoop *loop_;
    TcpClient client_;
    RpcChannel::RpcChannelPtr channel_;
    rpcService::RaftService::Stub RaftServiceStub_;
    google::protobuf::Service *service_;


};

class Raft {
public:
    typedef std::shared_ptr<RpcClient> RpcClientPtr;
    enum status {
        kClosed,
        kFollower,
        kCandidate,
        kLeader
    };

    Raft(EventLoop *eventLoop, const InetAddress &addr);

    Raft(EventLoop *eventLoop, const InetAddress &addr, const std::vector<InetAddress> &clientAddrs);

    void debugRaft();

    void start();

private:
    void onAppendEntryMessage(google::protobuf::RpcController *controller,
                              const ::google::protobuf::Message *request,
                              ::google::protobuf::Message *response,
                              ::google::protobuf::Closure *done);

    void onRequestVoteEntryMessage(google::protobuf::RpcController *controller,
                                   const ::google::protobuf::Message *request,
                                   ::google::protobuf::Message *response,
                                   ::google::protobuf::Closure *done);

    void electionTimeout();
    void AppendEntryTimeout();

    void heartBeat();


    int64_t currentTerm = 1;
    std::string votedFor;
    std::vector<rpcService::AppendEntriesRequest_LogEntry> logs_;

    int64_t getVoteNum = 0;//used in follow

    int64_t commitIndex = 0;
    int64_t lastApplied = 0;

    std::vector<RpcClientPtr> clients_;
    RaftServiceImpl raftServiceImpl;
    InetAddress addr_;
    EventLoop *loop_;
    RpcServer server_;
    std::atomic<status> status_;
    std::string raftName_;


    TimerId electionTimer;
    TimerId appendEntryTimer;


};


#endif //DISTRIBUTED_LAB_RAFT_H
