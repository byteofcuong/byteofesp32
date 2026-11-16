# Đặc tả linh vật: mắt robot gợi hình con cáo

Cập nhật 2026-11-16 (đại tu biểu cảm). Số liệu chuẩn nằm trong `prototype.html` (hằng số `BASE`, `EX`, `EAR_PTS`, `EAR_PIVOT`, `EYE_DX`, `EYE_Y`, `MOUTH_Y`, `COL`); firmware (`initShapes()` trong `mascot.cpp`) phải dùng đúng các số này.

## Hướng đi và lịch sử
- Người dùng chỉ thích bản đầu tiên (hai mắt robot + một miệng trên nền đen, có trạng thái và biểu cảm), muốn giữ sự tối giản đó nhưng **gợi ra con cáo**.
- Đã thử và bị chê, không quay lại: cáo vẽ cả đầu bằng đa giác (8-bit, thô), cáo neon/visor/hologram, cáo kawaii đường cong + phụ kiện công nghệ, cáo lò xo nhiều hiệu ứng, cáo tối giản phẳng kiểu logo. Lý do chung: quá nhiều chi tiết, trông ẩu hoặc tẻ nhạt.
- Đại tu biểu cảm (2026-11-16), những thứ đã thử và bị chê:
  - Ký hiệu kiểu emoji (tim bay, sao, 💢, "?", mắt xoáy, mắt `^ ^`/`> <` thay hình), má và lưỡi hồng: "gượng", "AI slop", hồng lạc quẻ.
  - Miệng người (chữ D há to, lòng miệng tối, khe U, uốn khóe): "dị dạng", "biến thái", mất nét động vật.
  - Lông mày: trùng chân tai, rối hình. Mắt bo tròn chỉ nghiêng (bỏ mí cắt): người dùng thích mí cắt hơn.
  - Mắt cười khoét đáy: "cắt ẩu". Khóe ω uốn lên: như ria mép. Miệng buồn kéo khóe xuống: trễ quá.

## Nhận diện riêng (chống đụng hàng)
Tra cứu 2026-11-16: mắt chữ nhật bo góc phát sáng, không con ngươi, vui thì nheo vòm là ngôn ngữ hình ảnh của Anki Cozmo/Vector, EMO và thư viện RoboEyes. Màn hình mặt + tai đã có ở Loona. Cáo robot đã có VISIONOID FOX. Dấu hiệu riêng của dự án:
- **Con ngươi khe dọc kiểu mắt cáo** (elip mảnh, cao ~60% mắt). Robot màn hình phổ biến đều dùng mắt trơn.
- **Màu cam** làm màu chủ đạo (Cozmo/Vector/EMO dùng xanh).
- **Tai đặc, lòng tai sẫm**, tham gia biểu cảm.
- **Miệng thú ω** có biến tấu bằng phụ kiện (răng nanh, lưỡi).

Không phải ý kiến pháp lý.

## Nguyên tắc
1. Chỉ có **hai mắt, một miệng**. Dấu hiệu cáo: **đôi tai nhọn** phía trên mắt, **mũi tam giác + miệng chữ ω**, **màu cam** khi rảnh và khi nói.
2. **Mọi biểu cảm là một bộ số liên tục**, nội suy bằng lò xo, không thay hình đột ngột. Riêng `sm` (vòm cười) và `hrt` (mắt tim) đổi hình khối mắt nên chuyển gần như tức thì (~3 khung).
3. **Đơn sắc** theo trạng thái trên nền đen. Đốm sáng, răng nanh, lưỡi là màu chính pha trắng; lòng tai, nước mắt là màu chính pha nền. Không hồng, không xanh.
4. **Miệng giữ khung ω**, chỉ biến tấu: nếp sâu/nông/lật nhẹ, gợn, lệch, răng nanh, lưỡi lè, chữ o đặc (ngạc nhiên). Miệng không bao giờ rộng hơn ω. Khi nói là chữ D đặc như bản đầu.
5. **Không ký hiệu emoji.** Ngoại lệ: chữ z khi ngủ, nước mắt cùng tông, và chỉ báo trạng thái (sóng âm, ba chấm, chấm báo "!", viền sáng).
6. Chỉ dùng hình firmware vẽ được: chữ nhật bo góc, elip, tam giác, đường cong nét tròn, vùng dưới/trên một đường cong.
7. Mỗi lần sửa: mở `prototype.html`, chụp và soi các biểu cảm + trạng thái trước khi đưa người dùng; với chỗ nghi ngờ thì phóng to.

