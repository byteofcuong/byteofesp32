#include "mascot.hpp"

#include <Arduino.h>

#include <cmath>

namespace {

// ---- Thông số thiết kế: phải khớp docs/design/mascot/prototype.html ----
constexpr float CX = 240, EYE_DX = 78, EYE_Y = 140, MOUTH_Y = 236;

// neutral, happy, sad, angry, surprised, sleepy
//                        w    h    r   lid  slope  cut    dy  mw   mc   mo  blush ear earSq  pw
constexpr FaceShape kShapes[] = {
    {86, 104, 30, 0.00f, 0.00f, 0.00f, 0, 34, 6, 0.0f, 0, 0, 1.00f, 0.14f},
    {92, 100, 34, 0.00f, 0.00f, 0.36f, -6, 52, 10, 0.3f, 1, -4, 1.05f, 0.20f},
    {84, 92, 30, 0.32f, 0.50f, 0.00f, 8, 32, -10, 0.0f, 0, 8, 0.70f, 0.24f},
    {88, 96, 26, 0.32f, -0.55f, 0.00f, 2, 30, -4, 0.0f, 0, 6, 0.82f, 0.09f},
    {98, 112, 49, 0.00f, 0.00f, 0.00f, -4, 16, 0, 0.9f, 0, -6, 1.08f, 0.36f},
    {90, 104, 30, 0.58f, 0.10f, 0.00f, 6, 20, 2, 0.0f, 0, 6, 0.78f, 0.14f},
};

// Tai trái: chân ngoài, chân trong, chóp; trục xoay. Tai phải đối xứng qua x = CX.
constexpr float kEar[3][2] = {{94, 82}, {192, 62}, {124, 4}};
constexpr float kEarPivot[2] = {143, 72};

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
constexpr uint16_t kBg = 0x0000;
constexpr uint16_t kWarm = rgb(0xFF, 0x9A, 0x3C);       // Rảnh, đang nói (cáo).
constexpr uint16_t kListen = rgb(0x6E, 0xF2, 0xA6);
constexpr uint16_t kThink = rgb(0xB7, 0x9B, 0xFF);
constexpr uint16_t kNotify = rgb(0xFF, 0xD2, 0x5E);
constexpr uint16_t kSleep = rgb(0x5B, 0x6B, 0x7A);
constexpr uint16_t kBlush = rgb(0xFF, 0x7E, 0xB3);
constexpr uint16_t kWhite = 0xFFFF;

constexpr uint32_t kFrameMs = 33;          // Tối đa ~30 khung/giây.
constexpr uint32_t kSleepAfterMs = 30000;  // Rảnh, biểu cảm bình thường, để yên 30 giây thì ngủ.
constexpr int kBandH = 56;                 // Vẽ theo dải 56 dòng.
constexpr int kBandW = 448;                // Bề rộng tối đa vùng vẽ.
// Ảnh tai: trục xoay đặt tại (kEarOX, kEarOY) trong ảnh; đủ chỗ cho tai nghiêng ±10° và ép 0.7–1.1.
constexpr int kEarW = 150, kEarH = 124, kEarOX = 75, kEarOY = 100;

float frand(float a, float b) { return a + (b - a) * (random(10001) / 10000.0f); }

// Pha trộn hai màu RGB565 (k = 0 → a, 1 → b).
uint16_t mix(uint16_t a, uint16_t b, float k) {
  k = k < 0 ? 0 : (k > 1 ? 1 : k);
  int ar = a >> 11, ag = (a >> 5) & 63, ab = a & 31;
  int br = b >> 11, bg = (b >> 5) & 63, bb = b & 31;
  return (uint16_t)(((int)(ar + (br - ar) * k) << 11) | ((int)(ag + (bg - ag) * k) << 5) | (int)(ab + (bb - ab) * k));
}

// Vẽ có dịch gốc tọa độ: mọi hình nhận tọa độ màn hình, tự trừ gốc của dải đang vẽ.
// Hình nằm ngoài dải đang vẽ thì bỏ qua (các hàm làm mịn cạnh tốn công dù bị cắt hết).
struct Canvas {
  TFT_eSprite* s;
  float ox, oy;
  float hh = kBandH;  // Chiều cao vùng vẽ (dải hoặc ảnh tai).
  int X(float x) const { return (int)lroundf(x - ox); }
  int Y(float y) const { return (int)lroundf(y - oy); }
  bool vis(float y0, float y1) const { return y1 >= oy - 2 && y0 <= oy + hh + 2; }

