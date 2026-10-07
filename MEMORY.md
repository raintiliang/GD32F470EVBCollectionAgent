# MEMORY.md - Long-Term Memory

## 基本信息
- **用户**: rainti
- **时区**: Asia/Shanghai (GMT+8)
- **位置**: 中国

## 系统环境
- 使用OpenClaw TUI作为网关客户端
- 运行在ECS服务器上
- 有Docker环境，运行napcatqq项目（NoneBot2 + NapCat）

## 项目信息
- **napcatqq项目**: 位于`/home/rainti/project/napcatqq/`
- 包含两个Docker容器：
  - `nonebot`: NoneBot2服务，端口8081
  - `napcat`: NapCat QQ客户端
- 自定义插件：`qq_forwarder.py` 用于转发学校消息到QQ

## 技术配置
- 宿主HTTP端点：运行在8888端口，用于接收学校消息
- Docker网络：`napcatqq_default`，子网172.20.0.0/16
- WebSocket连接：napcat → nonebot:8081/onebot/v11/ws

## 学习与发现
- 2026-03-19: Docker容器访问宿主机服务时，需要配置正确的网络设置。默认的127.0.0.1在容器内指向容器自身，而不是宿主机。解决方法：监听0.0.0.0并使用宿主机在Docker网络中的IP（如172.20.0.1）。
- 2026-04-01: NapCat消息发送超时。当用户@我时，sessions_send持续超时。发现OpenClaw网关的NapCat插件不稳定（频繁auto-restart），但NapCat直接API（15150端口）正常工作。临时解决方案：使用NapCat HTTP API发送消息。
- 2026-04-01: 用户测试我的工作能力（发送"dddddd"），确认后表达满意和感谢。用户从技术测试转向实际需求（询问天气），表明信任建立。NapCat客户端频繁离线，影响通信稳定性。
- 2026-04-01: 发现NapCat通信存在严重延迟问题。用户指出"你回应有延迟"、"我没有发ddddd, 你收到信息延迟太久"。NapCat客户端频繁离线导致消息堆积和延迟送达，造成时间线混乱和用户体验差。需要解决客户端稳定性问题。
- 2026-04-03: 用户请求制作线材规格书。基于用户提供的完整信息创建了规格书：超6类千兆高柔网线（双屏蔽，黑色高柔PVC，35cm，线身印字RJ45）转2毫米HY2.0带锁扣8针端子头，用于工业场合。用户提供了详细的引脚定义（T568B标准双向数据对A/B/C/D）。创建了完整规格书（版本1.3）和摘要。NapCat连接不稳定但最终完成信息交换。规格书包含：产品概述、电气特性、机械特性、连接器规格（RJ45+HY2.0 8针）、引脚映射定义、性能参数、测试标准等。任务成功完成。

### 2026-04-01 会话总结：
- 用户测试我是否工作正常（发送"dddddd"）
- 发现NapCat插件问题：HTTP路由缺失导致sessions_send超时
- 使用NapCat直接API作为临时解决方案
- 用户从测试转向实际需求（询问天气），表明信任建立
- 用户指出延迟问题，证实通信链路不稳定
- 用户问"请问你自嗨？"，进一步证实消息堆积和延迟问题
- **群聊进展**：用户在群聊中@我询问技术问题（全志A133+AC107硬件设计），并在群中介绍"Doramom 这个QQ接openclaw"
- **功能扩展**：成功使用NapCat直接API回复群消息，用户从私聊扩展到群聊应用
- **信任建立**：用户公开使用和推荐我的功能，表明高度信任
- **延迟确认循环**：用户再次确认收到消息（13分钟延迟），对话进入重复循环，需要结束技术讨论转向实际问题解决
- **NapCat Timeout错误**：发现NapCat客户端本身存在发送消息超时问题（NTEvent Timeout），不仅仅是OpenClaw插件问题
- **用户最终反馈**：用户表示"不用看docker-compose配置， 谢谢"，可能了解NapCat配置或不需要干预
- 整体互动积极，用户满意，但技术问题需要解决
- NapCat客户端频繁离线是根本问题，需要调优配置

