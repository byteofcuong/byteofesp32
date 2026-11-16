# Meta Muse (nghiên cứu 2026-11-15)

## Muse Charm
Thiết bị cỡ lòng bàn tay Meta giới thiệu tháng 9/2026: linh vật trên màn (kiểu Tamagotchi), bấm nút để nói với trợ lý Muse. Là hình mẫu trải nghiệm của dự án.

## Muse Gadgets SDK (ra 2026-10-02)
- Firmware ESP32 + SDK Linux, Apache 2.0: https://github.com/facebookincubator/muse-gadget-sdk
- Cần SDK token (gadgets.muse.ai) và ghép đôi qua app Muse (Developer mode).
- Target: esp32, esp32s3, esp32c5, esp32c6. Kích hoạt: bấm để nói, voice note (giữ nút 600ms), gọi thoại (thử nghiệm). Không thấy từ đánh thức.
- Board không PSRAM: không có tunnel mạng nhà; TTS và gọi thoại chỉ bật cho board có PSRAM.
- CYD 2.8" (`sdkconfig.esp32-2432s028`): chỉ màn trạng thái, không cảm ứng, không mic/loa, nhiễu ở đáy màn. Không có cấu hình cho 3248S035.
- Điều khoản SDK: theo một nguồn, dùng cá nhân, phi thương mại.
- Kết luận cho dự án: ADR 0004.

## Muse API và Muse Code
Theo các bài viết thứ cấp (chưa thống nhất về tên model và giá): có Muse API trả theo token và gói Muse Code (trợ lý lập trình) trả theo tháng. Cần xác minh trên trang developer của Meta (open question Q6).

## Meta Model API (xác minh 2026-11-16 trên trang của Meta)
- API chính thức. Tạo key tại dev.meta.ai ("Start building", key một chạm). Base URL `https://api.meta.ai/v1`. Tương thích OpenAI SDK, cũng nhận định dạng Anthropic Messages. Biến môi trường mẫu: `MODEL_API_KEY`.
- Trả theo token, riêng với app Muse. Giá mỗi triệu token (vào / ra): `muse-spark-1.3` $1.25 / $4.25 (1M context); `muse-ember-1.0` $0.10 / $0.50 (≤200k); `muse-wonder-1.0` $4 / $20. Bản "contributor" rẻ hơn nhưng dữ liệu được dùng để cải thiện sản phẩm Meta. Muse Voice Transcribe $3 / 1000 phút.
- Trang của Meta không nói token trong app Muse (muse.ai) dùng được qua API.

## muse2api (không chính thức)
- Nhiều repo GitHub cùng tên (bản phổ biến nhất ~1.3k sao, 2026-11): đóng gói ngược (reverse-engineer) giao diện web muse.ai thành API kiểu OpenAI, dùng phiên đăng nhập của tài khoản, xoay vòng nhiều tài khoản, có bản kèm "máy đăng ký" tài khoản tự động.
- Không phải của Meta. Rủi ro: trái điều khoản nên tài khoản có thể bị khóa; phải đưa cookie/phiên đăng nhập cho mã bên thứ ba; hỏng mỗi khi Meta đổi giao thức.
- Đo trên máy người dùng 2026-11-16 (Docker, model `muse-spark`, stream, 3 câu hỏi ngắn tiếng Việt): chữ đầu tiên sau 20.1s (lần đầu, trình duyệt còn khởi động), 18.1s (câu hỏi thời tiết, Muse tự tra web), 10.9s. Thêm 3 câu chỉ cần trả lời một từ: 12.5s, 12.5s, 7.9s. Chậm hơn nhiều so với 2–3s README ghi; phần chậm nằm ở Muse vì muse2api tái dùng cùng một thread (chỉ mở thread mới khi đủ 24 bong bóng hoặc trang bị kẹt). Trả lời tiếng Việt tốt. System prompt chỉ có tác dụng một phần: nó vẫn tự xưng là Muse và gọi người dùng theo tên trong hồ sơ tài khoản.
- Người dùng vẫn chọn dùng nó, có giới hạn (ADR 0006): chạy ngoài repo, chỉ mở cổng 127.0.0.1, ghim commit đã đọc, một tài khoản của chính người dùng.

## Nguồn
- https://dev.meta.ai/products/meta-model-api/
- https://dev.meta.ai/resources/blog/build-with-muse-spark/
- https://www.hackster.io/news/meta-brings-its-muse-ai-agent-to-esp32-and-raspberry-pi-f6694ff1a306
- https://www.unite.ai/meta-open-sources-muse-gadget-sdks-for-diy-ai-hardware-devices/
- https://hackernoon.com/i-built-my-own-muse-charm-on-an-esp32-stick-one-fast-brain-one-slow-brain
- https://www.tomsguide.com/ai/meta-is-making-a-tamagotchi-like-ai-companion-for-the-ai-era-meet-the-muse-charm
- https://www.yottalabs.ai/post/meta-muse-wonder-pricing-api-cost-muse-code-plans
