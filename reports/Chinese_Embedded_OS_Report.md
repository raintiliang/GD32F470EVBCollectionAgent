# 中国工业与汽车芯片平台嵌入式操作系统调研报告

> **调研日期：** 2026-05-20
> **用途：** 项目路线图规划 · OS选型参考 · SIL认证目标
> **范围：** 中国市场 · 工业 / 汽车芯片平台

---

## 一、调研说明

本报告系统梳理了在中国工业与汽车芯片平台上当前使用的主要嵌入式操作系统，按统一格式（8个维度）为每个OS平台建立档案，供项目路线图与OS选型决策参考。

**8个维度说明：**
1. **OS类型** — RTOS / Linux类应用OS / 整车OS / Hypervisor等
2. **部署领域** — 工业控制 / 汽车MCU / 座舱 / ADAS / IoT / 电网等
3. **成熟度** — 原型 / 生产验证 / 规模量产
4. **开源/商业** — 开源协议（Apache/Mulan/GPL等）或专有
5. **安全认证** — ISO 26262 / IEC 61508 / ASIL等级
6. **RISC-V支持** — BSP/port状态
7. **社区规模** — GitHub/Gitee数据、基金会支持、主要贡献者
8. **主要用户** — OEM/Tier1/工业客户（已知）

---

## 二、OS平台详细档案

---

### 2.1 RT-Thread（睿赛德操作系统）

| 维度 | 详情 |
|---|---|
| **OS类型** | 开源RTOS（实时操作系统），含完整中间件组件和IoT平台 |
| **部署领域** | 智能家居、安防、工业自动化、车载MCU、穿戴设备、智慧城市、能源、医疗、消费电子 |
| **成熟度** | **规模量产** — 装机量超**20亿台**，开发者超20万（2024），80+芯片厂商官方支持 |
| **开源/商业** | **开源** — Apache 2.0许可（商业友好）；由睿赛德科技维护 |
| **安全认证** | **ISO 26262 ASIL-D**（TÜV SÜD认证）；RT-Thread已推出车规版本 |
| **RISC-V支持** | ✅ **深度适配** — 平头哥玄铁全系列（C906/C908/C908X/R908A）已完成适配，提供RTOS SDK；HPMicro先楫半导体HPM6000系列官方支持；Gitee/Cloud-Huawei等均有BSP |
| **社区规模** | GitHub: rt-thread主仓10K+ stars，34个仓库；国内最大嵌入式开源社区；CSDN年度文章近2000篇；300+软件包生态；2024年GitHub stars突破1万 |
| **主要用户** | 先楫半导体（HPMicro）、匠芯创、ST（意法半导体）、国际芯片厂商；车载领域：睿赛德"程翧"平台已与多家车企合作，目标2025-2026量产 |

**补充说明：**
- 诞生于2006年，国内历史最长的开源RTOS之一
- 支持POSIX、CMSIS标准接口，兼容性强
- "程翧"车载融合软件平台：2023-2024年商业化量产，2025-2026年广泛部署

---

### 2.2 SylixOS（翼辉操作系统）

| 维度 | 详情 |
|---|---|
| **OS类型** | 商业大型实时操作系统（RTOS），多核64位，高可靠任务关键型 |
| **部署领域** | 航空航天、轨道交通、航空电子、汽车电子、电力电网、工业自动化、网络设备、国防 |
| **成熟度** | **规模量产** — 二十年代际积累，200+硬件平台支持，核心行业长期稳定运行 |
| **开源/商业** | **专有商业** — 翼辉信息拥有完整自主知识产权；内核自主化率100%，超200万行代码 |
| **安全认证** | ✅ **最全面的安全认证组合**：ISO 26262 **ASIL-D**（TÜV SÜD）、IEC 61508 **SIL3**（TÜV SÜD）、EN 50128 **SIL4**（TÜV SÜD）、DO-178C（航空）、支持适航认证 |
| **RISC-V支持** | ✅ **已适配** — 翼辉已官方支持RISC-V架构（含玄铁系列）；作为高可靠RTOS，RISC-V版本面向工业/车载关键任务 |
| **社区规模** | 封闭商业社区；翼辉官网提供文档与支持；主论坛为中国软件行业协会嵌入式专委；非开源，社区规模以企业级合作为主 |
| **主要用户** | 航空航天（火箭卫星姿态控制，商用卫星在轨800天+无故障）、轨道交通（高铁/地铁信号系统）、国家电网/电站、汽车OEM/Tier1（动力/底盘/网关）、防务单位 |

