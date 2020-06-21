//
// Created by parallels on 5/26/20.
//

#include "Codec.h"
#include "log/logger.h"
#include <utility>
#include "Buffer.h"

google::protobuf::Message* Codec::createMessage(const std::string& typeName)
{
    google::protobuf::Message* message = nullptr;
    const google::protobuf::Descriptor* descriptor =
            google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
    if (descriptor)
    {
        const google::protobuf::Message* prototype =
                google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            message = prototype->New();
        }
    }
    return message;
}
int32_t asInt32(const char* buf)
{
    int32_t be32 = 0;
    ::memcpy(&be32, buf, sizeof(be32));
    //FIXME net to nost

    return be32;
}

Codec::MessagePtr Codec::parse( const char * buff, int len,ErrorCode * errncode ) {
    MessagePtr ret;
    int32_t  typelen =  asInt32(buff);
  //  std::cout << typelen <<std::endl;
    std::string typeName(buff + kHeadersize , typelen - 1);
 //   std::cout << typeName <<std::endl;
   // std::string typeName1("Jimmy.MessageChat");

    ret.reset(createMessage(typeName));
    if(ret){
        const char * start = buff + kHeadersize + typelen;
        int32_t  datalen = len - kHeadersize - typelen;
        if(ret-> ParseFromArray(start,datalen)){
           //    std::cout << ret->DebugString() << std::endl;
               *errncode = Codec::kNoError;
        }else{
            *errncode = Codec::kParaError;
        }
    }else{
        *errncode = Codec::kTypeError;
    }
    return ret;

}



void Codec::FillBuffer(const google::protobuf::Message &message,  char * buff, int *len) {
    //留下4字节空白
    Writer writer;
    writer.writeInt32();

    //写入typename长度和typename
    const std::string& typeName = message.GetTypeName();
    int32_t typelen = typeName.size() + 1;
    writer.writeInt32(&typelen,buff);
    writer.writeAny(const_cast<char *>(typeName.c_str()), buff, typeName.size());
    writer.writeByte("\0",buff);

    //写入序列化结果
    int byte_size = google::protobuf::internal::ToIntSize(message.ByteSizeLong());
    uint8_t * start = (uint8_t*)buff + writer.writeSeek();
    message.SerializeWithCachedSizesToArray(start);

    //写入总长度
    *len = sizeof typelen + typelen + byte_size;
    writer.setSeek(0);
    writer.writeInt32(len,buff);

}

void Codec::onMessage(Buffer *buff, Codec::TcpConnectionPtr conn) {
    LOG_TRACE<<"readableBytes in buffer: " << buff->readableBytes();
    if(buff->readableBytes() >= kHeadersize * 2 ){
        int32_t len = buff->peekInt32();
        LOG_TRACE<<"readableBytes > kHeadersize*2 , len in packet:" << len;
        LOG_TRACE<<"readableBytes in buffer: " << buff->readableBytes();
        if(buff->readableBytes() >= len + kHeadersize ) {

            buff->retrieve(kHeadersize);
            int32_t namelen = buff->peekInt32();
            LOG_TRACE<<"namelen: " << len;
            if(len - namelen - kHeadersize < maxSize){
                ErrorCode errorCode;
                MessagePtr messagePtr =  parse(buff->peek(),len ,&errorCode);
                buff->retrieve(len);
                LOG_TRACE << "ErrorCode:" << errorCode;
                if(errorCode == kNoError){
                    if(onMessageCompleteCallback_){
                        LOG_TRACE << "call onProtobufMessage";
                        onMessageCompleteCallback_(messagePtr,std::move(conn));
                    }
                }else{
                    onErrorCallback_(buff,std::move(conn));
                }

            } else{
                // payload size over protobuf max limit
                buff->retrieveAll();
            }

        }
    }

}

