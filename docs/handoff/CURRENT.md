---
date: 2026-11-16
branch: main
commit: b862af7
---
# Handoff

## Mục tiêu
Hiển thị lên màn board ESP32-3248S035R: màu đúng, chữ, cảm ứng vẽ được. **Đã xong**: người dùng xác nhận màu đúng và cảm ứng "tốt như bản đầu".

## Trạng thái
- `firmware/` chạy trên board (COM5): màn kiểm tra 4 màu, hiệu chỉnh cảm ứng 4 điểm + kiểm tra 5 điểm, màn chính vẽ tay, nút XOA và CAN CHINH, LED RGB đổi màu.
- Toàn bộ firmware, ADR 0005, docs, ontology đã merge vào `main` và push lên GitHub. Nhánh `firmware/display-touch` còn ở máy, đã merge, xóa được.
- Bản sao lưu flash gốc: `backups/factory_backup.bin` (git bỏ qua). Đã thử khôi phục một lần, chạy đúng.
- Công cụ đã cài: esptool 5.6.0, PlatformIO 6.2.0 (pip --user, gọi qua `python -m`).

- **Đang làm: linh vật (bước 1 lộ trình), chưa commit.** Đã có `firmware/src/touch.*` (cảm ứng tách module), `mascot.*` (trạng thái, biểu cảm, chớp mắt, đảo mắt, thở, ngủ gật sau 30s, chạm mắt nháy mắt, giật tai), `main.cpp` (chạm đổi trạng thái, giữ 1s đổi biểu cảm, giữ 5s hiệu chỉnh; LED theo trạng thái; thanh chữ 3 dòng tự đo độ rộng, dài nhất 311px). Đang nạp bản cáo vẽ bằng sprite 8-bit, ~22 fps.
- Người dùng chê bản cáo hiện tại "xấu kinh khủng", muốn **cáo công nghệ đáng yêu**. Đã đưa 3 phác thảo SVG (A neon, B chibi đeo visor, C hologram): người dùng chê cả ba. Sau đó gửi 3 tấm ảnh AI tạo bằng Canva (media id MAHbCdjFZ5k, MAHbCatIszg, MAHbCRcNoEE; mỗi tấm vài mẫu cáo) và 2 kiểu vẽ bằng code có chuyển động (D pixel art 24x20 ô, E chỉ đôi mắt kiểu EMO + tai cáo). Người dùng chọn vẽ bằng code, loại pixel art và kiểu mắt EMO. Đã đưa bản kawaii đường cong mềm (F: LED trán + vạch mạch; G: tai nghe phát sáng). Người dùng: F "có vẻ ổn nhưng đơ, chưa ra dáng công nghệ". Đã đưa bản v4 dựa trên F: tai ngoe nguẩy, đuôi vẫy, mắt có vòng quét HUD xoay, lòng tai mạch điện + chóp tai kim loại, xung sáng chạy dọc mạch, khung HUD 4 góc + vạch quét. Người dùng: "còn đơ hơn, không mượt". Đã tra cách làm (Dasai Mochi dùng chuỗi khung hình vẽ sẵn; Rive dùng cỗ máy trạng thái; nguyên tắc hoạt hình: lò xo thay chuyển động tuyến tính, mắt đi trước đầu, tai/đuôi trễ nhịp, co giãn giữ thể tích, ít hiệu ứng mà rõ). Đã đưa v5 (lò xo vật lý): người dùng "xấu kinh khủng, trông ẩu, đập đi xây lại". **Làm lại từ đầu theo quy trình**: đặc tả `docs/design/mascot/spec.md` (tối giản, biểu cảm dồn vào mắt và tai, công nghệ = dải đèn trạng thái dưới cằm, quy tắc chống "ẩu") + bản mẫu dựng từ tham số `docs/design/mascot/prototype.html`, tự mở trình duyệt chụp và sửa 3 vòng trước khi gửi. Bản v7 (cáo tối giản phẳng) bị chê "siêu xấu, tẻ nhạt". Người dùng chỉ ra hướng: quay về bản đầu (hai mắt + một miệng) nhưng gợi ra con cáo. Đã làm v8 theo hướng đó: mắt robot biến hình bằng lò xo + tai cáo nét viền + mũi tam giác + miệng ω + màu cam khi rảnh; tự soi nhiều vòng (bỏ mắt xếch, bỏ túm lông má, sửa tai "lá cờ" và "nút play"). Đặc tả `docs/design/mascot/spec.md` đã viết lại theo hướng mới. Người dùng chê tai "không giống tai" và lo đụng hàng. Đã tra: mắt bo góc không con ngươi là ngôn ngữ của Cozmo/Vector/EMO/RoboEyes. Đã đổi: tai đặc lòng sẫm + con ngươi khe dọc kiểu mắt cáo (dấu hiệu riêng), ghi vào spec. Người dùng **chốt hàng 3** (tai đặc + con ngươi khe dọc). **Đã port lên firmware** (`firmware/src/mascot.*`, `touch.*`, `main.cpp`): sprite 16-bit vẽ theo dải, tai vẽ sẵn, nét pha với pixel bên dưới, SPI 80MHz → **30 fps**; người dùng xác nhận trên màn thật: liền mạch, sạch, mượt. Đã commit trên nhánh `feature/fox-mascot` (chưa merge vào `main`, chưa push). Lên board: dùng cùng mô hình lò xo, các lớp ảnh dịch chuyển lệch nhau (parallax). Nếu duyệt: vẽ bằng SVG, render ra ảnh từng lớp (đầu, mắt, miệng, tai) rồi chuyển RGB565 cho firmware ghép và cử động. Cách làm mới: phác thảo, duyệt rồi mới code; vẽ bằng sprite 16-bit có làm mịn cạnh (8-bit làm hình thô).

