"""Stop: yêu cầu cập nhật bản bàn giao nếu đã có thay đổi mới hơn nó.

Lỗi của chính hook không được chặn người dùng, nên mọi lỗi đều thoát 0.
"""
import json
import os
import subprocess
import sys

HANDOFF = "docs/handoff/CURRENT.md"
REASON = (
    "Đã có thay đổi mới hơn docs/handoff/CURRENT.md. Trước khi kết thúc, hãy cập nhật file đó: "
    "đang làm gì, đã xong gì kèm lệnh kiểm chứng, việc tiếp theo, quyết định còn treo, bẫy đã gặp."
)


def git(root: str, *args: str) -> str:
    return subprocess.run(
        ["git", "-C", root, *args], capture_output=True, text=True, encoding="utf-8", errors="replace", check=True
    ).stdout


def dirty_paths(root: str) -> list[str]:
    """Các file thay đổi chưa commit, trừ chính bản bàn giao."""
    fields = git(root, "status", "--porcelain", "-z", "-uall").split("\0")
    paths: list[str] = []
    index = 0
    while index < len(fields):
        entry = fields[index]
        index += 1
        if not entry:
            continue
        status, path = entry[:2], entry[3:]
        if "R" in status or "C" in status:
            index += 1  # Trường kế tiếp là tên cũ của file.
        if not path.startswith("docs/handoff/"):
            paths.append(path)
    return paths


def is_stale(root: str) -> bool:
    handoff = os.path.join(root, HANDOFF)
    dirty = dirty_paths(root)
    if not os.path.exists(handoff):
        return bool(dirty)
    handoff_time = os.path.getmtime(handoff)
    for path in dirty:
        full = os.path.join(root, path)
        # File đã xoá không có mtime, coi là mới hơn bản bàn giao.
        if not os.path.exists(full) or os.path.getmtime(full) > handoff_time:
            return True
    head = git(root, "rev-parse", "HEAD").strip()
    if git(root, "log", "-1", "--format=%H", "--", HANDOFF).strip() == head:
        return False
    return int(git(root, "log", "-1", "--format=%ct", "HEAD").strip()) > handoff_time


def main() -> None:
    sys.stdout.reconfigure(encoding="utf-8")
    payload = json.load(sys.stdin)
    if payload.get("stop_hook_active"):
        return  # Đã nhắc một lần trong lượt này, không lặp vô hạn.
    # CLAUDE_PROJECT_DIR có thể rỗng trên Windows, nên tìm gốc repo bằng git.
    root = git(payload.get("cwd") or os.getcwd(), "rev-parse", "--show-toplevel").strip()
    if is_stale(root):
        print(json.dumps({"decision": "block", "reason": REASON}, ensure_ascii=False))


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f"handoff_stop: bỏ qua vì lỗi: {error}", file=sys.stderr)
