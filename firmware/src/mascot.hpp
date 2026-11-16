// Linh vật: hai mắt robot có con ngươi khe dọc, tai cáo đặc, mũi tam giác, miệng chữ ω.
// Thiết kế và mọi con số: docs/design/mascot/spec.md và docs/design/mascot/prototype.html.
// Vẽ vào sprite 16-bit theo từng dải ngang (đỡ RAM), dùng hàm vẽ làm mịn cạnh của TFT_eSPI.
#pragma once

#include <TFT_eSPI.h>

// Trạng thái trợ lý: do luồng hội thoại quyết (docs/design/assistant.md).
enum class AssistantState : uint8_t { Idle, Listening, Thinking, Speaking, Notifying, kCount };

// Biểu cảm: độc lập với trạng thái; sau này server gửi theo câu trả lời.
enum class Expression : uint8_t { Neutral, Happy, Sad, Angry, Surprised, Sleepy, kCount };

const char* toString(AssistantState s);
const char* toString(Expression e);

// Thông số hình dạng của một biểu cảm; mọi trường đều nội suy bằng lò xo.
struct FaceShape {
  float w, h, r;       // Kích thước mắt.
  float lid, slope;    // Mí trên: phần che (0..0.6), độ nghiêng (+ trễ ra ngoài, - trễ vào trong).
  float cut;           // Khoét dưới thành mắt cười (0..0.55).
  float dy;            // Dịch cả khuôn mặt theo chiều dọc.
  float mw, mc, mo;    // Miệng: rộng, độ cong (+ cười), độ mở.
  float blush;         // Má hồng (0..1).
  float ear, earSq;    // Tai: góc nghiêng (độ, + cụp ra ngoài), độ ép (1 = bình thường).
  float pw;            // Bề rộng con ngươi (tỉ lệ bề rộng mắt).
};
constexpr int kShapeFields = sizeof(FaceShape) / sizeof(float);

class Mascot {
 public:
  bool begin(TFT_eSPI& tft);

  void setState(AssistantState s, uint32_t now);
  void setExpression(Expression e, uint32_t now);
  AssistantState state() const { return state_; }
  Expression expression() const { return expr_; }

  void wake(uint32_t now) { last_interact_ = now; }
  bool sleeping(uint32_t now) const;

  // Chạm: nháy một mắt, nảy lên, giật tai. Trả về true nếu chạm trúng mắt.
  bool poke(int32_t x, int32_t y, uint32_t now);

  // Cập nhật lò xo, hành vi và vẽ một khung nếu đã tới lúc.
  void update(uint32_t now);

 private:
  struct Spring {
    float x = 0, v = 0;
    void step(float target, float k, float c, float dt) {
      v += (k * (target - x) - c * v) * dt;
      x += v * dt;
    }
  };
  struct Box {
    int x0, y0, x1, y1;
  };

  void drawFace(int ox, int oy, uint32_t now);
  // Vẽ lại ảnh tai khi tư thế/màu đổi (vẽ tai trực tiếp tốn ~36ms/khung, đo 2026-11-16).
  void renderEars(float angle, float squash, uint16_t color);
  float blinkOpen(uint32_t now);
  Box frameBox(bool asleep) const;

  TFT_eSPI* tft_ = nullptr;
  TFT_eSprite* spr_ = nullptr;
  TFT_eSprite* ear_spr_[2] = {nullptr, nullptr};
  int ear_key_angle_ = 9999, ear_key_squash_ = 0;
  uint16_t ear_key_color_ = 0;

  AssistantState state_ = AssistantState::Idle;
  Expression expr_ = Expression::Neutral;
  uint32_t last_interact_ = 0, last_frame_ = 0;

  Spring shape_[kShapeFields];
  Spring gx_, gy_, tilt_, bob_, ear_kick_;
  float target_gx_ = 0, target_gy_ = 0, target_tilt_ = 0;
  uint32_t next_look_ = 0, next_tilt_ = 0, next_blink_ = 0, next_twitch_ = 0;
  uint32_t blink_at_ = 0;  // 0 = không chớp.
  bool double_blink_ = false;
  int wink_eye_ = -1;      // -1: cả hai mắt chớp; 0/1: chỉ mắt đó nháy.

  // Giá trị của khung đang vẽ (tính trong update, dùng trong drawFace).
  FaceShape cur_{};
  float open_ = 1, breath_ = 0, talk_ = 0, t_ = 0;
  bool asleep_ = false;
  uint16_t color_ = 0;
  int eye_cx_[2] = {0, 0}, eye_cy_ = 0, eye_hw_ = 0, eye_hh_ = 0;  // Hộp mắt để biết chạm trúng.

  Box prev_box_{0, 0, 0, 0};
  uint32_t fps_window_start_ = 0, frames_ = 0, draw_us_ = 0, push_us_ = 0;
};
