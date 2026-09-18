# MAATS - Material Accelerated Aging Test System

[![Build Status](https://img.shields.io/badge/Build-Success-brightgreen)](https://github.com/raintiliang/GD32F470EVBCollectionAgent)
[![Platform](https://img.shields.io/badge/MCU-GD32F470Z-blue)](https://www.gigadevice.com/)
[![OS](https://img.shields.io/badge/RTOS-FreeRTOS_11.2.0-orange)](https://www.freertos.org/)

MAATS (Material Accelerated Aging Test System) is a control and monitoring system designed for material accelerated aging tests. It maintains a controlled environment with specific CO2 concentration and humidity for material sample reactions.

## 🚀 Key Features

- **Precise Environment Control**: PID-based closed-loop control for CO2 concentration and humidity.
- **Real-time Data Acquisition**: Supports S8-005 (CO2) via UART and AHT10 (Temp/Humid) via I2C.
- **Industrial Communication**: 4G MQTT for remote cloud monitoring and RS485 (Modbus RTU) for dehumidifier control.
- **Robust Storage**: Local data logging to TF card in CSV format.
- **Local Interaction**: 5-inch TFT display for status visualization and keypad for local control.
- **Cloud Integration**: Real-time dashboard and history query via Web client.

## 🏗 System Architecture

### Hardware Components
- **MCU**: GigaDevice GD32F470Z (ARM Cortex-M4F @ 240MHz)
- **Sensors**: 
  - CO2: Sensirion S8-005 (UART)
  - Temp/Humid: AHT10 / SHT40 (I2C)
- **Actuators**:
  - CO2 Valve: GPIO
  - Humidifier: GPIO
  - Dehumidifier: RS485 (Modbus RTU)
- **Connectivity**: 4G LTE Module (MQTT)
- **Storage**: MicroSD Card (FAT32)
- **Display**: 5" TFT RGB LCD

### Software Tasks (FreeRTOS)
| Task Name | Priority | Cycle | Responsibility |
|-----------|----------|-------|----------------|
| `SensorTask` | 3 | 1s | Acquire CO2, Temperature, and Humidity |
| `ActuatorTask` | 2 | 500ms | Control valves and RS485 devices |
| `ControlTask` | 1 | 200ms | Run PID algorithms for CO2 and Humidity |
| `CommTask` | 2 | 5s | Sync data with cloud via MQTT |
| `StorageTask` | 4 | 60s | Log data to CSV on TF card |
| `DisplayTask` | 5 | 1s | Refresh local UI |
| `KeyScanTask` | 5 | 100ms | Handle user input |

## 🛠 Build & Environment

### Prerequisites
- **Toolchain**: `arm-none-eabi-gcc` (Tested with 14.2.1)
- **Build System**: CMake 3.10+ & Make
- **SDK**: GD32F4xx Firmware Library

### Compilation
```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake ..
make -j$(nproc)
```

### Flashing
Using OpenOCD:
```bash
openocd -f interface/stlink.cfg -f target/gd32f4x.cfg -c "program build/MAATS_ColletAgent.elf verify reset exit"
```

## 📂 Project Structure
```text
.
├── CMakeLists.txt      # Build configuration
├── main.c              # Application entry and FreeRTOS tasks
├── docs/               # SRS, Test Plans, and Design Documents
├── toolchain.cmake     # Cross-compilation settings
└── gd32f4xx_it.c       # Interrupt Service Routines
```

## 📜 License
Internal Project - Copyright (c) 2026 raintiliang.
