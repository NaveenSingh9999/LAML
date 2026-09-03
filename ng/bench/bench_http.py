#!/usr/bin/env python3
"""HTTP keep-alive bench: C threads x R requests each. Reports rps, p50/p99."""
import http.client, sys, time, threading

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 18090
C = int(sys.argv[2]) if len(sys.argv) > 2 else 20
R = int(sys.argv[3]) if len(sys.argv) > 3 else 100

lats = []
lock = threading.Lock()
errs = [0]

def worker():
    c = http.client.HTTPConnection("127.0.0.1", PORT, timeout=10)
    for _ in range(R):
        t0 = time.monotonic()
        try:
            c.request("GET", "/bench")
            r = c.getresponse()
            r.read()
            assert r.status == 200
        except Exception:
            errs[0] += 1
            try: c.close()
            except Exception: pass
            c = http.client.HTTPConnection("127.0.0.1", PORT, timeout=10)
            continue
        with lock:
            lats.append((time.monotonic() - t0) * 1000)

ths = [threading.Thread(target=worker) for _ in range(C)]
t0 = time.monotonic()
for t in ths: t.start()
for t in ths: t.join()
dt = time.monotonic() - t0
lats.sort()
n = len(lats)
def pct(p): return lats[min(n - 1, int(n * p / 100))] if n else 0
print(f"conns={C} reqs={n} errs={errs[0]} time_s={dt:.2f} rps={n/dt:.0f} "
      f"p50_ms={pct(50):.2f} p99_ms={pct(99):.2f} max_ms={lats[-1] if n else 0:.2f}")
