cmake_minimum_required(VERSION 3.15)
project(kvClient)

set(CMAKE_CXX_STANDARD 14)
include_directories("../")
include_directories("./")
include_directories("../net")
include_directories("../Timer")

aux_source_directory(../log LOG_LIB_SRCS)
aux_source_directory(../base BASE_LIB_SRCS)
aux_source_directory(../Timer Timer_LIB_SRCS)
aux_source_directory(../net NET_LIB_SRCS)
aux_source_directory(../Rpc RPC_LIB_SRCS)
aux_source_directory(../Raft Raft_LIB_SRCS)

message("NET_LIB_SRCS")
message("${NET_LIB_SRCS}")

add_executable(kvClient
        client.cpp
        ${NET_LIB_SRCS}
        ${LOG_LIB_SRCS}
        ${BASE_LIB_SRCS}
        ${Timer_LIB_SRCS}
        ${RPC_LIB_SRCS}
        ${Raft_LIB_SRCS}
         ServiceClient.h ServiceClient.cpp serviceMessage.pb.cc serviceMessage.pb.h)
find_package(Protobuf REQUIRED)
find_package (Threads REQUIRED)
target_link_libraries(kvClient
        ${CMAKE_THREAD_LIBS_INIT}
        )

target_link_libraries(kvClient ${Protobuf_LIBRARIES})