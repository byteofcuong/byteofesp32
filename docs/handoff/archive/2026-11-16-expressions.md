---
date: 2026-11-16
branch: main
commit: 8c5b4f2
---
# Handoff

## Mục tiêu
Trợ lý giọng nói để bàn trên board ESP32-3248S035R. Bước 1 của lộ trình (linh vật trên màn) **đã xong**: người dùng chốt thiết kế và xác nhận trên màn thật là liền mạch, sạch, mượt.

## Trạng thái
- `main` đã có và đã push: firmware màn + cảm ứng + linh vật (`firmware/src/main.cpp`, `mascot.*`, `touch.*`), đặc tả và bản mẫu linh vật (`docs/design/mascot/`), ontology.
- Linh vật: hai mắt robot có **con ngươi khe dọc**, **tai cáo đặc lòng sẫm**, mũi tam giác, miệng ω, màu cam khi rảnh; 5 trạng thái trợ lý, 6 biểu cảm; chuyển động bằng lò xo; 30 fps.
- Thử trên board: chạm mắt = nháy mắt; chạm chỗ khác = đổi trạng thái; giữ 1 giây = đổi biểu cảm; giữ 5 giây = hiệu chỉnh cảm ứng; để yên 30 giây = ngủ.
- `AGENTS.md` đã cập nhật trạng thái dự án (trước đó vẫn ghi "chưa có code").
- Nhánh `firmware/display-touch`, `feature/fox-mascot` còn ở máy, đã merge, xóa được. Sao lưu flash gốc ở `backups/factory_backup.bin` (git bỏ qua).

## Quyết định
- ADR 0005: PlatformIO + Arduino + TFT_eSPI; cảm ứng tự đọc kiểu LovyanGFX (PENIRQ GPIO36, trung vị 7 lượt); SPI 80MHz.
- Thiết kế linh vật: xem `docs/design/mascot/spec.md` (có mục chống đụng hàng với Cozmo/Vector/EMO và danh sách phương án đã loại). Mọi thay đổi hình: sửa `prototype.html`, tự chụp và soi trước khi đưa người dùng.
- Đã loại, đừng thử lại: LovyanGFX cho màn (sai màu); `getTouch` của TFT_eSPI; sprite 8-bit (hình thô); nét làm mịn pha với màu cố định (vết cắt); vẽ tai mỗi khung (36ms).
- Commit message: một câu tiếng Anh, không dấu vết AI.

