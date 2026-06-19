
## Usage
1. Flash `firmware/esp32_metrics.ino` on the board (set your Wi-Fi credentials)
2. `docker compose up -d`
3. Open Grafana at `http://localhost:3000` (admin/admin)


# esp32-monitoring

A minimal IoT monitoring stack: a Heltec ESP32-S3 exposes its metrics over Wi-Fi in Prometheus format, scraped and visualized via Prometheus + Grafana running locally (Docker Compose).

```
ESP32-S3 (Arduino firmware) --Wi-Fi--> /metrics --> Prometheus --> Grafana
```

## Hardware

- **Device** : Heltec WiFi Kit 32 V3 (ESP32-S3) — Arduino firmware
- USB-C cable **with data transfer** (some cables are charge-only)

## Software stack

| Component | Role |
|---|---|
| Arduino IDE | Compiling and flashing the firmware |
| `WebServer.h` | HTTP server running on the ESP32 |
| `SSD1306Wire` | OLED screen driver |
| Prometheus | Scrapes `/metrics` every 10s |
| Grafana | Metrics visualization |
| Docker Compose | Local orchestration of Prometheus + Grafana |

## Setup

### 1. Firmware

1. Install the ESP32 board package in Arduino IDE (`espressif/arduino-esp32`).
2. Install the **ESP8266 and ESP32 OLED driver for SSD1306** library (ThingPulse).
3. Create `secret.h` **in the same folder as the `.ino` file**:
   ```cpp
   #define WIFI_SSID "your_ssid"
   #define WIFI_PASSWORD "your_password"
   ```
4. Select the **Heltec WiFi Kit 32 V3** board (or *ESP32S3 Dev Module*) and the correct port (`Tools > Port`).
5. Flash (`secret.h` is gitignored, never commit it).

Endpoints exposed by the board:
- `/` — simple HTML dashboard
- `/temp` — chip temperature as JSON
- `/metrics` — Prometheus format
- `/on`, `/off` — LED control

### 2. Monitoring stack

```bash
cp .env.example .env   # set GF_ADMIN_PASSWORD
docker compose up -d
```

- Prometheus: `http://localhost:9090`
- Grafana: `http://localhost:3000` (login `admin` / password from `.env`)

Configure `prometheus/prometheus.yml` with the board's IP (shown on the OLED once connected to Wi-Fi):
```yaml
scrape_configs:
  - job_name: 'esp32'
    static_configs:
      - targets: ['<BOARD_IP>:80']
    metrics_path: '/metrics'
```

In Grafana: `Connections > Data sources > Add Prometheus`, URL `http://prometheus:9090`, check **Default**.

## OLED / LED pinout

| Pin | GPIO |
|---|---|
| SDA | 17 |
| SCL | 18 |
| RST | 21 |
| Vext | 36 |
| Built-in LED | 35 |

## Issues encountered

Documented here because they're non-obvious and easy to hit again on this exact hardware.

**USB-serial driver** — on some Macs the `/dev/cu.usbserial-xxxx` port shows up natively, on others the Silicon Labs CP210x driver needs to be installed and manually approved under *System Settings > Privacy & Security*. Check this first with `ls /dev/cu.*` before attempting to flash.

**`BluetoothSerial` fails to compile** — this library only implements Bluetooth Classic (SPP), available solely on the original ESP32. The ESP32-S3 only supports BLE. Use `BLESerial` or a native BLE library if Bluetooth is needed.

**`temprature_sens_read()` undefined at link time** — legacy function limited to the original ESP32. On ESP32-S3, use `temperatureRead()` instead (available natively, no extra include required).

**`Resource busy` on flash** — the serial port can only be used by one program at a time. An open `screen` session (even closed via the window's X button instead of `Ctrl+A` then `K`) or the IDE's serial monitor can keep the port locked. Check with `lsof /dev/cu.usbserial-xxxx` and `kill -9 <PID>` if needed.

**`no upload port provided`** — compilation succeeds but the port isn't selected under `Tools > Port`, or the board isn't detected by macOS (often a charge-only USB-C cable).

**Imported Grafana dashboard shows no data** — the community dashboard *Node Exporter Full* (ID `1860`) expects metrics from `node_exporter` (CPU, RAM, disk of an actual Linux machine). A custom metric like `esp32_chip_temperature` matches none of its queries: `No data` is expected here, not a bug to fix. Build a dedicated panel with its own query to see actual data.

**No datasource selector on import** — some newer dashboards no longer declare an explicit datasource variable in their JSON; Grafana automatically applies whichever datasource is marked **Default**. Make sure a Prometheus datasource is set as default before importing.

## License

MIT