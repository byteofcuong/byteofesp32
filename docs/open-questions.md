# Câu hỏi mở

Điều chưa xác minh. Khi trả lời được: ghi kết quả, nguồn, ngày, rồi chuyển vào tài liệu tương ứng.

| # | Câu hỏi | Cách xác minh | Trạng thái |
|---|---|---|---|
| Q1 | Chip 8 chân cạnh cuộn `2R2` là chip gì (sạc + tăng áp?) | Ảnh chụp rõ mã in trên chip, tra datasheet | Mở |
| Q2 | Bước chân cổng BAT1: 1.25mm hay 2.0mm | Đo bằng thước / so với SPEAK1 | Mở (ảnh gợi ý 1.25mm) |
| Q3 | SW1 có đúng là công tắc nguồn pin | Gắn pin, rút USB, gạt thử | Mở |
| Q4 | Điện áp pin đọc qua chân ADC nào (GPIO34 là LDR) | Sketch đọc ADC khi có/không pin | Mở |
| Q5 | Màn là ST7796 hay ILI9488 | Đọc ID lệnh 0xD3 | **Đã trả lời 2026-11-16: ST7796** (ID `77 96`) |
| Q6 | Token Muse của người dùng (số lượng ở `docs/private/`) có dùng qua API key được không, có hạn dùng không | Trang developer của Meta; gọi thử một request từ server | **Đã trả lời 2026-11-16: ADR 0006.** Token trong app không có API key chính thức; dùng `muse2api` (đóng gói ngược web muse.ai) chạy riêng trên máy. API chính thức (Meta Model API, dev.meta.ai) trả theo token riêng, đăng nhập từ máy người dùng bị lỗi "Token exchange failed" |
| Q7 | Muse có dùng được ở Việt Nam và hiểu tiếng Việt không | Cài app Muse, thử | Mở |
| Q8 | Piper có giọng tiếng Việt đủ nghe không; cần TTS cloud không | Thử trên PC | Mở |
| Q9 | Chọn framework firmware: Arduino/PlatformIO hay ESP-IDF | ADR khi bắt đầu viết firmware | **Đã trả lời 2026-11-16: ADR 0005** (PlatformIO + Arduino + TFT_eSPI) |
| Q11 | Vì sao LovyanGFX 1.3.0 hiện sai màu trên tấm màn này | So từng lệnh khởi tạo với TFT_eSPI (bắt SPI hoặc đọc mã) | Mở, chỉ cần nếu sau này muốn dùng LovyanGFX |
| Q10 | Server chạy ở đâu: PC (chỉ khi bật) hay máy 24/7 | Người dùng quyết | **Đã trả lời 2026-11-16: ADR 0008** (PC ở nhà bật suốt + Cloudflare Tunnel + tên miền riêng) |
