// server.cpp
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#include <mutex>
#include <queue>
#include <list>

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



// LRU缓存类（线程安全）
class LRUCache {
    public:
        LRUCache(size_t max_size = 100) : max_size(max_size) {}
    
        void put(const std::string& key, const std::string& value) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = cache_map.find(key);
            
            // 如果存在，更新值并移到链表头部
            if (it != cache_map.end()) {
                cache_list.erase(it->second);
            }
            
            cache_list.push_front({key, value});
            cache_map[key] = cache_list.begin();
            
            // 如果超过容量，淘汰尾部数据
            if (cache_map.size() > max_size) {
                auto last = cache_list.end();
                last--;
                cache_map.erase(last->key);
                cache_list.pop_back();
            }
        }
    
        std::string get(const std::string& key) {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = cache_map.find(key);
            if (it == cache_map.end()) {
                return "nil";
            }
            
            // 移到链表头部表示最近使用
            cache_list.splice(cache_list.begin(), cache_list, it->second);
            return it->second->value;
        }
    
    private:
        struct Node {
            std::string key;
            std::string value;
        };
        
        std::list<Node> cache_list;                   // 双向链表（维护访问顺序）
        std::unordered_map<std::string, typename std::list<Node>::iterator> cache_map; // 哈希表
        std::mutex mutex;                             // 保护LRU结构的互斥锁
        size_t max_size;                              // 最大容量
    };
    
    extern LRUCache db; // 替换原有的线程安全哈希表



