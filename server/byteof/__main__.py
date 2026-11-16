"""Chạy HomeServer: `python -m byteof` trong thư mục server/ (README.md).

Console: gõ câu hỏi để hỏi Brain; `/s <trạng thái>`, `/e <biểu cảm>` để điều khiển tay; `/q` để thoát.
"""

from __future__ import annotations

import asyncio
import logging
import sys

from . import config, protocol
from .brain import OpenAICompatBrain
from .hub import DeviceHub
from .turn import run_turn

HELP = (
    "Gõ câu hỏi rồi Enter. Lệnh: /s <state>  /e <expression>  /q\n"
    f"  state: {' '.join(protocol.STATES)}\n"
    f"  expression: {' '.join(protocol.EXPRESSIONS)}"
)


async def console(hub: DeviceHub, brain: OpenAICompatBrain) -> None:
    print(HELP, flush=True)
    while True:
        line = (await asyncio.to_thread(sys.stdin.readline))
        if not line:  # Hết stdin.
            return
        line = line.strip()
        if not line:
            continue
        if line == "/q":
            return
        try:
            if line.startswith("/s "):
                await hub.set_state(line[3:].strip())
            elif line.startswith("/e "):
                await hub.set_expression(line[3:].strip())
            elif line.startswith("/"):
                print(HELP, flush=True)
            else:
                reply = await run_turn(hub, brain, line)
                if reply:
                    print(f"[{reply.expression}] {reply.text}", flush=True)
        except protocol.ProtocolError as exc:
            print(f"lỗi: {exc}", flush=True)


async def main() -> None:
    cfg = config.load()
    hub = DeviceHub(cfg.device_token)
    brain = OpenAICompatBrain(cfg.brain_base_url, cfg.brain_api_key, cfg.brain_model)
    server = await hub.start(cfg.host, cfg.port)
    logging.getLogger("byteof").info("listening on %s:%d, brain=%s at %s", cfg.host, cfg.port,
                                     cfg.brain_model, cfg.brain_base_url)
    try:
        await console(hub, brain)
    finally:
        server.close()
        await server.wait_closed()


if __name__ == "__main__":
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
        sys.stdin.reconfigure(encoding="utf-8")
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(name)s %(message)s", datefmt="%H:%M:%S")
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
