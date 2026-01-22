#include "thread_pool_webserver.hpp"
#include "Socket.hpp"
#include "../thread_learning/simple_thread_pool.hpp"
#include "logger.hpp"
#include <sys/uio.h> 
#include <iostream>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>

// ===================== [MOD] 工具：设置 fd 为非阻塞 =====================
// 为什么要非阻塞：配合 epoll 才能高效处理大量连接
// 注意：非阻塞 + EPOLLET 必须读/写到 EAGAIN（下面会实现）
static bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;//O_NONBLOCK是非阻塞标
}

// 构造函数
SimpleWebServer::SimpleWebServer(int port)
    : m_port(port), m_server_socket(nullptr), m_epoll_fd(-1), m_running(false) {
    Logger::getInstance().debug("WebServer constructor called with port " + std::to_string(port));
}

// 析构函数
SimpleWebServer::~SimpleWebServer() {
    Logger::getInstance().debug("WebServer destructor called");
    stop();
}

// ===================== 启动服务器 =====================
void SimpleWebServer::start() {
    if (!initializeServerSocket()) {
        Logger::getInstance().error("Failed to initialize server socket");
        return;
    }
    if (!initializeEpoll()) {
        Logger::getInstance().error("Failed to initialize epoll");
        return;
    }

    Logger::getInstance().info("Web server started on port " + std::to_string(m_port));
    m_running = true;

    auto& threadPool = SimpleThreadPool::getInstance();
    auto* events = new epoll_event[MAX_EVENTS];//epoll_event存事件类型和自定义数据,数组可以一下返回多个就绪队列，event是输出缓冲区

    while (m_running) {
        //获取过期时间
        int timeout=m_timer.getNextTick();
        //没任务
        if(timeout==-1) timeout=1000;
        int nfds = epollWait( events, timeout);
        // 【新增】处理完 IO 事件后，立刻检查是否有超时事件
        // 这一步会执行所有过期的回调，关闭那些僵尸连接
        m_timer.tick();
        if (nfds == -1) break;
        processEvents(events, nfds, threadPool);
    }

    delete[] events;
    cleanup();
    std::cout << "Web server stopped" << std::endl;
}

// ===================== 初始化服务器 Socket =====================
bool SimpleWebServer::initializeServerSocket() {
    m_server_socket = new Socket();
    if (!m_server_socket->is_valid()) {
        Logger::getInstance().error("Error creating socket");
        delete m_server_socket;
        m_server_socket = nullptr;
        return false;
    }

    if (!m_server_socket->setReuseAddr()) {
        Logger::getInstance().error("Error setting socket options");
        delete m_server_socket;
        m_server_socket = nullptr;
        return false;
    }

    if (!m_server_socket->bind("0.0.0.0", m_port)) {
        Logger::getInstance().error("Error binding socket");
        delete m_server_socket;
        m_server_socket = nullptr;
        return false;
    }

    if (!m_server_socket->listen(128)) {
        Logger::getInstance().error("Error listening");
        delete m_server_socket;
        m_server_socket = nullptr;
        return false;
    }

    Logger::getInstance().debug("Server socket initialized successfully");
    return true;
}

// ===================== 初始化 epoll =====================
bool SimpleWebServer::initializeEpoll() {
    m_epoll_fd = epoll_create1(0);
    if (m_epoll_fd == -1) {
        Logger::getInstance().error("Error creating epoll");
        delete m_server_socket;
        m_server_socket = nullptr;
        return false;
    }

    // [MOD] server socket 用 data.fd（本来你就这么做了）
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = m_server_socket->getFd();

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server_socket->getFd(), &event) == -1) {
        Logger::getInstance().error("Error adding server socket to epoll");
        close(m_epoll_fd);
        delete m_server_socket;
        m_server_socket = nullptr;
        return false;
    }

    Logger::getInstance().debug("Epoll initialized successfully");
    return true;
}

// ===================== epoll_wait =====================
int SimpleWebServer::epollWait(struct epoll_event* events,int timeout) {
    // 1000ms 超时，避免永久阻塞，方便 stop()
    int nfds = epoll_wait(m_epoll_fd, events, MAX_EVENTS, timeout);
    if (nfds == -1 && m_running) {
        Logger::getInstance().error("Error in epoll_wait");
    }
    return nfds;
}

