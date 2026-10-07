// HAL - Hardware Abstraction Layer
// Unified interface for IP camera applications
// Supports: CVITEK, Ingenic (T31), RK platforms

#ifndef _HAL_COMMON_H_
#define _HAL_COMMON_H_

#include <stdint.h>
#include <stdbool.h>

// Error codes
#define HAL_OK                  0
#define HAL_ERROR              -1
#define HAL_ERROR_PARAM        -2
#define HAL_ERROR_MEMORY       -3
#define HAL_ERROR_TIMEOUT      -4
#define HAL_ERROR_NOT_SUPPORT  -5
#define HAL_ERROR_NOT_INIT     -6
#define HAL_ERROR_BUSY         -7

// Platform definition
#ifndef PLATFORM_CVITEK
#define PLATFORM_CVITEK    1
#endif
#ifndef PLATFORM_INGENIC
#define PLATFORM_INGENIC   2
#endif
#ifndef PLATFORM_RK
#define PLATFORM_RK        3
#endif

// Current platform selection
#ifndef PLATFORM_TYPE
#define PLATFORM_TYPE PLATFORM_INGENIC
#endif

// Log macros
#define HAL_LOG_LEVEL_NONE   0
#define HAL_LOG_LEVEL_ERROR  1
#define HAL_LOG_LEVEL_WARN   2
#define HAL_LOG_LEVEL_INFO   3
#define HAL_LOG_LEVEL_DEBUG  4

#ifndef HAL_LOG_LEVEL
#define HAL_LOG_LEVEL HAL_LOG_LEVEL_INFO
#endif

#define HAL_LOG(level, fmt, ...) \
    do { \
        if (level <= HAL_LOG_LEVEL) { \
            printf("[HAL][%s] " fmt, #level, ##__VA_ARGS__); \
        } \
    } while(0)

#define HAL_LOG_ERROR(fmt, ...) HAL_LOG(HAL_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define HAL_LOG_WARN(fmt, ...)  HAL_LOG(HAL_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define HAL_LOG_INFO(fmt, ...)  HAL_LOG(HAL_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define HAL_LOG_DEBUG(fmt, ...) HAL_LOG(HAL_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

// Common data types
typedef struct {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
} hal_rect_t;

typedef struct {
    uint32_t width;
    uint32_t height;
} hal_size_t;

typedef struct {
    uint32_t id;
    char name[32];
    uint32_t caps;  // capability bitmap
} hal_device_info_t;

typedef enum {
    HAL_PIXEL_FORMAT_YUV_SEMIPLANAR_420 = 0,  // NV12/NV21
    HAL_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    HAL_PIXEL_FORMAT_YUV_PLANAR_420,          // I420/YV12
    HAL_PIXEL_FORMAT_YUV_PLANAR_422,
    HAL_PIXEL_FORMAT_RGB_888,
    HAL_PIXEL_FORMAT_BGR_888,
    HAL_PIXEL_FORMAT_ARGB_8888,
    HAL_PIXEL_FORMAT_ABGR_8888,
    HAL_PIXEL_FORMAT_RAW_BAYER_8,
    HAL_PIXEL_FORMAT_RAW_BAYER_10,
    HAL_PIXEL_FORMAT_RAW_BAYER_12,
} hal_pixel_format_t;

typedef enum {
    HAL_VIDEO_CODEC_H264 = 0,
    HAL_VIDEO_CODEC_H265,
    HAL_VIDEO_CODEC_JPEG,
    HAL_VIDEO_CODEC_MJPEG,
} hal_video_codec_t;

typedef enum {
    HAL_AUDIO_CODEC_AAC = 0,
    HAL_AUDIO_CODEC_G711A,
    HAL_AUDIO_CODEC_G711U,
    HAL_AUDIO_CODEC_MP3,
    HAL_AUDIO_CODEC_PCM,
} hal_audio_codec_t;

// Memory buffer descriptor
typedef struct {
    void *virt_addr;     // Virtual address
    uint32_t phys_addr;  // Physical address
    uint32_t size;       // Buffer size
    uint32_t fd;         // File descriptor for DMA buffer
    void *priv;          // Platform private data
} hal_buffer_t;

// Frame descriptor
typedef struct {
    hal_buffer_t buf;          // Buffer info
    uint64_t pts;              // Presentation timestamp (us)
    uint32_t seq;              // Frame sequence number
    hal_pixel_format_t format; // Pixel format
    uint32_t width;            // Frame width
    uint32_t height;           // Frame height
    uint32_t stride;           // Line stride (bytes)
    uint32_t size;             // Actual data size
    bool key_frame;            // Is key frame (for encoded frames)
    void *priv;                // Platform private data
} hal_frame_t;

// Stream configuration
typedef struct {
    hal_video_codec_t codec;   // Video codec type
    uint32_t width;           // Stream width
    uint32_t height;          // Stream height
    uint32_t fps;             // Frame rate
    uint32_t bitrate;         // Bitrate (bps)
    uint32_t gop;             // GOP size
    uint32_t profile;         // Codec profile
    uint32_t level;           // Codec level
    uint32_t rc_mode;         // Rate control mode
} hal_video_stream_config_t;




#endif // _HAL_COMMON_H_