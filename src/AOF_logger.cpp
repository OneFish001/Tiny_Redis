#include "../include/AOF_logger.h"

#include <iostream>
#include <fstream>
#include <algorithm>

AOFLogger::AOFLogger(const std::string& filename)
    : filename(filename),running(true){
        // 启动后台线程
        flush_thread = std::thread(&AOFLogger::background_flush, this);
}


AOFLogger::~AOFLogger() {
    // 停止后台线程
    running = false;
    cv.notify_one();
    flush_thread.join();

    // 确保所有数据落盘
    std::unique_lock<std::mutex> lock(mutex);
    if (!active_buffer.empty()) {
        flush_buffer_to_disk(active_buffer);
    }
    if (!pending_buffer.empty()) {
        flush_buffer_to_disk(pending_buffer);
    }
}

void AOFLogger::append(const std::string& command) {
    std::lock_guard<std::mutex> lock(mutex);
    active_buffer.push_back(command + "\n");
    std::cout<<"append:"<<command<<std::endl;
    // 触发异步刷盘的条件（缓冲区大小或关键操作）
    if (active_buffer.size() >= 1) {
        swap_buffers();
        cv.notify_one();
    }
}

// 原子交换缓冲区
void AOFLogger::swap_buffers() {
    active_buffer.swap(pending_buffer);
    active_buffer.clear();
    active_buffer.reserve(1024); // 预分配空间提升性能
}

// 后台线程主循环
void AOFLogger::background_flush() {
    while (running) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { 
            return !running || !pending_buffer.empty(); 
        });

        // 交换缓冲区
        std::vector<std::string> local_buffer;
        if (!pending_buffer.empty()) {
            
            local_buffer.swap(pending_buffer);
        }
        lock.unlock();

        // 实际刷盘操作
        if (!local_buffer.empty()) {
            flush_buffer_to_disk(local_buffer);
        }
    }
}



// 保证写入的原子性
void AOFLogger::flush_buffer_to_disk(const std::vector<std::string>& buffer) {
    std::cout<<"flush_buffer_to_disk"<<std::endl;

    std::cout<<"打开文件："<<filename<<std::endl;
    std::ofstream file(filename, std::ios::app);
    std::cout<<"已创建："<<filename<<std::endl;
    if (file.is_open()) {
        for (const auto& cmd : buffer) {
            file << cmd;
            file.flush(); // 确保每条命令都fsync
        }
    } else {
        // 处理文件打开失败（可根据需要添加重试逻辑）
        std::cerr << "无法打开AOF文件: " << filename << std::endl;
    }
}