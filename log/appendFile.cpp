//
// Created by parallels on 4/15/20.
//

#include "appendFile.h"
#include <assert.h>

#include <utility>

size_t appendFile::write(const char *logline, size_t len) {
    return fwrite(logline,len,sizeof(char),fp_);
}

void appendFile::flush() {
    fflush(fp_);
}

void appendFile::append(const char *logline, size_t len) {
  size_t n =  write(logline,len);
  writtenBytes_ += n;
  //TODO send again if not send all data previously;
//  while(n < len){
//      size_t n =  write(logline,len);
//  }
}

appendFile::~appendFile() {
    fclose(fp_);
}


FILE* openfile(std::string filename){
    FILE * file = ::fopen(filename.c_str(),"ab");
    if(file == NULL){
        perror("fopen");
        abort();
    }
    return file;
}

appendFile::appendFile(std::string filename):fp_(openfile(std::move(filename))) {
    assert(fp_);
    setbuffer(fp_,buffer_,sizeof buffer_);
}


