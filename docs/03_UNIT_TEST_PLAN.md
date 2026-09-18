# 材料测试设备 — 软件单元测试计划
**文档编号:** MTD-UTP-001  
**版本:** V1.0  
**日期:** 2026-09-18

---

## 1. 测试范围与策略

### 1.1 测试对象
| 层级 | 被测模块 | 核心技术 |
|------|---------|---------|
| 设备固件 | 传感器驱动、执行器驱动、 PID控制器、FSM状态机、存储模块 | C + Unity/CMock |
| 服务器后端 | API路由、业务逻辑、数据库操作、WebSocket管理 | Python + pytest |
| Web客户端 | 组件功能、API集成、ECharts交互 | JavaScript + Vitest/Jest |

### 1.2 覆盖率目标
| 层级 | 语句覆盖率 | 分支覆盖率 | 关键路径 |
|------|----------|-----------|---------|
| 设备固件 | ≥ 80% | ≥ 70% | PID/FSM 100% |
| 服务器后端 | ≥ 90% | ≥ 80% | API端点 100% |
| Web客户端 | ≥ 80% | ≥ 70% | 组件交互 100% |

---

## 2. 设备固件单元测试

### 2.1 测试框架
- **框架:** Unity + CMock (CMocka备选)
- **运行环境:** 本地GCC (MinGW/MSYS2) 或 CI上交叉编译
- **Mock策略:** 硬件外设(I2C/UART/GPIO)全部Mock

### 2.2 测试用例

#### 2.2.1 CO2传感器驱动 (`co2_sensor.c`)
| 用例ID | 描述 | 输入 | 预期输出 |
|--------|------|------|---------|
| UT-CO2-01 | 正常解析数据帧 | `"425 850 26.3 65.2\r\n"` | co2=850, temp=26.3, humid=65.2 |
| UT-CO2-02 | 帧不完整 | `"425 850 26.3"` | 返回上帧值或错误码 |
| UT-CO2-03 | CO2超量程 | `"425 6000 26.3 65.2"` | 标记异常但不崩溃 |
| UT-CO2-04 | 校验和错误帧 | `"000 100 00.0 00.0"` | 丢弃该帧 |

#### 2.2.2 温湿度传感器驱动 (`aht10.c`)
| 用例ID | 描述 | 输入 | 预期输出 |
|--------|------|------|---------|
| UT-AHT-01 | 正常读温湿度 | 模拟I2C返回0xAC+0xXX | 温湿度值在合理范围 |
| UT-AHT-02 | I2C NACK错误 | I2C返回NACK | 返回错误码，不死锁 |
| UT-AHT-03 | 温度越界 (<-40°C) | raw值全0 | 标记传感器故障 |
| UT-AHT-04 | 连续3次读取超时 | 超时×3 | 传感器断线处理 |

#### 2.2.3 PID控制器 (`pid_controller.c`)
| 用例ID | 描述 | 输入 | 预期输出 |
|--------|------|------|---------|
| UT-PID-01 | KP=0, KI=0, KD=0 | setpoint=1000, pv=800 | output=0 |
| UT-PID-02 | 纯比例控制 | KP=1.0, set=1000, pv=500 | 输出=500 |
| UT-PID-03 | 积分饱和 | 长时间 pv>>set | 输出不超过上限(255) |
| UT-PID-04 | 微分作用 | pv突变(step) | 输出瞬间反向抑制 |
| UT-PID-05 | 手动模式(输出清零) | disable() | integral term清零 |

#### 2.2.4 状态机 (`control_fsm.c`)
| 用例ID | 描述 | 当前态 | 事件 | 预期次态 |
|--------|------|--------|------|---------|
| UT-FSM-01 | 正常启动 | IDLE | EV_START | REACTING |
| UT-FSM-02 | 暂停反应 | REACTING | EV_PAUSE | PAUSED |
| UT-FSM-03 | 继续反应 | PAUSED | EV_RESUME | REACTING |
| UT-FSM-04 | 停止复位 | REACTING/PAUSED | EV_STOP | IDLE |
| UT-FSM-05 | 超限报警 | REACTING | EV_ALARM | ALARM |
| UT-FSM-06 | ALARM确认 | ALARM | EV_ACK | IDLE |
| UT-FSM-07 | IDLE下启动 | IDLE | EV_START | REACTING |
| UT-FSM-08 | ALARM下启动 | ALARM | EV_START | 拒绝(保持ALARM) |

