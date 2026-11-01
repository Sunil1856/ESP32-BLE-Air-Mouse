#include <Wire.h>
#include <BleMouse.h>

// ==========================================================
// PIN CONFIGURATION (ESP32 Classic DevKit)
// ==========================================================
// MPU-6050 I2C Pins:
#define SDA_PIN       21
#define SCL_PIN       22

// Buttons (Active LOW with internal INPUT_PULLUP):
#define BTN_ENTER     18  // Left Click / Select / Enter
#define BTN_BACK      19  // Right Click / Context Menu / Back
#define BTN_PAUSE     23  // (Optional) Hold to freeze cursor / reposition hand

// ==========================================================
// MPU6050 CONSTANTS & SETTINGS
// ==========================================================
#define MPU_ADDR      0x68  // Default I2C address of MPU6050

// Sensitivity & Deadzone settings:
// Increase SENSITIVITY if cursor moves too slow; decrease if too fast.
float SENSITIVITY_X = 1.6;
float SENSITIVITY_Y = 1.4;
const int DEADZONE  = 120;  // Raw gyro deadzone to stop jitter/hand tremor

// Invert Axis if needed (true/false)
const bool INVERT_X = false;
const bool INVERT_Y = true;   // Gyro X pitch is inverted for natural up/down

// BleMouse object (Name, Manufacturer, Battery %)
BleMouse bleMouse("ESP32 Air Mouse", "Antigravity", 100);

// Gyro calibration offsets
long gyroX_offset = 0;
long gyroY_offset = 0;
long gyroZ_offset = 0;

// Button state tracking (with debouncing)
bool lastEnterState = HIGH;
bool lastBackState  = HIGH;

// ==========================================================
// MPU-6050 LOW-LEVEL FUNCTIONS (NO EXTRA LIBRARY NEEDED!)
// ==========================================================
void mpuWrite(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void readRawGyro(int16_t &gx, int16_t &gy, int16_t &gz) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43); // Gyro data registers start at 0x43
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)MPU_ADDR, (size_t)6, true);

  if (Wire.available() >= 6) {
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();
  }
}

void initMPU() {
  Wire.begin(SDA_PIN, SCL_PIN, 400000); // 400kHz Fast I2C
  delay(100);

  // Wake up MPU-6050 (Power Management 1 register)
  mpuWrite(0x6B, 0x00);
  delay(50);

  // Set Gyro full scale range to +/- 500 deg/s (FS_SEL = 1, Reg 0x1B = 0x08)
  mpuWrite(0x1B, 0x08);
  delay(20);

  // Set DLPF (Digital Low Pass Filter) to ~42Hz bandwidth to smooth out noise
  mpuWrite(0x1A, 0x03);
  delay(20);
}

void calibrateGyro() {
  Serial.println("Calibrating MPU6050... Please keep the mouse STILL on desk!");
  long sumX = 0, sumY = 0, sumZ = 0;
  const int SAMPLES = 300;

  for (int i = 0; i < SAMPLES; i++) {
    int16_t gx = 0, gy = 0, gz = 0;
    readRawGyro(gx, gy, gz);
    sumX += gx;
    sumY += gy;
    sumZ += gz;
    delay(4);
  }

  gyroX_offset = sumX / SAMPLES;
  gyroY_offset = sumY / SAMPLES;
  gyroZ_offset = sumZ / SAMPLES;

  Serial.println("✅ Calibration Complete!");
  Serial.print("Offsets -> X: "); Serial.print(gyroX_offset);
  Serial.print(" | Z: "); Serial.println(gyroZ_offset);
}

// ==========================================================
// SETUP
// ==========================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=================================");
  Serial.println("   ESP32 BLE Wireless Air Mouse  ");
  Serial.println("=================================");

  // Setup button pins with internal pull-up resistors
  pinMode(BTN_ENTER, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_PAUSE, INPUT_PULLUP);

  // Initialize MPU6050 sensor
  initMPU();

  // Calibrate Gyro while lying still
  calibrateGyro();

  // Start BLE Mouse server
  Serial.println("Starting Bluetooth BLE Mouse...");
  bleMouse.begin();
  Serial.println("👉 Turn on Bluetooth on your PC/Phone and pair with 'ESP32 Air Mouse'!");
}

// ==========================================================
// MAIN LOOP
// ==========================================================
void loop() {
  // Only process if connected to a PC / Phone via Bluetooth
  if (bleMouse.isConnected()) {
    // 1. Read Button States
    bool enterPressed = (digitalRead(BTN_ENTER) == LOW);
    bool backPressed  = (digitalRead(BTN_BACK) == LOW);
    bool pauseActive  = (digitalRead(BTN_PAUSE) == LOW); // If held, freezes cursor

    // --- ENTER / LEFT CLICK BUTTON ---
    if (enterPressed != (lastEnterState == LOW)) {
      if (enterPressed) {
        bleMouse.press(MOUSE_LEFT);
        Serial.println("Left Click Pressed (Enter)");
      } else {
        bleMouse.release(MOUSE_LEFT);
        Serial.println("Left Click Released");
      }
      lastEnterState = enterPressed ? LOW : HIGH;
    }

    // --- BACK / RIGHT CLICK BUTTON ---
    if (backPressed != (lastBackState == LOW)) {
      if (backPressed) {
        bleMouse.press(MOUSE_RIGHT);
        Serial.println("Right Click Pressed (Back)");
      } else {
        bleMouse.release(MOUSE_RIGHT);
        Serial.println("Right Click Released");
      }
      lastBackState = backPressed ? LOW : HIGH;
    }

    // 2. Read Gyroscope & Move Mouse Cursor (unless Pause button is held)
    if (!pauseActive) {
      int16_t rawX = 0, rawY = 0, rawZ = 0;
      readRawGyro(rawX, rawY, rawZ);

      // Subtract calibrated offsets
      long diffX = rawX - gyroX_offset; // Pitch -> up/down
      long diffZ = rawZ - gyroZ_offset; // Yaw   -> left/right

      // Apply Deadzone to prevent hand tremor / jitter
      if (abs(diffX) < DEADZONE) diffX = 0;
      if (abs(diffZ) < DEADZONE) diffZ = 0;

      // Calculate mouse move deltas
      // Turning wrist left/right (Yaw / diffZ) -> Move X
      // Pitching wrist up/down (Pitch / diffX) -> Move Y
      int moveX = (int)(-diffZ / 250.0 * SENSITIVITY_X);
      int moveY = (int)(diffX / 250.0 * SENSITIVITY_Y);

      if (INVERT_X) moveX = -moveX;
      if (INVERT_Y) moveY = -moveY;

      // Send movement to PC if there is motion
      if (moveX != 0 || moveY != 0) {
        bleMouse.move(moveX, moveY);
      }
    }

    delay(10); // ~100Hz polling rate for smooth cursor movement
  } else {
    // Heartbeat on serial while waiting for pairing
    static unsigned long lastWait = 0;
    if (millis() - lastWait > 2000) {
      lastWait = millis();
      Serial.println("Waiting for Bluetooth connection from PC...");
    }
    delay(50);
  }
}
