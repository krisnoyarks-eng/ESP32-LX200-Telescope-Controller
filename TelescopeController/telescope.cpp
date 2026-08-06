#include "telescope.h"
#include "config.h"

Telescope telescope;

void Telescope::begin(int rxPin, int txPin, int baud) {
  serial_.begin(baud, SERIAL_8N1, rxPin, txPin);
  st_.connected = false;
  st_.tracking = false;
  st_.slewing = false;
  st_.slewRate = 3; // SLEW
  st_.ra = "--:--:--";
  st_.dec = "--*--:--";
  st_.alt = "--*--:--";
  st_.az = "---*--:--";
  st_.version = "unknown";
  st_.lastCmd = "";
  st_.lastReply = "";
  st_.targetName = "-";
  st_.targetRaHours = 0;
  st_.targetDecDeg = 0;
  updateLcd("Meade LX200 GPS", "Autostar II WiFi");
  delay(50);
  String v = getVersion();
  if (v.length() && !v.startsWith("Timeout")) {
    st_.connected = true;
    st_.version = v;
    updateLcd("LX200 GPS linked", v);
  } else {
#if DEMO_FALLBACK
    st_.connected = false;
    st_.version = "DEMO (no mount)";
    st_.ra = formatRA(simRa_);
    st_.dec = formatDec(simDec_);
    updateLcd("DEMO MODE", "No RS232 reply");
#else
    updateLcd("No mount reply", "Check RS232");
#endif
  }
}

void Telescope::updateLcd(const String& line1, const String& line2) {
  st_.lcd1 = line1.substring(0, 20);
  st_.lcd2 = line2.substring(0, 20);
}

void Telescope::refreshLcdFromStatus() {
  String l1 = "RA " + st_.ra;
  String l2 = "Dec" + st_.dec;
  if (st_.targetName.length() && st_.targetName != "-") {
    l1 = st_.targetName.substring(0, 16);
    l2 = (st_.slewing ? "Going to... " : (st_.tracking ? "Tracking " : "Select ")) +
         String(SLEW_RATE_NAMES[st_.slewRate]);
  }
  updateLcd(l1, l2);
}

String Telescope::sendRaw(const String& cmd, uint32_t timeoutMs) {
  st_.lastCmd = cmd;
  while (serial_.available()) serial_.read();
  serial_.print(cmd);
  serial_.flush();

  String response;
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (serial_.available() > 0) {
      char c = (char)serial_.read();
      response += c;
      if (c == '#') {
        st_.lastReply = response;
        String show = response;
        if (show.endsWith("#")) show.remove(show.length() - 1);
        updateLcd(cmd, show.length() ? show : "(ack)");
        return response;
      }
    }
    // Some LX200 cmds ACK with '0'/'1' without '#'
    if (response.length() == 1 && (response[0] == '0' || response[0] == '1' || response[0] == '2')) {
      st_.lastReply = response;
      updateLcd(cmd, "ACK " + response);
      return response;
    }
    delay(1);
  }

  if (response.length() == 0) {
    st_.lastReply = "Timeout";
#if DEMO_FALLBACK
    if (cmd.indexOf(":GR") >= 0) {
      st_.lastReply = formatRA(simRa_) + "#";
      updateLcd(":GR#", formatRA(simRa_));
      return st_.lastReply;
    }
    if (cmd.indexOf(":GD") >= 0) {
      st_.lastReply = formatDec(simDec_) + "#";
      updateLcd(":GD#", formatDec(simDec_));
      return st_.lastReply;
    }
    if (cmd.indexOf(":GA") >= 0) {
      st_.lastReply = formatDec(simAlt_) + "#";
      updateLcd(":GA#", formatDec(simAlt_));
      return st_.lastReply;
    }
    if (cmd.indexOf(":GZ") >= 0) {
      char buf[16];
      snprintf(buf, sizeof(buf), "%03d*%02d:%02d", (int)simAz_, 0, 0);
      st_.lastReply = String(buf) + "#";
      updateLcd(":GZ#", buf);
      return st_.lastReply;
    }
    if (cmd.indexOf(":GV") >= 0) {
      st_.lastReply = "LX200GPS-DEMO#";
      updateLcd(":GV#", "LX200GPS-DEMO");
      return st_.lastReply;
    }
    if (cmd.indexOf(":Sr") >= 0 || cmd.indexOf(":Sd") >= 0) {
      st_.lastReply = "1";
      updateLcd(cmd, "OK");
      return st_.lastReply;
    }
    if (cmd.indexOf(":MS") >= 0) {
      st_.slewing = true;
      st_.lastReply = "0";
      updateLcd("GO TO", st_.targetName);
      return st_.lastReply;
    }
    if (cmd.indexOf(":Q") >= 0) {
      st_.slewing = false;
      st_.lastReply = "";
      updateLcd("STOP", "Motors halted");
      return "";
    }
    if (cmd.indexOf(":Te") >= 0 || cmd.indexOf(":Td") >= 0 ||
        cmd.indexOf(":RG") >= 0 || cmd.indexOf(":RC") >= 0 ||
        cmd.indexOf(":RM") >= 0 || cmd.indexOf(":RS") >= 0) {
      st_.lastReply = "1";
      updateLcd(cmd, "OK");
      return "1";
    }
    if (cmd.indexOf(":Mn") >= 0 || cmd.indexOf(":Ms") >= 0 ||
        cmd.indexOf(":Me") >= 0 || cmd.indexOf(":Mw") >= 0) {
      updateLcd("MOVE", cmd);
      return "";
    }
#endif
    updateLcd(cmd, "Timeout");
    return "Timeout: No response from telescope.";
  }
  st_.lastReply = response;
  updateLcd(cmd, response);
  return response;
}

