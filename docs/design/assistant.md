# Thiết kế trợ lý (đề xuất)

Trạng thái: đề xuất, chưa có code. Các quyết định nền: ADR 0001–0004 trong `docs/decisions/`.

## Thành phần
- **Device**: firmware trên board. Hiện linh vật, nhận chạm, thu mic, phát loa. Không giữ API key.
- **HomeServer**: chương trình Python ở nhà. Nghe từ đánh thức, chuyển giọng nói ↔ chữ, gọi bộ não, chạy công cụ.
- **Brain**: model ngôn ngữ, đổi được giữa các nhà cung cấp (`BrainProvider`).
  - **FastBrain**: trả lời ngay trong lượt hỏi đáp (~2–3 giây).
  - **SlowBrain**: việc chạy nền (nghiên cứu, nhắc việc), gửi kết quả về sau.

## Một lượt hỏi đáp (Turn)
1. Kích hoạt: chạm màn, hoặc server nghe thấy từ đánh thức.
2. Device stream âm thanh mic (PCM 16kHz) lên HomeServer qua WebSocket trong mạng nhà.
3. HomeServer: speech-to-text → FastBrain → text-to-speech.
4. HomeServer gửi âm thanh thô về Device theo từng câu; Device phát qua DAC GPIO26.
5. Chữ câu trả lời hiện trên màn.

## Trạng thái linh vật (Mascot)
`idle` → `listening` (được kích hoạt) → `thinking` (đã nói xong) → `speaking` (có âm thanh trả lời) → `idle`.
`notifying`: SlowBrain có kết quả, linh vật gọi người dùng; chạm vào thì chuyển sang `speaking`.

## Việc chạy nền (BackgroundTask)
`queued` → `running` → `done` → `delivered` (đã báo cho người dùng). Lỗi: `running` → `failed`.

## Giao thức Device ↔ HomeServer (phác thảo)
- WebSocket, không TLS, chỉ trong mạng nhà.
- Lên: khung âm thanh mic, sự kiện chạm.
- Xuống: lệnh đổi trạng thái linh vật, chữ hiển thị, khung âm thanh để phát.
