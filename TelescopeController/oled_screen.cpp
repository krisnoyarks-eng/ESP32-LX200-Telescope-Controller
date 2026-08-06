#include "oled_screen.h"
#include "config.h"
#include <Wire.h>
#include <U8g2lib.h>

OledScreen oledScreen;

// Same stack as AdhanOLED (U8g2 + SH1106 128x64 on GPIO21/22)
#if OLED_IS_SH1106
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL_PIN, OLED_SDA_PIN);
#else
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL_PIN, OLED_SDA_PIN);
#endif

bool OledScreen::begin(int sda, int scl, int addr) {
  (void)addr;
  Wire.begin(sda, scl);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, 128, 28);
  u8g2.drawStr(4, 12, "Autostar II WiFi");
  u8g2.drawStr(4, 24, "LX200 GPS boot...");
  u8g2.sendBuffer();
  ok_ = true;
  return true;
}

void OledScreen::render(const ScopeStatus& st, const String& apIp, int clients) {
  if (!ok_) return;
  String l1 = st.lcd1.length() ? st.lcd1 : ("RA " + st.ra);
  String l2 = st.lcd2.length() ? st.lcd2 : ("Dec" + st.dec);
  if (l1.length() > 20) l1 = l1.substring(0, 20);
  if (l2.length() > 20) l2 = l2.substring(0, 20);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);

  // Autostar-style 2-line LCD bezel
  u8g2.drawFrame(0, 0, 128, 30);
  u8g2.drawStr(4, 12, l1.c_str());
  u8g2.drawStr(4, 24, l2.c_str());

  char line[32];
  snprintf(line, sizeof(line), "%s %s",
           st.connected ? "LX200GPS" : "DEMO",
           SLEW_RATE_NAMES[st.slewRate >= 0 && st.slewRate < 4 ? st.slewRate : 3]);
  u8g2.drawStr(0, 42, line);

  if (st.slewing) snprintf(line, sizeof(line), "GO TO  cli:%d", clients);
  else if (st.tracking) snprintf(line, sizeof(line), "TRACK  cli:%d", clients);
  else snprintf(line, sizeof(line), "IDLE   cli:%d", clients);
  u8g2.drawStr(0, 54, line);

  u8g2.drawStr(0, 62, apIp.c_str());
  u8g2.sendBuffer();
}