## Đang làm: đại tu biểu cảm (chưa commit)
- Người dùng muốn nhiều biểu cảm hơn, rõ nét hơn, sửa các biểu cảm kém (vui, giận, buồn, ngạc nhiên).
- `docs/design/mascot/prototype.html` (bản thứ 4 trong ngày): 12 biểu cảm (neutral, happy, excited, love, shy, sad, cry, angry, surprised, scared, confused, sleepy).
  - Mọi thông số đều liên tục, chạy bằng lò xo: mắt bo góc có mí trên/mí dưới cắt cong, to/nhỏ, con ngươi, tai, nảy/đung đưa/run.
    Người dùng xác nhận mắt mí cắt (giận, buồn...) là ổn.
  - **Mắt vui** dùng tham số `sm`: khối mắt bẹt dần và biến mất, còn lại vòm cong dày, hai đầu tròn. Lý do: kiểu khoét đáy mắt bị chê "cắt ẩu".
  - Bản 4 (mắt bo tròn, chỉ nghiêng) người dùng bảo không cần; mắt kiểu cũ đã được khôi phục.
  - Miệng: người dùng muốn có lại biểu cảm cần miệng, nhưng phải dễ thương.
    - `mouth()` hiện là khung ω + khe U nhỏ treo dưới đỉnh giữa (`oh`/`ow`), răng nanh (`fg`), lưỡi lè (`tg`), gợn sóng (`mv`), lệch (`ma`).
    - Người dùng vẫn chê "dị hợm".
    - Đã tìm hiểu cách vẽ miệng thú dễ thương: miệng nhỏ (miệng to "trông nguy hiểm"), đặt sát mắt; chỉ dùng "w" nhỏ, "o" nhỏ, răng nanh nhỏ.
    - Lỗi gốc: miệng đặt quá thấp (MOUTH_Y 236), thành cái mõm dài.
    - Bản mẫu giờ có `MOUTH_Y`, `MOUTH_S` chỉnh được. Đang thử MOUTH_Y 204, MOUTH_S 0.8.
    - Người dùng chỉ hướng: **khung ω giữ nguyên, biến tấu bằng phụ kiện**. Ví dụ cười lớn thì lòi 2 răng nanh. Khi nói thì dùng miệng D đặc như bản đầu tiên.
    - Đã làm theo hướng đó trong `mouth()`:
      - răng nanh `fL`/`fR`: cười lớn, hào hứng có 2 cái, giận có 1 cái;
      - lưỡi lè `tg`: tinh nghịch;
      - chữ o đặc `mo`: ngạc nhiên, giống bản đầu;
      - ω sâu/nông/rộng/lệch/gợn qua `mm`/`mw`/`ma`/`mv`;
      - đang nói: chữ D đặc.
    - Đã bỏ khe U.
    - Vị trí miệng vẫn như cũ (MOUTH_Y 236, MOUTH_S 1). Phương án dời miệng sát mắt (204, 0.8) chưa được người dùng chọn.
    - Sửa theo góp ý:
      - Đang nói: giữ chữ D suốt câu, độ há theo âm tiết. Mỗi âm tiết 0.16–0.30s, thỉnh thoảng ngắt. Làm mượt bằng lò xo k220/c26; mô phỏng ở 30 fps cho kết quả mép miệng đổi ≤ ~3px/khung.
      - Ngạc nhiên: giữ nét từ mũi xuống chữ o.
      - Tách các cặp người dùng thấy giống nhau:
        - cười lớn: mắt vòm, 2 răng nanh, tai thả, rung "ha ha ha" (`chk`);
        - hào hứng: mắt to tròn long lanh, tai dựng cao, lè lưỡi, nhảy tưng;
        - yêu: mí trên cong mềm sụp nửa chừng, con ngươi nở, nhìn thẳng, nghiêng đầu và đung đưa chậm.
      - ω thu hết trước khi chữ o lớn hẳn, nên không chồng hình.
      - Yêu (người dùng chê "không ra dáng yêu"): chính khối mắt thành trái tim cùng màu (`hrt`), dựng từ 2 hình tròn + đa giác nối mũi tim với **điểm tiếp tuyến phía ngoài** của mỗi hình tròn, mũi tim bo bằng đường cong bậc hai, không có nét viền; đập hai nhịp mỗi 1.2s.
        Mắt tim có tròng (người dùng yêu cầu), đảo theo hướng nhìn, hạ thấp để không chạm khe giữa hai thùy.
        Người dùng muốn tròng "hợp hơn với trái tim". Đã so 4 kiểu: khe dọc, tim nhỏ tối, tròn to, khe ngắn mập. **Người dùng chọn tim nhỏ tối** (tỉ lệ 0.32, cùng hàm `heartShape()`). Đã bỏ các kiểu còn lại.
        Đã gặp: tam giác tùy ý làm sườn tim gãy lõm; nét viền làm phình gờ ở tiếp điểm. Firmware nên vẽ tim vào sprite đệm như tai.
      - Buồn/khóc (bị chê "giống đang cười"): `mm < 0` lật hai nếp ω lên thành "︵︵", khóe trễ xuống.
    - **Đang chờ người dùng duyệt.**
    - Đã thử và hỏng: khóe ω uốn lên (trông như ria mép); khối há cùng màu (trông như răng hàm); khe trải từ nếp này sang nếp kia (trông như cái hàm).
  - Đơn sắc, không ký hiệu emoji; chỉ còn z khi ngủ và nước mắt cùng tông.
- Người dùng đã chê các bản trước:
  - Bản 1 (emoji: mắt ^ ^ / > <, tim, sao, 💢, má và lưỡi hồng): "gượng", "ai slop", hồng lạc quẻ.
  - Bản 2–3 (mí cắt góc, miệng há hoặc uốn): miệng "xấu, sợ, biến thái", mất nét động vật; mắt cười gượng.
  - Quy tắc rút ra: không đổi miệng, không cắt mắt góc nhọn, không ký hiệu.
- Kiểm chứng: đã mở trong browser pane, console không lỗi, lưới 12 biểu cảm vẽ đủ.
- **Đang chờ người dùng duyệt** bản mẫu. Duyệt xong thì:
  1. Port lên `firmware/src/mascot.*`. `Expression` enum phải mở rộng; `FaceShape` thêm trường mới, bỏ `cut`, `mw`, `mc`, `mo`.
  2. Đo fps bằng `python tools/serial_log.py COM5 8`.
  3. Cập nhật `spec.md`, ontology (enum Expression), commit trên nhánh mới.
- Bẫy đã gặp:
  - Lông mày nằm trùng chân tai (y 62–82) nên rối hình; đã bỏ.
  - Mí ghép từ 2 tam giác lộ vệt nối do làm mịn cạnh; phải dùng một tứ giác. Firmware cũng phải vẽ mí bằng một hình liền.
  - Mí vượt lên trên hộp mắt thì cắt mất chân tai.
  - Mắt cười bằng elip khoét đáy ra hình thanh dẹt, không đọc thành vui.
  - Tai xoay >10° trông như nút play; cụp tai thì dùng `earSq`.

