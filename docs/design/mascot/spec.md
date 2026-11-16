# Đặc tả linh vật: mắt robot gợi hình con cáo

Cập nhật 2026-11-16. Số liệu chuẩn nằm trong `prototype.html` (hằng số `SHAPES`, `EAR_POSE`, `EAR_PTS`, `EYE_DX`, `EYE_Y`, `MOUTH_Y`, `COL`); firmware phải dùng đúng các số này.

## Hướng đi và lịch sử
- Người dùng chỉ thích bản đầu tiên (hai mắt robot + một miệng trên nền đen, có trạng thái và biểu cảm), muốn giữ sự tối giản đó nhưng **gợi ra con cáo**.
- Đã thử và bị chê, không quay lại: cáo vẽ cả đầu bằng đa giác (8-bit, thô), cáo neon/visor/hologram, cáo kawaii đường cong + phụ kiện công nghệ, cáo lò xo nhiều hiệu ứng, cáo tối giản phẳng kiểu logo. Lý do chung: quá nhiều chi tiết, trông ẩu hoặc tẻ nhạt.

## Nhận diện riêng (chống đụng hàng)
Tra cứu 2026-11-16: mắt chữ nhật bo góc phát sáng, không con ngươi, vui thì nheo vòm là ngôn ngữ hình ảnh của Anki Cozmo/Vector (thiết kế bỏ con ngươi để mắt biến hình), EMO và thư viện RoboEyes; màn hình mặt + tai đã có ở Loona; cáo robot đã có VISIONOID FOX. Dấu hiệu riêng của dự án:
- **Con ngươi khe dọc kiểu mắt cáo** (elip mảnh, cao ~60% mắt, độ rộng đổi theo biểu cảm: khe khi bình thường, nở khi ngạc nhiên, hẹp khi giận). Robot màn hình phổ biến đều dùng mắt trơn.
- **Màu cam** làm màu chủ đạo (Cozmo/Vector/EMO dùng xanh).
- **Tai đặc, lòng tai sẫm**, tham gia biểu cảm.
Không phải ý kiến pháp lý.

## Nguyên tắc
1. Chỉ có **hai mắt, một miệng**. Dấu hiệu cáo: **đôi tai nhọn** (nét viền) phía trên mắt, **mũi tam giác + miệng chữ ω**, **màu cam** khi rảnh và khi nói.
2. Biểu cảm dồn vào hình mắt (kích thước, mí, khoét cười), tư thế tai và hình miệng. Mọi thông số nội suy bằng lò xo, không nhảy cóc.
3. Chỉ dùng hình firmware vẽ được: chữ nhật bo góc, elip, tam giác, đường cong nét tròn.
4. Mỗi lần sửa: mở `prototype.html`, chụp và soi đủ 6 biểu cảm + 5 trạng thái trước khi đưa người dùng.

## Bố cục (màn 480x320)
- Mắt: tâm (162, 140) và (318, 140); chữ nhật bo góc 86x104, bo 30; đốm sáng to ở góc trên, nhỏ ở góc dưới.
- Tai trái: khối tam giác đặc bo 12: (94,82) (192,62) (124,4); trục (143,72); tai phải đối xứng qua x = 240. Lòng tai: cùng hình co 55% về trọng tâm (dịch xuống 10px), phủ màu nền 45% cho sẫm. Chóp tai lệch ra ngoài ~20px so với giữa chân tai (lệch nhiều trông như lá cờ). Đã loại: tam giác viền ("không giống tai"), lòng tai khoét đen (thành hình sừng).
- Con ngươi: elip màu nền, rx = bề rộng mắt × (0.14 bình thường, 0.2 vui, 0.24 buồn, 0.09 giận, 0.36 ngạc nhiên) / 2, ry = 30% chiều cao mắt, tâm thấp hơn tâm mắt 7%; dịch theo hướng nhìn. Ẩn khi mắt cười. Đã loại: khe chữ nhật cao 62% (trông như chữ số "0").
- Mũi: tam giác bo 4, đáy rộng 20, tâm (240, 211). Miệng ω tại y ≈ 228–236.

