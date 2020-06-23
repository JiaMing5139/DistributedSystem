//
// Created by parallels on 5/25/20.
//

#ifndef MUDUO_STUDY_CONNECTOR_H
#define MUDUO_STUDY_CONNECTOR_H

#include "base/noncopyable.h"
#include "InetAddress.h"
#include <memory>
#include <atomic>
#include <utility>
    class Channel;
    class EventLoop;

    class Connector : noncopyable,
                      public std::enable_shared_from_this<Connector>
    {
    public:
        typedef std::function<void (int sockfd)> NewConnectionCallback;

        Connector(EventLoop* loop, const InetAddress& serverAddr);

        ~Connector();

        void setNewConnectionCallback(const NewConnectionCallback& cb)
        { newConnectionCallback_ = cb; }

        void setConnectFaildCallback(const std::function<void (int)>& cb){
            connectFaildCallback = cb;
        }

        void start();  // can be called in any thread
        void restart();  // must be called in loop thread
        void stop();  // can be called in any thread

        const InetAddress& serverAddress() const { return serverAddr_; }

    private:
        enum States { kDisconnected, kConnecting, kConnected };
        static const int kMaxRetryDelayMs = 30*1000;
        static const int kInitRetryDelayMs = 500;

        void setState(States s) { state_ = s; }
//        void startInLoop();
//        void stopInLoop();
         void connect();
//        void connecting(int sockfd);
         void handleWrite();
//        void handleError();
//        void retry(int sockfd);
//        int removeAndResetChannel();
//        void resetChannel();

        EventLoop* loop_;
        InetAddress serverAddr_;
        std::atomic<bool> connect_;
        std::atomic<States> state_;
        std::shared_ptr<Channel> channelptr_;
        NewConnectionCallback newConnectionCallback_;
        std::function<void (int)> connectFaildCallback;
        int retryDelayMs_;
    };


#endif //MUDUO_STUDY_CONNECTOR_H
