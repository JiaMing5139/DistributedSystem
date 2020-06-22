//
// Created by parallels on 6/15/20.
//

#include "Raft.h"
#include "log/logger.h"
#include <sstream>

namespace raft{
    void displayAppendEntry(const rpcService::AppendEntriesRequest &request) {

        std::ostringstream debug_info;
        debug_info << "---------------------------------------------" << std::endl;
        debug_info << "term      : " << request.term() << std::endl;
        debug_info << "prevlogindex: " << request.prevlogindex() << std::endl;
        debug_info << "prevlogterm: " << request.prevlogterm() << std::endl;
        const auto &log = request.logentries();
        if (log.empty())
            debug_info << "logentries   : " << "null" << std::endl;
        else
            debug_info << "logentries   : " << log[0].commandname() << std::endl;
        debug_info << "leadercommit: " << request.leadercommit() << std::endl;
        LOG_INFO << "AppendEntriesRequest request" << "\n" << debug_info.str();

    }
}

inline double randomTime(int start, int end) {
    timeval spec;
    gettimeofday(&spec, NULL);
    srand(spec.tv_usec);
    start *= 100;
    end *= 100;
    double tmp = start + rand() % (end - start);
    return  tmp/100;
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
    rpcService::AppendEntriesRequest_LogEntry logEntry;
    logs_.reserve(100);
    logEntry.set_term(0);
    logEntry.set_index(0);
    logs_.push_back(std::move(logEntry));
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
//        case kFollower: {
//            for (auto &client :clients_) {
//                client->setvoteGranted(false);
//            }
//            getVoteNum = 0;
//            currentTerm++;
//            this->status_ = kCandidate;
//            votedFor = raftName_; //vote for itself
//            getVoteNum++;
//            LOG_INFO << "num of clients:" << clients_.size();
//            request.set_term(currentTerm);
//            request.set_candidatename(raftName_);
//            for (auto &client :clients_) {
//                LOG_INFO << raftName_ << "->" << client->clientName() << " send RequestVote";
//                //  if (!client->voteGrante())  // correct?
//
//                client->requestVote(request, [&](google::protobuf::Message *response) {
//                    LOG_INFO << raftName_ << "<-" << client->clientName() << " get RequestVote Rsponse";
//                    if (status_ == kCandidate) {
//                        rpcService::RequestVoteResponse *requestVoteResponse = static_cast< rpcService::RequestVoteResponse * >(response);
//                        LOG_INFO << "recv RequestVoteResponse term: " << requestVoteResponse->term();
//                        LOG_INFO << "recv RequestVoteResponse votegranted: " << requestVoteResponse->votegranted();
//                        if (requestVoteResponse->term() > currentTerm) {
//                            LOG_INFO << "recv RequestVoteResponse: peer term:" << requestVoteResponse->term()
//                                     << " current term:" << currentTerm << " So currertTerm = peerTerm";
//                            currentTerm = requestVoteResponse->term();
//                        }
//
//                        if (requestVoteResponse->votegranted()) {
//                            this->getVoteNum++;
//                            client->setvoteGranted(true);
//                            if (getVoteNum > clients_.size() / 2) {
//                                this->status_ = kLeader;
//                                LOG_INFO << raftName_ << " Get Leader";
//                                for(auto & client: clients_){
//                                    client->setNextInedex(logs_.back().index() + 1);
//                                    client->setMatchIndex(0);
//                                }
//                                loop_->cancleTimer(electionTimer);
//                            }
//                        }
//                    }
//                });
//            }
//
//        }
//
//            break;
        case kFollower:
        case kCandidate: {
            for (auto &client :clients_) {
                client->setvoteGranted(false);
            }
            votedFor = "";
            getVoteNum = 0;

            currentTerm++;
            this->status_ = kCandidate;
            votedFor = raftName_; //vote for itself
            getVoteNum++;
            LOG_INFO << "getVoteNum" << getVoteNum;
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
                                for (auto &client :clients_)
                                    client->setNextInedex(logs_.back().index() + 1);
                                auto t = randomTime(5, 10);
                                double delay = t / 10;
                                loop_->runAfter(0.1, std::bind(&Raft::AppendEntryTimeout, this));
                                loop_->cancleTimer(electionTimer);
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
        case kLeader: {
            if(currentTerm < requestVoteRequest->term()) //need thinking
             status_ = kFollower;
        }
    }
    debugRaft();
    done->Run();
}

