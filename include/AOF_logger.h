#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>

class AOFLogger {
public:
    AOFLogger(const std::string& filename)  ;
    
    ~AOFLogger() ;
    
    // 主线程调用：追加日志到缓冲区（COW技术）
    void append(const std::string& command);

private:
    // 后台线程函数
    void background_flush();

    // 交换缓冲区（原子操作）
    void swap_buffers() ;

    // 实际刷盘操作
    void flush_buffer_to_disk(const std::vector<std::string>& buffer);

    std::vector<std::string> active_buffer;  // 主线程写入的缓冲区
    std::vector<std::string> pending_buffer; // 后台线程刷盘的缓冲区
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::thread flush_thread;
    std::string filename;
};