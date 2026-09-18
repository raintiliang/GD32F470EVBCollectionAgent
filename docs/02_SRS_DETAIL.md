# 材料测试设备 — 软件详细需求规格说明书
**文档编号:** MTD-SRS-DETAIL-001  
**版本:** V1.0  
**日期:** 2026-09-18  
**状态:** 草稿

---

## 1. 系统概述

### 1.1 系统边界

```
┌───────────────────────────────────────────────────────────────┐
│                      MAATS 软件系统                            │
│                                                                │
│  ┌──────────┐    ┌──────────┐    ┌──────────────────────┐  │
│  │ 设备固件  │    │ 服务器后端 │    │   Web客户端           │  │
│  │(FreeRTOS)│◄──►│ (FastAPI)│◄──►│   (React+AntD)       │  │
│  └──────────┘    └──────────┘    └──────────────────────┘  │
│       │                 │                     │               │
│       │  4G/MQTT       │  HTTPS/WSS          │  HTTPS       │
│       └────────────────┴─────────────────────┘               │
│                        │                                      │
│                   ┌────▼────┐                                 │
│                   │   用户  │                                  │
│                   └─────────┘                                  │
└───────────────────────────────────────────────────────────────┘
```

### 1.2 技术选型理由

| 组件 | 选型 | 理由 |
|------|------|------|
| 设备固件 | FreeRTOS | 成熟实时OS，多任务支持，生态丰富 |
| 服务器框架 | **FastAPI** | 异步高性能，自动化API文档，WebSocket原生支持，适合IoT |
| 前端框架 | React + Vite | 生态成熟，组件丰富，Vite快速构建 |
| UI组件库 | Ant Design Pro | 企业级 dashboard，开箱即用 |
| 图表库 | ECharts | 支持大数据量平滑曲线，性能好 |
| 数据库 | SQLite | 零运维，适合中小规模嵌入式IoT场景 |
| 实时推送 | WebSocket | 优于轮询，低延迟 |
| 认证 | JWT | 无状态，适合分布式/移动端 |

---

## 2. 设备端固件详细需求

### 2.1 任务清单与时序

#### 2.1.1 任务配置
| 任务名 | 优先级(1最高) | 栈大小 | 周期 | 说明 |
|--------|--------------|--------|------|------|
| KeyScanTask | 5 | 256B | 100ms | 按键扫描，消抖40ms |
| DisplayTask | 5 | 4KB | 1Hz | 屏幕刷新，FPS≥1 |
| SensorTask | 3 | 2KB | 1s | 传感器采集 |
| ActuatorTask | 2 | 1KB | 500ms | 执行器PID控制 |
| ControlTask | 1 | 2KB | 200ms | 闭环控制逻辑 |
| StorageTask | 4 | 4KB | 60s | TF卡CSV写入 |
| CommTask | 2 | 4KB | 5s | 4G/MQTT通讯 |

#### 2.1.2 状态机
```
IDLE ──[启动按钮]──► REACTING ──[暂停按钮]──► PAUSED
  ▲                      │                        │
  │                      │                        │
  └──[停止按钮/完成]───────┴─────[继续按钮]────────┘
```

| 状态 | 说明 |
|------|------|
| IDLE | 待命中，传感器继续采集但不进行控制 |
| REACTING | 反应进行中，PID控制CO2和湿度 |
| PAUSED | 暂停，PID输出置零，阀门关闭 |
| ALARM | 报警（超限），等待人工确认 |

#### 2.1.3 PID 控制参数
| 控制对象 | 目标 | PID参数 (参考) |
|---------|------|---------------|
| CO2浓度 | 可设置 (默认 1000 ppm) | Kp=0.8, Ki=0.1, Kd=0.2 |
| 湿度 | 可设置 (默认 70%RH) | Kp=1.0, Ki=0.15, Kd=0.3 |
| 控制周期 | 200ms | — |

### 2.2 传感器驱动

#### 2.2.1 CO2传感器 (UART)
```
协议: 主动发送型, 9600bps, 8N1
命令帧: 无 (传感器主动1次/秒)
数据帧示例: "425 850 26.3 65.2\r\n" (CO2=850ppm, T=26.3°C, Hum=65.2%)
```

