# DistributedSystem

Study MIT 6.824 lab with C++

I finished my own network layer in the last project, so I want to use my own network layer to do the lab, It's not robust enough, but I want to try to figure out how does the whole system work from scratch

### Not completed
- install snapshot RPC

1. when nextIndex get 0, it means that the rest of the logs are in the snapshot on the disk, sent the Install snapshot PRC to the follower
- linearizable semantics

1. Id of each operation to prevent the repeated client quest 

2. send a blank Appendentry RPC to synchronize the committed logs and make raft readable to prevent read the dirty data

## compile
####  Protobuf
```
cd Raft
protoc --cpp_out=./snapshot.proto
protoc --cpp_out=./Raft.proto
cd Service
protoc --cpp_out=./serviceMessage.proto
cd Rpc
protoc --cpp_out=./Rpc.proto
```

#### Cmake
```
server:
cmake CMakeList.txt
client:
cmake Service/CmakeList.txt
```
