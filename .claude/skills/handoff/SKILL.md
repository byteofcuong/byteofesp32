---
name: handoff
description: Ghi bàn giao để session sau làm tiếp mà không cần đọc lại lịch sử. Dùng khi kết thúc một phần việc, trước khi nghỉ giữa chừng, khi context sắp đầy, hoặc khi người dùng nói "xong", "nghỉ", "bàn giao", "handoff", "/handoff".
---

# handoff

Session sau chỉ biết những gì có trong repo và trong file này. Hook SessionStart tự đưa `docs/handoff/CURRENT.md` vào context, nên viết ngắn (tối đa ~60 dòng). Hook Stop nhắc khi có thay đổi mới hơn file này.

## Các bước

1. Chạy `git status --short --branch` và `git rev-parse --short HEAD`.
2. Chạy các lệnh trong mục Verification commands của `AGENTS.md`. Chỉ ghi kết quả thật. Lệnh chưa chạy thì ghi là chưa chạy.
3. Cập nhật kho kiến thức nếu session này làm thay đổi:
   - Phần cứng (pin, cổng, linh kiện đã xác minh): `docs/hardware/`.
   - Quyết định kiến trúc mới hoặc đổi: ADR mới trong `docs/decisions/` (số tiếp theo, ngày, trạng thái) và dòng trong `docs/decisions/README.md`. Không sửa ADR đã chấp nhận; ADR mới ghi "Thay thế ADR NNNN".
   - Câu hỏi đã trả lời hoặc câu hỏi mới: `docs/open-questions.md`.
   - Khái niệm, quan hệ, trạng thái, quy tắc: `.claude/ontology/ontology.json` + `.md` (`generated`, `commit`), rồi chạy lệnh check trong `AGENTS.md`.
   - Thông tin tài khoản, gói dịch vụ: chỉ `docs/private/`.
4. Nếu `docs/handoff/CURRENT.md` đã có, chuyển nó sang `docs/handoff/archive/<YYYY-MM-DD-HHmm>.md`.
5. Viết `CURRENT.md` mới theo mẫu dưới. Giữ lại quyết định còn hiệu lực từ bản cũ, bỏ những gì đã xong và không còn ảnh hưởng.
6. Báo người dùng 3–5 dòng: đã làm gì, kiểm chứng ra sao, bước tiếp theo, câu hỏi đang chờ họ.

## Mẫu

```markdown
---
date: YYYY-MM-DD
branch: <branch>
commit: <short sha>
---
# Handoff

## Mục tiêu
<Đang làm gì, vì sao, xong là như thế nào.>

## Trạng thái
<Đã xong gì, đang dở gì, file nào chưa commit.>

## Quyết định
<Quyết định còn hiệu lực và lý do (trỏ tới ADR). Ghi cả phương án đã loại để session sau không thử lại.>

## Việc tiếp theo
<Bước cụ thể, làm được ngay (file, lệnh, tiêu chí xong). Ghi rõ việc đang chờ người dùng.>

## Kiểm chứng
<Lệnh đã chạy và kết quả thật.>

## Rủi ro
<Điều chưa kiểm chứng, giả định, lỗi đã biết. Không có thì ghi "Không có".>
```

## Quy tắc

- Không chép nội dung transcript, secret, API key, hay thông tin tài khoản vào handoff (repo công khai).
- Không ghi lại những gì đọc được từ code hoặc git log. Ghi điều chúng không cho biết.
- Việc dở dang: ghi file nào đang sửa dở và trạng thái kiểm chứng hiện tại (kể cả fail).
- Handoff là ghi chép của session trước, không phải sự thật: kiểm tra lại repo trước khi dựa vào nó.
