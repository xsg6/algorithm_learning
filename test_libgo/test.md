# cmakeList.txt的编写
## cmake版本
`cmake_minimum_required(VERSION 3.10)`
---
## 项目名称
``` c++
project(MyProject 
    VERSION 1.0.0          # 项目版本
    DESCRIPTION "A C++ Demo Project"  # 项目描述
    LANGUAGES CXX          # 支持的语言（CXX 表示 C++）
)
```
## c++语言标准 
```c++
set(CMAKE_CXX_STANDARD 17)          # 启用 C++17 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制要求指定的标准，不 fallback 到低版本
set(CMAKE_CXX_EXTENSIONS OFF)       # 禁用编译器特定扩展（如 g++ 的 -std=gnu++17）
```
## 添加源文件
***这里列出两种方法，1、自动搜索所有源文件，2、手动列出所有源文件***
*自动搜索*
```c++
AUX_SOURCE_DIRECTORY(./ SRC_LIST)       # 搜索当前目录（./）的源文件，存到 SRC_LIST
AUX_SOURCE_DIRECTORY(./common COMMON_LIST)
```
*手动列出*
```c++
set(BASE_SRCS
    base/logger.cpp
    base/config.cpp
    base/thread_pool.cpp
)
#主程序入口
set(MAIN_SRCS
    main.cpp
)
# 汇总所有源文件
set(ALL_SRCS
    ${BASE_SRCS}
    ${NETWORK_SRCS}
    ${STORAGE_SRCS}
    ${MAIN_SRCS}
)
```
---
## 生成文件
*生成可执行文件*
``` c++
# 将 SOURCES 中的源文件编译为名为 MyApp 的可执行文件
add_executable(MyApp ${SOURCES})
```
*生成库文件*
***库文件类似于将通用函数/工程封装，提供给所有人调用，并隐藏代码细节（cpp文件）***
*静态库启动快，不依赖环境，占用内存较大，各项目相互独立不共享，需要重复链接*
*动态库启动稍慢，因为要加载，占用内存较小，各项目相互共享，仅需链接一次*
```c++
# 生成静态库（.a 或 .lib）
add_library(MyLib STATIC src/lib/utils.cpp)

# 生成动态库（.so 或 .dll）
add_library(MyLib SHARED src/lib/utils.cpp)
```
## 指定头文件目录
*核心逻辑是根据目标来指定头文件目录，目标就是主程序/库*
```c++
告诉编译器去哪里找头文件（.h），避免include 报错：
cmake
# 为目标 MyApp 指定头文件目录（PUBLIC 表示依赖该目标的其他模块也能使用这些头文件）
target_include_directories(MyApp
    PUBLIC           # 公开头文件目录（如供外部调用的接口）
        /usr/include/fastdfs  # 绝对路径（系统或全局安装的库头文件）
        ./jsoncpp             # 相对路径（项目内的 jsoncpp 头文件目录）
        ./mysql               # 项目内其他目录
    PRIVATE           #私有
)
```
## 链接依赖库
``` c++
如果项目依赖其他库（系统库、第三方库或自定义库），需指定链接规则：
（1）链接系统库或已安装的库
cmake
# 链接 pthread 线程库（Linux 下）
target_link_libraries(MyApp PRIVATE pthread)

# 链接数学库（-lm）
target_link_libraries(MyApp PRIVATE m)
（2）链接项目内的其他库（如上面生成的 MyLib）
cmake
# 链接自定义静态库 MyLib 到可执行文件 MyApp
target_link_libraries(myapp PRIVATE ./lib/liblogger.a)

# 方式 2：绝对路径（更推荐，通过 CMake 变量获取项目根目录）
target_link_libraries(myapp PRIVATE ${CMAKE_SOURCE_DIR}/lib/liblogger.a)
（3）链接第三方库（如 OpenCV、Boost）
通常需要先通过 find_package 找到库，再链接：
cmake
# 查找 OpenCV 库
find_package(OpenCV REQUIRED)

# 链接 OpenCV 到目标
target_link_libraries(MyApp PRIVATE ${OpenCV_LIBS})
```
## 编译选项
```c++
（1）添加编译器选项
cmake
# 为目标 MyApp 添加编译选项（-Wall 开启所有警告，-Werror 视警告为错误）
target_compile_options(MyApp PRIVATE -Wall -Werror)

# 调试模式下添加 -g 选项（生成调试信息）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(MyApp PRIVATE -g)
endif()
（2）添加预定义宏
cmake
# 定义宏 DEBUG（代码中可通过 #ifdef DEBUG 判断）
target_compile_definitions(MyApp PRIVATE DEBUG)

# 传递 CMake 变量到代码中（如项目版本号）
target_compile_definitions(MyApp PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
```
## 安装规则(可选)

## 子目录管理
```c++
如果项目包含多个子模块（如 src/、test/），可通过 add_subdirectory 引入子目录的 CMakeLists.txt：
cmake
# 包含 test 目录（该目录下需有自己的 CMakeLists.txt）
add_subdirectory(test)
```