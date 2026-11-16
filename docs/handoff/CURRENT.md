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
- ADR 0006: Muse gọi qua `muse2api` (github.com/czg86389-hub/muse2api, đã đọc mã ở commit `19db32a3d6ef`): chạy ngoài repo bằng Docker Desktop, cổng `127.0.0.1:18610`, ghim commit, một tài khoản, không dùng nút nâng cấp trực tuyến. HomeServer gọi `http://127.0.0.1:18610/v1` (kiểu OpenAI), khóa `MUSE2API_KEY` trong `server/.env`.
- Chỉ dùng Muse (người dùng quyết 2026-11-16): FastBrain và SlowBrain đều là Muse, không dùng Claude API. Chấp nhận độ trễ 8–20s; con cáo hiện `thinking` trong lúc chờ.
- Miệng giữ khung ω, chỉ biến tấu bằng phụ kiện. Mắt dùng mí cắt cong. Đơn sắc, không ký hiệu emoji.
- Vòm cười và mắt tim chuyển gần như tức thì. Các thông số khác dùng lò xo k300 c30.
- Hình nặng được vẽ vào ảnh đệm (tai, cụm miệng, vòm cười), chỉ vẽ lại khi đổi. Không dùng nét dày `drawWideLine` lớn.
- Các nhánh cũ `firmware/display-touch`, `feature/fox-mascot`, `feature/expressions` đã merge, xóa được.

## Việc tiếp theo
1. `muse2api` đã clone ở `D:\Direc of code\pet_project\muse2api` (ghim `19db32a3d6ef`), `docker-compose.yml` sửa cổng `127.0.0.1:18610:18610` + `MUSE2API_MANAGED_DEPLOYMENT=1`, khóa ngẫu nhiên trong `.env` của nó. Container đã chạy (2026-11-16): `docker ps` báo `127.0.0.1:18610->18610/tcp`; `/v1/models` không khóa → 401, có khóa (gọi trong container) → 200 với `muse-spark`, `muse-image`, `muse-video`. Người dùng đã nạp extension và nhập cookie (`POST /admin/accounts` 200). Chat thử qua `http://127.0.0.1:18610/v1/chat/completions` (model `muse-spark`, stream): HTTP 200, tiếng Việt tốt, nhưng chữ đầu tiên mất 10.9–20.1s (chi tiết `docs/research/muse.md`). Script thử nằm ở scratchpad, chưa đưa vào repo.
   - Đo thêm 3 câu cực ngắn ("chào", "1+1", "ok"): 12.5s, 12.5s, 7.9s. Độ trễ do Muse, không do muse2api (nó tái dùng cùng thread, không mở trang mới). Người dùng chọn chỉ dùng Muse.
2. Bước 2 lộ trình: nối board với server Python trên PC qua WiFi + WebSocket, server gửi trạng thái và biểu cảm. Lệnh Serial `e`/`s` là tiền đề cho giao thức này.
3. Khi có loa: phát âm thanh qua DAC GPIO26, miệng D há theo biên độ thật thay cho nhịp giả.
4. Chờ người dùng:
   - mua loa, mic INMP441, pin;
   - Q1: ảnh mã chip 8 chân cạnh `2R2`;
   - Q2: đo cổng BAT1;
   - Q6 đã trả lời (ADR 0006): người dùng chọn gọi Muse qua `muse2api` (đóng gói ngược web muse.ai), dù đã được báo rủi ro. Đăng nhập dev.meta.ai (API chính thức) bị lỗi "Token exchange failed".
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
- Docker Desktop trên máy này hay sập lúc khởi động vì file socket cũ ("The file cannot be accessed by the system" ở `%LOCALAPPDATA%\Docker\run\dockerInference`, `%LOCALAPPDATA%\docker-secrets-engine\engine.sock`). Cách gỡ 2026-11-16: tắt Docker, đổi tên thư mục chứa socket (`run` → `run.old-…`, `docker-secrets-engine` → `….old-…`), bật lại. Xóa từng file không được.
- Thay đổi docs chưa commit trên `main`: ADR 0006, `docs/decisions/README.md`, ontology (thêm `Muse2Api`, R10), `docs/open-questions.md`, `docs/research/muse.md`, bản bàn giao này. Ontology check sau khi sửa: `errors=0 warnings=9`. Gộp vào nhánh bước 2 (không commit thẳng lên `main`).
- `muse2api`: trái điều khoản Meta (tài khoản có thể bị khóa), hỏng khi Meta đổi web, cookie hết hạn phải nhập lại. Mặc định nó lắng nghe `0.0.0.0` và `docker-compose.yml` mở `18610:18610` ra mọi mạng: phải đổi thành `127.0.0.1:18610:18610`. `data/accounts.json` của nó chứa cookie.
- Push SPI ~26ms/khung là trần (~30 fps); thêm vùng vẽ là tụt fps ngay.
- Lịch sử `main` từng viết lại và force push một lần.
- `serial_log.py` và script đo mở COM5 sẽ reset board; khi chúng chạy thì không nạp được.
- Q11: chưa rõ vì sao LovyanGFX sai màu.
- Quét fps: `cd firmware && python tools/fps_sweep.py COM5` (mất ~2.5 phút, mở COM5 nên reset board).
