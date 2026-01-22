#ifndef HEAPTIMER_HPP
#define HEAPTIMER_HPP
#include<queue>
#include<unordered_map>
#include<time.h>
#include<algorithm>
#include<arpa/inet.h>
#include<functional>
#include<assert.h>
#include<chrono>//C++ 标准库中用于时间处理的头文件
typedef std::function<void()> timeoutCallback;
typedef std::chrono::high_resolution_clock Clock;//简化名字
typedef std::chrono::milliseconds MS;
struct TimerNode{
    int id;//fd
    Clock::time_point expires;//到期时间
    timeoutCallback cb;//回调函数
    //重载<,进行堆比较
    bool operator<(const TimerNode& t) const{
        return expires<t.expires;
    }
};
class heapTimer{
private:
    std::vector<TimerNode> heap_;
    std::unordered_map<int,size_t> ref_;//映射，fd到索引
    //删除指定位置节点
    void del_(size_t i){ 
        assert(!heap_.empty()&&i>=0&&i<heap_.size());
        //将要删除节点换到队尾，然后pop_back
        size_t n=heap_.size()-1;
        if(i<n){
            swapNode_(i,n);
            if(!siftDown_(i,n)){
                siftUp_(i);
            }
        }
        //移动队尾
        ref_.erase(heap_.back().id);
        heap_.pop_back();
    }
    //上调
    void siftUp_(size_t i){
        assert(!heap_.empty()&&i<heap_.size());
        size_t j=(i-1)/2;//父节点
        while(j>=0){
            if(heap_[j]<heap_[i]){
                break;
            }
            swapNode_(i,j);
            i=j;
            j=(i-1)/2;
        }
    }
    //下沉
    bool siftDown_(size_t index,size_t n){ 
        assert(index>=0&&index<heap_.size());
        size_t i=index;
        size_t j=i*2+1;
        while(j<n){
            if(j+1<n&&heap_[j+1]<heap_[j])j++;
            if(heap_[i]<heap_[j])break;
            swapNode_(i,j);
            i=j;
            j=i*2+1;
        }
        return i>index;
    }
    void swapNode_(size_t i,size_t j){
        assert(i>=0&&i<heap_.size());
        assert(j>=0&&j<heap_.size());
        std::swap(heap_[i],heap_[j]);
        ref_[heap_[i].id]=i;
        ref_[heap_[j].id]=j;
    }
public:
    heapTimer(){heap_.reserve(64);}//reserve()设置初始容量,单位为元素个数
    ~heapTimer(){clear();}
    //调整计时器的超时时间
    void adjust(int fd,int timeout){
        assert(!heap_.empty()&&ref_.count(fd));//断言：调试用的
        heap_[ref_[fd]].expires=Clock::now()+MS(timeout);
        //调整堆,下沉
        siftDown_(ref_[fd],heap_.size());
    }
    //添加计时器，新来的fd
    void add(int id,int timeout,const timeoutCallback& cb){
        assert(id>=0);
        size_t i;
        if(ref_.count(id)==0){
            //新节点
            i=heap_.size();
            ref_[id]=i;
            heap_.push_back({id,Clock::now()+MS(timeout),cb});
            siftUp_(i);
        }else{
            //已经存在的节点
            i=ref_[id];
            heap_[i].expires=Clock::now()+MS(timeout);
            heap_[i].cb=cb;
            siftDown_(i,heap_.size());
        }
    }
    //删除id的定时器，主动删，不是超时
    void doWork(int id){
        if(heap_.empty()||ref_.count(id)==0){
            return;
        }
        size_t i=ref_[id];
        TimerNode node=heap_[i];
        node.cb();
        del_(i);
    }
    //清超时节点
    void tick(){
        if(heap_.empty()){return;} 
        while(!heap_.empty()){
            TimerNode node=heap_.front();//取堆顶，连接最久的
            if(std::chrono::duration_cast<MS>(node.expires-Clock::now()).count()>0)break;
            //过期了
            node.cb();//执行回调
            pop();
        }
    }
    //删除堆顶
    void pop(){
        assert(!heap_.empty());
        del_(0);
    }
    //清空堆
    void clear(){
        ref_.clear();
        heap_.clear();
    }
    //获取下次清空时间差
    int getNextTick(){
        tick();//刷新
        size_t res=-1;
        if(!heap_.empty()){
            res=std::chrono::duration_cast<MS>(heap_.front().expires-Clock::now()).count();
            if(res<0)res=0;
        }
        return res;
    }

};
#endif