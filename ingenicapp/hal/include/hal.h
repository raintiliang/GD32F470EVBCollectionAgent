// HAL - Hardware Abstraction Layer
// Main header including all HAL modules

#ifndef _HAL_H_
#define _HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

// Platform selection
// Define one of these before including hal.h
// #define PLATFORM_CVITEK
// #define PLATFORM_INGENIC  
// #define PLATFORM_RK

#ifndef PLATFORM_TYPE
#define PLATFORM_TYPE PLATFORM_INGENIC
#endif

// Common types and definitions
#include "hal_common.h"

// System abstraction
#include "hal_system.h"

// Video abstraction
#include "hal_video.h"

// Audio abstraction
#include "hal_audio.h"

// AI abstraction
#include "hal_ai.h"

// OSD abstraction
#include "hal_osd.h"

// Network abstraction
#include "hal_network.h"

// Peripheral abstraction
#include "hal_peripheral.h"

// HAL initialization and cleanup
int hal_init_all(void);
int hal_deinit_all(void);

// HAL version information
const char* hal_get_version(void);
const char* hal_get_platform(void);
const char* hal_get_build_date(void);

// HAL configuration
typedef struct {
    hal_system_config_t system;
    hal_vi_config_t video_input;
    hal_venc_config_t video_encoder;
    hal_audio_config_t audio;
    hal_ai_model_config_t ai_model;
    hal_network_service_config_t network;
    void *platform_specific;  // Platform-specific configuration
} hal_global_config_t;

int hal_configure(const hal_global_config_t *config);
int hal_get_configuration(hal_global_config_t *config);

// HAL status and diagnostics
typedef struct {
    hal_system_status_t system;
    hal_video_stats_t video_stats;
    hal_audio_status_t audio_status;
    hal_ai_stats_t ai_stats;
    hal_osd_status_t osd_status;
    hal_network_status_t network_status;
    hal_peripheral_status_t peripheral_status;
    uint32_t total_memory_usage;
    uint32_t cpu_usage;
    uint32_t temperature;
} hal_global_status_t;

int hal_get_global_status(hal_global_status_t *status);
int hal_dump_status(void);

// HAL error handling
const char* hal_strerror(int error_code);
int hal_get_last_error(void);
void hal_clear_last_error(void);
void hal_set_error_handler(void (*handler)(int error_code, const char* error_msg));

// HAL logging control
void hal_set_log_level(int level);
void hal_set_log_file(const char* filename);
void hal_set_log_callback(void (*callback)(int level, const char* message));

// HAL factory reset
int hal_factory_reset(void);
int hal_save_configuration(const char* filename);
int hal_load_configuration(const char* filename);

// HAL platform detection
int hal_detect_platform(void);
int hal_get_platform_capabilities(void);

#ifdef __cplusplus
}
#endif

#endif // _HAL_H_