# Ingenic IMP API 分析报告

**分析日期**: 2026-03-20  
**SDK版本**: Ingenic-SDK-T31-1.1.6-20221013  
**目标**: 理解IMP API架构，设计CVITEK到Ingenic的API映射

## 1. IMP 系统架构概览

Ingenic Media Platform (IMP) 采用模块化设计，主要模块包括：

### 1.1 核心模块
- **IMP_System**: 系统初始化和模块绑定
- **IMP_FrameSource**: 视频输入（类似CVITEK的VI）
- **IMP_Encoder**: 视频编码（H.264/H.265/JPEG）
- **IMP_ISP**: 图像信号处理
- **IMP_Audio**: 音频输入/输出
- **IMP_OSD**: 屏幕叠加显示
- **IMP_IVS**: 智能视频分析（移动侦测等）

### 1.2 数据流概念
```
FrameSource (视频源)
    ├── Channel 0 → 绑定到 → OSD Group 0 → 绑定到 → Encoder Group 0
    └── Channel 1 → 绑定到 → IVS Group 0 → 绑定到 → OSD Group 1 → 绑定到 → Encoder Group 1
```

### 1.3 关键数据结构
- **IMPCell**: 标识设备、组、输出的三元组 `{device_id, group_id, output_id}`
- **IMPFSChnAttr**: FrameSource通道属性
- **IMPEncoderChnAttr**: 编码器通道属性

## 2. API 详细分析

### 2.1 系统初始化 (imp_system.h)

#### 核心函数
```c
// 系统初始化
int IMP_System_Init(void);

// 系统退出
int IMP_System_Exit(void);

// 模块绑定（建立数据流）
int IMP_System_Bind(IMPCell *srcCell, IMPCell *dstCell);

// 解绑
int IMP_System_UnBind(IMPCell *srcCell, IMPCell *dstCell);

// 获取版本信息
const char *IMP_System_GetVersion(void);
```

#### 使用模式
```c
// 典型初始化流程
IMP_System_Init();

// 绑定示例：FrameSource Channel0 → Encoder Group0
IMPCell fs_chn0 = {DEV_ID_FS, 0, 0};    // FrameSource, Group0, Output0
IMPCell enc_grp0 = {DEV_ID_ENC, 0, 0}; // Encoder, Group0, Output0（输入时无效）
IMP_System_Bind(&fs_chn0, &enc_grp0);
```

### 2.2 视频输入 (imp_framesource.h)

#### 通道属性结构
```c
typedef struct {
    int picWidth;               // 图像宽度
    int picHeight;              // 图像高度
    IMPPixelFormat pixFmt;      // 像素格式
    IMPFSChnCrop crop;          // 裁剪属性
    IMPFSChnScaler scaler;      // 缩放属性
    int outFrmRateNum;          // 输出帧率分子
    int outFrmRateDen;          // 输出帧率分母
    int nrVBs;                  // Video buffer数量
    IMPFSChnType type;          // 通道类型（物理/扩展）
} IMPFSChnAttr;
```

#### 核心函数
```c
// 创建通道
int IMP_FrameSource_CreateChn(int chnNum, IMPFSChnAttr *chnAttr);

// 销毁通道
int IMP_FrameSource_DestroyChn(int chnNum);

// 使能通道（开始输出）
int IMP_FrameSource_EnableChn(int chnNum);

// 关闭通道
int IMP_FrameSource_DisableChn(int chnNum);

// 设置/获取通道属性
int IMP_FrameSource_SetChnAttr(int chnNum, const IMPFSChnAttr *chnAttr);
int IMP_FrameSource_GetChnAttr(int chnNum, IMPFSChnAttr *chnAttr);

// 设置帧缓存深度
int IMP_FrameSource_SetFrameDepth(int chnNum, int depth);

// 获取/释放帧
int IMP_FrameSource_GetFrame(int chnNum, IMPFrameInfo **frame);
int IMP_FrameSource_ReleaseFrame(int chnNum, IMPFrameInfo *frame);

// 带超时的获取帧
int IMP_FrameSource_GetTimedFrame(int chnNum, IMPFrameTimestamp *framets, 
                                   int block, void *framedata, IMPFrameInfo *frame);
```

