#include "mascot.hpp"

#include <Arduino.h>

#include <cmath>
#include <cstddef>

namespace {

// ---- Thông số thiết kế: phải khớp docs/design/mascot/prototype.html ----
// Cả mặt dời xuống 30px so với bản đầu (người dùng 2026-11-16: hình cũ y 4..250 nằm quá cao trên màn 320).
constexpr float CX = 240, EYE_DX = 78, EYE_Y = 170, MOUTH_Y = 266;

// Bảng biểu cảm (thứ tự = enum Expression). Dựng lúc khởi động từ bộ gốc + phần khác biệt, như bản mẫu.
FaceShape kShapes[static_cast<int>(Expression::kCount)];

void initShapes() {
  FaceShape b{};
  b.w = 86; b.h = 104; b.r = 30; b.sL = b.sR = 1; b.pw = 0.14f; b.ph = 0.3f; b.hi = 1;
  b.mw = 1; b.mm = 1; b.earSq = 1;
  for (auto& s : kShapes) s = b;
  auto at = [](Expression e) -> FaceShape& { return kShapes[static_cast<int>(e)]; };
  { auto& s = at(Expression::Happy); s.mm = 1.25f; s.sm = 1; s.dy = -6; s.ear = -6; s.earSq = 1.06f; s.bnc = 0.5f; s.jump = 1; }
  { auto& s = at(Expression::Laugh); s.fL = s.fR = 1; s.mm = 1.2f; s.sm = 1; s.dy = -4; s.ear = 4; s.earSq = 0.9f; s.chk = 1; s.tiltB = -4; s.jump = 1; }
  { auto& s = at(Expression::Excited); s.tg = 0.8f; s.mm = 1.3f; s.w = 92; s.h = 112; s.r = 36; s.pw = 0.17f; s.ph = 0.32f; s.hi = 1.6f;
    s.ear = -10; s.earSq = 1.15f; s.dy = -4; s.bnc = 1.2f; s.jump = 1; }
  { auto& s = at(Expression::Love); s.hrt = 1; s.mm = 1.3f; s.ear = 3; s.earSq = 0.96f; s.sway = 0.8f; s.tiltB = 6; s.dy = -2; }
  { auto& s = at(Expression::Shy); s.mw = 0.75f; s.uL = s.uR = 0.26f; s.uB = -0.25f; s.ear = 8; s.earSq = 0.88f; s.sway = 0.4f;
    s.gx = -0.6f; s.gy = 0.5f; s.gh = 0.9f; }
  { auto& s = at(Expression::Smug); s.tg = 1; s.uL = s.uR = 0.4f; s.uA = 0.15f; s.ear = -3; s.gx = 0.3f; s.gh = 0.5f; }
  { auto& s = at(Expression::Sad); s.mm = -0.6f; s.mw = 0.9f; s.h = 98; s.uL = s.uR = 0.2f; s.uA = -0.75f; s.uB = 0.15f; s.dL = s.dR = 0.06f;
    s.pw = 0.22f; s.ph = 0.32f; s.hi = 1.2f; s.ear = 8; s.earSq = 0.64f; s.dy = 8; s.gy = 0.5f; s.gh = 0.6f; }
  { auto& s = at(Expression::Cry); s.mm = -0.7f; s.mv = 0.6f; s.h = 96; s.uL = s.uR = 0.22f; s.uA = -0.8f; s.uB = 0.15f; s.dL = s.dR = 0.16f;
    s.dB = -0.2f; s.pw = 0.24f; s.ph = 0.34f; s.hi = 1.6f; s.ear = 9; s.earSq = 0.6f; s.dy = 8; s.trem = 0.5f; s.tear = 1; }
  { auto& s = at(Expression::Angry); s.fR = 1; s.h = 98; s.uL = s.uR = 0.3f; s.uA = 0.8f; s.uB = -0.1f; s.dL = s.dR = 0.1f; s.pw = 0.08f;
    s.ear = 6; s.earSq = 0.72f; s.tint = 0.7f; s.trem = 0.25f; }
  { auto& s = at(Expression::Surprised); s.mo = 1; s.w = 98; s.h = 114; s.r = 49; s.pw = 0.1f; s.ph = 0.14f; s.ear = -8; s.earSq = 1.1f;
    s.dy = -4; s.jump = 1; }
  { auto& s = at(Expression::Scared); s.mm = 0.85f; s.mw = 0.9f; s.mv = 1.6f; s.w = 92; s.h = 110; s.r = 42; s.uL = s.uR = 0.04f; s.uA = -0.5f;
    s.pw = 0.09f; s.ph = 0.1f; s.ear = 9; s.earSq = 0.6f; s.trem = 1; }
  { auto& s = at(Expression::Confused); s.ma = 2; s.sL = 0.9f; s.sR = 1.06f; s.uL = 0.3f; s.dL = 0.24f; s.uB = -0.7f; s.dB = -0.7f; s.tiltB = 9;
    s.gx = 0.35f; s.gy = -0.35f; s.gh = 0.7f; }
  { auto& s = at(Expression::Bored); s.mm = 0.6f; s.mw = 0.8f; s.uL = s.uR = 0.5f; s.dL = s.dR = 0.08f; s.pw = 0.12f; s.ear = 4; s.earSq = 0.9f;
    s.gy = 0.15f; s.gh = 0.8f; }
  { auto& s = at(Expression::Sleepy); s.mw = 0.7f; s.mm = 0.55f; s.uL = s.uR = 0.66f; s.uA = -0.2f; s.uB = 0.1f; s.dL = s.dR = 0.12f;
    s.pw = 0.12f; s.hi = 0.6f; s.gy = 0.6f; s.gh = 1; s.ear = 6; s.earSq = 0.78f; s.dy = 6; s.doze = 1; }
}

// Trái tim trơn (mắt yêu): 2 hình tròn bán kính R tại (±D, C) + đa giác nối mũi tim (0, B) với ĐIỂM TIẾP TUYẾN
// phía ngoài của mỗi hình tròn; mũi bo bằng đường cong bậc hai qua 2 điểm cách mũi 8 đơn vị.
// Đã thử: tam giác tùy ý làm sườn gãy lõm; nét viền phình gờ ở tiếp điểm (2026-11-16).
struct HeartGeo {
  static constexpr float R = 24, D = 21, C = -12, B = 44;
  float tip[2][2], nr[2][2];  // [0] phải, [1] trái: tiếp điểm, điểm gần mũi.
  HeartGeo() {
    for (int i = 0; i < 2; ++i) {
      float sd = i == 0 ? 1 : -1, vx = -sd * D, vy = B - C, dist = hypotf(vx, vy);
      float th = atan2f(vy, vx), al = acosf(R / dist);
      float ax = sd * D + R * cosf(th + al), ay = C + R * sinf(th + al);
      float bx = sd * D + R * cosf(th - al), by = C + R * sinf(th - al);
      bool first = sd * ax > sd * bx;
      tip[i][0] = first ? ax : bx;
      tip[i][1] = first ? ay : by;
      float dx = tip[i][0], dy = tip[i][1] - B, l = hypotf(dx, dy);
      nr[i][0] = dx / l * 8;
      nr[i][1] = B + dy / l * 8;
    }
  }
};
const HeartGeo kHeart;

// Tai trái: chân ngoài, chân trong, chóp; trục xoay. Tai phải đối xứng qua x = CX.
constexpr float kEar[3][2] = {{94, 112}, {192, 92}, {124, 34}};
constexpr float kEarPivot[2] = {143, 102};

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
constexpr uint16_t kBg = 0x0000;
constexpr uint16_t kWarm = rgb(0xFF, 0x9A, 0x3C);       // Rảnh, đang nói (cáo).
constexpr uint16_t kListen = rgb(0x6E, 0xF2, 0xA6);
constexpr uint16_t kThink = rgb(0xB7, 0x9B, 0xFF);
constexpr uint16_t kNotify = rgb(0xFF, 0xD2, 0x5E);
constexpr uint16_t kSleep = rgb(0x5B, 0x6B, 0x7A);
constexpr uint16_t kAngry = rgb(0xFF, 0x6A, 0x3A);  // Ám khi giận.
constexpr uint16_t kWhite = 0xFFFF;

constexpr uint32_t kFrameMs = 33;          // Tối đa ~30 khung/giây.
constexpr uint32_t kSleepAfterMs = 30000;  // Rảnh, biểu cảm bình thường, để yên 30 giây thì ngủ.
constexpr int kBandH = 56;                 // Vẽ theo dải 56 dòng.
constexpr int kBandW = 448;                // Bề rộng tối đa vùng vẽ.
// Ảnh tai: trục xoay đặt tại (kEarOX, kEarOY) trong ảnh; đủ chỗ cho tai nghiêng ±10° và ép 0.7–1.1.
constexpr int kEarW = 150, kEarH = 124, kEarOX = 75, kEarOY = 100;
// Ảnh cụm mũi + miệng (khi không nói) và ảnh vòm mắt cười: vẽ lại chỉ khi thông số đổi, mỗi khung chỉ dán.
// Đo 2026-11-16: vẽ trực tiếp ω 24 đoạn tốn ~9ms/khung, vòm nét dày 26px tốn ~37ms/khung (13 fps).
constexpr int kMouthW = 96, kMouthH = 60, kMouthOX = 48, kMouthOY = 36;  // Gốc = (mx, my) của miệng.
// Ảnh vòm dán ĐẶC (dán trong suốt từng pixel quá chậm): khung ôm sát vòm (đỉnh ey-35, đáy ey+25) để nền đen
// của ảnh không đè lên chân tai kể cả khi đầu nghiêng.
constexpr int kArcW = 112, kArcH = 68, kArcOX = 56, kArcOY = 40;         // Gốc = tâm mắt (ex, ey).
// Chấm báo đẩy riêng ở góc phải trên, NGOÀI vùng mặt (x ≥ 400): nới vùng mặt ra tới 470 làm mỗi khung đẩy thêm
// ~22% điểm ảnh, thông báo tụt còn ~21 fps (đo 2026-11-16).
constexpr int kBadgeX = 400, kBadgeY = 30, kBadgeW = 76, kBadgeH = 84;  // Dừng ở x 476: không đè viền phải.

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
  void circle(float x, float y, float r, uint16_t c, uint32_t bg) {
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
  // Tam giác bo góc bán kính r (tổng Minkowski): tam giác co vào r quanh tâm nội tiếp, nới ra bằng 3 dải cạnh
  // tô đặc + 3 hình tròn ở góc; mép ngoài làm mịn bằng nét mảnh. (Bản cũ dùng nét dày 2r làm mịn cạnh: 2 tai tốn
  // ~36ms mỗi lần vẽ lại, mà khi chuyển biểu cảm tai vẽ lại mỗi khung nên bị khựng, đo 2026-11-16.)
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
    for (int i = 0; i < 3; ++i) {
      const float* a0 = q[i];
      const float* b0 = q[(i + 1) % 3];
      float ex = b0[0] - a0[0], ey = b0[1] - a0[1], len = hypotf(ex, ey);
      if (len < 0.01f) continue;
      float nx = ey / len, ny = -ex / len;
      if (nx * (a0[0] - ix) + ny * (a0[1] - iy) < 0) { nx = -nx; ny = -ny; }  // Pháp tuyến hướng ra ngoài.
      float oax = a0[0] + nx * r, oay = a0[1] + ny * r, obx = b0[0] + nx * r, oby = b0[1] + ny * r;
      tri(a0[0], a0[1], b0[0], b0[1], obx, oby, c);
      tri(a0[0], a0[1], obx, oby, oax, oay, c);
      line(oax, oay, obx, oby, 1.5f, c);
    }
    for (int i = 0; i < 3; ++i) circle(q[i][0], q[i][1], r, c, kReadBg);
  }
  // Nét dày đầu tròn dọc đường cong bậc hai, dựng bằng chuỗi hình tròn chồng nhau (cách nhau ≤ ~1/3 bán kính
  // nên mép gần như trơn). Rẻ hơn nhiều so với drawWideLine bề rộng lớn (vòm mắt cười 26px: ~25ms/lần vẽ).
  void thickQuad(float x0, float y0, float cx, float cy, float x1, float y1, float wd, uint16_t c) {
    float r = wd / 2, len = hypotf(cx - x0, cy - y0) + hypotf(x1 - cx, y1 - cy);
    int n = (int)ceilf(len / fmaxf(1.5f, r * 0.35f));
    if (n < 2) n = 2;
    for (int i = 0; i <= n; ++i) {
      float t = (float)i / n, u = 1 - t;
      circle(u * u * x0 + 2 * u * t * cx + t * t * x1, u * u * y0 + 2 * u * t * cy + t * t * y1, r, c, kReadBg);
    }
  }
  // Vùng màu c phía TRÊN một đường cong bậc hai (mí trên), từ dòng top xuống tới đường cong.
  // Điểm điều khiển nằm giữa hai đầu theo x nên x chạy tuyến tính theo t: tô từng cột, rồi một nét mảnh
  // cùng màu dọc mép để làm mịn cạnh.
  void fillAbove(float xa, float ya, float cy, float xb, float yb, float top, uint16_t c) { curveFill(xa, ya, cy, xb, yb, top, true, c); }
  // Vùng màu c phía DƯỚI đường cong (mí dưới), từ đường cong xuống tới dòng bottom.
  void fillBelow(float xa, float ya, float cy, float xb, float yb, float bottom, uint16_t c) { curveFill(xa, ya, cy, xb, yb, bottom, false, c); }
  void curveFill(float xa, float ya, float cy, float xb, float yb, float lim, bool above, uint16_t c) {
    float ylo = fminf(lim, fminf(ya, fminf(yb, cy))), yhi = fmaxf(lim, fmaxf(ya, fmaxf(yb, cy)));
    if (!vis(ylo, yhi)) return;
    float xl = fminf(xa, xb), xr = fmaxf(xa, xb);
    for (int x = (int)floorf(xl); x <= (int)ceilf(xr); ++x) {
      float t = (x - xa) / (xb - xa);
      t = t < 0 ? 0 : (t > 1 ? 1 : t);
      float u = 1 - t, y = u * u * ya + 2 * u * t * cy + t * t * yb;
      int y0 = above ? Y(lim) : Y(y), y1 = above ? Y(y) : Y(lim);
      if (y1 > y0) s->drawFastVLine(X(x), y0, y1 - y0, c);
    }
    quad(xa, ya, (xa + xb) / 2, cy, xb, yb, 1.6f, c);
  }
  // Trái tim trơn tâm (x, y), tỉ lệ ngang kx, dọc ky (kích thước gốc ~90x80), xem HeartGeo.
  void heart(float x, float y, float kx, float ky, uint16_t c) {
    const HeartGeo& g = kHeart;
    if (!vis(y + (g.C - g.R) * ky, y + g.B * ky)) return;
    for (int sd = -1; sd <= 1; sd += 2) {
      float lx = x + sd * g.D * kx, ly = y + g.C * ky;
      if (fabsf(kx - ky) < 0.03f * kx) circle(lx, ly, g.R * kx, c, kReadBg);
      else ellipse(lx, ly, g.R * kx, g.R * ky, c);
    }
    // Đa giác lồi: điểm gần mũi phải, tiếp điểm phải, tâm thùy phải, tâm thùy trái, tiếp điểm trái, điểm gần
    // mũi trái, rồi cung mũi tim (bậc hai qua mũi) quay về điểm gần mũi phải. Tô hình quạt từ một điểm trong.
    float p[11][2];
    int n = 0;
    auto add = [&](float u, float v) { p[n][0] = x + u * kx; p[n][1] = y + v * ky; ++n; };
    add(g.nr[0][0], g.nr[0][1]);
    add(g.tip[0][0], g.tip[0][1]);
    add(g.D, g.C);
    add(-g.D, g.C);
    add(g.tip[1][0], g.tip[1][1]);
    add(g.nr[1][0], g.nr[1][1]);
    for (int i = 1; i <= 4; ++i) {  // Cung mũi tim từ điểm gần mũi trái về điểm gần mũi phải.
      float t = i / 5.0f, u = 1 - t;
      add(u * u * g.nr[1][0] + t * t * g.nr[0][0], u * u * g.nr[1][1] + 2 * u * t * g.B + t * t * g.nr[0][1]);
    }
    float fx = x, fy = y + (g.C + g.B) / 2 * ky;
    for (int i = 0; i < n; ++i) tri(fx, fy, p[i][0], p[i][1], p[(i + 1) % n][0], p[(i + 1) % n][1], c);
    // Làm mịn hai sườn và mũi (fillTriangle không làm mịn cạnh).
    line(p[1][0], p[1][1], p[0][0], p[0][1], 1.5f, c);
    line(p[4][0], p[4][1], p[5][0], p[5][1], 1.5f, c);
    for (int i = 5; i < n; ++i) line(p[i][0], p[i][1], p[(i + 1) % n][0], p[(i + 1) % n][1], 1.5f, c);
  }
};