  void smoothRect(float x, float y, float w, float h, float r, uint16_t c, uint16_t bg) {
    int iw = (int)lroundf(w), ih = (int)lroundf(h);
    if (iw < 1 || ih < 1 || !vis(y, y + h)) return;
    int ir = (int)lroundf(fminf(r, fminf(iw, ih) / 2.0f - 0.5f));
    s->fillSmoothRoundRect(X(x), Y(y), iw, ih, ir < 0 ? 0 : ir, c, bg);
  }
  void circle(float x, float y, float r, uint16_t c, uint16_t bg) {
    if (vis(y - r, y + r)) s->fillSmoothCircle(X(x), Y(y), (int)lroundf(r), c, bg);
  }
  void ellipse(float x, float y, float rx, float ry, uint16_t c) {
    if (vis(y - ry, y + ry)) s->fillEllipse(X(x), Y(y), (int)lroundf(rx), (int)lroundf(ry), c);
  }
  void tri(float x0, float y0, float x1, float y1, float x2, float y2, uint16_t c) {
    if (vis(fminf(y0, fminf(y1, y2)), fmaxf(y0, fmaxf(y1, y2)))) s->fillTriangle(X(x0), Y(y0), X(x1), Y(y1), X(x2), Y(y2), c);
  }
  // Nét dày làm mịn cạnh. Mép pha với ĐÚNG pixel bên dưới (kReadBg): nếu pha với một màu cố định,
  // chỗ các đoạn chồng lên nhau thành vệt tối như vết cắt (người dùng thấy ở tai và miệng, 2026-11-16).
  static constexpr uint32_t kReadBg = 0x00FFFFFF;
  void line(float x0, float y0, float x1, float y1, float wd, uint16_t c) {
    if (vis(fminf(y0, y1) - wd, fmaxf(y0, y1) + wd)) s->drawWideLine(x0 - ox, y0 - oy, x1 - ox, y1 - oy, wd, c, kReadBg);
  }
  // Đường cong bậc hai nét tròn: lấy mẫu rồi nối bằng các đoạn thẳng dày có đầu tròn.
  void quad(float x0, float y0, float cx, float cy, float x1, float y1, float wd, uint16_t c) {
    float px = x0, py = y0;
    for (int i = 1; i <= 8; ++i) {
      float t = i / 8.0f, u = 1 - t;
      float x = u * u * x0 + 2 * u * t * cx + t * t * x1, y = u * u * y0 + 2 * u * t * cy + t * t * y1;
      line(px, py, x, y, wd, c);
      px = x;
      py = y;
    }
  }
  // Tam giác bo góc bán kính r (tổng Minkowski): tam giác co vào r quanh tâm nội tiếp + nét dày 2r đầu tròn.
  void roundedTri(const float p[3][2], float r, uint16_t c) {
    float a = hypotf(p[1][0] - p[2][0], p[1][1] - p[2][1]);
    float b = hypotf(p[0][0] - p[2][0], p[0][1] - p[2][1]);
    float d = hypotf(p[0][0] - p[1][0], p[0][1] - p[1][1]);
    float per = a + b + d;
    float ix = (a * p[0][0] + b * p[1][0] + d * p[2][0]) / per, iy = (a * p[0][1] + b * p[1][1] + d * p[2][1]) / per;
    float area = fabsf((p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) - (p[2][0] - p[0][0]) * (p[1][1] - p[0][1])) / 2;
    float inr = 2 * area / per;
    r = fminf(r, inr * 0.85f);
    float k = 1 - r / inr;
    float q[3][2];
    for (int i = 0; i < 3; ++i) {
      q[i][0] = ix + (p[i][0] - ix) * k;
      q[i][1] = iy + (p[i][1] - iy) * k;
    }
    tri(q[0][0], q[0][1], q[1][0], q[1][1], q[2][0], q[2][1], c);
    for (int i = 0; i < 3; ++i) line(q[i][0], q[i][1], q[(i + 1) % 3][0], q[(i + 1) % 3][1], 2 * r, c);
  }
};

}  // namespace

