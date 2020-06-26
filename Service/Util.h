//
// Created by parallels on 6/25/20.
//

#ifndef DISTRIBUTED_LAB_UTIL_CPP
#define DISTRIBUTED_LAB_UTIL_CPP

#include <vector>
#include <string>
namespace Utils {
    inline std::vector<std::string> splitString(const char *str, size_t len) {
        bool recording = false;
        const char * start = str;
        std::vector<std::string> ret;
        int lastlens = 0;
        for (int i = 0; i < len; i++) {
            if (str[i] == ' ' or str[i] == '\0' or str[i] == '\n') {
                if (recording) {
                    std::string newStr = std::string(start, start +(i- lastlens));
                    ret.push_back(newStr);
                    recording = false;
                }
            } else {
                if(!recording) {
                    lastlens = i;
                    start = str + i;
                    recording = true;
                }
            }

        }
        return ret;
    }

    inline std::vector<std::string> splitString(const std::string & str) {
        auto start = str.begin();
        auto endofStr = str.begin();
        std::vector<std::string> ret;
        do{
            endofStr = find(start, str.end(), ' ');
            std::string tmp(start, endofStr);
            ret.push_back(std::move(tmp));
            start= endofStr + 1;
        }   while (endofStr != str.end());
        return ret;
    }
}

#endif //DISTRIBUTED_LAB_UTIL_CPP
