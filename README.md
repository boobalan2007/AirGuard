[README_1.md](https://github.com/user-attachments/files/34272637/README_1.md)
# AirGuard 🚛👀

**A two-stage sensor + AI safety system to prevent heavy-vehicle blind-spot accidents.**

Heavy vehicles (trucks, buses, construction vehicles) have large blind spots near the wheels. Drivers often cannot see a person, child or animal standing close to the vehicle, and many lives are lost in blind-spot run-over accidents while turning or moving off. AirGuard adds a reliable **"second pair of eyes"** using cheap sensors plus smart AI.

## How it works

```
Ultrasonic + PIR   ->  something is CLOSE to the wheel      (Stage 1: cheap, always ON)
        |
        v
Camera + YOLOv8 on Raspberry Pi 5  ->  is it a HUMAN?        (Stage 2: AI confirms)
        |
        v
If YES  ->  Relay (brake / stop) + Servo (alert)            (Action)
```

Two stages means the cheap sensors watch all the time, and the AI camera only runs to **confirm** — this reduces false alarms and saves power.

## Hardware

- ESP32 DevKit — reads sensors, controls relay + servo
- Raspberry Pi 5 (4GB) — runs the AI camera
- HC-SR04 ultrasonic sensor
- PIR motion sensor
- Relay module
- Servo motor
- Android phone as a wireless camera (IP Webcam app)

### ESP32 pin connections

| Part          | Pin |
|---------------|-----|
| HC-SR04 Trig  | D25 |
| HC-SR04 Echo  | D34 |
| PIR OUT       | D27 |
| Relay IN      | D26 |
| Servo signal  | D13 |

## Software

- `airguard_esp32.ino` — ESP32 code: reads sensors, listens to the Pi, drives relay + servo. Needs the **ESP32Servo** library.
- `airguard_pi.py` — Raspberry Pi code: reads the phone camera, runs YOLOv8, and tells the ESP32 when a person is seen.

### Technologies used

Python, C/C++ (Arduino), YOLOv8 (Ultralytics), PyTorch, OpenCV, NCNN, PySerial, IP Webcam, Roboflow, Google Colab.

## Current status (honest)

- ✅ YOLOv8 person detection running **live** on Raspberry Pi 5 (phone camera over WiFi)
- ✅ Ultrasonic, PIR and relay working on the ESP32
- 🔄 Servo control and the Pi ↔ ESP32 serial trigger — integration in progress
- 🔜 Custom-trained model (person, head, helmet, hand, leg) for partial-body detection under the wheel

## Author

Boobalan K — Computer Science & Engineering, Panimalar Engineering College, Chennai
