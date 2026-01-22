#!/usr/bin/env python3
import socket, threading, time

SERVER_IP = "127.0.0.1"
SERVER_PORT = 8080
REQUEST_COUNT = 10000
CONCURRENCY = 50
TIMEOUT = 5

lock = threading.Lock()
total = ok = fail = 0
lat_sum = 0.0

def worker(tid, n):
    global total, ok, fail, lat_sum
    for _ in range(n):
        s = None
        t0 = time.perf_counter()
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.settimeout(TIMEOUT)
            s.connect((SERVER_IP, SERVER_PORT))

            req = (
                "GET / HTTP/1.1\r\n"
                f"Host: {SERVER_IP}:{SERVER_PORT}\r\n"
                "Connection: close\r\n"
                "\r\n"
            ).encode()

            s.sendall(req)

            # 短连接模式：读到 EOF 即结束
            while True:
                data = s.recv(4096)
                if not data:
                    break

            dt = time.perf_counter() - t0
            with lock:
                total += 1
                ok += 1
                lat_sum += dt
        except Exception:
            with lock:
                total += 1
                fail += 1
        finally:
            if s:
                try: s.close()
                except: pass

def main():
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
    print(f"total={total} ok={ok} fail={fail}")
    print(f"duration={dur:.3f}s rps={total/dur:.2f}")
    if ok:
        print(f"avg_latency={lat_sum/ok*1000:.3f} ms")

if __name__ == "__main__":
    main()
