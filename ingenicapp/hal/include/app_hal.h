/**
 * @file app_hal.h
 * @brief Hardware Abstraction Layer for IP Camera Applications
 * 
 * This header defines a unified interface for IP camera hardware operations,
 * allowing the same application code to run on multiple platforms:
 * - CVITEK (CVI_* API)
 * - Ingenic (IMP_* API) 
 * - Rockchip (RK_* API)
 * - Future platforms
 * 
 * Design Principles:
 * 1. Platform-agnostic interface definitions
 * 2. Minimal abstraction overhead
 * 3. Support for real-time IP camera requirements
 * 4. Extensible for future platforms
 * 
 * @version 1.0.0
 * @date 2026-03-20
 */

#ifndef __APP_HAL_H__
#define __APP_HAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*-----------------------------------------------------------------------------
 *  Common Types and Macros
 *----------------------------------------------------------------------------*/

/**
 * @brief Standard return codes (platform-independent)
 */
typedef enum {
    HAL_OK           = 0,    /**< Success */
    HAL_ERR          = -1,   /**< General error */
    HAL_ERR_PARAM    = -2,   /**< Invalid parameter */
    HAL_ERR_TIMEOUT  = -3,   /**< Operation timeout */
    HAL_ERR_NO_MEM   = -4,   /**< Memory allocation failed */
    HAL_ERR_NOT_INIT = -5,   /**< Module not initialized */
    HAL_ERR_BUSY     = -6,   /**< Resource busy */
    HAL_ERR_IO       = -7,   /**< I/O error */
    HAL_ERR_UNSUPPORTED = -8, /**< Feature not supported */
} hal_err_t;

/**
 * @brief Video frame format
 */
typedef enum {
    HAL_FMT_YUV420SP,     /**< YUV420 semi-planar (NV12/NV21) */
    HAL_FMT_YUV420P,      /**< YUV420 planar (I420) */
    HAL_FMT_YUV422SP,     /**< YUV422 semi-planar */
    HAL_FMT_YUV422P,      /**< YUV422 planar */
    HAL_FMT_RGB888,       /**< RGB 24-bit */
    HAL_FMT_BGR888,       /**< BGR 24-bit */
    HAL_FMT_RGBA8888,     /**< RGBA 32-bit */
    HAL_FMT_BGRA8888,     /**< BGRA 32-bit */
    HAL_FMT_H264,         /**< H.264 encoded stream */
    HAL_FMT_H265,         /**< H.265 encoded stream */
    HAL_FMT_JPEG,         /**< JPEG encoded image */
} hal_video_format_t;

/**
 * @brief Audio format
 */
typedef enum {
    HAL_AUDIO_FMT_PCM,    /**< PCM raw audio */
    HAL_AUDIO_FMT_AAC,    /**< AAC encoded audio */
    HAL_AUDIO_FMT_G711A,  /**< G.711 A-law */
    HAL_AUDIO_FMT_G711U,  /**< G.711 μ-law */
} hal_audio_format_t;

/**
 * @brief Video frame structure
 */
typedef struct {
    void *data;                 /**< Frame data pointer */
    uint32_t size;              /**< Frame size in bytes */
    uint32_t width;             /**< Frame width in pixels */
    uint32_t height;            /**< Frame height in pixels */
    hal_video_format_t format;  /**< Frame format */
    uint64_t timestamp;         /**< Timestamp in microseconds */
    uint32_t sequence;          /**< Frame sequence number */
    uint32_t stride;            /**< Line stride in bytes */
} hal_video_frame_t;

/**
 * @brief Audio frame structure
 */
typedef struct {
    void *data;                 /**< Audio data pointer */
    uint32_t size;              /**< Audio size in bytes */
    hal_audio_format_t format;  /**< Audio format */
    uint32_t sample_rate;       /**< Sample rate in Hz */
    uint32_t channels;          /**< Number of channels */
    uint64_t timestamp;         /**< Timestamp in microseconds */
} hal_audio_frame_t;

/*-----------------------------------------------------------------------------
 *  System Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief System configuration
 */
