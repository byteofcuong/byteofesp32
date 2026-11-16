import json
import re
from pathlib import Path

import pytest

from byteof import protocol
from byteof.brain import SYSTEM_PROMPT, parse_reply
from byteof.turn import speak_seconds

FIRMWARE = Path(__file__).resolve().parents[2] / "firmware" / "src" / "mascot.cpp"


def _firmware_names(var_hint: str) -> list[str]:
    src = FIRMWARE.read_text(encoding="utf-8")
    fn = src[src.index(f"const char* toString({var_hint}"):]
    body = fn[: fn.index("}")]
    return re.findall(r'"([a-z]+)"', body)


def test_names_match_firmware():
    assert tuple(_firmware_names("AssistantState")) == protocol.STATES
    assert tuple(_firmware_names("Expression")) == protocol.EXPRESSIONS


def test_encode_state_and_expression():
    assert json.loads(protocol.state("thinking")) == {"t": "state", "state": "thinking"}
    assert json.loads(protocol.expression("happy")) == {"t": "expression", "expression": "happy"}
    with pytest.raises(protocol.ProtocolError):
        protocol.state("dancing")
    with pytest.raises(protocol.ProtocolError):
        protocol.expression("HAPPY")


def test_expression_hold():
    assert json.loads(protocol.expression("sad", 4.56)) == {"t": "expression", "expression": "sad", "hold": 4.6}
    for bad in (0, -1, 601):
        with pytest.raises(protocol.ProtocolError):
            protocol.expression("sad", bad)


@pytest.mark.parametrize("raw", ["not json", "[]", '{"t": 1}', '{"t": "hello"}', '{"t": "touch", "kind": "poke"}',
                                 '{"t": "nope"}', b"\x00"])
def test_decode_rejects(raw):
    with pytest.raises(protocol.ProtocolError):
        protocol.decode(raw)


def test_decode_accepts():
    assert protocol.decode('{"t":"hello","token":"x","fw":"0.2.0"}')["token"] == "x"
    assert protocol.decode('{"t":"touch","kind":"double"}')["kind"] == "double"


@pytest.mark.parametrize("raw,text,expr", [
    ("[happy] Chào bạn!", "Chào bạn!", "happy"),
    ("  [Sad]Tiếc quá.", "Tiếc quá.", "sad"),
    ("[dancing] Hả?", "Hả?", "neutral"),
    ("Không có thẻ.", "Không có thẻ.", "neutral"),
    ("", "", "neutral"),
    # Mẫu thật từ muse2api (2026-11-17): đoạn chép lặp + thẻ hành động ở cuối.
    ("[happy] Được rồi, mình hẹn giờ 5 phút luộc trứng cho bạn nhé.\n"
     "timer[happy] Được rồi, mình hẹn giờ 5 phút luộc trứng cho bạn nhé.\n[timer 300]",
     "Được rồi, mình hẹn giờ 5 phút luộc trứng cho bạn nhé.", "happy"),
    ("[excited] Giỏi quá!\n[remind 18:30 gọi điện cho mẹ]", "Giỏi quá!", "excited"),
])
def test_parse_reply(raw, text, expr):
    r = parse_reply(raw)
    assert (r.text, r.expression) == (text, expr)


def test_system_prompt_lists_every_expression():
    for e in protocol.EXPRESSIONS:
        assert f"[{e}]" in SYSTEM_PROMPT


def test_speak_seconds_clamped():
    assert speak_seconds("") == 1.5
    assert speak_seconds("a" * 100) == pytest.approx(6.5)
    assert speak_seconds("a" * 10_000) == 12.0
