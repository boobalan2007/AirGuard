/*
  AirGuard - Phase 1 (ESP32)
  --------------------------------------------------------------
  Sensors + Relay + Servo, with camera confirmation from the
  Raspberry Pi over USB serial.

  FLOW:
    Stage 1: Ultrasonic (HC-SR04) checks if something is CLOSE to the wheel.
    Stage 2: PIR checks for MOVEMENT near the wheel.
    Stage 3: Raspberry Pi camera (YOLOv8) confirms it is a HUMAN
             and sends '1' over serial ('0' = no person).
    ACTION : If an object is close AND the camera confirms a person
             -> Relay ON (brake/stop) + Servo moves (alert/barrier).

  ESP32 PINS:
    HC-SR04  Trig = D25, Echo = D34
    PIR      OUT  = D27
    Relay    IN   = D26
    Servo    sig  = D13

  NOTE: Needs the "ESP32Servo" library
        (Arduino IDE -> Tools -> Manage Libraries -> search "ESP32Servo" -> Install).
*/

#include <ESP32Servo.h>

// ---------- Pins ----------
const int TRIG_PIN  = 25;
const int ECHO_PIN  = 34;   // input-only pin, good for echo
const int PIR_PIN   = 27;
const int RELAY_PIN = 26;
const int SERVO_PIN = 13;

// ---------- Settings ----------
const int DANGER_CM   = 50;   // object closer than this = danger zone (change as needed)
const int SERVO_SAFE  = 0;    // servo rest position (degrees)
const int SERVO_ALERT = 90;   // servo action position (degrees)

// Some relay modules are ACTIVE-LOW. If your relay works opposite,
// swap these two values (RELAY_ON = LOW, RELAY_OFF = HIGH).
const int RELAY_ON  = HIGH;
const int RELAY_OFF = LOW;

Servo myServo;
bool cameraPerson = false;    // last message from Pi: true = person seen

// Read distance in cm from HC-SR04
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (~5m)
  if (duration == 0) return -1;                   // no echo
  return duration / 58;                           // microseconds -> cm
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN,  INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);   // relay OFF at start

  myServo.attach(SERVO_PIN);
  myServo.write(SERVO_SAFE);            // servo at rest

  Serial.println("AirGuard ESP32 ready");
}

void loop() {
  // 1) Read message from Raspberry Pi (camera result)
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '1') cameraPerson = true;    // Pi: person detected
    if (c == '0') cameraPerson = false;   // Pi: no person
  }

  // 2) Read sensors
  long distance  = readDistanceCM();
  bool objectClose = (distance > 0 && distance < DANGER_CM);
  bool motion      = (digitalRead(PIR_PIN) == HIGH);

  // 3) Decide: danger = object close to wheel AND camera confirms a human
  bool danger = objectClose && cameraPerson;

  if (danger) {
    digitalWrite(RELAY_PIN, RELAY_ON);  // brake / stop
    myServo.write(SERVO_ALERT);         // servo acts
  } else {
    digitalWrite(RELAY_PIN, RELAY_OFF); // normal
    myServo.write(SERVO_SAFE);
  }

  // 4) Print status (for Serial Monitor)
  Serial.print("dist=");   Serial.print(distance);
  Serial.print("cm close="); Serial.print(objectClose);
  Serial.print(" motion=");  Serial.print(motion);
  Serial.print(" camera=");  Serial.print(cameraPerson);
  Serial.print(" => ");
  Serial.println(danger ? "DANGER! BRAKE + SERVO" : "safe");

  delay(100);
}
