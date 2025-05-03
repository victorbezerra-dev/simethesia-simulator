/*
  ==========================================
  🧠 Anesthesia Simulation - PK/PD Model
  📟 Serial Command Interface
  ==========================================

  ✅ AVAILABLE COMMANDS (via Serial Monitor):

  ▶️ Start / Stop Simulation:
    - start             → Starts the simulation
    - stop              → Stops the simulation

  🧍‍♂️ Configure patient (sex, weight_kg, height_cm, age):
    - patient <M/F> <weight> <height> <age>
    - Example: patient M 75 180 30

  🎲 Generate random patient:
    - generate_patient
    - Random sex, weight (50–100kg), height (150–200cm), age (18–90y)

  🧩 Activate clinical challenge:
    - challenge <number>
    - Challenges available:
        0 - Reset (no challenge)
        1 - Hemorrhage
        2 - Vasodilation
        3 - Vasoconstriction
        4 - Vasoactive drugs
        5 - Neuronal challenge

  📤 Output data sent via Serial (every 100 ms):
    - time (s), BIS, infusion rate (u)

  📥 Input via I2C:
    - Infusion rate (float) sent by master device

  Designed for real-time control system simulations
  ==========================================
*/

#include <Wire.h>

float c1 = 0;
float c2 = 0;
float c3 = 0;
float ce = 0;
float BIS = 100;
float infusionRate = 8.75;
float ke0 = 0.456;
float gamma = 2.69;
float ce50 = 4.92;
float E0 = 95.9;
float Emax = 87.5;
float k10;
float k12;
float k13;
float k21;
float k31;
float V1 = 4.27;
float V2;
float V3 = 238;
float sampleTime = 0.1;
float weight = 75.0;
float height = 175.0;
float age = 30.0;

char sex = 'M';

unsigned long lastUpdate = 0;
bool simulationRunning = false;

int activeChallenge = 0;

float V1_base = 4.27;
float ke0_base = 0.456;
float E0_base = 95.9;
float Emax_base = 87.5;
float k10_base;


void applyChallenge(int challenge) {
  V1 = V1_base;
  k10 = k10_base;
  ke0 = ke0_base;
  E0 = E0_base;
  Emax = Emax_base;

  switch (challenge) {
    case 1: V1 *= 0.75; k10 *= 1.3; Serial.println("🔥 Hemorrhage activated"); break;
    case 2: V1 *= 1.4; Serial.println("🌀 Vasodilation activated"); break;
    case 3: V1 *= 0.6; ke0 *= 0.7; Serial.println("⏬ Vasoconstriction activated"); break;
    case 4: ke0 *= 1.5; Emax *= 1.2; Serial.println("💊 Vasoactive drugs activated"); break;
    case 5: E0 *= 0.85; Serial.println("⚡ Neuronal challenge activated"); break;
    default: Serial.println("🔄 System reset to baseline"); break;
  }
}

void configurePatient(char s, float w, float h, float a) {
  sex = s;
  weight = w;
  height = h;
  age = a;

  float LBM = (sex == 'M')
              ? 1.10 * weight - 128 * (weight * weight) / (height * height)
              : 1.07 * weight - 148 * (weight * weight) / (height * height);

  k10_base = 0.443 + 0.0107 * (weight - 77) - 0.0159 * (LBM - 59) + 0.0062 * (height - 177);
  k10 = k10_base;
  k12 = 0.302 - 0.0056 * (age - 53);
  k13 = 0.196;
  k21 = (1.29 - 0.024 * (age - 53)) / (18.9 - 0.391 * (age - 53));
  k31 = 0.0035;
  V2 = (18.9 - 0.391 * (age - 53));
  Serial.println("✅ Patient configured.");
  Serial.print("🎲  Patient data: ");
  Serial.print(s); Serial.print(" | ");
  Serial.print(w); Serial.print(" kg | ");
  Serial.print(h); Serial.print(" cm | ");
  Serial.print(a); Serial.println(" y/o");
}

void setup() {
  Wire.begin(8);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);

  configurePatient(sex, weight, height, age);

  Serial.println("System ready. Type 'start' to begin simulation.");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "start") {
      simulationRunning = true;
      Serial.println("▶ Simulation started");
    } else if (cmd == "stop") {
      simulationRunning = false;
      Serial.println("⏹ Simulation stopped");
    } if (cmd.startsWith("patient")) {
      cmd = cmd.substring(8);
      cmd.trim();

      char s = cmd.charAt(0);
      cmd = cmd.substring(1);
      cmd.trim();

      int firstSpace = cmd.indexOf(' ');
      float w = cmd.substring(0, firstSpace).toFloat();
      cmd = cmd.substring(firstSpace + 1);

      firstSpace = cmd.indexOf(' ');
      float h = cmd.substring(0, firstSpace).toFloat();
      cmd = cmd.substring(firstSpace + 1);

      float a = cmd.toFloat();

      configurePatient(s, w, h, a);
    } else if (cmd.startsWith("challenge")) {
      int challenge;
      sscanf(cmd.c_str(), "challenge %d", &challenge);
      if (challenge >= 0 && challenge <= 5) {
        activeChallenge = challenge;
        applyChallenge(activeChallenge);
      } else {
        Serial.println("⚠ Invalid challenge number.");
      }
    } else if (cmd == "generate_patient") {
      char s = random(0, 2) == 0 ? 'M' : 'F';
      float w = random(500, 1000) / 10.0;
      float h = random(1500, 2000) / 10.0;
      float a = random(18, 90);

      configurePatient(s, w, h, a);

      Serial.println("Generated patient with success.");

    } else {
      Serial.println("❌ Unknown command.");
    }
  }

  if (simulationRunning && millis() - lastUpdate >= (unsigned long)(sampleTime * 1000)) {
    lastUpdate = millis();

    float c1_old = c1, c2_old = c2, c3_old = c3, ce_old = ce;
    c1 = c1_old + ((infusionRate / V1) - (k10 + k12 + k13) * c1_old + (k21 * V2 / V1) * c2_old + (k31 * V3 / V1) * c3_old) * sampleTime;
    c2 = c2_old + (k12 * V1 / V2 * c1_old - k21 * c2_old) * sampleTime;
    c3 = c3_old + (k13 * V1 / V3 * c1_old - k31 * c3_old) * sampleTime;
    ce = ce_old + ke0 * (c1_old - ce_old) * sampleTime;
    BIS = E0 - Emax * (pow(ce, gamma) / (pow(ce50, gamma) + pow(ce, gamma)));

    Serial.print(millis() / 1000.0, 2);
    Serial.print(",");
    Serial.print(BIS, 2);
    Serial.print(",");
    Serial.print(infusionRate, 2);
    Serial.println(";");
  }
}

void requestEvent() {
  byte* bisInByte = (byte*)&BIS;
  Wire.write(bisInByte, sizeof(float));
}

void receiveEvent(int bytes) {
  byte data[4];
  for (int i = 0; i < 4; i++) {
    data[i] = Wire.read();
  }
  infusionRate = *((float*)data);
}