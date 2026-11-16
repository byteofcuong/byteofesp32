// Linh vật: hai mắt robot có con ngươi khe dọc, tai cáo đặc, mũi tam giác, miệng chữ ω.
// Thiết kế và mọi con số: docs/design/mascot/spec.md và docs/design/mascot/prototype.html.
// Vẽ vào sprite 16-bit theo từng dải ngang (đỡ RAM), dùng hàm vẽ làm mịn cạnh của TFT_eSPI.
#pragma once

#include <TFT_eSPI.h>

// Trạng thái trợ lý: do luồng hội thoại quyết (docs/design/assistant.md).
enum class AssistantState : uint8_t { Idle, Listening, Thinking, Speaking, Notifying, kCount };

// Biểu cảm: độc lập với trạng thái; sau này server gửi theo câu trả lời. Thứ tự khớp bản mẫu.
enum class Expression : uint8_t {
  Neutral, Happy, Laugh, Excited, Love, Shy, Smug, Sad, Cry, Angry, Surprised, Scared, Confused, Bored, Sleepy,
  kCount
};

const char* toString(AssistantState s);
const char* toString(Expression e);

// Thông số hình dạng của một biểu cảm; mọi trường đều nội suy bằng lò xo (ý nghĩa: prototype.html).
struct FaceShape {
  float w, h, r;            // Kích thước mắt.
  float sL, sR;             // Tỉ lệ từng mắt.
  float sm, hrt;            // Mắt cười (vòm cong), mắt trái tim (0..1).
  float uL, uR, uA, uB;     // Mí trên: che từng mắt, nghiêng (+ cau), võng (+ giữa trũng).
  float dL, dR, dB;         // Mí dưới: che từng mắt, cong (+ giữa đội lên).
  float pw, ph, hi;         // Con ngươi rộng/cao (tỉ lệ mắt), cỡ đốm sáng.
  float mw, mm, ma, mv, mo; // Miệng ω: rộng, sâu nếp (< 0 lật ngược = mếu), lệch, gợn; chữ o (0..1).
  float tg, fL, fR;         // Lưỡi lè, răng nanh trái/phải (0..1).
  float ear, earSq;         // Tai: góc (độ, + cụp ra ngoài), độ ép (1 = bình thường).
  float dy;                 // Dịch cả khuôn mặt theo chiều dọc.
  float bnc, chk, sway, trem, tiltB;  // Nảy, cười khúc khích, đung đưa, run, nghiêng đầu.
  float tint, tear;         // Ám đỏ (giận), nước mắt.
  float gx, gy, gh;         // Hướng nhìn ưu tiên và mức ưu tiên.
  float jump;               // > 0: bật lên khi vào biểu cảm.
  float doze;               // Gà gật: mí sụp thêm rồi cố mở, đầu gật theo (buồn ngủ).
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

  // Chạm một lần: trúng mắt thì nháy mắt đó; trúng mũi/miệng thì "boop" (híp mắt, nảy);
  // chỗ khác thì đánh mắt nhìn về điểm chạm ~1.8 s, tai giật nhẹ.
  void tap(int32_t x, int32_t y, uint32_t now);

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
  TFT_eSprite* mouth_spr_ = nullptr;                 // Ảnh đệm cụm mũi + miệng (khi không nói).
  TFT_eSprite* arc_spr_[2] = {nullptr, nullptr};     // Ảnh đệm vòm mắt cười.
  TFT_eSprite* badge_spr_ = nullptr;                 // Chấm báo khi thông báo: đẩy riêng, ngoài vùng mặt.
  uint32_t mouth_key_ = 0, arc_key_[2] = {0, 0};
  int ear_key_angle_ = 9999, ear_key_squash_ = 0;
  uint16_t ear_key_color_ = 0;

  AssistantState state_ = AssistantState::Idle;
  Expression expr_ = Expression::Neutral;
  uint32_t last_interact_ = 0, last_frame_ = 0;

  Spring shape_[kShapeFields];
  Spring gx_, gy_, tilt_, bob_, ear_kick_, talk_s_;
  float target_gx_ = 0, target_gy_ = 0, target_tilt_ = 0, talk_target_ = 0;
  uint32_t next_look_ = 0, next_tilt_ = 0, next_blink_ = 0, next_twitch_ = 0, next_syllable_ = 0;
  uint32_t blink_at_ = 0;  // 0 = không chớp.
  uint32_t look_until_ = 0;  // Đang nhìn điểm vừa chạm tới lúc này (bỏ qua hướng nhìn ưu tiên của biểu cảm).
  uint32_t next_alert_ = 0;  // Nhịp "gọi" khi thông báo: nhún hai nhịp + giật tai.
  int alert_step_ = 0;
  bool edge_lit_ = false;  // Viền sáng thông báo đang vẽ trên màn (cần xóa khi thôi).
  bool double_blink_ = false;
  int wink_eye_ = -1;      // -1: cả hai mắt chớp; 0/1: chỉ mắt đó nháy.

  // Giá trị của khung đang vẽ (tính trong update, dùng trong drawFace).
  FaceShape cur_{};
  float open_ = 1, breath_ = 0, talk_ = -1, t_ = 0;  // talk_ < 0: không nói.
  float move_x_ = 0, move_y_ = 0, move_tilt_ = 0;    // Chuyển động theo biểu cảm.
  bool asleep_ = false;
  uint16_t color_ = 0;
  int eye_cx_[2] = {0, 0}, eye_cy_ = 0, eye_hw_ = 0, eye_hh_ = 0;  // Hộp mắt để biết chạm trúng.

  Box prev_box_{0, 0, 0, 0};
  uint32_t fps_window_start_ = 0, frames_ = 0, draw_us_ = 0, push_us_ = 0, max_us_ = 0;
};
