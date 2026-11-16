---
date: 2026-11-17
branch: feature/server-link
commit: 42c473f (+ thay đổi chưa commit của bước 2)
---
# Handoff

## Mục tiêu
Trợ lý giọng nói để bàn trên board ESP32-3248S035R. Bước 1 (linh vật trên màn) xong. Phần đại tu biểu cảm cũng đã xong và người dùng duyệt trên màn thật ("Ổn, commit").

## Trạng thái
- `feature/expressions` (818f4a3) đã merge fast-forward vào `main` và push lên origin.
- Có 15 biểu cảm: neutral, happy, laugh, excited, love, shy, smug, sad, cry, angry, surprised, scared, confused, bored, sleepy.
- Bộ thông số đầy đủ, quy tắc thiết kế và những phương án đã bị chê nằm trong `docs/design/mascot/spec.md`. Bản mẫu là `docs/design/mascot/prototype.html`.
- Cảm xúc tự xuất hiện (2026-11-17, `docs/design/behavior.md`, `firmware/src/behavior.*`): đã BỎ chạm đôi đổi biểu cảm và giữ đổi trạng thái. Ba lớp: hội thoại (server gửi cảm xúc kèm `hold` = thời gian nói + 3s, board tự phai), phản xạ chạm khi rảnh (vuốt ve → happy, chọc liên tục → angry, giữ tay → love/shy, chạm lúc ngủ → giật mình rồi ngái ngủ), đời sống khi rảnh (chán → buồn ngủ → ngủ; ngày 60/120/180s, đêm 22–6h 30/45/75s, UTC+7 qua `configTzTime`). Vào `thinking` thì gật đầu, nghĩ lâu đổi bên liếc mỗi 6s. Giữ > 5s vẫn là hiệu chỉnh cảm ứng.
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
0. Đã tắt (2026-11-17, người dùng yêu cầu): server `byteof`, quick tunnel `cloudflared` (địa chỉ cũ hết hiệu lực; lần sau chạy lại thì địa chỉ mới, phải sửa `BYTEOF_SERVER_URL` hoặc trang cài WiFi), container `muse2api` (`docker stop`; Docker Desktop vẫn chạy vì có container dự án khác `vietstrain-*`, KHÔNG tắt Docker). `muse2api` có `restart: always` nên Docker khởi động lại thì nó tự lên. Bật lại: `docker start muse2api`, `cd server && .venv/Scripts/python -m byteof`, `cloudflared tunnel --url http://localhost:8765`.
1. `muse2api` đã clone ở `D:\Direc of code\pet_project\muse2api` (ghim `19db32a3d6ef`), `docker-compose.yml` sửa cổng `127.0.0.1:18610:18610` + `MUSE2API_MANAGED_DEPLOYMENT=1`, khóa ngẫu nhiên trong `.env` của nó. Container đã chạy (2026-11-16): `docker ps` báo `127.0.0.1:18610->18610/tcp`; `/v1/models` không khóa → 401, có khóa (gọi trong container) → 200 với `muse-spark`, `muse-image`, `muse-video`. Người dùng đã nạp extension và nhập cookie (`POST /admin/accounts` 200). Chat thử qua `http://127.0.0.1:18610/v1/chat/completions` (model `muse-spark`, stream): HTTP 200, tiếng Việt tốt, nhưng chữ đầu tiên mất 10.9–20.1s (chi tiết `docs/research/muse.md`). Script thử nằm ở scratchpad, chưa đưa vào repo.
   - Đo thêm 3 câu cực ngắn ("chào", "1+1", "ok"): 12.5s, 12.5s, 7.9s. Độ trễ do Muse, không do muse2api (nó tái dùng cùng thread, không mở trang mới). Người dùng chọn chỉ dùng Muse.
