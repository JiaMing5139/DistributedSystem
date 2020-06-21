//
// Created by parallels on 4/9/20.
//

#include "logger.h"
#include <iostream>
#include <time.h>
#include <sstream>
#include <string.h>
#include "Timer/Timestamp.h"
#include "base/currentThread.h"
namespace Jimmy {
Logger::LogLevel initLogLevel(){
    //fixme from a config or env
    return Logger::TRACE;
}
__thread char t_errnobuf[512];

void defaultoutput(const char * msg,size_t len){
    std::cout << msg ;
}

Logger::Output Logger::output = defaultoutput;

Logger::LogLevel  g_logLevel =  initLogLevel();

void Jimmy::Logger::setLevel(Jimmy::Logger::LogLevel level) {
    g_logLevel = level;
}


    Logger::Logger(const std::string & file, int line, Logger::LogLevel level, const char *func,bool ifabort):
    ifabort(ifabort)
    {
        std:: string log_type;
        switch (level){
            case TRACE: log_type = "TRACE";
                break;
            case ERROR:log_type = "ERROR";
                break;
            case INFO: log_type = "INFO";
        }
        _os<<"tid:" << Jimmy::CurrentThread::tid() << " " << Timestamp::CurrentTimeStringWithCached()  <<" "<< file <<" line:" << line << " " <<  log_type <<" ";
    }






    Logger::~Logger() {
        if(errno >0 ){
#ifdef __linux__
            strcpy(t_errnobuf,strerror_r(errno,t_errnobuf,sizeof t_errnobuf));
#endif
#ifdef __APPLE__
            strerror_r(errno,t_errnobuf,sizeof t_errnobuf);
#endif
            _os <<" errno:" << errno <<" ";
           _os << t_errnobuf;
        }
        _os << std::endl;
        Logger::output(_os.str().c_str(),_os.str().size());


        if(ifabort){

            abort();
        }

    }

    void Logger::setOutput(Output out) {
        Logger::output = std::move(out);
    }



}