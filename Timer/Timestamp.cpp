//
// Created by parallels on 5/6/20.
//

#include "Timestamp.h"
#include <string.h>
#include <iostream>

__thread time_t cache_time = 0;
__thread struct tm  cached_locoaltimeStruct;
__thread struct tm ret;


Timestamp::Timestamp(int64_t microTime):mircotime_(microTime) {

}

bool operator<(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.mircoSecond() < rhs.mircoSecond();
}

bool operator==(const Timestamp &lhs, const Timestamp &rhs) {
    return lhs.mircoSecond() == rhs.mircoSecond();
}

std::ostream &operator<<(std::ostream & os, const Timestamp & timestamp) {

    time_t seconds = timestamp.SecondsSinceEpoch();
    auto locoaltimeStruct = localtime(&seconds);
    std::string time_s(asctime(locoaltimeStruct));
    return os << time_s;
}

int64_t Timestamp::cachedtime() {
    if(cache_time == 0 ){
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        time_t now_seconds = tv.tv_sec;
        cache_time = now_seconds;
    }
    return cache_time;
}

struct tm * Timestamp::timetoStruct() {


    if(cache_time == 0){   //第一次调用 初始化cached time和struct
        time_t first  = cachedtime();
        const time_t *time   = &first;
        memcpy(&cached_locoaltimeStruct,localtime(time), sizeof (struct tm));
        return &cached_locoaltimeStruct;
    }else{
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        time_t now_seconds = tv.tv_sec;
     //  std::cout << "cached second:" << cache_time << " current time " << now_seconds << std::endl;
        if(now_seconds - cachedtime() >= 5){
            const time_t *time =  &now_seconds;
            cache_time = now_seconds;
            memcpy(&cached_locoaltimeStruct,localtime(time), sizeof (struct tm));
            return &cached_locoaltimeStruct;
        }else{
        //    std::cout << "cached_locoaltimeStruct :" << cached_locoaltimeStruct.tm_sec << std::endl;
            memcpy(&ret,&cached_locoaltimeStruct, sizeof (struct tm));
            ret.tm_sec +=  now_seconds - cachedtime();
            ret.tm_sec = ret.tm_sec%60;
           // std::cout << "changed_locoaltimeStruct :" << ret.tm_sec << std::endl;
            return  &ret;
        }
    }


}

