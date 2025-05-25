# FASTPASS Sensor Node Firmware

This repository contains the firmware used for the various sensor nodes deployed as part of the **FASTPASS: IoT Real-Time Study Space Availability System**, a senior design project at the University of Miami.

The FASTPASS system uses three complementary algorithms to estimate real-time study space occupancy. **Motion detection** is performed using a VL53L5CX LiDAR sensor, which tracks directional movement across an 8×8 time-of-flight pixel grid. By analyzing the sequence and timing of activated zones, the system distinguishes left-to-right versus right-to-left movement, allowing it to increment or decrement an internal headcount. **Wi-Fi sniffing** operates in promiscuous mode, capturing probe requests and data frames from nearby devices. MAC addresses are hashed and stored in a sliding time window, with only those above a specified RSSI threshold (e.g., -70 dBm) counted to infer the number of people in the area. **Noise level estimation** uses an INMP441 MEMS microphone with a 1024-point FFT to isolate energy in the 100 Hz–5 kHz range, where human conversation dominates. The root-mean-square (RMS) value is averaged over 30 seconds and converted to dB SPL, providing a “quiet,” “moderate,” or “loud” classification for each space. Together, these algorithms enable robust, privacy-conscious crowd monitoring. Each `.ino` sketch represents different motion tracking algorithms and node orientation for each sensor node. These nodes were built using the ESP32-WROOM-32E and interface with LiDAR sensors (VL53L5CX, VL53L1X), a MEMS microphone (INMP441), and Firebase for real-time database synchronization.

## File Descriptions

### `Unit_1a.ino`, `Unit_1b.ino`, `Unit_1c.ino`   
* Utilizes side-to-side motion detection to detect first-floor entries and exits, with zone A acting as a buffer zone.

### `Unit_2a.ino`, `Unit_2b.ino`, `Unit_2c.ino`, `Unit_3d.ino`
* Combines noise level detection and Wi-Fi sniffing for estimating occupancy in zones C, D, E, and B, respectively.  

### `Unit_3a.ino`  
* Utilizes forward-to-back motion detection to detect first-floor entries and exits, with zone A acting as a buffer zone. This sketch was specifically designed to be placed under stairs facing the direction of traffic.

### `Unit_3b.ino`  
* Utilizes side-to-side motion detection to detect first-floor entries and exits, with zone A acting as a buffer zone. This sketch was placed in an orientation to track entries and exits for the elevator lobby.

### `Unit_3c.ino`  
* Utilizes the VL53L1X sensor unit to track exits at the exit gate of the library.

## Features

- **ESP32-WROOM-32E based**
- **VL53L5CX or VL53L1X Time-of-Flight LiDAR support**
- **INMP441 MEMS microphone audio acquisition**
- **MAC address sniffing in promiscuous mode**
- **Firebase Realtime Database integration**
- **Configurable thresholds for presence detection**

## Tools & Dependencies

- [Arduino IDE with ESP32 Board Manager](https://github.com/espressif/arduino-esp32)
- [SparkFun VL53L5CX Arduino Library](https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library)
- `arduinoFFT.h` for microphone analysis
- `WiFi.h` (ESP32 WiFi stack)

## Notes

- Ensure Wi-Fi credentials and Firebase API keys are configured inside each sketch before uploading.
- Sampling frequency, FFT resolution, and smoothing parameters are manually defined in code.
- Power management is critical—ensure 3.3V regulation is stable and properly decoupled.

## Hardware Overview

Each sensor unit consists of some combination of the following:
- ESP32-WROOM-32E (on Freenove breakout)
- INMP441 microphone
- VL53L1X or VL53L5CX LiDAR module
- Powered via 18650 Li-ion battery or regulated wall adapter
- Custom 3D-printed enclosure with divided compartments

Refer to the `/hardware/` folder for wiring diagrams and 3D models.

---

Developed as part of the **FASTPASS Senior Design Project** at the University of Miami. Designed and tested by:  
**Nicolas Ramos** – Electrical Engineer  
University of Miami | Spring 2025
