#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "wifi_credentials.h" // WLAN-Daten importieren

#define led_pin 2    // TX-Pin (GPIO1)
#define button_pin 3 // RX-Pin (GPIO3)

ESP8266WebServer server(80);
RTC_DS3231 rtc;
bool rtcConnected = true;

struct Timer {
  String days;
  String time;
  int duration;
};

std::vector<Timer> timers;
unsigned long ledOffTime = 0;

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
  String timerListHtml;
  for (size_t i = 0; i < timers.size(); i++) {
    timerListHtml += "<tr><td>" + timers[i].days + "</td><td>" + timers[i].time + "</td><td>" + String(timers[i].duration) + "</td><td><button onclick=\"location.href='/delete_timer?index=" + String(i) + "'\">Delete</button></td></tr>";
  }
  html.replace("<!-- Timer entries will be dynamically inserted here -->", timerListHtml);
  server.send(200, "text/html", html);
}

void handleSettings() {
  String html = readHTMLFile("/settings.html");

  if (rtcConnected) {
    DateTime now = rtc.now();
    char currentDate[11];
    char currentTime[6];
    sprintf(currentDate, "%02d.%02d.%04d", now.day(), now.month(), now.year());
    sprintf(currentTime, "%02d:%02d", now.hour(), now.minute());
    html.replace("{{datetime}}", String(currentDate) + " " + String(currentTime));
    html.replace("{{current_date}}", String(currentDate));
    html.replace("{{current_time}}", String(currentTime));
    html.replace("{{disable_controls}}", "");
  } else {
    html.replace("{{datetime}}", "RTC not connected");
    html.replace("{{current_date}}", "");
    html.replace("{{current_time}}", "");
    html.replace("{{disable_controls}}", "disabled");
  }

  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (rtcConnected && server.hasArg("date") && server.hasArg("time")) {
    String date = server.arg("date");
    String time = server.arg("time");
    int day, month, year, hour, minute;
    if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) == 3 && sscanf(time.c_str(), "%d:%d", &hour, &minute) == 2) {
      rtc.adjust(DateTime(year, month, day, hour, minute, 0));
    }
  }
  server.sendHeader("Location", "/settings");
  server.send(303);
}

void handleAddTimer() {
  if (server.hasArg("days") && server.hasArg("time") && server.hasArg("duration")) {
    Timer newTimer = {server.arg("days"), server.arg("time"), server.arg("duration").toInt()};
    timers.push_back(newTimer);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDeleteTimer() {
  if (server.hasArg("index")) {
    int index = server.arg("index").toInt();
    if (index >= 0 && index < timers.size()) {
      timers.erase(timers.begin() + index);
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleLEDOn() {
  digitalWrite(led_pin, LOW); // Turn on the LED
  ledOffTime = millis() + timers[0].duration * 60000;
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
    rtcConnected = false;
  } else if (rtc.lostPower()) {
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
  server.on("/add_timer", HTTP_POST, handleAddTimer);
  server.on("/delete_timer", handleDeleteTimer);
  server.on("/led/on", handleLEDOn);
  server.on("/led/off", handleLEDOff);
  server.begin();
}

void loop() {
  ArduinoOTA.handle(); // Handle OTA updates
  server.handleClient(); // Handle web server

  if (rtcConnected) {
    DateTime now = rtc.now();
    for (auto& timer : timers) {
      int hour, minute;
      sscanf(timer.time.c_str(), "%d:%d", &hour, &minute);
      if (now.hour() == hour && now.minute() == minute && now.second() == 0) {
        // Check if today is one of the specified days
        String day = String(now.dayOfTheWeek() + 1); // Convert to 1-7 (Sun-Sat)
        if (timer.days.indexOf(day) >= 0) {
          digitalWrite(led_pin, LOW); // Turn on the LED
          ledOffTime = millis() + timer.duration * 60000;
        }
      }
    }
  }

  if (ledOffTime > 0 && millis() > ledOffTime) {
    digitalWrite(led_pin, HIGH); // Turn off the LED
    ledOffTime = 0;
  }

  yield(); // Watchdog-Timer zurücksetzen
}
