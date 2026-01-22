#ifndef SIMPLE_WEBSERVER_HPP
#define SIMPLE_WEBSERVER_HPP

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>          // [MOD] Conn 表用 shared_ptr/unique_ptr
#include <mutex>           // [MOD] Conn 表多线程访问需要互斥锁
#include <sys/epoll.h>
#include <algorithm>
#include <sstream>
#include"Buffer.hpp"
#include "heapTimer.hpp"
// ===================== HTTP请求结构体 =====================
// [KEEP] 保留你的定义。注意：std::string 可以存二进制（含 '\0'），前提是你必须按长度处理，不能用 C 字符串逻辑。
typedef struct {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
} HttpRequest;

// ===================== HTTP响应结构体 =====================
typedef struct {
    std::string version;
    int status_code;
    std::string status_msg;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
} HttpResponse;

// Forward declaration
class Socket;
class SimpleThreadPool;

// ===================== Web服务器类 =====================
class SimpleWebServer {
public:
    SimpleWebServer(int port = 8080);
    ~SimpleWebServer();

    void start();
    void stop();

    using HandlerFunc = std::function<void(const HttpRequest&, HttpResponse&)>;

    void get(const std::string& path, HandlerFunc handler);
    void post(const std::string& path, HandlerFunc handler);
    void any(const std::string& path, HandlerFunc handler);

private:
    // ===================== 网络相关 =====================
    int m_port;
    Socket* m_server_socket;
    int m_epoll_fd;
    static const int MAX_EVENTS = 1000;
    bool m_running;
    heapTimer m_timer; // 定时器管理器
    static const int TIMEOUT_MS = 60000; // 默认超时时间 60秒（Keep-Alive）
    // ===================== 路由表 =====================
    std::unordered_map<std::string, HandlerFunc> m_get_routes;
    std::unordered_map<std::string, HandlerFunc> m_post_routes;
    std::unordered_map<std::string, HandlerFunc> m_any_routes;

    // =====================================================================
    // [MOD] 新增：Conn 连接上下文 + Conn 表（fd -> Conn）
    //
    // 为什么要加：
    // 1) epoll_event 建议只存 fd（event.data.fd），避免存 Socket* 裸指针导致 UAF
    // 2) HTTP 是流式协议，会发生半包/粘包/keep-alive 多请求，必须有 inbuf/outbuf 保存状态
    // 3) 非阻塞 send 可能部分写，需要 outbuf 支持“未发送完继续发”
    // =====================================================================
    struct Conn {
        int fd = -1;                             // [MOD] 连接 fd
        std::unique_ptr<Socket> sock;            // [MOD] Conn 托管 Socket 生命周期（避免裸指针）
        Buffer inbuf;                       // [MOD] 读缓冲区：半包/粘包/keep-alive 需要
        Buffer outbuf;                      // [MOD] 写缓冲区：部分写/大响应/文件传输需要
        bool want_close = false;                 // [MOD] 标记是否需要关闭连接

    };

    std::unordered_map<int, std::shared_ptr<Conn>> m_conns; // [MOD] Conn 表：活跃连接目录
    std::mutex m_conns_mtx;                                  // [MOD] 多线程访问保护

    // ===================== 你原来的“阻塞式处理一个连接”接口（建议删除/不再使用） =====================
    // [MOD] 这些函数是“accept 后把 Socket* 交给 worker 然后 while 循环读写”的风格。
    // 在 fd+Conn表 + EPOLLONESHOT 的事件驱动模型中，这些将被 handle_io/tryParseOneRequest/append_response 替代。
    // 你可以先保留声明以便逐步迁移，但新实现里不应该再调用它们。
    //
    // void handle_connection(Socket* client_socket);              // [MOD] 不再使用（事件驱动替代）
    // bool parse_request(Socket* client_socket, HttpRequest& request); // [MOD] 不再使用（从 Conn.inbuf 解析替代）
    // void send_response(Socket* client_socket, const HttpResponse& response); // [MOD] 不再使用（append 到 outbuf 替代）
    // void cleanupClientSocket(Socket* client_socket);            // [MOD] 不再使用（closeConnection(fd) 替代）
    // void parseRequestBody(Socket* client_socket, std::istringstream& iss, HttpRequest& request); // [MOD] 不再使用
    // bool parseHeaders(std::istringstream& iss, std::unordered_map<std::string, std::string>& headers); // [MOD] 不再使用
    // bool isValidClientSocket(Socket* client_socket);            // [MOD] 不再使用

    // ===================== 初始化/事件循环相关（保留 + 小调整） =====================
    bool initializeServerSocket();
    bool initializeEpoll();
    int epollWait(struct epoll_event* events,int timeout);
    bool isServerSocketEvent(const struct epoll_event& event);
    void processEvents(struct epoll_event* events, int nfds, SimpleThreadPool& threadPool);
    void handleNewConnection(SimpleThreadPool& threadPool);

    // =====================================================================
    // [MOD] 修改：addToEpollAndSubmitTask / handleClientEvent 只用 fd，不再用 Socket*
    //
    // 为什么：
    // 1) epoll_event.data.ptr 指向 Socket* 容易悬空
    // 2) fd 作为稳定标识符，配合 Conn 表能安全拿到连接上下文
    // =====================================================================
    void handleClientEvent(const struct epoll_event& event, SimpleThreadPool& threadPool); // [MOD] event.data.fd

    void cleanup();

    // ===================== [MOD] Conn 表操作 =====================
    std::shared_ptr<Conn> getConn(int fd);        // [MOD] fd -> Conn
    void addConn(const std::shared_ptr<Conn>& c); // [MOD] 插入 Conn 表
    void eraseConn(int fd);                       // [MOD] 从 Conn 表删除

    // ===================== [MOD] EPOLLONESHOT：防止同 fd 并发处理 =====================
    void rearm(int fd, uint32_t events);          // [MOD] worker 处理完后重新监听
    void closeConnection(int fd);                 // [MOD] 统一关闭：DEL + close + erase

    // ===================== [MOD] 线程池 worker 入口：处理一次事件（读/解析/写） =====================
    void handle_io(int fd, uint32_t events);      // [MOD] 新的核心处理函数

    // ===================== [MOD] 非阻塞读写：循环到 EAGAIN =====================
    bool readToInbuf(const std::shared_ptr<Conn>& c);      // [MOD] 读到 inbuf
    bool writeFromOutbuf(const std::shared_ptr<Conn>& c);  // [MOD] 写 outbuf

    // ===================== [MOD] HTTP 解析：从 Conn.inbuf 拆出完整请求 =====================
    bool tryParseOneRequest(const std::shared_ptr<Conn>& c, HttpRequest& req); // [MOD]

    // ===================== 业务构建响应（保留你原来的路由机制） =====================
    void build_response(const HttpRequest& request, HttpResponse& response);
    std::string status_code_to_message(int code);
    SimpleWebServer::HandlerFunc findRouteHandler(const HttpRequest& request);
    void buildNotFoundResponse(HttpResponse& response);

    // ===================== [MOD] 响应发送：append 到 outbuf，不直接 send =====================
    void append_response(const std::shared_ptr<Conn>& c, const HttpResponse& response); // [MOD]
    void sendBadRequest(const std::shared_ptr<Conn>& c);                                // [MOD]

    // ===================== keep-alive 逻辑（仍然需要） =====================
    bool shouldKeepAlive(const HttpRequest& request);
    void setConnectionHeader(HttpResponse& response, bool keep_alive);
};

#endif // SIMPLE_WEBSERVER_HPP