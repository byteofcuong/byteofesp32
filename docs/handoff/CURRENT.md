---
date: 2026-11-16
branch: firmware/display-touch
commit: 23db86f
---
# Handoff

## Mục tiêu
Hiển thị lên màn board ESP32-3248S035R: màu đúng, chữ, cảm ứng vẽ được. **Đã xong**: người dùng xác nhận màu đúng và cảm ứng "tốt như bản đầu".

## Trạng thái
- `firmware/` chạy trên board (COM5): màn kiểm tra 4 màu, hiệu chỉnh cảm ứng 4 điểm + kiểm tra 5 điểm, màn chính vẽ tay, nút XOA và CAN CHINH, LED RGB đổi màu.
- Đã commit trên nhánh `firmware/display-touch` (tách từ `main` ở `23db86f`): `firmware/`, ADR 0005, docs/hardware, open-questions, AGENTS.md, ontology, bàn giao này. Chưa merge vào `main`, chưa push.
- Bản sao lưu flash gốc: `backups/factory_backup.bin` (git bỏ qua). Đã thử khôi phục một lần, chạy đúng.
- Công cụ đã cài: esptool 5.6.0, PlatformIO 6.2.0 (pip --user, gọi qua `python -m`).

## Quyết định
- ADR 0005: PlatformIO + Arduino + TFT_eSPI cho màn. Cảm ứng tự đọc theo thuật toán LovyanGFX (PENIRQ GPIO36 + trung vị 7 lượt), hiệu chỉnh tự viết lưu NVS khóa `cal_lgfx`.
- Đã loại, đừng thử lại:
  - LovyanGFX cho màn: sai màu. Đã thử đảo màu, BGR, B6=0x02, 24-bit, SPI 10MHz, chuỗi khởi tạo LCDwiki; đọc ngược bộ nhớ màn vẫn đúng; chip là ST7796.
  - `getTouch`/`calibrateTouch` của TFT_eSPI: đứt nét, lệch nửa dưới.
  - Bộ lọc vá theo lực ấn và cắt đầu/đuôi nét: làm cụt nét mà vẫn có đoạn lạ.
  - Giữ BOOT lúc khởi động: chip vào chế độ nạp code.

## Việc tiếp theo
1. Hỏi người dùng có merge `firmware/display-touch` vào `main` và push không.
2. Bước tiếp của dự án: chọn giữa giao diện linh vật (LVGL hoặc vẽ bằng TFT_eSPI) và thử loa (DAC GPIO26) khi người dùng có loa. Mic, pin, loa chưa mua.
3. Vẫn chờ người dùng: ảnh rõ mã chip 8 chân cạnh `2R2` (Q1), đo cổng BAT1 (Q2), token Muse có API key không (Q6).

## Kiểm chứng
- `cd firmware && python -m platformio run -e cyd35 -t upload`: SUCCESS.
- `python firmware/tools/serial_log.py COM5 8`: `[lcd] init ok, 480x320, backlight GPIO27`, `psram 0`, `[ready] home screen`.
- Log 3 phút sau khi nạp bản cuối: `[touchtest] worst=5px` (5 điểm: 5, 1, 2, 3, 2px), 121 nét vẽ.
- Người dùng xác nhận bằng mắt: 4 dải màu đúng; cảm ứng "tốt như bản đầu".
- Ontology check: `errors=0 warnings=7`. `python .claude/hooks/test_handoff_hooks.py`: OK.

## Rủi ro
- `serial_log.py` reset board qua RTS khi mở cổng; khi nó đang chạy thì không nạp được (COM5 bận).
- Một lần ghi log 2 phút không thu được dòng chạm nào, chưa rõ nguyên nhân (có thể người dùng vẽ trước khi board khởi động lại xong).
- Chưa rõ vì sao LovyanGFX sai màu (Q11); nếu cần LovyanGFX cho LVGL sau này phải tìm ra.
