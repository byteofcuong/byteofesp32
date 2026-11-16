// Firmware linh vật (docs/design/mascot/spec.md). Log qua Serial 115200 (firmware/tools/serial_log.py).
//
// Cảm ứng (để thử, sau này server điều khiển trạng thái và biểu cảm):
//   chạm nhanh vào mắt   -> nháy mắt đó, nảy lên
//   chạm nhanh chỗ khác  -> trạng thái trợ lý tiếp theo
//   giữ 0,6–5 giây        -> biểu cảm tiếp theo
//   giữ trên 5 giây       -> hiệu chỉnh lại cảm ứng
//   chạm lúc đang ngủ     -> chỉ đánh thức
#include <Arduino.h>
#include <TFT_eSPI.h>

#include "board_cyd35.hpp"
#include "mascot.hpp"
#include "touch.hpp"

namespace {

TFT_eSPI tft;
Mascot mascot;

constexpr uint32_t kLongPressMs = 600;
constexpr uint32_t kCalibratePressMs = 5000;
constexpr uint32_t kReleaseDebounceMs = 40;  // PENIRQ có thể chập chờn khi lực ấn đổi.

void setLed(bool r, bool g, bool b) {
  // LED chung anode: LOW là sáng.
  digitalWrite(board::kLedRed, r ? LOW : HIGH);
  digitalWrite(board::kLedGreen, g ? LOW : HIGH);
  digitalWrite(board::kLedBlue, b ? LOW : HIGH);
}

// LED theo trạng thái: nghe xanh lá, nghĩ xanh dương nháy, nói vàng cam, thông báo vàng nháy.
void updateLed(uint32_t now) {
  bool blink_on = (now / 300) % 2 == 0;
  if (mascot.sleeping(now)) {
    setLed(false, false, false);
    return;
  }
  switch (mascot.state()) {
    case AssistantState::Listening: setLed(false, true, false); break;
    case AssistantState::Thinking: setLed(false, false, blink_on); break;
    case AssistantState::Speaking: setLed(true, true, false); break;
    case AssistantState::Notifying: setLed(blink_on, blink_on, false); break;
    default: setLed(false, false, false);
  }
}

template <typename E>
E next(E e) {
  return static_cast<E>((static_cast<int>(e) + 1) % static_cast<int>(E::kCount));
}

void handleTouch(uint32_t now) {
  static bool down = false;
  static uint32_t down_at = 0, last_seen = 0;
  static int32_t x = 0, y = 0;
  static bool have_xy = false, consumed = false;

  if (touch::pressed()) {
    last_seen = now;
    if (!down) {
      down = true;
      down_at = now;
      have_xy = false;
      consumed = mascot.sleeping(now);  // Chạm lúc ngủ chỉ để đánh thức.
      mascot.wake(now);
      if (consumed) Serial.println("[mascot] woke up");
    }
    if (!have_xy) have_xy = touch::read(&x, &y);
    if (!consumed && now - down_at > kCalibratePressMs) {
      consumed = true;
      touch::waitRelease();
      touch::calibrate();
      touch::verify();
      tft.fillScreen(TFT_BLACK);
      down = false;
    }
    return;
  }
  if (!down || now - last_seen < kReleaseDebounceMs) return;
  down = false;
  if (consumed) return;
  if (last_seen - down_at < kLongPressMs) {
    if (!(have_xy && mascot.poke(x, y, now))) mascot.setState(next(mascot.state()), now);
  } else {
    mascot.setExpression(next(mascot.expression()), now);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("[boot] byteofesp32 mascot (fox robot eyes)");

  pinMode(board::kLedRed, OUTPUT);
  pinMode(board::kLedGreen, OUTPUT);
  pinMode(board::kLedBlue, OUTPUT);
  setLed(false, false, false);

  tft.init();  // Bật luôn đèn nền GPIO27 (TFT_BL), cấu hình chân TOUCH_CS.
  tft.setRotation(1);  // Nằm ngang 480x320.
  tft.fillScreen(TFT_BLACK);
  Serial.printf("[lcd] init ok, %dx%d, backlight GPIO%d\n", tft.width(), tft.height(), board::kLcdBacklight);
  Serial.printf("[mem] free heap %lu, psram %lu\n", static_cast<unsigned long>(ESP.getFreeHeap()),
                static_cast<unsigned long>(ESP.getPsramSize()));

  touch::begin(tft);
  if (!mascot.begin(tft)) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Khong du RAM cho linh vat", 20, 20, 2);
  }
  Serial.println("[ready] mascot");
}

void loop() {
  uint32_t now = millis();
  handleTouch(now);
  updateLed(now);
  mascot.update(now);
}