#### 2.2.2 温湿度传感器 (I2C)
```
型号: AHT10
I2C地址: 0x38
寄存器:
  - 0xAC (触发测量)
  - 0xE5 (读温湿度)
换算公式:
  Humidity = (raw/2^20) * 100 [%RH]
  Temperature = ((raw & 0xFFF000) >> 12) / 256 - 50 [°C]
```

### 2.3 执行器驱动

#### 2.3.1 电磁阀 (GPIO)
```
控制IO: GPIOx (推挽输出)
启动延迟: ≤ 200ms
关闭延迟: ≤ 100ms
状态反馈: 无 (开环控制)
```

#### 2.3.2 加湿/除湿 (GPIO + RS485)
```
加湿: GPIO, 高电平=ON
除湿: RS485 Modbus RTU
  - 功能码: 0x05 (写单个线圈)
  - 地址: 0x0001 = 开启除湿
  - 波特率: 9600, 8N1
```

### 2.4 存储规格

#### 2.4.1 文件命名
```
格式: YYYY-MM-DD.csv
示例: 2026-09-18.csv
路径: /maats_data/YYYY/MM/
```

#### 2.4.2 TF卡管理
- 单文件最大: 约 1MB/天 (86400条/分 → 1440行)
- 存储空间检查: 启动时检查剩余容量 < 100MB 报警
- 滚动删除: 保留最近30个文件

### 2.5 4G/MQTT 通讯

#### 2.5.1 MQTT配置
```python
BROKER = "iot.example.com"  # 可配置
PORT = 1883
CLIENT_ID = f"maats_{DEVICE_ID}"
KEEPALIVE = 30
QOS = 1 (设备→服务器), QOS=2 (服务器→设备)
```

---

## 3. 服务器后端详细需求

### 3.1 项目结构
```
backend/
├── main.py                 # FastAPI 入口
├── config.py               # 配置管理
├── auth/
│   ├── router.py           # 认证路由
│   ├── schemas.py          # Pydantic 模型
│   └── service.py          # 登录/Token逻辑
├── device/
│   ├── router.py           # 设备路由
│   ├── service.py          # MQTT订阅/设备命令
│   └── manager.py          # 设备连接状态管理
├── data/
│   ├── router.py           # 历史数据API
│   ├── service.py          # SQLite操作
│   └── schemas.py          # 数据模型
├── websocket/
│   ├── manager.py          # WebSocket连接管理
│   └── router.py           # WSS端点
├── models/
│   ├── user.py             # 用户模型
│   └── record.py           # 数据记录模型
├── database.py             # SQLite初始化
└── requirements.txt
```

### 3.2 数据库模型

