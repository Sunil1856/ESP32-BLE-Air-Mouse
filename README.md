# 🖱️ ESP32 BLE Wireless Air Mouse with MPU-6050

A high-precision, wireless **Bluetooth Low Energy (BLE) Air Mouse** built using an **ESP32** microcontroller and an **MPU-6050 (GY-521)** 6-axis gyroscope and accelerometer, featuring dedicated **Enter (Left Click)** and **Back (Right Click)** buttons.

No USB dongle or receiver required! It connects directly as a standard Bluetooth mouse to any **Windows PC, macOS, Android, iPhone/iPad, Linux, or Smart TV**.

---

## ✨ Features

- **Direct Bluetooth BLE**: Emulates a native HID mouse. No driver or software installation required on the computer.
- **Zero-Drift Auto-Calibration**: Automatically samples gyroscope offsets for 2 seconds on boot to eliminate cursor drift.
- **Anti-Jitter Deadzone**: Built-in digital deadzone and DLPF filter to eliminate hand tremors for smooth, pixel-perfect cursor control.
- **Direct MPU-6050 Driver**: Uses raw I2C communication (400kHz Fast Mode) without requiring heavy third-party sensor libraries.
- **Mouse Buttons**:
  - **Enter / Left Click** (GPIO 18): Single click, double click, and click-and-drag supported.
  - **Back / Right Click** (GPIO 19): Opens context menus and navigation.
  - **(Optional) Pause Button** (GPIO 23): Hold to freeze cursor movement while repositioning your wrist.

---

## 🛠️ Hardware Requirements

| Component | Quantity | Description |
|---|---|---|
| **ESP32 DevKit** | 1 | ESP32-WROOM-32 / Classic Dev Module |
| **MPU-6050 (GY-521)** | 1 | 6-Axis Gyroscope & Accelerometer |
| **Push Buttons** | 2 (or 3) | Tactile push buttons |
| **Jumper Wires** | - | Connecting wires |
| **Breadboard / Perfboard**| 1 | For prototyping |

---

## ⚡ Circuit & Wiring Diagram

The MPU-6050 module has 8 pins, but **only 4 pins** are needed. The other 4 pins (`XDA`, `XCL`, `AD0`, `INT`) can be left completely disconnected.

```
+------------------+         +----------------------+
|   ESP32 DevKit   |         | MPU-6050 (GY-521)    |
|                  |         |                      |
|             3.3V +---------> VCC                  |
|              GND +---------> GND                  |
|          GPIO 21 +---------> SDA                  |
|          GPIO 22 +---------> SCL                  |
|                  |         | XDA (Not connected)  |
|                  |         | XCL (Not connected)  |
|                  |         | AD0 (Not connected)  |
|                  |         | INT (Not connected)  |
|                  |         +----------------------+
|                  |
|          GPIO 18 +---------> [Enter Button] -----> GND (Left Click)
|          GPIO 19 +---------> [Back Button]  -----> GND (Right Click)
|          GPIO 23 +---------> [Pause Button] -----> GND (Optional Cursor Freeze)
+------------------+
```

> **Note**: Buttons connect directly between the GPIO pins and **GND**. The ESP32's internal `INPUT_PULLUP` resistors are enabled in software, so no external pull-up resistors are required.

---

## 🚀 Getting Started

### 1. Arduino IDE Setup
1. Install the latest [Arduino IDE](https://www.arduino.cc/en/software).
2. Install the **ESP32 Board Package**:
   - Go to **Tools > Board > Boards Manager...**
   - Search for **`esp32`** by Espressif Systems and click **Install**.
3. Install the **ESP32-BLE-Mouse** Library:
   - Download the library from [ESP32-BLE-Mouse GitHub](https://github.com/T-vK/ESP32-BLE-Mouse).
   - In Arduino IDE, go to **Sketch > Include Library > Add .ZIP Library...** and select the downloaded ZIP.

### 2. Uploading the Code
1. Open [`ESP32_Air_Mouse.ino`](ESP32_Air_Mouse.ino) in Arduino IDE.
2. Select your board: **Tools > Board > esp32 > ESP32 Dev Module**.
3. Select your serial port: **Tools > Port > COM...**.
4. Click **Upload (➔)**.

### 3. Pairing with PC / Laptop
1. Power up the ESP32. **Keep the device still on the table for 2 seconds** while it calibrates the gyro offsets.
2. On your PC/Laptop, open **Bluetooth Settings** > **Add Device** > **Bluetooth**.
3. Look for **`ESP32 Air Mouse`** and click **Pair/Connect**.
4. Pick up the sensor in your hand and move it in the air — the cursor will follow your hand movement!

---

## ⚙️ Customization & Sensitivity

You can adjust the sensitivity and deadzone at the top of [`ESP32_Air_Mouse.ino`](ESP32_Air_Mouse.ino):

```cpp
// Sensitivity settings: Increase for faster movement, decrease for slower
float SENSITIVITY_X = 1.6;
float SENSITIVITY_Y = 1.4;

// Deadzone filter: Increase if cursor moves when hand is resting
const int DEADZONE  = 120;

// Axis Inversion: Set true/false depending on sensor orientation
const bool INVERT_X = false;
const bool INVERT_Y = true;
```

---

## 📜 License

This project is licensed under the MIT License - feel free to use and modify for personal or commercial projects.
