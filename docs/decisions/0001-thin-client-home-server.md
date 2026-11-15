# ADR 0001: Board là thin client, server ở nhà làm phần nặng

- Ngày: 2026-11-15
- Trạng thái: Chấp nhận

## Bối cảnh
Board là ESP32 classic không PSRAM, chỉ khoảng 320KB RAM dùng được sau khi trừ WiFi và màn hình. Kết nối TLS, giải nén MP3, nhận dạng giọng nói và gọi LLM ngay trên chip không vừa.

## Quyết định
- Device chỉ làm: hiển thị linh vật, nhận chạm, stream âm thanh mic lên, phát âm thanh thô nhận về.
- HomeServer (Python, trong mạng nhà) làm: từ đánh thức, speech-to-text, gọi bộ não, text-to-speech, công cụ, việc chạy nền.
- Device ↔ HomeServer dùng WebSocket không TLS trong mạng nhà; âm thanh xuống là PCM thô, không MP3.
- API key chỉ nằm trên HomeServer.

## Hệ quả
- Server phải bật thì trợ lý mới hoạt động.
- Đổi logic không cần nạp lại firmware.
- Mẫu tham khảo: bài "I built my own Muse Charm on an ESP32 stick" (HackerNoon) dùng cùng cách chia việc.
