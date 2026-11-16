# Hành vi linh vật: trạng thái và cảm xúc tự xuất hiện

Trạng thái: chạy trên board 2026-11-17 (nhánh `feature/server-link`): đã thấy chán → buồn ngủ → ngủ theo ngưỡng ban đêm; phản xạ chạm chờ người dùng thử. Thay cho việc chạm để đổi biểu cảm, giữ để đổi trạng thái (bỏ theo yêu cầu người dùng 2026-11-17).
Code: `firmware/src/behavior.*` (board), `server/byteof/turn.py` (server).

## Rút ra từ nghiên cứu
- **Robot đồng hành** (Anki Vector, EMO, Eilik): ít cảm xúc gốc, nhiều phản ứng nhỏ; mỗi phản ứng có kích hoạt → biểu lộ → **thời gian nghỉ (cool-down)** để không lặp; cảm xúc **ổn định, không đổi qua đổi lại liên tục**; để một mình thì tự chơi, chán, rồi ngủ; chạm, vuốt cho phản ứng **tức thì, xử lý tại chỗ**; mắt mềm = thư giãn, không nhìn chằm chằm quá lâu.
- **Trợ lý giọng nói:** phải luôn thấy rõ máy đang nghe, đang nghĩ hay đang nói; im lặng không có dấu hiệu làm người dùng hoang mang.
- **Che độ trễ** (Muse mất 8–20s): báo đã nhận câu hỏi (gật đầu) và **nhìn đi chỗ khác như đang nghĩ** làm máy có vẻ nhanh và sống hơn; ánh nhìn càng chủ động thì càng có vẻ phản hồi tốt.

## Ba lớp, ưu tiên từ trên xuống
1. **Hội thoại (server quyết):** trạng thái `listening` / `thinking` / `speaking` / `notifying` và cảm xúc của câu trả lời. Cảm xúc gửi kèm thời gian giữ (`hold`); hết giờ thì phai về bình thường.
2. **Phản xạ chạm (board tự làm, chỉ khi `idle`):** tức thì, không chờ server. Đang hội thoại thì chạm chỉ ra phản ứng nhỏ (nháy, boop, nhìn), không đè cảm xúc câu trả lời.
3. **Đời sống khi rảnh (board tự làm):** chớp, nhìn quanh, giật tai (đã có); chán → buồn ngủ → ngủ theo thời gian không ai tương tác; ban đêm buồn ngủ sớm hơn.

## Hội thoại
| Lúc | Trạng thái | Cảm xúc | Chi tiết |
|---|---|---|---|
| Nhận câu hỏi | `thinking` | bình thường | gật nhẹ một cái (đã nghe), liếc lên một bên, ba chấm (đã có) |
| Nghĩ lâu (> 6s) | `thinking` | bình thường | đổi bên liếc, thỉnh thoảng chớp đôi |
| Có câu trả lời | `speaking` | thẻ của Muse | giữ suốt lúc nói + 3s |
| Nói xong | `idle` | thẻ của Muse | hết `hold` thì phai về bình thường |
| Lỗi | `idle` | `confused` | giữ 3s |

## Phản xạ chạm (chỉ khi `idle`)
| Cử chỉ | Phản ứng | Giữ | Nghỉ |
|---|---|---|---|
| Chạm mắt / mũi / chỗ khác | nháy mắt / "boop" cười / nhìn theo (đã có) | – | – |
| 3–6 lần chạm trong 3s (vuốt ve) | `happy` | 3s | 6s |
| ≥ 7 lần chạm trong 3s (chọc liên tục) | `angry` (khó chịu) | 2.5s | 20s |
| Giữ tay 0.6–5s (xoa đầu) | `love`; xoa lại khi còn trong thời gian nghỉ thì `shy` | 3.5s | 15s |
| Chạm lúc đang ngủ | `surprised` 0.6s → `sleepy` 2s → bình thường (ngái ngủ) | – | – |
| Giữ > 5s | hiệu chỉnh cảm ứng (giữ nguyên) | – | – |

Board vẫn báo `touch` (`tap`, `long`) lên server; `long` để dành cho bấm-để-nói khi có mic.

## Khi rảnh (không chạm, không lệnh server)
| Không tương tác | Ban ngày | Ban đêm (22:00–6:00) |
|---|---|---|
| `bored` 4s (một lần mỗi đợt rảnh) | 60s | 30s |
| `sleepy` (gà gật, giữ tới khi có tương tác) | 120s | 45s |
| ngủ | 180s | 75s |

Giờ địa phương lấy từ NTP, múi giờ UTC+7. Board chưa có giờ (chưa nối mạng) thì coi là ban ngày.

## Giao thức
Tin `expression` thêm trường tùy chọn `hold` (giây). Có `hold`: hết giờ board tự phai về `neutral`. Không có: giữ tới lệnh sau (như cũ, cho lệnh tay `/e`).
