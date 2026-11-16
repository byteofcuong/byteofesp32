# ADR 0005: Firmware dùng PlatformIO + Arduino + TFT_eSPI

- Ngày: 2026-11-16
- Trạng thái: Chấp nhận

## Bối cảnh
Cần chọn công cụ viết firmware (open question Q9). Người dùng mới làm phần cứng; agent cần build, nạp và đọc log bằng dòng lệnh để tự kiểm chứng. Màn ST7796 và cảm ứng XPT2046 dùng chung bus SPI; sau này cần LVGL.

## Quyết định
- Build và nạp bằng **PlatformIO Core** (CLI, cài bằng pip), project ở `firmware/`, cấu hình trong `firmware/platformio.ini`.
- Framework **Arduino** cho ESP32 (platform espressif32 8.0.0, Arduino core 3.3.12).
- Thư viện màn hình **TFT_eSPI 2.5.43**, cấu hình hoàn toàn bằng `build_flags` (`USER_SETUP_LOADED`), không sửa file trong thư viện. Firmware gốc của nhà sản xuất cũng dùng TFT_eSPI + LVGL v8.
- **Cảm ứng XPT2046 không dùng hàm của TFT_eSPI.** Firmware tự đọc theo thuật toán của LovyanGFX (`Touch_XPT2046::getTouchRaw`): chân PENIRQ GPIO36 mức thấp mới coi là chạm; một lần truyền SPI 1MHz đọc 7 lượt Y/Z1/X/Z2, lấy trung vị. Dùng chung đối tượng SPI của TFT_eSPI (`getSPIinstance()`).
- Hiệu chỉnh cảm ứng tự viết: chạm giữ 4 điểm cách mép 30px, mỗi điểm trung bình 16 lần đọc, tự nhận trục bị đổi; lưu NVS (namespace `touch`, khóa `cal_lgfx`). Nút "CAN CHINH" trên màn để làm lại, kèm kiểm tra 5 điểm in sai số ra Serial.

## Phương án đã loại
- **LovyanGFX 1.3.0**: thử ngày 2026-11-16, hiện sai màu trên tấm màn này (đỏ ra xanh lá, xanh lá ra xanh dương, xanh dương ra đen, trắng ra xanh nhạt). Đã thử và đều không sửa được: đảo màu, BGR, thanh ghi B6 0x02, màu 24-bit, SPI 10MHz, chuỗi khởi tạo của LCDwiki. Đọc ngược bộ nhớ màn vẫn đúng. Chip màn trả ID 7796. TFT_eSPI cùng chân, cùng 40MHz thì đúng màu. Chưa rõ lệnh nào gây khác biệt.
- ESP-IDF thuần: khó cho người mới, chưa cần.
- Arduino IDE: không chạy lặp lại bằng dòng lệnh được.

## Hệ quả
- Lần build đầu PlatformIO tải bộ công cụ ESP32 (vài trăm MB) vào `~/.platformio`.
- Nạp firmware xóa firmware demo; bản sao lưu flash gốc ở `backups/factory_backup.bin` (không commit), khôi phục bằng `python -m esptool --port COM5 write-flash 0 backups/factory_backup.bin`.
- Cảm ứng của TFT_eSPI (`getTouch`, `calibrateTouch`) đã thử và loại: nét đứt quãng, lệch dần về nửa dưới màn, đoạn lạ ở đầu/cuối nét khi lực ấn yếu. Các bộ lọc vá (ngưỡng lực, cắt đầu/đuôi theo thời gian) đánh đổi nhạy lấy sạch mà vẫn không ổn. Đọc kiểu LovyanGFX thì người dùng xác nhận tốt như bản đầu.
- Không đọc nút BOOT lúc khởi động để làm gì: giữ BOOT (GPIO0) khi reset làm chip vào chế độ nạp code.
