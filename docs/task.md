# MAATS Project Execution Tasks

Based on SRS and Gantt chart, this document tracks the step-by-step implementation of the Material Accelerated Aging Test System.

## Phase 1: Firmware Development (Current)

### [T1] Driver Implementation
- [x] **T1.1: UART CO2 Sensor (S8-005)**
    - Implement UART configuration (9600, 8N1).
    - Implement non-blocking frame reception.
    - Implement `parse_co2_frame` logic.
- [ ] **T1.2: I2C Temp/Humid Sensor (AHT10)**
    - Implement I2C master configuration.
    - Implement trigger and read sequence for AHT10.
    - Implement data conversion formulas.
- [ ] **T1.3: GPIO Actuators**
    - Configure CO2 valve GPIO (Output).
    - Configure Humidifier GPIO (Output).
- [ ] **T1.4: RS485 Modbus RTU (Dehumidifier)**
    - Configure UART for RS485 with DE/RE control.
    - Implement basic Modbus RTU frame generation (Function 0x05).

### [T2] Core Logic & OS Integration
- [ ] **T2.1: PID Controller**
    - Implement `PID_Controller` struct and `pid_update` function.
    - Setup PID instances for CO2 and Humidity.
- [ ] **T2.2: FSM State Machine**
    - Implement state transition logic (IDLE, REACTING, PAUSED, ALARM).
    - Link KeyScan events to FSM transitions.
- [ ] **T2.3: Task Logic Refinement**
    - Implement `vSensorTask` loop (1s acquisition).
    - Implement `vControlTask` loop (200ms PID update).
    - Implement `vActuatorTask` loop (Apply control outputs).

### [T3] Storage & Communication
- [ ] **T3.1: TF Card (FATFS)**
    - Configure SDIO/SPI for SD Card.
    - Integrate FATFS library.
    - Implement CSV logging in `vStorageTask`.
- [ ] **T3.2: 4G MQTT Communication**
    - Implement AT command interface for 4G module.
    - Implement MQTT client logic (Connect, Pub, Sub).
    - Implement telemetry sync in `vCommTask`.

### [T4] Local UI
- [ ] **T4.1: Display Driver**
    - Configure RGB/SPI interface for 5" TFT.
    - Implement basic drawing/text functions.
- [ ] **T4.2: UI Dashboard**
    - Layout sensor values and reaction status on screen.

---

## Phase 2: Backend Development

### [T5] FastAPI Services
- [ ] **T5.1: Project Setup & Auth**
    - Initialize FastAPI project.
    - Implement JWT authentication.
- [ ] **T5.2: MQTT & Real-time**
    - Setup MQTT worker to bridge device data to DB/WSS.
    - Implement WebSocket for live dashboard data.
- [ ] **T5.3: Data & Control API**
    - Implement history data query API.
    - Implement remote control command API.

---

## Phase 3: Frontend Development

### [T6] Web Dashboard (React)
- [ ] **T6.1: Layout & Auth**
    - Setup React + AntD Pro.
    - Implement login and responsive navigation.
- [ ] **T6.2: Real-time Dashboard**
    - Integrate ECharts for CO2/Temp/Humid curves.
    - Implement WebSocket integration.
- [ ] **T6.3: History & Reports**
    - Build history query page with data export.

---

## Phase 4: Testing & Integration

- [ ] **T7: Unit Testing** (According to 03_UNIT_TEST_PLAN.md)
- [ ] **T8: System Testing** (According to 04_SYSTEM_TEST_PLAN.md)
- [ ] **T9: Calibration & Final Prep**