## Port firmware (nhánh `feature/expressions`, chưa commit)
- `mascot.hpp/.cpp` đã port bản mẫu:
  - 15 biểu cảm, `FaceShape` 39 trường đều chạy qua lò xo;
  - mí cong tô theo cột (`Canvas::fillAbove/fillBelow`);
  - mắt tim (`Canvas::heart`, tiếp tuyến tính sẵn trong `HeartGeo`);
  - nhịp nói qua lò xo; bỏ má hồng và lấp lánh khi thông báo.
- Ảnh đệm: cụm mũi + miệng khi không nói (dán trong suốt) và vòm mắt cười (dán đặc, khung ôm sát để không đè tai). Chỉ vẽ lại khi thông số đổi (`quantKey`).
- `main.cpp`: lệnh Serial `e` (biểu cảm tiếp), `s` (trạng thái tiếp) để đo bằng script.
- Kiểm chứng:
  - `python -m platformio run -e cyd35 -t upload`: SUCCESS, RAM 7.9%, flash 27.2%.
  - Script quét fps (gửi `e`/`s` qua COM5, đọc dòng `[mascot] fps=`):
    - neutral 28.3, happy 25.9, laugh 29.1, excited 24.2, love 22.1, shy 24.9, smug 23.6, sad 22.5;
    - cry 19.7, angry 24.4, surprised 24.9, scared 21.5, confused 26.4, bored 22.1, sleepy 23.8;
    - listening 24.8, thinking 25.9, speaking 30.3, notifying 28.9;
    - heap 152148. Push SPI cố định ~26ms/khung.
  - Trước khi dùng ảnh đệm: happy 13 fps (vòm nét dày 26px tốn ~37ms), neutral 24.5 (ω 24 đoạn ~9ms). Dán trong suốt từng pixel cũng chậm, nên vòm dán đặc.
- Vòng góp ý 1 trên màn thật, đã sửa cả firmware lẫn bản mẫu:
  - mắt hào hứng bỏ con ngươi tròn to, về khe dọc với đốm sáng long lanh;
  - miệng buồn trễ vừa như bản đầu (mm -0.6, bỏ kéo khóe);
  - 3 chấm suy nghĩ dời ra x 414.., y 84 cho khỏi đè tai;
  - buồn ngủ dùng chữ o nhỏ;
  - cả mặt dời xuống 30px (EYE_Y 170, MOUTH_Y 266, tai +30, khung đẩy y 24..320);
  - giật: nhịp run hạ từ 53/47 xuống 23/19 rad/s (cũ trông như giật khi lấy mẫu 25 fps); cụm miệng dán đặc, vẽ trước mắt.
- Đo lại sau vòng 1:
  - neutral 30.0, happy 26.9, laugh 27.2, excited 30.3, love 24.9, shy 26.5, smug 26.2, sad 24.9;
  - cry 21.5, angry 25.2, surprised 27.0, scared 25.2, confused 26.2, bored 23.8, sleepy 24.4;
  - listening 27.5, thinking 27.6, speaking 30.3, notifying 29.2.
- Vòng 2: người dùng chê miệng buồn ngủ "dị dạng".
  - Nguyên nhân: ở mo 0.4, chữ ω chưa thu hết nên còn một ω tí xíu chồng lên chữ o.
  - Đã sửa công thức thành `half = 24*mw*max(0, 1 - mo*2.5)` ở cả firmware và bản mẫu. Chưa nạp lại board.
  - Bản mẫu thêm tham số `st` (0 = chữ o không nối mũi).
  - Người dùng chọn miệng ω nhỏ nếp nông (mw 0.7, mm 0.55), và muốn mắt "giống đang buồn ngủ hơn, đừng nhìn thẳng". Đã làm:
    - mí trên 0.66, mí dưới 0.12, con ngươi nhìn xuống (gy 0.6, gh 1);
    - đốm sáng tắt khi mí che quá nửa (áp cho mọi biểu cảm, như chán);
    - tham số `doze`: mí sụp thêm 0.14 rồi cố mở, đầu gật 3px, nhịp ~5.7s.
  - Đã nạp board. Đo lại: neutral 30.0, sleepy 24.3, cry 22.9 (thấp nhất), các biểu cảm khác 24–30.
