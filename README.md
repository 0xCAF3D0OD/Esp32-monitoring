# Esp32-monitoring
Minimal IoT monitoring stack for Heltec ESP32-S3 boards. The microcontroller exposes a `/metrics` endpoint in Prometheus format over Wi-Fi, scraped every 10 seconds by a local Prometheus instance and visualized in Grafana — both running via Docker Compose.