#### 2.2.5 CSV存储 (`storage.c`)
| 用例ID | 描述 | 输入 | 预期输出 |
|--------|------|------|---------|
| UT-STO-01 | 正常写一条记录 | 有效数据 | 文件增加一行CSV |
| UT-STO-02 | 文件系统满 | 返回-ENOSPC | 报警+停止记录 |
| UT-STO-03 | 文件未闭合时重启 | 写N条后断电模拟 | FAT恢复，无数据丢失 |
| UT-STO-04 | 文件名格式 | 时间2026-09-18 10:00 | 文件名`2026-09-18.csv` |
| UT-STO-05 | 滚动删除 | 已有31个文件 | 删除最旧文件 |

#### 2.2.6 MQTT客户端 (`mqtt_client.c`)
| 用例ID | 描述 | 输入 | 预期输出 |
|--------|------|------|---------|
| UT-MQ-01 | 连接成功 | CONNECT帧ACK | 在线状态置位 |
| UT-MQ-02 | 连接超时 | Broker无响应 | 重连退避(30s/60s/120s) |
| UT-MQ-03 | 发布遥测数据 | valid JSON payload | QoS=1发布成功 |
| UT-MQ-04 | 收到控制命令 | `{"cmd":"pause"}` | 解析并触发FSM事件 |
| UT-MQ-05 | 非法JSON | `{invalid}` | 丢弃并记录错误 |

### 2.3 测试环境
```
宿主机: Windows x64 (MSYS2/MinGW) 或 Linux
编译器: gcc 10+
框架: Unity v2.5.x + CMock
覆盖率: gcov + lcov (HTML报告)
CI: GitHub Actions / Gitee Go
```

---

## 3. 服务器后端单元测试

### 3.1 测试框架
- **框架:** pytest + pytest-asyncio
- **数据库:** SQLite in-memory (测试隔离)
- **Mock:** unittest.mock (MQTT/设备)
- **覆盖率:** pytest-cov

### 3.2 测试用例

#### 3.2.1 认证模块 (`auth/`)
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-AUTH-01 | 正确登录 | admin/正确密码 | 返回有效JWT |
| UT-AUTH-02 | 错误密码 | admin/错误密码 | 401 |
| UT-AUTH-03 | 用户不存在 | unknown/xxx | 401 |
| UT-AUTH-04 | JWT过期 | 过期token | 401 Token expired |
| UT-AUTH-05 | 伪造JWT | 无效签名 | 401 Invalid token |
| UT-AUTH-06 | Token刷新 | 有效token请求/me | 返回用户信息 |

#### 3.2.2 设备API (`device/`)
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-DEV-01 | 查询设备信息 | 有效device_id | 返回设备信息 |
| UT-DEV-02 | 设备不在线 | offline设备 | 返回online=0 |
| UT-DEV-03 | 非法设备ID | device_id不存在 | 404 |
| UT-DEV-04 | 无权限修改 | 普通用户写管理API | 403 |

#### 3.2.3 历史数据API (`data/`)
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-DATA-01 | 分页查询 | page=1, size=100 | 返回100条+total |
| UT-DATA-02 | 日期范围过滤 | start&end | 仅返回范围内数据 |
| UT-DATA-03 | 空结果 | 无数据日期 | 返回空数组 |
| UT-DATA-04 | 越界参数 | page_size=99999 | 限制到MAX=5000 |
| UT-DATA-05 | 导出CSV | 有效日期范围 | Content-Type: text/csv |