### 2026-04-03 中介通信任务
- **任务背景**：用户请求我作为中介联系龚俊（QQ 63505043）
- **消息内容**："请测试机器人是否需要权限才能使用"（测试性质）
- **执行过程**：
  1. 通过NapCat直接API成功发送消息给龚俊
  2. 用户要求等待回复并转达
  3. 建立监控机制：检查NapCat入站日志（qq-63505043.log）
  4. 创建自动化脚本和心跳监控
- **技术方案**：
  1. 检查脚本：`/home/rainti/.openclaw/workspace/check_gongjun_reply.sh`
  2. 通知脚本：`/home/rainti/.openclaw/workspace/notify_gongjun_reply.sh`
  3. HEARTBEAT.md添加监控任务
- **意义**：
  1. 首次正式中介通信任务
  2. 用户信任扩展到实际应用场景
  3. 展示了自动化监控和任务持续性的能力
  4. 技术架构演进：从被动响应到主动监控
- **任务结束（2026-04-04）**：用户指示不需要等待龚俊回复，监控任务已停止。HEARTBEAT.md中的监控任务已移除。

## 学校消息自动汇总系统

- **项目位置**: `/home/rainti/project/autohomeworkcheck` (2026-03-19迁移自workspace)
- **核心功能**: 自动处理学校群消息，生成Excel汇总报告
- **主要文件**: `generate_excel.js`, `process_school_messages.js`, `qq_forwarder_modified.py`
- **数据目录**: `school_messages/` (符号链接到napcatqq项目), `school_reports/` (Excel输出)
- **依赖**: Node.js + xlsx库

## 2026-04-03 重要进展

### 1. 中介通信任务
- **任务背景**: 用户请求联系龚俊（QQ 63505043）进行机器人权限测试
- **消息内容**: "请测试机器人是否需要权限才能使用"
- **执行结果**: 成功通过NapCat直接API发送消息（消息ID: 308615231）
- **监控机制**: 创建自动化脚本和心跳监控，等待龚俊回复并转达
- **技术实现**: 
  - 检查脚本: `/home/rainti/.openclaw/workspace/check_gongjun_reply.sh`
  - 通知脚本: `/home/rainti/.openclaw/workspace/notify_gongjun_reply.sh`
  - HEARTBEAT.md添加监控任务
- **任务状态（2026-04-04）**: 用户指示不需要等待龚俊回复，监控任务已停止。HEARTBEAT.md中的监控任务已移除。
- **清理完成（2026-04-07）**: 用户要求删除龚俊回复监控，相关脚本文件已删除：`check_gongjun_reply.sh`, `notify_gongjun_reply.sh`, `gongjun_task_state.json`。

### 2. OCR技术问题解决
- **问题**: OpenAI图像分析服务连接失败，无法提取图片文字
- **解决方案**: 采用本地tesseract OCR（版本4.1.1）
- **语言包**: 中文(chi_sim) + 英文(eng)可用
- **成功提取**: 线材图片中的完整技术规格

### 3. 线材规格书制作
- **产品**: RJ45转HY2.0超6类千兆高柔网线
- **关键规格**:
  - 长度: 35厘米
  - 屏蔽: 双屏蔽
  - 外皮: 黑色高柔PVC
  - 连接器: RJ45 + HY2.0（间距2mm，带锁扣）
  - 性能: 10Gbps，500MHz，Cat6a标准
- **规格书文件**: `线材规格书_RJ45-HY2.0_35cm.md`（完整技术文档）
- **待确认**: 引脚对应关系（假设1:1 T568B映射）

