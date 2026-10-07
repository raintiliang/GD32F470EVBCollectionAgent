// HAL OSD (On-Screen Display) Abstraction
// Unified interface for video overlay and text/graphic rendering

#ifndef _HAL_OSD_H_
#define _HAL_OSD_H_

#include "hal_common.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// OSD color definition
typedef struct {
    uint8_t a;  // Alpha (0-255, 0=transparent, 255=opaque)
    uint8_t r;  // Red
    uint8_t g;  // Green
    uint8_t b;  // Blue
} hal_osd_color_t;

// OSD font definition
typedef struct {
    char name[32];          // Font name
    uint32_t size;          // Font size in pixels
    uint32_t style;         // Font style bitmap
    bool anti_alias;        // Anti-aliasing enabled
    uint32_t outline_width; // Outline width (0 = no outline)
    hal_osd_color_t color;  // Font color
    hal_osd_color_t bg_color; // Background color
    hal_osd_color_t outline_color; // Outline color
} hal_osd_font_t;

// OSD region configuration
typedef struct {
    uint32_t region_id;          // Region ID
    hal_rect_t rect;             // Region rectangle
    hal_pixel_format_t format;   // Pixel format
    uint32_t z_order;            // Z-order (higher = on top)
    bool global_alpha;           // Enable global alpha blending
    uint8_t alpha;               // Global alpha value (0-255)
    bool invert_color;           // Invert colors
    bool motion_smooth;          // Enable motion smoothing
    uint32_t frame_rate;         // Update frame rate
    void *priv;                  // Platform private data
} hal_osd_region_config_t;

// OSD graphic type
typedef enum {
    HAL_OSD_GRAPHIC_RECTANGLE = 0,
    HAL_OSD_GRAPHIC_CIRCLE,
    HAL_OSD_GRAPHIC_LINE,
    HAL_OSD_GRAPHIC_ELLIPSE,
    HAL_OSD_GRAPHIC_POLYGON,
    HAL_OSD_GRAPHIC_IMAGE,
} hal_osd_graphic_type_t;

// OSD graphic configuration
typedef struct {
    hal_osd_graphic_type_t type;
    union {
        struct {  // Rectangle
            hal_rect_t rect;
            uint32_t border_width;
            hal_osd_color_t fill_color;
            hal_osd_color_t border_color;
            bool filled;
        } rectangle;
        struct {  // Circle
            int32_t center_x;
            int32_t center_y;
            uint32_t radius;
            uint32_t border_width;
            hal_osd_color_t fill_color;
            hal_osd_color_t border_color;
            bool filled;
        } circle;
        struct {  // Line
            int32_t x1, y1;
            int32_t x2, y2;
            uint32_t width;
            hal_osd_color_t color;
            uint32_t dash_pattern;  // 0 = solid, bit pattern for dashed
        } line;
        struct {  // Image
            hal_buffer_t image_data;
            hal_rect_t dst_rect;
            uint8_t alpha;
        } image;
    };
} hal_osd_graphic_t;

// OSD text configuration
typedef struct {
    char text[256];              // Text content (UTF-8)
    hal_rect_t rect;             // Text bounding box
    hal_osd_font_t font;         // Font configuration
    uint32_t alignment;          // Text alignment
    uint32_t line_spacing;       // Line spacing in pixels
    bool word_wrap;              // Enable word wrapping
    bool scroll_enable;          // Enable scrolling
    uint32_t scroll_speed;       // Scroll speed (pixels per second)
    uint32_t scroll_direction;   // 0=left, 1=right, 2=up, 3=down
} hal_osd_text_config_t;

// OSD time configuration
typedef struct {
    hal_rect_t rect;             // Time display rectangle
    hal_osd_font_t font;         // Font for time display
    uint32_t format;             // Time format
    bool show_date;              // Show date
    bool show_time;              // Show time
    bool show_week;              // Show week day
    bool show_millisecond;       // Show milliseconds
    uint32_t timezone;           // Timezone offset in minutes
    char date_format[32];        // Date format string
    char time_format[32];        // Time format string
} hal_osd_time_config_t;

// Region management
int hal_osd_init(void);
int hal_osd_deinit(void);
int hal_osd_create_region(const hal_osd_region_config_t *config, uint32_t *region_id);
int hal_osd_destroy_region(uint32_t region_id);
int hal_osd_show_region(uint32_t region_id);
int hal_osd_hide_region(uint32_t region_id);
int hal_osd_set_region_position(uint32_t region_id, const hal_rect_t *rect);
int hal_osd_get_region_position(uint32_t region_id, hal_rect_t *rect);
int hal_osd_set_region_alpha(uint32_t region_id, uint8_t alpha);
int hal_osd_get_region_alpha(uint32_t region_id, uint8_t *alpha);
int hal_osd_set_region_zorder(uint32_t region_id, uint32_t z_order);
int hal_osd_get_region_zorder(uint32_t region_id, uint32_t *z_order);

// Graphic operations
int hal_osd_draw_graphic(uint32_t region_id, const hal_osd_graphic_t *graphic);
int hal_osd_clear_graphic(uint32_t region_id, uint32_t graphic_id);
int hal_osd_clear_all_graphics(uint32_t region_id);
int hal_osd_update_graphic(uint32_t region_id, uint32_t graphic_id, const hal_osd_graphic_t *graphic);

