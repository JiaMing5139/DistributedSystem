//
// Created by parallels on 4/15/20.
//

#ifndef LOG_APPENDFILE_H
#define LOG_APPENDFILE_H

#include <string>
class appendFile{
public:
    explicit appendFile(std::string filename);

    ~appendFile();

    void append(const char* logline, size_t len);

    void flush();

    off_t writtenBytes() const { return writtenBytes_; }


private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[64*1024];
    off_t writtenBytes_;
};


#endif //LOG_APPENDFILE_H
