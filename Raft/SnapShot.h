//
// Created by parallels on 6/26/20.
//

#ifndef DISTRIBUTED_LAB_SNAPSHOT_H
#define DISTRIBUTED_LAB_SNAPSHOT_H



#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "snapshot.pb.h"
#include "Raft.pb.h"
class SnapShot{
public:
    explicit SnapShot(std::string path):path_(std::move(path)){};
    bool Saved(int64_t lastterm, int64_t lastindex,const std::vector<rpcService::AppendEntriesRequest_LogEntry> & logs);
    bool Saved(const snapshot::snapshot &snap);
    snapshot::snapshot Read();
private:
    std::string path_;
};


#endif //DISTRIBUTED_LAB_SNAPSHOT_H
