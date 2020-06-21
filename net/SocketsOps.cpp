//
// Created by parallels on 5/12/20.
//

#include "SocketsOps.h"
#include <cstdint>
#include "log/logger.h"
#include "base/Types.h"
#include <fcntl.h>
void setNonBlockAndCloseOnExec(int sockfd)
{
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);

    (void)ret;
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
 //   if(inet_aton(ip, &addr->sin_addr) < 0)
     LOG_TRACE <<"inet_pton " << ip ;
    if (::inet_pton(AF_INET, ip, (void*)&addr->sin_addr) <= 0)
    {
        LOG_SYSFATAL << "sockets::fromIpPort";
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port,
                         struct sockaddr_in6* addr)
{

}

int sockets::createNonblockingOrDie(sa_family_t family)
{
#ifdef SOCK_NONBLOCK
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
#else
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    setNonBlockAndCloseOnExec(sockfd);
#endif


    return sockfd;
}

int sockets::createblockingOrDie(sa_family_t family)
{
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    return sockfd;
}

void sockets::bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
    if (ret < 0)
    {
        LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void sockets::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, 1000);
    if (ret < 0)
    {
        LOG_SYSFATAL << "sockets::listenOrDie fd:" <<sockfd;
    }
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

#if 1
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
   // struct sockaddr_in tmp= *(struct sockaddr_in*)addr;
#

    if (connfd < 0)
    {
        perror("accept");
        int savedErrno = errno;
        LOG_TRACE << "Socket::accept";
        perror("accpet");
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_TRACE << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_TRACE << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr)
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in* addr)
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

void sockets::shutdown(int fd,int how){
    if (::shutdown(fd, SHUT_WR) < 0)
    {
        if(errno != 107) //Transport endpoint is not connected
        LOG_SYSFATAL << "sockets::shutdownWrite";
    }
}

int sockets::connect(int sockfd, const struct sockaddr* addr)
{
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}

