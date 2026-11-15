# ADR 0002: Bộ não đổi được: Muse API hoặc Claude API

- Ngày: 2026-11-16
- Trạng thái: Chấp nhận

## Bối cảnh
Người dùng có hai nguồn model: token Muse của Meta và credit API đi kèm gói Claude (chi tiết trong `docs/private/`). Chưa xác minh token Muse có gọi được qua API key không (open question Q6).

## Quyết định
- HomeServer gọi model qua một lớp `BrainProvider`, chọn bằng cấu hình (ví dụ `brain = "muse"` hoặc `brain = "claude"`).
- Dùng **API key** của nhà cung cấp. Không dùng đăng nhập gói thuê bao claude.ai trong server: Anthropic không cho sản phẩm bên thứ ba dùng đăng nhập claude.ai, và Claude Code khởi động quá chậm cho giọng nói.
- FastBrain dùng model nhanh, rẻ (ví dụ Claude Haiku 5.5); SlowBrain dùng model mạnh hơn.
- Model chỉ nhận và trả chữ; speech-to-text và text-to-speech là thành phần riêng.

## Hệ quả
- Thêm nhà cung cấp mới chỉ cần viết thêm một provider.
- Phải viết lời dặn hệ thống (system prompt) và định nghĩa công cụ sao cho dùng được với cả hai nhà cung cấp.
