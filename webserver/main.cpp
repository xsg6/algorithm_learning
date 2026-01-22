#include "thread_pool_webserver.hpp"
#include "logger.hpp"
#include <iostream>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// 全局变量保存服务器指针
SimpleWebServer* g_server = nullptr;

// 信号处理函数
void signalHandler(int signal) {
    Logger::getInstance().info("Received signal " + std::to_string(signal) + ", stopping server...");
    if (g_server) {
        g_server->stop();
    }
}

// 实现守护进程功能
bool daemonize() {
    // 创建子进程
    pid_t pid = fork();
    
    if (pid < 0) {
        Logger::getInstance().error("Failed to fork first child");
        return false;
    }
    
    // 父进程退出
    if (pid > 0) {
        exit(0);
    }
    
    // 设置新的会话
    if (setsid() < 0) {
        Logger::getInstance().error("Failed to create new session");
        return false;
    }
    
    // 再次创建子进程，确保不是会话组长
    pid = fork();
    
    if (pid < 0) {
        Logger::getInstance().error("Failed to fork second child");
        return false;
    }
    
    // 父进程退出
    if (pid > 0) {
        exit(0);
    }
    
    // 设置工作目录为根目录
    if (chdir("/") < 0) {
        Logger::getInstance().error("Failed to change working directory");
        return false;
    }
    
    // 关闭标准文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // 重新打开标准文件描述符到/dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    
    return true;
}

int main() {
    int port = 8080; // 服务器端口
     // 询问用户是否后台运行
    std::cout << "Do you want to run the server in background? (y/n): ";
    char choice;
    std::cin >> choice;
    bool is_daemon = (choice == 'y' || choice == 'Y');
    if (is_daemon) {
        std::cout << "Daemonizing process..." << std::endl;
        
        // 【第一步】先变身守护进程
        // 注意：这里不要先 init 日志，否则线程会丢失
        if (!daemonize()) {
            std::cerr << "Failed to daemonize server" << std::endl;
            return 1;
        }
        // --- 程序此刻已在后台运行，且工作目录被改为了 "/" ---
    }

    // 【第二步】变身成功后，再初始化日志系统
    if (is_daemon) {
        // 后台模式：必须用绝对路径！
        // 因为 daemonize() 里执行了 chdir("/")，如果写相对路径，会试图去根目录创建文件，导致权限不足失败。
        Logger::getInstance().init("/home/dministrator/myPro_c/webserver/build/webserver.log");
    } else {
        // 前台模式：可以使用相对路径（直接生成在当前运行目录下）
        Logger::getInstance().init("webserver.log");
        Logger::getInstance().info("Starting server in foreground mode on port " + std::to_string(port));
    }

    if (is_daemon) {
        Logger::getInstance().info("Starting server in background mode on port " + std::to_string(port));
    }
    
    // 创建服务器实例
    SimpleWebServer server(port);
    g_server = &server;
    
    // 注册信号处理函数，用于优雅地停止服务器
    struct sigaction sa;
    sa.sa_handler = signalHandler;  // 设置信号处理函数
    sigemptyset(&sa.sa_mask);       // 清空信号集，确保处理信号时不阻塞其他信号
    sa.sa_flags = 0;                // 无特殊标志
    
    // 注册SIGINT信号处理（Ctrl+C）
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        Logger::getInstance().error("Error registering SIGINT handler");
        return 1;
    }
    
    // 注册SIGTERM信号处理（终止信号）
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        Logger::getInstance().error("Error registering SIGTERM handler");
        return 1;
    }
    
    // 注册路由处理函数示例
    server.get("/", [](const HttpRequest& req, HttpResponse& res) {
        Logger::getInstance().info("Received GET request for /");
        res.status_code = 200;
        res.status_msg = "OK";
        res.body = "<html><body><h1>Welcome to Simple Web Server!</h1>"
                   "<p>Current Time: " + std::to_string(time(nullptr)) + "</p>"
                   "</body></html>";
    });
    
    server.get("/hello", [](const HttpRequest& req, HttpResponse& res) {
        Logger::getInstance().info("Received GET request for /hello");
        res.status_code = 200;
        res.status_msg = "OK";
        res.body = "<html><body><h1>Hello, World!</h1></body></html>";
    });
    
    server.post("/echo", [](const HttpRequest& req, HttpResponse& res) {
        Logger::getInstance().info("Received POST request for /echo");
        res.status_code = 200;
        res.status_msg = "OK";
        res.body = "<html><body><h1>Echo POST Data:</h1><pre>" + req.body + "</pre></body></html>";
    });
    
   
    // 启动服务器
    server.start();
    
    Logger::getInstance().info("Server stopped successfully");
    return 0;
}