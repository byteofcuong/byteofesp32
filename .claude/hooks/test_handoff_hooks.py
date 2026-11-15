"""Test các hook bàn giao trên repo git tạm. Chạy: python .claude/hooks/test_handoff_hooks.py"""
import json
import os
import subprocess
import sys
import tempfile
import time
import unittest

HOOKS = os.path.dirname(os.path.abspath(__file__))
HANDOFF = os.path.join("docs", "handoff", "CURRENT.md")


def run_hook(name, repo, payload=None):
    env = {k: v for k, v in os.environ.items() if k != "CLAUDE_PROJECT_DIR"}
    env["PYTHONIOENCODING"] = "utf-8"
    return subprocess.run(
        [sys.executable, "-I", os.path.join(HOOKS, name)],
        input=json.dumps({"cwd": repo, **(payload or {})}),
        capture_output=True, text=True, encoding="utf-8", env=env, check=True,
    ).stdout


class HandoffHooks(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(ignore_cleanup_errors=True)
        self.repo = self.tmp.name
        self.git("init", "-q")
        self.git("config", "user.email", "t@example.com")
        self.git("config", "user.name", "t")
        self.write(HANDOFF, "bàn giao đầu")
        self.write("a.txt", "a")
        self.git("add", ".")
        self.git("commit", "-qm", "init")
        self.set_time(HANDOFF, time.time() + 5)

    def tearDown(self):
        self.tmp.cleanup()

    def git(self, *args):
        subprocess.run(["git", "-C", self.repo, *args], check=True, capture_output=True)

    def write(self, rel, text):
        path = os.path.join(self.repo, rel)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as handle:
            handle.write(text)

    def set_time(self, rel, when):
        os.utime(os.path.join(self.repo, rel), (when, when))

    def assert_blocks(self):
        self.assertEqual(json.loads(run_hook("handoff_stop.py", self.repo))["decision"], "block")

    def assert_silent(self):
        self.assertEqual(run_hook("handoff_stop.py", self.repo), "")

    def test_sach_thi_im_lang(self):
        self.assert_silent()

    def test_file_moi_hon_thi_chan(self):
        self.write("b.txt", "b")
        self.set_time("b.txt", time.time() + 60)
        self.assert_blocks()

    def test_file_cu_hon_ban_giao_thi_im_lang(self):
        self.write("b.txt", "b")
        self.set_time("b.txt", time.time() - 60)
        self.assert_silent()

    def test_khong_lap_khi_stop_hook_active(self):
        self.write("b.txt", "b")
        self.set_time("b.txt", time.time() + 60)
        self.assertEqual(run_hook("handoff_stop.py", self.repo, {"stop_hook_active": True}), "")

    def test_chi_sua_ban_giao_thi_im_lang(self):
        self.write(HANDOFF, "mới")
        self.assert_silent()

    def test_xoa_file_thi_chan(self):
        os.remove(os.path.join(self.repo, "a.txt"))
        self.assert_blocks()

    def test_thieu_ban_giao_va_co_thay_doi_thi_chan(self):
        self.git("rm", "-qf", HANDOFF)
        self.git("commit", "-qm", "xoá bàn giao")
        self.write("b.txt", "b")
        self.assert_blocks()

    def test_commit_moi_khong_kem_ban_giao_thi_chan(self):
        self.write("b.txt", "b")
        self.git("add", "b.txt")
        self.git("commit", "-qm", "b")
        self.set_time(HANDOFF, time.time() - 60)
        self.assert_blocks()

    def test_commit_kem_ban_giao_thi_im_lang(self):
        self.write("b.txt", "b")
        self.write(HANDOFF, "mới")
        self.git("add", ".")
        self.git("commit", "-qm", "b")
        self.set_time(HANDOFF, time.time() - 60)
        self.assert_silent()

    def test_ngoai_git_khong_lam_hong(self):
        with tempfile.TemporaryDirectory() as outside:
            self.assertEqual(run_hook("handoff_stop.py", outside), "")

    def test_dung_o_thu_muc_con_van_tim_thay_ban_giao(self):
        sub = os.path.join(self.repo, "sub")
        os.makedirs(sub)
        self.assertIn("bàn giao đầu", run_hook("handoff_session_start.py", sub))
        self.write("sub/b.txt", "b")
        self.set_time("sub/b.txt", time.time() + 60)
        self.assertEqual(json.loads(run_hook("handoff_stop.py", sub))["decision"], "block")

    def test_session_start_in_ban_giao(self):
        self.assertIn("bàn giao đầu", run_hook("handoff_session_start.py", self.repo))

    def test_precompact_nhac_cap_nhat(self):
        self.assertIn("CURRENT.md", run_hook("handoff_precompact.py", self.repo))


if __name__ == "__main__":
    unittest.main()
