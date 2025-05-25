# FASTPASS Hardware Assets

This directory contains the physical and electrical design files used in the construction of the FASTPASS sensor node. These assets include the custom 3D-printed enclosure for the ESP32-based sensing unit and a KiCAD schematic detailing the wiring between the ESP32 breakout board and various sensors.

---

## 3D-Printed Enclosure

The enclosure was designed using Autodesk Fusion 360 and optimized for compact deployment in public campus environments. It houses the ESP32 microcontroller, LiDAR sensor, microphone, and supporting hardware.

### Files:

- `Senior Design Final Enclosure Shell.stl`  
  The main body of the sensor housing. Includes internal dividers to separate power and sensing components, and mounting structures for securing the breadboard and battery. Some exterior components include key holes for security purposes.

- `Senior Design Final Front Lid.stl`  
  A sliding front panel that aligns with the LiDAR and microphone cutouts. Designed for easy access to the sensors while maintaining a clean, flush fit.

- `Senior Design Final Back Lid.stl`  
  Rear panel with cutouts for power input and optional debug port access. Also slides into place, offering secure but removable closure.

---

## Electrical Schematic

- `Fast Pass Wire Diagram.kicad_sch`  
  A KiCAD 7.0-compatible schematic showing the wiring between:
  - ESP32-WROOM-32E (via Freenove breakout board)
  - INMP441 MEMS microphone (I²S)
  - VL53L5CX or VL53L1X LiDAR (I²C)
  - Power delivery from 18650 battery pack (via onboard regulator)
  
This schematic is used for documenting prototype wiring on a solderless breadboard, prior to any custom PCB development.

Ensure I²S and I²C pins on the ESP32 match those specified in the firmware (`Unit_1a.ino`, `Unit_3b.ino`, etc.).

---

## Notes

- All STL files were validated and tested on Bambu Lab A1 printers.
- Sensor cutouts were dimensioned for Pololu VL53L5CX and Adafruit INMP441 form factors.
- Breadboard and ESP32 module mounting were done using adhesives and command strips to ensure components did not loosen or fall.

---

For more information on the FASTPASS system, refer to the main project documentation in `/docs` and firmware source code in `/firmware`.

Designed and tested by:  
**Nicolas Ramos** – Electrical Engineer  
University of Miami | Spring 2025
