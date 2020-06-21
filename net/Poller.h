

#include "base/noncopyable.h"
#include "memory"
#include "Channel.h"
#include <vector>
#include <unordered_map>
#include "EventLoop.h"
class Poller : noncopyable{

public:
    typedef EventLoop::Channelptrlist Channelptrlist;
    typedef EventLoop::Channelptr Channelptr;

    explicit Poller(EventLoop * loop):ownerLoop_(loop){}
    virtual ~Poller();
    virtual void poller_wait(int timeOut, EventLoop::Channelptrlist* activedChannels) = 0;
    virtual void updateChannel(EventLoop::Channelptr) = 0;
    virtual void removeChannel(EventLoop::Channelptr) = 0;
    virtual bool hasChannel(EventLoop::Channelptr) = 0;
    virtual int channelNum() {return channels_.size();};
    void assertInLoopThread(){
        ownerLoop_->assertInLoopThread();
    }

protected:
    typedef std::unordered_map<int,EventLoop::Channelptr> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;



};