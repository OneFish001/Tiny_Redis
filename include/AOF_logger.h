#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>

class AOFLogger {
public:
    AOFLogger(const std::string& filename = "appendonly.aof") 
        : filename(filename), running(true), flush_thread(&AOFLogger::background_flush, this) {}
    
    ~AOFLogger() {
        running = false;
        cv.notify_one();
        flush_thread.join();
        flush_buffer_to_disk();
    }

    // 主线程调用：追加日志到缓冲区（COW技术）
    void append(const std::string& command) {
        std::lock_guard<std::mutex> lock(mutex);
        active_buffer.push_back(command + "\n");
        
        // 缓冲区达到阈值时触发后台刷盘
        if (active_buffer.size() >= 1024) {
            swap_buffers();
            cv.notify_one();
        }
    }

private:
    // 后台线程函数
    void background_flush() {
        while (running) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this] { return !running || !pending_buffer.empty(); });
            
            if (!pending_buffer.empty()) {
                std::vector<std::string> local_buffer;
                local_buffer.swap(pending_buffer); // COW：快速交换缓冲区
                lock.unlock();
                
                flush_buffer_to_disk(local_buffer);
            }
        }
    }

    // 交换缓冲区（原子操作）
    void swap_buffers() {
        std::vector<std::string> new_buffer;
        active_buffer.swap(new_buffer);
        pending_buffer.swap(new_buffer);
    }

    // 实际刷盘操作
    void flush_buffer_to_disk(const std::vector<std::string>& buffer) {
        std::ofstream file(filename, std::ios::app);
        for (const auto& cmd : buffer) {
            file << cmd;
        }
    }

    std::vector<std::string> active_buffer;  // 主线程写入的缓冲区
    std::vector<std::string> pending_buffer; // 后台线程刷盘的缓冲区
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::thread flush_thread;
    std::string filename;
};