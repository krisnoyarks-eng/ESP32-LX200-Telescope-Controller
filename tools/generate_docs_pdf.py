#!/usr/bin/env python3
"""Generate wiring + flowchart PDFs for Meade LX200 GPS + Autostar II Wi-Fi handbox."""
from pathlib import Path
from reportlab.lib.pagesizes import letter
from reportlab.lib.units import inch
from reportlab.pdfgen import canvas
from reportlab.lib.colors import HexColor
import math

OUT = Path(__file__).resolve().parents[1] / "docs"
OUT.mkdir(parents=True, exist_ok=True)

BG = HexColor("#0b1220")
PANEL = HexColor("#152238")
ACCENT = HexColor("#5eb1ff")
OK = HexColor("#3dd68c")
WARN = HexColor("#f5c542")
TEXT = HexColor("#e8eefc")
MUTED = HexColor("#8fa0bf")
LINE = HexColor("#3a4d6e")
LCD = HexColor("#7CFF7C")


def rounded_rect(c, x, y, w, h, r=8, fill=PANEL, stroke=LINE):
    c.setFillColor(fill)
    c.setStrokeColor(stroke)
    c.setLineWidth(1.2)
    c.roundRect(x, y, w, h, r, fill=1, stroke=1)


def box(c, x, y, w, h, title, lines, fill=PANEL):
    rounded_rect(c, x, y, w, h, fill=fill)
    c.setFillColor(ACCENT)
    c.setFont("Helvetica-Bold", 11)
    c.drawString(x + 10, y + h - 18, title)
    c.setFillColor(TEXT)
    c.setFont("Helvetica", 9)
    ty = y + h - 34
    for line in lines:
        c.drawString(x + 10, ty, line)
        ty -= 12


def arrow(c, x1, y1, x2, y2, label=""):
    c.setStrokeColor(OK)
    c.setFillColor(OK)
    c.setLineWidth(2)
    c.line(x1, y1, x2, y2)
    ang = math.atan2(y2 - y1, x2 - x1)
    size = 7
    p = c.beginPath()
    p.moveTo(x2, y2)
    p.lineTo(x2 - size * math.cos(ang - 0.4), y2 - size * math.sin(ang - 0.4))
    p.lineTo(x2 - size * math.cos(ang + 0.4), y2 - size * math.sin(ang + 0.4))
    p.close()
    c.drawPath(p, fill=1, stroke=0)
    if label:
        c.setFillColor(MUTED)
        c.setFont("Helvetica", 8)
        c.drawCentredString((x1 + x2) / 2, (y1 + y2) / 2 + 6, label)


def page_header(c, title, subtitle):
    width, height = letter
    c.setFillColor(BG)
    c.rect(0, 0, width, height, fill=1, stroke=0)
    c.setFillColor(ACCENT)
    c.setFont("Helvetica-Bold", 16)
    c.drawString(0.55 * inch, height - 0.55 * inch, title)
    c.setFillColor(MUTED)
    c.setFont("Helvetica", 9)
    c.drawString(0.55 * inch, height - 0.78 * inch, subtitle)