**补充说明：**
- 翼辉信息2021年完成1.5亿元B轮融资（国风投、深创投、中车投资等）
- 2018年通过DO-178C航空软件认证，成为首个符合ARINC 653标准的国产操作系统
- VSOA分布式软总线已获ISO 26262 ASIL-D认证

---

### 2.3 Intewell（科银京成操作系统）

| 维度 | 详情 |
|---|---|
| **OS类型** | 商业RTOS + 虚拟化架构，支持GPOS+RTOS混合部署（实时/非实时融合） |
| **部署领域** | 工业控制、轨道交通、汽车电子、航空电子、能源电力 |
| **成熟度** | **规模量产** — 在多个国家重点行业长期部署；国内首个在三行业（工业控制/轨道交通/汽车电子）均获TÜV南德功能安全认证的操作系统 |
| **开源/商业** | **专有商业** — 科银京成产品；不开源 |
| **安全认证** | ✅ **ISO 26262 ASIL-D** + **IEC 61508 SIL3** + **EN 50128 SIL4**（TÜV南德）；三行业安全认证全拿下 |
| **RISC-V支持** | ⚠️ **适配中** — 资料有限；商业RTOS对RISC-V的适配进度需向厂商确认 |
| **社区规模** | 封闭商业社区；以厂商直接技术支持为主，无公开GitHub/Gitee数据 |
| **主要用户** | 工业自动化OEM/Tier1、轨道交通信号系统、汽车整车厂（动力域/底盘域）、能源电力SCADA系统 |

**补充说明：**
- 虚拟化技术支持同一SoC内多OS并行（Linux + RTOS混合）
- 定位与SylixOS高度重叠，均面向高可靠关键任务

---

### 2.4 OneOS（中国移动物联网操作系统）

| 维度 | 详情 |
|---|---|
| **OS类型** | 轻量级物联网操作系统（RTOS内核+应用框架） |
| **部署领域** | 物联网、工业IoT边缘、消费电子、智慧城市、能源电力（水电气）、轨道交通、航空航天 |
| **成熟度** | **规模量产** — 已服务360+客户（截至2024）；中国移动背景带来天然规模化推广渠道 |
| **开源/商业** | **开源** — Apache 2.0；由中移物联网有限公司运营；代码托管于Gitee |
| **安全认证** | ✅ **ISO 26262 ASIL-D** + **IEC 61508 SIL3**（行业首家同时获得）；**CCRC EAL4+**信息安全认证（国内首张物联网OS领域IT产品信息安全分级认证）|
| **RISC-V支持** | ✅ **已支持** — 支持ARM Cortex-M/R/A、MIPS、**RISC-V**（含玄铁系列）；兼容POSIX、CMSIS标准接口 |
| **社区规模** | Gitee: cmcc-oneos；开发者社区依托中国移动物联网生态；Gitlink（开源中国）有专门仓库；社区活动以高校公开课、大学生专题赛、联合实验室形式推进 |
| **主要用户** | 中国移动内部物联网项目、360+企业客户（能源/工控/消费类）、轨道交通/航空航天（有功能安全需求的项目）|

**补充说明：**
- 支持JavaScript、MicroPython高级语言开发，降低门槛
- 提供图形化开发工具，定位为快速上手的IoT OS
- 竞争定位：低功耗、低成本、运营商渠道优势

---

### 2.5 OpenHarmony / HarmonyOS（开源鸿蒙）

