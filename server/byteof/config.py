"""Cấu hình từ server/.env (mẫu: server/.env.example). Không in khóa ra log."""

from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path

ENV_FILE = Path(__file__).resolve().parent.parent / ".env"


@dataclass
class Config:
    host: str
    port: int
    device_token: str
    brain_base_url: str
    brain_api_key: str
    brain_model: str


def load() -> Config:
    from dotenv import load_dotenv

    load_dotenv(ENV_FILE)
    missing = [k for k in ("DEVICE_TOKEN", "MUSE2API_KEY") if not os.environ.get(k)]
    if missing:
        raise SystemExit(f"missing {', '.join(missing)} in {ENV_FILE} (run: python tools/make_secrets.py)")
    return Config(
        host=os.environ.get("BYTEOF_HOST", "127.0.0.1"),
        port=int(os.environ.get("BYTEOF_PORT", "8765")),
        device_token=os.environ["DEVICE_TOKEN"],
        brain_base_url=os.environ.get("MUSE2API_BASE_URL", "http://127.0.0.1:18610/v1"),
        brain_api_key=os.environ["MUSE2API_KEY"],
        brain_model=os.environ.get("BRAIN_MODEL", "muse-spark"),
    )
