# byteofesp32

Trợ lý giọng nói để bàn (kiểu Meta Muse Charm) chạy trên board **ESP32-3248S035R** (màn 3.5"), nói chuyện qua một server ở nhà. Người dùng là chủ repo, mới làm quen phần cứng. Trạng thái: **giai đoạn thiết kế, chưa có code**. Xem `docs/handoff/CURRENT.md`.

## Đọc trước khi làm
- `docs/handoff/CURRENT.md`: đang ở đâu, việc tiếp theo. Hook SessionStart tự nạp file này. Bản cũ trong `docs/handoff/archive/`.
- `docs/hardware/esp32-3248s035r.md`: pinout, cổng, nguồn. Mọi code phần cứng phải khớp file này.
- `docs/decisions/`: quyết định kiến trúc (ADR). Muốn làm khác một ADR thì viết ADR mới thay thế nó.
- `docs/open-questions.md`: điều chưa xác minh. Không trình bày chúng như sự thật.
- `.claude/ontology/ontology.md`: khái niệm của dự án và tên gọi thống nhất giữa code, docs và giao diện. Đọc trước khi đổi thiết kế.
- `docs/private/` (git bỏ qua, chỉ có trên máy): tài khoản, gói dịch vụ. Chỉ đọc khi cần.

## Stack (dự kiến, chưa chốt)
- Firmware: ESP32 classic (không PSRAM), Arduino qua PlatformIO hoặc ESP-IDF: sẽ chốt bằng ADR. LVGL cho giao diện.
- Server: Python trên PC Windows (sau có thể chuyển sang máy chạy 24/7): WebSocket, openWakeWord, faster-whisper, Piper, client LLM (Muse API hoặc Claude API).
- Firmware đã chốt (ADR 0005): PlatformIO + Arduino (espressif32 8.0.0) + TFT_eSPI 2.5.43 cho màn; cảm ứng tự đọc kiểu LovyanGFX.
- Công cụ trên máy: Python 3.13.7, Node, git, gh, esptool 5.6.0 và PlatformIO 6.2.0 (pip --user, gọi bằng `python -m esptool`, `python -m platformio`). Board ở cổng **COM5**.

## Layout
- `docs/`: kho kiến thức (xem trên).
- `firmware/`: PlatformIO project (`platformio.ini` chứa chân màn/cảm ứng, `src/main.cpp`, `include/board_cyd35.hpp`, `tools/serial_log.py`).
- `server/`: chưa tạo.
- `backups/`: bản sao lưu flash gốc (git bỏ qua).
- `.claude/`: skill `handoff`; hook bàn giao (SessionStart nạp bản bàn giao, Stop nhắc khi bàn giao cũ, PreCompact); ontology.

## Verification commands
Shell: Git Bash, chạy từ gốc repo. Chạy ngày 2026-11-16.
```
python -I ~/.claude/skills/ontology/scripts/check_ontology.py .claude/ontology/ontology.json --md .claude/ontology/ontology.md   # errors=0, warnings=8 (quy tắc chưa có test tự động)
python .claude/hooks/test_handoff_hooks.py   # test các hook bàn giao
```
Firmware (chạy từ `firmware/`, cần board cắm COM5 cho hai lệnh sau):
```
python -m platformio run -e cyd35              # SUCCESS, RAM ~7%, flash ~26%
python -m platformio run -e cyd35 -t upload    # nạp lên board, SUCCESS
python tools/serial_log.py COM5 8              # reset board, in log: [lcd] init ok 480x320, [ready] mascot, [mascot] fps≈27–30
```
Chưa có test tự động cho firmware. Phần nhìn (màu, cảm ứng) phải nhờ người dùng xác nhận; cảm ứng có số đo `[touchtest] worst=` sau khi bấm "CAN CHINH". Khi đang ghi log, đừng nạp (cổng COM5 bận).

## Definition of done
Một thay đổi xong khi các lệnh kiểm chứng ở trên đã chạy lại sau thay đổi và pass, kết quả thật được báo lại. Thay đổi về phần cứng hoặc kiến trúc thì cập nhật luôn `docs/hardware/`, ADR và ontology (chạy lại lệnh check).

## Quy tắc
- Board là **3.5" ESP32-3248S035R**, không phải 2.8" ESP32-2432S028R. Code cho bản 2.8" làm màn đen (đèn nền ở GPIO27, không phải GPIO21).
- Màn dùng TFT_eSPI, không dùng LovyanGFX (sai màu). Cảm ứng không dùng `getTouch()` của TFT_eSPI (đứt nét, đoạn lạ).
- Trước lần nạp firmware đầu tiên: sao lưu flash gốc (`esptool read_flash 0 0x400000 ...`). File `*.bin` không commit.
- API key (Muse, Claude) chỉ nằm trên server, trong `server/.env`. Không đưa lên board, không commit.
- Repo công khai trên GitHub: thông tin cá nhân, tài khoản, số dư chỉ ghi vào `docs/private/`.
- Khi hướng dẫn phần cứng cho người dùng: nêu chữ in trên board, cắm hay hàn, và cực pin.
- Trả lời người dùng bằng tiếng Việt, thuật ngữ kỹ thuật giữ tiếng Anh.
- Commit message: một câu tiếng Anh ngắn, chuyên nghiệp, dạng mệnh lệnh (ví dụ "Add display and touch firmware"). Không có phần thân, không có dòng `Co-Authored-By` hay bất kỳ dấu vết nào của AI.

## Cuối session
Chạy skill `handoff`: chuyển bản cũ vào `docs/handoff/archive/`, viết `docs/handoff/CURRENT.md` mới; cập nhật phần cứng, ADR, open questions, ontology nếu có thay đổi.

## Team
- Codex: chưa hỏi người dùng. Hỏi trước lần đầu gửi code sang Codex và ghi câu trả lời vào đây.
- Chưa có lệnh chạy lâu, cổng hay tài nguyên dùng chung.
