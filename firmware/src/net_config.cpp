#include "net_config.hpp"

#include <Preferences.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef BYTEOF_SERVER_URL
#define BYTEOF_SERVER_URL ""
#endif

namespace net_config {
namespace {
constexpr const char* kNs = "byteof-net";
}

Config load() {
  Config c;
  Preferences p;
  p.begin(kNs, true);
  c.count = min<int>(p.getUChar("n", 0), kMaxNetworks);
  for (int i = 0; i < c.count; ++i) {
    c.nets[i].ssid = p.getString(("s" + String(i)).c_str(), "");
    c.nets[i].pass = p.getString(("p" + String(i)).c_str(), "");
  }
  c.server_url = p.getString("url", BYTEOF_SERVER_URL);
  p.end();
  return c;
}

void save(const String& ssid, const String& pass, const String& server_url) {
  Config old = load();
  Network nets[kMaxNetworks];
  int n = 0;
  nets[n++] = {ssid, pass};
  for (int i = 0; i < old.count && n < kMaxNetworks; ++i) {
    if (old.nets[i].ssid != ssid) nets[n++] = old.nets[i];
  }
  Preferences p;
  p.begin(kNs, false);
  p.putUChar("n", n);
  for (int i = 0; i < n; ++i) {
    p.putString(("s" + String(i)).c_str(), nets[i].ssid);
    p.putString(("p" + String(i)).c_str(), nets[i].pass);
  }
  if (server_url.length() > 0) p.putString("url", server_url);
  p.end();
}

void clear() {
  Preferences p;
  p.begin(kNs, false);
  p.clear();
  p.end();
}

bool parseUrl(const String& url, Url* out) {
  String u = url;
  u.trim();
  Url r;
  if (u.startsWith("wss://")) {
    r.tls = true;
    r.port = 443;
    u = u.substring(6);
  } else if (u.startsWith("ws://")) {
    r.port = 80;
    u = u.substring(5);
  } else {
    return false;
  }
  int slash = u.indexOf('/');
  String hostport = slash < 0 ? u : u.substring(0, slash);
  if (slash >= 0) r.path = u.substring(slash);
  int colon = hostport.indexOf(':');
  if (colon >= 0) {
    long port = hostport.substring(colon + 1).toInt();
    if (port <= 0 || port > 65535) return false;
    r.port = static_cast<uint16_t>(port);
    hostport = hostport.substring(0, colon);
  }
  if (hostport.length() == 0) return false;
  r.host = hostport;
  *out = r;
  return true;
}

}  // namespace net_config