| 维度 | 详情 |
|---|---|
| **OS类型** | 全场景智能终端OS（微内核+Linux内核双路线）；分布式架构；软总线互联 |
| **部署领域** | 智能手机、平板、PC、智能汽车（座舱+驾驶）、智能家居、可穿戴、IoT设备 |
| **成熟度** | **规模量产** — 2025-06：设备超**11.9亿台**，开发者720万+，上架应用/元服务超2.5万个；2025年5月鸿蒙电脑正式发布 |
| **开源/商业** | **开源（OpenHarmony）** — Apache 2.0/Mulan；由开放原子开源基金会运营；HarmonyOS（华为商用版）基于OpenHarmony商业闭源 |
| **安全认证** | ✅ **ISO 26262 ASIL-D**（华为自动驾驶操作系统微内核，2020年TÜV SÜD认证，业界首个同时获得Safety ASIL-D + Security CC EAL5+双认证的商用OS内核）；HarmonyOS微内核达到车规最高安全等级 |
| **RISC-V支持** | ⚠️ **有限支持** — OpenHarmony主要支持ARM架构；RISC-V支持在路线图中但非重点；阿里平头哥玄铁暂无OpenHarmony官方适配 |
| **社区规模** | Gitee: openharmony（主仓）；2025年9月迁移至GitCode；720万+注册开发者；开放原子基金会背书；是仅次于鸿蒙本身的国内最大OS开源生态 |
| **主要用户** | 华为（手机/PC/车机）、赛力斯问界、奇瑞智界、北汽享界、江淮尊界、上汽尚界（"五界"鸿蒙智行模式）、比亚迪（5G模组合作）、哪吒汽车（智能座舱）、吉利几何系列；通用行业设备厂商 |

**补充说明：**
- "鸿蒙智行"模式：华为深度参与整车定义/设计/渠道，2024年销量44.5万辆，2025年目标百万辆
- HarmonyOS NEXT（鸿蒙5）已突破1000万终端（2025年里程碑）
- OpenHarmony多内核设计：LiteOS内核（轻量）+ Linux内核（丰富生态）

---

### 2.6 NIO SkyOS（蔚来整车全域操作系统）

| 维度 | 详情 |
|---|---|
| **OS类型** | 整车全域操作系统（Full-Vehicle OS）；自研微内核；虚拟化；多域合一（智驾/座舱/车身/底盘/动力） |
| **部署领域** | 蔚来品牌智能电动汽车（NT3平台） |
| **成熟度** | **量产前夕** — 2024年7月27日全量发布；历时4年研发，投入超23000人/月；2025年随NT3平台车型量产 |
| **开源/商业** | **专有自研** — 蔚来完全自主研发；不对外授权；内部使用 |
| **安全认证** | ⚠️ 认证进行中 — 蔚来表示将达ISO 26262 ASIL-D；具体认证状态未公开 |
| **RISC-V支持** | ❌ **不适用** — 自研内核，绑定自研神玑NX9031芯片（Arm架构） |
| **社区规模** | ❌ 无公开社区；蔚来内部研发体系 |
| **主要用户** | 蔚来品牌全系车型（NT3平台：ET9等） |

**补充说明：**
- "1+4+N"架构：1个微内核 + 4个域（L/M/R/C）+ N个应用程序
- SkyOS-H（Hypervisor层）打造多场景虚拟化资源池
- 蔚来数字系统副总裁王启研主导研发

---

### 2.7 OpenEuler Embedded（开放原子基金会边缘OS）

| 维度 | 详情 |
|---|---|
| **OS类型** | 开源嵌入式Linux（服务器/边缘场景）；支持OpenAMP混合部署；容器原生；基于Yocto构建 |
| **部署领域** | 工业边缘服务器、边缘AI、ARM/RISC-V/LoongArch网关、云计算边缘节点、机器人 |
| **成熟度** | **生产验证** — openEuler 22.03 LTS（2022年起）；24.03 LTS（2024）强化RISC-V；工业边缘场景批量部署 |
| **开源/商业** | **开源** — GPL/EPL等（主包）；由开放原子开源基金会运营；可免费商用 |
| **安全认证** | ⚠️ 主OS需客户自行做功能安全认证；OpenEuler主身通过FIPS 140-2等通用安全认证 |
| **RISC-V支持** | ✅ **原生支持（重点）** — openEuler 24.03 LTS主打RISC-V原生支持；中科院软件所发起"RISC-V Kernel Unification Plan"，统一RISC-V OS基线；支持X86/ARM/SW64/**RISC-V**/LoongArch/PowerPC |
| **社区规模** | 开放原子基金会（OpenAtom）；Linux Foundation国际协作；ROS SIG + Embedded SIG活跃；15家社区合作伙伴；RISC-V Kernel Unification Plan推进中 |
| **主要用户** | 工业边缘网关厂商、边缘AI方案商（中科院软件所合作）、ROS机器人开发者、国内服务器/云厂商 |

**补充说明：**
- 混合部署案例：OpenEuler Embedded + Zephyr（实时核）+ OpenAMP，已实现双OS同SoC运行演示
- 嵌入式版本支持裸机管理和容器管理双模式