const char* toString(AssistantState s) {
  static const char* names[] = {"idle", "listening", "thinking", "speaking", "notifying"};
  return names[static_cast<int>(s)];
}

const char* toString(Expression e) {
  static const char* names[] = {"neutral", "happy", "sad", "angry", "surprised", "sleepy"};
  return names[static_cast<int>(e)];
}

bool Mascot::begin(TFT_eSPI& tft) {
  tft_ = &tft;
  spr_ = new TFT_eSprite(&tft);
  spr_->setColorDepth(16);
  if (spr_->createSprite(kBandW, kBandH) == nullptr) {
    Serial.printf("[mascot] sprite %dx%d failed, free heap %lu\n", kBandW, kBandH,
                  static_cast<unsigned long>(ESP.getFreeHeap()));
    return false;
  }
  for (int i = 0; i < 2; ++i) {
    ear_spr_[i] = new TFT_eSprite(&tft);
    ear_spr_[i]->setColorDepth(16);
    if (ear_spr_[i]->createSprite(kEarW, kEarH) == nullptr) {
      Serial.println("[mascot] ear sprite failed");
      return false;
    }
  }
  const float* src = reinterpret_cast<const float*>(&kShapes[0]);
  for (int i = 0; i < kShapeFields; ++i) shape_[i].x = src[i];
  uint32_t now = millis();
  last_interact_ = now;
  next_look_ = now + 1000;
  next_tilt_ = now + 6000;
  next_blink_ = now + 2000;
  next_twitch_ = now + 4000;
  fps_window_start_ = now;
  tft.fillScreen(kBg);
  Serial.printf("[mascot] band sprite %dx%d 16-bit ok, free heap %lu\n", kBandW, kBandH,
                static_cast<unsigned long>(ESP.getFreeHeap()));
  return true;
}

void Mascot::setState(AssistantState s, uint32_t now) {
  state_ = s;
  last_interact_ = now;
  next_look_ = now;
  Serial.printf("[mascot] state=%s\n", toString(s));
}

void Mascot::setExpression(Expression e, uint32_t now) {
  expr_ = e;
  last_interact_ = now;
  Serial.printf("[mascot] expression=%s\n", toString(e));
}

bool Mascot::sleeping(uint32_t now) const {
  return state_ == AssistantState::Idle && expr_ == Expression::Neutral && now - last_interact_ > kSleepAfterMs;
}

bool Mascot::poke(int32_t x, int32_t y, uint32_t now) {
  last_interact_ = now;
  int hit = -1;
  for (int i = 0; i < 2; ++i) {
    if (abs(x - eye_cx_[i]) <= eye_hw_ + 20 && abs(y - eye_cy_) <= eye_hh_ + 20) hit = i;
  }
  wink_eye_ = hit >= 0 ? hit : static_cast<int>(random(2));
  blink_at_ = now;
  double_blink_ = false;
  bob_.v -= 260;
  ear_kick_.v += 120;
  Serial.printf("[mascot] poke eye=%d\n", hit);
  return hit >= 0;
}

float Mascot::blinkOpen(uint32_t now) {
  if (blink_at_ == 0 || now < blink_at_) return 1;
  float d = (now - blink_at_) / 1000.0f;
  if (d < 0.07f) return 1 - d / 0.07f;
  if (d < 0.2f) return 1 - powf(1 - (d - 0.07f) / 0.13f, 3);
  if (double_blink_) {
    double_blink_ = false;
    blink_at_ = now + 90;
  } else {
    blink_at_ = 0;
    wink_eye_ = -1;
  }
  return 1;
}

Mascot::Box Mascot::frameBox(bool asleep) const {
  // Chỉ đẩy vùng có hình để giữ tốc độ khung hình.
  Box b{80, 0, 400, 280};
  if (state_ == AssistantState::Listening) { b.x0 = 16; b.x1 = 464; }
  if (state_ == AssistantState::Thinking) b.x1 = 440;
  if (state_ == AssistantState::Notifying) { b.x0 = 52; b.x1 = 436; }
  if (asleep) b.x1 = 452;
  return b;
}

