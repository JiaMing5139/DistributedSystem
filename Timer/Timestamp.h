//
// Created by parallels on 5/6/20.
//

#ifndef MUDUO_STUDY_TIMESTAMP_H
#define MUDUO_STUDY_TIMESTAMP_H


#include <cstdint>
#include <sys/time.h>
#include <ostream>
#include <iostream>
class Timestamp;
bool operator < (const Timestamp & ,const Timestamp &);
bool operator == (const Timestamp & ,const Timestamp &);
std::ostream & operator << (std::ostream & , const Timestamp &);
class Timestamp {
public:
    friend bool operator < (const Timestamp & ,const Timestamp &);
    friend bool operator == (const Timestamp & ,const Timestamp &);
    friend  std::ostream & operator << (std::ostream & , const Timestamp &);
    Timestamp(int64_t microTime);
    Timestamp() = default;

    static const int kMicroSecondsPersecond = 1000 * 1000;


    //
    time_t SecondsSinceEpoch() const{
        return static_cast<time_t>(mircotime_ / kMicroSecondsPersecond);
    }

    time_t mircoSecond() const{
        return mircotime_;
    }




    static Timestamp now(){
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t seconds = tv.tv_sec;
        return seconds*kMicroSecondsPersecond + tv.tv_usec ;
    }

    static int64_t cachedtime();
    static struct tm * timetoStruct();
    static std::string CurrentTimeStringWithCached(){
        struct tm * locoaltimeStruct = timetoStruct();
        std::string time_s(asctime(locoaltimeStruct));
        return time_s;
    }

private:
    int64_t mircotime_;

};

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPersecond);
    return Timestamp(timestamp.mircoSecond() + delta);
}


#endif //MUDUO_STUDY_TIMESTAMP_H
