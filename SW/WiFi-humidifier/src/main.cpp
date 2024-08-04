#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <DS3231.h>
#include "wifi_credentials.h" // WLAN-Daten importieren

#define led_pin 1    // TX-Pin (GPIO1)
#define button_pin 3 // RX-Pin (GPIO3)

ESP8266WebServer server(80);
DS3231 rtc;

void handleRoot() {
  String html = "<html>\
  <head>\
    <title>ESP8266 Control</title>\
    <style>\
      body { font-family: Arial, sans-serif; text-align: center; }\
      .button { padding: 10px 20px; font-size: 20px; cursor: pointer; }\
      .on { background-color: #4CAF50; color: white; }\
      .off { background-color: #f44336; color: white; }\
      .header { overflow: hidden; background-color: #f1f1f1; padding: 20px 10px; }\
      .header a { float: left; color: black; text-align: center; padding: 12px; text-decoration: none; font-size: 18px; line-height: 25px; border-radius: 4px; }\
      .header a:hover { background-color: #ddd; color: black; }\
      .header a.active { background-color: #4CAF50; color: white; }\
    </style>\
  </head>\
  <body>\
    <div class=\"header\">\
      <a href=\"/\" class=\"active\">Dashboard</a>\
      <a href=\"/settings\">Settings</a>\
    </div>\
    <h1>ESP8266 LED Control</h1>\
    <p><button class=\"button on\" onclick=\"location.href='/led/on'\">Turn On</button></p>\
    <p><button class=\"button off\" onclick=\"location.href='/led/off'\">Turn Off</button></p>\
  </body>\
  </html>";
  server.send(200, "text/html", html);
}

void handleSettings() {
  String html = "<html>\
  <head>\
    <title>ESP8266 Settings</title>\
    <style>\
      body { font-family: Arial, sans-serif; text-align: center; }\
      .header { overflow: hidden; background-color: #f1f1f1; padding: 20px 10px; }\
      .header a { float: left; color: black; text-align: center; padding: 12px; text-decoration: none; font-size: 18px; line-height: 25px; border-radius: 4px; }\
      .header a:hover { background-color: #ddd; color: black; }\
      .header a.active { background-color: #4CAF50; color: white; }\
      .form { margin: 20px; }\
      input[type=text] { padding: 10px; width: 80%; font-size: 18px; margin: 10px 0; }\
      input[type=submit] { padding: 10px 20px; font-size: 18px; cursor: pointer; background-color: #4CAF50; color: white; border: none; }\
    </style>\
  </head>\
  <body>\
    <div class=\"header\">\
      <a href=\"/\">Dashboard</a>\
      <a href=\"/settings\" class=\"active\">Settings</a>\
    </div>\
    <h1>ESP8266 Settings</h1>\
    <div class=\"form\">\
      <form action=\"/set_time\" method=\"POST\">\
        <label for=\"datetime\">Set Date and Time (YYYY-MM-DD HH:MM:SS):</label><br>\
        <input type=\"text\" id=\"datetime\" name=\"datetime\"><br>\
        <input type=\"submit\" value=\"Set Time\">\
      </form>\
    </div>\
  </body>\
  </html>";
  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (server.hasArg("datetime")) {
    String datetime = server.arg("datetime");
    int year, month, day, hour, minute, second;
    if (sscanf(datetime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
      rtc.setYear(year - 2000); // DS3231 library uses years since 2000
      rtc.setMonth(month);
      rtc.setDate(day);
      rtc.setHour(hour);
      rtc.setMinute(minute);
      rtc.setSecond(second);
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
