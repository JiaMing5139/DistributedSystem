//
// Created by jiaming pan on 5/24/20.
//

#ifndef MUDUO_STUDY_MYEVENTFD_H
#define MUDUO_STUDY_MYEVENTFD_H
namespace Jimmy{

class myeventFd {
public:
    myeventFd();
    int eventfd_write();
    int eventfd_read();
    int fd();
    ~myeventFd();


private:
    int createFd();
    int readfd_;

#ifdef __APPLE__
    int fds[2];
#endif
};


#endif //MUDUO_STUDY_MYEVENTFD_H
}