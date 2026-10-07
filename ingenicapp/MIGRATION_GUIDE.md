# socapp 到 HAL 迁移指南

## 概述

本文档描述如何将现有的 `socapp`（基于 CVITEK API）迁移到硬件抽象层（HAL）架构，使其能够在多平台上运行，包括：
1. **CVITEK 平台**（现有平台）
2. **Ingenic T31 平台**（目标平台）
3. **未来平台**（如 Rockchip、海思等）

## 迁移策略

### 1. 保持现有代码结构

现有的 `socapp` 模块结构保持不变，只替换平台相关的 API 调用：

```
socapp/modules/
├── video/                    # 保持现有文件结构
│   ├── include/
│   │   ├── app_ipcam_vi.h    # 改为调用 HAL API
│   │   ├── app_ipcam_venc.h
│   │   └── ...
│   └── src/
│       ├── app_ipcam_vi.c    # 替换 CVI_* 调用为 hal_* 调用
│       └── ...
└── ...
```

### 2. 创建平台适配层

为每个平台创建适配器：

```
ingenicapp/
├── hal/
│   ├── include/
│   │   └── app_hal.h         # 统一的 HAL 接口
│   └── src/
│       ├── app_hal_common.c  # 通用实现
│       ├── platform_cvitek.c # CVITEK 平台实现
│       ├── platform_ingenic.c # Ingenic 平台实现
│       └── platform_rk.c     # Rockchip 平台实现
└── modules/                  # 从 socapp 复制的模块
```

### 3. 逐步迁移过程

**阶段 1：分析现有 API 使用**
- 识别所有 `CVI_` 开头的函数调用
- 记录每个模块的平台依赖程度
- 确定迁移顺序

**阶段 2：实现 HAL 接口**
- 设计统一的 HAL 接口
- 创建 CVITEK 适配器（保持现有功能）
- 验证 HAL 接口的完整性

**阶段 3：替换 API 调用**
- 逐个模块替换 `CVI_` 调用为 `hal_` 调用
- 保持现有模块间的接口不变
- 确保功能一致性

**阶段 4：平台适配器开发**
- 实现 Ingenic T31 适配器
- 测试基本功能（视频采集、编码）
- 逐步添加高级功能（AI、音频等）

**阶段 5：优化和测试**
- 性能优化
- 稳定性测试
- 多平台验证

## API 映射表

### 系统初始化

| CVITEK API | HAL API | Ingenic API |
|------------|---------|-------------|
| `CVI_SYS_Init()` | `hal_sys_init()` | `IMP_System_Init()` |
| `CVI_VB_Init()` | 在 `hal_sys_init()` 中处理 | `IMP_System_Bind()` |
| `CVI_SYS_Exit()` | `hal_sys_deinit()` | `IMP_System_Exit()` |

### 视频输入 (VI)

| CVITEK API | HAL API | Ingenic API |
|------------|---------|-------------|
| `CVI_VI_SetDevAttr()` | `hal_vi_init()` | `IMP_FrameSource_SetFrameDepth()` |
| `CVI_VI_SetChnAttr()` | 在 `hal_vi_init()` 中配置 | `IMP_FrameSource_SetChnAttr()` |
| `CVI_VI_EnableChn()` | `hal_vi_start()` | `IMP_FrameSource_EnableChn()` |
| `CVI_VI_DisableChn()` | `hal_vi_stop()` | `IMP_FrameSource_DisableChn()` |
| `CVI_VI_GetChnFrame()` | `hal_vi_get_frame()` | `IMP_FrameSource_GetFrame()` |
| `CVI_VI_ReleaseChnFrame()` | `hal_vi_release_frame()` | `IMP_FrameSource_ReleaseFrame()` |

### 视频编码 (VENC)

| CVITEK API | HAL API | Ingenic API |
|------------|---------|-------------|
| `CVI_VENC_CreateChn()` | `hal_venc_init()` | `IMP_Encoder_CreateChn()` |
| `CVI_VENC_StartRecvFrame()` | 在 `hal_venc_init()` 中处理 | `IMP_Encoder_StartRecvPic()` |
| `CVI_VENC_GetStream()` | 由 HAL 内部处理 | `IMP_Encoder_GetStream()` |
| `CVI_VENC_ReleaseStream()` | 由 HAL 内部处理 | `IMP_Encoder_ReleaseStream()` |
| `CVI_VENC_RequestIDR()` | `hal_venc_request_idr()` | `IMP_Encoder_RequestIdr()` |

### 音频处理

| CVITEK API | HAL API | Ingenic API |
|------------|---------|-------------|
| `CVI_AUDIO_Init()` | `hal_audio_init()` | `IMP_Audio_SetPubAttr()` |
| `CVI_AUDIO_Start()` | `hal_audio_start()` | `IMP_Audio_Enable()` |
| `CVI_AUDIO_Stop()` | `hal_audio_stop()` | `IMP_Audio_Disable()` |
| `CVI_AUDIO_ReadFrame()` | `hal_audio_get_frame()` | `IMP_Audio_Record()` |

### AI 推理

| CVITEK API | HAL API | Ingenic API |
|------------|---------|-------------|
| `CVI_AI_CreateHandle()` | `hal_ai_init()` | 无直接对应，可能需要软件实现 |
| `CVI_AI_Forward()` | `hal_ai_process()` | 使用 TensorFlow Lite 或 NCNN |
| `CVI_AI_DestroyHandle()` | 在 `hal_ai_deinit()` 中处理 | - |

## 迁移示例

### 原始 CVITEK 代码 (`app_ipcam_vi.c`)