---

### 2.8 Zephyr RTOS（Linux基金会项目）

| 维度 | 详情 |
|---|---|
| **OS类型** | 开源RTOS（Apache 2.0）；Linux基金会项目；安全连接物联网设备 |
| **部署领域** | 物联网传感器、可穿戴、工业MCU、边缘AI、汽车ADAS、医疗设备 |
| **成熟度** | **规模量产** — 750+支持的开发板（Arm + RISC-V）；2024年发布4.0，2025年3月发布4.1；安全认证工具链接近完成 |
| **开源/商业** | **开源** — Apache 2.0（商业友好）；Linux基金会管理 |
| **安全认证** | 🔄 **认证中** — ISO 26262 ASIL-B认证进行中（Doulos、Honda、IAR等合作推进）；IEC 61508 SIL3预认证；PikeOS联合方案已可提供认证包 |
| **RISC-V支持** | ✅ **主线支持（重点）** — RISC-V是第一类架构支持；所有RISC-V主线内核上游推进；与openEuler合作混合部署 |
| **社区规模** | Linux基金会；2025年新加入Honda（汽车）、IAR（工具链）、Microchip、Renesas（铂金会员）等；2025年1月Doulos加入推进功能安全；全球性社区 |
| **主要用户** | Renesas、NXP、Microchip、ST；Honda（汽车OEM）；工业IoT方案商；Edge AI创业公司；HBMicro（国内HPMicro官方Zephyr支持）|

**补充说明：**
- 2024年社区回顾：750+开发板、4.0/4.1版本发布、AI/ML集成
- 国内：与OpenEuler Embedded合作实现混合部署（RISC-V + Zephyr + Linux）
- RISC-V vectors + AI方向：适合边缘AI推理场景

---

### 2.9 PikeOS（SYSGO商业Hypervisor/RTOS）

| 维度 | 详情 |
|---|---|
| **OS类型** | 微内核Hypervisor + RTOS混合；多OS同平台并行（Android/Linux/FreeRTOS/RTOS等）；分区隔离 |
| **部署领域** | 汽车ADAS/IVI/网关、航空电子、轨道交通、工业自动化、医疗设备、军事 |
| **成熟度** | **规模量产** — 20年+历史；欧洲航空航天/汽车行业主要认证OS；国内通过代理商引入 |
| **开源/商业** | **专有商业** — SYSGO（德国）产品；不开源 |
| **安全认证** | ✅ **最全面认证组合**：ISO 26262 **ASIL D**（汽车）、DO-178C DAL A（航空）、ECSS Cat. A、EN 50716 **SIL4**（铁路）、ISO 26262-6 ASIL D、IEC 61508 **SIL3**；提供认证工具包（Certification Kits）|
| **RISC-V支持** | ✅ **已支持** — x86、ARM v7/v8、SPARC/LEON v8、PowerPC、**RISC-V** |
| **社区规模** | 无公开社区；SYSGO提供商业支持；合作伙伴包括ST、Infineon等国际Tier1 |
| **主要用户** | 国际Tier1（大陆、采埃孚等）、欧洲汽车OEM；国内通过代理引入；航空航天（LEON处理器）|

**补充说明：**
- 认证包（Certification Kits）是核心差异化：帮助客户快速通过DO-178C/ISO 26262认证
- 适合需要Hypervisor隔离 + 多OS并存 + 功能安全认证的项目
- 在欧洲市场与QNX同为高安全认证OS选择

---

### 2.10 FreeRTOS（亚马逊云RTOS）

| 维度 | 详情 |
|---|---|
| **OS类型** | 开源RTOS（MIT许可证）；AWS IoT Edge主力OS；云原生集成 |
| **部署领域** | IoT边缘设备、微控制器、低功耗传感、工业边缘、汽车配件 |
| **成熟度** | **规模量产** — 全球最大装机量开源RTOS之一（数10亿级）；AWS默认IoT OS |
| **开源/商业** | **开源** — MIT许可证（极度商业友好）；亚马逊主导开发维护 |
| **安全认证** | ⚠️ 主版本无功能安全认证；AWS提供Commercial LTS版本含安全补丁；ISO 26262认证需使用第三方商业版本（Weston Embedded等） |
| **RISC-V支持** | ✅ **主线支持** — FreeRTOS官方支持RISC-V（VisionFive/玄铁等）；AWS IoT ExpressLink模块已用RISC-V；平头哥玄铁RTOS SDK已包含FreeRTOS适配 |
| **社区规模** | MIT许可；GitHub主仓；AWS统一维护；全球最大开源RTOS社区之一；国际生态成熟 |
| **主要用户** | AWS IoT生态设备商、博世、英飞凌、NXP；消费电子/工业IoT品牌；国内嵌入式厂商（作为备选RTOS）|

