"""Đo fps từng biểu cảm và trạng thái trên board.

Mở cổng (board tự reset qua RTS), chờ [ready], rồi lần lượt gửi 'e' qua 15 biểu cảm và 's' qua các trạng thái.
Mỗi bước in: fps và khung chậm nhất lúc đứng yên, và khung chậm nhất trong cửa sổ có lúc chuyển (độ khựng).
Dùng: python tools/fps_sweep.py COM5   (chạy từ firmware/, cần pyserial; ~2.5 phút)
"""
import re
import sys
import time

import serial

port = sys.argv[1] if len(sys.argv) > 1 else "COM5"
ser = serial.Serial(port, 115200, timeout=0.2)
ser.dtr = False
ser.rts = True
time.sleep(0.1)
ser.rts = False

buf = ""
def read_until(pattern, timeout):
    global buf
    end = time.time() + timeout
    while time.time() < end:
        chunk = ser.read(512).decode("utf-8", "replace")
        if chunk:
            buf += chunk
            m = re.search(pattern, buf)
            if m:
                buf = buf[m.end():]
                return m
    return None

if not read_until(r"\[ready\] mascot", 15):
    print("no [ready]"); sys.exit(1)
print("ready")
fps_re = r"\[mascot\] fps=([\d.]+) frame=(\d+)ms \(push (\d+)ms\) max=(\d+)ms state=(\w+) expr=(\w+) asleep=(\d) heap=(\d+)"

def measure(label):
    tr = read_until(fps_re, 6)     # Cửa sổ có lúc chuyển: lấy khung chậm nhất (độ khựng).
    m = read_until(fps_re, 7)      # Cửa sổ đứng yên.
    if m:
        print(f"{label:<22} fps={m.group(1):>5} frame={m.group(2):>3}ms max={m.group(4):>3}ms  | khi chuyển: fps={tr.group(1) if tr else '?':>5} max={tr.group(4) if tr else '?':>3}ms  state={m.group(5)} heap={m.group(8)}")
    else:
        print(f"{label:<22} (no fps line)")

measure("neutral")
for _ in range(14):
    ser.write(b"e")
    m = read_until(r"expression=(\w+)", 3)
    measure(m.group(1) if m else "?")
ser.write(b"e")  # Về neutral.
read_until(r"expression=(\w+)", 3)
for _ in range(4):
    ser.write(b"s")
    m = read_until(r"state=(\w+)\r?\n", 3)
    measure("state " + (m.group(1) if m else "?"))
ser.write(b"s")
ser.close()