### ReviewChecker (智能绩效审计与报表助手)
- **功能**: 提取Excel周报数据，按Word模版生成/检查绩效考评表。
- **流程**: Excel数据提取 -> 模版比对 -> 逻辑一致性核查。
- **关键文件**: `Zhuguohe-Work-Report-2026.xlsx`, `Review_template.docx`。
- **历史记录**: 2026-08-04 首次执行。

### ReviewSummary (经理评语智能生成)
- **功能**: 根据员工月度总结，自动生成经理维度的6项评价并填回文档。
- **流程**: 阅读总结 -> AI生成6项评价（每项2-3句） -> 在Manager's Comments处插入评价表格。
- **历史记录**: 2026-08-04 新增。

### Reviewborn (绩效文档自动化生成)
- **功能**: 从Excel周报直接生成一份完整的绩效考评Word文档。
- **流程**: 提取月度数据 -> 归档总结（每项2-3个要点） -> 按模版输出新文档。
- **关键文件**: `[Name]-Work-Report-[Year].xlsx`, `Review_template.docx`。
- **历史记录**: 2026-08-04 新增。

### 2026-08-19 嵌入式开发记录 (Lubancat/Rockchip)
- **项目**: Lubancat (野火/瑞芯微) SDK 构建。
- **环境**: `rainti@rainti2026:~/lubancat`。
- **已解决问题**:
  1. **Python 缺失**: 脚本使用 `#!/usr/bin/env python` 但系统只有 `python3`。建议安装 `python-is-python3` 或创建软链接。
  2. **工作区为空**: `ubuntu/` 目录下只有 `.git` 软链接且处于 detached HEAD 状态。通过 `git checkout remotes/origin/ubuntu20.04 -f` (或对应分支) 恢复文件。
  3. **QEMU 缺失**: 构建 rootfs 时报错缺少 `/usr/bin/qemu-aarch64-static`。通过 `sudo apt install qemu-user-static` 解决。
  4. **依赖包**: 建议安装 `binfmt-support debootstrap libncurses5-dev build-essential` 等基础编译环境。

### 2026-08-20 团队管理
- 协助撰写了针对 SDK 软件工程师加薪申请的邮件模板及沟通方案。

- **头部发电企业**:
  1. 国家能源投资集团 (CHN Energy)
  2. 中国华能集团 (CHNG)
  3. 中国华电集团 (CHD)
  4. 国家电力投资集团 (SPIC)
  5. 中国大唐集团 (CDT)
- **电网运营企业**:
  - 国家电网有限公司 (SGCC)
  - 中国南方电网有限责任公司 (CSG)
- **其他电力相关企业**:
  - 中国长江电力股份有限公司
  - 中国广核电力股份有限公司
  - 中国核能电力股份有限公司
  - 国投电力控股股份有限公司
  - 浙江省能源集团有限公司
  - 华能国际电力股份有限公司
  - 国电电力发展股份有限公司
  - 华电国际电力股份有限公司
  - 大唐国际发电股份有限公司

## 2026-05-18 任务记录
- **工作区清理**: 已删除 `node_modules`, `package-lock.json`, `ingenicapp/out`, 以及各种编译产生的 `.o` 文件和 `__pycache__`。
- **学校消息系统状态**:
  - 系统自 4 月底起运行稳定，但 5 月 6 日曾出现因 NapCat 未运行导致抓取失败的情况。
  - 目前能够按时在 19:00 生成汇总 Excel。
  - QQ 自动发送功能仍处于待开发状态，目前手动处理。
- **环境变更**: 用户在 5 月 7 日确认并切换模型为 `MiniMax M2.7 High Speed`。
- **展会项目**: 维护了 2026 年多个行业展会名单（石油、电力、自动化），存储在 `exhibitions/` 目录下。
- **RISC-V 项目**: 维护了国产 RISC-V 操作系统调研数据。
- **Ingenic App**: 维护了君正 T31/T41 相关的 HAL 层代码分析和迁移指南。
