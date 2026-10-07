# 视频编码器(VENC)功能封存文档

**封存日期**: 2026-03-23  
**封存原因**: 用户要求暂停编码功能开发，启动其他功能开发  
**相关文件**: 
- `/home/rainti/.openclaw/workspace/ingenicapp/hal/include/hal_video.h` - 完整HAL视频接口
- `/data/Ingenic-SDK-T31-1.1.6-20221013/sdk/5.4.0/samples/t31app/hal/src/platform_ingenic.c` - Ingenic平台实现
- `/home/rainti/.openclaw/workspace/ingenicapp/hal/include/app_hal.h` - 简化版HAL接口

## 📊 实现状态总览

### HAL视频编码器函数 (共14个)

| 状态 | 数量 | 百分比 |
|------|------|--------|
| ✅ 已实现 | 3 | 21% |
| ❌ 未实现 | 11 | 79% |
| 🔄 部分实现 | 0 | 0% |

## ✅ 已实现函数 (3个)

### 1. `hal_venc_init(const hal_venc_config_t *config)`
**实现状态**: ✅ 完整实现（含存根）  
**IMP API映射**:
- `IMP_Encoder_CreateGroup(channel_id)`
- `IMP_Encoder_SetDefaultParam(&channel_attr, ...)`
- `IMP_Encoder_CreateChn(channel_id, &channel_attr)`
- `IMP_Encoder_RegisterChn(channel_id, channel_id)`
- `IMP_Encoder_StartRecvPic(channel_id)`

**位置**: `platform_ingenic.c:471-609`  
**限制**: 仅支持通道0，多通道需扩展

### 2. `hal_venc_encode(const hal_video_frame_t *in_frame, hal_video_frame_t *out_packet)`
**实现状态**: ✅ 完整实现（含存根）  
**IMP API映射**:
- `IMP_Encoder_PollingStream(channel_id, 1000)`
- `IMP_Encoder_GetStream(channel_id, &stream, 1)`
- `IMP_Encoder_ReleaseStream(channel_id, &stream)`

**位置**: `platform_ingenic.c:610-723`  
**说明**: 合并了获取码流功能，与完整HAL设计中的`encode_frame()`和`get_stream()`分离设计不同

### 3. `hal_venc_request_idr(uint32_t channel_id)`
**实现状态**: ✅ 完整实现（含存根）  
**IMP API映射**: `IMP_Encoder_RequestIdr(channel_id, 1)`  
**位置**: `platform_ingenic.c:724-757`

## ❌ 未实现函数 (11个)

### 表1：通道生命周期管理 (4个)

| HAL函数 | IMP API映射 | 优先级 | 实现复杂度 | 关键挑战 |
|---------|-------------|--------|------------|----------|
| `hal_venc_deinit(void)` | 1. `IMP_Encoder_StopRecvPic(channel_id)`<br>2. `IMP_Encoder_UnRegisterChn(channel_id, channel_id)`<br>3. `IMP_Encoder_DestroyChn(channel_id)`<br>4. `IMP_Encoder_DestroyGroup(channel_id)` | 高 | 低 | 需要遍历所有已创建通道 |
| `hal_venc_create_channel(const hal_venc_config_t *config, uint32_t *chn_id)` | 1. `IMP_Encoder_CreateGroup(*chn_id)`<br>2. `IMP_Encoder_SetDefaultParam()`<br>3. `IMP_Encoder_CreateChn(*chn_id, &attr)`<br>4. `IMP_Encoder_RegisterChn(*chn_id, *chn_id)` | 高 | 中 | 通道ID分配和管理 |
| `hal_venc_destroy_channel(uint32_t chn_id)` | 1. `IMP_Encoder_UnRegisterChn(chn_id, chn_id)`<br>2. `IMP_Encoder_DestroyChn(chn_id)`<br>3. `IMP_Encoder_DestroyGroup(chn_id)` | 高 | 中 | 资源清理顺序 |
| `hal_venc_start(uint32_t chn_id)` | `IMP_Encoder_StartRecvPic(chn_id)` | 中 | 低 | 简单映射 |

### 表2：编码控制与状态管理 (3个)

| HAL函数 | IMP API映射 | 优先级 | 实现复杂度 | 关键挑战 |
|---------|-------------|--------|------------|----------|
| `hal_venc_stop(uint32_t chn_id)` | `IMP_Encoder_StopRecvPic(chn_id)` | 中 | 低 | 简单映射 |
| `hal_venc_encode_frame(uint32_t chn_id, const hal_frame_t *frame, hal_frame_t *encoded_frame)` | **IMP推送模型不适配**<br>需使用`IMP_Encoder_GetStream()`异步获取 | 高 | 高 | HAL拉取模型 vs IMP推送模型 |
| `hal_venc_get_stream(uint32_t chn_id, hal_frame_t *stream_frame, int timeout_ms)` | 1. `IMP_Encoder_PollingStream(chn_id, timeout_ms)`<br>2. `IMP_Encoder_GetStream(chn_id, &stream, block)` | 高 | 中 | 需要与当前`hal_venc_encode()`函数分离 |

### 表3：动态参数调整 (2个)

