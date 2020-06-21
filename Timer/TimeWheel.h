//
// Created by parallels on 6/4/20.
//

#ifndef MUDUO_STUDY_TIMEWHEEL_H
#define MUDUO_STUDY_TIMEWHEEL_H

#include <memory>
#include <list>
class TcpConnection;
class EventLoop;
class TimeWheel {
public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionptr;
    TimeWheel(EventLoop * loop,int nums);
    typedef std::weak_ptr<TcpConnection> TcpConnectionWeakPtr;
    struct ConnEntry{
        explicit ConnEntry(const TcpConnectionptr& tcpConnectionptr):
        tcpConnectionWeakPtr_(tcpConnectionptr){
        }
        ~ConnEntry();
        TcpConnectionWeakPtr tcpConnectionWeakPtr_;
    };
    typedef std::weak_ptr<ConnEntry> ConnEntryWeakPtr;
    typedef std::shared_ptr<ConnEntry> ConnEntryPtr;
    typedef std::list<ConnEntryPtr> Bucket;
    typedef std::list<Bucket> ConnectionWheel;

    ConnEntryPtr addConnection( const TcpConnectionptr& tcpConnectionptr);
    void upadateConnection( const ConnEntryWeakPtr& connEntryPtr);
    void onTimer();

private:
    EventLoop * loop_;
    ConnectionWheel connectionWheel_;



};


#endif //MUDUO_STUDY_TIMEWHEEL_H