```c
// 系统初始化
CVI_S32 ret = CVI_SYS_Init();
if (ret != CVI_SUCCESS) {
    printf("CVI_SYS_Init failed: %d\n", ret);
    return -1;
}

// VI 初始化
VI_DEV_ATTR_S dev_attr = {0};
// ... 填充 dev_attr
ret = CVI_VI_SetDevAttr(VI_DEV0, &dev_attr);
ret |= CVI_VI_EnableDev(VI_DEV0);

// 获取视频帧
VIDEO_FRAME_INFO_S stFrame;
ret = CVI_VI_GetChnFrame(VI_PIPE0, VI_CHN0, &stFrame, 1000);
if (ret == CVI_SUCCESS) {
    // 处理帧
    CVI_VI_ReleaseChnFrame(VI_PIPE0, VI_CHN0, &stFrame);
}
```

### 迁移后的 HAL 代码

```c
// 系统初始化
hal_sys_config_t sys_config = {
    .video_mem_size = 128,
    .audio_mem_size = 16,
    .ai_mem_size = 64,
    .enable_hardware_codec = true,
    .enable_ai_accel = false,
    .sensor_model = "GC1084",
};

hal_err_t err = hal_sys_init(&sys_config);
if (err != HAL_OK) {
    printf("hal_sys_init failed: %s\n", hal_err_to_string(err));
    return -1;
}

// VI 初始化
hal_vi_config_t vi_config = {
    .width = 1920,
    .height = 1080,
    .format = HAL_FMT_YUV420SP,
    .fps = 30,
    .channel_id = 0,
    .sensor_id = 0,
    .sensor_name = "gc1084",
};

err = hal_vi_init(&vi_config);
if (err != HAL_OK) {
    printf("hal_vi_init failed: %s\n", hal_err_to_string(err));
    return -1;
}

// 获取视频帧
hal_video_frame_t frame;
err = hal_vi_get_frame(0, &frame, 1000);
if (err == HAL_OK) {
    // 处理帧
    hal_vi_release_frame(0, &frame);
}
```

## 构建系统修改

### 原始 `build/config.mk`

```makefile
# CVITEK 特定配置
CFLAGS += -DARCH_CV181X -D__CV181X__
LDFLAGS += -lcvi_vi -lcvi_venc -lcvi_audio -lcviai
```

### 迁移后的构建配置

```makefile
# 平台选择
PLATFORM ?= cvitek
# PLATFORM = ingenic
# PLATFORM = rk

ifeq ($(PLATFORM),cvitek)
CFLAGS += -DPLATFORM_CVITEK
LDFLAGS += -lcvi_vi -lcvi_venc -lcvi_audio -lcviai
else ifeq ($(PLATFORM),ingenic)
CFLAGS += -DPLATFORM_INGENIC
LDFLAGS += -limp_framesource -limp_encoder -limp_audio
else ifeq ($(PLATFORM),rk)
CFLAGS += -DPLATFORM_RK
LDFLAGS += -lrk_vi -lrk_venc -lrk_audio
endif

# HAL 通用配置
CFLAGS += -I$(HAL_DIR)/include
SRCS += $(HAL_DIR)/src/app_hal_common.c
SRCS += $(HAL_DIR)/src/platform_$(PLATFORM).c
```

## 测试策略

### 1. 单元测试
- 为每个 HAL 接口创建测试用例
- 模拟不同平台的行为
- 验证错误处理

### 2. 集成测试
- 测试模块间的交互
- 验证数据流完整性
- 性能基准测试

### 3. 平台测试
- **CVITEK 平台**: 确保现有功能不变
- **Ingenic 平台**: 验证基本功能工作
- **跨平台一致性**: 确保相同输入产生相同输出

### 4. 回归测试
- 保持向后兼容性
- 确保新改动不破坏现有功能
- 自动化测试套件

## 常见问题处理

### Q1: 如何处理平台特有的功能？
**A**: 通过 `hal_sys_get_platform()` 检测平台，使用条件编译或运行时判断：

```c
char platform[32];
hal_sys_get_platform(platform, sizeof(platform));

if (strcmp(platform, "CVITEK") == 0) {
    // CVITEK 特有功能
} else if (strcmp(platform, "Ingenic") == 0) {
    // Ingenic 特有功能
} else {
    // 通用实现或返回不支持错误
    return HAL_ERR_UNSUPPORTED;
}
```

### Q2: AI 模块在 Ingenic 上无硬件加速怎么办？
**A**: 提供软件备选方案：
1. 使用 TensorFlow Lite 进行 CPU 推理
2. 降低 AI 模型复杂度
3. 提供配置选项关闭 AI 功能

### Q3: 如何保证实时性能？
**A**: 
1. 尽量减少 HAL 层的开销
2. 使用零拷贝机制传递数据
3. 平台适配器直接调用硬件 API
4. 性能分析和优化

### Q4: 第三方库依赖如何处理？
**A**: 分层处理：
1. **标准库** (libc, pthread): 所有平台支持
2. **开源库** (FFmpeg, OpenSSL): 跨平台编译
3. **专有库** (CVI_*, IMP_*): 通过 HAL 抽象

## 下一步计划

1. **完成 HAL 接口设计**（当前阶段）
2. **创建 CVITEK 适配器**（验证设计）
3. **分析 Ingenic SDK 的 IMP API**（设计映射）
4. **移植 video 模块**（核心功能）
5. **逐步移植其他模块**
6. **创建构建系统和测试框架**

## 结论

通过 HAL 架构，我们可以：
- ✅ 保持现有 `socapp` 代码结构
- ✅ 实现多平台支持
- ✅ 降低未来移植成本
- ✅ 提高代码可维护性
- ✅ 支持平台特有优化

迁移过程是渐进式的，可以在保持现有功能的同时，逐步添加对新平台的支持。

---

**文档版本**: 1.0  
**最后更新**: 2026-03-20  
**作者**: kid (🦊)