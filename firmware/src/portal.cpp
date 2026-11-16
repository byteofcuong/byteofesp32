#include "portal.hpp"

#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#include "net_config.hpp"

namespace portal {
namespace {

DNSServer* dns = nullptr;
WebServer* web = nullptr;
bool saved_flag = false;
String scanned_options;  // <option> cho các WiFi quét được.
String default_url;

String esc(const String& s) {
  String o;
  o.reserve(s.length() + 8);
  for (char c : s) {
    switch (c) {
      case '&': o += "&amp;"; break;
      case '<': o += "&lt;"; break;
      case '>': o += "&gt;"; break;
      case '"': o += "&quot;"; break;
      default: o += c;
    }
  }
  return o;
}

const char kHead[] PROGMEM =
    "<!doctype html><html lang=vi><head><meta charset=utf-8>"
    "<meta name=viewport content='width=device-width,initial-scale=1'><title>byteof</title><style>"
    "body{font-family:system-ui,sans-serif;background:#111;color:#eee;margin:0;padding:20px;max-width:420px}"
    "h1{font-size:20px;color:#ff9a3c}label{display:block;margin:14px 0 4px}"
    "input,select{width:100%;box-sizing:border-box;padding:10px;font-size:16px;border-radius:8px;border:1px solid #444;"
    "background:#222;color:#eee}button{margin-top:18px;width:100%;padding:12px;font-size:17px;border:0;"
    "border-radius:8px;background:#ff9a3c;color:#111}small{color:#999}</style></head><body>";

void scan() {
  scanned_options = "";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String s = WiFi.SSID(i);
    if (s.length() == 0 || scanned_options.indexOf(">" + esc(s) + "<") >= 0) continue;
    scanned_options += "<option value=\"" + esc(s) + "\">" + esc(s) + "</option>";
  }
  WiFi.scanDelete();
  Serial.printf("[portal] scanned %d networks\n", n);
}

void handleRoot() {
  String page = FPSTR(kHead);
  page += "<h1>Cài WiFi cho con cáo</h1>"
          "<form method=post action=/save>"
          "<label>Mạng WiFi (2.4GHz)</label><input name=ssid list=nets required autocomplete=off>"
          "<datalist id=nets>" + scanned_options + "</datalist>"
          "<label>Mật khẩu WiFi</label><input name=pass type=password>"
          "<label>Địa chỉ server</label><input name=url value=\"" + esc(default_url) + "\" placeholder='wss://fox.ten-mien.com'>"
          "<small>Thường giữ nguyên. Chỉ đổi khi server đổi địa chỉ.</small>"
          "<button>Lưu và kết nối</button></form>"
          "<p><small>Board nhớ tối đa 5 mạng; về chỗ cũ nó tự vào lại.</small></p></body></html>";
  web->send(200, "text/html; charset=utf-8", page);
}

void handleSave() {
  String ssid = web->arg("ssid"), pass = web->arg("pass"), url = web->arg("url");
  ssid.trim();
  url.trim();
  net_config::Url parsed;
  if (ssid.length() == 0 || (url.length() > 0 && !net_config::parseUrl(url, &parsed))) {
    web->send(400, "text/html; charset=utf-8",
              String(FPSTR(kHead)) + "<h1>Thiếu tên WiFi hoặc địa chỉ server sai</h1>"
              "<p>Địa chỉ phải bắt đầu bằng wss:// hoặc ws://</p><p><a href=/ style=color:#ff9a3c>Quay lại</a></p>");
    return;
  }
  net_config::save(ssid, pass, url);
  web->send(200, "text/html; charset=utf-8",
            String(FPSTR(kHead)) + "<h1>Đã lưu</h1><p>Con cáo đang vào mạng <b>" + esc(ssid) +
            "</b>. Bạn có thể ngắt WiFi này trên điện thoại.</p></body></html>");
  Serial.printf("[portal] saved network %s\n", ssid.c_str());
  saved_flag = true;
}

// Điện thoại dò captive portal bằng nhiều đường dẫn khác nhau; trả về chuyển hướng để nó tự mở trang.
void handleRedirect() {
  web->sendHeader("Location", "http://192.168.4.1/", true);
  web->send(302, "text/plain", "");
}

}  // namespace

void start(const String& ap_ssid, const String& ap_pass, const String& server_url) {
  default_url = server_url;
  saved_flag = false;
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);  // STA để quét mạng xung quanh.
  scan();
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str());
  dns = new DNSServer();
  dns->start(53, "*", IPAddress(192, 168, 4, 1));
  web = new WebServer(80);
  web->on("/", HTTP_GET, handleRoot);
  web->on("/save", HTTP_POST, handleSave);
  web->onNotFound(handleRedirect);
  web->begin();
  Serial.printf("[portal] AP %s up at 192.168.4.1\n", ap_ssid.c_str());
}

void loop() {
  if (dns) dns->processNextRequest();
  if (web) web->handleClient();
}

bool saved() { return saved_flag; }

void stop() {
  if (web) {
    web->stop();
    delete web;
    web = nullptr;
  }
  if (dns) {
    dns->stop();
    delete dns;
    dns = nullptr;
  }
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  Serial.println("[portal] stopped");
}

}  // namespace portal
