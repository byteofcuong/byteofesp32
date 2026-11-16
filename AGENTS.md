# byteofesp32

Trợ lý giọng nói để bàn (kiểu Meta Muse Charm) chạy trên board **ESP32-3248S035R** (màn 3.5"), nói chuyện qua một server ở nhà. Người dùng là chủ repo, mới làm quen phần cứng. Trạng thái: **linh vật chạy trên board; board cài WiFi bằng điện thoại và nối server Python qua Cloudflare Tunnel (wss://); server gọi Muse; chưa có âm thanh**. Xem `docs/handoff/CURRENT.md`.

## Đọc trước khi làm
- `docs/handoff/CURRENT.md`: đang ở đâu, việc tiếp theo. Hook SessionStart tự nạp file này. Bản cũ trong `docs/handoff/archive/`.
- `docs/hardware/esp32-3248s035r.md`: pinout, cổng, nguồn. Mọi code phần cứng phải khớp file này.
- `docs/decisions/`: quyết định kiến trúc (ADR). Muốn làm khác một ADR thì viết ADR mới thay thế nó.
- `docs/open-questions.md`: điều chưa xác minh. Không trình bày chúng như sự thật.
- `.claude/ontology/ontology.md`: khái niệm của dự án và tên gọi thống nhất giữa code, docs và giao diện. Đọc trước khi đổi thiết kế.
- `docs/private/` (git bỏ qua, chỉ có trên máy): tài khoản, gói dịch vụ. Chỉ đọc khi cần.

## Stack
- Firmware: ESP32 classic (không PSRAM).
- Server: Python 3.13 trên PC Windows bật suốt (`server/`, package `byteof`, venv `server/.venv`): WebSocket (`websockets` 17.4), Brain gọi Muse qua muse2api (ADR 0006). Ra Internet qua Cloudflare Tunnel (ADR 0008). Sau: openWakeWord, faster-whisper, Piper.
- Firmware đã chốt (ADR 0005): PlatformIO + Arduino (espressif32 8.0.0) + TFT_eSPI 2.5.43 cho màn; cảm ứng tự đọc kiểu LovyanGFX. Mạng (ADR 0007, 0008): WebSockets 2.7.3 + ArduinoJson 7.4.3, TLS bằng bộ CA Mozilla nhúng, phân vùng `huge_app.csv`.
- Công cụ trên máy: Python 3.13.7, Node, git, gh, esptool 5.6.0 và PlatformIO 6.2.0 (pip --user, gọi bằng `python -m esptool`, `python -m platformio`), Docker Desktop, `cloudflared` 2026.11.0 (`C:\Program Files (x86)\cloudflared\cloudflared.exe`). Board ở cổng **COM5**.

## Layout
- `docs/`: kho kiến thức (xem trên).
- `firmware/`: PlatformIO project (`platformio.ini` chứa chân màn/cảm ứng, `src/main.cpp`, `src/mascot.*` linh vật, `src/touch.*` cảm ứng, `src/behavior.*` cảm xúc tự xuất hiện (docs/design/behavior.md), `src/server_link.*` WiFi + WebSocket (task lõi 0), `src/net_config.*` WiFi và địa chỉ server trong NVS, `src/portal.*` trang cài WiFi, `include/board_cyd35.hpp`, `include/secrets.example.h`, `certs/` bộ CA, `tools/serial_log.py`, `tools/fps_sweep.py`).
- `server/`: HomeServer (`byteof/`: `hub.py` WebSocket, `brain.py`, `turn.py`, `protocol.py`, `config.py`; `tests/`; `tools/make_secrets.py` tạo `server/.env` và `firmware/include/secrets.h`; mẫu `env.example`).
- `backups/`: bản sao lưu flash gốc (git bỏ qua).
- `.claude/`: skill `handoff`; hook bàn giao (SessionStart nạp bản bàn giao, Stop nhắc khi bàn giao cũ, PreCompact); ontology.

## Verification commands
Shell: Git Bash, chạy từ gốc repo. Chạy ngày 2026-11-16.
```
python -I ~/.claude/skills/ontology/scripts/check_ontology.py .claude/ontology/ontology.json --md .claude/ontology/ontology.md   # errors=0, warnings=8 (quy tắc chưa có test tự động)
python .claude/hooks/test_handoff_hooks.py   # test các hook bàn giao
cd server && .venv/Scripts/python -m pytest -q   # 27 passed (giao thức, thẻ cảm xúc, hub với board giả)
```
Firmware (chạy từ `firmware/`, cần board cắm COM5 cho hai lệnh sau):
```
python -m platformio run -e cyd35              # SUCCESS, RAM ~16%, flash ~40% (của 3MB)
python -m platformio run -e cyd35 -t upload    # nạp lên board, SUCCESS
python tools/serial_log.py COM5 25             # reset board, in log: [mascot] band … free heap ~190KB, [link] status=online heap≈76KB
PYTHONIOENCODING=utf-8 python tools/fps_sweep.py COM5   # ~2.5 phút: fps từng biểu cảm/trạng thái (online: 18.7–30.2 fps, max khi chuyển ≤ 63ms)
```
Chưa có test tự động cho firmware. Phần nhìn (màu, cảm ứng) phải nhờ người dùng xác nhận; cảm ứng có số đo `[touchtest] worst=` sau khi bấm "CAN CHINH". Khi đang ghi log, đừng nạp (cổng COM5 bận).

## Definition of done
Một thay đổi xong khi các lệnh kiểm chứng ở trên đã chạy lại sau thay đổi và pass, kết quả thật được báo lại. Thay đổi về phần cứng hoặc kiến trúc thì cập nhật luôn `docs/hardware/`, ADR và ontology (chạy lại lệnh check).

## Quy tắc
- Board là **3.5" ESP32-3248S035R**, không phải 2.8" ESP32-2432S028R. Code cho bản 2.8" làm màn đen (đèn nền ở GPIO27, không phải GPIO21).
- Màn dùng TFT_eSPI, không dùng LovyanGFX (sai màu). Cảm ứng không dùng `getTouch()` của TFT_eSPI (đứt nét, đoạn lạ).
- Trước lần nạp firmware đầu tiên: sao lưu flash gốc (`esptool read_flash 0 0x400000 ...`). File `*.bin` không commit.
- API key (Muse, Claude) chỉ nằm trên server, trong `server/.env`. Không đưa lên board, không commit. Công cụ của agent bị chặn đọc/sửa file `.env*`: nhờ người dùng sửa.
- WiFi không ghi trong code hay `secrets.h`: cài bằng điện thoại qua trang cài WiFi của board (ADR 0008). `secrets.h` chỉ có token và địa chỉ server mặc định.
- RAM board rất sát (ADR 0008): thêm ảnh đệm hay bộ đệm cho linh vật phải đo lại `heap=` trong log `[link]` và thử bắt tay TLS (`[link] status=online`).
- Repo công khai trên GitHub: thông tin cá nhân, tài khoản, số dư chỉ ghi vào `docs/private/`.
- Khi hướng dẫn phần cứng cho người dùng: nêu chữ in trên board, cắm hay hàn, và cực pin.
- Trả lời người dùng bằng tiếng Việt, thuật ngữ kỹ thuật giữ tiếng Anh.
- Commit message: một câu tiếng Anh ngắn, chuyên nghiệp, dạng mệnh lệnh (ví dụ "Add display and touch firmware"). Không có phần thân, không có dòng `Co-Authored-By` hay bất kỳ dấu vết nào của AI.

## Cuối session
Chạy skill `handoff`: chuyển bản cũ vào `docs/handoff/archive/`, viết `docs/handoff/CURRENT.md` mới; cập nhật phần cứng, ADR, open questions, ontology nếu có thay đổi.

## Team
- Codex: chưa hỏi người dùng. Hỏi trước lần đầu gửi code sang Codex và ghi câu trả lời vào đây.
- Dịch vụ chạy lâu: `muse2api` (ADR 0006), Docker container `muse2api`, cài ở thư mục anh em `../muse2api` (ngoài repo), cổng `127.0.0.1:18610`. Khóa nằm trong `../muse2api/.env`, không đọc ra chat. Bật: `docker compose up -d` trong thư mục đó (Docker Desktop phải chạy).
- HomeServer: `cd server && .venv/Scripts/python -m byteof` (cổng `127.0.0.1:8765`, console gõ câu hỏi). Đường hầm: `cloudflared` (tạm: quick tunnel `cloudflared tunnel --url http://localhost:8765`, địa chỉ đổi mỗi lần chạy; sẽ thay bằng named tunnel + tên miền).
- Cổng COM5 chỉ một tiến trình dùng: dừng `serial_log.py`/`fps_sweep.py` trước khi nạp.
