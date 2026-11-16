#include "behavior.hpp"

#include <time.h>

namespace behavior {
namespace {

// Ngưỡng và thời gian: bảng trong docs/design/behavior.md (đổi ở đây thì sửa cả bảng).
constexpr uint32_t kTapWindowMs = 3000;
constexpr int kPetTaps = 3;   // 3–6 lần chạm trong 3s: vuốt ve.
constexpr int kPokeTaps = 7;  // ≥ 7 lần trong 3s (~2.3 lần/giây): chọc liên tục.
constexpr uint32_t kHappyHold = 3000, kHappyCool = 6000;
constexpr uint32_t kAnnoyHold = 2500, kAnnoyCool = 20000;
constexpr uint32_t kLoveHold = 3500, kLoveCool = 15000, kShyHold = 3000;
constexpr uint32_t kBoredHold = 4000;
constexpr uint32_t kStartleHold = 600, kGroggyHold = 2000;

struct IdleTimes {
  uint32_t bored, sleepy, sleep;
};
constexpr IdleTimes kDay{60000, 120000, 180000};
constexpr IdleTimes kNight{30000, 45000, 75000};  // 22:00–6:00 giờ địa phương.
constexpr int kTapLog = 8;

Mascot* m = nullptr;
uint32_t taps[kTapLog];
int tap_count = 0;  // Số lần chạm đã ghi (vòng tròn kTapLog).
uint32_t happy_cool = 0, annoy_cool = 0, love_cool = 0;
uint32_t last_activity = 0;
bool bored_done = false;   // Mỗi đợt rảnh chỉ chán một lần.
bool idle_face = false;    // Biểu cảm hiện tại do lớp rảnh đặt (buồn ngủ): có tương tác thì gỡ.
uint32_t timed_until = 0;  // Biểu cảm có hẹn giờ phai; 0 = không.
bool has_next = false;     // Chuỗi hai bước (giật mình -> ngái ngủ -> bình thường).
Expression next_expr = Expression::Neutral;
uint32_t next_hold = 0;

bool reached(uint32_t now, uint32_t t) { return static_cast<int32_t>(now - t) >= 0; }

// localtime_r đọc lại chuỗi múi giờ mỗi lần gọi: chỉ tính lại mỗi giây, không phải mỗi vòng loop().
bool isNight(uint32_t now) {
  static uint32_t checked = 0;
  static bool night = false;
  if (checked != 0 && now - checked < 1000) return night;
  checked = now | 1;
  time_t t = time(nullptr);
  if (t < 1700000000) return night = false;  // Chưa có giờ thật (chưa nối mạng): coi là ban ngày.
  struct tm lt;
  localtime_r(&t, &lt);
  return night = lt.tm_hour >= 22 || lt.tm_hour < 6;
}

void show(Expression e, uint32_t hold_ms, uint32_t now, const char* why) {
  m->setExpression(e, now, true);
  timed_until = hold_ms ? now + hold_ms : 0;
  has_next = false;
  idle_face = false;
  Serial.printf("[behavior] %s -> %s hold=%lums\n", why, toString(e), static_cast<unsigned long>(hold_ms));
}

// Có người tương tác (chạm, lệnh server): đếm lại thời gian rảnh, gỡ mặt buồn ngủ.
void activity(uint32_t now) {
  last_activity = now;
  bored_done = false;
  if (idle_face) {
    idle_face = false;
    m->setExpression(Expression::Neutral, now, true);
  }
}

int recentTaps(uint32_t now) {
  int n = 0;
  for (int i = 0; i < min(tap_count, kTapLog); ++i) n += now - taps[i] <= kTapWindowMs;
  return n;
}

}  // namespace

void begin(Mascot* mascot) {
  m = mascot;
  last_activity = millis();
}

void onTap(int32_t x, int32_t y, uint32_t now) {
  activity(now);
  m->tap(x, y, now);
  // Đang hội thoại: chỉ phản ứng nhỏ, không đè cảm xúc của câu trả lời.
  if (m->state() != AssistantState::Idle) return;
  taps[tap_count++ % kTapLog] = now;
  int n = recentTaps(now);
  if (n >= kPokeTaps && reached(now, annoy_cool)) {
    show(Expression::Angry, kAnnoyHold, now, "poke");
    annoy_cool = happy_cool = now + kAnnoyCool;
    tap_count = 0;
  } else if (n >= kPetTaps && n < kPokeTaps && reached(now, happy_cool)) {
    show(Expression::Happy, kHappyHold, now, "pet");
    happy_cool = now + kHappyCool;
  }
}

void onHold(uint32_t now) {
  activity(now);
  if (m->state() != AssistantState::Idle) return;
  if (!reached(now, love_cool)) {
    show(Expression::Shy, kShyHold, now, "hold again");  // Xoa liên tục: ngượng.
  } else {
    show(Expression::Love, kLoveHold, now, "hold");
    love_cool = now + kLoveCool;
  }
}

void onWake(uint32_t now) {
  activity(now);
  show(Expression::Surprised, kStartleHold, now, "woken");
  has_next = true;
  next_expr = Expression::Sleepy;
  next_hold = kGroggyHold;
}

void onServerState(AssistantState s, uint32_t now) {
  activity(now);
  m->wake(now);
  m->setState(s, now);
}

void onServerExpression(Expression e, uint32_t hold_ms, uint32_t now) {
  activity(now);
  m->wake(now);
  show(e, hold_ms, now, "server");
}

void update(uint32_t now) {
  if (m == nullptr) return;
  // Hết thời gian giữ: sang bước sau của chuỗi, hoặc phai về bình thường (không tính là tương tác).
  if (timed_until != 0 && reached(now, timed_until)) {
    timed_until = 0;
    if (has_next) {
      has_next = false;
      m->setExpression(next_expr, now, false);
      timed_until = now + next_hold;
    } else {
      m->setExpression(Expression::Neutral, now, false);
    }
  }

  // Rảnh lâu: chán một lúc, rồi buồn ngủ, rồi ngủ (Mascot::sleeping dùng ngưỡng setSleepAfter).
  const IdleTimes& t = isNight(now) ? kNight : kDay;
  m->setSleepAfter(t.sleep);
  if (m->state() != AssistantState::Idle || timed_until != 0 || m->sleeping(now)) return;
  const uint32_t idle = now - last_activity;
  const Expression e = m->expression();
  if (idle >= t.sleepy && e == Expression::Neutral) {
    m->setExpression(Expression::Sleepy, now, false);
    idle_face = true;
    Serial.println("[behavior] idle -> sleepy");
  } else if (idle >= t.bored && !bored_done && e == Expression::Neutral) {
    m->setExpression(Expression::Bored, now, false);
    timed_until = now + kBoredHold;
    bored_done = true;
    Serial.println("[behavior] idle -> bored");
  }
}

}  // namespace behavior
