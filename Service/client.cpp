//
// Created by parallels on 6/25/20.
//

#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "Service/ServiceClient.h"

int main(int argc,char ** argv){

    if (argc<2) {
        perror("local port peer port1 port2 port3..");
        return 0;
    }
    std::vector<InetAddress> points;
    for(int i = 1 ; i < argc ; i ++){

        points.push_back(std::move(InetAddress(atoi( argv[i]))));
    }

    EventLoop loop;
    ServiceClient client(&loop,points);
    client.start();
    loop.loop();
}