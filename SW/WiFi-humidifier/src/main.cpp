#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <FS.h>
#include "wifi_credentials.h" // WLAN-Daten importieren

#define led_pin 1    // TX-Pin (GPIO1)
#define button_pin 3 // RX-Pin (GPIO3)

ESP8266WebServer server(80);
RTC_DS3231 rtc;

String readHTMLFile(const char* path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    return String("Failed to open file");
  }
  String content = file.readString();
  file.close();
  return content;
}

void handleRoot() {
  String html = readHTMLFile("/index.html");
  server.send(200, "text/html", html);
}

void handleSettings() {
  DateTime now = rtc.now();
  char currentDateTime[25];
  sprintf(currentDateTime, "%04d-%02d-%02dT%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute());
  String html = readHTMLFile("/settings.html");
  html.replace("{{datetime}}", String(currentDateTime));
  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (server.hasArg("datetime")) {
    String datetime = server.arg("datetime");
    int year, month, day, hour, minute;
    if (sscanf(datetime.c_str(), "%d-%d-%dT%d:%d", &year, &month, &day, &hour, &minute) == 5) {
      rtc.adjust(DateTime(year, month, day, hour, minute, 0));
    }
  }
  server.sendHeader("Location", "/settings");
  server.send(303);
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

  // Initialize RTC
  Wire.begin(0, 2);
  if (!rtc.begin()) {
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    return;
  }

  // Start the web server
  server.on("/", handleRoot);
  server.on("/settings", handleSettings);
  server.on("/set_time", HTTP_POST, handleSetTime);
  server.on("/led/on", handleLEDOn);
  server.on("/led/off", handleLEDOff);
  server.begin();
}

void loop() {
  ArduinoOTA.handle(); // Handle OTA updates
  server.handleClient(); // Handle web server

  yield(); // Watchdog-Timer zurücksetzen
}
