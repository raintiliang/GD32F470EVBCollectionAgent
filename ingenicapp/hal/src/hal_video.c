/**
 * @file hal_video.c
 * @brief HAL Video Module Implementation
 * 
 * This file provides a unified interface for video operations across
 * multiple platforms, including:
 * - Video Input (VI): Camera sensor capture
 * - Video Processing (VPSS): Scaling, cropping, format conversion
 * - Video Encoding (VENC): H.264/H.265/JPEG encoding
 * 
 * @version 1.0.0
 * @date 2026-03-24
 */

#include "app_hal.h"
#include "hal_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/*-----------------------------------------------------------------------------
 *  Module-private definitions
 *----------------------------------------------------------------------------*/

// Default maximum channels
#define MAX_VI_CHANNELS  2
#define MAX_VPSS_CHANNELS 4
#define MAX_VENC_CHANNELS 2

// Video buffer pool configuration
#define VIDEO_BUFFER_POOL_SIZE 4
#define VIDEO_BUFFER_SIZE (1920 * 1080 * 3)  // Max size for 1080p RGB

// Video buffer structure
typedef struct {
    void *data;
    uint32_t size;
    uint32_t capacity;
    bool in_use;
    uint64_t timestamp;
    uint32_t sequence;
} video_buffer_t;

// Video input channel state
typedef struct {
    bool initialized;
    bool capturing;
    uint32_t channel_id;
    hal_vi_config_t config;
    video_buffer_t buffers[VIDEO_BUFFER_POOL_SIZE];
    uint32_t buffer_count;
    uint32_t next_buffer_idx;
    uint64_t frame_counter;
    pthread_mutex_t lock;
    void *platform_priv;  // Platform-specific private data
} vi_channel_t;

// Video processing channel state
typedef struct {
    bool initialized;
    hal_vpss_config_t config;
    pthread_mutex_t lock;
    void *platform_priv;
} vpss_channel_t;

// Video encoding channel state
typedef struct {
    bool initialized;
    bool encoding;
    hal_venc_config_t config;
    uint32_t frame_counter;
    pthread_mutex_t lock;
    void *platform_priv;
} venc_channel_t;

// Module state
typedef struct {
    bool initialized;
    
    // Video input
    vi_channel_t vi_channels[MAX_VI_CHANNELS];
    
    // Video processing
    vpss_channel_t vpss_channels[MAX_VPSS_CHANNELS];
    
    // Video encoding
    venc_channel_t venc_channels[MAX_VENC_CHANNELS];
    
    // Module-level lock
    pthread_mutex_t module_lock;
} video_module_t;

/*-----------------------------------------------------------------------------
 *  Static variables
 *----------------------------------------------------------------------------*/

static video_module_t g_video_module = {
    .initialized = false,
    .vi_channels = {{0}},
    .vpss_channels = {{0}},
    .venc_channels = {{0}},
    .module_lock = PTHREAD_MUTEX_INITIALIZER
};

// Error code compatibility macros
#define HAL_ERR_PARAM      HAL_ERROR_PARAM
#define HAL_ERR_NOT_INIT   HAL_ERROR_NOT_INIT
#define HAL_ERR_BUSY       HAL_ERROR_BUSY
#define HAL_ERR_NO_MEM     HAL_ERROR_MEMORY
#define HAL_ERR_TIMEOUT    HAL_ERROR_TIMEOUT
#define HAL_ERR_UNSUPPORTED HAL_ERROR_NOT_SUPPORT

/*-----------------------------------------------------------------------------
 *  Private helper functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Validate VI channel ID
 * 
 * @param channel_id Channel ID
 * @return true if valid, false otherwise
 */
static bool validate_vi_channel(uint32_t channel_id)
{
    if (channel_id >= MAX_VI_CHANNELS) {
        HAL_LOG_ERROR("Video: Invalid VI channel ID %u (max %u)", channel_id, MAX_VI_CHANNELS - 1);
        return false;
    }
    return true;
}