typedef struct {
    uint32_t video_mem_size;    /**< Video memory pool size in MB */
    uint32_t audio_mem_size;    /**< Audio memory pool size in MB */
    uint32_t ai_mem_size;       /**< AI memory pool size in MB */
    bool enable_hardware_codec; /**< Enable hardware codec acceleration */
    bool enable_ai_accel;       /**< Enable AI hardware acceleration */
    const char *sensor_model;   /**< Sensor model name (e.g., "GC1084") */
} hal_sys_config_t;

/**
 * @brief Initialize hardware abstraction layer
 * 
 * @param config System configuration
 * @return hal_err_t Error code
 */
hal_err_t hal_sys_init(const hal_sys_config_t *config);

/**
 * @brief Deinitialize hardware abstraction layer
 * 
 * @return hal_err_t Error code
 */
hal_err_t hal_sys_deinit(void);

/**
 * @brief Get platform information
 * 
 * @param platform_name Buffer to store platform name
 * @param buf_size Buffer size
 * @return hal_err_t Error code
 */
hal_err_t hal_sys_get_platform(char *platform_name, uint32_t buf_size);

/*-----------------------------------------------------------------------------
 *  Video Input (VI) Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief Video input configuration
 */
typedef struct {
    uint32_t width;             /**< Input width in pixels */
    uint32_t height;            /**< Input height in pixels */
    hal_video_format_t format;  /**< Input format */
    uint32_t fps;               /**< Frame rate (frames per second) */
    uint32_t channel_id;        /**< Channel ID (0-based) */
    uint32_t sensor_id;         /**< Sensor ID */
    const char *sensor_name;    /**< Sensor name (e.g., "gc1084") */
} hal_vi_config_t;

/**
 * @brief Initialize video input module
 * 
 * @param config Video input configuration
 * @return hal_err_t Error code
 */
hal_err_t hal_vi_init(const hal_vi_config_t *config);

/**
 * @brief Start video capture
 * 
 * @param channel_id Channel ID to start
 * @return hal_err_t Error code
 */
hal_err_t hal_vi_start(uint32_t channel_id);

/**
 * @brief Stop video capture
 * 
 * @param channel_id Channel ID to stop
 * @return hal_err_t Error code
 */
hal_err_t hal_vi_stop(uint32_t channel_id);

/**
 * @brief Get video frame
 * 
 * @param channel_id Channel ID
 * @param frame Frame structure to fill
 * @param timeout_ms Timeout in milliseconds
 * @return hal_err_t Error code
 */
hal_err_t hal_vi_get_frame(uint32_t channel_id, hal_video_frame_t *frame, uint32_t timeout_ms);

/**
 * @brief Release video frame (return buffer to pool)
 * 
 * @param channel_id Channel ID
 * @param frame Frame to release
 * @return hal_err_t Error code
 */
hal_err_t hal_vi_release_frame(uint32_t channel_id, const hal_video_frame_t *frame);

/*-----------------------------------------------------------------------------
 *  Video Processing (VPSS) Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief Video processing configuration
 */
typedef struct {
    uint32_t input_width;       /**< Input width */
    uint32_t input_height;      /**< Input height */
    hal_video_format_t input_format; /**< Input format */
    uint32_t output_width;      /**< Output width */
    uint32_t output_height;     /**< Output height */
    hal_video_format_t output_format; /**< Output format */
    uint32_t crop_x;            /**< Crop region X offset */
    uint32_t crop_y;            /**< Crop region Y offset */
    uint32_t crop_width;        /**< Crop region width */
    uint32_t crop_height;       /**< Crop region height */
} hal_vpss_config_t;

/**
 * @brief Initialize video processing subsystem
 * 
 * @param config VPSS configuration
 * @return hal_err_t Error code
 */
hal_err_t hal_vpss_init(const hal_vpss_config_t *config);

/**
 * @brief Process video frame
 * 
 * @param in_frame Input frame
 * @param out_frame Output frame
 * @return hal_err_t Error code
 */
hal_err_t hal_vpss_process(const hal_video_frame_t *in_frame, hal_video_frame_t *out_frame);

