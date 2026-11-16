---
date: 2026-11-16
branch: main
commit: 8c5b4f2
---
# Handoff

## Mục tiêu
Trợ lý giọng nói để bàn trên board ESP32-3248S035R. Bước 1 của lộ trình (linh vật trên màn) **đã xong**: người dùng chốt thiết kế và xác nhận trên màn thật là liền mạch, sạch, mượt.

## Trạng thái
- `main` đã có và đã push: firmware màn + cảm ứng + linh vật (`firmware/src/main.cpp`, `mascot.*`, `touch.*`), đặc tả và bản mẫu linh vật (`docs/design/mascot/`), ontology.
- Linh vật: hai mắt robot có **con ngươi khe dọc**, **tai cáo đặc lòng sẫm**, mũi tam giác, miệng ω, màu cam khi rảnh; 5 trạng thái trợ lý, 6 biểu cảm; chuyển động bằng lò xo; 30 fps.
- Thử trên board: chạm mắt = nháy mắt; chạm chỗ khác = đổi trạng thái; giữ 1 giây = đổi biểu cảm; giữ 5 giây = hiệu chỉnh cảm ứng; để yên 30 giây = ngủ.
- `AGENTS.md` đã cập nhật trạng thái dự án (trước đó vẫn ghi "chưa có code").
- Nhánh `firmware/display-touch`, `feature/fox-mascot` còn ở máy, đã merge, xóa được. Sao lưu flash gốc ở `backups/factory_backup.bin` (git bỏ qua).

## Quyết định
- ADR 0005: PlatformIO + Arduino + TFT_eSPI; cảm ứng tự đọc kiểu LovyanGFX (PENIRQ GPIO36, trung vị 7 lượt); SPI 80MHz.
- Thiết kế linh vật: xem `docs/design/mascot/spec.md` (có mục chống đụng hàng với Cozmo/Vector/EMO và danh sách phương án đã loại). Mọi thay đổi hình: sửa `prototype.html`, tự chụp và soi trước khi đưa người dùng.
- Đã loại, đừng thử lại: LovyanGFX cho màn (sai màu); `getTouch` của TFT_eSPI; sprite 8-bit (hình thô); nét làm mịn pha với màu cố định (vết cắt); vẽ tai mỗi khung (36ms).
- Commit message: một câu tiếng Anh, không dấu vết AI.

## Việc tiếp theo
1. Bước 2 lộ trình: nối board với server Python trên PC qua WiFi + WebSocket để server điều khiển trạng thái và biểu cảm linh vật (chưa cần âm thanh).
2. Khi người dùng có loa: thử phát âm thanh qua DAC GPIO26, nhép miệng theo biên độ.
3. Chờ người dùng: mua loa, mic INMP441, pin; ảnh rõ mã chip 8 chân cạnh `2R2` (Q1), đo cổng BAT1 (Q2), token Muse có API key không (Q6). Chưa hỏi người dùng về Codex.

## Kiểm chứng
- `cd firmware && python -m platformio run -e cyd35 -t upload`: SUCCESS (RAM ~7%, flash ~26%).
- `python firmware/tools/serial_log.py COM5 8`: `[ready] mascot`, `[mascot] fps=27.4–30.0 frame=32–35ms (push 25ms) heap=198920`.
- Ontology check: `errors=0 warnings=8`. `python .claude/hooks/test_handoff_hooks.py`: OK.
- Người dùng xác nhận trên màn thật: liền mạch, sạch, mượt (2026-11-16).

## Rủi ro
- SPI 80MHz vượt mức thông dụng 40MHz; đang chạy sạch nhưng nếu màn nhiễu thì hạ về 40MHz (giảm còn ~19 fps).
- Lịch sử `main` đã viết lại và force push một lần (2026-11-16); bản cũ ở `refs/original/` trên máy.
- `serial_log.py` reset board qua RTS khi mở cổng; khi nó đang chạy thì không nạp được.
- Q11: chưa rõ vì sao LovyanGFX sai màu, chỉ cần nếu sau này muốn dùng LovyanGFX cho LVGL.