2. Bước 2 (nhánh `feature/server-link`, CHƯA commit phần này; commit `42c473f` chỉ có docs Muse):
   - **Chạy trên board (2026-11-17):** board cài WiFi bằng điện thoại (người dùng đã cài xong), vào server qua `wss://` quick tunnel, `[link] status=online heap≈76KB`; server thấy `device online fw=0.3.0`. Quét fps khi online: 23.4–30.3.
   - Đã viết: ADR 0007, ADR 0008 (cài WiFi bằng điện thoại + Cloudflare Tunnel + TLS + ngân sách RAM), `docs/design/protocol.md`, `server/` (24 test pass), firmware `server_link`/`net_config`/`portal`, `firmware/certs/`, spec linh vật (mục RAM), ontology (WifiSetup, ServerTunnel, DeviceToken, R11, R12; check errors=0 warnings=11), AGENTS.md.
   - RAM: linh vật giảm từ ~150KB xuống ~87KB (một ảnh tai + lật gương; tai và vòm cười lưu độ phủ 8-bit, dán qua bảng màu `blitMask`; dải vẽ 40 dòng; chấm báo tạo khi cần). Lúc cài WiFi, `Mascot::end()` trả hết.
   - Còn:
     a. Người dùng đã hỏi thử Muse (bản server cũ): `[happy] Chào …` chạy. Còn: người dùng Ctrl+C rồi chạy lại server ở tab "byteof server" (agent không được dừng tab người dùng đã gõ) để nạp code mới (hold); thử phản xạ chạm trên board mới nạp. Đã kiểm trên board: lớp rảnh chạy đúng (log `[behavior] idle -> bored`, `-> sleepy`, `asleep=1`). Server 27 test pass. Nghiên cứu công cụ: Muse không có function calling, nhưng làm theo thẻ cuối câu (`[timer 300]`, `[remind 18:30 …]`, thử 2026-11-17); chờ người dùng chọn công cụ và tính cách/tên con cáo.
     b. Người dùng sửa `server/.env`: `BYTEOF_HOST=0.0.0.0` → `127.0.0.1` (agent bị chặn đụng `.env`).
     c. Người dùng mua tên miền + tạo tài khoản Cloudflare; agent tạo named tunnel (`cloudflared tunnel login/create/route dns`), chạy như service Windows, đổi `BYTEOF_SERVER_URL` (hoặc sửa ở trang cài WiFi).
     d. Server và cloudflared phải tự bật khi PC khởi động (chưa làm).
     e. Commit (hỏi người dùng trước).
   - Lỗi đã sửa 2026-11-17: script dọn mã đo tạm xóa nhầm khối vẽ ảnh đệm miệng (tìm `
  }` sai) → con cáo mất miệng; đã lấy lại từ git và soát mọi dòng bị xóa trong diff `mascot.cpp`. Bài học: dọn bằng Edit tool có chuỗi khớp đúng, không cắt theo dấu `}`.
   - fps online đo lại khi đủ miệng: 18.7 (cry) – 30.2; tắt WiFi chỉ kém bản gốc ~1 fps; WiFi làm mặt nặng chậm thêm 3–4 fps (không do nhịp hỏi mạng). Lệnh Serial `e`/`s` giờ đi qua lớp hành vi (trước đó lớp rảnh chen vào làm số đo trạng thái sai).
   - Bẫy: file nguồn firmware dùng CRLF (script sửa phải đổi `
` → `
`); `fps_sweep.py` cần `PYTHONIOENCODING=utf-8`; mở COM5 reset board; WiFiManager 2.0.17 không hợp Arduino core 3 (đã tự viết portal); `cryptography` cài ở user site nên `gen_crt_bundle.py` chạy không `-I`.
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
- `muse2api`: trái điều khoản Meta (tài khoản có thể bị khóa), hỏng khi Meta đổi web, cookie hết hạn phải nhập lại. Mặc định nó lắng nghe `0.0.0.0` và `docker-compose.yml` mở `18610:18610` ra mọi mạng: phải đổi thành `127.0.0.1:18610:18610`. `data/accounts.json` của nó chứa cookie.
- Push SPI ~26ms/khung là trần (~30 fps); thêm vùng vẽ là tụt fps ngay.
- Lịch sử `main` từng viết lại và force push một lần.
- `serial_log.py` và script đo mở COM5 sẽ reset board; khi chúng chạy thì không nạp được.
- Q11: chưa rõ vì sao LovyanGFX sai màu.
- Quét fps: `cd firmware && python tools/fps_sweep.py COM5` (mất ~2.5 phút, mở COM5 nên reset board).
