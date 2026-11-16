---
date: 2026-11-16
branch: main
commit: 818f4a3
---
# Handoff

## Mục tiêu
Trợ lý giọng nói để bàn trên board ESP32-3248S035R. Bước 1 (linh vật trên màn) xong. Phần đại tu biểu cảm cũng đã xong và người dùng duyệt trên màn thật ("Ổn, commit").

## Trạng thái
- `feature/expressions` (818f4a3) đã merge fast-forward vào `main` và push lên origin.
- Có 15 biểu cảm: neutral, happy, laugh, excited, love, shy, smug, sad, cry, angry, surprised, scared, confused, bored, sleepy.
- Bộ thông số đầy đủ, quy tắc thiết kế và những phương án đã bị chê nằm trong `docs/design/mascot/spec.md`. Bản mẫu là `docs/design/mascot/prototype.html`.
- Cử chỉ chạm trên board:
  - chạm 1 lần: tương tác (mắt thì nháy, mũi thì "boop", chỗ khác thì nhìn theo);
  - chạm 2 lần: đổi biểu cảm;
  - giữ 0.6–5 s: đổi trạng thái;
  - giữ > 5 s: hiệu chỉnh cảm ứng.
- Thông báo: viền màn hình sáng nhấp nháy + chấm báo "!" + nhún, vẫy tai.
- Lệnh Serial `e`/`s` để đổi biểu cảm/trạng thái (dùng cho đo fps, sau này cho server).

## Quyết định
- Miệng giữ khung ω, chỉ biến tấu bằng phụ kiện. Mắt dùng mí cắt cong. Đơn sắc, không ký hiệu emoji.
- Vòm cười và mắt tim chuyển gần như tức thì. Các thông số khác dùng lò xo k300 c30.
- Hình nặng được vẽ vào ảnh đệm (tai, cụm miệng, vòm cười), chỉ vẽ lại khi đổi. Không dùng nét dày `drawWideLine` lớn.
- Các nhánh cũ `firmware/display-touch`, `feature/fox-mascot`, `feature/expressions` đã merge, xóa được.

## Việc tiếp theo
1. Bước 2 lộ trình: nối board với server Python trên PC qua WiFi + WebSocket, server gửi trạng thái và biểu cảm. Lệnh Serial `e`/`s` là tiền đề cho giao thức này.
2. Khi có loa: phát âm thanh qua DAC GPIO26, miệng D há theo biên độ thật thay cho nhịp giả.
3. Chờ người dùng:
   - mua loa, mic INMP441, pin;
   - Q1: ảnh mã chip 8 chân cạnh `2R2`;
   - Q2: đo cổng BAT1;
   - Q6: token Muse có API key không.
   - Chưa hỏi người dùng về Codex.

## Kiểm chứng (2026-11-16)
- `cd firmware && python -m platformio run -e cyd35 -t upload`: SUCCESS. RAM 7.9%, flash 27.5%.
- Quét fps qua Serial (gửi `e`/`s`, đọc `[mascot] fps=… max=…`):
  - 23.6 (cry) đến 30.3 fps;
  - khung chậm nhất khi chuyển biểu cảm 41–52ms;
  - notifying 24.1 fps;
  - heap còn ~138KB.
- Ontology check `errors=0`. `python .claude/hooks/test_handoff_hooks.py`: OK. Kết quả thật xem lệnh chạy khi commit.
- Người dùng xác nhận trên màn thật: ổn.

## Rủi ro
- Push SPI ~26ms/khung là trần (~30 fps); thêm vùng vẽ là tụt fps ngay.
- Lịch sử `main` từng viết lại và force push một lần.
- `serial_log.py` và script đo mở COM5 sẽ reset board; khi chúng chạy thì không nạp được.
- Q11: chưa rõ vì sao LovyanGFX sai màu.
- Quét fps: `cd firmware && python tools/fps_sweep.py COM5` (mất ~2.5 phút, mở COM5 nên reset board).