**补充说明：**
- MIT许可，无任何商业限制
- 国内玄铁适配：平头哥RTOS SDK提供FreeRTOS参考
- 适合对成本极度敏感、无功能安全要求的IoT项目

---

## 三、国产/国内背景OS总览对比

| OS | 类型 | 主要领域 | 成熟度 | 开源 | 安全认证 | RISC-V | 社区规模 | 代表用户 |
|---|---|---|---|---|---|---|---|---|
| **RT-Thread** | 开源RTOS | 消费/工业/车载 | 规模量产20亿+ | Apache 2.0 | ASIL-D | ✅深度适配 | 最大(10K+GitHub stars) | 先楫, ST, 车载Tier1 |
| **SylixOS** | 商业RTOS | 航天/航空/汽车/电网 | 规模量产 | 专有 | ASIL-D/SIL3/SIL4 | ✅已适配 | 企业级 | 航天院所, 轨道交通, 电网, 汽车 |
| **Intewell** | 商业RTOS+虚拟化 | 工业/轨交/汽车 | 规模量产 | 专有 | ASIL-D/SIL3/SIL4 | ⚠️待确认 | 企业级 | 工业OEM, 轨交, 汽车 |
| **OneOS** | 开源IoT OS | IoT/工业/能源 | 规模量产 | Apache 2.0 | ASIL-D/SIL3 + EAL4+ | ✅已支持 | 中等(移动生态) | 中国移动客户群 |
| **OpenHarmony** | 全场景OS | 手机/车机/PC/IoT | 规模量产11.9亿 | Apache 2.0/Mulan | ASIL-D (Huawei) | ⚠️有限 | 最大开源(720万开发者) | 华为, 赛力斯, 奇瑞, 北汽, 上汽 |
| **NIO SkyOS** | 整车OS | 蔚来汽车 | 量产前夕 | 专有自研 | 🔄认证中 | ❌ | 无 | 蔚来 |
| **OpenEuler Emb.** | 开源Linux | 边缘/工业 | 生产验证 | GPL | 客户自认证 | ✅原生重点 | 活跃(Linux Foundation) | 边缘AI厂商, 中科院 |
| **Zephyr** | 开源RTOS | IoT/汽车/工业 | 规模量产750+板 | Apache 2.0 | 🔄认证中(ASIL-B) | ✅主线支持 | 活跃(Linux Foundation) | Renesas, NXP, Honda |
| **PikeOS** | 商业Hypervisor | 汽车/航空/轨交 | 规模量产 | 专有 | ASIL-D/SIL3/SIL4/DO-178C | ✅已支持 | 封闭商业 | 大陆, 采埃孚, 欧洲航空 |
| **FreeRTOS** | 开源RTOS | IoT/边缘 | 规模量产 | MIT | ⚠️需商业版 | ✅主线支持 | 全球最大之一 | AWS生态, 博世, NXP |

---

## 四、SIL认证选型参考

### 认证组合最全的OS（可直接用于SIL目标项目）

| 优先级 | OS | 认证覆盖 | 适合目标 |
|---|---|---|---|
| ⭐⭐⭐ | **SylixOS** | ISO 26262 ASIL-D + IEC 61508 SIL3 + EN 50128 SIL4 + DO-178C | 工业功能安全 + 汽车ASIL-D + 轨道交通SIL4 |
| ⭐⭐⭐ | **Intewell** | ISO 26262 ASIL-D + IEC 61508 SIL3 + EN 50128 SIL4 | 同上，三行业全认证国内唯一 |
| ⭐⭐⭐ | **PikeOS** | ISO 26262 ASIL-D + IEC 61508 SIL3 + EN 50716 SIL4 + DO-178C DAL A | 最高安全等级（航空+汽车+轨交），有认证包 |
| ⭐⭐ | **RT-Thread** | ISO 26262 ASIL-D（TÜV SÜD） | 汽车ASIL-D目标，需车规级RTOS |
| ⭐⭐ | **OneOS** | ISO 26262 ASIL-D + IEC 61508 SIL3 + CCRC EAL4+ | 工业+汽车双认证 + 信息安全 |

