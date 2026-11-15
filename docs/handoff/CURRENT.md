---
date: 2026-11-16
branch: setup/knowledge-base
commit: 0565c72
---
# Handoff

## Mục tiêu
Làm trợ lý giọng nói để bàn kiểu Muse Charm trên board ESP32-3248S035R (3.5"). Session này: nhận dạng board, chốt kiến trúc, dựng kho kiến thức (AGENTS.md, docs/, ontology, hook và skill bàn giao). Xong khi session sau mở ra là biết đang ở đâu và làm gì tiếp.

## Trạng thái
- Xong: AGENTS.md + CLAUDE.md; `docs/hardware/` (pinout, cổng, mic, nguồn, danh sách mua); ADR 0001–0004; `docs/design/assistant.md`; `docs/research/`; `docs/open-questions.md` (Q1–Q10); ontology 23 khái niệm; hook bàn giao chép từ project Black-Room; skill handoff theo mẫu claude-village.
- Đã commit toàn bộ khung trên nhánh `setup/knowledge-base` (tách từ `main` ở `0565c72`). Chưa merge vào `main`, chưa push. `docs/private/` bị git bỏ qua (repo công khai).
- Chưa có code firmware/server. Chưa mua loa, mic, pin. Chưa cài PlatformIO, esptool.

## Quyết định
- Board là 3.5" (người bán ghi nhầm 2.8"): đèn nền GPIO27, ST7796. Chân trống: 21, 22, 35.
- ADR 0001: board là thin client, server Python ở nhà làm phần nặng.
- ADR 0002: bộ não đổi được, Muse API hoặc Claude API, luôn bằng API key.
- ADR 0003: từ đánh thức xử lý trên server, chạm màn là cách dự phòng.
- ADR 0004: đã loại firmware Muse Gadget SDK (không hỗ trợ board 3.5", không có tiếng nói khi không PSRAM).
- Đã loại: dùng thẻ SD thay RAM; model từ đánh thức trên chip; đăng nhập claude.ai trong server; combo có MAX98357.

## Việc tiếp theo
1. Hỏi người dùng có muốn merge `setup/knowledge-base` vào `main` và push lên GitHub không (repo công khai).
2. Chờ người dùng: ảnh rõ mã chip 8 chân cạnh `2R2` (Q1), đo bước chân BAT1 (Q2), kiểm tra token Muse có API key không (Q6).
3. Cài esptool (`pip install esptool`), sao lưu flash gốc: `python -m esptool --port COMx read_flash 0 0x400000 cyd_factory_backup.bin`.
4. Chốt framework firmware (Q9) bằng ADR 0005, dựng project dò pin trong `firmware/`: màn, đèn nền GPIO27, cảm ứng, LED, loa, ADC GPIO34/35.

## Kiểm chứng
- `python .claude/hooks/test_handoff_hooks.py`: Ran 13 tests, OK.
- Ontology check với `--md`: `classes=23 relations=13 states=2 enums=2 rules=8 anchors_ok=61 errors=0 warnings=8` (8 cảnh báo: quy tắc chưa có test vì chưa có code).
- Ontology `--hook`: không in gì, exit 0.
- `git check-ignore`: `docs/private/accounts.md` bị bỏ qua đúng.
- Hook trong `.claude/settings.json` chỉ có hiệu lực từ session sau; chưa thấy chúng chạy thật trong một session.

## Rủi ro
- Phần nguồn (chip sạc, SW1, chân đo pin) suy ra từ linh kiện, chưa có tài liệu.
- Giá linh kiện và thông tin Muse API lấy từ nguồn thứ cấp.
- Neo ontology trỏ vào tài liệu; số dòng trong link của `ontology.md` lệch nếu sửa AGENTS.md hay docs (chạy `--resolve` để sửa).
