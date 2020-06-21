
cmake_minimum_required(VERSION 3.15)
project(prcProto)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-lpthread -O2")

include_directories("../")
include_directories("../net")

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

aux_source_directory(../net NET_LIB_SRCS)
aux_source_directory(../log LOG_LIB_SRCS)
aux_source_directory(../base BASE_LIB_SRCS)
aux_source_directory(../Timer Timer_LIB_SRCS)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(client
        rpc.pb.cc
        rpc.pb.h
        message.pb.cc
        message.pb.h
        ${NET_LIB_SRCS}
        ${LOG_LIB_SRCS}
        ${BASE_LIB_SRCS}
        ${Timer_LIB_SRCS}
        RpcChannel.cpp RpcChannel.h
        Codec.h
        Codec.cpp
        TestMain/client.cpp Raft.cpp Raft.h)

add_executable(server
        rpc.pb.cc
        rpc.pb.h
        message.pb.cc
        message.pb.h
        ${NET_LIB_SRCS}
        ${LOG_LIB_SRCS}
        ${BASE_LIB_SRCS}
        ${Timer_LIB_SRCS}
        RpcChannel.cpp RpcChannel.h
        Codec.cpp
        TestMain/server.cpp Raft.cpp Raft.h)
target_link_libraries(client ${Protobuf_LIBRARIES})
target_link_libraries(server ${Protobuf_LIBRARIES})