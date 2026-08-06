#pragma once
#include <Arduino.h>

struct ScopeStatus {
  String ra;
  String dec;
  String alt;
  String az;
  String version;
  String lastCmd;
  String lastReply;
  String lcd1;       // Autostar-style line 1
  String lcd2;       // Autostar-style line 2
  bool connected;
  bool tracking;
  bool slewing;
  int slewRate;      // 0..3 Guide..Slew
  String targetName;
  float targetRaHours;
  float targetDecDeg;
};

class Telescope {
public:
  void begin(int rxPin, int txPin, int baud);
  String sendRaw(const String& cmd, uint32_t timeoutMs = 2000);
  String getRA();
  String getDec();
  String getAlt();
  String getAz();
  String getVersion();
  bool setRA(float raHours);
  bool setDec(float decDeg);
  int  slewToTarget();
  void stopAll();
  void move(char dir);   // 'n','s','e','w' (Autostar ▲▼◄►)
  void haltMove(char dir);
  bool setTracking(bool on);
  void syncHere();
  int  cycleSlewRate();  // SPEED key
  void setSlewRate(int idx);
  void updateLcd(const String& line1, const String& line2);
  void refreshLcdFromStatus();
  // Push observer site/time into LX200 GPS (Autostar setup equivalent)
  void pushSiteToMount(float latDeg, float lonDeg, int year, int month, int day,
                       int hour, int minute, int second, int tzMinutes);
  ScopeStatus& status() { return st_; }
  void refreshPosition(bool force = false);

  static String formatRA(float hours);
  static String formatDec(float deg);
  static bool parseRA(const String& s, float& hoursOut);
  static bool parseDec(const String& s, float& degOut);

private:
  HardwareSerial serial_{2};
  ScopeStatus st_;
  uint32_t lastPosMs_ = 0;
  float simRa_ = 5.5f;
  float simDec_ = 22.0f;
  float simAlt_ = 45.0f;
  float simAz_ = 180.0f;
};

extern Telescope telescope;