/**
 * @brief Validate VPSS channel ID
 * 
 * @param channel_id Channel ID
 * @return true if valid, false otherwise
 */
static bool validate_vpss_channel(uint32_t channel_id)
{
    if (channel_id >= MAX_VPSS_CHANNELS) {
        HAL_LOG_ERROR("Video: Invalid VPSS channel ID %u (max %u)", channel_id, MAX_VPSS_CHANNELS - 1);
        return false;
    }
    return true;
}

/**
 * @brief Validate VENC channel ID
 * 
 * @param channel_id Channel ID
 * @return true if valid, false otherwise
 */
static bool validate_venc_channel(uint32_t channel_id)
{
    if (channel_id >= MAX_VENC_CHANNELS) {
        HAL_LOG_ERROR("Video: Invalid VENC channel ID %u (max %u)", channel_id, MAX_VENC_CHANNELS - 1);
        return false;
    }
    return true;
}

/**
 * @brief Allocate video buffer
 * 
 * @param size Buffer size
 * @return video_buffer_t* Pointer to allocated buffer, NULL on failure
 */
static video_buffer_t *allocate_video_buffer(uint32_t size)
{
    video_buffer_t *buffer = (video_buffer_t *)malloc(sizeof(video_buffer_t));
    if (!buffer) {
        HAL_LOG_ERROR("Video: Failed to allocate buffer structure");
        return NULL;
    }
    
    buffer->data = malloc(size);
    if (!buffer->data) {
        HAL_LOG_ERROR("Video: Failed to allocate buffer data (%u bytes)", size);
        free(buffer);
        return NULL;
    }
    
    buffer->size = 0;
    buffer->capacity = size;
    buffer->in_use = false;
    buffer->timestamp = 0;
    buffer->sequence = 0;
    
    return buffer;
}

/**
 * @brief Free video buffer
 * 
 * @param buffer Buffer to free
 */
static void free_video_buffer(video_buffer_t *buffer)
{
    if (!buffer) {
        return;
    }
    
    if (buffer->data) {
        free(buffer->data);
    }
    free(buffer);
}

/**
 * @brief Initialize platform-specific video layer
 * 
 * @return int 0 on success, negative error code on failure
 */
static int platform_video_init(void)
{
    // Default: platform layer not implemented
    // This should be overridden by platform-specific implementation
    HAL_LOG_WARN("Video: Platform layer not implemented, using stub implementation");
    return 0;
}

/**
 * @brief Deinitialize platform-specific video layer
 * 
 * @return int 0 on success, negative error code on failure
 */
static int platform_video_deinit(void)
{
    HAL_LOG_WARN("Video: Platform layer not implemented, using stub implementation");
    return 0;
}

/**
 * @brief Platform-specific VI initialization
 * 
 * @param channel_id Channel ID
 * @param config VI configuration
 * @return int 0 on success, negative error code on failure
 */
static int platform_vi_init(uint32_t channel_id, const hal_vi_config_t *config)
{
    HAL_LOG_WARN("Video: Platform VI initialization not implemented for channel %u", channel_id);
    return 0;
}

/**
 * @brief Platform-specific VI start capture
 * 
 * @param channel_id Channel ID
 * @return int 0 on success, negative error code on failure
 */
static int platform_vi_start(uint32_t channel_id)
{
    HAL_LOG_WARN("Video: Platform VI start not implemented for channel %u", channel_id);
    return 0;
}

/**
 * @brief Platform-specific VI stop capture
 * 
 * @param channel_id Channel ID
 * @return int 0 on success, negative error code on failure
 */
static int platform_vi_stop(uint32_t channel_id)
{
    HAL_LOG_WARN("Video: Platform VI stop not implemented for channel %u", channel_id);
    return 0;
}

/**
 * @brief Platform-specific VI get frame
 * 
 * @param channel_id Channel ID
 * @param frame Output frame
 * @param timeout_ms Timeout in milliseconds
 * @return int 0 on success, negative error code on failure
 */
static int platform_vi_get_frame(uint32_t channel_id, hal_video_frame_t *frame, uint32_t timeout_ms)
{
    HAL_LOG_WARN("Video: Platform VI get frame not implemented for channel %u", channel_id);
    return 0;
}

