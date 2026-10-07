/**
 * @file hal_osd.c
 * @brief HAL OSD (On-Screen Display) Implementation
 * 
 * This file provides a unified interface for OSD operations across
 * multiple platforms. It serves as a wrapper that delegates to
 * platform-specific implementations.
 * 
 * @version 1.0.0
 * @date 2026-03-24
 */

#include "hal_osd.h"
#include "hal_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Compatibility macros for error codes
#define HAL_ERR_PARAM      HAL_ERROR_PARAM
#define HAL_ERR_NO_MEM     HAL_ERROR_MEMORY
#define HAL_ERR_NOT_INIT   HAL_ERROR_NOT_INIT
#define HAL_ERR_BUSY       HAL_ERROR_BUSY
#define HAL_ERR_UNSUPPORTED HAL_ERROR_NOT_SUPPORT

/*-----------------------------------------------------------------------------
 *  Module-private definitions
 *----------------------------------------------------------------------------*/

// OSD region structure
typedef struct {
    uint32_t region_id;
    hal_osd_region_config_t config;
    bool visible;
    uint32_t graphic_count;
    uint32_t text_count;
    uint32_t time_count;
    void *platform_priv;
} osd_region_t;

// Module state
typedef struct {
    bool initialized;
    uint32_t next_region_id;
    osd_region_t *regions;
    uint32_t max_regions;
    uint32_t active_regions;
    pthread_mutex_t lock;
    hal_osd_font_t default_font;
} osd_module_t;

/*-----------------------------------------------------------------------------
 *  Static variables
 *----------------------------------------------------------------------------*/

static osd_module_t g_osd_module = {
    .initialized = false,
    .next_region_id = 1,
    .regions = NULL,
    .max_regions = 0,
    .active_regions = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .default_font = {
        .name = "default",
        .size = 16,
        .style = 0,
        .anti_alias = true,
        .outline_width = 0,
        .color = {255, 255, 255, 255},  // White
        .bg_color = {0, 0, 0, 0},       // Transparent
        .outline_color = {0, 0, 0, 255} // Black
    }
};

/*-----------------------------------------------------------------------------
 *  Private helper functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Find region by ID
 * 
 * @param region_id Region ID
 * @return Pointer to region or NULL if not found
 */
