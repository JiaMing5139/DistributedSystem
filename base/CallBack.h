//
// Created by parallels on 5/11/20.
//

#ifndef MUDUO_STUDY_CALLBACK_H
#define MUDUO_STUDY_CALLBACK_H

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void ()> funcCallback;
typedef std::function<void ()> TimerCallback;

typedef std::function<void (ssize_t size, const TcpConnectionPtr &conn)>  HighWaterMarkCallback;
typedef std::function<void (const TcpConnectionPtr &conn)> TcpEventCallback;
typedef std::function<void (Buffer *,TcpConnectionPtr conn)> readTcpEventCallback;
#endif //MUDUO_STUDY_CALLBACK_H
