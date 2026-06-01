<div align="center">

# 🌱 FloraHub
### Smart Plant Monitoring & Automated Watering System

![ESP32](https://img.shields.io/badge/ESP32-IoT-blue?style=for-the-badge&logo=espressif)
![Arduino](https://img.shields.io/badge/Arduino-C++-00979D?style=for-the-badge&logo=arduino)
![IoT](https://img.shields.io/badge/IoT-Smart%20Agriculture-success?style=for-the-badge)
![License](https://img.shields.io/badge/Status-Completed-brightgreen?style=for-the-badge)

**An intelligent IoT-powered plant care ecosystem that automates irrigation, monitors environmental conditions, and provides real-time plant health insights.**

</div>

---

## 📖 Overview

FloraHub is a smart plant monitoring and automated watering system built using the ESP32 microcontroller. The system continuously monitors soil moisture, temperature, humidity, and water tank levels while automatically watering plants whenever required.

Designed as a complete IoT product, FloraHub combines:

- Embedded Systems
- Internet of Things (IoT)
- Automation
- Product Development

into a single intelligent plant-care solution.

---

## 🎯 Problem Statement

Plant owners often face challenges such as:

- Underwatering due to busy schedules
- Overwatering caused by guesswork
- Empty water reservoirs going unnoticed
- Lack of environmental monitoring
- No remote visibility into plant health

FloraHub solves these problems through continuous monitoring, intelligent automation, and real-time data visualization.

---

# ✨ Key Features

## 🌿 Smart Irrigation

- Continuous soil moisture monitoring
- Automatic pump activation when soil becomes dry
- Automatic pump shutdown when optimal moisture is reached
- Prevents overwatering through threshold-based control

---

## 💧 Water Tank Monitoring

- Real-time tank level measurement using ultrasonic sensing
- Detects low-water conditions
- Audible buzzer alerts when refill is required

---

## 🌡 Environmental Monitoring

- Temperature monitoring using DHT11
- Humidity monitoring using DHT11
- Real-time environmental tracking

---

## 📟 OLED Dashboard

Displays:

- Soil Moisture
- Temperature
- Humidity
- Water Tank Status
- Pump Status

---

## 👀 Interactive OLED Eyes

- Animated eye expressions
- Touch-based interaction
- Proximity-based activation
- Automatic return to monitoring mode

---

## 🌐 IoT Web Dashboard

- ESP32 hosted web interface
- Real-time sensor monitoring
- Responsive mobile-friendly layout
- Automatic refresh every 3 seconds

---

## 🔄 Smart Connectivity

- Automatic Wi-Fi recovery
- Background reconnection attempts
- Offline automation support
- Continuous operation during network outages

---

# 🛠 Hardware Components

| Component | Purpose |
|------------|----------|
| ESP32 Development Board | Main Controller |
| Soil Moisture Sensor | Soil Monitoring |
| DHT11 Sensor | Temperature & Humidity |
| HC-SR04 Ultrasonic Sensor | Water Tank Monitoring |
| SSD1306 OLED Display | User Interface |
| TTP223 Touch Sensor | User Interaction |
| IR Proximity Sensor | Human Detection |
| Relay Module | Pump Switching |
| Mini Water Pump | Irrigation |
| Piezo Buzzer | Low Water Alert |
| Breadboard & Jumper Wires | Connections |

---

# 🔌 Circuit Connections

| Component | ESP32 GPIO |
|------------|------------|
| Soil Moisture Sensor | GPIO 34 |
| DHT11 | GPIO 4 |
| Touch Sensor | GPIO 27 |
| IR Sensor | GPIO 32 |
| Buzzer | GPIO 26 |
| Relay | GPIO 23 |
| Ultrasonic Trigger | GPIO 18 |
| Ultrasonic Echo | GPIO 19 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |

---

# ⚙️ System Architecture

```text
                    ┌───────────────┐
                    │     ESP32     │
                    └───────┬───────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼

 Soil Moisture       DHT11 Sensor       HC-SR04 Sensor
    Sensor          Temp & Humidity      Tank Level

        │                   │                   │
        └───────────────────┼───────────────────┘
                            │

                    Automation Engine

                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼

    Water Pump          OLED UI         Web Dashboard
      Relay

                            │
                            ▼

                         Buzzer
                     Low Water Alert
```

---

# 🔄 Workflow

```text
START

   │

   ▼

Initialize Sensors

   │

   ▼

Read Sensor Values

   │

   ├── Soil Moisture

   ├── Temperature

   ├── Humidity

   └── Water Level

   │

   ▼

Check Soil Status

   │

   ├── Dry ?
   │      │
   │      └── Pump ON
   │
   └── Wet ?
          │
          └── Pump OFF

   │

   ▼

Check Water Tank

   │

   ├── Low Water ?
   │        │
   │        └── Buzzer ON
   │
   └── Tank OK

   │

   ▼

Update OLED Display

   │

   ▼

Update Web Dashboard

   │

   ▼

Repeat
```

---

# 📊 Automation Logic

### Soil Moisture

| Condition | Action |
|------------|---------|
| ADC ≥ 4000 | Pump ON |
| ADC < 3000 | Pump OFF |

---

### Water Tank

| Distance | Status |
|-----------|--------|
| ≥ 20 cm | Tank Empty |
| < 20 cm | Tank OK |

---

# 📱 Web Dashboard

The dashboard provides:

✅ Real-time Temperature Monitoring

✅ Real-time Humidity Monitoring

✅ Soil Moisture Tracking

✅ Water Tank Status

✅ Pump Activity Monitoring

✅ Mobile-Friendly Interface

✅ Automatic Data Refresh

---

# 📸 Project Gallery

### Prototype

```text
Add project images here:

/images/prototype.jpg
/images/dashboard.png
/images/circuit.png
/images/oled_display.jpg
```

---

# 🧪 Testing Results

### Automated Irrigation

✔ Successfully detected dry soil conditions

✔ Activated pump automatically

✔ Stopped watering at target moisture level

---

### Tank Monitoring

✔ Correctly detected low-water conditions

✔ Triggered audible alerts

✔ Cleared alerts immediately after refill

---

### OLED Interaction

✔ Stable animation triggering

✔ Smooth display transitions

✔ Anti-flicker protection working

---

### Wi-Fi Recovery

✔ Reconnected automatically

✔ Continued offline automation

✔ No system freezing observed

---

# 🚧 Challenges Faced

### Delay-Based Blocking

Traditional delay functions caused sensor and web dashboard lag.

**Solution:** Implemented a fully non-blocking architecture using `millis()`.

---

### Relay Instability

Electrical noise occasionally caused relay chatter.

**Solution:** High-impedance relay shutdown logic was implemented.

---

### ADC Noise

Wi-Fi activity introduced fluctuations in soil sensor readings.

**Solution:** Timed sensor polling and hysteresis thresholds were used.

---

# 🔮 Future Enhancements

- MQTT Integration
- Mobile Application
- Push Notifications
- AI Plant Health Analysis
- Cloud Data Logging
- OTA Firmware Updates
- Deep Sleep Power Optimization
- Multi-Plant Monitoring
- Voice Assistant Integration
- Automatic Grow Light Control

---

# 📂 Repository Structure

```text
FloraHub
│
├── Source_Code
└── README.md
```

---

# 📚 Libraries Used

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
```

---

# 🎓 Learning Outcomes

This project demonstrates practical implementation of:

- Embedded Systems Design
- IoT Development
- Sensor Integration
- Automation Systems
- Web Dashboard Development
- Product Design Thinking

---

# 👨‍💻 Developed By

**Alen C Francis**

Integrated Training Program on Rapid Prototyping, Embedded Systems & Automation

---

<div align="center">

### 🌱 Smart Plants. Smarter Living.

**If you found this project useful, consider giving it a ⭐ on GitHub.**

</div>
