//
// Created by parallels on 6/4/20.
//

#include "TimeWheel.h"
#include "log/logger.h"
#include <utility>
#include "EventLoop.h"
#include "TcpConnection.h"

TimeWheel::TimeWheel(EventLoop *loop, int nums) :
        loop_(loop),
        connectionWheel_(nums) {
    loop->runEvery(1, std::bind(&TimeWheel::onTimer, this));
}

TimeWheel::ConnEntryPtr TimeWheel::addConnection(const TcpConnectionptr &tcpConnectionptr) {
    ConnEntryPtr entry(new ConnEntry(std::move(tcpConnectionptr)));
    loop_->runInLoop([&]() {
        loop_->assertInLoopThread();
        connectionWheel_.back().push_back(entry);
    });
    return entry;
}

void TimeWheel::onTimer() {
    loop_->assertInLoopThread();
    for (auto item:connectionWheel_) {
        LOG_TRACE << "onTimer: Wheel front size:" << item.size();
    }
    Bucket bucket;
    connectionWheel_.push_back(std::move(bucket));
    connectionWheel_.pop_front();
}

void TimeWheel::upadateConnection(const TimeWheel::ConnEntryWeakPtr &connEntryPtr) {


    loop_->runInLoop([&]() {
        loop_->assertInLoopThread();
        auto connEntry = connEntryPtr.lock();
        if (connEntry) {
            connectionWheel_.back().push_back(connEntry);
        }
    });
}


TimeWheel::ConnEntry::~ConnEntry() {
    auto conn = tcpConnectionWeakPtr_.lock();
    LOG_TRACE << "~ConnEntry :" << conn->peerAddr();
    if (conn) {

        conn->shutdown();
    } else {
        LOG_TRACE << "~ConnEntry  :shutdown failed " << conn->peerAddr();
    }
}
