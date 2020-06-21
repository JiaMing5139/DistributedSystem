//
// Created by parallels on 5/12/20.
//

#ifndef MUDUO_STUDY_INETADDRESS_H
#define MUDUO_STUDY_INETADDRESS_H

#include "base/Copyable.h"
#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ostream>
class InetAddress;
std::ostream & operator << (std::ostream & os,const InetAddress & inetAddress);

class InetAddress: public Copyable {
public:
    friend  std::ostream & operator << (std::ostream & os,const InetAddress & inetAddress);
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);


    InetAddress(std::string ip, uint16_t port, bool ipv6 = false);

    explicit InetAddress(const struct sockaddr_in& addr)
            : addr_(addr)
    { }

    explicit InetAddress(const struct sockaddr_in6& addr)
            : addr6_(addr)
    { }

    void setSockAddrInet6(const struct sockaddr_in6& addr);
    void setSockAddrInet4(const struct sockaddr_in &addr);
    struct sockaddr* getSockaddr();

private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
    bool ipv6 = false;
    std::string ipstr_;
    uint16_t portstr_;

};


#endif //MUDUO_STUDY_INETADDRESS_H