- Vòng 3: người dùng chê chuyển biểu cảm chậm, không mượt; muốn chạm là tương tác; thông báo không ra trạng thái. Đã làm (firmware + bản mẫu):
  - lò xo biểu cảm k300 c30;
  - `roundedTri` bỏ nét dày 2r, thay bằng 3 dải cạnh tô đặc + 3 hình tròn góc + nét mảnh 1.5 (tai vẽ lại khi chuyển hết khựng 36ms);
  - vòm mắt cười vẽ bằng chuỗi hình tròn (`thickQuad`);
  - log thêm `max=` (khung chậm nhất trong 5 s).
  - Chạm (`main.cpp`):
    - 1 lần = `Mascot::tap`: trúng mắt thì nháy; trúng mũi/miệng thì "boop" (đá lò xo `sm`, nảy, giật tai); chỗ khác thì nhìn theo điểm chạm 1.8 s;
    - 2 lần trong 350 ms = biểu cảm tiếp;
    - giữ 0.6–5 s = trạng thái tiếp;
    - giữ > 5 s = hiệu chỉnh cảm ứng.
  - Thông báo: mặt hào hứng không lưỡi, nhìn thẳng; nhún 2 nhịp + giật tai mỗi 1.6 s; chấm báo + vòng lan ở (428, 76).
  - Đo (script `fps_sweep2`, đứng yên / khung chậm nhất khi chuyển):
    - neutral 29.8 / 42ms, happy 29.9 / 49, laugh 30.1 / 49, love 26.9 / 48, cry 23.6 / 46;
    - notifying 24.8 / 48, listening 27.7 / 52.
    - Trước vòng này: happy max 58–70ms.
- Vòng 4: thông báo bị chê "chưa rõ ràng". Đã làm:
  - viền màn hình sáng nhấp nháy (chu kỳ 0.8s, vẽ thẳng lên TFT sau các dải, xóa khi thôi);
  - chấm báo to (r 13) có dấu "!" + 2 vòng lan, vẽ trong sprite riêng 80x84 đẩy ở (400, 30), ngoài vùng mặt; nới vùng mặt ra x 470 thì tụt còn 21 fps;
  - tai vẫy mạnh (±300) theo nhịp nhún.
  - Đo: notifying 23.9 fps, max 48ms khi chuyển; heap 137296.
- Vòng 5:
  - **Chuyển đổi hình khối mắt:** người dùng muốn bình thường→vui và cười lớn→hào hứng chuyển thật nhanh. `sm`/`hrt` giờ không dùng lò xo mà bám mũ `x += (t - x)*min(1, dt*20)`, khoảng 3 khung; vẫn giữ vận tốc "boop".
  - **Viền thông báo nháy đen:** vùng mặt đè lên viền dưới và ảnh chấm báo đè lên viền phải, mỗi khung ghi đen rồi viền mới vẽ lại. Đã sửa: khung mặt y1 = 316, ảnh chấm báo rộng 76 (dừng ở x 476).
  - Đo: neutral 30.3, happy 29.7, laugh 30.0, excited 29.4, notifying 24.1. Khung chậm nhất khi chuyển 41–52ms.
  - **Đang chờ người dùng thử trên màn thật.**

## Việc tiếp theo
0. Người dùng xem bản port trên board; sửa theo góp ý. Sau đó cập nhật `spec.md`, ontology (enum Expression 15 giá trị), commit trên `feature/expressions`.
1. Bước 2 lộ trình: nối board với server Python trên PC qua WiFi + WebSocket để server điều khiển trạng thái và biểu cảm linh vật (chưa cần âm thanh).
2. Khi người dùng có loa: thử phát âm thanh qua DAC GPIO26, nhép miệng theo biên độ.
3. Chờ người dùng: mua loa, mic INMP441, pin; ảnh rõ mã chip 8 chân cạnh `2R2` (Q1), đo cổng BAT1 (Q2), token Muse có API key không (Q6). Chưa hỏi người dùng về Codex.

## Kiểm chứng
- `cd firmware && python -m platformio run -e cyd35 -t upload`: SUCCESS (RAM ~7%, flash ~26%).
- `python firmware/tools/serial_log.py COM5 8`: `[ready] mascot`, `[mascot] fps=27.4–30.0 frame=32–35ms (push 25ms) heap=198920`.
- Ontology check: `errors=0 warnings=8`. `python .claude/hooks/test_handoff_hooks.py`: OK.
- Người dùng xác nhận trên màn thật: liền mạch, sạch, mượt (2026-11-16).

## Rủi ro
- SPI 80MHz vượt mức thông dụng 40MHz; đang chạy sạch nhưng nếu màn nhiễu thì hạ về 40MHz (giảm còn ~19 fps).
- Lịch sử `main` đã viết lại và force push một lần (2026-11-16); bản cũ ở `refs/original/` trên máy.
- `serial_log.py` reset board qua RTS khi mở cổng; khi nó đang chạy thì không nạp được.
- Q11: chưa rõ vì sao LovyanGFX sai màu, chỉ cần nếu sau này muốn dùng LovyanGFX cho LVGL.
