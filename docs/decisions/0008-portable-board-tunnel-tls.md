# ADR 0008: Board mang đi đâu cũng chạy: cài WiFi bằng điện thoại, vào server qua Cloudflare Tunnel bằng wss://

- Ngày: 2026-11-17
- Trạng thái: Chấp nhận
- Thay thế một phần ADR 0001 (Device ↔ HomeServer "chỉ trong mạng nhà, không TLS") và ADR 0007 (WiFi ghi trong `secrets.h`). Phần còn lại của hai ADR giữ nguyên.

## Bối cảnh
- Người dùng muốn mang board đi nơi khác mà không phải sửa file hay nạp lại firmware (2026-11-16).
- HomeServer vẫn là PC ở nhà (ADR 0001). Người dùng chọn để PC bật suốt và mở server ra Internet qua đường hầm, thay vì thuê VPS (Q10).
- So sánh đường hầm (2026-11-16): ngrok miễn phí 1GB/tháng (không đủ khi stream âm thanh); Tailscale Funnel có giới hạn băng thông không công bố; Cloudflare Tunnel không giới hạn thực tế nhưng địa chỉ cố định cần tên miền riêng (~$10/năm). Người dùng chọn Cloudflare + tên miền.

## Quyết định
- **Cài WiFi:** board lưu tối đa 5 WiFi và địa chỉ server trong NVS (`firmware/src/net_config.*`). Không vào được mạng nào thì phát WiFi riêng `byteof-XXXX`, mật khẩu `foxXXXXXXXX` (suy từ MAC, hiện trên màn), kèm trang cài ở `192.168.4.1` (`firmware/src/portal.*`, tự viết vì WiFiManager 2.0.17 có trước Arduino core 3). Lúc cài, linh vật nhường RAM (`Mascot::end()`) và màn hiện hướng dẫn; bấm Lưu xong board khởi động lại.
- **Đường tới server:** `wss://<tên miền>/` → Cloudflare → `cloudflared` trên PC → `ws://127.0.0.1:8765`. Server chỉ nghe `127.0.0.1` (không cần mở Windows Firewall). Vẫn hỗ trợ `ws://IP:8765` trong mạng nhà để thử.
- **TLS trên board:** `WebSocketsClient::beginSslWithBundle` với bộ CA gốc Mozilla dạng nén của ESP32 (`firmware/certs/`), vì Cloudflare (gói miễn phí) tự đổi CA giữa Let's Encrypt, Google Trust Services và SSL.com. Đồng bộ giờ NTP trước khi bắt tay.
- **Server ra Internet:** token trong lời chào (ADR 0007) giữ nguyên; `max_size` tin nhắn 4096 byte.
- **RAM cho TLS** (đo 2026-11-17): WiFi STA giữ ~67KB, bắt tay TLS cần thêm ~54KB, giữ kết nối ~43KB. Để vừa, linh vật giảm từ ~150KB xuống ~87KB:
  - một ảnh tai thay vì hai (tai phải dán lật gương);
  - tai và vòm cười lưu độ phủ 8-bit, dán qua bảng màu (`blitMask`);
  - dải vẽ 40 dòng thay vì 56 (fps không đổi);
  - ảnh chấm báo chỉ tạo khi đang thông báo.
  Kết quả: heap sau linh vật 189.9KB, thấp nhất lúc bắt tay ~65KB, khi online ~76KB.

## Hệ quả
- Đến chỗ mới: dùng điện thoại cài WiFi một lần, không sửa file. PC ở nhà phải bật, chạy server và `cloudflared`.
- Đổi tên miền hay địa chỉ server: sửa ở trang cài WiFi, không phải nạp lại firmware.
- Thêm vùng vẽ hay ảnh đệm mới cho linh vật phải tính lại RAM với TLS. Xem số đo `heap=` trong log `[link] status=…`.
- Có WiFi, các mặt nặng chậm hơn 3–4 fps (khóc 22.7 → 18.7 so với cùng firmware tắt WiFi, đo 2026-11-17); mặt nhẹ và trạng thái gần như không đổi. Không phải do nhịp hỏi mạng (5 hay 20ms như nhau).
- Bộ CA cần làm mới 1–2 năm một lần (`firmware/certs/README.md`).