// Băm các số đã lượng tử hóa (bước 1/50) thành khóa ảnh đệm.
uint32_t quantKey(std::initializer_list<float> v) {
  uint32_t h = 2166136261u;
  for (float x : v) {
    h ^= (uint32_t)(int32_t)lroundf(x * 50);
    h *= 16777619u;
  }
  return h;
}

// Cụm mũi + miệng thú khi KHÔNG nói: khung ω giữ nguyên, biến tấu bằng phụ kiện (prototype.html, mouth()).
void drawMouthShape(Canvas& cv, float mx, float my, const FaceShape& p, uint16_t col, float t) {
  const float ny = my - 22, j = my - 8;
  const float nose[3][2] = {{mx - 11, ny - 6}, {mx + 11, ny - 6}, {mx, ny + 6}};
  cv.roundedTri(nose, 4, col);
  const float mo = fmaxf(0, fminf(1, p.mo)), half = 24 * p.mw * fmaxf(0, 1 - mo * 2.5f);  // ω thu hết khi mo ≥ 0.4.
  if (mo > 0.02f) {  // Chữ o đặc, có nét từ mũi xuống.
    float k = fminf(1, mo * 1.6f), rx = (7 + 4 * mo) * k, ry = (6 + 8 * mo) * k;
    cv.line(mx, ny + 6, mx, my + 6 - ry + 2, 4, col);
    cv.smoothRect(mx - rx, my + 6 - ry, 2 * rx, 2 * ry, rx, col, kBg);
  }
  if (half < 4) return;
  constexpr int N = 24;
  float pts[N + 1][2];
  for (int i = 0; i <= N; ++i) {
    float u = i * 2.0f / N - 1, au = fabsf(u);
    // mm < 0 (buồn): hai nếp lật nhẹ lên như bản đầu (kéo khóe xuống thêm bị chê trễ quá, xấu).
    pts[i][0] = mx + u * half;
    pts[i][1] = j + au + p.mm * 5 * sinf(PI * au) - p.ma * u * 3 +
                p.mv * 1.6f * sinf(u * PI * 3 + t * 8);
  }
  cv.line(mx, ny + 6, mx, pts[N / 2][1], 4, col);
  if (p.tg > 0.02f) {  // Lưỡi lè: viên nhỏ sáng hơn màu chính một chút, có rãnh giữa.
    float tw = 13 * p.tg, th = 16 * p.tg, tx = mx + 6, ty = pts[N / 2 + 3][1] - 2;
    cv.smoothRect(tx - tw / 2, ty, tw, th, tw / 2, mix(col, kWhite, 0.35f), kBg);
    cv.line(tx, ty + 4, tx, ty + th - 5, 2, mix(col, kBg, 0.35f));
  }
  const float fang[2] = {p.fL, p.fR};
  for (int s = 0; s < 2; ++s) {  // Răng nanh nhỏ lòi ra dưới đáy nếp ω.
    float f = fang[s];
    if (f < 0.02f) continue;
    const float* q = pts[N / 2 + (s == 0 ? -1 : 1) * N / 4];
    const float tri[3][2] = {{q[0] - 5 * f, q[1] + 1}, {q[0] + 5 * f, q[1] + 1}, {q[0], q[1] + 1 + 10 * f}};
    cv.roundedTri(tri, 1.5f, mix(col, kWhite, 0.75f));
  }
  for (int i = 0; i < N; ++i) cv.line(pts[i][0], pts[i][1], pts[i + 1][0], pts[i + 1][1], 5, col);
}

}  // namespace

