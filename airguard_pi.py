#!/usr/bin/env python3
"""
AirGuard - Phase 2 (Raspberry Pi 5)
--------------------------------------------------------------
Reads the phone camera (IP Webcam app), runs YOLOv8n, and tells
the ESP32 over USB serial whether a PERSON is seen.

Sends '1' to the ESP32 when a person is detected, '0' when not.

Run on the Pi (inside the env):
    source ~/airguard-env/bin/activate
    python airguard_pi.py

Stop with Ctrl + C.
"""

import time
import cv2
import serial
from ultralytics import YOLO

# ---------- Settings (change these for your setup) ----------
CAMERA_URL  = "http://10.251.134.168:8080/video"  # your phone's IP Webcam address + /video
SERIAL_PORT = "/dev/ttyUSB0"                       # ESP32 USB port on the Pi (may be /dev/ttyACM0)
BAUD        = 115200
PERSON_CLASS = 0     # in YOLO, class 0 = person
CONF         = 0.4   # confidence threshold (0.0 - 1.0)

# ---------- Connect to ESP32 ----------
print("Opening serial to ESP32 ...")
try:
    esp = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
    time.sleep(2)   # wait for the ESP32 to reset
    print("ESP32 connected on", SERIAL_PORT)
except Exception as e:
    print("WARNING: could not open ESP32 serial:", e)
    esp = None      # keep running so you can still test detection

# ---------- Load model ----------
print("Loading YOLOv8n ...")
model = YOLO("yolov8n.pt")

# ---------- Open camera ----------
print("Opening camera stream ...")
cap = cv2.VideoCapture(CAMERA_URL)

last_sent = None   # remember last signal so we send only on change

print("AirGuard running. Point the camera at a person. Ctrl+C to stop.")
try:
    while True:
        ok, frame = cap.read()
        if not ok:
            print("No frame, reconnecting ...")
            time.sleep(0.5)
            cap.open(CAMERA_URL)
            continue

        # run detection
        results = model.predict(frame, conf=CONF, verbose=False)

        # is any detected box a person?
        person_found = False
        for box in results[0].boxes:
            if int(box.cls[0]) == PERSON_CLASS:
                person_found = True
                break

        # send to ESP32 only when the result changes
        signal = b'1' if person_found else b'0'
        if signal != last_sent:
            if esp is not None:
                esp.write(signal)
            last_sent = signal
            print("PERSON detected -> sent 1" if person_found else "no person -> sent 0")

        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nStopping AirGuard ...")
finally:
    if esp is not None:
        esp.write(b'0')   # make sure ESP32 goes back to safe
        esp.close()
    cap.release()
