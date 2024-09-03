# WiFi-humidifier

## Anleitung
1. Im Ordner `include` eine Datei mit dem Namen `wifi_credentials.h` und mit folgendem Inhalt erstellen. Die SSID und das Passwort anpassen.
```c
#ifndef WIFI_CREDENTIALS_H
#define WIFI_CREDENTIALS_H

const char* ssid = "ssid";      // set correct ssid
const char* password = "pass";  // set correct password

#endif // WIFI_CREDENTIALS_H
```
2. Programm auf den Controller laden
3. Die IP-Adresse des Controllers herausfinden. Am besten über den Router.
4. Die IP-Adresse des Controllers im Browser eingeben. Die folgende Webseite wird angezeigt.
![alt text](img/dashboard.png)
5. Unter Settings kann die Zeit gesetzt werden.
![alt text](img/settings.png)

## Hardware
![alt text](img/hardware.png)
1. ESP01-S, Controller inkl. WLAN-Modul
2. DS3231, RTC-Modul
3. LED, simuliert Ausgang
4. Spannungsanschluss 3.3V

## Optimierungen
- Besseres Anmelden im Netzwerk --> Access Point erstellen, wenn keine WLAN-Credentials verfügbar sind
- WLAN Credentials auf dem Chip speichern
- `ESP8266mDNS`-Bibliothek verwenden, damit der Webserver über einen Namen aufgerufen werden kann
- NodeMCU (grösserer Controller mit mehr Pins) verwenden um den Controller bei Bedarf auch mit Hardware erweitern können
![alt text](img/nodeMcu.png)