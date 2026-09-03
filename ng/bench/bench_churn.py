#!/usr/bin/env python3
"""Churn: connect/close WS ROUNDS x BATCH, then verify server alive + RSS delta."""
import socket, base64, os, sys, time, subprocess

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 18092
BATCH = int(sys.argv[2]) if len(sys.argv) > 2 else 100
ROUNDS = int(sys.argv[3]) if len(sys.argv) > 3 else 10
PID = int(sys.argv[4]) if len(sys.argv) > 4 else 0

def rss(pid):
    try:
        with open(f"/proc/{pid}/status") as f:
            for line in f:
                if line.startswith("VmRSS:"): return int(line.split()[1])
    except Exception: return -1
    return -1

def one():
    s = socket.create_connection(("127.0.0.1", PORT), timeout=10)
    key = base64.b64encode(os.urandom(16)).decode()
    s.sendall(f"GET /c HTTP/1.1\r\nHost: x\r\nUpgrade: websocket\r\n"
              f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
              f"Sec-WebSocket-Version: 13\r\n\r\n".encode())
    s.recv(256)
    s.close()

r0 = rss(PID)
for r in range(ROUNDS):
    for _ in range(BATCH): one()
r1 = rss(PID)
# alive?
s = socket.create_connection(("127.0.0.1", PORT), timeout=10)
s.close()
print(f"total={BATCH*ROUNDS} rss_before_kb={r0} rss_after_kb={r1} "
      f"delta_kb={r1-r0 if r0>=0 and r1>=0 else -999} alive=yes")
