# Giao thức Device ↔ HomeServer (v1)

Trạng thái: chạy được trên board từ 2026-11-17 (nhánh `feature/server-link`). Quyết định nền: ADR 0001, 0007, 0008.
Code: `server/byteof/protocol.py` (server), `firmware/src/server_link.cpp` (board). Hai bên phải khớp file này.

## Kết nối
- Bình thường: `wss://<tên miền>/` qua Cloudflare Tunnel (ADR 0008). `cloudflared` trên PC chuyển vào server ở `ws://127.0.0.1:8765/`.
- Thử trong mạng nhà: `ws://<IP PC>:8765/` (đặt `BYTEOF_HOST=0.0.0.0` và mở Windows Firewall).
- Địa chỉ server lưu trong NVS của board, sửa ở trang cài WiFi; mặc định lấy `BYTEOF_SERVER_URL` trong `secrets.h`.
- Board tự nối lại sau 3s khi mất kết nối. Thư viện gửi ping mỗi 15s; 2 lần không có pong thì ngắt. Cloudflare cắt WebSocket im lặng quá 100s, ping 15s giữ nó sống.
- Server nhận tin tối đa 4096 byte.
- Mỗi khung là một chuỗi JSON (text frame), có trường `t` là loại tin.
- Chỉ một board tại một thời điểm. Board mới chào thành công thì board cũ bị ngắt.

## Chào hỏi
Tin đầu tiên của board phải là `hello`. Server đóng kết nối (mã 4001) nếu tin đầu không phải `hello`, sai `token`, hoặc quá 5s chưa chào.

```json
{"t": "hello", "token": "<DEVICE_TOKEN>", "fw": "0.2.0", "state": "idle", "expression": "neutral"}
```
Server trả lời `{"t": "welcome"}` rồi mới gửi lệnh.

## Server → board
| `t` | Trường | Ý nghĩa |
|---|---|---|
| `welcome` | | Chào hỏi thành công |
| `state` | `state` | Đổi trạng thái: `idle`, `listening`, `thinking`, `speaking`, `notifying` |
| `expression` | `expression`, `hold` (tùy chọn, giây, 0–600) | Đổi biểu cảm: `neutral`, `happy`, `laugh`, `excited`, `love`, `shy`, `smug`, `sad`, `cry`, `angry`, `surprised`, `scared`, `confused`, `bored`, `sleepy`. Có `hold`: board giữ rồi tự phai về `neutral` (`docs/design/behavior.md`); không có: giữ tới lệnh sau |

Giá trị lạ thì board bỏ qua và ghi log `[link] bad …`.

## Board → server
| `t` | Trường | Ý nghĩa |
|---|---|---|
| `hello` | `token`, `fw`, `state`, `expression` | Xem trên |
| `touch` | `kind`: `tap`, `long` (`double` còn nhận cho bản cũ) | Người dùng chạm. Board tự phản xạ tại chỗ (`docs/design/behavior.md`); server chỉ được báo. `long` để dành cho bấm-để-nói |

## Một lượt hỏi đáp hiện tại (chưa có mic, loa)
Người dùng gõ câu hỏi ở console server. Server làm như sau:
1. Gửi `state` = `thinking`, `expression` = `neutral` (board tự gật đầu, liếc đi như đang nghĩ).
2. Gọi Brain (Muse qua muse2api, ADR 0006), chờ trả lời.
3. Gửi `expression` theo thẻ cảm xúc Brain chọn (thẻ cuối cùng trong câu trả lời) kèm `hold` = thời gian nói + 3s, rồi `state` = `speaking`.
4. In câu trả lời ra console. Giữ `speaking` khoảng 65ms mỗi ký tự (tối thiểu 1.5s, tối đa 12s), rồi gửi `state` = `idle`.
5. Brain lỗi thì gửi `expression` = `confused` (`hold` 3s), `state` = `idle`.
