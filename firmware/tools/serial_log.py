"""Reset board rồi in log Serial trong N giây. Dùng để kiểm chứng không cần màn hình.

Chạy: python firmware/tools/serial_log.py [COM5] [giây]
"""
import sys
import time

import serial  # pyserial, đi kèm esptool/platformio


def main() -> None:
    port = sys.argv[1] if len(sys.argv) > 1 else "COM5"
    seconds = float(sys.argv[2]) if len(sys.argv) > 2 else 10
    with serial.Serial(port, 115200, timeout=0.2) as ser:
        # Mạch tự reset của board: RTS kéo EN xuống, DTR giữ cao để IO0 ở mức cao (chạy bình thường).
        ser.dtr = False
        ser.rts = True
        time.sleep(0.1)
        ser.rts = False
        end = time.time() + seconds
        while time.time() < end:
            line = ser.readline()
            if line:
                sys.stdout.write(line.decode("utf-8", errors="replace"))
                sys.stdout.flush()


if __name__ == "__main__":
    main()