// Text operations
int hal_osd_draw_text(uint32_t region_id, const hal_osd_text_config_t *text_cfg, uint32_t *text_id);
int hal_osd_update_text(uint32_t region_id, uint32_t text_id, const char *new_text);
int hal_osd_clear_text(uint32_t region_id, uint32_t text_id);
int hal_osd_clear_all_texts(uint32_t region_id);
int hal_osd_set_text_color(uint32_t region_id, uint32_t text_id, const hal_osd_color_t *color);
int hal_osd_set_text_position(uint32_t region_id, uint32_t text_id, const hal_rect_t *rect);

// Time display operations
int hal_osd_show_time(uint32_t region_id, const hal_osd_time_config_t *time_cfg, uint32_t *time_id);
int hal_osd_hide_time(uint32_t region_id, uint32_t time_id);
int hal_osd_update_time_format(uint32_t region_id, uint32_t time_id, const hal_osd_time_config_t *new_cfg);
int hal_osd_enable_time_autoupdate(uint32_t region_id, uint32_t time_id, bool enable);

// Bitmap operations
int hal_osd_draw_bitmap(uint32_t region_id, 
                       const hal_buffer_t *bitmap_data, 
                       uint32_t width, 
                       uint32_t height, 
                       hal_pixel_format_t format, 
                       const hal_rect_t *dst_rect, 
                       uint8_t alpha, 
                       uint32_t *bitmap_id);

int hal_osd_update_bitmap(uint32_t region_id, 
                         uint32_t bitmap_id, 
                         const hal_buffer_t *new_bitmap_data);

// Canvas operations
int hal_osd_lock_canvas(uint32_t region_id, hal_buffer_t *canvas);
int hal_osd_unlock_canvas(uint32_t region_id);
int hal_osd_flush_canvas(uint32_t region_id);

// Animation support
typedef struct {
    uint32_t duration_ms;       // Animation duration in milliseconds
    uint32_t frame_count;       // Total frame count
    uint32_t current_frame;     // Current frame index
    bool loop;                  // Loop animation
    bool auto_start;            // Start automatically
    void *keyframes;            // Keyframe data
    size_t keyframe_size;       // Keyframe data size
} hal_osd_animation_t;

int hal_osd_create_animation(uint32_t region_id, 
                            const hal_osd_animation_t *animation, 
                            uint32_t *anim_id);

int hal_osd_start_animation(uint32_t region_id, uint32_t anim_id);
int hal_osd_stop_animation(uint32_t region_id, uint32_t anim_id);
int hal_osd_pause_animation(uint32_t region_id, uint32_t anim_id);
int hal_osd_resume_animation(uint32_t region_id, uint32_t anim_id);
int hal_osd_set_animation_frame(uint32_t region_id, uint32_t anim_id, uint32_t frame);

// OSD channel binding
int hal_osd_bind_video_channel(uint32_t region_id, uint32_t video_chn_id);
int hal_osd_unbind_video_channel(uint32_t region_id);
int hal_osd_bind_venc_channel(uint32_t region_id, uint32_t venc_chn_id);
int hal_osd_unbind_venc_channel(uint32_t region_id);

// Transparency and blending
typedef enum {
    HAL_OSD_BLEND_NONE = 0,      // No blending (replace)
    HAL_OSD_BLEND_ALPHA,         // Alpha blending
    HAL_OSD_BLEND_ADDITIVE,      // Additive blending
    HAL_OSD_BLEND_SUBTRACTIVE,   // Subtractive blending
    HAL_OSD_BLEND_MULTIPLY,      // Multiply blending
    HAL_OSD_BLEND_SCREEN,        // Screen blending
} hal_osd_blend_mode_t;

int hal_osd_set_blend_mode(uint32_t region_id, hal_osd_blend_mode_t mode);
int hal_osd_set_transparency(uint32_t region_id, uint8_t transparency);  // 0=opaque, 255=transparent

// OSD status
typedef struct {
    uint32_t region_count;
    uint32_t active_regions;
    uint32_t total_graphics;
    uint32_t total_texts;
    uint32_t memory_usage;
    uint32_t update_fps;
} hal_osd_status_t;

int hal_osd_get_status(hal_osd_status_t *status);

// Font management
int hal_osd_load_font(const char *font_path, const char *font_name);
int hal_osd_unload_font(const char *font_name);
int hal_osd_get_available_fonts(char **font_list, uint32_t *count);
int hal_osd_set_default_font(const hal_osd_font_t *font);

// Platform-specific OSD operations
int hal_osd_platform_init(void);
int hal_osd_platform_deinit(void);

// OSD region structure (platform implementation detail)
typedef struct {
    uint32_t region_id;
    hal_osd_region_config_t config;
    bool visible;
    uint32_t graphic_count;
    uint32_t text_count;
    uint32_t time_count;
    void *platform_priv;
} osd_region_t;

#ifdef __cplusplus
}
#endif

#endif // _HAL_OSD_H_