def make_wiring_pdf():
    path = OUT / "WIRING_DIAGRAM.pdf"
    c = canvas.Canvas(str(path), pagesize=letter)
    page_header(
        c,
        "Meade LX200 GPS — ESP32 wiring",
        "Product: LX200 GPS fork mount · Autostar II protocol · RS232 RJ jacks on base panel",
    )

    box(
        c,
        0.5 * inch,
        6.0 * inch,
        2.5 * inch,
        2.5 * inch,
        "ESP32 DevKit",
        [
            "3V3 → OLED + MAX3232*",
            "GND → common ground",
            "GPIO21 → OLED SDA",
            "GPIO22 → OLED SCL",
            "GPIO16 RX2 ← MAX R1OUT",
            "GPIO17 TX2 → MAX T1IN",
            "USB 5V power",
            "*or 5V if MAX needs it",
        ],
    )

    box(
        c,
        3.3 * inch,
        7.0 * inch,
        2.1 * inch,
        1.5 * inch,
        "SSD1306 OLED",
        [
            "VCC→3V3  GND→GND",
            "SDA→GPIO21",
            "SCL→GPIO22",
            "Shows Autostar LCD",
        ],
        fill=HexColor("#1a2a18"),
    )

    box(
        c,
        3.3 * inch,
        5.0 * inch,
        2.2 * inch,
        1.8 * inch,
        "MAX3232 module",
        [
            "TTL side ↔ ESP32",
            "RS232 side ↔ RJ cable",
            "Levels: ±RS232 safe",
            "Never skip this chip",
        ],
        fill=HexColor("#2a2010"),
    )

    box(
        c,
        5.8 * inch,
        5.0 * inch,
        2.0 * inch,
        2.5 * inch,
        "LX200 GPS base",
        [
            "Use RS232 jack",
            "(NOT the HBX port)",
            "RJ11 / RJ12 plug",
            "9600 8N1",
            "Autostar II cmds",
            "Power LED on = OK",
            "12VDC telescope PSU",
        ],
        fill=HexColor("#201828"),
    )

    arrow(c, 3.0 * inch, 7.6 * inch, 3.3 * inch, 7.6 * inch, "I2C")
    arrow(c, 3.0 * inch, 6.0 * inch, 3.3 * inch, 6.0 * inch, "TTL")
    arrow(c, 5.5 * inch, 5.9 * inch, 5.8 * inch, 5.9 * inch, "RS232")

    # RJ pinout table — Meade #507 style
    rounded_rect(c, 0.5 * inch, 2.15 * inch, 7.3 * inch, 2.6 * inch)
    c.setFillColor(ACCENT)
    c.setFont("Helvetica-Bold", 12)
    c.drawString(0.7 * inch, 4.45 * inch, "Meade #507-style RS232 cable (RJ11 ↔ MAX3232)")
    c.setFillColor(TEXT)
    c.setFont("Courier", 8.5)
    rows = [
        "Looking into LX200 GPS RS232 jack (clip/tab down, pins 1..6 left→right):",
        "",
        "RJ pin   Signal on mount          Wire to MAX3232 RS232 side",
        "------   -----------------------  --------------------------------",
        "1,2,6    unused                   leave open",
        "3        RX into telescope        ← MAX3232 T1OUT (RS232 TX from ESP)",
        "4        GND                      ↔ MAX3232 GND",
        "5        TX from telescope        → MAX3232 R1IN  (RS232 RX to ESP)",
        "",
        "TTL side of MAX3232: T1IN←ESP32 GPIO17   R1OUT→ESP32 GPIO16   VCC/GND",
        "Handbox Autostar II stays on HBX if you want — ESP32 uses RS232 only.",
    ]
    y = 4.2 * inch
    for row in rows:
        c.drawString(0.7 * inch, y, row)
        y -= 11

    c.setFillColor(WARN)
    c.setFont("Helvetica-Bold", 9)
    c.drawString(
        0.5 * inch,
        1.7 * inch,
        "CRITICAL: Plug into the jack labeled RS232 — do NOT use HBX for this ESP32 link.",
    )
    c.setFillColor(MUTED)
    c.setFont("Helvetica", 8)
    c.drawString(
        0.5 * inch,
        1.4 * inch,
        "HBX is the coiled Autostar II handbox bus. RS232 is the computer / LX200 command port.",
    )
    c.drawString(
        0.5 * inch,
        1.15 * inch,
        "If timeouts: swap RJ pins 3↔5 once, confirm 9600 baud, mount powered (red LED on).",
    )
    c.drawString(
        0.5 * inch,
        0.7 * inch,
        "Compatible product: Meade LX200 GPS (Autostar II / Smart Mount Technology base panel).",
    )
    c.showPage()

    page_header(c, "Bring-up checklist — LX200 GPS", "After wiring MAX3232 + RJ into RS232")
    box(
        c,
        0.5 * inch,
        5.4 * inch,
        7.3 * inch,
        2.8 * inch,
        "Power & ports on the fork base (from your photos)",
        [
            "OFF/ON rocker · 12VDC · RETICLE · HBX · RS232 · RS232 · AUTOGUIDER …",
            "1. Power mount 12V — red LED on control panel.",
            "2. Insert RJ into either RS232 jack (both speak LX200 serial).",
            "3. Leave HBX free OR plug physical Autostar II there (optional).",
            "4. Flash TelescopeController.ino · Serial 115200 shows AP IP.",
            "5. Phone → Wi-Fi ESP32_Telescope_AP / telescopepassword",
            "6. Browser http://192.168.4.1 — Autostar II virtual handbox",
            "7. Tap Get RA / Get Dec — reply prints on green LCD + raw box",
        ],
    )
    box(
        c,
        0.5 * inch,
        2.4 * inch,
        7.3 * inch,
        2.7 * inch,
        "Autostar II keys → LX200 commands",
        [
            "▲▼◄►  = :Mn# :Ms# :Mw# :Me#   (hold) · release sends :Qn# etc.",
            "GO TO  = set RA/Dec of library object + :MS# (+ tracking :Te#)",
            "SPEED  = cycle :RG# :RC# :RM# :RS#  (Guide/Center/Find/Slew)",
            "Get RA / Get Dec = :GR# / :GD#   Ver = :GV#   STOP = :Q#",
            "SS / STAR / M / NGC = filter object library on the webpage",
            "ENTER = load target · MODE = cycle RA/Dec · Target · Alt/Az · Link",
        ],
    )
    c.setFillColor(MUTED)
    c.setFont("Helvetica", 8)
    c.drawString(0.5 * inch, 1.0 * inch, "DEMO_FALLBACK=1: UI works without a mount for bench testing the handbox screen.")
    c.save()
    print("Wrote", path)


