# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(Tiny_Redis)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

include_directories(include)


# 定义可执行文件
# add_executable(server src/server.cpp)
# add_executable(client src/client.cpp)
# add_executable(aof_logger src/AOF_logger.cpp)


add_executable(server 
    src/server.cpp
    src/AOF_logger.cpp  # 添加这行
)

add_executable(client src/client.cpp)