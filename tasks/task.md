# 项目任务追踪 (Task Tracking)

## 📅 2026-10-07 任务状态

### 1. 基础架构与文档 [已完成]
- [x] 完成项目立项书 (01_SRS.md)
- [x] 完成详细需求规格说明书 (02_SRS_DETAIL.md)
- [x] 完成单元测试计划 (03_UNIT_TEST_PLAN.md)
- [x] 完成系统测试计划 (04_SYSTEM_TEST_PLAN.md)
- [x] 完成项目甘特图 (05_GANTT.md)
- [x] 同步本地代码至 GitHub 仓库 (`GD32F470EVBCollectionAgent`)

### 2. 固件开发 - 传感器与驱动 [已完成]
- [x] **CO2 传感器驱动 (UART)**
    - [x] UART 接口定义与结构体设计
    - [x] 协议解析逻辑实现 (sscanf 方式)
    - [x] 针对 GD32F4xx 的底层 UART 初始化实现
    - [x] 增加滑动平均滤波算法 (窗口大小=10)
- [x] **温湿度传感器驱动 (I2C)**
    - [x] I2C 接口封装与 GPIO 配置 (PB6/PB7)
    - [x] AHT10 触发测量与 6 字节数据读取逻辑
    - [x] 原始数据转换为标准浮点温湿度逻辑
- [x] **执行器控制 (GPIO/RS485)**
    - [x] 电磁阀与加湿机 GPIO 控制 (PC0/PC1)
    - [x] RS485 (USART2) 驱动与 DE 方向切换逻辑 (PB12)
    - [x] Modbus RTU CRC16 算法与写寄存器帧实现

### 3. 系统核心逻辑 [进行中]
- [x] **PID 闭环控制算法**
    - [x] 增量式/位置式 PID 计算框架
    - [x] CO2 浓度控制逻辑与输出映射
    - [x] 湿度双向控制 (加湿/除湿) 与互锁逻辑
- [x] **FreeRTOS 任务调度与 TFT 显示**
    - [x] 细化多任务调度架构 (Sensor, Control, Storage, Comm, Display)
    - [x] 实现了基于队列 (Queue) 和互斥量 (Mutex) 的任务间同步机制
    - [x] 5 寸屏底层驱动接口设计 (`tft_display.c`)
    - [x] 基于 LVGL 的 UI 刷新管理框架 (`ui_manager.c`)
- [x] **存储与通讯**
    - [x] 基于 FATFS 的存储管理接口 (`storage_manager.c`)
    - [x] CSV 数据持久化存储逻辑
    - [x] MQTT JSON 负载封装逻辑 (`comm_manager.c`)
- [ ] **项目收尾与联调**

---
## 📝 后续行动计划
1. **完善 4G 模块 (SIM7600/Air780) AT 指令驱动**。
2. **细化 LVGL UI 具体界面设计**。
3. **整体项目构建与联调测试**。