/**
 * @brief Platform-specific VI release frame
 * 
 * @param channel_id Channel ID
 * @param frame Frame to release
 * @return int 0 on success, negative error code on failure
 */
static int platform_vi_release_frame(uint32_t channel_id, const hal_video_frame_t *frame)
{
    HAL_LOG_WARN("Video: Platform VI release frame not implemented for channel %u", channel_id);
    return 0;
}

/**
 * @brief Platform-specific VPSS initialization
 * 
 * @param config VPSS configuration
 * @return int 0 on success, negative error code on failure
 */
static int platform_vpss_init(const hal_vpss_config_t *config)
{
    HAL_LOG_WARN("Video: Platform VPSS initialization not implemented");
    return 0;
}

/**
 * @brief Platform-specific VPSS process frame
 * 
 * @param in_frame Input frame
 * @param out_frame Output frame
 * @return int 0 on success, negative error code on failure
 */
static int platform_vpss_process(const hal_video_frame_t *in_frame, hal_video_frame_t *out_frame)
{
    HAL_LOG_WARN("Video: Platform VPSS process not implemented");
    return 0;
}

/**
 * @brief Platform-specific VENC initialization
 * 
 * @param config VENC configuration
 * @return int 0 on success, negative error code on failure
 */
static int platform_venc_init(const hal_venc_config_t *config)
{
    HAL_LOG_WARN("Video: Platform VENC initialization not implemented");
    return 0;
}

/**
 * @brief Platform-specific VENC encode frame
 * 
 * @param in_frame Input frame
 * @param out_packet Output encoded packet
 * @return int 0 on success, negative error code on failure
 */
static int platform_venc_encode(const hal_video_frame_t *in_frame, hal_video_frame_t *out_packet)
{
    HAL_LOG_WARN("Video: Platform VENC encode not implemented");
    return 0;
}

/**
 * @brief Platform-specific VENC request IDR
 * 
 * @param channel_id Channel ID
 * @return int 0 on success, negative error code on failure
 */
