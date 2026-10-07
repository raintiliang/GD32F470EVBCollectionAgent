/**
 * @file app_hal_common.c
 * @brief Common implementations for Hardware Abstraction Layer
 * 
 * This file contains common utilities and platform-independent functions
 * for the HAL implementation.
 * 
 * @version 1.0.0
 * @date 2026-03-20
 */

#include "app_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
 *  Common Utility Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Get current timestamp in microseconds
 * 
 * @return uint64_t Timestamp in microseconds
 */
static uint64_t __attribute__((unused)) get_current_timestamp_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

/**
 * @brief Convert error code to string
 * 
 * @param err Error code
 * @return const char* Error string
 */
const char *hal_err_to_string(hal_err_t err)
{
    switch (err) {
        case HAL_OK:           return "Success";
        case HAL_ERR:          return "General error";
        case HAL_ERR_PARAM:    return "Invalid parameter";
        case HAL_ERR_TIMEOUT:  return "Operation timeout";
        case HAL_ERR_NO_MEM:   return "Memory allocation failed";
        case HAL_ERR_NOT_INIT: return "Module not initialized";
        case HAL_ERR_BUSY:     return "Resource busy";
        case HAL_ERR_IO:       return "I/O error";
        case HAL_ERR_UNSUPPORTED: return "Feature not supported";
        default:               return "Unknown error";
    }
}

/**
 * @brief Print HAL debug information
 * 
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void hal_debug_print(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    printf("[HAL %s] ", timestamp);
    vprintf(format, args);
    printf("\n");
    
    va_end(args);
}

/*-----------------------------------------------------------------------------
 *  Memory Management
 *----------------------------------------------------------------------------*/

/**
 * @brief Allocate aligned memory for video/audio frames
 * 
 * @param size Size in bytes
 * @param alignment Alignment requirement (must be power of 2)
 * @return void* Pointer to allocated memory, or NULL on failure
 */
void *hal_mem_alloc_aligned(size_t size, size_t alignment)
{
    void *ptr = NULL;
    
    // Check alignment is power of 2
    if ((alignment & (alignment - 1)) != 0) {
        return NULL;
    }
    
    // Allocate with extra space for alignment and original pointer
    void *original_ptr = malloc(size + alignment + sizeof(void*));
    if (original_ptr == NULL) {
        return NULL;
    }
    
    // Calculate aligned pointer
    uintptr_t original_addr = (uintptr_t)original_ptr;
    uintptr_t aligned_addr = (original_addr + alignment + sizeof(void*) - 1) & ~(alignment - 1);
    
    // Store original pointer before aligned memory
    void **ptr_store = (void**)(aligned_addr - sizeof(void*));
    *ptr_store = original_ptr;
    
    ptr = (void*)aligned_addr;
    
    // Zero the allocated memory
    memset(ptr, 0, size);
    
    return ptr;
}

/**
 * @brief Free aligned memory
 * 
 * @param ptr Pointer returned by hal_mem_alloc_aligned()
 */
void hal_mem_free_aligned(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    
    // Get original pointer from before the aligned memory
    void **ptr_store = (void**)((uintptr_t)ptr - sizeof(void*));
    void *original_ptr = *ptr_store;
    
    free(original_ptr);
}

/**
 * @brief Allocate video frame structure
 * 
 * @param width Frame width
 * @param height Frame height
 * @param format Frame format
 * @return hal_video_frame_t* Allocated frame, or NULL on failure
 */
