// Firmware linh vật (docs/design/mascot/spec.md). Log qua Serial 115200 (firmware/tools/serial_log.py).
//
// Trạng thái và biểu cảm tự xuất hiện (docs/design/behavior.md, src/behavior.*): server quyết theo hội thoại,
// board tự phản xạ khi chạm và tự chán/buồn ngủ/ngủ khi rảnh. Không còn chạm để đổi biểu cảm hay trạng thái.
// Cảm ứng (xử lý tại chỗ, báo lên server):
//   chạm                  -> trúng mắt thì nháy, trúng mũi thì "boop", chỗ khác thì nhìn theo;
//                            chạm vài lần liền là vuốt ve (vui), chọc liên tục thì khó chịu
//   giữ 0,6–5 giây        -> xoa đầu (thương; xoa liền nữa thì ngượng); sau này dùng cho bấm-để-nói
//   giữ trên 5 giây       -> hiệu chỉnh lại cảm ứng
//   chạm lúc đang ngủ     -> giật mình rồi ngái ngủ
#include <Arduino.h>
#include <TFT_eSPI.h>

#include "behavior.hpp"
#include "board_cyd35.hpp"
#include "mascot.hpp"
#include "net_config.hpp"
#include "server_link.hpp"
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
      if (consumed) behavior::onWake(now);
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
    if (have_xy) behavior::onTap(x, y, now);
    server_link::sendTouch("tap");
  } else {
    behavior::onHold(now);
    server_link::sendTouch("long");
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
  behavior::begin(&mascot);
  Serial.println("[ready] mascot");
  server_link::begin();  // Sau khi linh vật đã giữ RAM cho sprite.
}

// Lệnh từ server (ADR 0007) qua lớp hành vi: biểu cảm có thể kèm thời gian giữ rồi phai (docs/design/behavior.md).
void applyServerCommands(uint32_t now) {
  server_link::Command cmd;
  while (server_link::poll(&cmd)) {
    if (cmd.kind == server_link::Command::Kind::State) {
      behavior::onServerState(static_cast<AssistantState>(cmd.value), now);
    } else {
      behavior::onServerExpression(static_cast<Expression>(cmd.value), cmd.hold_ms, now);
    }
  }
  server_link::report(mascot.state(), mascot.expression());
}

// Một dòng trạng thái mạng ở mép trên (y 4–22, ngoài vùng mặt từ y 24 và viền thông báo 4px), chỉ vẽ khi đổi.
// Chữ không dấu: font có sẵn của TFT_eSPI không có tiếng Việt.
void drawNetStatus(uint32_t now) {
  using server_link::Status;
  static int shown = -1;
  static uint32_t since = 0;
  static Status last = Status::Offline;
  Status s = server_link::status();
  if (s != last) {
    last = s;
    since = now;
  }
  char line[80] = "";
  switch (s) {
    case Status::Setup:
      snprintf(line, sizeof(line), "Cai WiFi: dien thoai vao mang %s, mat khau %s", server_link::setupSsid(),
               server_link::setupPass());
      break;
    case Status::Joining:
      if (now - since > 3000) snprintf(line, sizeof(line), "Dang vao WiFi...");
      break;
    case Status::NoServer:
      if (now - since > 6000) snprintf(line, sizeof(line), "Co WiFi, chua noi duoc server");
      break;
    default:
      break;
  }
  int key = line[0] == 0 ? 0 : static_cast<int>(s) + 1;
  if (key == shown) return;
  shown = key;
  tft.fillRect(6, 4, 468, 18, TFT_BLACK);
  if (line[0]) {
    tft.setTextColor(0x8C51, TFT_BLACK);  // Xám ấm, không tranh với linh vật.
    tft.setTextDatum(TC_DATUM);
    tft.drawString(line, 240, 5, 2);
  }
}

// Lệnh thử qua Serial: 'e' biểu cảm tiếp theo, 's' trạng thái tiếp theo (đo fps từng biểu cảm bằng script),
// 'w' mở trang cài WiFi, 'W' xóa mọi WiFi và địa chỉ server đã lưu rồi mở trang cài WiFi.
void handleSerial(uint32_t now) {
  while (Serial.available() > 0) {
    int c = Serial.read();
    // Đi qua lớp hành vi như lệnh server: lớp rảnh không chen vào giữa lúc đo (chán, buồn ngủ).
    if (c == 'e') behavior::onServerExpression(next(mascot.expression()), 0, now);
    if (c == 's') behavior::onServerState(next(mascot.state()), now);
    if (c == 'W') net_config::clear();
    if (c == 'w' || c == 'W') server_link::requestSetup();
  }
}

// Chế độ cài WiFi: trả RAM của linh vật cho WiFi và hiện hướng dẫn cố định (chữ không dấu: font không có tiếng Việt).
bool setupScreen() {
  static bool shown = false;
  if (server_link::status() != server_link::Status::Setup) return false;
  if (shown) return true;
  shown = true;
  mascot.end();
  setLed(false, false, true);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(0xFCC7, TFT_BLACK);  // Cam như con cáo.
  tft.drawString("Cai WiFi cho con cao", 24, 24, 4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[64];
  snprintf(buf, sizeof(buf), "1. Dien thoai vao WiFi:  %s", server_link::setupSsid());
  tft.drawString(buf, 24, 84, 2);
  snprintf(buf, sizeof(buf), "2. Mat khau:  %s", server_link::setupPass());
  tft.drawString(buf, 24, 114, 2);
  tft.drawString("3. Trang cai tu mo. Neu khong, mo trinh duyet: 192.168.4.1", 24, 144, 2);
  tft.drawString("4. Chon WiFi nha, nhap mat khau, bam Luu.", 24, 174, 2);
  tft.setTextColor(0x8C51, TFT_BLACK);
  tft.drawString("Luu xong board tu khoi dong lai va vao mang.", 24, 230, 2);
  server_link::setupMemoryReleased();
  return true;
}

void loop() {
  uint32_t now = millis();
  if (setupScreen()) {
    handleSerial(now);
    delay(20);
    return;
  }
  handleSerial(now);
  applyServerCommands(now);
  handleTouch(now);
  behavior::update(now);
  updateLed(now);
  mascot.update(now);
  drawNetStatus(now);
}