// ===================== 事件分发 =====================
bool SimpleWebServer::isServerSocketEvent(const struct epoll_event& event) {
    return event.data.fd == m_server_socket->getFd();
}

void SimpleWebServer::processEvents(struct epoll_event* events, int nfds, SimpleThreadPool& threadPool) {
    for (int i = 0; i < nfds; i++) {
        if (isServerSocketEvent(events[i])) {
            handleNewConnection(threadPool);//服务器socket就是有新连接
        } else {
            handleClientEvent(events[i], threadPool);//客户端socket就是i/o
        }
    }
}

// ===================== [MOD] Conn 表：安全管理连接对象生命周期 =====================
std::shared_ptr<SimpleWebServer::Conn> SimpleWebServer::getConn(int fd) {
    std::lock_guard<std::mutex> lk(m_conns_mtx);
    auto it = m_conns.find(fd);
    if (it == m_conns.end()) return nullptr;
    return it->second;
}

void SimpleWebServer::addConn(const std::shared_ptr<Conn>& c) {
    std::lock_guard<std::mutex> lk(m_conns_mtx);
    m_conns[c->fd] = c;
}

void SimpleWebServer::eraseConn(int fd) {
    std::lock_guard<std::mutex> lk(m_conns_mtx);
    m_conns.erase(fd);
}

// ===================== 接受新连接 =====================
void SimpleWebServer::handleNewConnection(SimpleThreadPool&) {
    std::unique_ptr<Socket> client_socket = m_server_socket->acceptUnique();
    if (!client_socket) {
        Logger::getInstance().warning("Failed to accept new connection");
        return;
    }

    int fd = client_socket->getFd();

    // [MOD] 必须非阻塞（配合 epoll）
    if (!set_nonblocking(fd)) {
        Logger::getInstance().warning("Failed to set nonblocking for client fd=" + std::to_string(fd));
        // delete client_socket;
        return;
    }

    // [MOD] 创建 Conn，并放入 Conn 表
    // 为什么：需要保存 inbuf/outbuf/状态，且避免 epoll 存裸指针造成 UAF
    auto conn = std::make_shared<Conn>();
    conn->fd = fd;
    conn->sock=std::move(client_socket); // Conn 托管 Socket 生命周期

    

    // [MOD] 加入 epoll：事件里只带 fd，不带 ptr
    // [MOD] 增加 EPOLLONESHOT：保证同一 fd 同一时刻只会有一个 worker 在处理
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ev.data.fd = fd;

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        Logger::getInstance().error("Error adding client fd to epoll fd=" + std::to_string(fd));
        return;
    }
    addConn(conn);
    // 【新增】添加定时器
    // 回调函数：调用 closeConnection 关闭这个 fd
    m_timer.add(fd, TIMEOUT_MS, [this, fd]() {
        Logger::getInstance().info("Connection timeout, closing fd=" + std::to_string(fd));
        this->closeConnection(fd);
    });
    
    Logger::getInstance().debug("New connection accepted fd=" + std::to_string(fd));
}

// ===================== [MOD] 统一关闭连接（必须先 DEL 再 close 再 erase） =====================
void SimpleWebServer::closeConnection(int fd) {
    if (m_epoll_fd != -1) {
        epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    }
    ::close(fd);//使用全局close，因为这里自定义了close函数，这里用的linux内核close，用于关闭fd
    eraseConn(fd);
}

