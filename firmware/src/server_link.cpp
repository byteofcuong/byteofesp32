#include "server_link.hpp"

#if __has_include("secrets.h")
#include "secrets.h"
#define BYTEOF_HAVE_SECRETS 1
#else
#define BYTEOF_HAVE_SECRETS 0
#endif

#if BYTEOF_HAVE_SECRETS
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <time.h>

#include "net_config.hpp"
#include "portal.hpp"

// Bộ chứng chỉ gốc Mozilla dạng nén của ESP32 (firmware/certs/README.md). Cloudflare tự đổi CA nên không ghim một CA.
extern const uint8_t kCertBundleStart[] asm("_binary_certs_x509_crt_bundle_start");
extern const uint8_t kCertBundleEnd[] asm("_binary_certs_x509_crt_bundle_end");
#endif

#include <atomic>

namespace server_link {
namespace {

constexpr const char* kFirmwareVersion = "0.3.0";
constexpr int kQueueLen = 8;
constexpr int kTouchLen = 4;

QueueHandle_t cmd_queue = nullptr;    // Server -> loop().
QueueHandle_t touch_queue = nullptr;  // loop() -> server (chỉ số trong kTouchKinds).
std::atomic<uint8_t> cur_state{0}, cur_expr{0};
std::atomic<Status> cur_status{Status::Offline};
std::atomic<bool> setup_requested{false};
std::atomic<bool> setup_mem_free{false};
char ap_ssid[24] = "", ap_pass[16] = "";

const char* const kTouchKinds[] = {"tap", "double", "long"};

template <typename E>
bool parseEnum(const char* name, E* out) {
  if (name == nullptr) return false;
  for (int i = 0; i < static_cast<int>(E::kCount); ++i) {
    if (strcmp(name, toString(static_cast<E>(i))) == 0) {
      *out = static_cast<E>(i);
      return true;
    }
  }
  return false;
}

#if BYTEOF_HAVE_SECRETS
constexpr uint32_t kJoinTimeoutMs = 15000;
constexpr uint32_t kWifiLostMs = 10000;       // Mất WiFi lâu hơn thì quay lại tìm mạng.
constexpr uint32_t kPortalRescanMs = 120000;  // Đang cài đặt mà mạng đã biết xuất hiện lại thì tự vào.

WebSocketsClient ws;
String server_host;  // Giữ chuỗi sống suốt thời gian ws dùng.

void setStatus(Status s) {
  if (cur_status.exchange(s) != s) {
    static const char* names[] = {"offline", "joining", "setup", "no-server", "online"};
    Serial.printf("[link] status=%s heap=%lu largest=%lu\n", names[static_cast<int>(s)],
                  static_cast<unsigned long>(ESP.getFreeHeap()), static_cast<unsigned long>(ESP.getMaxAllocHeap()));
  }
}

void sendHello() {
  JsonDocument doc;
  doc["t"] = "hello";
  doc["token"] = BYTEOF_DEVICE_TOKEN;
  doc["fw"] = kFirmwareVersion;
  doc["state"] = toString(static_cast<AssistantState>(cur_state.load()));
  doc["expression"] = toString(static_cast<Expression>(cur_expr.load()));
  String out;
  serializeJson(doc, out);
  ws.sendTXT(out);
}

void handleText(const uint8_t* payload, size_t len) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, len)) {
    Serial.println("[link] bad json");
    return;
  }
  const char* t = doc["t"] | "";
  Command cmd;
  if (strcmp(t, "welcome") == 0) {
    setStatus(Status::Online);
    return;
  } else if (strcmp(t, "state") == 0) {
    AssistantState s;
    if (!parseEnum(doc["state"].as<const char*>(), &s)) {
      Serial.printf("[link] bad state %s\n", doc["state"].as<const char*>());
      return;
    }
    cmd = {Command::Kind::State, static_cast<uint8_t>(s), 0};
  } else if (strcmp(t, "expression") == 0) {
    Expression e;
    if (!parseEnum(doc["expression"].as<const char*>(), &e)) {
      Serial.printf("[link] bad expression %s\n", doc["expression"].as<const char*>());
      return;
    }
    // "hold" (giây, tùy chọn): giữ rồi phai về bình thường (docs/design/behavior.md). Kẹp 0–600s.
    const float hold_s = fminf(600.0f, fmaxf(0.0f, doc["hold"] | 0.0f));
    cmd = {Command::Kind::Expression, static_cast<uint8_t>(e), static_cast<uint32_t>(hold_s * 1000)};
  } else {
    Serial.printf("[link] bad type %s\n", t);
    return;
  }
  xQueueSend(cmd_queue, &cmd, 0);  // Đầy thì bỏ (loop() rút mỗi khung nên gần như không xảy ra).
}

