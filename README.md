# esp32-monitoring

Minimal IoT monitoring stack for Heltec ESP32-S3 boards. The microcontroller exposes a `/metrics` endpoint in Prometheus format over Wi-Fi, scraped every 10 seconds by a local Prometheus instance and visualized in Grafana — both running via Docker Compose.

## Stack
- **Device** : Heltec WiFi Kit 32 V3 (ESP32-S3) — Arduino firmware
- **Metrics** : chip temperature via `/metrics` (Prometheus text format)
- **Infra** : Prometheus + Grafana on Docker Compose

## Usage
1. Flash `firmware/esp32_metrics.ino` on the board (set your Wi-Fi credentials)
2. `docker compose up -d`
3. Open Grafana at `http://localhost:3000` (admin/admin)
