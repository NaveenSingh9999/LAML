#!/usr/bin/env python3
"""WS fan-out bench: N clients in one room, each sends K msgs, all recv all.
Server: bench_chat.lm (broadcast relay). Reports fan-out latency p50/p99.
Note: per-client recv is the bottleneck at scale; this measures system e2e."""
import socket, base64, os, struct, sys, time, threading

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 18091
N = int(sys.argv[2]) if len(sys.argv) > 2 else 100
K = int(sys.argv[3]) if len(sys.argv) > 3 else 5

def conn():
    s = socket.create_connection(("127.0.0.1", PORT), timeout=10)
    key = base64.b64encode(os.urandom(16)).decode()
    s.sendall(f"GET /c HTTP/1.1\r\nHost: x\r\nUpgrade: websocket\r\n"
              f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
              f"Sec-WebSocket-Version: 13\r\n\r\n".encode())
    assert "101" in s.recv(512).decode()
    return s

def send(s, text):
    b = text.encode(); m = os.urandom(4)
    s.sendall(bytes([0x81, 0x80 | len(b)]) + m +
              bytes(c ^ m[i % 4] for i, c in enumerate(b)))

def recv(s):
    h = s.recv(2)
    ln = h[1] & 0x7F
    if ln == 126: ln = struct.unpack("!H", s.recv(2))[0]
    d = b""
    while len(d) < ln: d += s.recv(ln - len(d))
    return d.decode()

conns = [conn() for _ in range(N)]
print(f"connected={N}", flush=True)
lats = []
lock = threading.Lock()
stop = [False]

def reader(s):
    while not stop[0]:
        try:
            t0 = time.monotonic()
            m = recv(s)
            dt = (time.monotonic() - t0) * 1000
            if m.startswith("m:"):
                with lock: lats.append(dt)
        except Exception:
            return

readers = [threading.Thread(target=reader, args=(s,), daemon=True) for s in conns]
for t in readers: t.start()
t0 = time.monotonic()
for k in range(K):
    for i, s in enumerate(conns):
        try: send(s, f"m:{i}:{k}")
        except Exception: pass
# wait for fan-out: expect N*K*N -ish (minus except-sender); poll until quiet
time.sleep(5)
stop[0] = True
dt = time.monotonic() - t0
for s in conns:
    try: s.close()
    except Exception: pass
lats.sort()
n = len(lats)
def pct(p): return lats[min(n - 1, int(n * p / 100))] if n else 0
print(f"clients={N} rounds={K} got={n} time_s={dt:.1f} "
      f"p50_ms={pct(50):.1f} p99_ms={pct(99):.1f}")
