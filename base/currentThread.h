//
// Created by jiaming pan on 4/14/20.
//

#ifndef CURRENTTHREAD_CURRENTTHREAD_H
#define CURRENTTHREAD_CURRENTTHREAD_H
#include <sys/syscall.h>
#include <sys/types.h>
#include "unistd.h"
#include <stdio.h>
namespace  Jimmy{
namespace CurrentThread {
    extern __thread int t_cachedTid;
    extern __thread pid_t t_cachedPid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char *t_threadName;


    pid_t gettid();

    void cacheTid();

    inline int tid(){
        if (__builtin_expect(t_cachedTid == 0, 0))  // optimize compiling , 优化编译过程，告诉编译器 表达式结果更可能是0;
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}
}


#endif //CURRENTTHREAD_CURRENTTHREAD_H
