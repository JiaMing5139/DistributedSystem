//
// Created by parallels on 4/13/20.
//

#include <sys/syscall.h>
#include <sys/types.h>
#include "unistd.h"
#include <stdio.h>
namespace Jimmy{
namespace CurrentThread{
        int i = 0;
         __thread int t_cachedTid = 0;
         __thread char t_tidString[32] = {0};
         __thread int t_tidStringLength;
         __thread const char* t_threadName = nullptr;

        pid_t gettid(){
            return  static_cast<pid_t >(syscall(SYS_gettid));
        }

        void cacheTid(){
            if(t_cachedTid == 0){
                t_cachedTid = gettid();
                t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
            }

        }
    }


}