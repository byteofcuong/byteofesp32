# ADR 0006: Gọi Muse qua muse2api chạy riêng trên máy

- Ngày: 2026-11-16
- Trạng thái: Chấp nhận
- Thay thế một phần ADR 0002 (cách lấy khóa cho provider Muse). Phần `BrainProvider` đổi được của ADR 0002 giữ nguyên.

## Bối cảnh
- ADR 0002 định dùng API key chính thức của từng nhà cung cấp. Với Muse, API chính thức là Meta Model API (key ở dev.meta.ai, trả theo token riêng, xem `docs/research/muse.md`).
- Người dùng có tài khoản Muse trên muse.ai (chi tiết ở `docs/private/`) và muốn dùng nó. Đăng nhập dev.meta.ai từ máy người dùng báo "Token exchange failed" (2026-11-16).
- `muse2api` (github.com/czg86389-hub/muse2api, MIT) đóng gói ngược web muse.ai thành API kiểu OpenAI (`/v1/chat/completions`), chạy Chromium không giao diện bằng phiên đăng nhập (cookie) của tài khoản.
- Đọc mã ở commit `19db32a3d6ef` (2026-11-16): cookie chỉ gửi tới `muse.ai`. Ra ngoài còn có GitHub (kiểm tra bản mới, chỉ đọc) và `api.ipify.org` (thử proxy, không kèm cookie). Mặc định lắng nghe `0.0.0.0:18610`.

## Quyết định
- Người dùng chọn dùng `muse2api` cho provider Muse, dù đã được báo rủi ro.
- `muse2api` chạy **ngoài repo này**, thành một dịch vụ riêng trên cùng máy với HomeServer (Docker Desktop). Không chép mã của nó vào repo, không commit dữ liệu của nó (`data/accounts.json` chứa cookie).
- Chỉ mở cổng cho máy mình: ánh xạ `127.0.0.1:18610:18610`, không mở ra mạng LAN hay Internet.
- Ghim vào một commit đã đọc. Không dùng nút "nâng cấp trực tuyến"; muốn lên bản mới thì đọc diff rồi mới đổi commit.
- Chỉ dùng một tài khoản của chính người dùng. Không dùng tính năng xoay vòng nhiều tài khoản hay công cụ đăng ký tài khoản tự động.
- HomeServer gọi nó như một API kiểu OpenAI: `base_url = http://127.0.0.1:18610/v1`, khóa là `MUSE2API_KEY` trong `server/.env`.
- **Chỉ dùng Muse** (người dùng quyết 2026-11-16, sau khi xem số đo độ trễ): cả FastBrain lẫn SlowBrain đều gọi Muse qua muse2api. Không dùng Claude API. Lớp `BrainProvider` của ADR 0002 vẫn giữ để đổi được về sau.
- Độ trễ chấp nhận: đo được 7.9–20.1s tới chữ đầu tiên, kể cả câu trả lời một từ. Nguyên nhân nằm ở Muse (agent chạy máy ảo trên cloud), không ở muse2api: muse2api tái dùng cùng một thread, chỉ mở thread mới khi đủ 24 bong bóng. Bù bằng giao diện: Device chuyển sang trạng thái `thinking` ngay khi nhận câu hỏi.

## Hệ quả
- Rủi ro: trái điều khoản của Meta, tài khoản có thể bị khóa; hỏng khi Meta đổi giao diện web; phiên đăng nhập hết hạn thì phải nhập lại cookie.
- Độ trễ chữ đầu tiên đo thật 7.9–20.1s (README ghi 2–3s). Mỗi lượt nói sẽ chờ lâu; con cáo phải hiện `thinking` suốt lúc chờ.
- Mọi lượt hỏi rơi vào cùng một thread trên muse.ai (Muse nhớ ngữ cảnh giữa các lượt, và người dùng thấy chúng trong lịch sử web). System prompt bị ghép vào câu hỏi, chỉ có tác dụng một phần.
- Máy chạy server cần Docker và đủ RAM cho Chromium (`shm_size` 2GB trong `docker-compose.yml` của họ).
- Nếu sau này dùng được Meta Model API chính thức thì chỉ cần đổi `base_url` và khóa, không phải sửa code.
