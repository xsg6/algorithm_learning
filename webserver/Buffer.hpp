#ifndef BUFFER_HPP
#define BUFFER_HPP 
#include<vector>
#include<string>
#include<algorithm>
#include<cstring>
class Buffer { 
private:
    std::vector<char> buffer_;
    size_t writerIndex_;
    size_t readIndex_;
    char* begin(){return &*buffer_.begin();}
    const char*begin() const{return &*buffer_.begin();}
    void makeSpace(size_t len){
        if(writableBytes()+prependableBytes()<len+kCheapPrepend){
            buffer_.resize(writerIndex_+len);
        }else{
            size_t readable=readableBytes();
            std::copy(begin()+readIndex_,begin()+writerIndex_,begin()+kCheapPrepend);
            readIndex_=kCheapPrepend;
            writerIndex_=readIndex_+readable;
        }
    }
    size_t prependableBytes() const{
        return readIndex_;
    }

public:
    static const int kCheapPrepend=8;//预留空间
    static const int kInitialSize=1024;//缓冲区大小byte
    explicit Buffer(size_t initialSize=kInitialSize):buffer_(kCheapPrepend+initialSize),writerIndex_(kCheapPrepend),readIndex_(kCheapPrepend){}
    //可读字节
    size_t readableBytes() const{
        return writerIndex_-readIndex_;
    }
    //可写字节
    size_t writableBytes() const{
        return buffer_.size()-writerIndex_;
    }
    //返回读指针
    const char* peek() const{
        return begin()+readIndex_;
    }
    //消费长为len的数据
    void retrieve(size_t len){
        if(len<readableBytes()){
            readIndex_+=len;
        }else{
            retrieveAll();
        }
    }
    void retrieveAll(){
        readIndex_=kCheapPrepend;
        writerIndex_=kCheapPrepend;
    }
    //转回string
    std::string retrieveAllAsString(){
        return retrieveAsString(readableBytes());
    }
    std::string retrieveAsString(size_t len){
        std::string result(peek(),len);
        retrieve(len);
        return result;
    }
    //重载
    void append(const char*data,size_t len){
        ensureWritableBytes(len);
        std::copy(data,data+len,beginWrite());
        writerIndex_+=len;
    }
    void append(const std::string& str){
        append(str.data(),str.size());
    }
    //确保空间够写
    void ensureWritableBytes(size_t len){
        if(writableBytes()<len){
            makeSpace(len);
        }
    }
    void hasWritten(size_t len) {
        writerIndex_ += len;
    }
    char* beginWrite(){return begin()+writerIndex_;}
    const char* beginWrite() const{return begin()+writerIndex_;}

};
#endif