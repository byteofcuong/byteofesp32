// Mẫu bí mật cho firmware. Chép thành secrets.h (git bỏ qua) hoặc chạy: python server/tools/make_secrets.py
// Thiếu secrets.h thì firmware vẫn build và chạy, chỉ không nối mạng (ADR 0007).
// WiFi KHÔNG ghi ở đây: cài bằng điện thoại qua trang cài WiFi của board (ADR 0008).
#pragma once

// Phải giống DEVICE_TOKEN trong server/.env.
#define BYTEOF_DEVICE_TOKEN "doi-thanh-chuoi-ngau-nhien"

// Địa chỉ server mặc định (điền sẵn ở trang cài WiFi; đổi được ở đó mà không cần nạp lại).
// Qua Cloudflare Tunnel: "wss://fox.ten-mien-cua-ban.com". Trong mạng nhà: "ws://192.168.1.100:8765".
#define BYTEOF_SERVER_URL ""