hal_video_frame_t *hal_video_frame_alloc(uint32_t width, uint32_t height, hal_video_format_t format)
{
    hal_video_frame_t *frame = (hal_video_frame_t*)malloc(sizeof(hal_video_frame_t));
    if (frame == NULL) {
        return NULL;
    }
    
    // Calculate frame size based on format
    size_t frame_size = 0;
    switch (format) {
        case HAL_FMT_YUV420SP:
        case HAL_FMT_YUV420P:
            frame_size = width * height * 3 / 2;
            break;
        case HAL_FMT_YUV422SP:
        case HAL_FMT_YUV422P:
            frame_size = width * height * 2;
            break;
        case HAL_FMT_RGB888:
        case HAL_FMT_BGR888:
            frame_size = width * height * 3;
            break;
        case HAL_FMT_RGBA8888:
        case HAL_FMT_BGRA8888:
            frame_size = width * height * 4;
            break;
        default:
            frame_size = width * height; // Conservative estimate
    }
    
    // Allocate aligned frame data (64-byte alignment for cache optimization)
    frame->data = hal_mem_alloc_aligned(frame_size, 64);
    if (frame->data == NULL) {
        free(frame);
        return NULL;
    }
    
    frame->size = frame_size;
    frame->width = width;
    frame->height = height;
    frame->format = format;
    frame->timestamp = 0;
    frame->sequence = 0;
    frame->stride = width; // Default stride
    
    return frame;
}

/**
 * @brief Free video frame structure
 * 
 * @param frame Frame to free
 */
void hal_video_frame_free(hal_video_frame_t *frame)
{
    if (frame == NULL) {
        return;
    }
    
    if (frame->data != NULL) {
        hal_mem_free_aligned(frame->data);
    }
    
    free(frame);
}

/**
 * @brief Allocate audio frame structure
 * 
 * @param size Audio size in bytes
 * @param format Audio format
 * @param sample_rate Sample rate in Hz
 * @param channels Number of channels
 * @return hal_audio_frame_t* Allocated frame, or NULL on failure
 */
hal_audio_frame_t *hal_audio_frame_alloc(uint32_t size, hal_audio_format_t format, 
                                         uint32_t sample_rate, uint32_t channels)
{
    hal_audio_frame_t *frame = (hal_audio_frame_t*)malloc(sizeof(hal_audio_frame_t));
    if (frame == NULL) {
        return NULL;
    }
    
    // Allocate aligned audio data
    frame->data = hal_mem_alloc_aligned(size, 64);
    if (frame->data == NULL) {
        free(frame);
        return NULL;
    }
    
    frame->size = size;
    frame->format = format;
    frame->sample_rate = sample_rate;
    frame->channels = channels;
    frame->timestamp = 0;
    
    return frame;
}

/**
 * @brief Free audio frame structure
 * 
 * @param frame Frame to free
 */
void hal_audio_frame_free(hal_audio_frame_t *frame)
{
    if (frame == NULL) {
        return;
    }
    
    if (frame->data != NULL) {
        hal_mem_free_aligned(frame->data);
    }
    
    free(frame);
}

/*-----------------------------------------------------------------------------
 *  Default Stub Implementations
 * 
 * These functions provide default implementations that can be overridden
 * by platform-specific code.
 *----------------------------------------------------------------------------*/

static bool g_hal_initialized = false;

hal_err_t hal_sys_init(const hal_sys_config_t *config)
{
    if (config == NULL) {
        return HAL_ERR_PARAM;
    }
    
    hal_debug_print("Initializing HAL with config:");
    hal_debug_print("  Video memory: %u MB", config->video_mem_size);
    hal_debug_print("  Audio memory: %u MB", config->audio_mem_size);
    hal_debug_print("  AI memory: %u MB", config->ai_mem_size);
    hal_debug_print("  Hardware codec: %s", config->enable_hardware_codec ? "enabled" : "disabled");
    hal_debug_print("  AI acceleration: %s", config->enable_ai_accel ? "enabled" : "disabled");
    hal_debug_print("  Sensor: %s", config->sensor_model ? config->sensor_model : "unknown");
    
    g_hal_initialized = true;
    return HAL_OK;
}