void Mascot::update(uint32_t now) {
  if (spr_ == nullptr || now - last_frame_ < kFrameMs) return;
  float dt = fminf((now - last_frame_) / 1000.0f, 1.0f / 30);
  last_frame_ = now;
  t_ = now / 1000.0f;
  asleep_ = sleeping(now);

  Expression target = expr_;
  if (state_ == AssistantState::Notifying) target = Expression::Happy;
  if (asleep_) target = Expression::Sleepy;

  // Liếc mắt theo trạng thái.
  if (now >= next_look_) {
    if (state_ == AssistantState::Thinking) { target_gx_ = frand(0.55f, 0.85f); target_gy_ = frand(-0.85f, -0.55f); }
    else if (state_ == AssistantState::Listening) { target_gx_ = frand(-0.15f, 0.15f); target_gy_ = -0.1f; }
    else if (asleep_) { target_gx_ = 0; target_gy_ = 0.3f; }
    else {
      target_gx_ = frand(-0.9f, 0.9f);
      target_gy_ = frand(-0.6f, 0.6f);
      if (random(10) < 3) { target_gx_ = 0; target_gy_ = 0; }
    }
    if (!asleep_ && fabsf(target_gx_ - gx_.x) > 0.8f && blink_at_ == 0) blink_at_ = now;  // Liếc xa thì chớp.
    next_look_ = now + 900 + random(2400);
  }
  // Nghiêng đầu tò mò thỉnh thoảng.
  if (now >= next_tilt_) {
    target_tilt_ = target_tilt_ != 0 ? 0 : (random(2) ? 7.0f : -7.0f);
    next_tilt_ = now + (target_tilt_ != 0 ? 1600 : 6000 + random(6000));
  }
  if (blink_at_ == 0 && now >= next_blink_ && !asleep_) {
    blink_at_ = now;
    double_blink_ = random(10) < 2;
    next_blink_ = now + 2600 + random(2600);
  }
  if (!asleep_ && state_ == AssistantState::Idle && now >= next_twitch_) {
    ear_kick_.v += 160;
    next_twitch_ = now + 4000 + random(6000);
  }

  // Lò xo: thông số biểu cảm k170 c22; liếc k320 c30; nghiêng k60 c11; nảy k160 c9; tai k260 c9.
  const float* tgt = reinterpret_cast<const float*>(&kShapes[static_cast<int>(target)]);
  float* cur = reinterpret_cast<float*>(&cur_);
  for (int i = 0; i < kShapeFields; ++i) {
    shape_[i].step(tgt[i], 170, 22, dt);
    cur[i] = shape_[i].x;
  }
  gx_.step(target_gx_, 320, 30, dt);
  gy_.step(target_gy_, 320, 30, dt);
  tilt_.step(asleep_ ? 0 : target_tilt_, 60, 11, dt);
  bob_.step(0, 160, 9, dt);
  ear_kick_.step(state_ == AssistantState::Listening ? -5 : 0, 260, 9, dt);

  open_ = blinkOpen(now);
  breath_ = sinf(t_ * 2 * PI / (asleep_ ? 6.0f : 4.0f)) * (asleep_ ? 4 : 2);
  if (state_ == AssistantState::Notifying) breath_ -= fabsf(sinf(t_ * 5)) * 8;
  talk_ = state_ == AssistantState::Speaking ? fabsf(sinf(t_ * 9) * 0.6f + sinf(t_ * 23) * 0.4f) : 0;
  switch (state_) {
    case AssistantState::Listening: color_ = kListen; break;
    case AssistantState::Thinking: color_ = kThink; break;
    case AssistantState::Notifying: color_ = kNotify; break;
    default: color_ = kWarm;
  }
  if (asleep_) color_ = kSleep;

  // Vẽ theo dải: xóa vùng cũ không còn thuộc khung mới, rồi vẽ và đẩy từng dải.
  uint32_t t0 = micros();
  renderEars(cur_.ear + ear_kick_.x, cur_.earSq, color_);
  Box b = frameBox(asleep_);
  if (prev_box_.x1 > 0) {
    if (prev_box_.x0 < b.x0) tft_->fillRect(prev_box_.x0, prev_box_.y0, b.x0 - prev_box_.x0, prev_box_.y1 - prev_box_.y0, kBg);
    if (prev_box_.x1 > b.x1) tft_->fillRect(b.x1, prev_box_.y0, prev_box_.x1 - b.x1, prev_box_.y1 - prev_box_.y0, kBg);
  }
  prev_box_ = b;
  int w = b.x1 - b.x0;
  for (int y = b.y0; y < b.y1; y += kBandH) {
    int h = min(kBandH, b.y1 - y);
    spr_->fillSprite(kBg);
    drawFace(b.x0, y, now);
    uint32_t tp = micros();
    spr_->pushSprite(b.x0, y, 0, 0, w, h);
    push_us_ += micros() - tp;
  }
  draw_us_ += micros() - t0;

  ++frames_;
  if (now - fps_window_start_ >= 5000) {
    Serial.printf("[mascot] fps=%.1f frame=%lums (push %lums) state=%s expr=%s asleep=%d heap=%lu\n",
                  frames_ * 1000.0f / (now - fps_window_start_), static_cast<unsigned long>(draw_us_ / frames_ / 1000),
                  static_cast<unsigned long>(push_us_ / frames_ / 1000), toString(state_), toString(expr_), asleep_,
                  static_cast<unsigned long>(ESP.getFreeHeap()));
    frames_ = 0;
    draw_us_ = 0;
    push_us_ = 0;
    fps_window_start_ = now;
  }
}

