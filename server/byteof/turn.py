"""Một lượt hỏi đáp (docs/design/protocol.md, mục "Một lượt hỏi đáp hiện tại")."""

from __future__ import annotations

import asyncio
import logging

from .brain import Brain, Reply
from .hub import DeviceHub

log = logging.getLogger("byteof.turn")

MS_PER_CHAR = 65  # Chưa có loa: giữ "speaking" theo độ dài câu để miệng cử động vừa đủ.
MIN_SPEAK_S = 1.5
MAX_SPEAK_S = 12.0
LINGER_S = 3.0      # Cảm xúc câu trả lời còn lại sau khi nói xong rồi mới phai (docs/design/behavior.md).
ERROR_HOLD_S = 3.0


def speak_seconds(text: str) -> float:
    return min(MAX_SPEAK_S, max(MIN_SPEAK_S, len(text) * MS_PER_CHAR / 1000))


async def run_turn(hub: DeviceHub, brain: Brain, question: str, sleep=asyncio.sleep) -> Reply | None:
    # Board tự gật đầu khi vào "thinking" và liếc đi như đang nghĩ (che 8–20s của Muse).
    await hub.set_state("thinking")
    await hub.set_expression("neutral")
    try:
        reply = await brain.ask(question)
    except Exception as exc:  # Lỗi mạng, muse2api hết phiên...: báo bằng mặt, không làm sập server.
        log.error("brain failed: %s", exc)
        await hub.set_expression("confused", ERROR_HOLD_S)
        await hub.set_state("idle")
        return None
    speak = speak_seconds(reply.text)
    # Cảm xúc giữ suốt lúc nói + LINGER_S, board tự phai về neutral (không cần server nhớ).
    await hub.set_expression(reply.expression, speak + LINGER_S)
    await hub.set_state("speaking")
    log.info("[%s] %s", reply.expression, reply.text)
    await sleep(speak)
    await hub.set_state("idle")
    return reply
