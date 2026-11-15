# ADR 0004: Không dùng firmware Muse Gadget SDK trên board này

- Ngày: 2026-11-15
- Trạng thái: Chấp nhận

## Bối cảnh
Meta phát hành Muse Gadgets (firmware ESP32 mã nguồn mở, Apache 2.0, github.com/facebookincubator/muse-gadget-sdk). Danh sách board chỉ có bản CYD 2.8" (`sdkconfig.esp32-2432s028`), và bản đó chỉ có màn hình trạng thái: không cảm ứng, không mic, không loa. README ghi TTS và gọi thoại trực tiếp cần PSRAM. Board 3.5" của dự án không có trong danh sách.

## Quyết định
Không dùng firmware Muse Gadget trên board này. Dùng kiến trúc tự làm (ADR 0001); Muse, nếu dùng, chỉ là một `BrainProvider` (ADR 0002).

## Hệ quả
- Muốn trải nghiệm Muse Gadget đầy đủ thì cần board ESP32-S3 có PSRAM nằm trong danh sách hỗ trợ.
- Sửa firmware Muse cho board này vẫn được về giấy phép, nhưng vướng TLS và giải nén MP3 trên chip; không làm.
