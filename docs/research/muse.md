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

## Nguồn
- https://www.hackster.io/news/meta-brings-its-muse-ai-agent-to-esp32-and-raspberry-pi-f6694ff1a306
- https://www.unite.ai/meta-open-sources-muse-gadget-sdks-for-diy-ai-hardware-devices/
- https://hackernoon.com/i-built-my-own-muse-charm-on-an-esp32-stick-one-fast-brain-one-slow-brain
- https://www.tomsguide.com/ai/meta-is-making-a-tamagotchi-like-ai-companion-for-the-ai-era-meet-the-muse-charm
- https://www.yottalabs.ai/post/meta-muse-wonder-pricing-api-cost-muse-code-plans
