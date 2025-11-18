// ================== Libraries ==================
#include "math.h"
#include "SoftwareSerial.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
// ================== Pin Definitions ==================
#define HEATER_PIN   (2)
#define HEATER2_PIN  (4)
#define FAN_PIN      (12)
#define ATOMIZER_PIN (7)

// ================== Sensor ==================
Adafruit_AM2320 am2320 = Adafruit_AM2320();

// ================== Serial Communication ==================
String data;
char d1;

// ================== Setpoints and Offsets ==================
const float SET_TEMP      = 37;
const float SET_HUMIDITY  = 90;
const float TEMP_OFFSET   = 0.22;
const float HUMID_OFFSET  = 1.57;

// ================== State Variables ==================
float temp = 0.0;
float humidity = 0.0;

bool heater = false;          // Actual heater ON/OFF state
bool heaterStandby = true;    // Whether temperature control is active

bool heater2 = false;          // Actual heater ON/OFF state
bool heater2Standby = true;    // Whether temperature control is active

bool fan = false;             // Actual fan ON/OFF state
bool fanStandby = true;       // Whether fan control is active

bool atomizer = false;        // Actual humidifier ON/OFF state
bool atomizerStandby = true;  // Whether humidity control is active

// ================== Logging Parameters ==================
unsigned long lastLogSec = 0xFFFFFFFF;     // Used to detect 1 Hz logging interval
const unsigned long LOG_PERIOD_MS = 1000;  // Log once per second
unsigned long lastLoopMs = 0;

// ================== Setup ==================
void setup() {
  Serial.begin(9600);

  pinMode(HEATER_PIN, OUTPUT);
  pinMode(HEATER2_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(ATOMIZER_PIN, OUTPUT);

  // Default output states
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(HEATER2_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(ATOMIZER_PIN, HIGH);  // Default HIGH for your atomizer logic

  Wire.begin();
  am2320.begin();

  // Print CSV header
  Serial.println(F("time_s,temperature_C,humidity_pct,heater,heater2,humidifier,fan"));
}

// ================== Main Loop ==================
void loop() {
  const unsigned long nowMs  = millis();
  const unsigned long nowSec = nowMs / 1000UL;

  // ---------- Handle Serial Commands ----------
  if (Serial.available()) {
    data = Serial.readString();
    d1 = data.charAt(0);
    if      (d1 == 'A') heaterStandby   = false;
    else if (d1 == 'a') heaterStandby   = true;
    else if (d1 == 'B') heater2Standby = false;
    else if (d1 == 'b') heater2Standby = true;
    else if (d1 == 'C') atomizerStandby = false;
    else if (d1 == 'c') atomizerStandby = true;
    else if (d1 == 'F') fanStandby      = false;
    else if (d1 == 'f') fanStandby      = true;
    else if (d1 == 'H' || d1 == 'h') {
      Serial.println(F("time_s,temperature_C,humidity_pct,heater,heater2,humidifier,fan"));
    }
  }

  // ---------- Read Sensors ----------
  float tRead = am2320.readTemperature();
  if (!isnan(tRead)) temp = tRead;

  float hRead = am2320.readHumidity();
  if (!isnan(hRead)) humidity = hRead;

  // ---------- Heater Control ----------
  if (!heaterStandby) {
    if (heater) {
      if (temp > (SET_TEMP - TEMP_OFFSET)) {
        digitalWrite(HEATER_PIN, LOW);
        heater = false;
      }
    } else {
      if (temp <= (SET_TEMP - TEMP_OFFSET)) {
        digitalWrite(HEATER_PIN, HIGH);
        heater = true;
      }
    }
  } else {
    if (heater) {
      digitalWrite(HEATER_PIN, LOW);
      heater = false;
    }
  }

  // ---------- Heater2 Control ----------
  if (!heater2Standby) {
    if (heater2) {
      if (temp > (SET_TEMP - TEMP_OFFSET)) {
        digitalWrite(HEATER2_PIN, LOW);
        heater2 = false;
      }
    } else {
      if (temp <= (SET_TEMP - TEMP_OFFSET)) {
        digitalWrite(HEATER2_PIN, HIGH);
        heater2 = true;
      }
    }
  } else {
    if (heater2) {
      digitalWrite(HEATER2_PIN, LOW);
      heater2 = false;
    }
  }

  // ---------- Fan Control ----------
  if (!fanStandby) {
    digitalWrite(FAN_PIN, HIGH);
    fan = true;
  } else {
    digitalWrite(FAN_PIN, LOW);
    fan = false;
  }

  // ---------- Humidifier Control ----------
  // Uses your original pulse-trigger logic for atomizer
  if (!atomizerStandby) {
    if (atomizer) {
      if (humidity >= SET_HUMIDITY) {
        // Trigger a low-high pulse to turn atomizer off
        digitalWrite(ATOMIZER_PIN, LOW);
        delay(100);
        digitalWrite(ATOMIZER_PIN, HIGH);
        atomizer = false;
      }
    } else {
      if (humidity < SET_HUMIDITY) {
        // Trigger a low-high pulse to turn atomizer on
        digitalWrite(ATOMIZER_PIN, LOW);
        delay(100);
        digitalWrite(ATOMIZER_PIN, HIGH);
        atomizer = true;
      }
    }
  } else {
    // When in standby, ensure atomizer is off
    if (atomizer) {
      digitalWrite(ATOMIZER_PIN, LOW);
      delay(100);
      digitalWrite(ATOMIZER_PIN, HIGH);
    } else {
      digitalWrite(ATOMIZER_PIN, HIGH);
    }
    atomizer = false;
  }

  // ---------- CSV Logging (Once Per Second) ----------
  if (nowSec != lastLogSec) {
    lastLogSec = nowSec;

    // Format: time_s,temperature_C,humidity_pct,heater,heater2,humidifier,fan
    Serial.print(nowSec);
    Serial.print(',');

    Serial.print(temp, 2);
    Serial.print(',');

    Serial.print(humidity, 2);
    Serial.print(',');

    Serial.print(heater ? 1 : 0);
    Serial.print(',');

    Serial.print(heater2 ? 1 : 0);
    Serial.print(',');

    Serial.print(atomizer ? 1 : 0);
    Serial.print(',');

    Serial.println(fan ? 1 : 0);
  }

  // ---------- Delay to smooth loop timing ----------
  const unsigned long targetNext = lastLoopMs + LOG_PERIOD_MS / 2;
  if (targetNext > nowMs) {
    delay(targetNext - nowMs);
  }
  lastLoopMs = nowMs;
}
