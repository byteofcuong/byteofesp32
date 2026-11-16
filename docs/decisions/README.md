# Quyết định kiến trúc (ADR)

Mỗi quyết định một file `NNNN-ten-ngan.md`. Không sửa nội dung ADR đã chấp nhận; muốn đổi thì viết ADR mới và ghi "Thay thế ADR NNNN". Skill `/handoff` nhắc ghi ADR khi session có quyết định mới.

| ADR | Tiêu đề | Trạng thái |
|---|---|---|
| [0001](0001-thin-client-home-server.md) | Board là thin client, server ở nhà làm phần nặng | Chấp nhận |
| [0002](0002-swappable-brain.md) | Bộ não đổi được: Muse API hoặc Claude API | Chấp nhận; cách lấy khóa Muse thay bởi 0006 |
| [0003](0003-wake-word-on-server.md) | Từ đánh thức xử lý trên server | Chấp nhận |
| [0004](0004-no-muse-gadget-firmware.md) | Không dùng firmware Muse Gadget SDK trên board này | Chấp nhận |
| [0005](0005-firmware-platformio-arduino-tft-espi.md) | Firmware dùng PlatformIO + Arduino + TFT_eSPI | Chấp nhận |
| [0006](0006-muse-via-muse2api.md) | Gọi Muse qua muse2api chạy riêng trên máy | Chấp nhận |
