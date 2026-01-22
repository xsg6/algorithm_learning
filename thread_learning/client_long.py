#!/usr/bin/env python3
import socket
import threading
import time
import re

# 配置参数
SERVER_IP = "127.0.0.1"
SERVER_PORT = 8080
REQUEST_COUNT = 50000   # 总请求数 (建议调大一点，因为长连接很快)
CONCURRENCY = 50        # 并发线程数
TIMEOUT = 5             # Socket 超时

# 全局统计
lock = threading.Lock()
total = 0
ok = 0
fail = 0
lat_sum = 0.0

# 预编译正则，用于提取 Content-Length
CL_PATTERN = re.compile(b'Content-Length:\s*(\d+)', re.IGNORECASE)

def read_response(sock):
    """
    解析 HTTP 响应，确保读取完整的 Response。
    长连接必须基于 Content-Length 判断边界。
    """
    response_buffer = b""
    content_length = -1
    header_end_idx = -1
    
    while True:
        # 1. 读取数据
        chunk = sock.recv(4096)
        if not chunk:
            raise ConnectionError("Server closed connection")
        
        response_buffer += chunk
        
        # 2. 如果还没找到 Header 结束符，尝试查找
        if header_end_idx == -1:
            header_end_idx = response_buffer.find(b"\r\n\r\n")
            
            if header_end_idx != -1:
                # 找到了 Header，解析 Content-Length
                headers = response_buffer[:header_end_idx]
                match = CL_PATTERN.search(headers)
                if match:
                    content_length = int(match.group(1))
                else:
                    # 如果没有 Content-Length (且不是 chunked)，通常意味着出错或 HTTP/1.0
                    # 这里为了简单，假设必须有 CL
                    content_length = 0 
        
        # 3. 如果已经拿到了 Header，检查 Body 是否接收完整
        if header_end_idx != -1:
            # 当前 buffer 总长度 vs (Header长度 + 4 + Body长度)
            total_expected_len = header_end_idx + 4 + content_length
            if len(response_buffer) >= total_expected_len:
                # 读取完毕，如果有粘包（多读了下一次请求的数据），这里为了简单 benchmark 忽略处理
                # 严谨的客户端需要把多余数据缓存给下一次 recv
                return True

def worker(tid, n):
    global total, ok, fail, lat_sum
    
    # 构造 Keep-Alive 请求
    req_str = (
        "GET /hello HTTP/1.1\r\n"
        f"Host: {SERVER_IP}:{SERVER_PORT}\r\n"
        "Connection: keep-alive\r\n"  # <--- 关键修改
        "\r\n"
    ).encode()

    sock = None
    count = 0

    while count < n:
        # 如果没有连接，建立连接
        if sock is None:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1) # 禁用 Nagle 算法，降低延迟
                sock.settimeout(TIMEOUT)
                sock.connect((SERVER_IP, SERVER_PORT))
            except Exception:
                with lock:
                    fail += 1
                    total += 1
                count += 1
                if sock: sock.close()
                sock = None
                continue

        t0 = time.perf_counter()
        try:
            # 发送请求
            sock.sendall(req_str)
            
            # 读取完整响应
            read_response(sock)

            dt = time.perf_counter() - t0
            with lock:
                ok += 1
                total += 1
                lat_sum += dt
            
            count += 1
            
        except Exception as e:
            # 发生任何错误（超时、断开、解析失败）
            with lock:
                fail += 1
                total += 1
            count += 1
            # 关闭当前连接，下次循环会重连
            if sock:
                try: sock.close()
                except: pass
            sock = None
    
    # 任务结束，关闭连接
    if sock:
        try: sock.close()
        except: pass

def main():
    print(f"Benchmarking {SERVER_IP}:{SERVER_PORT} with Keep-Alive...")
    print(f"Concurrency: {CONCURRENCY}, Total Requests: {REQUEST_COUNT}")

    per = REQUEST_COUNT // CONCURRENCY
    rem = REQUEST_COUNT % CONCURRENCY
    threads = []
    t0 = time.perf_counter()

    for i in range(CONCURRENCY):
        n = per + (1 if i < rem else 0)
        th = threading.Thread(target=worker, args=(i, n))
        th.start()
        threads.append(th)

    for th in threads:
        th.join()

    dur = time.perf_counter() - t0
    
    print("-" * 40)
    print(f"Total Requests : {total}")
    print(f"Successful     : {ok}")
    print(f"Failed         : {fail}")
    print(f"Duration       : {dur:.3f} s")
    print(f"QPS (RPS)      : {ok/dur:.2f}")
    if ok:
        print(f"Avg Latency    : {lat_sum/ok*1000:.3f} ms")
    print("-" * 40)

if __name__ == "__main__":
    main()