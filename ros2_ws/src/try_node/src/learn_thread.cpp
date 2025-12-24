#include<iostream>
#include<thread>
#include<chrono>//时间库
#include<functional>//函数包装器
#include "httplib.h"
class Download
{
public:
    void download(const std::string &host,const std::string &path,const std::function<void(const std::string&,const std::string&)> &callback)
    {
        std::cout<<"线程"<<std::this_thread::get_id()<<std::endl;
        httplib::Client client(host);
        auto res=client.Get(path);
        if(res&& res->status==200)
        {
            callback(path,res->body);
        }
        else
        {
            callback(path,"");
        }
    }
    void start_download(const std::string &host,const std::string &path,const std::function<void(const std::string&,const std::string&)> &callback)
    {
        auto download_fun=std::bind(&Download::download,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
        std::thread thread(download_fun,host,path,callback);
        thread.detach();//分离线程
    }  
}; 
int main(){
    auto d=Download();
    auto word_count=[](const std::string &path,const std::string &result)->void{
         std::cout<<"下载完成"<< path <<result.length()<<result.substr(0,5)<<std::endl;
    };
    d.start_download("http://0.0.0.0:8000","/novel1.txt",word_count);
    d.start_download("http://0.0.0.0:8000","/novel2.txt",word_count);
    d.start_download("http://0.0.0.0:8000","/novel3.txt",word_count);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000*10));
    return 0;
}