```sql
-- 用户表
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(64) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    role VARCHAR(16) DEFAULT 'operator',  -- admin / operator
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 设备表
CREATE TABLE devices (
    id VARCHAR(64) PRIMARY KEY,
    name VARCHAR(128),
    device_key VARCHAR(128),          -- 设备认证密钥
    online_status BOOLEAN DEFAULT 0,
    last_seen DATETIME,
    co2_setpoint INTEGER DEFAULT 1000,
    humid_setpoint REAL DEFAULT 70.0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 实时数据快照 (最新)
CREATE TABLE latest_data (
    device_id VARCHAR(64) PRIMARY KEY,
    co2 INTEGER,
    temperature REAL,
    humidity REAL,
    state VARCHAR(16),
    timestamp DATETIME,
    FOREIGN KEY (device_id) REFERENCES devices(id)
);

-- 历史数据表 (分区/分表优化)
CREATE TABLE history_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id VARCHAR(64),
    co2 INTEGER,
    temperature REAL,
    humidity REAL,
    state VARCHAR(16),
    timestamp DATETIME,
    INDEX idx_device_time (device_id, timestamp)
);

-- 操作日志
CREATE TABLE operation_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id VARCHAR(64),
    username VARCHAR(64),
    action VARCHAR(32),       -- start/pause/stop/resume/set_param
    params TEXT,              -- JSON
    result VARCHAR(16),       -- success/fail
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### 3.3 API 详细规格

#### 3.3.1 登录
```
POST /api/auth/login
Body: { "username": "admin", "password": "xxxx" }
Response 200: { "access_token": "eyJ...", "token_type": "bearer", "expires_in": 86400 }
Response 401: { "detail": "Invalid credentials" }
```

#### 3.3.2 实时数据 (WebSocket)
```
GET /api/realtime?token={jwt}
WSS Response (JSON):
{
  "type": "telemetry",
  "device_id": "MAATS-001",
  "co2": 1050,
  "temperature": 25.3,
  "humidity": 68.5,
  "state": "REACTING",
  "elapsed_seconds": 3600,
  "timestamp": "2026-09-18T12:00:00Z"
}
```

#### 3.3.3 历史数据查询
```
GET /api/history?device_id=MAATS-001&start=2026-09-01&end=2026-09-18&page=1&page_size=1000
Response 200:
{
  "total": 43200,
  "page": 1,
  "page_size": 1000,
  "data": [
    {"timestamp": "2026-09-01T00:00:00Z", "co2": 850, "temperature": 24.1, "humidity": 65.2, "state": "REACTING"},
    ...
  ]
}
```

#### 3.3.4 远程控制
```
POST /api/control
Headers: Authorization: Bearer {jwt}
Body: { "device_id": "MAATS-001", "cmd": "pause" }
Response 200: { "ack": true, "cmd": "pause", "device_id": "MAATS-001" }
```

#### 3.3.5 参数设置
```
PUT /api/parameters
Body: { "device_id": "MAATS-001", "co2_sp": 1500, "humid_sp": 75.0 }
Response 200: { "success": true, "co2_sp": 1500, "humid_sp": 75.0 }
```

### 3.4 WebSocket 管理
- 同一个 JWT 只能有一个活跃连接（防止多端控制冲突）
- 连接超时: 24h 后自动断开，需重新登录
- 心跳: 客户端每 30s 发送 `ping`，服务器回复 `pong`

---

## 4. Web客户端详细需求

### 4.1 页面路由（PC/手机共用）
```
/login              -- 登录页 (无需认证)
/dashboard          -- 实时监控仪表板 (需认证)
/history            -- 历史数据查询 (需认证)
/settings           -- 参数设置 (需认证)
/control            -- 控制面板 (需认证)
/device-info        -- 设备信息 (需认证)
```

> **跨平台策略**: 同一套 React 代码，通过响应式布局同时适配 PC 和手机浏览器。无需开发原生APP。

### 4.2 响应式布局规格（PC + 手机自适应）

#### 4.2.1 断点与布局策略

| 断点 | 视口宽度 | 列数 | 数字卡片 | 曲线图 |
|------|---------|------|---------|--------|
| Mobile S | < 576px | 1列 | 全宽，垂直堆叠 | 全宽，高度 240px |
| Mobile L | 576px ~ 767px | 2列 | 2列 | 全宽，高度 280px |
| Tablet | 768px ~ 991px | 3列 | 3列 | 全宽，高度 320px |
| Desktop | 992px ~ 1439px | 3列 | 3列 | 全宽，高度 360px |
| Large Desktop | ≥ 1440px | 3列宽松 | 3列宽松 | 全宽，高度 400px |

#### 4.2.2 触摸交互要求

| 交互元素 | 触摸目标尺寸 | 实现方式 |
|---------|------------|---------|
| 主按钮 (启动/暂停/停止) | 宽度100% × 高度56px | `min-height: 56px` |
| 次要按钮 | 宽度100% × 高度44px | `min-height: 44px` |
| 表格行 | 高度44px | `line-height: 44px` |
| 导航菜单项 | 高度48px | 底部TabBar |
| 曲线时间轴 | 宽度100% × 高度48px | 滑动手势 |

#### 4.2.3 手机端特有交互

| 功能 | 实现 | 说明 |
|------|------|------|
| 下拉刷新 | `pull-to-refresh` | 刷新实时数据 |
| 底部TabBar | 固定底部导航 | 首页/历史/控制/我的 |
| 返回导航 | 顶部返回按钮 | React Router `useNavigate` |
| 滑动解锁控制 | 需滑动确认 | 防止误触危险操作 |
| 数字键盘 | `inputmode="numeric"` | 参数设置时弹出数字键盘 |
| 安全区域适配 | `env(safe-area-inset-*)` | iPhone刘海/底部横条 |
| 弱网提示 | 离线状态banner | `navigator.onLine` 监听 |
| 全屏震动 | `navigator.vibrate()` | 报警时手机震动反馈 |

#### 4.2.4 移动端组件适配

| PC组件 | 手机等效 | 框架处理 |
|--------|---------|---------|
| Ant Design Statistic | AntD Mobile Card | 响应式断点切换 |
| Ant Design Table | 卡片列表/虚拟列表 | `@ant-design/cssinjs` 媒体查询 |
| 桌面端ECharts | 移动端ECharts | 同一组件，touch事件适配 |
| 左侧导航栏 | 底部TabBar | React Router + Zustand |
| 鼠标Hover | 点击/触摸 | 所有交互用`onClick` |

---

### 4.3 PC端实时监控页面布局（Desktop ≥ 992px）

```
┌─────────────────────────────────────────────────────────┐
│  MAATS 材料测试系统    [用户: admin]  [退出登录]          │
├───────────────┬──────────────────┬──────────────────────┤
│ CO2浓度       │ 温度             │ 湿度                 │
│ 1050 ppm  ▲  │ 25.3°C  ─      │ 68.5%  ▼            │
│ [目标: 1000] │                  │ [目标: 70%]          │
├───────────────┴──────────────────┴──────────────────────┤
│  实时曲线 (最近60分钟)                                   │
│  ┌─────────────────────────────────────────────────┐  │
│  │ ~~~\                                         │  │
│  │      ~~~CO2  ~~~温  ~~~湿                     │  │
│  │          ─────────────────────────────────     │  │
│  └─────────────────────────────────────────────────┘  │
├───────────────────────────────────────────────────────  │
│ 反应状态: [REACTING]  已持续: 01:30:45                 │
│ [启动] [暂停] [停止] [重启]                              │
└─────────────────────────────────────────────────────────┘
```

### 4.4 手机端布局示意图（Mobile < 768px）

```
┌────────────────────────┐
│ MAATS  [admin]    ≡  │  ← 顶部Bar
├────────────────────────┤
│ ┌──────────────────┐  │
│ │   CO2  1050 ppm  │  │  ← 全宽卡片
│ │  ▲  目标: 1000   │  │
│ └──────────────────┘  │
│ ┌────────┐┌────────┐  │
│ │ 温度   ││ 湿度   │  │  ← 2列卡片
│ │ 25.3°C││ 68.5% │  │
│ └────────┘└────────┘  │
├────────────────────────┤
│ ┌──────────────┐     │
│ │   实时曲线    │     │  ← 全宽，高度240px
│ │  (可滑动缩放) │     │
│ └──────────────┘     │
├────────────────────────┤
│  REACTING  01:30:45  │  ← 状态栏
│ ┌────┐┌────┐┌────┐  │
│ │启动││暂停││停止│  │  ← 56px高大按钮
│ └────┘└────┘└────┘  │
├────────────────────────┤
│  🏠 │ 📊 │ ⚙️ │ 👤 │  ← 底部TabBar
└────────────────────────┘
```

### 4.5 关键组件规格

| 组件 | 规格 |
|------|------|
| 实时曲线 | ECharts line chart, 3条线(CO2/温度/湿度), 自动滚动刷新, 显示网格和tooltip |
| 数字卡片 | Ant Design Statistic, 含当前值/目标值/趋势箭头 |
| 控制按钮 | Ant Design Button, danger/success/warning 三态 |
| 历史曲线 | ECharts, 支持缩放(zoom), 时间范围选择器 |
| 加载状态 | Ant Design Spin + Skeleton |

### 4.6 响应式布局
| 断点 | 布局 |
|------|------|
| Desktop (≥1200px) | 3列数字卡片 + 宽图 |
| Tablet (768-1199px) | 3列数字卡片 + 窄图 |
| Mobile (<768px) | 单列堆叠，全宽图 |

---

## 5. 安全性需求

| 需求 | 实现 |
|------|------|
| 传输加密 | HTTPS (TLS 1.2+), WSS |
| 认证 | JWT, 24h过期, PyJWT |
| 密码存储 | bcrypt, salt轮数≥12 |
| CORS | 仅允许已知域名 |
| 输入校验 | Pydantic, 所有API参数验证 |
| SQL注入 | 参数化查询 (SQLite) |
| 控制权限 | 仅 admin/operator 角色可控制 |
| 操作审计 | 所有操作写入 operation_log |

---

## 6. 性能需求

| 指标 | 目标 |
|------|------|
| API响应时间 (P95) | ≤ 200ms |
| WebSocket延迟 (P95) | ≤ 500ms |
| 并发WebSocket连接 | ≥ 50 |
| 历史查询 (1000条) | ≤ 1s |
| 数据库容量 | 支持 ≥ 1000万条记录 |
| 内存占用 (服务器) | ≤ 256MB RAM |
