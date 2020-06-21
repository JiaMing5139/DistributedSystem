//
// Created by parallels on 5/26/20.
//

#ifndef MUDUO_STUDY_CODEC_H
#define MUDUO_STUDY_CODEC_H

#include <functional>
#include <memory>
#include <google/protobuf/message.h>

class Buffer;
class TcpConnection;
class Codec {
public:
    enum ErrorCode
    {
        kNoError = 0,
        kTypeError,
        kParaError
    };
    typedef  std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::shared_ptr<google::protobuf::Message> MessagePtr;
    typedef std::function<void (MessagePtr message, std::shared_ptr<TcpConnection> connptr)> OnMessageCompleteCallback;
    typedef std::function<void (Buffer *, std::shared_ptr<TcpConnection> connptr)> OnErrorCallback;

    void setOnMessageCompleteCallback(const OnMessageCompleteCallback & cb){onMessageCompleteCallback_ = cb;}
    void setOnErrorCallback(const OnErrorCallback & cb) { onErrorCallback_ = cb; }

    void onMessage(Buffer *buff, TcpConnectionPtr conn);


     void FillBuffer(const google::protobuf::Message & message, char *, int *len);
     Codec::MessagePtr parse( const char * buff, int len,ErrorCode *) ;
     google::protobuf::Message* createMessage(const std::string& typeName);
private:
    class Writer{
    public:
        void writeInt32(const int32_t * src, char * dst){
            writeAny((const char*)src, dst,4);
        }
        void writeInt32( ){
            writeSeek_ += 4;
        }
        void writeByte(const char * src, char * dst){
            writeAny(src, dst, 1) ;
        }
        void writeAny(const char * src, char * dst , int len ){
            memcpy(dst + writeSeek_,src,len);
            writeSeek_ += len;
        }
        void setSeek(size_t pos) {
            writeSeek_ = 0;
        }
        size_t writeSeek(){
            return writeSeek_;
        }
    private:
        size_t  writeSeek_ = 0;
    };


    OnMessageCompleteCallback onMessageCompleteCallback_;
    OnErrorCallback onErrorCallback_;

    const static int kHeadersize = 4;
    const static int minSize = 2*kHeadersize + 30;
    const static int maxSize =   256*256*256*64;


};


#endif //MUDUO_STUDY_CODEC_H
