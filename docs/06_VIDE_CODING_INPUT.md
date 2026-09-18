# 材料测试设备 — Vide Coding 输入信息整理
**文档编号:** MTD-VIDE-001  
**版本:** V1.0  
**日期:** 2026-09-18

> 本文档是 Vide Coding（视频编程/远程编程协作）的输入信息，供AI编程助手或远程工程师快速理解项目上下文。

---

## 1. 项目一句话描述

**材料加速老化测试系统**：在受控CO₂浓度和湿度环境下对材料样品进行持续反应测试，支持本地屏控制 + 4G远程Web监控。

---

## 2. 硬件配置速查

```
传感器:
  CO2:  UART (9600bps)  Sensirion S8-005 或 MG-811
  温湿度: I2C (0x38)   AHT10 / SHT40

执行器:
  电磁阀(CO2): GPIO  高=充气, 低=停止
  加湿:         GPIO  高=开启, 低=停止
  除湿:         RS485 Modbus RTU (9600bps)

通信:
  4G模块: UART  MQTT协议  MQTT Broker地址可配置

存储:
  TF卡: SPI/SDIO  FAT32  CSV格式  1次/分钟

显示:
  5寸TFT  480×272 或 800×480  RGB/SPI接口
  4~6个按键 (设置/确认/上/下/启动/停止)
```

---

## 3. 固件关键代码片段

### 3.1 I2C 温湿度采集 (AHT10)
```c
// I2C地址: 0x38
#define AHT10_ADDR 0x38
#define AHT_CMD_TRIGGER 0xAC

float aht10_read_temperature(void) {
    uint8_t cmd[2] = {AHT_CMD_TRIGGER, 0x33, 0x00};
    HAL_I2C_Master_Transmit(&hi2c1, AHT10_ADDR<<1, cmd, 2, 100);
    uint8_t data[6];
    HAL_I2C_Master_Receive(&hi2c1, AHT10_ADDR<<1, data, 6, 100);
    uint32_t raw_temp = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
    return (raw_temp / 65536.0) * 200.0 - 50.0;
}
```

### 3.2 CO2 UART 解析
```c
// 9600bps, 8N1, 传感器主动发送 "425 850 26.3 65.2\r\n"
int parse_co2_frame(const char *frame, int *co2, float *temp, float *humid) {
    // 格式: "425 850 26.3 65.2"
    int scanned = sscanf(frame, "%*d %d %f %f", co2, temp, humid);
    return (scanned == 3) ? 0 : -1;
}
```

### 3.3 PID 控制
```c
typedef struct {
    float kp, ki, kd;
    float setpoint;
    float integral;
    float prev_error;
    uint16_t output_min;
    uint16_t output_max;
} PID_Controller;

uint16_t pid_update(PID_Controller *pid, float pv) {
    float error = pid->setpoint - pv;
    pid->integral += pid->ki * error;
    pid->integral = CLAMP(pid->integral, pid->output_min, pid->output_max);
    float derivative = pid->kd * (error - pid->prev_error);
    uint16_t output = (uint16_t)(pid->kp * error + pid->integral + derivative);
    pid->prev_error = error;
    return CLAMP(output, pid->output_min, pid->output_max);
}
```

### 3.4 FSM 状态机
```c
typedef enum { STATE_IDLE, STATE_REACTING, STATE_PAUSED, STATE_ALARM }反应状态_t;

void fsm_handle(反应状态_t *state, fsm_event_t event) {
    switch(*state) {
        case STATE_IDLE:
            if(event == EV_START) *state = STATE_REACTING;
            break;
        case STATE_REACTING:
            if(event == EV_PAUSE) *state = STATE_PAUSED;
            else if(event == EV_STOP) *state = STATE_IDLE;
            else if(event == EV_ALARM) *state = STATE_ALARM;
            break;
        case STATE_PAUSED:
            if(event == EV_RESUME) *state = STATE_REACTING;
            else if(event == EV_STOP) *state = STATE_IDLE;
            break;
        case STATE_ALARM:
            if(event == EV_ACK) *state = STATE_IDLE;
            break;
    }
}
```

### 3.5 MQTT 发布 (参考paho-mqtt风格)
```c
#define TOPIC_TELEMETRY "maats/%s/telemetry"
#define TOPIC_CONTROL "maats/%s/control"

char payload[256];
int len = snprintf(payload, sizeof(payload),
    "{\"co2\":%d,\"temp\":%.1f,\"humid\":%.1f,\"state\":%d}",
    co2_val, temperature, humidity, current_state);

mqtt_publish(TOPIC_TELEMETRY, device_id, payload, len, QOS_1);
```

