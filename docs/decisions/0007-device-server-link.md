# ADR 0007: Nối board với server bằng WebSocket JSON, mạng chạy trên lõi riêng

- Ngày: 2026-11-16
- Trạng thái: Chấp nhận

## Bối cảnh
Bước 2 của lộ trình: server điều khiển trạng thái và biểu cảm của linh vật qua WiFi. ADR 0001 đã chọn WebSocket không TLS trong mạng nhà. Linh vật đang chạy ~30 fps trên vòng `loop()` (lõi 1); nối TCP tới server đang tắt có thể chặn vài giây.

## Quyết định
- Tin nhắn là JSON trong text frame, trường `t` là loại tin. Chi tiết: `docs/design/protocol.md`.
- Board chào bằng `hello` kèm `DEVICE_TOKEN` (chuỗi ngẫu nhiên dùng chung giữa board và server); sai thì server đóng kết nối. Không phải bảo mật mạnh, chỉ chặn máy khác trong mạng nhà điều khiển nhầm.
- Firmware: thư viện `links2004/WebSockets` 2.7.3 + `bblanchon/ArduinoJson` 7.4.3. WiFi và WebSocket chạy trong một FreeRTOS task trên lõi 0; lệnh từ server đi qua hàng đợi sang `loop()` (lõi 1). Vẽ linh vật không bao giờ chờ mạng.
- Bảng phân vùng `huge_app.csv` (vùng app 3MB, không OTA): bản có WiFi nặng ~1.16MB, đầy 88% vùng 1.25MB mặc định. Nạp luôn qua USB nên không cần OTA.
- Tên WiFi, mật khẩu, địa chỉ server, token nằm trong `firmware/include/secrets.h` (git bỏ qua; mẫu `secrets.example.h`). Thiếu file này thì firmware vẫn build và chạy, không nối mạng.
- Server: Python 3.13, thư viện `websockets` (asyncio). Cấu hình trong `server/.env` (git bỏ qua; mẫu `server/env.example`).

## Hệ quả
- Đổi logic hội thoại chỉ cần sửa server.
- Địa chỉ IP của PC lấy từ DHCP; đổi IP thì phải sửa `secrets.h` và nạp lại. Về sau có thể đặt IP cố định trên router hoặc dùng mDNS.
- Windows Firewall phải cho phép Python nhận kết nối vào cổng 8765 trong mạng Private. Người dùng tự bấm cho phép.
- Âm thanh (mic, loa) sẽ thêm tin nhị phân sau; v1 chỉ có tin JSON.