// ===================== [MOD] re-arm ONESHOT（worker 处理完后再恢复监听） =====================
void SimpleWebServer::rearm(int fd, uint32_t events) {
    epoll_event ev;
    ev.events = events | EPOLLONESHOT; // 关键：重新武装 ONESHOT
    ev.data.fd = fd;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

// ===================== 客户端事件：只拿 fd，交给线程池 =====================
void SimpleWebServer::handleClientEvent(const struct epoll_event& event, SimpleThreadPool& threadPool) {
    // [MOD] 不再 event.data.ptr -> Socket*
    // 改为：event.data.fd -> fd，再通过 Conn 表查 Conn
    int fd = event.data.fd;
    uint32_t ev = event.events;//用unit32_t的原因是epoll底层就是这个

    threadPool.submit([this, fd, ev]() {
        handle_io(fd, ev);
    });
}

// ===================== [MOD] 非阻塞读取：循环读到 EAGAIN =====================
// 为什么：EPOLLET(边缘触发) 下如果不读空缓冲，可能不再触发下一次事件，导致“卡死”
bool SimpleWebServer::readToInbuf(const std::shared_ptr<Conn>& c) {
    char extrabuf[65536];
    // [修正] 不要在这里初始化 vec，这里初始化会导致后面循环用旧指针
    
    while (true) {
        // [修正] 每次循环都要重新计算剩余空间和最新的写指针位置
        const size_t writable = c->inbuf.writableBytes();
        
        struct iovec vec[2];
        vec[0].iov_base = c->inbuf.beginWrite(); // 获取最新的写位置
        vec[0].iov_len = writable;
        vec[1].iov_base = extrabuf;
        vec[1].iov_len = sizeof(extrabuf);
        
        // 只有当 writable 小于 extrabuf 大小时，才需要启用第二块内存
        // 这里的逻辑可以保留，或者简单点总是传 2 也可以（如果 vec[0] 很大 vec[1] 就不会被用到）
        const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
        
        ssize_t n = ::readv(c->fd, vec, iovcnt);
        
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return true; 
            return false;
        }
        if (n == 0) {
            return false;
        }
        
        // n > 0，处理数据
        if (static_cast<size_t>(n) <= writable) {
            c->inbuf.hasWritten(n); 
        } else {
            c->inbuf.hasWritten(writable); 
            c->inbuf.append(extrabuf, n - writable);
        }
        // 循环继续，进入下一次 readv，此时 inbuf.beginWrite() 已经变了，逻辑正确
    }
}

// ===================== [MOD] 非阻塞写：循环写到 EAGAIN / 写完 =====================
// 为什么：send 可能部分写，或者 EAGAIN（内核发送缓冲满）；不处理会导致响应截断（文件下载必崩）
bool SimpleWebServer::writeFromOutbuf(const std::shared_ptr<Conn>& c) {
    while (c->outbuf.readableBytes() > 0) {
        ssize_t n = ::send(c->fd, c->outbuf.peek(), c->outbuf.readableBytes(), 0);

        if (n > 0) {
            c->outbuf.retrieve(n); // O(1) 移动读指针
            if (c->outbuf.readableBytes() == 0) {
                return true; // 发送完毕
            }
            // 没发完继续发
        } 
        else if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return true; // 内核缓冲满，等待下次 EPOLLOUT
            }
            if (errno == EINTR) {
                continue; // 信号中断，重试
            }
            // 真实错误
            Logger::getInstance().error("Send error fd=" + std::to_string(c->fd));
            return false;
        } 
        else {
            // n == 0，视为异常断开
            return false;
        }
    }
    return true;
}
// ===================== [MOD] HTTP 解析：从 inbuf 中拆出一个完整请求 =====================
// 核心目标：支持半包/粘包/keep-alive
// 规则：
// 1) 先找 "\r\n\r\n" 确认 header 完整
// 2) 解析 Content-Length（不支持 chunked）
// 3) 等待 inbuf 中 body 字节达到 Content-Length
// 4) 消费掉这个请求的数据（inbuf erase），返回 true
// ===================== [FIXED] HTTP 解析 =====================
// ===================== [FIXED] HTTP 解析逻辑 =====================
bool SimpleWebServer::tryParseOneRequest(const std::shared_ptr<Conn>& c, HttpRequest& req) {
    const char* buf = c->inbuf.peek();
    size_t len = c->inbuf.readableBytes();
    
    // 1. 查找 HTTP 头部结束标记 \r\n\r\n
    const char* crlf = "\r\n\r\n";
    auto it = std::search(buf, buf + len, crlf, crlf + 4);
    
    // 如果没找到完整的头，说明数据还没收全，返回 false 等待更多数据
    if (it == buf + len) return false; 
    
    size_t header_len = (it - buf) + 4;
    
    // 2. 解析 Header (之前这里你是空的，必须补上！)
    std::string header_part(buf, header_len); 
    std::istringstream iss(header_part);
    std::string line;
    
    // A. 解析请求行: GET / HTTP/1.1
    if (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream line_ss(line);
        line_ss >> req.method >> req.path >> req.version;
    }

    // B. 解析 Headers
    while (std::getline(iss, line) && line != "\r") {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            // 去除 value 前导空格
            size_t first_not_space = val.find_first_not_of(' ');
            if (first_not_space != std::string::npos) {
                val = val.substr(first_not_space);
            }
            req.headers[key] = val;
        }
    }
    
    // 3. 解析 Content-Length (为了处理 Post 请求 Body)
    size_t body_len = 0;
    auto cl_it = req.headers.find("Content-Length");
    if (cl_it != req.headers.end()) {
        try {
            body_len = std::stoul(cl_it->second);
        } catch (...) {
            body_len = 0;
        }
    }
    
    // 4. 检查 Body 是否完整
    // 如果 buffer 总长度 < 头长度 +体长度，说明 Body 还没收全
    if (len < header_len + body_len) return false; 
    
    // 5. 提取 Body
    req.body.assign(buf + header_len, body_len);
    
    // 6. [关键] 从 Buffer 中彻底移除这个请求的数据（移动读指针）
    c->inbuf.retrieve(header_len + body_len);
    
    return true;
}

