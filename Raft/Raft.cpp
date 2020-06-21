//
// Created by parallels on 6/15/20.
//

#include "Raft.h"
#include "log/logger.h"
#include <sstream>

inline int randomTime(int start, int end) {
    timeval spec;
    gettimeofday(&spec, NULL);
    srand(spec.tv_usec);
    return start + rand() % (end - start);
}

Raft::Raft(EventLoop *eventLoop, const InetAddress &addr) :
        addr_(addr),
        loop_(eventLoop),
        server_(eventLoop, addr),
        status_(kClosed) {

}

Raft::Raft(EventLoop *eventLoop, const InetAddress &addr, const std::vector<InetAddress> &clientAddrs) :
        addr_(addr),
        loop_(eventLoop),
        server_(eventLoop, addr),
        status_(kClosed) {
    for (auto &addr : clientAddrs) {
        RpcClientPtr clientPtr(new RpcClient(eventLoop, addr));
        clients_.push_back(std::move(clientPtr));
    }
    std::ostringstream os;
    os << addr;
    raftName_ = os.str();

    raftServiceImpl.setOnAppendEntriesCallback(
            std::bind(&Raft::onAppendEntryMessage, this, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3, std::placeholders::_4));
    raftServiceImpl.setoOVoteRequestCallback(
            std::bind(&Raft::onRequestVoteEntryMessage, this, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3, std::placeholders::_4));
    server_.registerService(&raftServiceImpl);
    debugRaft();
}

void Raft::start() {
    if (status_ == kClosed) {
        server_.start();
        auto t = randomTime(1, 5);
        LOG_INFO << "set electionTimeout " << t;
        electionTimer = loop_->runEvery(t, std::bind(&Raft::electionTimeout, this));
        status_ = kFollower;
        debugRaft();
    }
}

void Raft::heartBeat() {


}

void Raft::electionTimeout() {
    LOG_INFO << "electionTimeout";
    rpcService::RequestVoteRequest request;
    switch (status_) {
        case kFollower: {
            for (auto &client :clients_) {
                client->setvoteGranted(false);
            }
            getVoteNum = 0;
            currentTerm++;
            this->status_ = kCandidate;
            votedFor = raftName_; //vote for itself
            getVoteNum++;
            LOG_INFO << "num of clients:" << clients_.size();
            request.set_term(currentTerm);
            request.set_candidatename(raftName_);
            for (auto &client :clients_) {
                LOG_INFO << raftName_ << "->" << client->clientName() << " send RequestVote";
                //  if (!client->voteGrante())  // correct?

                client->requestVote(request, [&](google::protobuf::Message *response) {
                    LOG_INFO << raftName_ << "<-" << client->clientName() << " get RequestVote Rsponse";
                    if (status_ == kCandidate) {
                        rpcService::RequestVoteResponse *requestVoteResponse = static_cast< rpcService::RequestVoteResponse * >(response);
                        LOG_INFO << "recv RequestVoteResponse term: " << requestVoteResponse->term();
                        LOG_INFO << "recv RequestVoteResponse votegranted: " << requestVoteResponse->votegranted();
                        if (requestVoteResponse->term() > currentTerm) {
                            LOG_INFO << "recv RequestVoteResponse: peer term:" << requestVoteResponse->term()
                                     << " current term:" << currentTerm << " So currertTerm = peerTerm";
                            currentTerm = requestVoteResponse->term();
                        }

                        if (requestVoteResponse->votegranted()) {
                            this->getVoteNum++;
                            client->setvoteGranted(true);
                            if (getVoteNum > clients_.size() / 2) {
                                this->status_ = kLeader;
                                LOG_INFO << raftName_ << " Get Leader";
                                loop_->cancleTimer(electionTimer);
                            }
                        }
                    }
                });
            }

        }

            break;
        case kCandidate: {
            for (auto &client :clients_) {
                client->setvoteGranted(false);
            }
            getVoteNum = 0;
            currentTerm++;
            this->status_ = kCandidate;
            votedFor = raftName_; //vote for itself
            getVoteNum++;
            LOG_INFO << "num of clients:" << clients_.size();
            request.set_term(currentTerm);
            request.set_candidatename(raftName_);
            for (auto &client :clients_) {
                LOG_INFO << raftName_ << "->" << client->clientName() << " send RequestVote";
                client->requestVote(request, [&](google::protobuf::Message *response) {
                    LOG_INFO << raftName_ << "<-" << client->clientName() << " get RequestVote Rsponse";
                    if (status_ == kCandidate) {
                        rpcService::RequestVoteResponse *requestVoteResponse = static_cast< rpcService::RequestVoteResponse * >(response);
                        LOG_INFO << "recv RequestVoteResponse term: " << requestVoteResponse->term();
                        LOG_INFO << "recv RequestVoteResponse votegranted: " << requestVoteResponse->votegranted();
                        if (requestVoteResponse->term() > currentTerm) {
                            LOG_INFO << "recv RequestVoteResponse: peer term:" << requestVoteResponse->term()
                                     << " current term:" << currentTerm << " So currertTerm = peerTerm";
                            currentTerm = requestVoteResponse->term();
                        }
                        if (requestVoteResponse->votegranted()) {
                            this->getVoteNum++;
                            client->setvoteGranted(true);
                            if (getVoteNum > clients_.size() / 2) {
                                this->status_ = kLeader;
                                LOG_INFO << raftName_ << " Get Leader";
                                auto t = randomTime(5, 10);
                                double delay = t/10;
                                loop_->runAfter(delay,std::bind(&Raft::appendEntryTimer,this));
                            }
                        }
                    }
                });
            }
        }
            break;


    }
    auto t = randomTime(1, 5);
    LOG_INFO << "reset electionTimeout " << t;
    loop_->cancleTimer(electionTimer);
    electionTimer = loop_->runEvery(t, std::bind(&Raft::electionTimeout, this));
    debugRaft();

}

