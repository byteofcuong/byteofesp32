"""Thử hub bằng một board giả (client websockets) trên 127.0.0.1, cổng ngẫu nhiên."""

import asyncio
import json

from websockets.asyncio.client import connect
from websockets.exceptions import ConnectionClosed

from byteof import protocol
from byteof.brain import Reply
from byteof.hub import DeviceHub
from byteof.turn import run_turn

TOKEN = "test-token"


def run(coro):
    return asyncio.run(asyncio.wait_for(coro, 10))


async def _start(hub: DeviceHub):
    server = await hub.start("127.0.0.1", 0)
    port = server.sockets[0].getsockname()[1]
    return server, f"ws://127.0.0.1:{port}/"


async def _hello(ws, token=TOKEN):
    await ws.send(json.dumps({"t": "hello", "token": token, "fw": "test", "state": "idle", "expression": "neutral"}))


async def _close_code(ws) -> int:
    try:
        await ws.recv()
    except ConnectionClosed as exc:
        return exc.rcvd.code
    raise AssertionError("connection stayed open")


def test_welcome_then_commands():
    async def body():
        hub = DeviceHub(TOKEN)
        server, url = await _start(hub)
        async with connect(url) as ws:
            await _hello(ws)
            assert json.loads(await ws.recv()) == {"t": "welcome"}
            await hub.connected.wait()
            assert await hub.set_state("listening")
            assert json.loads(await ws.recv()) == {"t": "state", "state": "listening"}
        server.close()
        await server.wait_closed()
    run(body())


def test_wrong_token_closed():
    async def body():
        hub = DeviceHub(TOKEN)
        server, url = await _start(hub)
        async with connect(url) as ws:
            await _hello(ws, token="wrong")
            assert await _close_code(ws) == protocol.CLOSE_BAD_HELLO
        assert not hub.online
        server.close()
        await server.wait_closed()
    run(body())


def test_first_message_must_be_hello():
    async def body():
        hub = DeviceHub(TOKEN)
        server, url = await _start(hub)
        async with connect(url) as ws:
            await ws.send(json.dumps({"t": "touch", "kind": "tap"}))
            assert await _close_code(ws) == protocol.CLOSE_BAD_HELLO
        server.close()
        await server.wait_closed()
    run(body())


def test_new_device_replaces_old_and_touch_reported():
    async def body():
        touches = []
        hub = DeviceHub(TOKEN, on_touch=touches.append)
        server, url = await _start(hub)
        async with connect(url) as old, connect(url) as new:
            await _hello(old)
            await old.recv()
            await _hello(new)
            await new.recv()
            assert await _close_code(old) == protocol.CLOSE_REPLACED
            await new.send(json.dumps({"t": "touch", "kind": "long"}))
            for _ in range(50):
                if touches:
                    break
                await asyncio.sleep(0.02)
            assert touches == ["long"]
        server.close()
        await server.wait_closed()
    run(body())


class FakeBrain:
    def __init__(self, reply=None, error=None):
        self.reply, self.error = reply, error

    async def ask(self, question):
        if self.error:
            raise self.error
        return self.reply


async def _turn_frames(brain) -> list[dict]:
    hub = DeviceHub(TOKEN)
    server, url = await _start(hub)
    async with connect(url) as ws:
        await _hello(ws)
        await ws.recv()
        await hub.connected.wait()

        async def no_sleep(_s):
            return None

        await run_turn(hub, brain, "chào", sleep=no_sleep)
        frames = []
        while frames[-1:] != [{"t": "state", "state": "idle"}]:
            frames.append(json.loads(await ws.recv()))
    server.close()
    await server.wait_closed()
    return frames


def test_turn_sequence():
    frames = run(_turn_frames(FakeBrain(reply=Reply("Chào bạn!", "happy"))))
    assert frames == [
        {"t": "state", "state": "thinking"},
        {"t": "expression", "expression": "neutral"},
        # Giữ suốt lúc nói (tối thiểu 1.5s) + 3s rồi board tự phai.
        {"t": "expression", "expression": "happy", "hold": 4.5},
        {"t": "state", "state": "speaking"},
        {"t": "state", "state": "idle"},
    ]


def test_turn_brain_error_shows_confused():
    frames = run(_turn_frames(FakeBrain(error=RuntimeError("muse2api down"))))
    assert frames == [
        {"t": "state", "state": "thinking"},
        {"t": "expression", "expression": "neutral"},
        {"t": "expression", "expression": "confused", "hold": 3.0},
        {"t": "state", "state": "idle"},
    ]


def test_send_without_device_is_skipped():
    async def body():
        hub = DeviceHub(TOKEN)
        assert await hub.set_state("idle") is False
    run(body())
