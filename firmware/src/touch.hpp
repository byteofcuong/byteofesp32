// Cảm ứng XPT2046: đọc theo thuật toán LovyanGFX và hiệu chỉnh 4 điểm (ADR 0005).
#pragma once

#include <TFT_eSPI.h>

namespace touch {

// Gọi sau tft.init(). Nạp hiệu chỉnh từ NVS; chưa có thì chạy hiệu chỉnh + kiểm tra.
void begin(TFT_eSPI& tft);

// Chân PENIRQ đang báo chạm.
bool pressed();

// Tọa độ màn hình của điểm đang chạm; false nếu không chạm hoặc mẫu hỏng.
bool read(int32_t* x, int32_t* y);

// Chạm giữ 4 điểm để hiệu chỉnh, lưu NVS.
void calibrate();

// Chạm 5 điểm, in sai số ra Serial ([touchtest] worst=...).
void verify();

void waitRelease();

}  // namespace touch