def flow_box(c, x, y, w, h, text, fill=PANEL):
    rounded_rect(c, x, y, w, h, r=6, fill=fill)
    c.setFillColor(TEXT)
    c.setFont("Helvetica", 8)
    words = text.split()
    lines, cur = [], ""
    for wrd in words:
        trial = (cur + " " + wrd).strip()
        if c.stringWidth(trial, "Helvetica", 8) < w - 12:
            cur = trial
        else:
            lines.append(cur)
            cur = wrd
    if cur:
        lines.append(cur)
    ty = y + h / 2 + 4 * (len(lines) - 1)
    for line in lines[:4]:
        c.drawCentredString(x + w / 2, ty, line)
        ty -= 10


def make_flowchart_pdf():
    path = OUT / "SYSTEM_FLOWCHARTS.pdf"
    c = canvas.Canvas(str(path), pagesize=letter)

    page_header(
        c,
        "LX200 GPS · Autostar II Wi-Fi system flow",
        "Virtual handbox + OLED mirror the physical Autostar II workflow",
    )
    flow_box(c, 2.6 * inch, 8.5 * inch, 2.8 * inch, 0.55 * inch, "Phone browser Autostar II UI", HexColor("#1a3048"))
    arrow(c, 4.0 * inch, 8.5 * inch, 4.0 * inch, 8.05 * inch, "Wi-Fi AP")
    flow_box(c, 2.3 * inch, 7.3 * inch, 3.4 * inch, 0.7 * inch, "ESP32 WebServer · LX200 cmd translate", HexColor("#152850"))
    arrow(c, 2.8 * inch, 7.3 * inch, 1.8 * inch, 6.55 * inch)
    arrow(c, 5.2 * inch, 7.3 * inch, 6.2 * inch, 6.55 * inch)
    flow_box(c, 0.6 * inch, 5.8 * inch, 2.3 * inch, 0.7 * inch, "OLED Autostar 2-line LCD", HexColor("#1a2a18"))
    flow_box(c, 5.2 * inch, 5.8 * inch, 2.4 * inch, 0.7 * inch, "UART2 → MAX3232", HexColor("#2a2010"))
    arrow(c, 6.4 * inch, 5.8 * inch, 6.4 * inch, 5.1 * inch, "RJ RS232")
    flow_box(c, 5.1 * inch, 4.3 * inch, 2.6 * inch, 0.75 * inch, "LX200 GPS Smart Mount RS232", HexColor("#201828"))
    flow_box(c, 0.6 * inch, 4.3 * inch, 2.3 * inch, 0.75 * inch, "Celestial library SS/STAR/M", HexColor("#241838"))
    arrow(c, 2.9 * inch, 4.65 * inch, 5.1 * inch, 4.65 * inch, "GO TO")

    box(
        c,
        0.5 * inch,
        1.3 * inch,
        7.3 * inch,
        2.6 * inch,
        "Instant monitor path (Get RA / Get Dec)",
        [
            "1. Tap Get Right Ascension on virtual handbox",
            "2. Browser fetch /execute?cmd=:GR#  over Wi-Fi",
            "3. ESP32 writes :GR# on UART → MAX3232 → RS232 jack",
            "4. Mount replies e.g. 05:32:00#",
            "5. ESP32 returns text → green LCD + raw response box update instantly",
            "6. Same string mirrored on physical OLED Autostar-style panel",
        ],
    )
    c.showPage()

    page_header(c, "GO TO + Track (Autostar II)", "ENTER loads target · GO TO slews like the handbox")
    steps = [
        (8.5, "Pick object (SS / STAR / M) · SCROLL · ENTER"),
        (7.7, "POST /api/goto → compute RA/Dec (Sun/Moon/planets use site time)"),
        (6.9, ":SrHH:MM:SS#  and  :SdsDD*MM:SS#"),
        (6.1, ":MS# slew  (0=ok 1=horizon 2=not aligned)"),
        (5.3, ":Te# tracking ON (auto-follow)"),
        (4.5, "LCD shows GO TO · OLED updates · raw box prints reply"),
        (3.7, "SPEED changes rate · ▲▼◄► jog · STOP = :Q#"),
    ]
    for y, text in steps:
        flow_box(c, 1.3 * inch, y * inch - 0.15 * inch, 5.8 * inch, 0.5 * inch, text)
        if y > 3.7:
            arrow(c, 4.2 * inch, (y - 0.15) * inch, 4.2 * inch, (y - 0.65) * inch)

    box(
        c,
        0.5 * inch,
        1.2 * inch,
        7.3 * inch,
        2.0 * inch,
        "Product match checklist",
        [
            "✓ Meade LX200 GPS OTA tube + dual fork (your photos)",
            "✓ Base panel: HBX + dual RS232 RJ + Autoguider + Reticle",
            "✓ Protocol: Autostar II / LX200 classic ASCII (:cmd#)",
            "✓ Virtual UI: ENTER · MODE · GO TO · D-pad · SPEED/CALD/M/FOCUS/SS/STAR/RET/IC/NGC",
        ],
    )
    c.save()
    print("Wrote", path)


if __name__ == "__main__":
    make_wiring_pdf()
    make_flowchart_pdf()
