// server.cpp
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

#define PORT 6379
#define BUFFER_SIZE 4096


//线程池类实现
class ThreadPool{
    public:
    ThreadPool(size_t num_threads=4);//默认四个线程
    ~ThreadPool();
    void enqueue(int client_socket);//添加任务到队列

    private:
        std::vector<std::thread>  threads;//线程数组
        std::queue<int>  tasks;//任务队列
        std::mutex queue_mutex;//保护任务队列的互斥锁
        std::condition_variable condition;//线程间通知条件变量
        bool stop;//线程池停止标志

};

extern ThreadPool pool;//全局线程池实例

//线程安全的哈希表包装类
class SafeMap{
    public:
        void set(const std::string &key,const std::string &value);
        std::string get(const std::string &key);
        std::unordered_map<std::string,std::string> getMap(){
            std::lock_guard<std::mutex> lock(map_mutex);
            return map ;
        }
    
    private:
        std::unordered_map<std::string,std::string>  map;
        std::mutex map_mutex;//保护哈希表的互斥锁

};

extern SafeMap db;//全局线程安全数据库