void
Raft::onRequestVoteEntryMessage(google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                                ::google::protobuf::Message *response, ::google::protobuf::Closure *done) {
    rpcService::RequestVoteResponse *requestVoteResponse = static_cast< rpcService::RequestVoteResponse * >(response);
    ::google::protobuf::Message *request_tmp = const_cast< ::google::protobuf::Message *>(request);
    rpcService::RequestVoteRequest *requestVoteRequest = static_cast< rpcService::RequestVoteRequest * >(request_tmp);
    LOG_INFO << "recv RequestVoteRequest from" << requestVoteRequest->candidatename();
    switch (status_) {
        case kCandidate:
        case kFollower: {
            if (currentTerm > requestVoteRequest->term()) {
                LOG_INFO << "handing RequestVoteRequest"
                         << "return false  |because currentTerm >  requestVoteRequest->term()";
                requestVoteResponse->set_votegranted(false);
            } else if (currentTerm < requestVoteRequest->term()) {
                votedFor = requestVoteRequest->candidatename();
                currentTerm = requestVoteRequest->term();
                requestVoteResponse->set_votegranted(true);
                auto t = randomTime(1, 5);
                LOG_INFO << "reset electionTimeout " << t;
                loop_->cancleTimer(electionTimer);
                electionTimer = loop_->runEvery(t, std::bind(&Raft::electionTimeout, this));
                LOG_INFO << "handing RequestVoteRequest" << " return yes  | new timeout is " << t;
            } else if (votedFor.empty() or votedFor == requestVoteRequest->candidatename()) {
                votedFor = requestVoteRequest->candidatename();
                currentTerm = requestVoteRequest->term();
                requestVoteResponse->set_votegranted(true);
                auto t = randomTime(1, 5);
                LOG_INFO << "reset electionTimeout " << t;
                loop_->cancleTimer(electionTimer);
                electionTimer = loop_->runEvery(t, std::bind(&Raft::electionTimeout, this));
                LOG_INFO << "handing RequestVoteRequest" << " return yes  | not voted | new timeout is " << t;

            } else {
                LOG_INFO << "handing RequestVoteRequest" << "return false  | voted someone already";
                requestVoteResponse->set_votegranted(false);
            }
            requestVoteResponse->set_term(currentTerm);
        }
        case kLeader:{
            //status_ = kFollower;
        }
    }
    debugRaft();
    done->Run();
}

void Raft::onAppendEntryMessage(google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                                ::google::protobuf::Message *response, ::google::protobuf::Closure *done) {
    rpcService::RequestVoteResponse *requestVoteResponse = static_cast< rpcService::RequestVoteResponse * >(response);
    ::google::protobuf::Message *request_tmp = const_cast< ::google::protobuf::Message *>(request);
    rpcService::RequestVoteRequest *requestVoteRequest = static_cast< rpcService::RequestVoteRequest * >(request_tmp);



    debugRaft();
    done->Run();
}

std::string statusToString(Raft::status s) {
    switch (s) {
        case Raft::kCandidate :
            return "Candidate";
        case Raft::kLeader :
            return "Leader";
        case Raft::kFollower :
            return "Follower";
        case Raft::kClosed :
            return "Closed";
        default:
            return "failed";
    }

}

int sizeofInttoStr(int64_t index) {
    char buf[65];
    snprintf(buf, sizeof buf, "%d", index);
    return strlen(buf);
}

void Raft::debugRaft() {

    std::ostringstream debug_info;
    debug_info << "---------------------------------------------" << std::endl;
    debug_info << "State      : " << statusToString(status_) << std::endl;
    debug_info << "CurrentTerm: " << currentTerm << std::endl;
    debug_info << "CommitInedx: " << commitIndex << std::endl;
    debug_info << "votedFOr   : " << votedFor << std::endl;
    debug_info << "peer            nextIndex       matchIndex      vote Granted" << std::endl;
    for (auto &client:clients_) {
        debug_info << client->clientName();
        std::string blank(16 - client->clientName().size(), ' ');
        debug_info << blank;
        debug_info << client->nextIndex();
        blank.resize(16 - sizeofInttoStr(client->nextIndex()), ' ');
        debug_info << blank;
        debug_info << client->matchIndex();
        blank.resize(16 - sizeofInttoStr(client->matchIndex()), ' ');
        debug_info << blank;
        debug_info << client->voteGrante();
        debug_info << "\n";
    }
    std::cout << debug_info.str() << std::endl;


}

void Raft::AppendEntryTimeout() {
    for( auto & client:clients_){
        client->AppendEntry();
    }

    auto t = randomTime(5, 10);
    double delay = t/10;
    loop_->runAfter(delay,std::bind(&Raft::appendEntryTimer,this));

}