String Telescope::getRA() {
  String r = sendRaw(":GR#");
  if (r.endsWith("#")) r.remove(r.length() - 1);
  if (!r.startsWith("Timeout")) st_.ra = r;
  return st_.ra;
}

String Telescope::getDec() {
  String r = sendRaw(":GD#");
  if (r.endsWith("#")) r.remove(r.length() - 1);
  if (!r.startsWith("Timeout")) st_.dec = r;
  return st_.dec;
}

String Telescope::getAlt() {
  String r = sendRaw(":GA#");
  if (r.endsWith("#")) r.remove(r.length() - 1);
  if (!r.startsWith("Timeout")) st_.alt = r;
  return st_.alt;
}

String Telescope::getAz() {
  String r = sendRaw(":GZ#");
  if (r.endsWith("#")) r.remove(r.length() - 1);
  if (!r.startsWith("Timeout")) st_.az = r;
  return st_.az;
}

String Telescope::getVersion() {
  String r = sendRaw(":GV#", 1500);
  if (r.endsWith("#")) r.remove(r.length() - 1);
  return r;
}

String Telescope::formatRA(float hours) {
  while (hours < 0) hours += 24.0f;
  while (hours >= 24.0f) hours -= 24.0f;
  int h = (int)hours;
  float mWhole = (hours - h) * 60.0f;
  int m = (int)mWhole;
  int s = (int)((mWhole - m) * 60.0f + 0.5f);
  if (s >= 60) { s = 0; m++; }
  if (m >= 60) { m = 0; h++; }
  if (h >= 24) h = 0;
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

String Telescope::formatDec(float deg) {
  char sign = deg < 0 ? '-' : '+';
  float a = fabsf(deg);
  int d = (int)a;
  float mWhole = (a - d) * 60.0f;
  int m = (int)mWhole;
  int s = (int)((mWhole - m) * 60.0f + 0.5f);
  if (s >= 60) { s = 0; m++; }
  if (m >= 60) { m = 0; d++; }
  char buf[16];
  snprintf(buf, sizeof(buf), "%c%02d*%02d:%02d", sign, d, m, s);
  return String(buf);
}

bool Telescope::parseRA(const String& s, float& hoursOut) {
  int h = 0, m = 0, sec = 0;
  if (sscanf(s.c_str(), "%d:%d:%d", &h, &m, &sec) < 2) return false;
  hoursOut = h + m / 60.0f + sec / 3600.0f;
  return true;
}

bool Telescope::parseDec(const String& s, float& degOut) {
  int d = 0, m = 0, sec = 0;
  char sign = '+';
  if (sscanf(s.c_str(), "%c%d*%d:%d", &sign, &d, &m, &sec) >= 2 ||
      sscanf(s.c_str(), "%c%d:%d:%d", &sign, &d, &m, &sec) >= 2) {
    degOut = d + m / 60.0f + sec / 3600.0f;
    if (sign == '-') degOut = -degOut;
    return true;
  }
  if (sscanf(s.c_str(), "%d*%d:%d", &d, &m, &sec) >= 2) {
    degOut = d + m / 60.0f + sec / 3600.0f;
    return true;
  }
  return false;
}

bool Telescope::setRA(float raHours) {
  String cmd = ":Sr" + formatRA(raHours) + "#";
  String r = sendRaw(cmd);
  st_.targetRaHours = raHours;
#if DEMO_FALLBACK
  simRa_ = raHours;
#endif
  return r.indexOf('1') >= 0 || r == "1";
}

bool Telescope::setDec(float decDeg) {
  String cmd = ":Sd" + formatDec(decDeg) + "#";
  String r = sendRaw(cmd);
  st_.targetDecDeg = decDeg;
#if DEMO_FALLBACK
  simDec_ = decDeg;
#endif
  return r.indexOf('1') >= 0 || r == "1";
}

int Telescope::slewToTarget() {
  String r = sendRaw(":MS#");
  if (r.length() == 0) return -1;
  char c = r.charAt(0);
  if (c == '0') {
    st_.slewing = true;
#if DEMO_FALLBACK
    st_.ra = formatRA(st_.targetRaHours);
    st_.dec = formatDec(st_.targetDecDeg);
    simRa_ = st_.targetRaHours;
    simDec_ = st_.targetDecDeg;
#endif
    updateLcd("GO TO " + st_.targetName, "Slewing...");
    return 0;
  }
  if (c == '1') { updateLcd("GO TO failed", "Below horizon"); return 1; }
  if (c == '2') { updateLcd("GO TO failed", "Not aligned"); return 2; }
  return -1;
}

void Telescope::stopAll() {
  sendRaw(":Q#");
  st_.slewing = false;
}

void Telescope::move(char dir) {
  char cmd[8];
  snprintf(cmd, sizeof(cmd), ":M%c#", dir);
  sendRaw(String(cmd));
}

void Telescope::haltMove(char dir) {
  char cmd[8];
  snprintf(cmd, sizeof(cmd), ":Q%c#", dir);
  sendRaw(String(cmd));
}

bool Telescope::setTracking(bool on) {
  String r = sendRaw(on ? ":Te#" : ":Td#");
  st_.tracking = on && (r.indexOf('1') >= 0 || r.length() == 0 || DEMO_FALLBACK);
  updateLcd(on ? "Tracking ON" : "Tracking OFF", SLEW_RATE_NAMES[st_.slewRate]);
  return st_.tracking == on || DEMO_FALLBACK;
}

void Telescope::syncHere() {
  sendRaw(":CM#");
  updateLcd("SYNC", "Position synced");
}

void Telescope::setSlewRate(int idx) {
  if (idx < 0) idx = 0;
  if (idx > 3) idx = 3;
  st_.slewRate = idx;
  sendRaw(String(SLEW_RATE_CMDS[idx]));
  updateLcd("SPEED", SLEW_RATE_NAMES[idx]);
}

int Telescope::cycleSlewRate() {
  int next = (st_.slewRate + 1) % 4;
  setSlewRate(next);
  return st_.slewRate;
}

void Telescope::pushSiteToMount(float latDeg, float lonDeg, int year, int month, int day,
                                int hour, int minute, int second, int tzMinutes) {
  // Latitude :StsDD*MM#  (+ North / - South)
  char sign = latDeg >= 0 ? '+' : '-';
  float alat = fabsf(latDeg);
  int latD = (int)alat;
  int latM = (int)((alat - latD) * 60.0f + 0.5f);
  char buf[24];
  snprintf(buf, sizeof(buf), ":St%c%02d*%02d#", sign, latD, latM);
  sendRaw(String(buf));

  // Longitude :SgDDD*MM# — Meade wants degrees West of Greenwich (0..360)
  // Our site uses east-positive lon (e.g. Colorado ≈ -105 → west 105)
  float west = -lonDeg;
  while (west < 0) west += 360.0f;
  while (west >= 360.0f) west -= 360.0f;
  int lonD = (int)west;
  int lonM = (int)((west - lonD) * 60.0f + 0.5f);
  snprintf(buf, sizeof(buf), ":Sg%03d*%02d#", lonD, lonM);
  sendRaw(String(buf));

  // Local time :SLHH:MM:SS#
  snprintf(buf, sizeof(buf), ":SL%02d:%02d:%02d#", hour, minute, second);
  sendRaw(String(buf));

  // Date :SCMM/DD/YY#
  int yy = year % 100;
  snprintf(buf, sizeof(buf), ":SC%02d/%02d/%02d#", month, day, yy);
  sendRaw(String(buf));

  // UTC offset :SGsHH# — hours from GMT (Autostar II). West zones → '+'
  int hours = abs(tzMinutes) / 60;
  char tsign = (tzMinutes <= 0) ? '+' : '-';
  snprintf(buf, sizeof(buf), ":SG%c%02d#", tsign, hours);
  sendRaw(String(buf));

  updateLcd("Site / time set", "Pushed to LX200 GPS");
}

void Telescope::refreshPosition(bool force) {
  uint32_t now = millis();
  if (!force && (now - lastPosMs_ < POS_CACHE_MS)) return;
  lastPosMs_ = now;
  getRA();
  getDec();
}
