"""Bộ não: gọi model qua API kiểu OpenAI. Hiện chỉ dùng Muse qua muse2api (ADR 0006)."""

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Protocol

from .protocol import EXPRESSIONS

SYSTEM_PROMPT = (
    "Bạn là trợ lý giọng nói để bàn, hình một con cáo nhỏ trên màn hình. "
    "Trả lời bằng tiếng Việt, 1 đến 3 câu ngắn, giọng thân thiện, không dùng markdown, không dùng emoji. "
    "Bắt đầu MỖI câu trả lời bằng đúng một thẻ cảm xúc trong ngoặc vuông, chọn một trong: "
    + " ".join(f"[{e}]" for e in EXPRESSIONS)
    + ". Ví dụ: [happy] Chào bạn!"
)

_ANY_TAG = re.compile(r"\[([^\[\]\n]{1,80})\]")


@dataclass
class Reply:
    text: str
    expression: str


def parse_reply(raw: str) -> Reply:
    """Tách thẻ cảm xúc khỏi câu trả lời. Không có thẻ hoặc thẻ lạ thì dùng neutral.

    muse2api đọc chữ từ trang web nên đôi khi chép lặp một đoạn (đo 2026-11-17:
    "[happy] A\\ntimer[happy] A\\n[timer 300]"). Vì vậy lấy thẻ cảm xúc CUỐI cùng, giữ phần chữ sau nó,
    và bỏ mọi thẻ khác trong ngoặc vuông (thẻ hành động, docs/design/behavior.md).
    """
    last = None
    for m in _ANY_TAG.finditer(raw):
        if m.group(1).strip().lower() in EXPRESSIONS:
            last = m
    if last is None:
        expr, body = "neutral", raw
    else:
        expr, body = last.group(1).strip().lower(), raw[last.end():]
    text = re.sub(r"\s+", " ", _ANY_TAG.sub(" ", body)).strip()
    return Reply(text, expr)


class Brain(Protocol):
    async def ask(self, question: str) -> Reply: ...


class OpenAICompatBrain:
    """BrainProvider dùng chuẩn /v1/chat/completions (muse2api, sau này Meta Model API)."""

    def __init__(self, base_url: str, api_key: str, model: str, timeout_s: float = 90.0):
        from openai import AsyncOpenAI  # Nhập muộn để test không cần mạng.

        self._client = AsyncOpenAI(base_url=base_url, api_key=api_key, timeout=timeout_s, max_retries=0)
        self._model = model

    async def ask(self, question: str) -> Reply:
        resp = await self._client.chat.completions.create(
            model=self._model,
            messages=[
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": question},
            ],
        )
        return parse_reply(resp.choices[0].message.content or "")
