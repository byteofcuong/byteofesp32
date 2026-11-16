// Cấu hình mạng lưu trong NVS (ADR 0008): các WiFi đã biết và địa chỉ server. Đổi qua trang cài WiFi (portal.*),
// không cần nạp lại firmware.
#pragma once

#include <Arduino.h>

namespace net_config {

constexpr int kMaxNetworks = 5;

struct Network {
  String ssid, pass;
};

struct Config {
  Network nets[kMaxNetworks];
  int count = 0;
  String server_url;  // ws://host:port/path hoặc wss://host[:port]/path.
};

struct Url {
  bool tls = false;
  String host, path = "/";
  uint16_t port = 0;
};

Config load();
// Thêm hoặc cập nhật một WiFi (lên đầu danh sách; đầy thì bỏ mạng cũ nhất) và lưu địa chỉ server.
void save(const String& ssid, const String& pass, const String& server_url);
// Đọc ws://… hoặc wss://…; sai thì trả false.
bool parseUrl(const String& url, Url* out);
// Xóa toàn bộ (lệnh Serial 'W').
void clear();

}  // namespace net_config
