#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <DS3231.h>
#include <FS.h>
#include "wifi_credentials.h" // WLAN-Daten importieren

#define led_pin 1    // TX-Pin (GPIO1)
#define button_pin 3 // RX-Pin (GPIO3)

ESP8266WebServer server(80);
DS3231 rtc;

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
  String html = readHTMLFile("/settings.html");
  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (server.hasArg("datetime")) {
    String datetime = server.arg("datetime");
    int year, month, day, hour, minute, second;
    if (sscanf(datetime.c_str(), "%d-%d-%dT%d:%d", &year, &month, &day, &hour, &minute) == 5) {
      rtc.setYear(year - 2000); // DS3231 library uses years since 2000
      rtc.setMonth(month);
      rtc.setDate(day);
      rtc.setHour(hour);
      rtc.setMinute(minute);
      rtc.setSecond(0); // Set seconds to 0 since datetime-local input does not provide seconds
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
  Wire.begin();

  // Check if the RTC has lost power
  if (rtc.getYear() == 85) { // Default year is 2000 + 85 = 2085
    // Set the RTC to the date & time this sketch was compiled
    rtc.setYear(2023 - 2000);   // Set year to 2023
    rtc.setMonth(8);            // Set month to August
    rtc.setDate(6);             // Set day to 6
    rtc.setHour(12);            // Set hour to 12
    rtc.setMinute(0);           // Set minute to 0
    rtc.setSecond(0);           // Set second to 0
  }

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
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