| HAL函数 | IMP API映射 | 优先级 | 实现复杂度 | 关键挑战 |
|---------|-------------|--------|------------|----------|
| `hal_venc_set_bitrate(uint32_t chn_id, uint32_t bitrate)` | `IMP_Encoder_SetDefaultParam()`重新配置 | 低 | 高 | IMP可能不支持动态调整 |
| `hal_venc_set_framerate(uint32_t chn_id, uint32_t fps)` | `IMP_Encoder_SetDefaultParam()`重新配置 | 低 | 高 | IMP可能不支持动态调整 |

### 表4：数据流绑定 (2个)

| HAL函数 | IMP API映射 | 优先级 | 实现复杂度 | 关键挑战 |
|---------|-------------|--------|------------|----------|
| `hal_venc_bind_vpss(uint32_t venc_chn_id, uint32_t vpss_chn_id)` | `IMP_System_Bind(&vpss_cell, &venc_cell)` | 中 | 中 | 需要理解IMP Cell模型 |
| `hal_venc_unbind_vpss(uint32_t venc_chn_id)` | `IMP_System_UnBind(&vpss_cell, &venc_cell)` | 中 | 低 | 简单映射 |

## 🔍 技术架构差异分析

### 1. 编码模型差异
- **HAL设计（拉取模型）**: `encode_frame()`同步编码输入帧
- **IMP实现（推送模型）**: 自动编码绑定的数据流，`get_stream()`异步获取结果

### 2. 通道管理差异
- **HAL设计**: 单函数创建通道，应用指定或获取通道ID
- **IMP实现**: 多步骤：创建组→设置参数→创建通道→注册通道

### 3. 参数调整能力
- **HAL期望**: 支持运行时动态调整码率、帧率
- **IMP限制**: 编码参数可能需要在创建时设置，运行时调整受限

### 4. 数据流绑定
- **HAL设计**: 显式绑定VPSS到编码器
- **IMP核心**: 通过`IMP_System_Bind()`绑定Cell三元组

## 📝 关键设计决策点

### 决策1：编码模型适配
**选项A**: 保持当前合并设计（`hal_venc_encode()` = 获取码流）
**选项B**: 分离为`encode_frame()` + `get_stream()`，`encode_frame()`为存根
**建议**: 选项B，更符合HAL设计，但需文档说明IMP限制

### 决策2：动态参数调整
**选项A**: 实现为"尽力而为"，失败时返回`HAL_ERR_UNSUPPORTED`
**选项B**: 实现重新初始化通道的方案
**建议**: 选项A + 选项B组合，提供降级方案

### 决策3：多通道支持
**选项A**: 扩展当前硬编码通道0为通道数组
**选项B**: 重构为通道管理器模式
**建议**: 选项A起步，逐步演进到选项B

## 🗂️ 相关文件与位置

### 设计文档
1. **完整HAL设计**: `/home/rainti/.openclaw/workspace/ingenicapp/hal/include/hal_video.h`
2. **简化版接口**: `/home/rainti/.openclaw/workspace/ingenicapp/hal/include/app_hal.h`
3. **IMP API分析**: `/home/rainti/.openclaw/workspace/ingenicapp/IMP_API_ANALYSIS.md`

### 实现代码
1. **Ingenic平台实现**: `/data/Ingenic-SDK-T31-1.1.6-20221013/sdk/5.4.0/samples/t31app/hal/src/platform_ingenic.c`
   - 已实现: 第471-757行（`hal_venc_init`, `hal_venc_encode`, `hal_venc_request_idr`）
2. **通用实现**: `/data/Ingenic-SDK-T31-1.1.6-20221013/sdk/5.4.0/samples/t31app/hal/src/app_hal_common.c`

### 测试代码
1. **HAL测试**: `/data/Ingenic-SDK-T31-1.1.6-20221013/sdk/5.4.0/samples/t31app/test/test_hal.c`
2. **简单测试**: `/data/Ingenic-SDK-T31-1.1.6-20221013/sdk/5.4.0/samples/t31app/test/test_simple.c`

## 🎯 重启开发建议

### 第一阶段：核心功能完善 (预计3-5天)
1. **通道管理**: `create_channel()`, `destroy_channel()`, `start()`, `stop()`, `deinit()`
2. **码流管理**: `get_stream()`, `release_stream()` (分离当前`encode()`)
3. **数据流绑定**: `bind_vpss()`, `unbind_vpss()`

### 第二阶段：高级功能 (预计2-3天)
4. **动态参数**: `set_bitrate()`, `set_framerate()` (含降级方案)
5. **性能优化**: 多通道性能测试与优化

### 第三阶段：集成测试 (预计1-2天)
6. **完整测试**: 多通道、动态调整、绑定流程测试
7. **文档更新**: 使用指南和限制说明

## 📅 封存信息

- **封存时间**: 2026-03-23 10:45 GMT+8
- **封存人**: kid (🦊)
- **封存版本**: HAL视频编码器 v0.3 (3/14实现)
- **预计重启工作量**: 6-10人日
- **技术债务**: 模型差异适配、动态参数限制、多通道管理

---
**备注**: 本文件为视频编码器功能开发暂停时的状态封存。重启开发时请参考本文件中的未实现功能列表、技术挑战和实现建议。