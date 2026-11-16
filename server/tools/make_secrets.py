"""Tạo server/.env và firmware/include/secrets.h (cả hai git bỏ qua) với cùng một DEVICE_TOKEN.

Chạy từ server/:  python tools/make_secrets.py
- Không ghi đè file đã có; token lấy lại từ file đã có để hai bên khớp nhau.
- MUSE2API_KEY lấy từ container muse2api (docker exec), không in ra.
- Tên WiFi và mật khẩu: người dùng tự điền vào secrets.h sau khi chạy.
Không in bất kỳ bí mật nào ra màn hình.
"""

from __future__ import annotations

import re
import secrets
import socket
import subprocess
from pathlib import Path

SERVER = Path(__file__).resolve().parents[1]
ENV = SERVER / ".env"
ENV_EXAMPLE = SERVER / "env.example"
SECRETS_H = SERVER.parent / "firmware" / "include" / "secrets.h"
SECRETS_EXAMPLE = SECRETS_H.with_name("secrets.example.h")


def lan_ip() -> str:
    """IP LAN của máy này (UDP connect không gửi gói nào, chỉ hỏi bảng định tuyến)."""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.connect(("192.168.1.1", 9))
        return s.getsockname()[0]


def muse2api_key() -> str:
    out = subprocess.run(["docker", "exec", "muse2api", "printenv", "MUSE2API_KEY"],
                         capture_output=True, text=True)
    return out.stdout.strip() if out.returncode == 0 else ""


def existing_token() -> str:
    if ENV.exists():
        m = re.search(r"^DEVICE_TOKEN=(\S+)$", ENV.read_text(encoding="utf-8"), re.M)
        if m:
            return m.group(1)
    if SECRETS_H.exists():
        m = re.search(r'BYTEOF_DEVICE_TOKEN\s+"([^"]+)"', SECRETS_H.read_text(encoding="utf-8"))
        if m:
            return m.group(1)
    return ""


def main() -> None:
    token = existing_token() or secrets.token_urlsafe(24)

    if ENV.exists():
        print(f"giữ nguyên {ENV}")
    else:
        key = muse2api_key()
        text = ENV_EXAMPLE.read_text(encoding="utf-8")
        text = text.replace("DEVICE_TOKEN=\n", f"DEVICE_TOKEN={token}\n")
        text = text.replace("MUSE2API_KEY=\n", f"MUSE2API_KEY={key}\n")
        ENV.write_text(text, encoding="utf-8", newline="\n")
        print(f"đã tạo {ENV}" + ("" if key else " (THIẾU MUSE2API_KEY: container muse2api chưa chạy?)"))

    # Bản cũ (có BYTEOF_WIFI_SSID, trước ADR 0008) thì viết lại theo mẫu mới, giữ token.
    if SECRETS_H.exists() and "BYTEOF_SERVER_URL" in SECRETS_H.read_text(encoding="utf-8"):
        print(f"giữ nguyên {SECRETS_H}")
    else:
        text = SECRETS_EXAMPLE.read_text(encoding="utf-8")
        text = text.replace('"doi-thanh-chuoi-ngau-nhien"', f'"{token}"')
        SECRETS_H.write_text(text, encoding="utf-8", newline="\n")
        print(f"đã tạo {SECRETS_H}. WiFi cài bằng điện thoại; địa chỉ server điền vào BYTEOF_SERVER_URL "
              f"(LAN: ws://{lan_ip()}:8765).")


if __name__ == "__main__":
    main()
