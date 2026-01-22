#include "Socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>

// ===================== 普通构造：创建新 socket =====================
Socket::Socket(int type) : sockfd(-1), isValid(false) {
    sockfd = ::socket(AF_INET, type, 0);
    if (sockfd < 0) {
        return;
    }
    isValid = true;
    std::memset(&addr, 0, sizeof(addr));
}

// ===================== [MOD] 新增：包装已有 fd 的构造函数（accept 专用） =====================
// 为什么：修复你原 accept() 里 new Socket() 造成的“创建了多余fd却被覆盖”的泄漏
Socket::Socket(int existing_fd, const struct sockaddr_in& peer_addr)
    : sockfd(existing_fd), addr(peer_addr), isValid(existing_fd >= 0) {}

// ===================== [MOD] move 构造/赋值 =====================
Socket::Socket(Socket&& other) noexcept
    : sockfd(other.sockfd), addr(other.addr), isValid(other.isValid) {
    other.sockfd = -1;
    other.isValid = false;
    std::memset(&other.addr, 0, sizeof(other.addr));
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        sockfd = other.sockfd;
        addr = other.addr;
        isValid = other.isValid;

        other.sockfd = -1;
        other.isValid = false;
        std::memset(&other.addr, 0, sizeof(other.addr));
    }
    return *this;
}

// ===================== 析构：关闭 fd =====================
Socket::~Socket() {
    close();
}

bool Socket::setReuseAddr() {
    if (!isValid) return false;
    int reuse = 1;
    return ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == 0;
}

bool Socket::bind(const std::string& ip, int port) {
    if (!isValid) return false;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        isValid = false;
        return false;
    }
    return true;
}

bool Socket::listen(int backlog) {
    if (!isValid) return false;
    if (::listen(sockfd, backlog) < 0) {
        isValid = false;
        return false;
    }
    return true;
}

// ===================== [MOD] acceptUnique：RAII 版本 =====================
std::unique_ptr<Socket> Socket::acceptUnique() {
    if (!isValid) return nullptr;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // [MOD] 优先用 accept4 直接给新 fd 设置 NONBLOCK（更高效）
    int client_fd = ::accept4(sockfd, (struct sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
    if (client_fd < 0) {
        // 如果系统不支持 accept4，则退回 accept + 上层再 setNonBlocking
        if (errno == ENOSYS) {
            client_fd = ::accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) return nullptr;
        } else {
            return nullptr;
        }
    }

    // [MOD] 直接用“包装已有 fd 的构造函数”，避免 fd 泄漏
    return std::unique_ptr<Socket>(new Socket(client_fd, client_addr));
}

bool Socket::connect(const std::string& ip, int port) {
    if (!isValid) return false;

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        // [MOD] 非阻塞 connect 下 EINPROGRESS 是正常状态
        if (errno == EINPROGRESS) {
            return true;
        }
        isValid = false;
        return false;
    }
    return true;
}

// ===================== [MOD] send：不打印错误；交给上层处理 EAGAIN/EINTR =====================
ssize_t Socket::send(const void* data, size_t length) {
    if (!isValid || !data || length == 0) return -1;

    while (true) {
        ssize_t n = ::send(sockfd, data, length, 0);
        if (n >= 0) return n;
        if (errno == EINTR) continue; // [MOD] 被信号打断，重试
        return -1;                    // EAGAIN/EWOULDBLOCK 等由上层判断
    }
}

// ===================== [MOD] recv：不打印错误；交给上层处理 EAGAIN/EINTR =====================
ssize_t Socket::recv(void* buffer, size_t length) {
    if (!isValid || !buffer || length == 0) return -1;

    while (true) {
        ssize_t n = ::recv(sockfd, buffer, length, 0);
        if (n >= 0) return n;
        if (errno == EINTR) continue; // [MOD] 被信号打断，重试
        return -1;
    }
}

void Socket::close() {
    if (sockfd >= 0) {
        ::close(sockfd);
        sockfd = -1;
    }
    isValid = false;
}

int Socket::getFd() const {
    return sockfd;
}

bool Socket::is_valid() const {
    return isValid;
}

bool Socket::setNonBlocking() {
    if (!isValid) return false;
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool Socket::setTimeout(int seconds) {
    if (!isValid) return false;

    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) return false;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) return false;
    return true;
}