#### 帧信息结构
```c
typedef struct {
    IMPPixelFormat pixfmt;      // 像素格式
    int width;                  // 宽度
    int height;                 // 高度
    void *virAddr;              // 虚拟地址
    uint32_t phyAddr;           // 物理地址
    uint64_t timeStamp;         // 时间戳
} IMPFrameInfo;
```

### 2.3 视频编码 (imp_encoder.h)

#### 编码器属性结构
```c
typedef struct {
    IMPEncoderPayloadType type;     // 编码类型（H264/H265/JPEG）
    IMPEncoderProfile profile;      // 编码档次
    IMPEncoderRcMode rcMode;       // 码率控制模式
    IMPEncoderGopMode gopMode;     // GOP模式
    IMPEncoderAttr attr;           // 编码属性
} IMPEncoderChnAttr;
```

#### 核心函数
```c
// 创建编码通道
int IMP_Encoder_CreateChn(int chnNum, IMPEncoderChnAttr *attr);

// 销毁编码通道
int IMP_Encoder_DestroyChn(int chnNum);

// 开始接收图像
int IMP_Encoder_StartRecvPic(int chnNum);

// 停止接收图像
int IMP_Encoder_StopRecvPic(int chnNum);

// 注册到Group
int IMP_Encoder_RegisterChn(int encGrp, int encChn);

// 注销从Group
int IMP_Encoder_UnRegisterChn(int encGrp, int encChn);

// 获取编码流
int IMP_Encoder_GetStream(int chnNum, IMPEncoderStream *stream, bool blockFlag);

// 释放编码流
int IMP_Encoder_ReleaseStream(int chnNum, IMPEncoderStream *stream);

// 请求IDR帧
int IMP_Encoder_RequestIdr(int chnNum, bool blockFlag);
```

#### 码流结构
```c
typedef struct {
    IMPEncoderPack *pack;       // 码包数组
    int packCount;              // 码包数量
    IMPEncoderStreamInfo info;  // 流信息
} IMPEncoderStream;

typedef struct {
    void *virAddr;              // 虚拟地址
    uint32_t phyAddr;           // 物理地址
    uint32_t length;            // 长度
    IMPEncoderNaluType type;    // NALU类型
    uint64_t timeStamp;         // 时间戳
} IMPEncoderPack;
```

### 2.4 音频处理 (imp_audio.h)

#### 音频属性结构
```c
typedef struct {
    IMPAudioSampleRate samplerate;  // 采样率
    IMPAudioSoundMode soundmode;    // 声道模式
    IMPAudioBitWidth bitwidth;      // 位宽
    int frmNum;                     // 帧数
} IMPAudioIOAttr;
```

#### 核心函数
```c
// 设置音频公共属性
int IMP_Audio_SetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

// 获取音频公共属性
int IMP_Audio_GetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

// 使能音频设备
int IMP_Audio_Enable(int audioDevId);

// 关闭音频设备
int IMP_Audio_Disable(int audioDevId);

// 录音（获取音频帧）
int IMP_Audio_Record(int audioDevId, void *data, int size);

// 播放（输出音频）
int IMP_Audio_Play(int audioDevId, void *data, int size);

// 设置音量
int IMP_Audio_SetVol(int audioDevId, int chnId, int vol);

// 获取音量
int IMP_Audio_GetVol(int audioDevId, int chnId, int *vol);
```

### 2.5 OSD叠加 (imp_osd.h)

#### 区域属性结构
```c
typedef struct {
    IMPOSDRegionType type;      // 区域类型（图片/文本/时间等）
    IMPRect rect;               // 区域矩形
    IMPOSDFormat format;        // 格式
    IMPOSDBmpAttr bmpattr;      // 位图属性
    IMPOSDTextAttr textattr;    // 文本属性
} IMPOSDRegionAttr;
```

#### 核心函数
```c
// 创建OSD区域
int IMP_OSD_CreateRegion(IMPOSDRegion *region, IMPOSDRegionAttr *attr);

// 销毁OSD区域
int IMP_OSD_DestroyRegion(IMPOSDRegion *region);

// 显示区域
int IMP_OSD_ShowRegion(IMPOSDRegion *region);

// 隐藏区域
int IMP_OSD_HideRegion(IMPOSDRegion *region);

// 设置区域位置
int IMP_OSD_SetRegionPosition(IMPOSDRegion *region, IMPRect *rect);

// 设置区域位图
int IMP_OSD_SetRegionBitmap(IMPOSDRegion *region, IMPOSDBmp *bmp);

// 设置区域文本
int IMP_OSD_SetRegionText(IMPOSDRegion *region, char *text);
```

