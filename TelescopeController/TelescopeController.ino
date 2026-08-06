/*
 * ESP32 Wi-Fi Autostar II handbox for Meade LX200 GPS
 * ----------------------------------------------------
 * Soft-AP Autostar II UI + SH1106 OLED (same as AdhanOLED) + LX200 RS232 (MAX3232).
 *
 * Mount: Meade LX200 GPS — plug into base "RS232" RJ jack (NOT the HBX port).
 * Protocol: Meade LX200 / Autostar II command set, 9600 8N1.
 * OLED: U8g2 SH1106 128x64 on GPIO21/22 (AdhanOLED pinout). Set OLED_IS_SH1106=0 for SSD1306.
 *
 * Board: ESP32 Dev Module
 * Libraries: U8g2 (Library Manager)
 *
 * AP: ESP32_Telescope_AP / telescopepassword → http://192.168.4.1
 * First open: Setup wizard asks for start location (lat/lon/time).
 */

#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "telescope.h"
#include "celestial.h"
#include "oled_screen.h"
#include "web_ui.h"

WebServer server(80);
ObserverSite site;
uint32_t lastOledMs = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("=== ESP32 Telescope Controller ==="));

  Celestial::beginDefaults(site);

  if (!oledScreen.begin(OLED_SDA_PIN, OLED_SCL_PIN, OLED_I2C_ADDR)) {
    Serial.println(F("OLED not found — continuing without local screen"));
  } else {
    Serial.println(F("OLED OK"));
  }

  telescope.begin(TELESCOPE_RX_PIN, TELESCOPE_TX_PIN, TELESCOPE_BAUD);
  Serial.print(F("Mount: "));
  Serial.println(telescope.status().version);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GW, AP_MASK);
  bool apOk = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print(F("AP start: "));
  Serial.println(apOk ? F("OK") : F("FAIL"));
  Serial.print(F("SSID: "));
  Serial.println(WIFI_SSID);
  Serial.print(F("IP: "));
  Serial.println(WiFi.softAPIP());

  webSetup(server, site);
  server.begin();
  Serial.println(F("Web server on :80"));

  oledScreen.render(telescope.status(), WiFi.softAPIP().toString(), 0);
}

void loop() {
  server.handleClient();

  uint32_t now = millis();
  if (now - lastOledMs >= OLED_REFRESH_MS) {
    lastOledMs = now;
    telescope.refreshPosition(false);
    oledScreen.render(
      telescope.status(),
      WiFi.softAPIP().toString(),
      WiFi.softAPgetStationNum()
    );
  }
}
