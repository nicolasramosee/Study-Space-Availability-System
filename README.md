# FASTPASS: IoT Real-Time Study Space Availability System 

**Senior Design Project — Spring 2025**
**University of Miami | Department of Electrical and Computer Engineering**
---

**Team Members:**

* **Nicolas Ramos** – Electrical Engineer
* **Bana Zoumot** – Computer Engineer
* **Jenna Viard** – Software Engineer
*  **Advisor:** Dr. Nigel John

---

## Project Overview

**FASTPASS** is a real-time smart campus system that empowers students and faculty to locate quiet, available study spaces within the Richter Library and other shared study areas. This system integrates LiDAR, microphone, and Wi-Fi sniffing technologies to detect occupancy and ambient noise levels, providing live data through a web-based dashboard.

Developed as a capstone project by a multidisciplinary team of engineers at the University of Miami, FASTPASS addresses a common academic challenge: the time-consuming and inefficient process of finding suitable study spaces during peak hours. The system delivers scalable, privacy-conscious, and real-time insights that improve productivity and student satisfaction. Because my role in this project was strictly related to all hardware and firmware design and development, this repository will largely highlight these areas.

---

## Key Features

* **Real-Time Occupancy Monitoring** using Wi-Fi sniffing (ESP32).
* **Environmental Sound Analysis** via INMP441 MEMS microphones.
* **Direction-Aware Entry Tracking** with VL53L5CX and VL53L1X LiDAR sensors.
* **Cloud Integration** using Firebase Realtime Database.
* **Intuitive React-Based Frontend** with search, filters, and live metrics.
* **Privacy by Design**: anonymized data, no cameras, no personal IDs.

---

## Technologies Used

* **Hardware:** ESP32-WROOM-32E, Pololu VL53L5CX, Adafruit VL53L1X, INMP441

* **Software:**

  * Embedded firmware (C++)
  * Firebase Realtime Database
  * React.js frontend
    
* **Tools:**

  * Figma (UI/UX Prototyping)
  * Autodesk Fusion 360 (3D enclosure design)
  * VS Code, Git, PlatformIO

---

## System Architecture

The system consists of:

1. **Sensor Nodes**: ESP32 microcontrollers read data from LiDAR and microphones and count Wi-Fi probe requests to infer real-time occupancy and noise levels.
2. **Backend**: Firebase handles data transmission, storage, and distribution.
3. **Frontend**: A student-facing React dashboard displays occupancy levels, sound ratings, and study space recommendations.
4. **Data Fusion Engine**: Combines historical data, real-time Wi-Fi, and LiDAR readings into a smooth, reliable occupancy estimate using an exponential moving average (EMA).

---

## Results

* Achieved ±7% accuracy in occupancy estimation during tests.
* 77% of user testers rated the system’s study space recommendations as highly accurate.
* Return loss of <10 dB in most zones (quiet areas).
* Sensor nodes maintained 90%+ uptime in field deployment.

---

## Outcomes

* Demonstrated the feasibility of privacy-conscious smart campus infrastructure.
* Provided real-world engineering experience across embedded systems, IoT, cloud development, and human-centered design.
* Recognized for aligning with UN Sustainable Development Goals (SDG 4 and SDG 9).

---

## Repository Contents

* `/hardware/` — Wiring diagrams, sensor modules, 3D enclosure models
* `/firmware/` — ESP32 Arduino code for LiDAR, microphone, Wi-Fi sniffing
* `/docs/` — Final Report

---

## Contact
Feel free to connect or reach out if you’d like to collaborate or ask questions!
- Email: nmree25@gmail.com
- Mobile: (786) 380-1981
- LinkedIn: www.linkedin.com/in/nicolas-ramos-503056344