## Bố cục (màn 480x320)
- Cả mặt dời xuống 30px so với bản đầu (bản đầu nằm y 4..250, bị chê quá cao).
- **Mắt:** tâm (162, 170) và (318, 170); chữ nhật bo góc 86x104, bo 30; đốm sáng to ở trên phải, nhỏ ở dưới trái.
- **Tai trái:** khối tam giác đặc bo 12: (94,112) (192,92) (124,34); trục (143,102). Tai phải đối xứng qua x = 240. Lòng tai: cùng hình co 55% về trọng tâm (dịch xuống 10px), pha màu nền 45%.
- **Con ngươi:** elip màu nền, rx = rộng mắt × `pw` / 2, ry = cao mắt × `ph`, dịch theo hướng nhìn.
- **Mí:** vùng màu nền trên/dưới một đường cong bậc hai.
  - Mí trên: `uL/uR` che, `uA` nghiêng (+ cau, − lo), `uB` võng.
  - Mí dưới: `dL/dR`, `dB` cong.
  - Mí là MỘT vùng liền; ghép hai tam giác thì lộ vệt nối.
  - Đốm sáng tắt khi chạm mép mí, và tắt hẳn khi mí che quá nửa.
- **Mắt cười:** khối mắt bẹt dần rồi tan vào một vòm nét dày 26px, hai đầu tròn.
- **Mắt yêu:** khối mắt thành trái tim cùng màu.
  - Dựng từ 2 hình tròn R24 tại (±21, −12), nối mũi tim (0, 44) với **điểm tiếp tuyến phía ngoài** của mỗi hình tròn; mũi bo bằng đường cong bậc hai. Tam giác tùy ý làm sườn gãy lõm; nét viền làm phình gờ.
  - Tròng là tim nhỏ tối (tỉ lệ 0.32). Tim đập hai nhịp mỗi 1.2 giây.
- **Mũi:** tam giác bo 4, đáy rộng 22, tại y 238. **Miệng ω** tại y ≈ 258–266.

## Màu
| Vai trò | Màu |
|---|---|
| Nền | `#000000` |
| Rảnh, đang nói (cáo) | `#FF9A3C` |
| Đang nghe | `#6EF2A6` |
| Đang nghĩ | `#B79BFF` |
| Thông báo | `#FFD25E` |
| Ngủ | `#5B6B7A` |
| Ám khi giận | `#FF6A3A` (pha 70%) |
| Đốm sáng | `#FFFFFF` |

## Biểu cảm (15)
| Biểu cảm | Mắt | Miệng | Tai, chuyển động |
|---|---|---|---|
| neutral | bo góc 86x104, khe dọc | ω | đứng yên, tự liếc |
| happy | vòm cười | ω sâu hơn | dựng; nảy nhẹ |
| laugh | vòm cười | ω + 2 răng nanh | thả; rung "ha ha ha" từng tràng, nghiêng đầu |
| excited | to 92x112, khe dọc, đốm sáng to | ω sâu + lè lưỡi | dựng cao; nhảy tưng |
| love | trái tim, tròng tim nhỏ | ω sâu | hơi thả; nghiêng đầu, đung đưa chậm |
| shy | mí trên che 26% | ω hẹp | thả; nhìn xuống góc, đung đưa |
| smug | mí trên che 40%, hơi cau | ω + lè lưỡi | hơi dựng; liếc ngang |
| sad | mí lo (đầu trong cao) | ω lật nhẹ | cụp; nhìn xuống |
| cry | như buồn, mí dưới võng | ω lật nhẹ, gợn | cụp; run, nước mắt |
| angry | mí cau (đầu trong thấp), con ngươi hẹp, ám đỏ | ω + 1 răng nanh | thả ra sau; run nhẹ |
| surprised | tròn 98x112, con ngươi chấm | chữ o đặc, có nét nối mũi | dựng; bật lên |
| scared | to, con ngươi rất nhỏ, mí hơi lo | ω gợn | cụp; run |
| confused | mắt trái nhỏ, híp hình thấu kính | ω lệch | nghiêng đầu; nhìn lên phải |
| bored | mí che 50% | ω nông, hẹp | nhìn xuống |
| sleepy | mí che 66%, nhìn xuống, tắt đốm sáng | ω nhỏ, nông | cụp; gà gật (mí sụp thêm rồi mở, đầu gật, chu kỳ ~5.7s) |