#### 3.2.4 控制API (`device/control`)
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-CTL-01 | 发送启动命令 | device在线 | MQTT发布control主题 |
| UT-CTL-02 | 设备不在线 | device离线 | 返回离线状态+重试提示 |
| UT-CTL-03 | 无效命令 | cmd="unknown" | 422 参数校验错误 |
| UT-CTL-04 | 操作日志记录 | 任意控制命令 | operation_log有记录 |

#### 3.2.5 WebSocket管理器
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-WS-01 | 有效token连接 | 有效JWT | 连接建立,发送实时数据 |
| UT-WS-02 | token无效 | 伪造token | 连接拒绝400 |
| UT-WS-03 | 同一token多连接 | token1×2 | 第二连接被拒绝(冲突) |
| UT-WS-04 | 设备数据推送 | MQTT收到数据 | 所有WS客户端收到相同数据 |
| UT-WS-05 | 客户端断线 | 意外断开 | 资源清理,无泄漏 |

### 3.3 测试环境
```
Python: 3.10+
依赖: pytest, pytest-asyncio, httpx (async client)
数据库: SQLite :memory:
Mock: unittest.mock + pytest-mock
覆盖率: pytest-cov (阈值: line ≥ 90%)
```

---

## 4. Web客户端单元测试

### 4.1 测试框架
- **框架:** Vitest + React Testing Library
- **组件测试:** Jest + @testing-library/react
- **E2E (可选):** Playwright
- **覆盖率:** V8 / Istanbul (via Vitest)

### 4.2 测试用例

#### 4.2.1 登录页
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-WEB-01 | 正常登录 | 正确用户名/密码 | 跳转/dashboard |
| UT-WEB-02 | 登录失败 | 错误密码 | 显示错误提示 |
| UT-WEB-03 | 记住登录 | 勾选"记住我" | 7天免登录 |
| UT-WEB-04 | 输入校验 | 空用户名 | 表单验证失败 |

#### 4.2.2 实时仪表板
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-WEB-05 | 显示实时数据 | WebSocket连接成功 | 数字卡片更新 |
| UT-WEB-06 | 离线状态 | 设备离线 | 显示红色离线标识 |
| UT-WEB-07 | 控制按钮权限 | operator角色 | 无"设置"按钮 |
| UT-WEB-08 | 控制按钮权限 | admin角色 | 有"设置"按钮 |

#### 4.2.3 控制面板
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-WEB-09 | 启动反应 | 点击启动 | 按钮loading→成功提示 |
| UT-WEB-10 | 启动失败 | 设备离线 | 显示错误toast |
| UT-WEB-11 | 暂停反应 | 点击暂停 | 状态变为PAUSED |
| UT-WEB-12 | 参数设置 | 设置CO2=1500 | 成功保存 |

#### 4.2.4 历史数据查询
| 用例ID | 描述 | 输入 | 预期 |
|--------|------|------|------|
| UT-WEB-13 | 曲线渲染 | 加载1000条数据 | ECharts正确显示 |
| UT-WEB-14 | 日期选择 | 选择日期范围 | 仅显示范围内数据 |
| UT-WEB-15 | 导出Excel | 点击导出 | 下载.csv文件 |
| UT-WEB-16 | 空数据 | 日期范围无数据 | 显示"暂无数据" |

### 4.3 测试环境
```
Node.js: 18+
框架: Vitest + React Testing Library
HTTP Mock: msw (Mock Service Worker)
ECharts Mock: jest-canvas-mock
```

---

## 5. 测试执行计划

| 阶段 | 时间 | 内容 |
|------|------|------|
| UT-1 | W2 | 设备固件驱动层测试 (CO2/I2C/GPIO) |
| UT-2 | W3 | 设备固件逻辑层测试 (PID/FSM/Storage) |
| UT-3 | W3 | 服务器认证+API测试 |
| UT-4 | W4 | 服务器数据+WebSocket测试 |
| UT-5 | W4 | Web客户端组件测试 |
| UT-6 | W5 | 全量回归测试 |

> **W = 开发周期第N周**
