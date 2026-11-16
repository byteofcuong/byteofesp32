// Đọc XPT2046 theo thuật toán của LovyanGFX (Touch_XPT2046::getTouchRaw), vì cảm ứng của
// TFT_eSPI đứt nét và tự vẽ đoạn lạ (ADR 0005):
//  - chân PENIRQ (GPIO36) mức thấp mới coi là đang chạm, không đoán qua lực ấn;
//  - một lần truyền SPI đọc 7 lượt Y, Z1, X, Z2 rồi lấy trung vị, loại mẫu nhiễu.
#include "touch.hpp"

#include <Arduino.h>
#include <Preferences.h>
#include <SPI.h>

#include <algorithm>

#include "board_cyd35.hpp"

namespace touch {
namespace {

TFT_eSPI* tft = nullptr;
Preferences prefs;

constexpr uint32_t kTouchSpiHz = 1000000;
constexpr int kCalMargin = 30;  // Điểm hiệu chỉnh cách mép màn 30px.
constexpr char kCalKey[] = "cal_lgfx";

struct TouchCal {
  uint8_t swap;  // 1: trục x màn hình ứng với trục y thô.
  int16_t rx0, rx1, ry0, ry1;  // Giá trị thô tại x = margin / width-margin, y = margin / height-margin.
};
TouchCal cal{};
bool cal_ok = false;

// Một lần đọc: trung vị của 7 lượt. False nếu không chạm hoặc mẫu hỏng.
bool readRawMedian(int32_t* rx, int32_t* ry) {
  if (!pressed()) return false;
  uint8_t d[57] = {0};
  for (int j = 0; j < 7; ++j) {
    d[j * 8 + 0] = 0x91;  // Y
    d[j * 8 + 2] = 0xB1;  // Z1
    d[j * 8 + 4] = 0xD1;  // X
    d[j * 8 + 6] = 0xC1;  // Z2
  }
  d[56] = 0x80;  // Kết thúc: tắt nguồn, bật lại PENIRQ.
  SPIClass& spi = tft->getSPIinstance();
  spi.beginTransaction(SPISettings(kTouchSpiHz, MSBFIRST, SPI_MODE0));
  digitalWrite(TOUCH_CS, LOW);
  spi.transfer(d, sizeof(d));
  digitalWrite(TOUCH_CS, HIGH);
  spi.endTransaction();

  int xt[7], yt[7], zt[7];
  int ix = 0, iy = 0, iz = 0;
  for (int j = 0; j < 7; ++j) {
    const uint8_t* p = &d[j * 8];
    int x = (p[5] << 8 | p[6]) >> 3;
    int y = (p[1] << 8 | p[2]) >> 3;
    int z = 0x3200 + y - x + (((p[3] << 8 | p[4]) - (p[7] << 8 | p[8])) >> 1);
    if (x > 128 && x <= 3968) xt[ix++] = x;
    if (y > 128 && y <= 3968) yt[iy++] = y;
    if (z > 0) zt[iz++] = z;
  }
  if (ix < 3 || iy < 3 || iz < 3) return false;
  std::sort(xt, xt + ix);
  std::sort(yt, yt + iy);
  std::sort(zt, zt + iz);
  if ((zt[iz >> 1] >> 8) == 0) return false;
  *rx = xt[ix >> 1];
  *ry = yt[iy >> 1];
  return true;
}

void rawToScreen(int32_t rx, int32_t ry, int32_t* x, int32_t* y) {
  int32_t a = cal.swap ? ry : rx;
  int32_t b = cal.swap ? rx : ry;
  int32_t w = tft->width() - 2 * kCalMargin;
  int32_t h = tft->height() - 2 * kCalMargin;
  *x = kCalMargin + (a - cal.rx0) * w / (cal.rx1 - cal.rx0);
  *y = kCalMargin + (b - cal.ry0) * h / (cal.ry1 - cal.ry0);
}

void drawTarget(int x, int y, uint16_t color) {
  tft->drawLine(x - 12, y, x + 12, y, color);
  tft->drawLine(x, y - 12, x, y + 12, color);
  tft->drawCircle(x, y, 6, color);
}

// Chờ người dùng chạm giữ một điểm, trả về trung bình 16 lần đọc.
void captureRaw(int x, int y, int32_t* rx, int32_t* ry) {
  drawTarget(x, y, TFT_RED);
  for (;;) {
    while (!pressed()) delay(10);
    delay(60);  // Chờ lực ấn ổn định.
    int32_t sx = 0, sy = 0;
    int n = 0;
    while (n < 16 && pressed()) {
      int32_t ax, ay;
      if (readRawMedian(&ax, &ay)) {
        sx += ax;
        sy += ay;
        ++n;
      }
    }
    if (n == 16) {
      *rx = sx / n;
      *ry = sy / n;
      break;
    }
  }
  drawTarget(x, y, TFT_GREEN);
  waitRelease();
}

void showMessage(const char* line1, const char* line2) {
  tft->fillScreen(TFT_BLACK);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(line1, tft->width() / 2, tft->height() / 2 - 12, 2);
  tft->drawString(line2, tft->width() / 2, tft->height() / 2 + 12, 2);
}

}  // namespace

bool pressed() { return digitalRead(board::kTouchIrq) == LOW; }

void waitRelease() {
  while (pressed()) delay(10);
  delay(150);
}

bool read(int32_t* x, int32_t* y) {
  if (!cal_ok) return false;
  int32_t rx, ry;
  if (!readRawMedian(&rx, &ry)) return false;
  rawToScreen(rx, ry, x, y);
  return *x >= 0 && *y >= 0 && *x < tft->width() && *y < tft->height();
}

void calibrate() {
  showMessage("Hieu chinh cam ung", "Cham va giu vao tam dau cong do cho toi khi no xanh");
  Serial.println("[touch] calibrating");
  const int m = kCalMargin, w = tft->width(), h = tft->height();
  const int pts[4][2] = {{m, m}, {w - m, m}, {m, h - m}, {w - m, h - m}};  // TL, TR, BL, BR
  int32_t raw[4][2];
  for (int i = 0; i < 4; ++i) {
    captureRaw(pts[i][0], pts[i][1], &raw[i][0], &raw[i][1]);
    Serial.printf("[touch] cal point %d screen=(%d,%d) raw=(%ld,%ld)\n", i, pts[i][0], pts[i][1],
                  static_cast<long>(raw[i][0]), static_cast<long>(raw[i][1]));
  }
  // Trục thô nào đổi nhiều khi đi từ trái sang phải thì ứng với trục x màn hình.
  int32_t dx_raw_x = abs(raw[1][0] - raw[0][0]);
  int32_t dx_raw_y = abs(raw[1][1] - raw[0][1]);
  cal.swap = dx_raw_y > dx_raw_x ? 1 : 0;
  auto A = [&](int i) { return cal.swap ? raw[i][1] : raw[i][0]; };
  auto B = [&](int i) { return cal.swap ? raw[i][0] : raw[i][1]; };
  cal.rx0 = (A(0) + A(2)) / 2;
  cal.rx1 = (A(1) + A(3)) / 2;
  cal.ry0 = (B(0) + B(1)) / 2;
  cal.ry1 = (B(2) + B(3)) / 2;
  cal_ok = cal.rx1 != cal.rx0 && cal.ry1 != cal.ry0;
  if (cal_ok) prefs.putBytes(kCalKey, &cal, sizeof(cal));
  Serial.printf("[touch] cal swap=%d rx0=%d rx1=%d ry0=%d ry1=%d ok=%d\n", cal.swap, cal.rx0,
                cal.rx1, cal.ry0, cal.ry1, cal_ok);
}

void verify() {
  showMessage("Kiem tra cam ung", "Cham vao tam tung dau cong do");
  const int pts[5][2] = {{60, 60}, {420, 60}, {240, 160}, {60, 260}, {420, 260}};
  int worst = 0;
  for (int i = 0; i < 5; ++i) {
    int32_t rx, ry, x, y;
    captureRaw(pts[i][0], pts[i][1], &rx, &ry);
    rawToScreen(rx, ry, &x, &y);
    int err = static_cast<int>(sqrtf(sq(x - pts[i][0]) + sq(y - pts[i][1])));
    worst = max(worst, err);
    tft->fillCircle(x, y, 3, TFT_YELLOW);
    Serial.printf("[touchtest] target=(%d,%d) got=(%ld,%ld) err=%dpx\n", pts[i][0], pts[i][1],
                  static_cast<long>(x), static_cast<long>(y), err);
  }
  Serial.printf("[touchtest] worst=%dpx\n", worst);
  char msg[40];
  snprintf(msg, sizeof(msg), "Sai so lon nhat: %d px", worst);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(msg, tft->width() / 2, 300, 2);
  delay(2500);
}

void begin(TFT_eSPI& display) {
  tft = &display;
  pinMode(board::kTouchIrq, INPUT);  // GPIO36 chỉ input; board có điện trở kéo lên.
  prefs.begin("touch", false);
  if (prefs.getBytesLength(kCalKey) == sizeof(cal)) {
    prefs.getBytes(kCalKey, &cal, sizeof(cal));
    cal_ok = true;
    Serial.printf("[touch] calibration loaded swap=%d rx0=%d rx1=%d ry0=%d ry1=%d\n", cal.swap,
                  cal.rx0, cal.rx1, cal.ry0, cal.ry1);
  } else {
    calibrate();
    verify();
  }
}

}  // namespace touch
