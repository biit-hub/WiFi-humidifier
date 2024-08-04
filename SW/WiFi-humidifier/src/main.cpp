#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "wifi_credentials.h" // WLAN-Daten importieren

#define led_pin 1    // TX-Pin (GPIO1)
#define button_pin 3 // RX-Pin (GPIO3)

ESP8266WebServer server(80);

void handleRoot() {
  String html = "<html>\
  <head>\
    <title>ESP8266 LED Control</title>\
    <style>\
      body { font-family: Arial, sans-serif; text-align: center; }\
      .button { padding: 10px 20px; font-size: 20px; cursor: pointer; }\
      .on { background-color: #4CAF50; color: white; }\
      .off { background-color: #f44336; color: white; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 LED Control</h1>\
    <p><button class=\"button on\" onclick=\"location.href='/led/on'\">Turn On</button></p>\
    <p><button class=\"button off\" onclick=\"location.href='/led/off'\">Turn Off</button></p>\
  </body>\
  </html>";
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  digitalWrite(led_pin, LOW); // Turn on the LED
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLEDOff() {
  digitalWrite(led_pin, HIGH); // Turn off the LED
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  // Initialize the TX pin (Onboard LED) as an output
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH); // Turn off the LED at the beginning (active low)

  // Initialize the RX pin (Button) as an input with internal pull-up resistor
  pinMode(button_pin, INPUT_PULLUP);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    yield(); // Watchdog-Timer zurücksetzen
  }

  // OTA setup
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
  });

  ArduinoOTA.onEnd([]() {
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  });

  ArduinoOTA.onError([](ota_error_t error) {
  });

  ArduinoOTA.begin();

  // Start the web server
  server.on("/", handleRoot);
  server.on("/led/on", handleLEDOn);
  server.on("/led/off", handleLEDOff);
  server.begin();
}

void loop() {
  ArduinoOTA.handle(); // Handle OTA updates
  server.handleClient(); // Handle web server

  yield(); // Watchdog-Timer zurücksetzen
}
