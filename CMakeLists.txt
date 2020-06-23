cmake_minimum_required(VERSION 3.15)
project(Distributed_lab)

set(CMAKE_CXX_STANDARD 14)
include_directories("./")
include_directories("./net")

aux_source_directory(./log LOG_LIB_SRCS)
aux_source_directory(./base BASE_LIB_SRCS)
aux_source_directory(./Timer Timer_LIB_SRCS)
aux_source_directory(./net NET_LIB_SRCS)
aux_source_directory(./Rpc RPC_LIB_SRCS)
aux_source_directory(./Raft Raft_LIB_SRCS)

message("NET_LIB_SRCS")
message("${NET_LIB_SRCS}")

add_executable(Distributed_lab
        main.cpp
        ${NET_LIB_SRCS}
        ${LOG_LIB_SRCS}
        ${BASE_LIB_SRCS}
        ${Timer_LIB_SRCS}
        ${RPC_LIB_SRCS}
        ${Raft_LIB_SRCS}
        Raft/State.cpp Raft/State.h Service/Service.cpp Service/Service.h Service/kvService.cpp Service/kvService.h)
find_package(Protobuf REQUIRED)
find_package (Threads REQUIRED)
target_link_libraries(Distributed_lab
        ${CMAKE_THREAD_LIBS_INIT}
        )

target_link_libraries(Distributed_lab ${Protobuf_LIBRARIES})