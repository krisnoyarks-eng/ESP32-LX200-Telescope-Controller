#pragma once

// =============================================================================
// Meade LX200 GPS (Autostar II) — ESP32 Wi-Fi handbox replacement
// Serial: mount base RS232 RJ11/RJ12 jacks via MAX3232 (#507-style pinout)
// =============================================================================

static const char* WIFI_SSID     = "ESP32_Telescope_AP";
static const char* WIFI_PASSWORD = "telescopepassword";
static const IPAddress AP_IP(192, 168, 4, 1);
static const IPAddress AP_GW(192, 168, 4, 1);
static const IPAddress AP_MASK(255, 255, 255, 0);

// UART2 → MAX3232 TTL → RS232 RJ into LX200 GPS "RS232" port (not HBX)
static const int TELESCOPE_RX_PIN = 16;  // ESP32 RX  ← MAX3232 R1OUT
static const int TELESCOPE_TX_PIN = 17;  // ESP32 TX  → MAX3232 T1IN
static const int TELESCOPE_BAUD   = 9600; // LX200 / Autostar II standard
static const uint32_t CMD_TIMEOUT_MS = 2000;

// --- OLED (same module as AdhanOLED clock) ---
// AdhanOLED/config.h: OLED_SDA=21, OLED_SCL=22, OLED_IS_SH1106=1
static const int OLED_SDA_PIN = 21;
static const int OLED_SCL_PIN = 22;
#ifndef OLED_IS_SH1106
#define OLED_IS_SH1106 1   // 1 = SH1106 (Adhan clock boards), 0 = SSD1306
#endif
static const int OLED_I2C_ADDR = 0x3C;
static const int OLED_WIDTH  = 128;
static const int OLED_HEIGHT = 64;

#ifndef DEMO_FALLBACK
#define DEMO_FALLBACK 1
#endif

static const float DEFAULT_LAT_DEG = 40.0f;
static const float DEFAULT_LON_DEG = -105.0f;
static const int   DEFAULT_TZ_MIN  = -360;

static const uint32_t OLED_REFRESH_MS = 700;
static const uint32_t POS_CACHE_MS    = 1500;

// Slew rates (Autostar SPEED): Guide, Center, Find, Slew
static const char* const SLEW_RATE_NAMES[] = {"GUIDE", "CENTER", "FIND", "SLEW"};
static const char* const SLEW_RATE_CMDS[]  = {":RG#", ":RC#", ":RM#", ":RS#"};
