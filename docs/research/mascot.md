# Linh vật trên màn: các repo tham khảo (nghiên cứu 2026-11-16)

## m5stack-avatar (khuôn mặt của robot Stack-chan)
- Tách **trạng thái khuôn mặt** khỏi **phần vẽ**. `DrawContext` giữ: biểu cảm (`Neutral, Happy, Angry, Sad, Doubt, Sleepy`), hướng nhìn (gaze) từng mắt, độ mở từng mắt, độ mở miệng, nhịp thở (breath), chữ trong bong bóng, bảng màu.
- Khuôn mặt ghép từ các bộ phận (`Eye`, `Mouth`, `Eyeblow`), mỗi bộ phận tự vẽ theo context; vẽ vào sprite rồi đẩy lên màn.
- Hai vòng chạy song song (task FreeRTOS): `drawLoop` vẽ lại khoảng 10ms một lần; `facialLoop` tạo biểu cảm tự nhiên:
  - đảo mắt (saccade) ngẫu nhiên mỗi 0,5–2,5 giây;
  - chớp mắt: mở 2,5–4,5 giây, nhắm 0,3–0,5 giây;
  - thở: khuôn mặt nhấp nhô nhẹ.
- Nhép miệng (lip sync): độ mở miệng lấy từ biên độ âm thanh đang phát.
- Repo gốc đã đóng băng ở v0.10.0.
- Nguồn: https://github.com/stack-chan/m5stack-avatar, dự án AI dùng nó: https://github.com/uezo/AIAvatarStackChan

## FluxGarage RoboEyes (mắt robot cho OLED)
- Tham số hình học: rộng, cao, bo góc từng mắt, khoảng cách giữa hai mắt; chế độ một mắt.
- Tâm trạng: `DEFAULT, HAPPY, TIRED, ANGRY`. Vị trí nhìn: 8 hướng + giữa.
- Hành vi tự động: chớp mắt ngẫu nhiên (`setAutoblinker(on, interval, variation)`), đảo mắt khi rảnh (`setIdleMode`), "tò mò" (mắt phía ngoài to lên khi nhìn sang bên).
- Hoạt cảnh một lần: `anim_confused()` (lắc ngang), `anim_laugh()` (nảy dọc).
- `update()` gọi trong `loop()`, tự giới hạn khung hình.
- Giấy phép **GPL-3.0**: chỉ học ý tưởng, không chép code vào repo.
- Nguồn: https://github.com/FluxGarage/RoboEyes

## Xiaozhi và các thành phần LVGL
- Cách 1: bộ GIF vẽ sẵn (ví dụ 21 cảm xúc, tên khớp với emoji Xiaozhi), LVGL phát bằng `lv_gif_set_src()`. Tốn flash và RAM, hợp board có PSRAM.
- Cách 2: vẽ bằng code lúc chạy (ví dụ `lvgl_kawaii_face`: mắt, lông mày, má hồng, miệng; `face_set_emotion(FACE_HAPPY, ...)` có chuyển cảnh mượt).
- Server gửi tên cảm xúc theo câu trả lời của LLM, thiết bị chỉ việc hiển thị.
- Nguồn: https://components.espressif.com/components/txp666/otto-emoji-gif-component, https://components.espressif.com/components/0015/lvgl_kawaii_face

## Rút ra cho dự án
- Tách hai trục: **trạng thái trợ lý** (idle, listening, thinking, speaking, notifying: do luồng hội thoại quyết) và **biểu cảm** (neutral, happy, sad, …: server gửi theo câu trả lời).
- Một "context" khuôn mặt (biểu cảm, hướng nhìn, độ mở mắt, độ mở miệng, thở, chữ) + bộ phận tự vẽ theo context.
- Hành vi tự nhiên chạy riêng: chớp mắt, đảo mắt, thở.
- Vẽ bằng code (không dùng GIF): board không PSRAM, flash 4MB.
- Nhép miệng theo biên độ âm thanh khi có loa.
