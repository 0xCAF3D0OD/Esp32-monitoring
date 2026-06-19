#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include "secret.h"
#include "SSD1306Wire.h"

#define SDA_OLED 17
#define SCL_OLED 18
#define RST_OLED 21
#define VEXT     36
#define LED_PIN  35

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

WebServer server(80);

SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, GEOMETRY_128_64);

void initDisplay() {
  pinMode(VEXT, OUTPUT);
  digitalWrite(VEXT, LOW);
  delay(100);
  pinMode(RST_OLED, OUTPUT);
  digitalWrite(RST_OLED, LOW);
  delay(50);
  digitalWrite(RST_OLED, HIGH);
  display.init();
  display.flipScreenVertically();
}

void showIP(String ip) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 10, "WiFi connecte !");
  display.drawString(64, 30, ip);
  display.display();
}

void handleLedOn() {
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/html", "<a href='/off'>Eteindre LED</a>");
}

void handleLedOff() {
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/html", "<a href='/on'>Allumer LED</a>");
}

void handleTemp() {
  float temp = temperatureRead();
  String json = "{\"temperature\": " + String(temp, 1) + "}";
  server.send(200, "application/json", json);
}

void handleMetrics() {
  float temp = temperatureRead();
  String metrics = "";
  metrics += "# HELP esp32_chip_temperature Temperature interne chip\n";
  metrics += "# TYPE esp32_chip_temperature gauge\n";
  metrics += "esp32_chip_temperature " + String(temp, 1) + "\n";
  server.send(200, "text/plain; version=0.0.4", metrics);
}

void handleDashboard() {
  String html = R"(
    <html>
    <body>
      <h1>ESP32 Dashboard</h1>
      <p>Temperature chip : <span id='temp'>...</span> C</p>
      <script>
        setInterval(() => {
          fetch('/temp')
            .then(r => r.json())
            .then(d => document.getElementById('temp').innerText = d.temperature);
        }, 2000);
      </script>
    </body>
    </html>
  )";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  initDisplay();

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 24, "Connexion WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  String ip = WiFi.localIP().toString();
  Serial.println("\nWiFi connecte. IP : " + ip);
  server.on("/", handleDashboard);
  server.on("/temp", handleTemp);
  server.on("/metrics", handleMetrics);
  server.on("/on", handleLedOn);
  server.on("/off", handleLedOff);
  server.begin();
  showIP(ip);  // affiche l'IP sur l'OLED une seule fois
}

void loop() {
  server.handleClient();  // obligatoire, gère les connexions entrantes
}