### 2.6 ISP处理 (imp_isp.h)

#### ISP核心函数
```c
// 打开ISP
int IMP_ISP_Open(void);

// 关闭ISP
int IMP_ISP_Close(void);

// 添加传感器
int IMP_ISP_AddSensor(IMPSensorInfo *info);

// 使能传感器
int IMP_ISP_EnableSensor(void);

// 关闭传感器
int IMP_ISP_DisableSensor(void);

// 设置AE模式
int IMP_ISP_SetAeMode(IMPIspAeMode mode);

// 设置曝光时间
int IMP_ISP_SetExposure(int expTime);

// 设置增益
int IMP_ISP_SetGain(int gain);

// 设置白平衡模式
int IMP_ISP_SetWBMode(IMPIspWBMode mode);
```

## 3. CVITEK 到 Ingenic API 映射表

### 3.1 系统初始化映射

| CVITEK API | Ingenic API | 说明 |
|------------|-------------|------|
| `CVI_SYS_Init()` | `IMP_System_Init()` | 系统初始化 |
| `CVI_VB_Init()` | `IMP_FrameSource_SetFrameDepth()` | 设置帧缓存深度 |
| `CVI_SYS_Bind()` | `IMP_System_Bind()` | 模块绑定 |
| `CVI_SYS_UnBind()` | `IMP_System_UnBind()` | 模块解绑 |
| `CVI_SYS_Exit()` | `IMP_System_Exit()` | 系统退出 |

### 3.2 视频输入映射

| CVITEK API | Ingenic API | 说明 |
|------------|-------------|------|
| `CVI_VI_SetDevAttr()` | `IMP_FrameSource_CreateChn()` | 创建视频通道 |
| `CVI_VI_SetChnAttr()` | `IMP_FrameSource_SetChnAttr()` | 设置通道属性 |
| `CVI_VI_EnableDev()` | `IMP_FrameSource_EnableChn()` | 使能通道 |
| `CVI_VI_DisableDev()` | `IMP_FrameSource_DisableChn()` | 关闭通道 |
| `CVI_VI_GetChnFrame()` | `IMP_FrameSource_GetFrame()` | 获取视频帧 |
| `CVI_VI_ReleaseChnFrame()` | `IMP_FrameSource_ReleaseFrame()` | 释放视频帧 |
| `CVI_VI_SetPipeAttr()` | `IMP_ISP_Set*()` 系列 | ISP参数设置 |

### 3.3 视频编码映射

| CVITEK API | Ingenic API | 说明 |
|------------|-------------|------|
| `CVI_VENC_CreateChn()` | `IMP_Encoder_CreateChn()` | 创建编码通道 |
| `CVI_VENC_StartRecvFrame()` | `IMP_Encoder_StartRecvPic()` | 开始接收图像 |
| `CVI_VENC_GetStream()` | `IMP_Encoder_GetStream()` | 获取编码流 |
| `CVI_VENC_ReleaseStream()` | `IMP_Encoder_ReleaseStream()` | 释放编码流 |
| `CVI_VENC_RequestIDR()` | `IMP_Encoder_RequestIdr()` | 请求IDR帧 |
| `CVI_VENC_DestroyChn()` | `IMP_Encoder_DestroyChn()` | 销毁编码通道 |

### 3.4 音频处理映射

| CVITEK API | Ingenic API | 说明 |
|------------|-------------|------|
| `CVI_AUDIO_Init()` | `IMP_Audio_SetPubAttr()` + `IMP_Audio_Enable()` | 音频初始化 |
| `CVI_AUDIO_Start()` | `IMP_Audio_Enable()` | 启动音频 |
| `CVI_AUDIO_Stop()` | `IMP_Audio_Disable()` | 停止音频 |
| `CVI_AUDIO_ReadFrame()` | `IMP_Audio_Record()` | 读取音频帧 |
| `CVI_AUDIO_WriteFrame()` | `IMP_Audio_Play()` | 写入音频帧 |
| `CVI_AUDIO_SetVolume()` | `IMP_Audio_SetVol()` | 设置音量 |

