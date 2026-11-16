// Nối board với HomeServer qua WiFi + WebSocket (ADR 0007, ADR 0008, docs/design/protocol.md).
// Mạng chạy trong một FreeRTOS task trên lõi 0; loop() (lõi 1) chỉ đọc lệnh từ hàng đợi nên không bao giờ chờ mạng.
// WiFi và địa chỉ server cài bằng điện thoại qua trang cài WiFi (portal.*), lưu trong NVS (net_config.*).
#pragma once

#include <Arduino.h>

#include "mascot.hpp"

namespace server_link {

enum class Status : uint8_t {
  Offline,     // Thiếu secrets.h (không có token): không nối mạng.
  Joining,     // Đang vào một WiFi đã biết.
  Setup,       // Đang phát WiFi cài đặt, chờ người dùng (setupSsid/setupPass).
  NoServer,    // Có WiFi, chưa chào được server.
  Online,      // Server đã trả "welcome".
};

struct Command {
  enum class Kind : uint8_t { State, Expression } kind;
  uint8_t value;     // AssistantState hoặc Expression.
  uint32_t hold_ms;  // Biểu cảm: thời gian giữ rồi phai về bình thường; 0 = giữ tới lệnh sau.
};

// Bắt đầu task mạng. Thiếu firmware/include/secrets.h thì không làm gì (chạy offline).
void begin();
// Lấy một lệnh server gửi (không chặn). Trả false khi hàng đợi rỗng.
bool poll(Command* out);
// Báo trạng thái hiện tại để gửi kèm lời chào khi nối lại.
void report(AssistantState s, Expression e);
// Báo một lần chạm: "tap", "double", "long". Bỏ qua khi chưa nối.
void sendTouch(const char* kind);
Status status();
// Tên và mật khẩu WiFi cài đặt (riêng từng board, suy từ địa chỉ MAC) để hiện lên màn.
const char* setupSsid();
const char* setupPass();
// Mở trang cài WiFi ngay (lệnh Serial 'w'), kể cả khi đang có mạng.
void requestSetup();
// loop() gọi khi đã trả RAM của linh vật (Status::Setup); task mạng chờ tín hiệu này rồi mới phát WiFi cài đặt.
void setupMemoryReleased();

}  // namespace server_link