void onEvent(WStype_t type, uint8_t* payload, size_t len) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[link] connected %s\n", server_host.c_str());
      sendHello();
      break;
    case WStype_DISCONNECTED:
      if (cur_status == Status::Online) Serial.println("[link] disconnected");
      if (cur_status == Status::Online) setStatus(Status::NoServer);
      break;
    case WStype_TEXT:
      handleText(payload, len);
      break;
    case WStype_ERROR:
      Serial.printf("[link] error %.*s\n", static_cast<int>(len), payload ? reinterpret_cast<char*>(payload) : "");
      break;
    default:
      break;
  }
}

// Vào WiFi mạnh nhất trong các mạng đã lưu. Trả false nếu không vào được mạng nào.
bool joinKnown(const net_config::Config& cfg) {
  if (cfg.count == 0) return false;
  setStatus(Status::Joining);
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  // Thứ tự thử: mạng đã lưu có mặt, mạnh trước.
  int order[net_config::kMaxNetworks], rssi[net_config::kMaxNetworks], m = 0;
  for (int k = 0; k < cfg.count; ++k) {
    int best = -1000;
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) == cfg.nets[k].ssid) best = max(best, static_cast<int>(WiFi.RSSI(i)));
    }
    if (best > -1000) {
      int j = m++;
      while (j > 0 && rssi[j - 1] < best) {
        order[j] = order[j - 1];
        rssi[j] = rssi[j - 1];
        --j;
      }
      order[j] = k;
      rssi[j] = best;
    }
  }
  WiFi.scanDelete();
  for (int j = 0; j < m; ++j) {
    const auto& net = cfg.nets[order[j]];
    Serial.printf("[wifi] joining %s (rssi %d)\n", net.ssid.c_str(), rssi[j]);
    WiFi.begin(net.ssid.c_str(), net.pass.c_str());
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < kJoinTimeoutMs) vTaskDelay(pdMS_TO_TICKS(100));
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[wifi] connected ip=%s rssi=%d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
      return true;
    }
    Serial.printf("[wifi] %s failed\n", net.ssid.c_str());
    WiFi.disconnect();
  }
  if (m == 0) Serial.println("[wifi] no known network in range");
  return false;
}

bool knownInRange(const net_config::Config& cfg) {
  int n = WiFi.scanNetworks();
  bool found = false;
  for (int i = 0; i < n && !found; ++i) {
    for (int k = 0; k < cfg.count; ++k) found = found || WiFi.SSID(i) == cfg.nets[k].ssid;
  }
  WiFi.scanDelete();
  return found;
}

