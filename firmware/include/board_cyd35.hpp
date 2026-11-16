// Chân phần cứng của board ESP32-3248S035R (3.5", chữ in "ESP32-035") ngoài màn hình.
// Chân màn và cảm ứng nằm trong build_flags của platformio.ini (TFT_eSPI đọc từ đó).
// Mọi số chân phải khớp docs/hardware/esp32-3248s035r.md.
#pragma once

namespace board {

constexpr int kLcdBacklight = 27;  // Trùng TFT_BL trong platformio.ini.
constexpr int kTouchIrq = 36;      // XPT2046 báo có chạm (chưa dùng, TFT_eSPI đọc theo vòng lặp).

// LED RGB ở mặt trước, mức thấp là sáng.
constexpr int kLedRed = 4;
constexpr int kLedGreen = 16;
constexpr int kLedBlue = 17;

constexpr int kBootButton = 0;  // Nhấn = mức thấp.

}  // namespace board