static osd_region_t *find_region(uint32_t region_id)
{
    if (!g_osd_module.initialized || !g_osd_module.regions) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < g_osd_module.max_regions; i++) {
        if (g_osd_module.regions[i].region_id == region_id) {
            return &g_osd_module.regions[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Allocate a new region
 * 
 * @return Pointer to allocated region or NULL if failed
 */
static osd_region_t *allocate_region(void)
{
    if (!g_osd_module.regions) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < g_osd_module.max_regions; i++) {
        if (g_osd_module.regions[i].region_id == 0) {
            // Found free slot
            return &g_osd_module.regions[i];
        }
    }
    
    return NULL;
}

/**
 * @brief Validate region configuration
 * 
 * @param config Region configuration
 * @return true if valid, false otherwise
 */
static bool validate_region_config(const hal_osd_region_config_t *config)
{
    if (!config) {
        return false;
    }
    
    // Validate rectangle
    if (config->rect.width == 0 || config->rect.height == 0) {
        HAL_LOG_ERROR("OSD: Invalid region dimensions");
        return false;
    }
    
    // Validate pixel format
    switch (config->format) {
        case HAL_PIXEL_FORMAT_ARGB_8888:
        case HAL_PIXEL_FORMAT_ABGR_8888:
        case HAL_PIXEL_FORMAT_RGB_888:
        case HAL_PIXEL_FORMAT_BGR_888:
        case HAL_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        case HAL_PIXEL_FORMAT_YUV_PLANAR_420:
            // Supported formats
            break;
        default:
            HAL_LOG_ERROR("OSD: Unsupported pixel format: %d", config->format);
            return false;
    }
    
    return true;
}

/**
 * @brief Initialize platform-specific OSD layer
 * 
 * @return int 0 on success, negative error code on failure
 */
static int platform_osd_init(void)
{
    // Default: platform layer not implemented
    // This should be overridden by platform-specific implementation
    HAL_LOG_WARN("OSD: Platform layer not implemented, using stub implementation");
    return 0;
}

/**
 * @brief Deinitialize platform-specific OSD layer
 * 
 * @return int 0 on success, negative error code on failure
 */
static int platform_osd_deinit(void)
{
    HAL_LOG_WARN("OSD: Platform layer not implemented, using stub implementation");
    return 0;
}

/**
 * @brief Platform-specific region creation
 * 
 * @param region Pointer to region
 * @return int 0 on success, negative error code on failure
 */
static int platform_osd_create_region(osd_region_t *region)
{
    HAL_LOG_WARN("OSD: Platform layer not implemented, region creation stub");
    return 0;
}

/**
 * @brief Platform-specific region destruction
 * 
 * @param region Pointer to region
 * @return int 0 on success, negative error code on failure
 */
static int platform_osd_destroy_region(osd_region_t *region)
{
    HAL_LOG_WARN("OSD: Platform layer not implemented, region destruction stub");
    return 0;
}

/**
 * @brief Platform-specific region visibility change
 * 
 * @param region Pointer to region
 * @param visible true to show, false to hide
 * @return int 0 on success, negative error code on failure
 */
static int platform_osd_set_region_visibility(osd_region_t *region, bool visible)
{
    HAL_LOG_WARN("OSD: Platform layer not implemented, visibility stub");
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Public API implementation
 *----------------------------------------------------------------------------*/

int hal_osd_init(void)
{
    int ret = 0;
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (g_osd_module.initialized) {
        HAL_LOG_WARN("OSD: Module already initialized");
        pthread_mutex_unlock(&g_osd_module.lock);
        return 0;
    }
    
    // Initialize platform layer first
    ret = platform_osd_init();
    if (ret < 0) {
        HAL_LOG_ERROR("OSD: Platform initialization failed: %d", ret);
        pthread_mutex_unlock(&g_osd_module.lock);
        return ret;
    }
    
    // Allocate memory for regions
    g_osd_module.max_regions = 8;  // Default maximum
    g_osd_module.regions = (osd_region_t *)calloc(g_osd_module.max_regions, sizeof(osd_region_t));
    if (!g_osd_module.regions) {
        HAL_LOG_ERROR("OSD: Failed to allocate memory for regions");
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NO_MEM;
    }
    
    g_osd_module.initialized = true;
    g_osd_module.next_region_id = 1;
    g_osd_module.active_regions = 0;
    
    HAL_LOG_INFO("OSD: Module initialized successfully");
    pthread_mutex_unlock(&g_osd_module.lock);
    
    return 0;
}

int hal_osd_deinit(void)
{
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return 0;
    }
    
    // Destroy all active regions
    for (uint32_t i = 0; i < g_osd_module.max_regions; i++) {
        if (g_osd_module.regions[i].region_id != 0) {
            platform_osd_destroy_region(&g_osd_module.regions[i]);
            memset(&g_osd_module.regions[i], 0, sizeof(osd_region_t));
        }
    }
    
    // Free region memory
    if (g_osd_module.regions) {
        free(g_osd_module.regions);
        g_osd_module.regions = NULL;
    }
    
    // Deinitialize platform layer
    platform_osd_deinit();
    
    g_osd_module.initialized = false;
    g_osd_module.max_regions = 0;
    g_osd_module.active_regions = 0;
    
    HAL_LOG_INFO("OSD: Module deinitialized");
    pthread_mutex_unlock(&g_osd_module.lock);
    
    return 0;
}

int hal_osd_create_region(const hal_osd_region_config_t *config, uint32_t *region_id)
{
    int ret = 0;
    osd_region_t *region = NULL;
    
    if (!config || !region_id) {
        return HAL_ERR_PARAM;
    }
    
    // Validate configuration
    if (!validate_region_config(config)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        HAL_LOG_ERROR("OSD: Module not initialized");
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Allocate a new region
    region = allocate_region();
    if (!region) {
        HAL_LOG_ERROR("OSD: No free region slots available");
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_BUSY;
    }
    
    // Initialize region
    region->region_id = g_osd_module.next_region_id++;
    memcpy(&region->config, config, sizeof(hal_osd_region_config_t));
    region->visible = false;
    region->graphic_count = 0;
    region->text_count = 0;
    region->time_count = 0;
    region->platform_priv = NULL;
    
    // Create platform-specific region
    ret = platform_osd_create_region(region);
    if (ret < 0) {
        HAL_LOG_ERROR("OSD: Platform region creation failed: %d", ret);
        memset(region, 0, sizeof(osd_region_t));
        pthread_mutex_unlock(&g_osd_module.lock);
        return ret;
    }
    
    *region_id = region->region_id;
    g_osd_module.active_regions++;
    
    HAL_LOG_DEBUG("OSD: Region created with ID %u, dimensions %ux%u", 
                  region->region_id, config->rect.width, config->rect.height);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_destroy_region(uint32_t region_id)
{
    osd_region_t *region = NULL;
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    // Destroy platform-specific region
    platform_osd_destroy_region(region);
    
    // Clear region data
    memset(region, 0, sizeof(osd_region_t));
    g_osd_module.active_regions--;
    
    HAL_LOG_DEBUG("OSD: Region %u destroyed", region_id);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_show_region(uint32_t region_id)
{
    osd_region_t *region = NULL;
    int ret = 0;
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    if (region->visible) {
        HAL_LOG_WARN("OSD: Region %u already visible", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return 0;
    }
    
    // Set platform-specific visibility
    ret = platform_osd_set_region_visibility(region, true);
    if (ret < 0) {
        HAL_LOG_ERROR("OSD: Failed to show region %u: %d", region_id, ret);
        pthread_mutex_unlock(&g_osd_module.lock);
        return ret;
    }
    
    region->visible = true;
    HAL_LOG_DEBUG("OSD: Region %u shown", region_id);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_hide_region(uint32_t region_id)
{
    osd_region_t *region = NULL;
    int ret = 0;
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    if (!region->visible) {
        HAL_LOG_WARN("OSD: Region %u already hidden", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return 0;
    }
    
    // Set platform-specific visibility
    ret = platform_osd_set_region_visibility(region, false);
    if (ret < 0) {
        HAL_LOG_ERROR("OSD: Failed to hide region %u: %d", region_id, ret);
        pthread_mutex_unlock(&g_osd_module.lock);
        return ret;
    }
    
    region->visible = false;
    HAL_LOG_DEBUG("OSD: Region %u hidden", region_id);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_set_region_position(uint32_t region_id, const hal_rect_t *rect)
{
    osd_region_t *region = NULL;
    
    if (!rect) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    // Update region position
    memcpy(&region->config.rect, rect, sizeof(hal_rect_t));
    
    // Platform-specific position update would go here
    HAL_LOG_WARN("OSD: Platform position update not implemented");
    
    HAL_LOG_DEBUG("OSD: Region %u position updated to (%d,%d)-(%dx%d)", 
                  region_id, rect->x, rect->y, rect->width, rect->height);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_get_region_position(uint32_t region_id, hal_rect_t *rect)
{
    osd_region_t *region = NULL;
    
    if (!rect) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    memcpy(rect, &region->config.rect, sizeof(hal_rect_t));
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_set_region_alpha(uint32_t region_id, uint8_t alpha)
{
    osd_region_t *region = NULL;
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    region->config.alpha = alpha;
    
    // Platform-specific alpha update would go here
    HAL_LOG_WARN("OSD: Platform alpha update not implemented");
    
    HAL_LOG_DEBUG("OSD: Region %u alpha set to %u", region_id, alpha);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_get_region_alpha(uint32_t region_id, uint8_t *alpha)
{
    osd_region_t *region = NULL;
    
    if (!alpha) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    *alpha = region->config.alpha;
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_set_region_zorder(uint32_t region_id, uint32_t z_order)
{
    osd_region_t *region = NULL;
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    region->config.z_order = z_order;
    
    // Platform-specific z-order update would go here
    HAL_LOG_WARN("OSD: Platform z-order update not implemented");
    
    HAL_LOG_DEBUG("OSD: Region %u z-order set to %u", region_id, z_order);
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_get_region_zorder(uint32_t region_id, uint32_t *z_order)
{
    osd_region_t *region = NULL;
    
    if (!z_order) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    region = find_region(region_id);
    if (!region) {
        HAL_LOG_ERROR("OSD: Region %u not found", region_id);
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_PARAM;
    }
    
    *z_order = region->config.z_order;
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

// Stub implementations for other functions (to be implemented)

int hal_osd_draw_graphic(uint32_t region_id, const hal_osd_graphic_t *graphic)
{
    HAL_LOG_WARN("OSD: draw_graphic not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_clear_graphic(uint32_t region_id, uint32_t graphic_id)
{
    HAL_LOG_WARN("OSD: clear_graphic not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_clear_all_graphics(uint32_t region_id)
{
    HAL_LOG_WARN("OSD: clear_all_graphics not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_update_graphic(uint32_t region_id, uint32_t graphic_id, const hal_osd_graphic_t *graphic)
{
    HAL_LOG_WARN("OSD: update_graphic not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_draw_text(uint32_t region_id, const hal_osd_text_config_t *text_cfg, uint32_t *text_id)
{
    HAL_LOG_WARN("OSD: draw_text not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_update_text(uint32_t region_id, uint32_t text_id, const char *new_text)
{
    HAL_LOG_WARN("OSD: update_text not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_clear_text(uint32_t region_id, uint32_t text_id)
{
    HAL_LOG_WARN("OSD: clear_text not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_clear_all_texts(uint32_t region_id)
{
    HAL_LOG_WARN("OSD: clear_all_texts not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_set_text_color(uint32_t region_id, uint32_t text_id, const hal_osd_color_t *color)
{
    HAL_LOG_WARN("OSD: set_text_color not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_set_text_position(uint32_t region_id, uint32_t text_id, const hal_rect_t *rect)
{
    HAL_LOG_WARN("OSD: set_text_position not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_show_time(uint32_t region_id, const hal_osd_time_config_t *time_cfg, uint32_t *time_id)
{
    HAL_LOG_WARN("OSD: show_time not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_hide_time(uint32_t region_id, uint32_t time_id)
{
    HAL_LOG_WARN("OSD: hide_time not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_update_time_format(uint32_t region_id, uint32_t time_id, const hal_osd_time_config_t *new_cfg)
{
    HAL_LOG_WARN("OSD: update_time_format not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_enable_time_autoupdate(uint32_t region_id, uint32_t time_id, bool enable)
{
    HAL_LOG_WARN("OSD: enable_time_autoupdate not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_draw_bitmap(uint32_t region_id, 
                       const hal_buffer_t *bitmap_data, 
                       uint32_t width, 
                       uint32_t height, 
                       hal_pixel_format_t format, 
                       const hal_rect_t *dst_rect, 
                       uint8_t alpha, 
                       uint32_t *bitmap_id)
{
    HAL_LOG_WARN("OSD: draw_bitmap not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_update_bitmap(uint32_t region_id, 
                         uint32_t bitmap_id, 
                         const hal_buffer_t *new_bitmap_data)
{
    HAL_LOG_WARN("OSD: update_bitmap not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_lock_canvas(uint32_t region_id, hal_buffer_t *canvas)
{
    HAL_LOG_WARN("OSD: lock_canvas not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_unlock_canvas(uint32_t region_id)
{
    HAL_LOG_WARN("OSD: unlock_canvas not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_flush_canvas(uint32_t region_id)
{
    HAL_LOG_WARN("OSD: flush_canvas not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_create_animation(uint32_t region_id, 
                            const hal_osd_animation_t *animation, 
                            uint32_t *anim_id)
{
    HAL_LOG_WARN("OSD: create_animation not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_start_animation(uint32_t region_id, uint32_t anim_id)
{
    HAL_LOG_WARN("OSD: start_animation not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_stop_animation(uint32_t region_id, uint32_t anim_id)
{
    HAL_LOG_WARN("OSD: stop_animation not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_pause_animation(uint32_t region_id, uint32_t anim_id)
{
    HAL_LOG_WARN("OSD: pause_animation not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_resume_animation(uint32_t region_id, uint32_t anim_id)
{
    HAL_LOG_WARN("OSD: resume_animation not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_set_animation_frame(uint32_t region_id, uint32_t anim_id, uint32_t frame)
{
    HAL_LOG_WARN("OSD: set_animation_frame not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_bind_video_channel(uint32_t region_id, uint32_t video_chn_id)
{
    HAL_LOG_WARN("OSD: bind_video_channel not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_unbind_video_channel(uint32_t region_id)
{
    HAL_LOG_WARN("OSD: unbind_video_channel not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_bind_venc_channel(uint32_t region_id, uint32_t venc_chn_id)
{
    HAL_LOG_WARN("OSD: bind_venc_channel not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_unbind_venc_channel(uint32_t region_id)
{
    HAL_LOG_WARN("OSD: unbind_venc_channel not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_set_blend_mode(uint32_t region_id, hal_osd_blend_mode_t mode)
{
    HAL_LOG_WARN("OSD: set_blend_mode not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_set_transparency(uint32_t region_id, uint8_t transparency)
{
    HAL_LOG_WARN("OSD: set_transparency not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_get_status(hal_osd_status_t *status)
{
    if (!status) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    
    if (!g_osd_module.initialized) {
        pthread_mutex_unlock(&g_osd_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    memset(status, 0, sizeof(hal_osd_status_t));
    status->region_count = g_osd_module.active_regions;
    status->active_regions = 0;
    
    // Count active regions (visible)
    for (uint32_t i = 0; i < g_osd_module.max_regions; i++) {
        if (g_osd_module.regions[i].region_id != 0 && g_osd_module.regions[i].visible) {
            status->active_regions++;
            status->total_graphics += g_osd_module.regions[i].graphic_count;
            status->total_texts += g_osd_module.regions[i].text_count;
        }
    }
    
    pthread_mutex_unlock(&g_osd_module.lock);
    return 0;
}

int hal_osd_load_font(const char *font_path, const char *font_name)
{
    HAL_LOG_WARN("OSD: load_font not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_unload_font(const char *font_name)
{
    HAL_LOG_WARN("OSD: unload_font not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_get_available_fonts(char **font_list, uint32_t *count)
{
    HAL_LOG_WARN("OSD: get_available_fonts not implemented");
    return HAL_ERR_UNSUPPORTED;
}

int hal_osd_set_default_font(const hal_osd_font_t *font)
{
    if (!font) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_osd_module.lock);
    memcpy(&g_osd_module.default_font, font, sizeof(hal_osd_font_t));
    pthread_mutex_unlock(&g_osd_module.lock);
    
    return 0;
}

int hal_osd_platform_init(void)
{
    return platform_osd_init();
}

int hal_osd_platform_deinit(void)
{
    return platform_osd_deinit();
}