Tai cụp bằng **ép thấp + nghiêng nhẹ** (≤ 10°); xoay mạnh làm tai trông như nút "play".

## Trạng thái trợ lý
| Trạng thái | Hiển thị |
|---|---|
| idle | cam; tự liếc, chớp (20% chớp đôi), giật tai 4–10 giây/lần, thỉnh thoảng nghiêng đầu; để yên 30 giây thì ngủ (mặt buồn ngủ, chữ z) |
| listening | xanh lá; cột sóng âm hai bên; tai hơi dựng |
| thinking | tím; nhìn lên góc phải; ba chấm ở x 414..454, y 84 (xa tai) |
| speaking | cam; miệng D đặc, há theo âm tiết (0.16–0.30 s/âm tiết, lò xo k220 c26) |
| notifying | vàng; mặt hào hứng không lưỡi, nhìn thẳng; nhún 2 nhịp + vẫy tai mỗi 1.6 s; **viền màn hình sáng nhấp nháy** (chu kỳ 0.8 s); **chấm báo "!"** ở (440, 72) với 2 vòng lan |

## Chạm (firmware `main.cpp` + `behavior.cpp`, bản mẫu: chạm vào khung lớn)
Từ 2026-11-17 không còn chạm để đổi biểu cảm hay trạng thái: cảm xúc tự xuất hiện theo `docs/design/behavior.md`.
| Cử chỉ | Kết quả |
|---|---|
| Chạm 1 lần vào mắt | nháy mắt đó |
| Chạm 1 lần vào mũi/miệng | "boop": híp mắt (đá vận tốc `sm`), nảy, giật tai |
| Chạm 1 lần chỗ khác | nhìn về điểm chạm 1.8 s, liếc xa thì chớp |
| Vài lần chạm liền / chọc liên tục / giữ tay | vui / khó chịu / thương, ngượng (bảng trong `behavior.md`, chỉ khi rảnh) |
| Giữ > 5 s | hiệu chỉnh cảm ứng |

## Trên firmware (`firmware/src/mascot.*`, đo 2026-11-16)
- Vẽ vào sprite 16-bit 448x40 **theo từng dải ngang** (trước 56 dòng; 40 dòng fps như nhau mà bớt 14KB RAM), chỉ đẩy vùng mặt (x 80..400, y 24..316; rộng hơn khi có sóng âm, ba chấm, chữ z).
- Làm mịn cạnh bằng `fillSmoothRoundRect`, `fillSmoothCircle`, `drawWideLine` của TFT_eSPI. Mép nét phải **pha với pixel bên dưới** (`bg = 0x00FFFFFF`); pha với màu cố định làm chỗ chồng nhau thành vệt tối như vết cắt.
- **Ảnh đệm** (vẽ lại chỉ khi thông số đổi, khóa băm `quantKey`):
  - tai 150x124, **một ảnh**: tai phải dán lật gương (hai tai đối xứng qua trục giữa);
  - cụm mũi + miệng 96x60 (16-bit), dán đặc và vẽ trước mắt;
  - vòm mắt cười 112x68 x2, dán đặc, khung ôm sát để nền đen không đè tai;
  - tai và vòm cười lưu **độ phủ 8-bit** thay vì màu 16-bit: vẽ bằng màu "đỏ" (hình ngoài) và "vàng" (lòng tai) vào sprite RGB332, lúc dán tra bảng 256 màu pha sẵn (`blitMask`). Nửa bộ nhớ, dán nhanh hơn `pushToSprite`;
  - Dán trong suốt từng pixel quá chậm.
