#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>
#include <string>
#include <memory>     // [MOD] acceptUnique() 返回 unique_ptr
#include <utility>    // [MOD] move support

class Socket {
private:
    int sockfd;
    struct sockaddr_in addr;
    bool isValid;

    // [MOD] 新增：包装“已有fd”的构造函数（accept 返回的fd）
    // 为什么：修复你原 accept() 里 new Socket() 造成的“创建了多余fd却被覆盖”的泄漏
    explicit Socket(int existing_fd, const struct sockaddr_in& peer_addr);

public:
    // 构造函数：创建socket
    Socket(int type = SOCK_STREAM);

    // [MOD] 禁用拷贝，避免多个对象 close 同一个 fd
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    // [MOD] 支持移动（可选，但推荐）
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    // 析构函数：关闭socket
    ~Socket();

    bool setReuseAddr();
    bool bind(const std::string& ip, int port);
    bool listen(int backlog = 128);

    // [MOD] 新增：RAII 版本 accept（推荐在 Conn 用 unique_ptr 时使用）
    std::unique_ptr<Socket> acceptUnique();

    bool connect(const std::string& ip, int port);

    // 发送/接收数据
    // [MOD] Socket 层不打印错误日志；返回值和 errno 交给上层决定（epoll 模式下 EAGAIN 是正常现象）
    ssize_t send(const void* data, size_t length);
    ssize_t recv(void* buffer, size_t length);

    void close();

    int getFd() const;
    bool is_valid() const;

    bool setNonBlocking();
    bool setTimeout(int seconds);
};

#endif // SOCKET_HPP