## Quyết định
- ADR 0005: PlatformIO + Arduino + TFT_eSPI cho màn. Cảm ứng tự đọc theo thuật toán LovyanGFX (PENIRQ GPIO36 + trung vị 7 lượt), hiệu chỉnh tự viết lưu NVS khóa `cal_lgfx`.
- Đã loại, đừng thử lại:
  - LovyanGFX cho màn: sai màu. Đã thử đảo màu, BGR, B6=0x02, 24-bit, SPI 10MHz, chuỗi khởi tạo LCDwiki; đọc ngược bộ nhớ màn vẫn đúng; chip là ST7796.
  - `getTouch`/`calibrateTouch` của TFT_eSPI: đứt nét, lệch nửa dưới.
  - Bộ lọc vá theo lực ấn và cắt đầu/đuôi nét: làm cụt nét mà vẫn có đoạn lạ.
  - Giữ BOOT lúc khởi động: chip vào chế độ nạp code.

## Việc tiếp theo
1. Bước tiếp của dự án: chọn giữa giao diện linh vật (LVGL hoặc vẽ bằng TFT_eSPI) và thử loa (DAC GPIO26) khi người dùng có loa. Mic, pin, loa chưa mua.
2. Vẫn chờ người dùng: ảnh rõ mã chip 8 chân cạnh `2R2` (Q1), đo cổng BAT1 (Q2), token Muse có API key không (Q6).

## Kiểm chứng
- Linh vật (2026-11-16, bản cuối): `python -m platformio run -e cyd35 -t upload` SUCCESS; log `[mascot] fps=27.4–30.0 frame=32–35ms (push 25ms) heap=198920`. Ontology check `errors=0 warnings=8`; test hook bàn giao OK. Người dùng xác nhận trên màn: liền mạch, sạch, mượt.
- `cd firmware && python -m platformio run -e cyd35 -t upload`: SUCCESS.
- `python firmware/tools/serial_log.py COM5 8`: `[lcd] init ok, 480x320, backlight GPIO27`, `psram 0`, `[ready] home screen`.
- Log 3 phút sau khi nạp bản cuối: `[touchtest] worst=5px` (5 điểm: 5, 1, 2, 3, 2px), 121 nét vẽ.
- Người dùng xác nhận bằng mắt: 4 dải màu đúng; cảm ứng "tốt như bản đầu".
- Ontology check: `errors=0 warnings=7`. `python .claude/hooks/test_handoff_hooks.py`: OK.

## Rủi ro
- Lịch sử `main` đã viết lại và force push ngày 2026-11-16 để commit message theo quy ước (một câu tiếng Anh, không dấu vết AI). Bản sao lịch sử cũ ở `refs/original/` trên máy, không có trên GitHub.
- `serial_log.py` reset board qua RTS khi mở cổng; khi nó đang chạy thì không nạp được (COM5 bận).
- Một lần ghi log 2 phút không thu được dòng chạm nào, chưa rõ nguyên nhân (có thể người dùng vẽ trước khi board khởi động lại xong).
- Chưa rõ vì sao LovyanGFX sai màu (Q11); nếu cần LovyanGFX cho LVGL sau này phải tìm ra.