- **RAM** (2026-11-17, ADR 0008): linh vật ~87KB (trước ~150KB). WiFi + bắt tay TLS cần phần còn lại; thêm ảnh đệm mới phải đo lại `heap=` trong log `[link]`. Khi cài WiFi, `Mascot::end()` trả hết ảnh đệm.
- **Hình rẻ thay nét dày:**
  - tam giác bo góc = tam giác co + 3 dải cạnh tô đặc + 3 hình tròn góc + nét mảnh 1.5 làm mịn. Nét dày 2r tốn 36ms cho 2 tai, gây khựng khi chuyển biểu cảm;
  - vòm cười = chuỗi hình tròn (`thickQuad`);
  - mí = tô theo cột + nét mảnh dọc mép.
- **Thông báo:**
  - viền vẽ thẳng lên TFT sau các dải;
  - chấm báo vẽ trong sprite riêng 76x84 ở (400, 30), chỉ tạo khi đang thông báo (nhường RAM cho TLS lúc thường);
  - vùng mặt và chấm báo không được đè lên viền, đè thì viền nháy đen;
  - nới vùng mặt để chứa chấm báo làm tụt 21 fps.
- **Run** ~3.7 Hz (23/19 rad/s). Tần số cao hơn khi lấy mẫu 25 fps trông như giật.
- **SPI** 80MHz; đẩy một khung ~26ms.
- **Kết quả:** 23.6 (khóc) đến 30.3 khung/giây; khung chậm nhất khi chuyển biểu cảm 41–52ms; heap còn ~138KB (2026-11-16, chưa có WiFi).
- **Có WiFi + TLS online** (2026-11-17, đủ miệng, lệnh đo đi qua lớp hành vi): 18.7 (khóc) đến 30.2 khung/giây; trạng thái 23.5–28.9; khung chậm nhất khi chuyển ≤ 63ms; heap ~75KB.
  - Cùng firmware, tắt WiFi: chỉ chậm hơn bản gốc ~1 fps (khóc 22.7). WiFi làm các mặt nặng (thương, buồn, khóc, giận, sợ, chán, buồn ngủ) chậm thêm 3–4 fps. Giãn nhịp hỏi mạng 5 → 20ms không đổi gì. Hướng tối ưu về sau: miệng gợn sóng (`mv`) đang vẽ lại ảnh đệm mỗi khung; đưa hàm vẽ nóng vào IRAM.
  - Dải 40 và 56 dòng cho fps như nhau (đo lại khi đủ miệng).
- **Lệnh Serial:** `e` biểu cảm tiếp, `s` trạng thái tiếp (đo bằng script). Log `[mascot] fps=… frame=… (push …) max=…`.

## Chuyển động (lò xo k, c)
- Thông số biểu cảm: k 300, c 30 (k 170 bị chê chậm). `sm`, `hrt`: bám mũ `x += (đích − x)·min(1, dt·20)`.
- Liếc mắt: k 320, c 30 (mắt dịch ±22px ngang, ±14px dọc; mắt bên đang liếc to hơn 7%).
- Nghiêng đầu: k 60, c 11. Nảy: k 160, c 9. Giật tai: hích vận tốc 160, k 260, c 9.
- Chớp: nhắm 70ms, mở 130ms (ease-out), mắt bè ngang 12% khi nhắm. Liếc xa thì chớp kèm.
- Thở: nhấp nhô 2px chu kỳ 4 giây (ngủ: 4px, 6 giây).
