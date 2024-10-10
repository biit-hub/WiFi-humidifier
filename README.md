# WiFi-humidifier

## Anleitung
Wenn bereits Logindaten für das Netzwerk eingegeben wurden, zu Schritt 5 springen
1. Mit einem Gerät (PC oder Smartphone) mit dem AccessPoint `humi` mit dem Passwort `password` verbinden.
2. Im Browser die IP-Adresse `192.168.4.1` aufrufen.
3. Logindaten für das Netzwerk eingeben.
4. Gerät neustarten. Das Gerät sollte sich jetzt mit dem Heimnetzwerk verbinden.
5. Im Browser `wifi-humidifier.local` eingeben. Die folgende Webseite wird angezeigt.
![alt text](img/dashboard.png)
6. Unter Settings kann die Zeit gesetzt werden.
![alt text](img/settings.png)

## Hardware
![alt text](img/hardware.png)
1. ESP01-S, Controller inkl. WLAN-Modul
2. DS3231, RTC-Modul
3. LED, simuliert Ausgang
4. Spannungsanschluss 3.3V

## Optimierungen
- NodeMCU (grösserer Controller mit mehr Pins) verwenden um den Controller bei Bedarf auch mit Hardware erweitern können
![alt text](img/nodeMcu.png)