### 3.6 CSV 写入 TF卡
```c
// FATFS + f_printf
FRESULT write_data_record(uint32_t timestamp, int co2, float temp, float humid, uint8_t state) {
    FIL fil;
    char line[128];
    datetime_t dt = timestamp_to_datetime(timestamp);
    snprintf(line, sizeof(line),
        "%04d-%02d-%02dT%02d:%02d:%02d,%d,%.1f,%.1f,%d\n",
        dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec,
        co2, temp, humid, state);
    FRESULT fr = f_open(&fil, get_current_csv_path(), FA_OPEN_APPEND | FA_WRITE);
    if(fr != FR_OK) return fr;
    UINT bw;
    fr = f_write(&fil, line, strlen(line), &bw);
    f_close(&fil);
    return fr;
}
```

---

## 4. 服务器后端关键信息

### 4.1 API 端点速查
```
POST /api/auth/login          登录 → JWT
GET  /api/realtime            WebSocket 实时数据
GET  /api/current             当前传感器值
GET  /api/history             历史数据 (分页+日期过滤)
GET  /api/history/download    下载CSV
PUT  /api/parameters          设置CO2/湿度目标值
POST /api/control             发送 start/pause/stop/resume
GET  /api/device/info         设备信息
```

### 4.2 MQTT 主题
```
maats/{device_id}/telemetry   设备→服务器 (传感器数据)
maats/{device_id}/control     服务器→设备 (控制命令)
maats/{device_id}/setpoint   服务器→设备 (目标值)
maats/{device_id}/ack        设备→服务器 (命令确认)
```

### 4.3 WebSocket 消息格式
```json
// 服务端 → 浏览器
{ "type": "telemetry", "co2": 1050, "temperature": 25.3,
  "humidity": 68.5, "state": "REACTING", "elapsed_seconds": 3600,
  "timestamp": "2026-09-18T12:00:00Z" }

// 浏览器 → 服务器 (心跳)
{ "type": "ping" }
```

---

## 5. Web前端关键信息

### 5.1 技术栈
```
React 18 + Vite + Ant Design Pro 5.x + ECharts + Zustand + React Router v6
```

### 5.2 核心页面路由
```
/login       登录
/dashboard   实时仪表板
/history     历史数据
/settings    参数设置
/control     控制面板
/device-info 设备信息
```

### 5.3 WebSocket 连接
```javascript
const ws = new WebSocket(`wss://your-server.com/api/realtime?token=${token}`);
ws.onmessage = (e) => {
  const data = JSON.parse(e.data);
  if(data.type === 'telemetry') updateDashboard(data);
};
ws.onclose = () => reconnect();
```

### 5.4 ECharts 实时曲线配置
```javascript
option = {
  xAxis: { type: 'time', name: '时间' },
  yAxis: [
    { name: 'CO2(ppm)', type: 'value' },
    { name: '温度(°C)', type: 'value' },
    { name: '湿度(%)', type: 'value' }
  ],
  series: [
    { name: 'CO2', type: 'line', data: [] },
    { name: '温度', type: 'line', data: [] },
    { name: '湿度', type: 'line', data: [] }
  ]
};
```

---

## 6. 快速命令参考

### 固件
```bash
# 编译
make -j4

# 烧录 (ST-Link)
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/app.elf verify reset"

# 串口日志
minicom -D /dev/ttyUSB0 -b 115200
```

### 后端
```bash
cd backend
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000

# MQTT测试
mosquitto_pub -h iot.example.com -t maats/TEST001/telemetry -m '{"co2":850}'
```

### 前端
```bash
cd frontend
npm install
npm run dev -- --host 0.0.0.0 --port 3000
```

---

## 7. 常见错误排查

| 现象 | 可能原因 | 解决方法 |
|------|---------|---------|
| CO2读数一直为0 | UART接线反了/波特率不对 | 示波器确认TX/RX, 确认9600bps |
| 温湿度I2C NACK | 地址错误/接线问题 | 用I2C Scanner确认0x38 |
| MQTT连接失败 | APN错/服务器地址错 | 检查SIM卡状态, ping服务器 |
| TF卡不识别 | 文件系统未格式化 | 用PC格式化为FAT32 |
| WebSocket断线 | JWT过期/服务器重启 | 重新登录获取新token |
| PID震荡 | Kp过大/积分饱和 | 降低Kp, 加积分限幅 |