// Chạy trang cài WiFi tới khi người dùng lưu, hoặc một mạng đã biết xuất hiện lại.
void runPortal() {
  net_config::Config cfg = net_config::load();
  setStatus(Status::Setup);
  // Phát WiFi + trang web cần ~65KB, và thêm nữa khi điện thoại vào; con cáo giữ ~150KB nên phải nhường
  // (2026-11-16: điện thoại vào là board hết RAM, abort trong ampdu_alloc_rx_ba_index).
  while (!setup_mem_free) vTaskDelay(pdMS_TO_TICKS(20));
  portal::start(ap_ssid, ap_pass, cfg.server_url);
  uint32_t last_scan = millis();
  while (!portal::saved()) {
    portal::loop();
    if (cfg.count > 0 && WiFi.softAPgetStationNum() == 0 && millis() - last_scan > kPortalRescanMs) {
      last_scan = millis();
      if (knownInRange(cfg)) break;
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
  vTaskDelay(pdMS_TO_TICKS(1500));  // Để trang "Đã lưu" kịp về điện thoại.
  portal::stop();
  Serial.println("[portal] restarting to free memory");
  Serial.flush();
  ESP.restart();  // Con cáo đã trả RAM: khởi động lại để có lại linh vật và vào mạng vừa cài.
}

void syncClock() {
  // Kiểm chứng chứng chỉ TLS cần giờ thật.
  // Múi giờ UTC+7: lớp hành vi dùng giờ địa phương để biết ban đêm (behavior.cpp).
  configTzTime("<+07>-7", "pool.ntp.org", "time.cloudflare.com");
  uint32_t t0 = millis();
  while (time(nullptr) < 1700000000 && millis() - t0 < 8000) vTaskDelay(pdMS_TO_TICKS(100));
  Serial.printf("[time] %s\n", time(nullptr) > 1700000000 ? "synced" : "not synced");
}

bool startSocket(const net_config::Config& cfg) {
  net_config::Url url;
  if (!net_config::parseUrl(cfg.server_url, &url)) {
    Serial.printf("[link] bad server url '%s'\n", cfg.server_url.c_str());
    return false;
  }
  server_host = String(url.tls ? "wss://" : "ws://") + url.host + ":" + String(url.port) + url.path;
  static String host, path;  // ws giữ con trỏ tới chuỗi.
  host = url.host;
  path = url.path;
  if (url.tls) {
    syncClock();
    ws.beginSslWithBundle(host.c_str(), url.port, path.c_str(), kCertBundleStart,
                          static_cast<size_t>(kCertBundleEnd - kCertBundleStart), "");
  } else {
    ws.begin(host.c_str(), url.port, path.c_str(), "");
  }
  ws.onEvent(onEvent);
  ws.setReconnectInterval(3000);
  ws.enableHeartbeat(15000, 3000, 2);
  setStatus(Status::NoServer);
  Serial.printf("[link] server %s\n", server_host.c_str());
  return true;
}

void task(void*) {
  for (;;) {
    net_config::Config cfg = net_config::load();
    if (setup_requested.exchange(false) || !joinKnown(cfg)) {
      runPortal();
      continue;
    }
    if (!startSocket(cfg)) {
      runPortal();  // Địa chỉ server sai: cho người dùng sửa.
      continue;
    }
    uint32_t lost_at = 0;
    for (;;) {
      if (setup_requested) break;
      if (WiFi.status() != WL_CONNECTED) {
        if (lost_at == 0) {
          lost_at = millis();
          Serial.println("[wifi] lost");
        }
        if (millis() - lost_at > kWifiLostMs) break;
      } else {
        lost_at = 0;
        ws.loop();
        uint8_t k;
        while (xQueueReceive(touch_queue, &k, 0) == pdTRUE) {
          if (cur_status != Status::Online) continue;
          char buf[40];
          snprintf(buf, sizeof(buf), "{\"t\":\"touch\",\"kind\":\"%s\"}", kTouchKinds[k]);
          ws.sendTXT(buf);
        }
      }
      vTaskDelay(pdMS_TO_TICKS(20));  // Hỏi TLS 50 lần/giây: đủ nhanh, bớt tranh bộ nhớ đệm flash với lõi vẽ.
    }
    ws.disconnect();
    WiFi.disconnect();
  }
}
#endif

}  // namespace

void begin() {
#if BYTEOF_HAVE_SECRETS
  uint64_t mac = ESP.getEfuseMac();
  snprintf(ap_ssid, sizeof(ap_ssid), "byteof-%04X", static_cast<unsigned>((mac >> 32) & 0xFFFF));
  snprintf(ap_pass, sizeof(ap_pass), "fox%08lx", static_cast<unsigned long>(mac & 0xFFFFFFFF));
  cmd_queue = xQueueCreate(kQueueLen, sizeof(Command));
  touch_queue = xQueueCreate(kTouchLen, sizeof(uint8_t));
  setStatus(Status::Joining);
  // Lõi 0 cùng chỗ với ngăn xếp WiFi; loop() vẽ linh vật ở lõi 1. TLS cần ngăn xếp lớn.
  xTaskCreatePinnedToCore(task, "server_link", 12288, nullptr, 1, nullptr, 0);
#else
  Serial.println("[link] no secrets.h, offline");
#endif
}

bool poll(Command* out) {
  return cmd_queue != nullptr && xQueueReceive(cmd_queue, out, 0) == pdTRUE;
}

void report(AssistantState s, Expression e) {
  cur_state = static_cast<uint8_t>(s);
  cur_expr = static_cast<uint8_t>(e);
}

void sendTouch(const char* kind) {
  if (touch_queue == nullptr || cur_status != Status::Online) return;
  for (uint8_t i = 0; i < 3; ++i) {
    if (strcmp(kind, kTouchKinds[i]) == 0) {
      xQueueSend(touch_queue, &i, 0);
      return;
    }
  }
}

Status status() { return cur_status; }
const char* setupSsid() { return ap_ssid; }
const char* setupPass() { return ap_pass; }
void requestSetup() { setup_requested = true; }
void setupMemoryReleased() { setup_mem_free = true; }

}  // namespace server_link