/*-----------------------------------------------------------------------------
 *  Video Encoding (VENC) Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief Video encoder configuration
 */
typedef struct {
    hal_video_format_t codec;   /**< Codec type (H264, H265, JPEG) */
    uint32_t width;             /**< Encoding width */
    uint32_t height;            /**< Encoding height */
    uint32_t fps;               /**< Target frame rate */
    uint32_t bitrate;           /**< Target bitrate in bps */
    uint32_t gop_size;          /**< GOP size (keyframe interval) */
    uint32_t profile;           /**< Codec profile */
    uint32_t level;             /**< Codec level */
} hal_venc_config_t;

/**
 * @brief Initialize video encoder
 * 
 * @param config Encoder configuration
 * @return hal_err_t Error code
 */
hal_err_t hal_venc_init(const hal_venc_config_t *config);

/**
 * @brief Encode video frame
 * 
 * @param in_frame Input raw frame
 * @param out_packet Output encoded packet
 * @return hal_err_t Error code
 */
hal_err_t hal_venc_encode(const hal_video_frame_t *in_frame, hal_video_frame_t *out_packet);

/**
 * @brief Request IDR frame (keyframe)
 * 
 * @param channel_id Channel ID
 * @return hal_err_t Error code
 */
hal_err_t hal_venc_request_idr(uint32_t channel_id);

/*-----------------------------------------------------------------------------
 *  Audio Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief Audio configuration
 */
typedef struct {
    hal_audio_format_t format;  /**< Audio format */
    uint32_t sample_rate;       /**< Sample rate in Hz */
    uint32_t channels;          /**< Number of channels (1=mono, 2=stereo) */
    uint32_t bitrate;           /**< Bitrate in bps (for encoded formats) */
} hal_audio_config_t;

/**
 * @brief Initialize audio module
 * 
 * @param config Audio configuration
 * @return hal_err_t Error code
 */
hal_err_t hal_audio_init(const hal_audio_config_t *config);

/**
 * @brief Start audio capture
 * 
 * @return hal_err_t Error code
 */
hal_err_t hal_audio_start(void);

/**
 * @brief Stop audio capture
 * 
 * @return hal_err_t Error code
 */
hal_err_t hal_audio_stop(void);

/**
 * @brief Get audio frame
 * 
 * @param frame Audio frame structure to fill
 * @param timeout_ms Timeout in milliseconds
 * @return hal_err_t Error code
 */
hal_err_t hal_audio_get_frame(hal_audio_frame_t *frame, uint32_t timeout_ms);

/*-----------------------------------------------------------------------------
 *  AI Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief AI model type
 */
typedef enum {
    HAL_AI_MODEL_FACE_DETECTION,    /**< Face detection model */
    HAL_AI_MODEL_PERSON_DETECTION,  /**< Person detection model */
    HAL_AI_MODEL_MOTION_DETECTION,  /**< Motion detection model */
    HAL_AI_MODEL_LICENSE_PLATE,     /**< License plate recognition */
    HAL_AI_MODEL_CUSTOM,            /**< Custom model */
} hal_ai_model_type_t;

/**
 * @brief AI detection result
 */
typedef struct {
    hal_ai_model_type_t model_type; /**< Model type */
    uint32_t object_id;             /**< Object ID */
    uint32_t x;                     /**< Bounding box X coordinate */
    uint32_t y;                     /**< Bounding box Y coordinate */
    uint32_t width;                 /**< Bounding box width */
    uint32_t height;                /**< Bounding box height */
    float confidence;               /**< Detection confidence (0.0-1.0) */
    char label[32];                 /**< Object label */
} hal_ai_detection_t;

/**
 * @brief AI configuration
 */
typedef struct {
    hal_ai_model_type_t model_type; /**< Model type */
    const char *model_path;         /**< Path to model file */
    float confidence_threshold;     /**< Detection confidence threshold */
    uint32_t max_detections;        /**< Maximum number of detections per frame */
} hal_ai_config_t;