// ===================== keep-alive 逻辑（复用你原意，但更安全） =====================
bool SimpleWebServer::shouldKeepAlive(const HttpRequest& request) {
    auto conn_it = request.headers.find("Connection");
    if (conn_it != request.headers.end()) {
        std::string v = conn_it->second;
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        if (v == "close") return false;
        if (v == "keep-alive") return true;
    }
    // HTTP/1.1 默认 keep-alive；HTTP/1.0 默认 close
    return request.version != "HTTP/1.0";
}

void SimpleWebServer::setConnectionHeader(HttpResponse& response, bool keep_alive) {
    response.headers["Connection"] = keep_alive ? "keep-alive" : "close";
}

// ===================== 构建响应（保留你原路由机制） =====================
void SimpleWebServer::build_response(const HttpRequest& request, HttpResponse& response) {
    response.version = "HTTP/1.1";
    response.headers["Server"] = "SimpleWebServer/1.0";
    response.headers["Content-Type"] = "text/html; charset=utf-8";

    HandlerFunc handler = findRouteHandler(request);
    if (handler) {
        handler(request, response);
    } else {
        buildNotFoundResponse(response);
    }

    // [MOD] Content-Length 必须准确（对 keep-alive 很关键）
    response.headers["Content-Length"] = std::to_string(response.body.size());
}

SimpleWebServer::HandlerFunc SimpleWebServer::findRouteHandler(const HttpRequest& request) {
    if (request.method == "GET") {
        auto it = m_get_routes.find(request.path);
        if (it != m_get_routes.end()) return it->second;
    } else if (request.method == "POST") {
        auto it = m_post_routes.find(request.path);
        if (it != m_post_routes.end()) return it->second;
    }
    auto it = m_any_routes.find(request.path);
    if (it != m_any_routes.end()) return it->second;
    return nullptr;
}

void SimpleWebServer::buildNotFoundResponse(HttpResponse& response) {
    response.status_code = 404;
    response.status_msg = "Not Found";
    response.body = "<html><body><h1>404 Not Found</h1></body></html>";
}

// ===================== [MOD] 响应组包：append 到 outbuf（不直接 send） =====================
// 为什么：非阻塞下 send 可能部分写/EAGAIN，必须先放到 outbuf，再由 writeFromOutbuf() 可靠发送
void SimpleWebServer::append_response(const std::shared_ptr<Conn>& c, const HttpResponse& response) {
    std::ostringstream oss;
    oss << response.version << " " << response.status_code << " " << response.status_msg << "\r\n";
    for (const auto& [k, v] : response.headers) {
        oss << k << ": " << v << "\r\n";
    }
    oss << "\r\n";
    
    std::string header = oss.str();
    
    // 直接 append 到 buffer
    c->outbuf.append(header);
    c->outbuf.append(response.body);
}

void SimpleWebServer::sendBadRequest(const std::shared_ptr<Conn>& c) {
    HttpResponse r;
    r.version = "HTTP/1.1";
    r.status_code = 400;
    r.status_msg = "Bad Request";
    r.headers["Content-Type"] = "text/plain";
    r.body = "400 Bad Request";
    r.headers["Content-Length"] = std::to_string(r.body.size());
    r.headers["Connection"] = "close";
    c->want_close = true;
    append_response(c, r);
}

