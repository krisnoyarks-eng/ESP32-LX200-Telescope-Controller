# Wiring — Meade LX200 GPS + ESP32 (same OLED as AdhanOLED)

## Product

- Telescope: **Meade LX200 GPS** (Autostar II base panel)
- Serial jack: labeled **RS232** (RJ11/RJ12) — **never HBX**
- Screen: **SH1106 128×64 I2C** — same module/pins as `Arduino/AdhanOLED`

## ESP32 GPIO map

| ESP32 pin | Direction | Device | Device pin |
|-----------|-----------|--------|------------|
| **GPIO21** | SDA | OLED (AdhanOLED) | SDA |
| **GPIO22** | SCL | OLED (AdhanOLED) | SCL |
| **3V3** | power | OLED VCC | VCC |
| **GND** | ground | OLED GND | GND |
| **GPIO17** (TX2) | out | MAX3232 | **T1IN** |
| **GPIO16** (RX2) | in | MAX3232 | **R1OUT** |
| **3V3 or 5V** | power | MAX3232 VCC | (check module) |
| **GND** | ground | MAX3232 GND | GND |
| USB 5V | power | ESP32 board | — |

`OLED_IS_SH1106=1` in `config.h` matches AdhanOLED. If your panel is a true SSD1306, set it to `0`.

## Telescope RS232 port (base panel)

From your photos the fork base has:

`OFF/ON · 12VDC · RETICLE · HBX · RS232 · RS232 · AUTOGUIDER …`

| Jack | Use with this project |
|------|------------------------|
| **RS232** | **YES** — plug the ESP32 MAX3232 cable here |
| **HBX** | NO — reserved for physical Autostar II coiled cable |
| RETICLE / AUTOGUIDER | accessories only |

### RJ11 pinout (#507 style)

Looking into the **RS232** jack (clip/tab down, pins 1→6 left to right):

| RJ pin | Mount signal | Wire to MAX3232 RS232 side |
|--------|--------------|----------------------------|
| 1, 2, 6 | unused | leave open |
| **3** | RX into telescope | **T1OUT** (from ESP TX) |
| **4** | GND | GND |
| **5** | TX from telescope | **R1IN** (to ESP RX) |

Baud: **9600 8N1**.

```
ESP32 GPIO17 ──► MAX3232 T1IN ──(level shift)── T1OUT ──► RJ pin 3 (scope RX)
ESP32 GPIO16 ◄── MAX3232 R1OUT ◄──(level shift)── R1IN  ◄── RJ pin 5 (scope TX)
ESP32 GND    ◄──────────────────────────────────────────► RJ pin 4
```

## First power-on (start location)

Like Autostar II, the webpage **forces a Setup wizard** until you enter:

- Latitude / longitude (telescope sitting location)
- Local date & time + UTC offset

Those values are saved in ESP32 flash and pushed to the mount with `:St` `:Sg` `:SL` `:SC` `:SG`.

## Libraries (Arduino IDE)

- **U8g2** (same as AdhanOLED) — not Adafruit SSD1306 anymore  
- Built-in: WiFi, WebServer, Preferences, Wire  

## PDFs

- `WIRING_DIAGRAM.pdf`
- `SYSTEM_FLOWCHARTS.pdf`
- Chrome preview: `preview/AutostarII_Preview.html`