void Raft::onAppendEntryMessage(google::protobuf::RpcController *controller, const ::google::protobuf::Message *request,
                                ::google::protobuf::Message *response, ::google::protobuf::Closure *done) {

    auto *AppendEntriesResponse = static_cast< rpcService::AppendEntriesResponse * >(response);
    auto *request_tmp = const_cast< ::google::protobuf::Message *>(request);
    rpcService::AppendEntriesRequest *  appendEntriesRequest = static_cast< rpcService::AppendEntriesRequest * >(request_tmp);

    LOG_INFO << "onAppendEntryMessage:recv onAppendEntryMessage from" << appendEntriesRequest->leaderid();
   // raft::displayAppendEntry(*appendEntriesRequest);

    if (appendEntriesRequest->term() < currentTerm) {
        LOG_INFO << "onAppendEntryMessage: AppendEntriesResponse->term() < currentTerm return false peer,local Term:" <<  AppendEntriesResponse->term()<<":" << currentTerm;
        AppendEntriesResponse->set_success(false);
        AppendEntriesResponse->set_term(currentTerm);
    } else { // cuurent <= AppendEntriesResponse->term()
        currentTerm = AppendEntriesResponse->term();
        AppendEntriesResponse->set_term(currentTerm);


        loop_->cancleTimer(electionTimer);
        auto t = randomTime(1, 5);
        LOG_INFO << "onAppendEntryMessage:  reset electionTimer " << t;
        electionTimer = loop_->runEvery(t, std::bind(&Raft::electionTimeout, this));

        switch (status_){
            case kLeader:{
                 status_ = kFollower;
            }
            case kCandidate:
            case kFollower:{
                if(appendEntriesRequest->prevlogindex() > logs_.size() or logs_[appendEntriesRequest->prevlogindex()].term() != appendEntriesRequest->prevlogterm() )
                {
                    LOG_INFO << "onAppendEntryMessage: log un matched return false " << appendEntriesRequest->leaderid();
                    AppendEntriesResponse->set_success(false);
                }

                else{
                    LOG_INFO << "onAppendEntryMessage: log  matched return true " << appendEntriesRequest->leaderid();
                    AppendEntriesResponse->set_success(true);
                    if(!appendEntriesRequest->logentries().empty()){
                            const auto & log1 = appendEntriesRequest->logentries();
                            logs_.push_back(log1[0]);
                    }
                    if(appendEntriesRequest->leadercommit() > commitIndex){
                        commitIndex = appendEntriesRequest->leadercommit() > logs_.back().index() ? logs_[logs_.size()].index() :appendEntriesRequest->leadercommit();
                    }
                }
            }
        }

    }


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
    LOG_INFO<<"AppendEntryTimeout";
    rpcService::AppendEntriesRequest request;
    request.set_term(currentTerm);
    request.set_leaderid(raftName_);

    for (auto &client:clients_) {
        request.clear_logentries();
        if (client->nextIndex() < logs_.size() and client->nextIndex() == (client->matchIndex() + 1) ) {
            const auto &logOri = logs_[client->nextIndex()];//FIXME
            auto log = request.add_logentries();
            log->set_index(logOri.index());
            log->set_term(logOri.term());
            log->set_command(logOri.command());
            log->set_commandname(logOri.commandname());
        }

        const auto &log2 = logs_[client->nextIndex() - 1];
        request.set_prevlogindex(log2.index());
        request.set_prevlogterm(log2.term());
        request.set_leadercommit(commitIndex);

        client->AppendEntry(request, [&](google::protobuf::Message *response) {
            auto *AppendEntriesResponse = static_cast<rpcService::AppendEntriesResponse *>(response);
            if(currentTerm < AppendEntriesResponse->term()){
                currentTerm = AppendEntriesResponse->term();
                status_ = kFollower;
            }else{
                    //TODO
            }
        });
    }
    raft::displayAppendEntry(request);
    debugRaft();
    auto t = randomTime(5, 10);
    double delay = t / 10;
    loop_->runAfter(0.2, std::bind(&Raft::AppendEntryTimeout, this));

}




