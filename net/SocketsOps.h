//
// Created by parallels on 5/12/20.
//

#ifndef MUDUO_STUDY_SOCKETSOPS_H
#define MUDUO_STUDY_SOCKETSOPS_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <arpa/inet.h>
namespace  sockets{
    struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
    struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);

    void fromIpPort(const char* ip, uint16_t port,
                    struct sockaddr_in* addr);
    void fromIpPort(const char* ip, uint16_t port,
                    struct sockaddr_in6* addr);

    int createblockingOrDie(sa_family_t family);
    int createNonblockingOrDie(sa_family_t family);
    void bindOrDie(int sockfd, const struct sockaddr* addr);

    void listenOrDie(int sockfd);
    int accept(int sockfd, struct sockaddr_in6* addr);
    void shutdown(int,int);
    int connect(int sockfd, const struct sockaddr* addr);
}


#endif //MUDUO_STUDY_SOCKETSOPS_H