### RISC-V平台选型建议

| 场景 | 推荐OS | 理由 |
|---|---|---|
| **汽车ASIL-D + RISC-V** | **SylixOS**（商业）/ **RT-Thread**（开源+认证） | 国内唯二有ASIL-D认证的RISC-V可用RTOS |
| **工业功能安全 + RISC-V** | **SylixOS** / **OpenEuler+Zephyr混合** | SIL3认证；Zephyr认证中可作为备选 |
| **车载IVI/网关 + RISC-V** | **Linux** (Debian/OpenEuler) + **Android** | 玄铁C908/R908A已有完整Linux/Android SDK |
| **边缘AI推理 + RISC-V** | **OpenEuler Embedded** + **Zephyr混合** | 奕斯伟EIC7702X等AI SoC适配 |
| **低成本IoT + RISC-V** | **FreeRTOS** / **RT-Thread** | MIT/Apache 2.0，无授权费 |
| **Hypervisor多OS + RISC-V** | **PikeOS** / **RTS Hypervisor** | 商业认证最全，支持RISC-V |

---

## 五、参考来源

### OS平台官方/权威来源

1. RT-Thread官网 - 关于页面（装机量/生态数据）
   https://www.rt-thread.org/about.html

2. RT-Thread GitHub主仓（stars/仓库数据）
   https://github.com/RT-Thread/rt-thread

3. RT-Thread ISO 26262 ASIL-D认证页面
   https://www.rt-thread.com/products/Certification-37.html

4. RT-Thread Medium - 10K Stars里程碑
   https://rt-thread.medium.com/rt-thread-on-github-surpasses-10-000-stars-a-new-milestone-achieved-4e1053a5449d

5. 翼辉SylixOS产品页（安全认证）
   https://www.acoinfo.com/product/system/sylixos-safe

6. 翼辉SylixOS介绍页（行业应用/自主化率）
   https://www.acoinfo.com/product/system/sylixos

7. 翼辉航空电子页（DO-178C/ARINC 653认证）
   https://www.acoinfo.com/industry-center/aviation

8. 翼辉智能汽车页（ASIL-D / VSOA）
   https://www.acoinfo.com/industry-center/automobile/

9. 翼辉关于页（融资/公司背景）
   https://www.acoinfo.com/about

10. 科银京成Intewell - CSDN介绍（安全认证详情）
    https://blog.csdn.net/Kyland2020/article/details/127844211

11. OneOS官网 - 功能安全认证
    https://os.iot.10086.cn/

12. OneOS ISO 26262 ASIL-D + IEC 61508 SIL3认证新闻
    https://www.oschina.net/news/178992

13. OneOS Gitee仓库
    https://gitee.com/cmcc-oneos

14. OpenHarmony Gitee主页
    https://gitee.com/openharmony

15. 华为鸿蒙ISO 26262 ASIL-D + CC EAL5+认证（SGS/IT之家）
    https://www.51fusa.com/client/knowledge/knowledgedetail/id/1543.html

16. 主机厂自研智能车载操作系统技术分析（知乎 - SkyOS详情）
    https://zhuanlan.zhihu.com/p/1888703877420916807

17. NIO官网 - SkyOS介绍
    https://www.nio.cn/innovation

18. 蔚来SkyOS全量发布新闻（凤凰汽车）
    https://auto.ifeng.com/c/8bZNbNeo66g

19. OpenEuler Wikipedia（支持的架构列表）
    https://en.wikipedia.org/wiki/EulerOS

20. OpenEuler 24.03 LTS - RISC-V原生支持公告
    https://www.openeuler.org/en/news/20240628-openEuler%2024.03%20LTS-%20Pioneering%20Native%20RISC-V%20Support/

21. Zephyr Project 2024年度回顾
    https://zephyrproject.org/zephyr-rtos-2024-wrap-up-a-year-of-growth-innovation-and-community-impact/

22. Zephyr 750+开发板支持新闻（Linux Foundation）
    https://www.linuxfoundation.org/press/zephyr-rtos-expands-ecosystem-with-renesas-and-wind-river-upgrading-to-platinum

