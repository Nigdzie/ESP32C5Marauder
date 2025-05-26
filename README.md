# ESP32-C5 Minimal Marauder-Style WiFi AP Scanner (Flipper Zero Ready)

A minimalist CLI WiFi AP scanner inspired by ESP32Marauder, ported to **ESP32-C5** (ESP-IDF 5.x), designed for **Flipper Zero** UART control.  
This project features zero decorations or graphics – just one AP per line, in a format that's perfectly readable on the small Flipper display.

---

## ✨ Features

- **Continuous AP scan (like original Marauder):**
  - Scans WiFi networks in a loop, auto-refreshing the results.
  - Each access point is printed in a single line:  
    `RSSI: -90 Ch: 6 BSSID: 38:43:7d:c7:dd:0f ESSID: Xw2`
- **No client scan** (AP list only – ideal for fast wardriving and debugging).
- **Works with UART/USB and Flipper Zero** – perfectly formatted for Flipper's screen.
- **UART junk filtering** – resistant to noise and malformed commands.
- **No ESP-IDF logs or debug messages on console** – just clean scan results.

---

## 🦑 CLI Commands

After connecting via UART (Flipper Zero, minicom, PuTTY, Arduino Serial Monitor), use these commands:

- `scanap` – start continuous AP scan (refresh every ~1.7 seconds)
- `stopscan` – stop the scan loop
- `help` – show available commands

---

## 🚀 Quick Start (ESP-IDF)

1. Clone the repository:
   ```bash
   git clone https://github.com/Nigdzie/ESP32C5Marauder.git
   cd ESP32C5Marauder
   ```

2. Compile and flash to your ESP32-C5:
   ```bash
   idf.py set-target esp32c5
   idf.py build
   idf.py flash monitor
   ```

3. Connect Flipper Zero to RX/TX/GND (or use any UART terminal).

---

## 🐬 Flipper Zero Integration

- Use the UART app or your own Flipper plugin.
- Enter `scanap` to start a live-updating AP list.
- Output example:
  ```
  RSSI: -91 Ch: 6 BSSID: 68:02:b8:d8:e1:71 ESSID: UPC4372007
  RSSI: -83 Ch: 11 BSSID: 3a:11:49:11:0e:11 ESSID: <hidden>
  ```
- `stopscan` stops the loop (`scanap` can be started again at any time).

---

## 🛠️ Who is this for?

- Anyone who wants to use ESP32-C5 as a "Flipper wardriver" add-on.
- For learning, testing, and fast WiFi AP monitoring.
- As a base for further development (client scan, deauth, beacon spam, etc).

---

## 💡 To-Do / Possible Extensions

- Add client scanning (scansta)
- WiFi attacks (deauth, beacon flood)
- Log saving to SD/Flash
- Paginated output / scrollable list

---

## 📜 License

Open-source, for **educational and development purposes only**.  
Note: Scanning third-party networks without consent may be illegal in your jurisdiction.

---

**Author:** [Nigdzie](https://github.com/Nigdzie)  
**Inspired by:** [ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder)
