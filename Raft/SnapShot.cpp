//
// Created by parallels on 6/27/20.
//

#include "SnapShot.h"
bool SnapShot::Saved(int64_t lastterm, int64_t lastindex,const std::vector<rpcService::AppendEntriesRequest_LogEntry> & logs){

    snapshot::snapshot snapshot1;
    snapshot1.set_lastterm(lastterm);
    snapshot1.set_lastindex(lastindex);
    for(const auto & log : logs){
        auto added_log = snapshot1.add_logentries();
        added_log->set_index(log.index());
        added_log->set_commandname(log.commandname());
        added_log->set_term(log.term());
    }

    google::protobuf::StringPiece s= snapshot1.SerializeAsString();
    int fd=  open(path_.c_str(),O_CREAT|O_TRUNC|O_WRONLY);
    if(fd < 0){
        perror("oepn");
        return false;
    }

    pid_t  pid = fork();
    if(pid == 0){
        int n = write(fd,s.data(),s.size());
        if(n < 0){
            perror("write");
            exit(-1);
        }
        exit(0);
    }
    return true;

}

snapshot::snapshot SnapShot::Read(){
    int fd=  open(path_.c_str(),O_RDONLY|O_CREAT);
    if(fd < 0){
        perror("oepn");
        abort();
    }
    char buf[8192];
    int n =  read(fd,buf, sizeof(buf));
    if(n < 0){
        perror("read");
        abort();
    }
    snapshot::snapshot snapshot1;
    if(!snapshot1.ParseFromArray(buf,n)){
        std::cout << "ParseFromArray error" << "\n";
    }
    std::cout << snapshot1.DebugString() << "\n";
    return snapshot1;
}

bool SnapShot::Saved(const snapshot::snapshot &snap) {
    google::protobuf::StringPiece s= snap.SerializeAsString();
    int fd=  open(path_.c_str(),O_CREAT|O_TRUNC|O_WRONLY);
    if(fd < 0){
        perror("oepn");
        return false;
    }

    pid_t  pid = fork();
    if(pid < 0){
        return false;
    }
    if(pid == 0){
        int n = write(fd,s.data(),s.size());
        if(n < 0){
            perror("write");
            exit(-1);
        }
        exit(0);
    }
    return true;
}
