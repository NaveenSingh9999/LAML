#!/usr/bin/env python3
"""Slow-client: 1 blackhole WS (never recvs, tiny buffer) + K active measurers.
Server: bench_chat.lm. Checks room stays fast + drops counter rises."""
import socket, base64, os, struct, sys, time, threading, urllib.request

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 18093
K = int(sys.argv[2]) if len(sys.argv) > 2 else 10
MSGS = int(sys.argv[3]) if len(sys.argv) > 3 else 200

def conn():
    s = socket.create_connection(("127.0.0.1", PORT), timeout=10)
    key = base64.b64encode(os.urandom(16)).decode()
    s.sendall(f"GET /c HTTP/1.1\r\nHost: x\r\nUpgrade: websocket\r\n"
              f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
              f"Sec-WebSocket-Version: 13\r\n\r\n".encode())
    s.recv(512)
    return s

def send(s, text):
    b = text.encode(); m = os.urandom(4); n = len(b)
    if n < 126: hdr = bytes([0x81, 0x80 | n])
    elif n < 65536: hdr = bytes([0x81, 0x80 | 126]) + struct.pack("!H", n)
    else: raise ValueError("too big")
    s.sendall(hdr + m + bytes(c ^ m[i % 4] for i, c in enumerate(b)))

def recv(s):
    s.settimeout(10)
    h = s.recv(2); ln = h[1] & 0x7F
    if ln == 126: ln = struct.unpack("!H", s.recv(2))[0]
    d = b""
    while len(d) < ln: d += s.recv(ln - len(d))
    return d.decode()

# blackhole: tiny recv buffer, never read
hole = conn()
hole.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 4096)

fast = [conn() for _ in range(K)]
lats = []
lock = threading.Lock()
stop = [False]

def reader(s):
    while not stop[0]:
        try:
            t0 = time.monotonic(); m = recv(s)
            if m.startswith("b:"):
                with lock: lats.append((time.monotonic() - t0) * 1000)
        except Exception: return

for s in fast:
    threading.Thread(target=reader, args=(s,), daemon=True).start()
# flood from one sender (not measured) so blackhole's queue overflows
src = conn()
t0 = time.monotonic()
big = "b:" + "x" * 4000
for _ in range(MSGS):
    try: send(src, big)
    except Exception: break
time.sleep(3)
stop[0] = True
dt = time.monotonic() - t0
for s in fast + [hole, src]:
    try: s.close()
    except Exception: pass
try:
    stats = urllib.request.urlopen(f"http://127.0.0.1:{PORT}/__stats", timeout=5).read().decode()
    drops = [l for l in stats.splitlines() if l.startswith("laml_dropped")]
except Exception as e:
    drops = [f"stats-unavailable: {e}"]
lats.sort()
n = len(lats)
def pct(p): return lats[min(n - 1, int(n * p / 100))] if n else 0
print(f"fast={K} msgs={MSGS} got={n} time_s={dt:.1f} "
      f"p50_ms={pct(50):.1f} p99_ms={pct(99):.1f} {drops}")