// ===================== [MOD] worker 入口：一次事件尽可能读/解析/写，然后 rearm =====================
// 关键设计：
// - EPOLLONESHOT 保证同 fd 不会同时进入两个 worker（避免竞态）
// - EPOLLET 下读/写都要循环到 EAGAIN
// - 不在 worker 里 sleep 不忙等：下一次请求靠 epoll 触发
// ===================== [FIXED] worker 入口 =====================
void SimpleWebServer::handle_io(int fd, uint32_t events) {
    auto c = getConn(fd);
    if (!c) return;

    if (events & (EPOLLERR | EPOLLHUP)) {
        closeConnection(fd);
        return;
    }
    // 【新增】只要有 IO 事件，就延长定时器
    if (m_timer.getNextTick() > 0) { // 简单检查一下避免无效调用
        m_timer.adjust(fd, TIMEOUT_MS);
    }
    // ----------------- [修复] 读事件：真正执行读取和解析 -----------------
    if (events & EPOLLIN) {
        // 1. 尝试把数据从内核读到 Buffer
        bool readAck = readToInbuf(c);
        
        // 如果 readToInbuf 返回 true (EAGAIN 或 读到数据)，继续处理
        // 如果返回 false (对端关闭或出错)，直接断开
        if (!readAck) {
            closeConnection(fd);
            return;
        }

        // 2. 循环处理 Buffer 中的请求（处理粘包/Pipeline）
        HttpRequest req;
        while (tryParseOneRequest(c, req)) {
            HttpResponse res;
            
            // 保持连接逻辑
            bool keep_alive = shouldKeepAlive(req);
            
            // 业务处理
            build_response(req, res);
            
            // 设置 Connection 头
            setConnectionHeader(res, keep_alive);
            if (!keep_alive) c->want_close = true;

            // 追加到写缓冲区
            append_response(c, res);
            
            // 重置 req 以便下一次循环使用
            req = HttpRequest();
        }
    }

    // ----------------- 写事件 / 或者 outbuf 有积压数据就尝试写 -----------------
    if ((events & EPOLLOUT) || c->outbuf.readableBytes() > 0) {
        if (!writeFromOutbuf(c)) {
            closeConnection(fd);
            return;
        }
    }

    // ----------------- 优雅关闭逻辑 -----------------
    if (c->want_close && c->outbuf.readableBytes() == 0) {
        closeConnection(fd);
        return;
    }

    // ----------------- rearm ONESHOT -----------------
    if (c->outbuf.readableBytes() > 0) {
        rearm(fd, EPOLLOUT | EPOLLET);
    } else {
        rearm(fd, EPOLLIN | EPOLLET);
    }
}

// ===================== 路由注册 =====================
void SimpleWebServer::get(const std::string& path, HandlerFunc handler) { m_get_routes[path] = handler; }
void SimpleWebServer::post(const std::string& path, HandlerFunc handler) { m_post_routes[path] = handler; }
void SimpleWebServer::any(const std::string& path, HandlerFunc handler) { m_any_routes[path] = handler; }

// ===================== 清理资源 =====================
void SimpleWebServer::cleanup() {
    if (m_epoll_fd != -1) {
        close(m_epoll_fd);
        m_epoll_fd = -1;
    }

    if (m_server_socket) {
        delete m_server_socket;
        m_server_socket = nullptr;
    }

    // [MOD] 关闭所有活跃连接（Conn 表）
    {
        std::lock_guard<std::mutex> lk(m_conns_mtx);
        for (auto& [fd, _] : m_conns) {
            ::close(fd);
        }
        m_conns.clear();
    }
}

void SimpleWebServer::stop() {
    m_running = false;
    if (m_server_socket != nullptr) {
        m_server_socket->close();
    }
    if (m_epoll_fd != -1) {
        close(m_epoll_fd);
        m_epoll_fd = -1;
    }
}

// ===================== 状态码到消息 =====================
std::string SimpleWebServer::status_code_to_message(int code) {
    static std::unordered_map<int, std::string> messages = {
        {200, "OK"},
        {400, "Bad Request"},
        {404, "Not Found"},
        {500, "Internal Server Error"}
    };
    auto it = messages.find(code);
    if (it != messages.end()) return it->second;
    return "Unknown Status";
}