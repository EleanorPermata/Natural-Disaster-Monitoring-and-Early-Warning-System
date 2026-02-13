# 🌍 Natural Disaster Monitoring and Early Warning System

## 📌 Project Overview
This project implements an **IoT-based Natural Disaster Monitoring and Early Warning System** using **ESP32** and multiple sensors.  
It continuously monitors environmental parameters such as **temperature, humidity, soil moisture, vibration, and tilt angle** to detect potential disasters like **landslides and earthquakes**.  

The system provides **real-time alerts** via:
- **LED indicators (Green, Yellow, Red)**
- **Piezo buzzer alarms**
- **Cloud dashboard (ThingSpeak)**
- **Push notifications through Telegram Bot**

---

## 🎯 Objectives
- Provide communities and authorities with **real-time environmental monitoring**.  
- Reduce risks and casualties by issuing **early warnings**.  
- Integrate **multi-sensor data collection** with cloud-based visualization.  
- Enable **automated alerts** for immediate response.  
- Store historical data for **trend analysis and prediction**.  

---

## 🛠️ Components

### 🔹 Hardware
- **ESP32 NodeMCU** – main microcontroller with Wi-Fi & Bluetooth  
- **DHT11 Sensor** – temperature & humidity monitoring  
- **Soil Moisture Sensor** – ground saturation detection  
- **SW-420 Vibration Sensor** – seismic activity detection  
- **MPU6050 Gyroscope/Accelerometer** – tilt angle measurement  
- **LEDs (Red, Yellow, Green)** – visual alerts  
- **Piezo Buzzer** – audio alerts  
- **Breadboard & Jumper Wires** – prototyping connections  

### 🔹 Software
- **Arduino IDE** – programming ESP32  
- **ThingSpeak** – cloud IoT platform for data visualization  
- **Telegram Bot API** – push notifications for disaster alerts  

---

## ⚙️ System Design
- **Green LED** → Normal conditions  
- **Yellow LED + buzzer (low pitch)** → Environmental warning (e.g., high soil moisture, temperature, or humidity)  
- **Red LED + buzzer (high pitch)** → Landslide or earthquake risk detected  
- **ThingSpeak Dashboard** → Real-time graphs for all sensor data  
- **Telegram Alerts** → Instant notifications to users and authorities  

---

## 💻 Source Code
The complete implementation is available in the repository under [`src/LPAN_Code_TP072606.ino`](src/LPAN_Code_TP072606.ino).  

Here’s a quick snippet showing how disaster alerts are triggered:

```cpp
if (landslideRisk || earthquakeRisk) {
  digitalWrite(RED_LED, HIGH);
  tone(BUZZER_PIN, 1000);
  sendTelegramMessage("EARLY WARNING: Landslide or Earthquake Detected!");
} else if (environmentalRisk) {
  digitalWrite(YELLOW_LED, HIGH);
  tone(BUZZER_PIN, 500);
} else {
  digitalWrite(GREEN_LED, HIGH);
  noTone(BUZZER_PIN);
}
```
---
## ⚠️ Limitations
- **Internet dependency**: Requires stable Wi-Fi for ThingSpeak and Telegram alerts.  
- **Sensor sensitivity**: Environmental noise may cause false positives.  
- **Power supply**: Continuous monitoring requires reliable energy sources.  

---

## 🏁 Conclusion
The Natural Disaster Monitoring and Early Warning System successfully achieved its objectives by providing **real-time monitoring and alerts** for landslides and earthquakes through IoT technology.  
By integrating multiple sensors (DHT11, soil moisture
