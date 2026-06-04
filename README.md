<div align="center">

# 🌱 FloraHub

### Smart Plant Monitoring & Automated Watering System

![ESP32](https://img.shields.io/badge/ESP32-IoT-blue?style=for-the-badge)
![Arduino](https://img.shields.io/badge/Arduino-C++-00979D?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Completed-success?style=for-the-badge)

An IoT-based smart plant care system that monitors soil moisture, temperature, humidity, and water levels while automatically watering plants when needed.

</div>

---

## 📌 Project Overview

FloraHub is an ESP32-powered smart irrigation system designed to reduce manual plant maintenance. It continuously monitors environmental conditions, automates watering, alerts users when the water tank is low, and provides real-time monitoring through a web dashboard.

### Key Features

- 🌿 Automatic plant watering
- 💧 Water tank level monitoring
- 🌡️ Temperature & humidity monitoring
- 📟 OLED status display
- 👀 Interactive OLED eye animations
- 🌐 ESP32 Web Dashboard
- 🔔 Low water level buzzer alert
- 📶 Automatic Wi-Fi reconnection

---

## 🛠 Tech Stack

| Category | Technology |
|-----------|------------|
| Microcontroller | ESP32 |
| Programming Language | C++ (Arduino) |
| IoT Dashboard | ESP32 Web Server |
| Display | SSD1306 OLED |
| Sensors | DHT11, Soil Moisture, HC-SR04, IR, TTP223 |
| Automation | Relay Controlled Water Pump |

---

## 🔌 Hardware Connections

### ESP32 Pin Mapping

| Component | Pin |
|------------|------|
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| DHT11 Data | GPIO 4 |
| Soil Moisture AO | GPIO 34 |
| Ultrasonic Trigger | GPIO 18 |
| Ultrasonic Echo | GPIO 19 |
| Buzzer | GPIO 26 |
| IR Sensor OUT | GPIO 32 |
| Touch Sensor OUT | GPIO 27 |
| Relay IN | GPIO 23 |

### Power Connections

| Component | VCC | GND |
|------------|-----|-----|
| OLED | 3.3V | GND |
| DHT11 | VIN | GND |
| HC-SR04 | VIN | GND |
| IR Sensor | 3.3V | GND |
| Touch Sensor | 3.3V | GND |
| Soil Moisture Sensor | 3.3V | GND |
| Relay Module | VIN | GND |

### Relay & Pump Wiring

| Relay Terminal | Connection |
|---------------|------------|
| COM | 9V Battery (+) |
| NO | Pump (+) |
| NC | Not Connected |

| Device | Connection |
|----------|------------|
| Pump (-) | 9V Battery (-) |

---

## ⚙️ How It Works

1. Soil moisture is continuously monitored.
2. If the soil becomes dry, the ESP32 activates the relay and starts the water pump.
3. Once adequate moisture is detected, the pump turns off automatically.
4. The ultrasonic sensor measures water level inside the tank.
5. When water becomes low, the buzzer alerts the user.
6. Temperature and humidity are monitored using the DHT11 sensor.
7. Sensor data is displayed on the OLED screen and web dashboard.
8. IR and touch sensors trigger interactive OLED eye animations.

---

## 📊 Automation Logic

| Condition | Action |
|------------|---------|
| Soil Moisture ≥ 4000 | Pump ON |
| Soil Moisture < 3000 | Pump OFF |
| Tank Distance ≥ 20 cm | Buzzer Alert |
| Tank Distance < 20 cm | Tank OK |

---

## 📚 Libraries Used

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
```

---

## 👨‍💻 Author

**Alen C Francis**

Integrated Training Program on Rapid Prototyping, Embedded Systems & Automation

---

<div align="center">

### 🌱 Smart Plants. Smarter Living.

</div>