23. Zephyr安全认证推进新闻（Doulos/Honda/IAR加入）
    https://www.linuxfoundation.org/press/doulos-honda-hubble-network-iar-inovex-and-microchip-technology-join-the-zephyr-project-as-it-gets-closer-to-safety-certification

24. SYSGO PikeOS产品页（架构/认证）
    https://www.sysgo.com/pikeos

25. PikeOS ISO 26262 ASIL-D认证公告
    https://www.sysgo.com/press-releases/pikeos-with-certification-kits-for-the-highest-safety-levels

26. FreeRTOS官方页面
    https://github.com/FreeRTOS

### 行业分析来源

27. 何小庆《嵌入式产业三大趋势》(embedded world China)
    https://embedded-world.com.cn/home/press_industry/id/5984

28. 何小庆《2023 to 2024: Outlook of China's Embedded System Industry》(EN)
    https://embedded-world.com.cn/en/press_industry/id/5957

29. 2025中国RISC-V生态大会（ScenSmart）
    https://www.scensmart.com/news/risc-v-2025/

30. 阿里平头哥玄铁C908发布（IT之家 - OS适配）
    https://www.ithome.com/0/650/986.htm

31. 玄铁RISC-V创新架构助推算力产业（电子工程专辑）
    https://www.eet-china.com/news/202504172866.html

32. RISC-V Exceeding Expectations in AI, China Deployment（EE Times）
    https://www.eetimes.com/risc-v-exceeding-expectations-in-ai-china-deployment/

33. RISC-V And Its Modularity（Embedded）
    https://www.embedded.com/risc-v-and-its-modularity-shine-across-applications/

34. Can RISC-V Cross the Threshold in 2025（Embedded）
    https://www.embedded.com/can-risc-v-cross-the-threshold-in-2025

35. 2024中国开源年度报告（开源社 - OpenHarmony社区数据）
    https://kaiyuanshe.atomgit.net/2024-China-Open-Source-Report/data.html

36. 2025中国RISC-V生态大会 - 玄铁R908A/车规（钛媒体）
    https://www.tmtpost.com/7476822.html

37. NIO SkyOS发布详情（汽车之家）
    https://chejiahao.autohome.com.cn/info/16081418

38. 上汽集团与华为合作（21经济网 - 鸿蒙智行五界）
    https://www.21jingji.com/article/20251013/herald/1dd634df1dc395f5d446acfcc2116533.html

39. 华为车BU - 2025鸿蒙智行百万辆目标（_OFweek新能源汽车）
    https://nev.ofweek.com/2025-04/ART-71000-8110-30661920.html

40. 翼辉SylixOS崛起之路（CSDN - DO-178C/适航详情）
    https://blog.csdn.net/hujunming/article/details/148370364

41. 陆石投资翼辉信息（客户/行业覆盖）
    http://www.landstone.cn/cases.html

42. 汽车RISC-V芯片行业报告ResearchInChina（AUTOSAR/Andes）
    http://www.researchinchina.com/Htmls/Report/2024/73952.html

43. 佐思汽研2025汽车操作系统与AIOS融合研究报告
    https://db.shujubang.com/home/login/index/gid/21121

44. 21IC - 主流嵌入式操作系统（RTOS）介绍
    https://www.21ic.com/tougao/article/27828.html

45. 知乎 - 国内主流嵌入式RTOS汇总
    https://zhuanlan.zhihu.com/p/616380492

46. 知乎 - 五大国产实时操作系统RTOS
    https://blog.csdn.net/Kyland2020/article/details/130243225

47. 知乎 - 2025年强烈推荐学习的20个RTOS
    https://zhuanlan.zhihu.com/p/16551672439

48. 格隆汇 - 2025中国物联网边缘网关行业市场规模
    https://m.gelonghui.com/p/1790680

49. Linux in Automotive Systems Market Share（Command Linux）
    https://commandlinux.com/statistics/linux-in-automotive-systems-market-share/

50. Automotive Operating Systems Market（Mordor Intelligence）
    https://www.mordorintelligence.com/industry-reports/automotive-operating-systems-market

---

*本报告基于2026-05-20网络调研整理，数据截至2025年底，仅供内部参考。部分商业OS的RISC-V适配状态建议直接向厂商确认。*