hal_err_t hal_sys_deinit(void)
{
    if (!g_hal_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Deinitializing HAL");
    g_hal_initialized = false;
    return HAL_OK;
}

hal_err_t hal_sys_get_platform(char *platform_name, uint32_t buf_size)
{
    if (platform_name == NULL || buf_size == 0) {
        return HAL_ERR_PARAM;
    }
    
    // Default platform name
    const char *default_name = "Generic";
    size_t name_len = strlen(default_name);
    
    if (name_len >= buf_size) {
        return HAL_ERR_PARAM;
    }
    
    strncpy(platform_name, default_name, buf_size - 1);
    platform_name[buf_size - 1] = '\0';
    
    return HAL_OK;
}

/*-----------------------------------------------------------------------------
 *  Video Stub Implementations
 *----------------------------------------------------------------------------*/

static bool g_vi_initialized = false;

hal_err_t hal_vi_init(const hal_vi_config_t *config)
{
    if (config == NULL) {
        return HAL_ERR_PARAM;
    }
    
    if (!g_hal_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Initializing VI: %ux%u %d fps, sensor=%s", 
                   config->width, config->height, config->fps,
                   config->sensor_name ? config->sensor_name : "unknown");
    
    g_vi_initialized = true;
    return HAL_OK;
}

hal_err_t hal_vi_start(uint32_t channel_id)
{
    if (!g_vi_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Starting VI channel %u", channel_id);
    return HAL_OK;
}

hal_err_t hal_vi_stop(uint32_t channel_id)
{
    if (!g_vi_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Stopping VI channel %u", channel_id);
    return HAL_OK;
}

hal_err_t hal_vi_get_frame(uint32_t channel_id, hal_video_frame_t *frame, uint32_t timeout_ms)
{
    if (!g_vi_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    if (frame == NULL) {
        return HAL_ERR_PARAM;
    }
    
    // This is a stub implementation - real implementations should fill the frame
    hal_debug_print("Getting frame from VI channel %u (timeout: %u ms)", channel_id, timeout_ms);
    
    return HAL_ERR_UNSUPPORTED;
}

/*-----------------------------------------------------------------------------
 *  Audio Stub Implementations
 *----------------------------------------------------------------------------*/

static bool g_audio_initialized = false;

hal_err_t hal_audio_init(const hal_audio_config_t *config)
{
    if (config == NULL) {
        return HAL_ERR_PARAM;
    }
    
    if (!g_hal_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Initializing audio: %u Hz, %u channels, %s format",
                   config->sample_rate, config->channels,
                   config->format == HAL_AUDIO_FMT_PCM ? "PCM" :
                   config->format == HAL_AUDIO_FMT_AAC ? "AAC" :
                   config->format == HAL_AUDIO_FMT_G711A ? "G.711A" : "G.711U");
    
    g_audio_initialized = true;
    return HAL_OK;
}

hal_err_t hal_audio_start(void)
{
    if (!g_audio_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Starting audio capture");
    return HAL_OK;
}

hal_err_t hal_audio_stop(void)
{
    if (!g_audio_initialized) {
        return HAL_ERR_NOT_INIT;
    }
    
    hal_debug_print("Stopping audio capture");
    return HAL_OK;
}

/*-----------------------------------------------------------------------------
 *  Peripheral Stub Implementations
 *----------------------------------------------------------------------------*/

hal_err_t hal_peripheral_set_ircut(hal_ircut_mode_t mode)
{
    const char *mode_str = "Unknown";
    switch (mode) {
        case HAL_IRCUT_DAY_MODE:   mode_str = "Day"; break;
        case HAL_IRCUT_NIGHT_MODE: mode_str = "Night"; break;
        case HAL_IRCUT_AUTO_MODE:  mode_str = "Auto"; break;
    }
    
    hal_debug_print("Setting IR-CUT mode to %s", mode_str);
    return HAL_OK;
}

hal_err_t hal_peripheral_set_led(uint32_t led_id, hal_led_mode_t mode)
{
    const char *mode_str = "Unknown";
    switch (mode) {
        case HAL_LED_OFF:         mode_str = "Off"; break;
        case HAL_LED_ON:          mode_str = "On"; break;
        case HAL_LED_BLINK_SLOW:  mode_str = "Slow Blink"; break;
        case HAL_LED_BLINK_FAST:  mode_str = "Fast Blink"; break;
    }
    
    hal_debug_print("Setting LED %u to %s", led_id, mode_str);
    return HAL_OK;
}

hal_err_t hal_peripheral_read_light_sensor(float *value)
{
    if (value == NULL) {
        return HAL_ERR_PARAM;
    }
    
    // Return a dummy value for stub implementation
    *value = 50.0f; // 50% light level
    return HAL_OK;
}