const char* toString(AssistantState s) {
  static const char* names[] = {"idle", "listening", "thinking", "speaking", "notifying"};
  return names[static_cast<int>(s)];
}

const char* toString(Expression e) {
  static const char* names[] = {"neutral", "happy", "laugh", "excited", "love", "shy", "smug", "sad",
                                "cry", "angry", "surprised", "scared", "confused", "bored", "sleepy"};
  return names[static_cast<int>(e)];
}

bool Mascot::begin(TFT_eSPI& tft) {
  tft_ = &tft;
  initShapes();
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
  mouth_spr_ = new TFT_eSprite(&tft);
  mouth_spr_->setColorDepth(16);
  bool ok = mouth_spr_->createSprite(kMouthW, kMouthH) != nullptr;
  for (int i = 0; i < 2; ++i) {
    arc_spr_[i] = new TFT_eSprite(&tft);
    arc_spr_[i]->setColorDepth(16);
    ok = ok && arc_spr_[i]->createSprite(kArcW, kArcH) != nullptr;
  }
  badge_spr_ = new TFT_eSprite(&tft);
  badge_spr_->setColorDepth(16);
  ok = ok && badge_spr_->createSprite(kBadgeW, kBadgeH) != nullptr;
  if (!ok) {
    Serial.println("[mascot] cache sprite failed");
    return false;
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
  Serial.printf("[mascot] band sprite %dx%d 16-bit ok, %d expressions, free heap %lu\n", kBandW, kBandH,
                static_cast<int>(Expression::kCount), static_cast<unsigned long>(ESP.getFreeHeap()));
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
  if (kShapes[static_cast<int>(e)].jump > 0) {  // Biểu cảm mạnh: bật lên khi vào.
    bob_.v -= 200;
    ear_kick_.v += 100;
  }
  Serial.printf("[mascot] expression=%s\n", toString(e));
}

bool Mascot::sleeping(uint32_t now) const {
  return state_ == AssistantState::Idle && expr_ == Expression::Neutral && now - last_interact_ > kSleepAfterMs;
}

void Mascot::tap(int32_t x, int32_t y, uint32_t now) {
  last_interact_ = now;
  for (int i = 0; i < 2; ++i) {  // Trúng mắt: nháy mắt đó.
    if (abs(x - eye_cx_[i]) <= eye_hw_ + 20 && abs(y - eye_cy_) <= eye_hh_ + 20) {
      wink_eye_ = i;
      blink_at_ = now;
      double_blink_ = false;
      bob_.v -= 200;
      ear_kick_.v += 120;
      Serial.printf("[mascot] tap eye=%d\n", i);
      return;
    }
  }
  if (abs(x - (int32_t)CX) < 45 && y > MOUTH_Y - 50 && y < MOUTH_Y + 30) {  // "Boop" vào mũi: híp mắt, nảy.
    shape_[offsetof(FaceShape, sm) / sizeof(float)].v += 14;
    bob_.v -= 240;
    ear_kick_.v += 160;
    Serial.println("[mascot] tap boop");
    return;
  }
  // Chỗ khác: đánh mắt nhìn về điểm chạm, liếc xa thì chớp; tai giật nhẹ.
  target_gx_ = fmaxf(-1, fminf(1, (x - CX) / 170.0f));
  target_gy_ = fmaxf(-1, fminf(1, (y - EYE_Y) / 120.0f));
  if (fabsf(target_gx_ - gx_.x) > 0.8f && blink_at_ == 0) blink_at_ = now;
  look_until_ = now + 1800;
  next_look_ = look_until_;
  ear_kick_.v += 110;
  bob_.v -= 80;
  Serial.printf("[mascot] tap look gx=%.2f gy=%.2f\n", target_gx_, target_gy_);
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
  Box b{80, 24, 400, 316};  // Chừa 4 dòng dưới cho viền thông báo (đè lên thì nháy đen).
  if (state_ == AssistantState::Listening) { b.x0 = 16; b.x1 = 464; }
  if (state_ == AssistantState::Thinking) b.x1 = 464;
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
  const bool alert = state_ == AssistantState::Notifying;
  if (alert) target = Expression::Excited;
  if (asleep_) target = Expression::Sleepy;
  FaceShape S = kShapes[static_cast<int>(target)];
  if (alert) {  // Thông báo: mắt to long lanh nhìn thẳng vào người dùng, không lè lưỡi, không nảy đều.
    S.tg = 0;
    S.bnc = 0;
    S.gx = S.gy = 0;
    S.gh = 1;
    if (now >= next_alert_) {  // Nhịp "gọi" mỗi 1.6 s: nhún hai nhịp liền + giật tai.
      bob_.v -= alert_step_ == 0 ? 230 : 170;
      ear_kick_.v += alert_step_ == 0 ? 300 : -260;  // Tai vẫy mạnh qua lại.
      next_alert_ = now + (alert_step_ == 0 ? 200 : 1400);
      alert_step_ ^= 1;
    }
  }

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
  // Nhịp nói: mỗi âm tiết 0.16–0.30 s một độ há ngẫu nhiên, thỉnh thoảng ngắt; lò xo k220 c26 làm mượt
  // (mô phỏng ở 30 fps: mép miệng đổi tối đa ~3 px/khung). Sau này thay bằng biên độ âm thanh thật.
  if (state_ == AssistantState::Speaking) {
    if (now >= next_syllable_) {
      bool pause = random(100) < 12;
      talk_target_ = pause ? 0.05f : frand(0.35f, 1.0f);
      next_syllable_ = now + (pause ? 250 + random(200) : 160 + random(140));
    }
  } else {
    talk_target_ = 0;
  }
  talk_s_.step(talk_target_, 220, 26, dt);

  // Lò xo: thông số biểu cảm k300 c30 (bản k170 c20 bị chê chuyển chậm, 2026-11-16; vẫn hơi nảy khi vào);
  // liếc k320 c30; nghiêng k60 c11; nảy k160 c9; tai k260 c9.
  const float* tgt = reinterpret_cast<const float*>(&S);
  float* cur = reinterpret_cast<float*>(&cur_);
  // Vòm mắt cười (sm) và mắt tim (hrt) đổi HÌNH khối mắt: khung giữa chừng trông dở, nên chuyển gần như tức thì
  // (~3 khung, người dùng yêu cầu 2026-11-16: bình thường -> vui, cười lớn -> hào hứng). Bám mũ, không vọt lố;
  // vẫn giữ vận tốc do "boop" đá vào sm.
  constexpr int kSm = offsetof(FaceShape, sm) / sizeof(float), kHrt = offsetof(FaceShape, hrt) / sizeof(float);
  const float snap = fminf(1, dt * 20);
  for (int i = 0; i < kShapeFields; ++i) {
    if (i == kSm || i == kHrt) {
      shape_[i].x += (tgt[i] - shape_[i].x) * snap + shape_[i].v * dt;
      shape_[i].v *= 1 - snap;
    } else {
      shape_[i].step(tgt[i], 300, 30, dt);
    }
    cur[i] = shape_[i].x;
  }
  // Hướng nhìn: pha giữa đảo mắt ngẫu nhiên và hướng ưu tiên của biểu cảm.
  const float gh = now < look_until_ ? 0 : cur_.gh;  // Đang nhìn điểm vừa chạm: bỏ hướng ưu tiên của biểu cảm.
  gx_.step(target_gx_ * (1 - gh) + S.gx * gh, 320, 30, dt);
  gy_.step(target_gy_ * (1 - gh) + S.gy * gh, 320, 30, dt);
  tilt_.step(asleep_ ? 0 : target_tilt_, 60, 11, dt);
  bob_.step(0, 160, 9, dt);
  ear_kick_.step(state_ == AssistantState::Listening ? -5 : 0, 260, 9, dt);

  // Chuyển động theo biểu cảm (biên độ là số đã qua lò xo nên vào/ra mượt).
  {
    const float t = t_;
    float b = -(0.5f - 0.5f * cosf(t * 9)) * 7 * cur_.bnc;
    float sw = sinf(t * 1.6f) * cur_.sway;
    float gate = fmaxf(0, sinf(t * 2.4f)), ck = -fabsf(sinf(t * 15)) * 5 * cur_.chk * gate;  // Cười "ha ha ha".
    move_x_ = sw * 8 + sinf(t * 23) * 1.6f * cur_.trem;  // Run ~3.7 Hz: nhanh hơn thì lấy mẫu 25 fps thành giật.
    move_y_ = b + ck + cur_.doze * 3 * sinf(t * 1.1f - 0.6f) + cosf(t * 19) * 1.1f * cur_.trem;  // Gật đầu khi gà gật.
    if (cur_.doze > 0.01f) {  // Gà gật: mí sụp thêm rồi cố mở lên, nhịp chậm ~5.7 s.
      float dz = cur_.doze * 0.14f * (0.5f - 0.5f * cosf(t * 1.1f));
      cur_.uL += dz;
      cur_.uR += dz;
    }
    move_tilt_ = sw * 4 + cur_.tiltB;
  }

  open_ = blinkOpen(now);
  breath_ = sinf(t_ * 2 * PI / (asleep_ ? 6.0f : 4.0f)) * (asleep_ ? 4 : 2);
  talk_ = state_ == AssistantState::Speaking ? fmaxf(0, talk_s_.x) : -1;
  switch (state_) {
    case AssistantState::Listening: color_ = kListen; break;
    case AssistantState::Thinking: color_ = kThink; break;
    case AssistantState::Notifying: color_ = kNotify; break;
    default: color_ = kWarm;
  }
  if (asleep_) color_ = kSleep;
  if (cur_.tint > 0.01f) color_ = mix(color_, kAngry, cur_.tint);

  // Vẽ theo dải: xóa vùng cũ không còn thuộc khung mới, rồi vẽ và đẩy từng dải.
  uint32_t t0 = micros();
  renderEars(cur_.ear + ear_kick_.x, cur_.earSq, color_);
  if (talk_ < 0) {
    const FaceShape& p = cur_;
    uint32_t key = quantKey({p.mw, p.mm, p.ma, p.mv, p.mo, p.tg, p.fL, p.fR, (float)color_,
                             p.mv > 0.02f ? fmodf(t_ * 8, 2 * PI) : 0.0f});
    if (key != mouth_key_) {
      mouth_key_ = key;
      mouth_spr_->fillSprite(kBg);
      Canvas mc{mouth_spr_, 0, 0, (float)kMouthH};
      drawMouthShape(mc, kMouthOX, kMouthOY, p, color_, t_);
    }
  }
  {
    const float sm = fmaxf(0, fminf(1, cur_.sm));
    for (int i = 0; i < 2; ++i) {
      const int side = i == 0 ? -1 : 1;
      const float sc = i == 0 ? cur_.sL : cur_.sR;  // Không theo phối cảnh liếc: đỡ vẽ lại mỗi lần đảo mắt.
      const float open = (wink_eye_ >= 0 && wink_eye_ != i) ? 1 : open_;
      if (sm <= 0.01f) continue;
      uint32_t key = quantKey({sm, open, sc, (float)color_});
      if (key == arc_key_[i]) continue;
      arc_key_[i] = key;
      arc_spr_[i]->fillSprite(kBg);
      Canvas ac{arc_spr_[i], 0, 0, (float)kArcH};
      const float ex = kArcOX, ey = kArcOY, a = cur_.w * sc * 0.4f, end = ey + 12;
      const float apex = ey + 12 - 34 * open * fminf(1, sm * 2);
      ac.thickQuad(ex - a, end, ex, 2 * apex - end, ex + a, end, 26 * sc, color_);
    }
  }
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
  // Viền màn hình sáng nhấp nháy khi thông báo (bản chấm nhỏ bị chê "chưa rõ ràng", 2026-11-16).
  if (state_ == AssistantState::Notifying && !asleep_) {
    float glow = 0.3f + 0.7f * (0.5f - 0.5f * cosf(t_ * 2 * PI / 0.8f));
    uint16_t ec = mix(kBg, color_, glow);
    tft_->fillRect(0, 0, 480, 4, ec);
    tft_->fillRect(0, 316, 480, 4, ec);
    tft_->fillRect(0, 4, 4, 312, ec);
    tft_->fillRect(476, 4, 4, 312, ec);
    // Chấm báo to có dấu "!" + hai vòng lan nối tiếp (như điện thoại), vẽ vào ảnh riêng rồi đẩy.
    badge_spr_->fillSprite(kBg);
    Canvas bc{badge_spr_, (float)kBadgeX, (float)kBadgeY, (float)kBadgeH};
    const float bx = 440, by = 72, pop = 1 + 0.12f * fmaxf(0, sinf(t_ * 2 * PI / 0.8f));
    for (int k = 0; k < 2; ++k) {  // Vòng dày 3px = 3 đường tròn mảnh làm mịn.
      float ph = fmodf(t_ / 1.6f + k * 0.5f, 1.0f), rr = 14 + ph * 22;
      uint16_t rc = mix(kBg, color_, 0.8f * (1 - ph));
      for (int d = 0; d < 3; ++d) badge_spr_->drawSmoothCircle(bc.X(bx), bc.Y(by), (int)rr - d, rc, kBg);
    }
    bc.circle(bx, by, 13 * pop, color_, kBg);
    bc.smoothRect(bx - 2.5f * pop, by - 8 * pop, 5 * pop, 11 * pop, 2.5f * pop, kBg, color_);
    bc.circle(bx, by + 6.5f * pop, 2.6f * pop, kBg, color_);
    badge_spr_->pushSprite(kBadgeX, kBadgeY);
    edge_lit_ = true;
  } else if (edge_lit_) {
    tft_->fillRect(0, 0, 480, 4, kBg);
    tft_->fillRect(0, 316, 480, 4, kBg);
    tft_->fillRect(0, 4, 4, 312, kBg);
    tft_->fillRect(476, 4, 4, 312, kBg);
    tft_->fillRect(kBadgeX, kBadgeY, kBadgeW, kBadgeH, kBg);
    edge_lit_ = false;
  }
  uint32_t frame_us = micros() - t0;
  draw_us_ += frame_us;
  if (frame_us > max_us_) max_us_ = frame_us;

  ++frames_;
  if (now - fps_window_start_ >= 5000) {
    Serial.printf("[mascot] fps=%.1f frame=%lums (push %lums) max=%lums state=%s expr=%s asleep=%d heap=%lu\n",
                  frames_ * 1000.0f / (now - fps_window_start_), static_cast<unsigned long>(draw_us_ / frames_ / 1000),
                  static_cast<unsigned long>(push_us_ / frames_ / 1000), static_cast<unsigned long>(max_us_ / 1000),
                  toString(state_), toString(expr_), asleep_,
                  static_cast<unsigned long>(ESP.getFreeHeap()));
    frames_ = 0;
    draw_us_ = 0;
    push_us_ = 0;
    max_us_ = 0;
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
  const float fx = move_x_, bob = bob_.x + breath_ + move_y_;
  const float tilt = tilt_.x + move_tilt_;
  const float cy = EYE_Y + p.dy + bob;
  const float t = t_;

  // ---- Tai: dán ảnh tai đã vẽ sẵn (renderEars), dịch theo hướng nhìn và nhấp nhô ----
  {
    float dx = gx * 0.35f + fx, dy = p.dy + bob + gy * 0.3f;
    for (int i = 0; i < 2; ++i) {
      float px = (i == 0 ? kEarPivot[0] : 2 * CX - kEarPivot[0]) + dx, py = kEarPivot[1] + dy;
      float top = py - kEarOY;
      if (cv.vis(top, top + kEarH)) ear_spr_[i]->pushToSprite(spr_, cv.X(px - kEarOX), cv.Y(top));
    }
  }

  // ---- Mũi + miệng thú: khung ω giữ nguyên, biến tấu bằng phụ kiện ----
  {
    float mx = CX + gx * 0.5f + fx, my = MOUTH_Y + p.dy + bob + gy * 0.4f, ny = my - 22;
    if (talk_ >= 0) {  // Đang nói: luôn là chữ D đặc như bản đầu, độ há theo nhịp âm tiết.
      const float nose[3][2] = {{mx - 11, ny - 6}, {mx + 11, ny - 6}, {mx, ny + 6}};
      cv.roundedTri(nose, 4, col);
      cv.line(mx, ny + 6, mx, my - 8, 4, col);
      float h = (3 + fminf(1.1f, talk_) * 17) / 0.75f;
      cv.smoothRect(mx - 16, my - 6, 32, h, fminf(16, h / 2), col, kBg);
    } else {  // Cụm mũi + miệng: ảnh đệm (renderMouth trong update), dán đặc, trước mắt.
      float top = my - kMouthOY;
      if (cv.vis(top, top + kMouthH)) mouth_spr_->pushToSprite(spr_, cv.X(mx - kMouthOX), cv.Y(top));
    }
  }

  // ---- Mắt ----
  const float sm = fmaxf(0, fminf(1, p.sm)), hrt = fmaxf(0, fminf(1, p.hrt)), gone = fmaxf(sm, hrt);
  for (int i = 0; i < 2; ++i) {
    const int side = i == 0 ? -1 : 1;
    const bool L = side < 0;
    const float persp = 1 + side * 0.07f * gx_.x;  // Liếc bên nào thì mắt bên đó to hơn chút.
    const float sc = persp * (L ? p.sL : p.sR);
    const float open = (wink_eye_ >= 0 && wink_eye_ != i) ? 1 : open_;
    const float ex = CX + side * EYE_DX + gx + fx, ey = cy + gy + side * tilt;
    // Khối mắt bẹt dần rồi tan khi thành vòm cười hoặc trái tim.
    const float w = p.w * sc * (1 + 0.12f * (1 - open)) * (1 - 0.25f * gone);
    const float h = p.h * sc * open * (1 - gone) * (1 - gone);
    const float cyb = ey - 6 * sm;
    eye_cx_[i] = (int)ex;
    eye_cy_ = (int)ey;
    eye_hw_ = (int)(p.w * sc / 2);
    eye_hh_ = (int)(p.h * sc / 2);

    if (sm > 0.01f) {  // Mắt cười: vòm nét dày hai đầu tròn (ảnh đệm vẽ trong update, dán đặc).
      float top = ey - kArcOY;
      if (cv.vis(top, top + kArcH)) arc_spr_[i]->pushToSprite(spr_, cv.X(ex - kArcOX), cv.Y(top));
    }
    if (hrt > 0.01f) {  // Mắt yêu: khối mắt thành trái tim, đập hai nhịp rồi nghỉ; tròng là tim nhỏ tối.
      float ph = fmodf(t, 1.2f) / 1.2f;
      float beat = 1 + 0.07f * (expf(-sq((ph - 0.08f) / 0.05f)) + 0.7f * expf(-sq((ph - 0.3f) / 0.05f)));
      float hk = fminf(1, hrt * 1.5f), k = sc * hk * beat, ky = k * fmaxf(0.12f, open), hy = ey + 2;
      cv.heart(ex, hy, k, ky, col);
      if (open > 0.5f) {
        float px = ex + gx_.x * 9 * k, py = hy + 5 * ky + gy_.x * 5 * ky;
        cv.heart(px, py - 3 * ky, 0.32f * k, 0.32f * ky, mix(col, kBg, hk));
        cv.circle(px + 19 * k, py - 19 * ky, 7 * k, mix(col, kWhite, 0.95f * hk), col);
        cv.circle(px - 11 * k, py + 12 * ky, 3 * k, mix(col, kWhite, 0.75f * hk), col);
      }
    }
    if (h >= 6) {
      const float r = fminf(p.r * sc, fminf(h, w) / 2);
      const float x0 = ex - w / 2, y0 = cyb - h / 2, x1 = ex + w / 2, y1 = cyb + h / 2;
      cv.smoothRect(x0, y0, w, h, r, col, kBg);
      const float fade = fmaxf(0, 1 - gone * 1.8f);  // Con ngươi, đốm sáng tan trước khi khối mắt bẹt hẳn.
      const float dl = L ? p.dL : p.dR, ul = L ? p.uL : p.uR;
      if (open > 0.3f && fade > 0.02f) {
        float px = ex + gx_.x * w * 0.14f, py = cyb + h * 0.06f + gy_.x * h * 0.08f;
        float pk = fade * fmaxf(0, fminf(1, 1 - (dl * (1 + p.dB) - 0.25f) * 3));
        if (pk > 0.02f) {  // Con ngươi khe dọc (viên thuốc màu nền).
          float rx = fmaxf(3.5f, w * p.pw / 2), ry = fmaxf(4, h * p.ph);
          cv.smoothRect(px - rx, py - ry, 2 * rx, 2 * ry, rx, mix(col, kBg, pk), col);
        }
        // Mí che quá nửa (buồn ngủ, chán): tắt đốm sáng cho mắt lơ mơ.
        float hk = fminf(1, (open - 0.5f) / 0.3f) * fade * fmaxf(0, fminf(1, (0.55f - ul) / 0.15f));
        if (hk > 0) {
          float hr = w * 0.1f * (0.8f + 0.2f * p.hi), hx = px + w * 0.15f, hy = py - h * 0.2f;
          // Đốm sáng chạm mép mí trên thì tắt dần, không để lại mẩu trắng bị cắt.
          float u01 = fmaxf(0, fminf(1, (hx - (ex - side * w / 2)) / w * side));
          float lidY = y0 + h * (ul + 0.32f * p.uA * (1 - 2 * u01));
          float hv = ul + fabsf(p.uA) > 0.01f ? fmaxf(0, fminf(1, (hy - hr - lidY) / 6)) : 1;
          if (hk * hv > 0.02f) cv.circle(hx, hy, hr, mix(col, kWhite, 0.95f * hk * hv), col);
          cv.circle(px - w * 0.13f, py + h * 0.17f, hr * 0.45f * p.hi, mix(col, kWhite, 0.75f * hk), col);
        }
      }
      // Mí trên: một vùng màu nền phía trên đường cong nối đầu trong - đầu ngoài (không ghép tam giác: lộ vệt nối).
      {
        float xi = ex - side * (w / 2 + 2), xo = ex + side * (w / 2 + 2);
        float yi = y0 + h * (ul + 0.32f * p.uA), yo = y0 + h * (ul - 0.32f * p.uA);
        float ymU = (yi + yo) / 2 + h * 0.16f * p.uB;
        if (fmaxf(yi, fmaxf(yo, ymU)) > y0 + 0.5f) cv.fillAbove(xo, yo, 2 * ymU - (yi + yo) / 2, xi, yi, y0 - 2, kBg);
      }
      // Mí dưới: vùng phía dưới đường cong; khóe ở độ cao dl, giữa lệch thêm dB * 0.3h.
      if (dl > 0.005f) {
        float yc = y1 - h * dl, ym = yc - h * 0.3f * p.dB;
        cv.fillBelow(x0 - 2, yc, 2 * ym - yc, x1 + 2, yc, y1 + 2, kBg);
      }
    }
    if (p.tear > 0.02f) {  // Nước mắt cùng tông màu, lăn từ khóe ngoài; mờ dần bằng pha màu với nền.
      float eh = p.h * sc;
      for (int k = 0; k < 2; ++k) {
        float q = fmodf(t * 0.7f + k * 0.5f + (side > 0 ? 0.25f : 0), 1.0f);
        float tx = ex + side * p.w * 0.32f, ty = ey + eh * 0.45f + q * 64, rr = 4 + 2 * fminf(1, q * 4);
        cv.ellipse(tx, ty, rr, rr * 1.25f, mix(kBg, mix(col, kWhite, 0.55f), p.tear * fminf(1, (1 - q) / 0.3f)));
      }
    }
  }

  // ---- Chi tiết theo trạng thái ----
  if (asleep_) {  // Chữ z bay lên.
    spr_->setTextDatum(MC_DATUM);
    for (int k = 0; k < 3; ++k) {
      float ph = fmodf(t * 0.35f + k / 3.0f, 1.0f), a = sinf(ph * PI);
      spr_->setTextColor(mix(kBg, col, a));
      spr_->drawString("z", cv.X(400 + ph * 34), cv.Y(142 - ph * 84), ph < 0.5f ? 2 : 4);
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
      cv.circle(414 + k * 20, 84 - ph * 6, 5 + ph * 2.5f, mix(kBg, col, 0.45f + 0.55f * ph), kBg);  // Xa tai phải.
    }
  }
}
