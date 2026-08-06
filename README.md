# ESP32 Autostar II — Meade LX200 GPS Wi‑Fi Controller

Replace (or mirror) the physical **Meade Autostar II** handbox with an **ESP32** soft‑AP web UI and the same **SH1106 OLED** used by the AdhanOLED clock project.

Connect your phone to the ESP32 Wi‑Fi network, open a browser, set your **start location**, pick Sun / Moon / planets / stars / Messier objects, and **GO TO** — commands go out the mount’s **RS232** jack via a MAX3232 level shifter.

---

## What this is for

- **Telescope:** Meade **LX200 GPS** (Autostar II / Smart Mount Technology base panel)
- **Goal:** Wireless hand controller + live RA/Dec monitoring when the physical Autostar II is missing or inconvenient
- **Protocol:** Classic Meade LX200 / Autostar II serial (`:GR#`, `:GD#`, `:MS#`, …) at **9600 8N1**
- **Screens:**
  - Webpage Autostar II lookalike (ENTER · MODE · GO TO · D‑pad · SPEED/SS/STAR/M/…)
  - Physical OLED mirrors the 2‑line Autostar LCD

## Features

- Soft‑AP web dashboard at `http://192.168.4.1`
- **First‑boot setup wizard** asks for start location (lat/lon/date/time) and pushes site/time to the mount
- Object library: Sun, Moon, planets, bright stars, Messier / deep‑sky
- Goto + sidereal tracking, slew‑rate SPEED key, N/S/E/W jog, STOP
- Instant command/response on the green LCD (`Get RA` / `Get Dec` → Wi‑Fi → ESP32 → RS232 → reply)
- DEMO mode works without a mount attached (UI testing)
- Wiring diagrams + flowcharts (PDF) and an offline Chrome HTML preview

## Hardware

| Part | Notes |
|------|--------|
| ESP32 Dev Module | Soft‑AP + web server |
| SH1106 128×64 I2C OLED | Same as AdhanOLED — GPIO **21** SDA, **22** SCL |
| MAX3232 TTL↔RS232 | **Required** — never wire RS232 levels to ESP32 GPIO |
| RJ11/RJ12 cable | Meade **#507‑style** into base jack labeled **RS232** (not **HBX**) |

### ESP32 pin map

| ESP32 | Goes to |
|-------|---------|
| GPIO21 | OLED SDA |
| GPIO22 | OLED SCL |
| 3V3 / GND | OLED (+ MAX3232 if 3.3 V capable) |
| GPIO17 (TX2) | MAX3232 **T1IN** |
| GPIO16 (RX2) | MAX3232 **R1OUT** |
| MAX3232 RS232 | LX200 GPS **RS232** RJ — pin **3** RX‑in, **4** GND, **5** TX‑out |

## How to use

1. Install **Arduino IDE** + ESP32 board support + library **U8g2**
2. Open `TelescopeController/TelescopeController.ino`
3. Board: **ESP32 Dev Module** → Upload
4. Join Wi‑Fi **`ESP32_Telescope_AP`** / password **`telescopepassword`**
5. Browser → **http://192.168.4.1**
6. Complete **Setup · Start location**
7. Pick an object → **ENTER** → **GO TO** (or tap Get RA / Get Dec to monitor)

## Chrome preview (no hardware)

Open [`docs/preview/AutostarII_Preview.html`](docs/preview/AutostarII_Preview.html) in Chrome to see the Autostar UI, setup wizard, and OLED mock offline.

## Docs

| File | Contents |
|------|----------|
| [`docs/WIRING.md`](docs/WIRING.md) | GPIO + RS232 jack wiring |
| [`docs/WIRING_DIAGRAM.pdf`](docs/WIRING_DIAGRAM.pdf) | Wiring PDF |
| [`docs/SYSTEM_FLOWCHARTS.pdf`](docs/SYSTEM_FLOWCHARTS.pdf) | System / GO TO flowcharts |
| [`docs/HOW_IT_WORKS.md`](docs/HOW_IT_WORKS.md) | Protocol & architecture |

## Project layout

```
TelescopeController/     Arduino sketch (open the .ino)
docs/                    Wiring, PDFs, Chrome preview
tools/                   PDF generator script
```

## License

Use freely for personal / educational astronomy projects. Meade, Autostar, and LX200 are trademarks of their respective owners; this is an independent hobby controller, not an official Meade product.