/**
 * @brief Initialize AI module
 * 
 * @param config AI configuration
 * @return hal_err_t Error code
 */
hal_err_t hal_ai_init(const hal_ai_config_t *config);

/**
 * @brief Process frame with AI model
 * 
 * @param frame Input video frame
 * @param detections Array to store detection results
 * @param max_detections Maximum number of detections to return
 * @param num_detections Actual number of detections found
 * @return hal_err_t Error code
 */
hal_err_t hal_ai_process(const hal_video_frame_t *frame, 
                         hal_ai_detection_t *detections, 
                         uint32_t max_detections, 
                         uint32_t *num_detections);

/*-----------------------------------------------------------------------------
 *  OSD (On-Screen Display) Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief OSD element type
 */
typedef enum {
    HAL_OSD_TYPE_TEXT,      /**< Text element */
    HAL_OSD_TYPE_RECT,      /**< Rectangle element */
    HAL_OSD_TYPE_LINE,      /**< Line element */
    HAL_OSD_TYPE_IMAGE,     /**< Image element */
    HAL_OSD_TYPE_TIME,      /**< Time display element */
} hal_osd_type_t;

/**
 * @brief OSD element
 */
typedef struct {
    hal_osd_type_t type;    /**< Element type */
    uint32_t x;             /**< X coordinate */
    uint32_t y;             /**< Y coordinate */
    uint32_t width;         /**< Width */
    uint32_t height;        /**< Height */
    uint32_t color;         /**< Color (ARGB format) */
    uint32_t bg_color;      /**< Background color (ARGB format) */
    const char *text;       /**< Text content (for text elements) */
    const void *image_data; /**< Image data (for image elements) */
} hal_osd_element_t;

/**
 * @brief Initialize OSD module
 * 
 * @return hal_err_t Error code
 */
hal_err_t hal_osd_init(void);

/**
 * @brief Add OSD element to display
 * 
 * @param channel_id Channel ID
 * @param element OSD element to add
 * @param element_id Output element ID
 * @return hal_err_t Error code
 */
hal_err_t hal_osd_add_element(uint32_t channel_id, const hal_osd_element_t *element, uint32_t *element_id);

/**
 * @brief Update OSD element
 * 
 * @param channel_id Channel ID
 * @param element_id Element ID
 * @param element Updated element data
 * @return hal_err_t Error code
 */
hal_err_t hal_osd_update_element(uint32_t channel_id, uint32_t element_id, const hal_osd_element_t *element);

/*-----------------------------------------------------------------------------
 *  Peripheral Abstraction
 *----------------------------------------------------------------------------*/

/**
 * @brief IR-CUT control
 */
typedef enum {
    HAL_IRCUT_DAY_MODE,     /**< Day mode (IR-CUT filter enabled) */
    HAL_IRCUT_NIGHT_MODE,   /**< Night mode (IR-CUT filter disabled) */
    HAL_IRCUT_AUTO_MODE,    /**< Auto mode (based on light sensor) */
} hal_ircut_mode_t;

/**
 * @brief LED control
 */
typedef enum {
    HAL_LED_OFF,            /**< LED off */
    HAL_LED_ON,             /**< LED on */
    HAL_LED_BLINK_SLOW,     /**< Slow blinking */
    HAL_LED_BLINK_FAST,     /**< Fast blinking */
} hal_led_mode_t;

/**
 * @brief Set IR-CUT mode
 * 
 * @param mode IR-CUT mode
 * @return hal_err_t Error code
 */
hal_err_t hal_peripheral_set_ircut(hal_ircut_mode_t mode);

/**
 * @brief Control LED
 * 
 * @param led_id LED ID (0-based)
 * @param mode LED mode
 * @return hal_err_t Error code
 */
hal_err_t hal_peripheral_set_led(uint32_t led_id, hal_led_mode_t mode);

/**
 * @brief Read light sensor value
 * 
 * @param value Output light level (0-100%)
 * @return hal_err_t Error code
 */
hal_err_t hal_peripheral_read_light_sensor(float *value);

#ifdef __cplusplus
}
#endif

#endif /* __APP_HAL_H__ */