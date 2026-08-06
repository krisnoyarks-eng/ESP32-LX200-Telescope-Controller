# How it works — LX200 GPS Autostar II Wi-Fi handbox

## Matched product

This firmware targets the **Meade LX200 GPS** fork mount with the base panel that includes:

- OFF/ON · 12VDC · RETICLE · **HBX** · **RS232** · **RS232** · AUTOGUIDER  
- Protocol: **Autostar II / Meade LX200** ASCII commands (`:…#`) at 9600 baud

The webpage is laid out like the physical **Autostar II** handbox: green LCD, ENTER / MODE / GO TO, D-pad, and SPEED / CALD / M / FOCUS / SS / STAR / RET / IC / NGC keys.

## Instant control & monitor

1. Tap **Get RA** or **Get Dec** (or any key that sends a command)  
2. Browser → ESP32 over Wi-Fi (`/execute?cmd=…`)  
3. ESP32 forwards the LX200 string through MAX3232 into the mount **RS232** jack  
4. Mount reply (e.g. `05:32:00#`) is returned immediately  
5. Shown on:
   - Virtual green **Autostar LCD**
   - **Raw response** panel
   - Physical **OLED** (2-line Autostar-style bezel)

## Autostar key mapping

| Handbox key | Action |
|-------------|--------|
| ENTER | Load selected library object as target |
| MODE | Cycle RA/Dec · Target · Alt/Az · Link |
| GO TO | `:Sr` / `:Sd` / `:MS#` + tracking `:Te#` |
| ▲▼◄► | `:Mn#` `:Ms#` `:Mw#` `:Me#` (hold); halt on release |
| SPEED | Cycle `:RG#` `:RC#` `:RM#` `:RS#` |
| SS / STAR / M / … | Filter object library |
| STOP | `:Q#` |
| Get RA / Dec | `:GR#` / `:GD#` |

## Object library

Sun, Moon, planets (ephemeris), bright stars, Messier/deep-sky — same catalog as before. Set site lat/lon/time before GO TO on moving objects.

## Files

- `TelescopeController.ino` — AP + loop  
- `web_ui.cpp` — Autostar II HTML + APIs  
- `telescope.cpp` — LX200 serial  
- `oled_screen.cpp` — local LCD mirror  
- `docs/SYSTEM_FLOWCHARTS.pdf` · `WIRING_DIAGRAM.pdf`