void Mascot::renderEars(float angle, float squash, uint16_t color) {
  // Chỉ vẽ lại khi góc đổi ≥ 0,25°, độ ép đổi ≥ 0,005 hoặc đổi màu.
  int ka = (int)lroundf(angle * 4), ks = (int)lroundf(squash * 200);
  if (ka == ear_key_angle_ && ks == ear_key_squash_ && color == ear_key_color_) return;
  ear_key_angle_ = ka;
  ear_key_squash_ = ks;
  ear_key_color_ = color;
  uint16_t inner = mix(color, kBg, 0.45f);
  for (int i = 0; i < 2; ++i) {
    int side = i == 0 ? -1 : 1;
    TFT_eSprite* s = ear_spr_[i];
    s->fillSprite(kBg);  // Nền đen, dán thẳng (tai vẽ trước mắt nên không che gì).
    // Tọa độ trong ảnh: trục xoay ở (kEarOX, kEarOY).
    Canvas cv{s, (float)(-kEarOX), (float)(-kEarOY), (float)kEarH};
    float px = side < 0 ? kEarPivot[0] : 2 * CX - kEarPivot[0], py = kEarPivot[1];
    float a = (side < 0 ? -angle : angle) * PI / 180, ca = cosf(a), sa = sinf(a);
    float e[3][2];
    for (int k = 0; k < 3; ++k) {
      float x = (side < 0 ? kEar[k][0] : 2 * CX - kEar[k][0]) - px, y = (kEar[k][1] - py) * squash;
      e[k][0] = x * ca - y * sa;
      e[k][1] = x * sa + y * ca;
    }
    cv.roundedTri(e, 12, color);
    float c0 = (e[0][0] + e[1][0] + e[2][0]) / 3, c1 = (e[0][1] + e[1][1] + e[2][1]) / 3 + 10;
    float in[3][2];
    for (int k = 0; k < 3; ++k) {
      in[k][0] = c0 + (e[k][0] - c0) * 0.55f;
      in[k][1] = c1 + (e[k][1] - c1) * 0.55f;
    }
    cv.roundedTri(in, 7, inner);
  }
}

