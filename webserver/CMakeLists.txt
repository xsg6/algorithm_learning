cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找线程库
find_package(Threads REQUIRED)

# 定义源文件
set(SOURCES
    main.cpp
    thread_pool_webserver.cpp
    Socket.cpp
    # 假设你的线程池文件路径如下，请根据实际情况调整
    ../thread_learning/simple_thread_pool.cpp
    logger.cpp
)

# 定义头文件目录
include_directories(
    .
    ..
    ../thread_learning
)

# 创建可执行文件
add_executable(webserver ${SOURCES})

# 链接线程库
target_link_libraries(webserver 
    Threads::Threads
)

# 添加编译选项
target_compile_options(webserver PRIVATE -Wall -Wextra -pthread)