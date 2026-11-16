// Trang cài WiFi (captive portal, ADR 0008): board phát WiFi riêng, người dùng vào bằng điện thoại, chọn mạng,
// nhập mật khẩu và địa chỉ server. Chạy trong task mạng (server_link.cpp), không đụng tới phần vẽ.
#pragma once

#include <Arduino.h>

namespace portal {

void start(const String& ap_ssid, const String& ap_pass, const String& server_url);
void loop();
// Người dùng vừa bấm Lưu (cấu hình đã ghi vào NVS).
bool saved();
void stop();

}  // namespace portal
