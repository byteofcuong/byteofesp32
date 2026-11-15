"""SessionStart: đưa bản bàn giao hiện tại vào context của session mới."""
import json
import os
import subprocess
import sys


def main() -> None:
    sys.stdout.reconfigure(encoding="utf-8")
    try:
        payload = json.load(sys.stdin)
    except ValueError:
        payload = {}
    # CLAUDE_PROJECT_DIR có thể rỗng trên Windows, nên tìm gốc repo bằng git.
    cwd = payload.get("cwd") or os.getcwd()
    found = subprocess.run(
        ["git", "-C", cwd, "rev-parse", "--show-toplevel"],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
    )
    root = found.stdout.strip() if found.returncode == 0 else cwd
    path = os.path.join(root, "docs", "handoff", "CURRENT.md")
    if not os.path.exists(path):
        return
    with open(path, encoding="utf-8") as handle:
        print("Bàn giao từ session trước (docs/handoff/CURRENT.md):\n\n" + handle.read())


if __name__ == "__main__":
    main()