## Màu
| Vai trò | Màu |
|---|---|
| Nền | `#000000` |
| Rảnh, đang nói (cáo) | `#FF9A3C` |
| Đang nghe | `#6EF2A6` |
| Đang nghĩ | `#B79BFF` |
| Thông báo | `#FFD25E` |
| Ngủ | `#5B6B7A` |
| Đốm sáng | `#FFFFFF` |
| Má hồng (khi vui) | `#FF7EB3` |

## Biểu cảm
| Biểu cảm | Mắt | Tai (góc, độ ép) | Miệng |
|---|---|---|---|
| neutral | bo góc 86x104 | 0°, 1 | ω nhỏ |
| happy | vòm cười (khoét dưới 0.36) | -4°, 1.05 | ω cong sâu, má hồng |
| sad | mí trễ ra ngoài | 8°, 0.7 | ω lật ngược |
| angry | mí trễ vào trong | 6°, 0.82 | ω phẳng |
| surprised | tròn 98x112 | -6°, 1.08 | chữ O (không nét nhân trung) |
| sleepy | mí che 58%, màu xám | 6°, 0.78 | ω nhỏ |

Tai cụp bằng cách **ép thấp + nghiêng nhẹ**, không xoay mạnh (xoay 16–28° làm tai trông như nút "play").

## Trạng thái trợ lý
| Trạng thái | Hiển thị |
|---|---|
| idle | cam; tự liếc mắt, chớp mắt (20% chớp đôi), giật tai 4–10 giây/lần, thỉnh thoảng nghiêng đầu; để yên 30 giây thì ngủ (chữ z) |
| listening | xanh lá; cột sóng âm hai bên; tai hơi dựng |
| thinking | tím; nhìn lên góc phải; ba chấm |
| speaking | cam; miệng mở theo nhịp nói |
| notifying | vàng; mắt cười, nảy nhẹ, lấp lánh 4 góc |

## Trên firmware (`firmware/src/mascot.*`, đo 2026-11-16)
- Vẽ vào sprite 16-bit 448x56, **theo từng dải ngang**, chỉ đẩy vùng có hình (x 80..400, rộng hơn khi có sóng âm, chấm, lấp lánh, chữ z). RAM còn ~199KB.
- Làm mịn cạnh bằng `fillSmoothRoundRect`, `fillSmoothCircle`, `drawWideLine` của TFT_eSPI. **Nét dày phải pha mép với pixel bên dưới** (`bg = 0x00FFFFFF`), không pha với màu cố định: pha với đen làm chỗ các đoạn chồng nhau thành vệt tối như vết cắt (người dùng thấy ở tai và miệng).
- **Tai vẽ sẵn vào hai ảnh riêng** (150x124), chỉ vẽ lại khi góc/độ ép/màu đổi; dán thẳng vì tai vẽ trước mắt. Vẽ tai mỗi khung tốn 36ms.
- SPI 80MHz (40MHz đẩy khung mất 43ms). Kết quả: **30 khung/giây** (khung 32ms, đẩy 25ms). Người dùng xác nhận màn sạch, mượt, không vết cắt.

## Chuyển động (lò xo k, c)
- Thông số biểu cảm: k 170, c 22. Liếc mắt: k 320, c 30 (mắt dịch ±22px ngang, ±14px dọc; mắt bên đang liếc to hơn 7%).
- Nghiêng đầu: k 60, c 11. Nảy: k 160, c 9. Giật tai: hích vận tốc 160, k 260, c 9.
- Chớp: nhắm 70ms, mở 130ms (ease-out), mắt bè ngang 14% khi nhắm. Liếc xa thì chớp kèm.
- Thở: nhấp nhô 2px chu kỳ 4 giây (ngủ: 4px, 6 giây).