void Mascot::drawFace(int ox, int oy, uint32_t now) {
  Canvas cv{spr_, (float)ox, (float)oy};
  const FaceShape& p = cur_;
  const uint16_t col = color_;
  const float gx = gx_.x * 22, gy = gy_.x * 14;
  const float bob = bob_.x + breath_;
  const float cy = EYE_Y + p.dy + bob;

  // ---- Tai: dán ảnh tai đã vẽ sẵn (renderEars), dịch theo hướng nhìn và nhấp nhô ----
  {
    float dx = gx * 0.35f, dy = p.dy + bob + gy * 0.3f;
    for (int i = 0; i < 2; ++i) {
      float px = (i == 0 ? kEarPivot[0] : 2 * CX - kEarPivot[0]) + dx, py = kEarPivot[1] + dy;
      float top = py - kEarOY;
      if (cv.vis(top, top + kEarH)) ear_spr_[i]->pushToSprite(spr_, cv.X(px - kEarOX), cv.Y(top));
    }
  }

  // ---- Má hồng (khi vui) ----
  if (p.blush > 0.02f) {
    uint16_t bc = mix(kBg, kBlush, 0.55f * p.blush);
    cv.ellipse(CX - EYE_DX - 18 + gx * 0.6f, cy + 64 + gy * 0.5f, 20, 9, bc);
    cv.ellipse(CX + EYE_DX + 18 + gx * 0.6f, cy + 64 + gy * 0.5f, 20, 9, bc);
  }

  // ---- Mắt ----
  for (int i = 0; i < 2; ++i) {
    int side = i == 0 ? -1 : 1;
    float scale = 1 + side * 0.07f * gx_.x;  // Liếc bên nào thì mắt bên đó to hơn chút.
    float open = (wink_eye_ >= 0 && wink_eye_ != i) ? 1 : open_;
    float ex = CX + side * EYE_DX + gx, ey = cy + gy + side * tilt_.x;
    float w = p.w * scale * (1 + 0.14f * (1 - open));
    float h = fmaxf(7, p.h * scale * open);
    float r = fminf(p.r * scale, fminf(h, w) / 2);
    float x0 = ex - w / 2, y0 = ey - h / 2;
    cv.smoothRect(x0, y0, w, h, r, col, kBg);
    eye_cx_[i] = (int)ex;
    eye_cy_ = (int)ey;
    eye_hw_ = (int)(w / 2);
    eye_hh_ = (int)(h / 2);

    // Con ngươi khe dọc (viên thuốc màu nền), dịch theo hướng nhìn; ẩn khi mắt cười.
    if (open > 0.3f && p.cut < 0.2f) {
      float rx = fmaxf(3.5f, w * p.pw / 2), ry = h * 0.3f;
      float pcx = ex + gx_.x * w * 0.14f, pcy = ey + h * 0.07f + gy_.x * h * 0.08f;
      cv.smoothRect(pcx - rx, pcy - ry, 2 * rx, 2 * ry, rx, kBg, col);
    }
    // Đốm sáng: ẩn khi mắt cười hoặc mí che quá nửa.
    float hi = fmaxf(0, fminf(1, (open - 0.55f) / 0.3f)) * fmaxf(0, 1 - p.cut * 5) * (p.lid > 0.45f ? 0 : 1);
    if (hi > 0.02f) {
      cv.circle(ex + w * 0.2f, y0 + h * 0.27f, w * 0.11f, mix(col, kWhite, 0.95f * hi), col);
      cv.circle(ex - w * 0.18f, y0 + h * 0.7f, w * 0.05f, mix(col, kWhite, 0.7f * hi), col);
    }
    if (open > 0.35f) {
      if (p.lid > 0.01f) {  // Mí trên: hình thang màu nền, nghiêng theo slope.
        float base = y0 + h * p.lid, tl = h * 0.32f * p.slope;
        float xo = ex + side * (w / 2 + 3), xi = ex - side * (w / 2 + 3);
        cv.tri(xi, y0 - 3, xo, y0 - 3, xo, base + tl, kBg);
        cv.tri(xi, y0 - 3, xo, base + tl, xi, base - tl, kBg);
      }
      if (p.cut > 0.01f) {  // Mắt cười: elip màu nền khoét từ dưới lên.
        float ry = h * 0.62f, top = y0 + h * (1 - 1.6f * p.cut);
        cv.ellipse(ex, top + ry, w * 0.98f, ry, kBg);
      }
    }
  }

  // ---- Mũi + miệng ----
  {
    float mx = CX + gx * 0.5f, my = MOUTH_Y + p.dy + bob + gy * 0.4f, ny = my - 22;
    const float nose[3][2] = {{mx - 10, ny - 6}, {mx + 10, ny - 6}, {mx, ny + 5}};
    cv.roundedTri(nose, 4, col);
    float open = fmaxf(talk_, p.mw < 26 ? p.mo : 0);
    if (open > 0.05f && p.mw < 26) {  // Chữ O, không có nhân trung.
      float rx = 7 + open * 4, ry = 6 + open * 8;
      cv.smoothRect(mx - rx, my + 6 - ry, 2 * rx, 2 * ry, rx, col, kBg);
    } else if (open > 0.05f) {  // Miệng mở khi nói.
      cv.line(mx, ny + 5, mx, my - 8, 4, col);
      float h = (6 + open * 16) / 0.75f;
      cv.smoothRect(mx - 16, my - 6, 32, h, fminf(16, h / 2), col, kBg);
    } else {  // Chữ ω: hai cung, cười thì cong sâu, buồn thì lật ngược.
      float c = 3 + p.mc * 0.6f, j = my - 8, wing = 9 + fmaxf(0, p.mc) * 0.35f;
      cv.line(mx, ny + 5, mx, j, 4, col);
      cv.quad(mx - wing * 2, j + 1, mx - wing, j + 1 + c * 2, mx, j, 5, col);
      cv.quad(mx, j, mx + wing, j + 1 + c * 2, mx + wing * 2, j + 1, 5, col);
    }
  }

  // ---- Chi tiết theo trạng thái ----
  const float t = t_;
  if (asleep_) {  // Chữ z bay lên.
    spr_->setTextDatum(MC_DATUM);
    for (int k = 0; k < 3; ++k) {
      float ph = fmodf(t * 0.35f + k / 3.0f, 1.0f), a = sinf(ph * PI);
      spr_->setTextColor(mix(kBg, rgb(0x9A, 0xA4, 0xB2), a));
      spr_->drawString("z", cv.X(400 + ph * 34), cv.Y(112 - ph * 84), ph < 0.5f ? 2 : 4);
    }
    return;
  }
  if (state_ == AssistantState::Listening) {  // Cột sóng âm hai bên.
    for (int k = 0; k < 3; ++k) {
      float h = 14 + 34 * fabsf(sinf(t * 6.5f + k * 1.1f)) * (0.6f + 0.4f * sinf(t * 1.7f + k));
      uint16_t c = mix(kBg, col, 0.9f - k * 0.22f);
      cv.smoothRect(46 - k * 13 - 3.5f, cy - h / 2, 7, h, 3.5f, c, kBg);
      cv.smoothRect(434 + k * 13 - 3.5f, cy - h / 2, 7, h, 3.5f, c, kBg);
    }
  }
  if (state_ == AssistantState::Thinking) {  // Ba chấm lần lượt phồng lên.
    for (int k = 0; k < 3; ++k) {
      float ph = fmaxf(0, sinf(t * 4.2f - k * 0.8f));
      cv.circle(388 + k * 20, 46 - ph * 6, 5 + ph * 2.5f, mix(kBg, col, 0.45f + 0.55f * ph), kBg);
    }
  }
  if (state_ == AssistantState::Notifying) {  // Lấp lánh bốn góc: sao bốn cánh.
    static const float pts[4][2] = {{64, 52}, {420, 60}, {72, 250}, {412, 244}};
    for (int k = 0; k < 4; ++k) {
      float v = fmaxf(0, sinf(t * 3.2f + k * 1.6f)), r = 4 + v * 9;
      if (v < 0.05f) continue;
      uint16_t c = mix(kBg, col, v);
      float x = pts[k][0], y = pts[k][1], q = r * 0.28f;
      cv.tri(x, y - r, x + q, y, x - q, y, c);
      cv.tri(x, y + r, x + q, y, x - q, y, c);
      cv.tri(x - r, y, x, y - q, x, y + q, c);
      cv.tri(x + r, y, x, y - q, x, y + q, c);
    }
  }
}
