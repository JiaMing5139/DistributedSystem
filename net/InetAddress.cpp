//
// Created by parallels on 5/12/20.
//

#include "InetAddress.h"
#include <string.h>
#include "SocketsOps.h"
#include <iostream>
InetAddress::InetAddress(uint16_t port, bool loopbackOnly, bool ipv6) :
        ipstr_("any_addr") ,
        portstr_(port)
{
    if(ipv6){
        ipv6 = true;
    }else{
        ipv6 = false;
        bzero(&addr_,sizeof addr_);
        addr_.sin_family=AF_INET;
        addr_.sin_port = htons(port);

        addr_.sin_addr.s_addr = INADDR_ANY;
    }

}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6):
ipstr_(std::move(ip)) ,
portstr_(port)
{

    if(ipv6){

    }else{
        sockets::fromIpPort(ipstr_.c_str(),port,&addr_);
    }

}

void InetAddress::setSockAddrInet6(const struct sockaddr_in6 &addr) {
        addr6_ = addr;
}

void InetAddress::setSockAddrInet4(const struct sockaddr_in &addr) {
    addr_ = addr;
}

struct sockaddr *InetAddress::getSockaddr() {
   return sockets::sockaddr_cast(&this->addr6_);
}

std::ostream &operator<<(std::ostream &os, const InetAddress &inetAddress) {
    struct sockaddr_in * addr4 =(struct sockaddr_in *)&inetAddress.addr6_;
    return os << inet_ntoa(addr4->sin_addr) << ":" << ntohs( addr4->sin_port);

}