### 3.5 OSD叠加映射

| CVITEK API | Ingenic API | 说明 |
|------------|-------------|------|
| `CVI_RGN_Create()` | `IMP_OSD_CreateRegion()` | 创建区域 |
| `CVI_RGN_AttachToChn()` | `IMP_OSD_ShowRegion()` | 显示区域 |
| `CVI_RGN_DetachFromChn()` | `IMP_OSD_HideRegion()` | 隐藏区域 |
| `CVI_RGN_SetBitMap()` | `IMP_OSD_SetRegionBitmap()` | 设置位图 |
| `CVI_RGN_SetDisplayAttr()` | `IMP_OSD_SetRegionPosition()` | 设置显示属性 |
| `CVI_RGN_Destroy()` | `IMP_OSD_DestroyRegion()` | 销毁区域 |

### 3.6 AI推理映射

| CVITEK API | Ingenic API | 说明 |
|------------|-------------|------|
| `CVI_AI_CreateHandle()` | 无直接对应 | Ingenic无专用AI硬件，需软件实现 |
| `CVI_AI_Forward()` | TensorFlow Lite / NCNN | 使用开源推理引擎 |
| `CVI_AI_DestroyHandle()` | 对应库的释放函数 | 释放资源 |

**注意**: Ingenic T31可能没有专用的AI硬件加速单元，AI功能可能需要：
1. 使用CPU运行轻量级模型
2. 禁用某些AI功能
3. 使用外置AI协处理器

## 4. 示例代码分析

### 4.1 视频采集和编码示例（简化版）

```c
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

int main(void)
{
    // 1. 系统初始化
    IMP_System_Init();
    
    // 2. 创建FrameSource通道
    IMPFSChnAttr fs_attr = {
        .picWidth = 1920,
        .picHeight = 1080,
        .pixFmt = PIX_FMT_NV12,
        .outFrmRateNum = 30,
        .outFrmRateDen = 1,
        .nrVBs = 3,
        .type = FS_PHY_CHANNEL,
    };
    IMP_FrameSource_CreateChn(0, &fs_attr);
    IMP_FrameSource_EnableChn(0);
    
    // 3. 创建编码通道
    IMPEncoderChnAttr enc_attr = {
        .type = PT_H264,
        .profile = PROFILE_H264_MAIN,
        .rcMode = IMP_ENC_RC_MODE_CBR,
        .attr = {
            .encAttr = {
                .h264Attr = {
                    .width = 1920,
                    .height = 1080,
                    .fpsNum = 30,
                    .fpsDen = 1,
                    .bitRate = 4000000, // 4 Mbps
                    .gop = 30,
                }
            }
        }
    };
    IMP_Encoder_CreateChn(0, &enc_attr);
    IMP_Encoder_RegisterChn(0, 0); // 注册到Group 0
    IMP_Encoder_StartRecvPic(0);
    
    // 4. 绑定数据流
    IMPCell fs_cell = {DEV_ID_FS, 0, 0};
    IMPCell enc_cell = {DEV_ID_ENC, 0, 0};
    IMP_System_Bind(&fs_cell, &enc_cell);
    
    // 5. 获取编码流
    IMPEncoderStream stream;
    while (1) {
        IMP_Encoder_GetStream(0, &stream, BLOCK);
        // 处理编码流
        IMP_Encoder_ReleaseStream(0, &stream);
    }
    
    // 6. 清理资源
    IMP_System_UnBind(&fs_cell, &enc_cell);
    IMP_Encoder_StopRecvPic(0);
    IMP_Encoder_UnRegisterChn(0, 0);
    IMP_Encoder_DestroyChn(0);
    IMP_FrameSource_DisableChn(0);
    IMP_FrameSource_DestroyChn(0);
    IMP_System_Exit();
    
    return 0;
}
```

### 4.2 音频采集示例

