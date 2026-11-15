"""PreCompact: yêu cầu bản tóm tắt giữ lại việc phải cập nhật bàn giao."""
import sys

sys.stdout.reconfigure(encoding="utf-8")
print(
    "Trong bản tóm tắt, ghi rõ việc đang dở và lệnh kiểm chứng đã chạy. "
    "Ngay sau khi nén, cập nhật docs/handoff/CURRENT.md nếu nó đã cũ."
)
