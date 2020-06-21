//
// Created by parallels on 5/12/20.
//

#ifndef MUDUO_STUDY_SOCKET_H
#define MUDUO_STUDY_SOCKET_H

#include "InetAddress.h"

class Socket {
public:
    Socket();
    explicit Socket(int fd);
    ~Socket();
    void bindAddress(InetAddress addr);
    void listen();
    void connect(InetAddress addr);

    int accept(InetAddress * peerAddr);
    int fd() const  {return socketfd_;}
    int fd()        {return socketfd_;}

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    void shutdownWrite();



private:
   const  int socketfd_;
   bool alive_ = true;

};


#endif //MUDUO_STUDY_SOCKET_H
