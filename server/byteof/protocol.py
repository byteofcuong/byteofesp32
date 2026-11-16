"""Tin nhắn giữa board và server (docs/design/protocol.md). Tên phải khớp firmware/src/mascot.cpp."""

from __future__ import annotations

import json

STATES = ("idle", "listening", "thinking", "speaking", "notifying")
EXPRESSIONS = (
    "neutral", "happy", "laugh", "excited", "love", "shy", "smug", "sad",
    "cry", "angry", "surprised", "scared", "confused", "bored", "sleepy",
)
TOUCH_KINDS = ("tap", "double", "long")

CLOSE_BAD_HELLO = 4001  # Tin đầu không phải hello, sai token, hoặc chào quá chậm.
CLOSE_REPLACED = 4002   # Có board khác vừa chào thành công.


class ProtocolError(ValueError):
    pass


def encode(t: str, **fields) -> str:
    return json.dumps({"t": t, **fields}, ensure_ascii=False, separators=(",", ":"))


def state(name: str) -> str:
    if name not in STATES:
        raise ProtocolError(f"unknown state {name!r}")
    return encode("state", state=name)


def expression(name: str, hold: float | None = None) -> str:
    """hold (giây): board giữ biểu cảm rồi tự phai về neutral (docs/design/behavior.md). None: giữ tới lệnh sau."""
    if name not in EXPRESSIONS:
        raise ProtocolError(f"unknown expression {name!r}")
    if hold is None:
        return encode("expression", expression=name)
    if not 0 < hold <= 600:
        raise ProtocolError(f"hold out of range: {hold}")
    return encode("expression", expression=name, hold=round(hold, 1))


def welcome() -> str:
    return encode("welcome")


def decode(raw: str | bytes) -> dict:
    """Đọc một tin từ board; sai định dạng thì báo ProtocolError."""
    if isinstance(raw, bytes):
        raise ProtocolError("binary frames are not used in v1")
    try:
        msg = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise ProtocolError(f"bad json: {exc}") from exc
    if not isinstance(msg, dict) or not isinstance(msg.get("t"), str):
        raise ProtocolError("message must be an object with string field 't'")
    t = msg["t"]
    if t == "hello":
        if not isinstance(msg.get("token"), str):
            raise ProtocolError("hello needs a string token")
    elif t == "touch":
        if msg.get("kind") not in TOUCH_KINDS:
            raise ProtocolError(f"unknown touch kind {msg.get('kind')!r}")
    else:
        raise ProtocolError(f"unknown message type {t!r}")
    return msg