static int platform_venc_request_idr(uint32_t channel_id)
{
    HAL_LOG_WARN("Video: Platform VENC request IDR not implemented for channel %u", channel_id);
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Public API implementation
 *----------------------------------------------------------------------------*/

int hal_video_init(void)
{
    int ret = 0;
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (g_video_module.initialized) {
        HAL_LOG_WARN("Video: Module already initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Initialize platform layer first
    ret = platform_video_init();
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform initialization failed: %d", ret);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    // Initialize VI channels
    for (int i = 0; i < MAX_VI_CHANNELS; i++) {
        vi_channel_t *channel = &g_video_module.vi_channels[i];
        channel->initialized = false;
        channel->capturing = false;
        channel->channel_id = i;
        memset(&channel->config, 0, sizeof(hal_vi_config_t));
        channel->buffer_count = 0;
        channel->next_buffer_idx = 0;
        channel->frame_counter = 0;
        channel->lock = PTHREAD_MUTEX_INITIALIZER;
        channel->platform_priv = NULL;
        
        // Initialize buffers
        for (int j = 0; j < VIDEO_BUFFER_POOL_SIZE; j++) {
            memset(&channel->buffers[j], 0, sizeof(video_buffer_t));
        }
    }
    
    // Initialize VPSS channels
    for (int i = 0; i < MAX_VPSS_CHANNELS; i++) {
        vpss_channel_t *channel = &g_video_module.vpss_channels[i];
        channel->initialized = false;
        memset(&channel->config, 0, sizeof(hal_vpss_config_t));
        channel->lock = PTHREAD_MUTEX_INITIALIZER;
        channel->platform_priv = NULL;
    }
    
    // Initialize VENC channels
    for (int i = 0; i < MAX_VENC_CHANNELS; i++) {
        venc_channel_t *channel = &g_video_module.venc_channels[i];
        channel->initialized = false;
        channel->encoding = false;
        memset(&channel->config, 0, sizeof(hal_venc_config_t));
        channel->frame_counter = 0;
        channel->lock = PTHREAD_MUTEX_INITIALIZER;
        channel->platform_priv = NULL;
    }
    
    g_video_module.initialized = true;
    
    HAL_LOG_INFO("Video: Module initialized successfully");
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_video_deinit(void)
{
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Stop all VI channels
    for (int i = 0; i < MAX_VI_CHANNELS; i++) {
        vi_channel_t *channel = &g_video_module.vi_channels[i];
        if (channel->initialized && channel->capturing) {
            platform_vi_stop(i);
            channel->capturing = false;
        }
        
        // Free buffers
        for (int j = 0; j < VIDEO_BUFFER_POOL_SIZE; j++) {
            video_buffer_t *buffer = &channel->buffers[j];
            if (buffer->data) {
                free(buffer->data);
                buffer->data = NULL;
            }
        }
        
        channel->initialized = false;
    }
    
    // Deinitialize VPSS channels
    for (int i = 0; i < MAX_VPSS_CHANNELS; i++) {
        vpss_channel_t *channel = &g_video_module.vpss_channels[i];
        channel->initialized = false;
    }
    
    // Deinitialize VENC channels
    for (int i = 0; i < MAX_VENC_CHANNELS; i++) {
        venc_channel_t *channel = &g_video_module.venc_channels[i];
        channel->initialized = false;
        channel->encoding = false;
    }
    
    // Deinitialize platform layer
    platform_video_deinit();
    
    g_video_module.initialized = false;
    
    HAL_LOG_INFO("Video: Module deinitialized");
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Video Input (VI) API Implementation
 *----------------------------------------------------------------------------*/

int hal_vi_init(const hal_vi_config_t *config)
{
    int ret = 0;
    
    if (!config) {
        return HAL_ERR_PARAM;
    }
    
    if (!validate_vi_channel(config->channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    vi_channel_t *channel = &g_video_module.vi_channels[config->channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (channel->initialized) {
        HAL_LOG_WARN("Video: VI channel %u already initialized", config->channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Validate configuration
    if (config->width == 0 || config->height == 0 || config->fps == 0) {
        HAL_LOG_ERROR("Video: Invalid VI configuration (width=%u, height=%u, fps=%u)",
                     config->width, config->height, config->fps);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_PARAM;
    }
    
    // Allocate buffers
    uint32_t buffer_size = config->width * config->height * 3;  // Conservative estimate
    if (buffer_size < 1024) buffer_size = 1024;  // Minimum size
    
    for (int i = 0; i < VIDEO_BUFFER_POOL_SIZE; i++) {
        video_buffer_t *buffer = allocate_video_buffer(buffer_size);
        if (!buffer) {
            HAL_LOG_ERROR("Video: Failed to allocate buffer %d for VI channel %u", i, config->channel_id);
            // Clean up already allocated buffers
            for (int j = 0; j < i; j++) {
                free_video_buffer(&channel->buffers[j]);
            }
            pthread_mutex_unlock(&channel->lock);
            pthread_mutex_unlock(&g_video_module.module_lock);
            return HAL_ERR_NO_MEM;
        }
        
        // Copy buffer to channel
        memcpy(&channel->buffers[i], buffer, sizeof(video_buffer_t));
        free(buffer);  // We've copied the data, free the temporary structure
        channel->buffer_count++;
    }
    
    // Store configuration
    memcpy(&channel->config, config, sizeof(hal_vi_config_t));
    
    // Platform-specific initialization
    ret = platform_vi_init(config->channel_id, config);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VI initialization failed: %d", ret);
        // Free buffers
        for (int i = 0; i < VIDEO_BUFFER_POOL_SIZE; i++) {
            if (channel->buffers[i].data) {
                free(channel->buffers[i].data);
                channel->buffers[i].data = NULL;
            }
        }
        channel->buffer_count = 0;
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    channel->initialized = true;
    channel->capturing = false;
    channel->frame_counter = 0;
    
    HAL_LOG_INFO("Video: VI channel %u initialized (%ux%u, %u fps, format=%d)",
                config->channel_id, config->width, config->height, config->fps, config->format);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_vi_start(uint32_t channel_id)
{
    int ret = 0;
    
    if (!validate_vi_channel(channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    vi_channel_t *channel = &g_video_module.vi_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VI channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    if (channel->capturing) {
        HAL_LOG_WARN("Video: VI channel %u already capturing", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Platform-specific start
    ret = platform_vi_start(channel_id);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VI start failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    channel->capturing = true;
    channel->frame_counter = 0;
    
    HAL_LOG_INFO("Video: VI channel %u started", channel_id);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_vi_stop(uint32_t channel_id)
{
    int ret = 0;
    
    if (!validate_vi_channel(channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    vi_channel_t *channel = &g_video_module.vi_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VI channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    if (!channel->capturing) {
        HAL_LOG_WARN("Video: VI channel %u not capturing", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Platform-specific stop
    ret = platform_vi_stop(channel_id);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VI stop failed: %d", ret);
        // Continue anyway
    }
    
    channel->capturing = false;
    
    HAL_LOG_INFO("Video: VI channel %u stopped", channel_id);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_vi_get_frame(uint32_t channel_id, hal_video_frame_t *frame, uint32_t timeout_ms)
{
    int ret = 0;
    
    if (!frame || !validate_vi_channel(channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    vi_channel_t *channel = &g_video_module.vi_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VI channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    if (!channel->capturing) {
        HAL_LOG_ERROR("Video: VI channel %u not capturing", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_BUSY;
    }
    
    // Platform-specific get frame
    ret = platform_vi_get_frame(channel_id, frame, timeout_ms);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VI get frame failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    channel->frame_counter++;
    
    HAL_LOG_DEBUG("Video: Got frame from VI channel %u (frame %lu)", channel_id, channel->frame_counter);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_vi_release_frame(uint32_t channel_id, const hal_video_frame_t *frame)
{
    int ret = 0;
    
    if (!frame || !validate_vi_channel(channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    vi_channel_t *channel = &g_video_module.vi_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VI channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Platform-specific release frame
    ret = platform_vi_release_frame(channel_id, frame);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VI release frame failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    HAL_LOG_DEBUG("Video: Released frame from VI channel %u", channel_id);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Video Processing (VPSS) API Implementation
 *----------------------------------------------------------------------------*/

int hal_vpss_init(const hal_vpss_config_t *config)
{
    int ret = 0;
    
    if (!config) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // For now, we use a single VPSS channel (0)
    uint32_t channel_id = 0;
    if (!validate_vpss_channel(channel_id)) {
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_PARAM;
    }
    
    vpss_channel_t *channel = &g_video_module.vpss_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (channel->initialized) {
        HAL_LOG_WARN("Video: VPSS channel %u already initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Validate configuration
    if (config->input_width == 0 || config->input_height == 0 ||
        config->output_width == 0 || config->output_height == 0) {
        HAL_LOG_ERROR("Video: Invalid VPSS configuration");
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_PARAM;
    }
    
    // Store configuration
    memcpy(&channel->config, config, sizeof(hal_vpss_config_t));
    
    // Platform-specific initialization
    ret = platform_vpss_init(config);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VPSS initialization failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    channel->initialized = true;
    
    HAL_LOG_INFO("Video: VPSS channel %u initialized (%ux%u -> %ux%u)",
                channel_id, config->input_width, config->input_height,
                config->output_width, config->output_height);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_vpss_process(const hal_video_frame_t *in_frame, hal_video_frame_t *out_frame)
{
    int ret = 0;
    
    if (!in_frame || !out_frame) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Use channel 0 for now
    uint32_t channel_id = 0;
    vpss_channel_t *channel = &g_video_module.vpss_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VPSS channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Platform-specific processing
    ret = platform_vpss_process(in_frame, out_frame);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VPSS process failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    HAL_LOG_DEBUG("Video: VPSS processed frame %ux%u -> %ux%u",
                 in_frame->width, in_frame->height,
                 out_frame->width, out_frame->height);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Video Encoding (VENC) API Implementation
 *----------------------------------------------------------------------------*/

int hal_venc_init(const hal_venc_config_t *config)
{
    int ret = 0;
    
    if (!config) {
        return HAL_ERR_PARAM;
    }
    
    // For now, we use channel 0
    uint32_t channel_id = 0;
    if (!validate_venc_channel(channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    venc_channel_t *channel = &g_video_module.venc_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (channel->initialized) {
        HAL_LOG_WARN("Video: VENC channel %u already initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return 0;
    }
    
    // Validate configuration
    if (config->width == 0 || config->height == 0 || config->fps == 0 || config->bitrate == 0) {
        HAL_LOG_ERROR("Video: Invalid VENC configuration");
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_PARAM;
    }
    
    // Validate codec
    if (config->codec != HAL_FMT_H264 && config->codec != HAL_FMT_H265 && config->codec != HAL_FMT_JPEG) {
        HAL_LOG_ERROR("Video: Unsupported codec: %d", config->codec);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_UNSUPPORTED;
    }
    
    // Store configuration
    memcpy(&channel->config, config, sizeof(hal_venc_config_t));
    
    // Platform-specific initialization
    ret = platform_venc_init(config);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VENC initialization failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    channel->initialized = true;
    channel->encoding = false;
    channel->frame_counter = 0;
    
    const char *codec_str = "unknown";
    switch (config->codec) {
        case HAL_FMT_H264: codec_str = "H.264"; break;
        case HAL_FMT_H265: codec_str = "H.265"; break;
        case HAL_FMT_JPEG: codec_str = "JPEG"; break;
        default: codec_str = "unknown"; break;
    }
    
    HAL_LOG_INFO("Video: VENC channel %u initialized (%s, %ux%u, %u fps, %u bps)",
                channel_id, codec_str, config->width, config->height, config->fps, config->bitrate);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_venc_encode(const hal_video_frame_t *in_frame, hal_video_frame_t *out_packet)
{
    int ret = 0;
    
    if (!in_frame || !out_packet) {
        return HAL_ERR_PARAM;
    }
    
    // Use channel 0 for now
    uint32_t channel_id = 0;
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    venc_channel_t *channel = &g_video_module.venc_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VENC channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Platform-specific encoding
    ret = platform_venc_encode(in_frame, out_packet);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VENC encode failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    if (!channel->encoding) {
        channel->encoding = true;
    }
    
    channel->frame_counter++;
    
    HAL_LOG_DEBUG("Video: VENC encoded frame %u", channel->frame_counter);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

int hal_venc_request_idr(uint32_t channel_id)
{
    int ret = 0;
    
    if (!validate_venc_channel(channel_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_video_module.module_lock);
    
    if (!g_video_module.initialized) {
        HAL_LOG_ERROR("Video: Module not initialized");
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    venc_channel_t *channel = &g_video_module.venc_channels[channel_id];
    
    pthread_mutex_lock(&channel->lock);
    
    if (!channel->initialized) {
        HAL_LOG_ERROR("Video: VENC channel %u not initialized", channel_id);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Platform-specific IDR request
    ret = platform_venc_request_idr(channel_id);
    if (ret < 0) {
        HAL_LOG_ERROR("Video: Platform VENC request IDR failed: %d", ret);
        pthread_mutex_unlock(&channel->lock);
        pthread_mutex_unlock(&g_video_module.module_lock);
        return ret;
    }
    
    HAL_LOG_DEBUG("Video: VENC channel %u IDR frame requested", channel_id);
    
    pthread_mutex_unlock(&channel->lock);
    pthread_mutex_unlock(&g_video_module.module_lock);
    
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Platform-specific video initialization (called by platform layer)
 *----------------------------------------------------------------------------*/

int hal_video_platform_init(void)
{
    return platform_video_init();
}

int hal_video_platform_deinit(void)
{
    return platform_video_deinit();
}