"""WebSocket server giữ kết nối với board (docs/design/protocol.md)."""

from __future__ import annotations

import asyncio
import hmac
import logging
from collections.abc import Callable

from websockets.asyncio.server import Server, ServerConnection, serve
from websockets.exceptions import ConnectionClosed

from . import protocol

log = logging.getLogger("byteof.hub")

HELLO_TIMEOUT_S = 5.0


class DeviceHub:
    def __init__(self, token: str, on_touch: Callable[[str], None] | None = None):
        if not token:
            raise ValueError("DEVICE_TOKEN is empty")
        self._token = token
        self._device: ServerConnection | None = None
        self._on_touch = on_touch
        self.connected = asyncio.Event()

    @property
    def online(self) -> bool:
        return self._device is not None

    async def send(self, frame: str) -> bool:
        """Gửi một tin cho board; không có board thì bỏ qua và trả False."""
        ws = self._device
        if ws is None:
            log.info("no device, skip %s", frame)
            return False
        try:
            await ws.send(frame)
            return True
        except ConnectionClosed:
            return False

    async def set_state(self, name: str) -> bool:
        return await self.send(protocol.state(name))

    async def set_expression(self, name: str, hold: float | None = None) -> bool:
        return await self.send(protocol.expression(name, hold))

    async def _hello(self, ws: ServerConnection) -> dict | None:
        try:
            raw = await asyncio.wait_for(ws.recv(), HELLO_TIMEOUT_S)
            msg = protocol.decode(raw)
        except (TimeoutError, protocol.ProtocolError) as exc:
            log.warning("bad hello from %s: %s", ws.remote_address, exc)
            return None
        if msg["t"] != "hello" or not hmac.compare_digest(msg["token"].encode(), self._token.encode()):
            log.warning("rejected device %s (wrong first message or token)", ws.remote_address)
            return None
        return msg

    async def handle(self, ws: ServerConnection) -> None:
        hello = await self._hello(ws)
        if hello is None:
            await ws.close(protocol.CLOSE_BAD_HELLO, "bad hello")
            return
        old, self._device = self._device, ws
        if old is not None:
            await old.close(protocol.CLOSE_REPLACED, "replaced")
        await ws.send(protocol.welcome())
        self.connected.set()
        log.info("device online %s fw=%s state=%s expression=%s", ws.remote_address,
                 hello.get("fw"), hello.get("state"), hello.get("expression"))
        try:
            async for raw in ws:
                try:
                    msg = protocol.decode(raw)
                except protocol.ProtocolError as exc:
                    log.warning("bad message: %s", exc)
                    continue
                if msg["t"] == "touch":
                    log.info("touch %s", msg["kind"])
                    if self._on_touch:
                        self._on_touch(msg["kind"])
        except ConnectionClosed:
            pass
        finally:
            if self._device is ws:
                self._device = None
                self.connected.clear()
                log.info("device offline")

    async def start(self, host: str, port: int) -> Server:
        # max_size nhỏ: board chỉ gửi tin JSON ngắn; server có thể ra Internet qua Cloudflare Tunnel (ADR 0008).
        return await serve(self.handle, host, port, ping_interval=20, ping_timeout=20, max_size=4096)