```c
#include <imp/imp_audio.h>

int audio_init(void)
{
    IMPAudioIOAttr attr = {
        .samplerate = AUDIO_SAMPLE_RATE_8000,
        .soundmode = AUDIO_SOUND_MODE_MONO,
        .bitwidth = AUDIO_BIT_WIDTH_16,
        .frmNum = 20,
    };
    
    // 设置音频属性
    IMP_Audio_SetPubAttr(AUDIO_DEV_ID_AI, &attr);
    IMP_Audio_SetPubAttr(AUDIO_DEV_ID_AO, &attr);
    
    // 使能音频设备
    IMP_Audio_Enable(AUDIO_DEV_ID_AI);
    IMP_Audio_Enable(AUDIO_DEV_ID_AO);
    
    return 0;
}
```

## 5. 架构差异与兼容性考虑

### 5.1 关键差异

1. **内存管理**: 
   - CVITEK: 需要显式调用 `CVI_VB_Init()` 初始化视频缓存池
   - Ingenic: 通过 `IMP_FrameSource_SetFrameDepth()` 设置帧深度，系统自动管理

2. **模块绑定**:
   - CVITEK: 使用 `CVI_SYS_Bind()` 绑定具体的源和目的通道
   - Ingenic: 使用 `IMP_System_Bind()` 绑定Cell，更灵活但概念更抽象

3. **ISP控制**:
   - CVITEK: 通过 `CVI_ISP_*` 系列函数控制
   - Ingenic: 通过 `IMP_ISP_*` 系列函数，API设计不同但功能相似

4. **AI支持**:
   - CVITEK: 有专用AI硬件和 `CVI_AI_*` API
   - Ingenic: 可能没有硬件AI加速，需要软件方案

### 5.2 兼容性策略

#### 策略1：完全抽象层
- 在HAL层完全隐藏平台差异
- 应用程序只调用HAL接口
- 平台适配器实现具体功能

#### 策略2：条件编译
- 使用 `#ifdef PLATFORM_CVITEK` / `#ifdef PLATFORM_INGENIC`
- 保持代码清晰，但增加维护复杂度

#### 策略3：运行时检测
- 动态加载平台特定库
- 支持热切换平台（如插拔不同硬件模块）

**推荐策略**: 策略1 + 策略2结合
- 核心功能通过HAL抽象
- 平台特有功能使用条件编译
- 提供统一的错误处理机制

## 6. 下一步行动计划

### 阶段1：深入理解IMP API（当前阶段）
- [x] 分析核心头文件
- [x] 研究示例代码
- [x] 创建API映射表
- [ ] 编写简单的测试程序验证理解

### 阶段2：设计Ingenic平台适配器
- [ ] 基于HAL接口设计Ingenic实现
- [ ] 实现系统初始化和视频采集
- [ ] 实现视频编码和音频处理
- [ ] 处理平台差异（如AI功能降级）

### 阶段3：移植验证
- [ ] 创建最小化测试工程
- [ ] 验证视频采集→编码→输出流程
- [ ] 性能测试和优化
- [ ] 完整功能集成测试

### 阶段4：多平台支持扩展
- [ ] 设计Rockchip平台适配器
- [ ] 创建统一的构建系统
- [ ] 编写跨平台开发指南

## 7. 风险与挑战

### 技术风险
1. **性能差异**: Ingenic T31的性能可能低于CVITEK平台，需要优化
2. **功能缺失**: 某些CVITEK特有功能在Ingenic上无法实现
3. **稳定性**: 新平台可能存在驱动或硬件问题

### 缓解措施
1. **渐进式移植**: 先实现核心功能，再添加高级特性
2. **降级方案**: 为缺失功能提供软件替代或禁用选项
3. **充分测试**: 建立自动化测试框架，确保稳定性

## 8. 结论

Ingenic IMP API提供了完整的视频处理框架，虽然与CVITEK API在设计理念和具体实现上存在差异，但核心功能基本对应。通过精心设计的硬件抽象层，可以实现跨平台支持。

**关键成功因素**:
1. 充分理解两个平台的API设计哲学
2. 设计灵活的抽象层，平衡性能与可移植性
3. 建立完善的测试验证体系
4. 文档化所有设计决策和兼容性考虑

**下一步**: 基于本分析报告，开始设计Ingenic平台适配器的具体实现。

---

**分析完成时间**: 2026-03-20  
**分析者**: kid (🦊)  
**文档版本**: 1.0