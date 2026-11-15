# ADR 0003: Từ đánh thức xử lý trên server

- Ngày: 2026-11-15
- Trạng thái: Chấp nhận

## Bối cảnh
Nhận dạng từ đánh thức phải giữ âm thanh và tính toán liên tục trong RAM. Board không có PSRAM; thẻ SD quá chậm để thay RAM. Model từ khóa siêu nhỏ trên chip hay nhận nhầm và tranh RAM với màn hình.

## Quyết định
- Device stream âm thanh mic lên HomeServer; server chạy openWakeWord.
- Khi chạy pin: Device chỉ gửi khi phát hiện có tiếng người (voice activity detection) để tiết kiệm WiFi.
- Luôn giữ cách kích hoạt dự phòng: chạm màn.

## Hệ quả
- Server tắt thì không gọi bằng giọng được, chỉ chạm màn.
- Âm thanh không rời mạng nhà nếu server đặt ở nhà.
