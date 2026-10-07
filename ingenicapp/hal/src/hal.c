// HAL - Hardware Abstraction Layer
// Main implementation file

#include "hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Platform detection
#ifdef PLATFORM_CVITEK
#define HAL_PLATFORM_STR "CVITEK"
#elif defined(PLATFORM_INGENIC)
#define HAL_PLATFORM_STR "Ingenic T31"
#elif defined(PLATFORM_RK)
#define HAL_PLATFORM_STR "Rockchip"
#else
#define HAL_PLATFORM_STR "Unknown"
#endif

// Version information
#define HAL_VERSION_MAJOR 1
#define HAL_VERSION_MINOR 0
#define HAL_VERSION_PATCH 0
#define HAL_BUILD_DATE __DATE__ " " __TIME__

// Global error code
static int g_last_error = HAL_OK;
static void (*g_error_handler)(int, const char*) = NULL;

// Global configuration
static hal_global_config_t g_global_config = {0};
static bool g_initialized = false;

// Internal function declarations
static int hal_platform_init(void);
static int hal_platform_deinit(void);
static const char* hal_error_to_string(int error_code);

// HAL initialization
int hal_init_all(void)
{
    if (g_initialized) {
        HAL_LOG_WARN("HAL already initialized\n");
        return HAL_OK;
    }
    
    HAL_LOG_INFO("Initializing HAL (Platform: %s)\n", HAL_PLATFORM_STR);
    
    // Initialize platform-specific components
    int ret = hal_platform_init();
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("Platform initialization failed: %d\n", ret);
        g_last_error = ret;
        return ret;
    }
    
    // Initialize system module
    ret = hal_system_init(&g_global_config.system);
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("System initialization failed: %d\n", ret);
        g_last_error = ret;
        goto error;
    }
    
    // Initialize video module
    ret = hal_vi_init();
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("Video input initialization failed: %d\n", ret);
        g_last_error = ret;
        goto error;
    }
    
    ret = hal_vpss_init();
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("Video processing initialization failed: %d\n", ret);
        g_last_error = ret;
        goto error;
    }
    
    ret = hal_venc_init();
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("Video encoder initialization failed: %d\n", ret);
        g_last_error = ret;
        goto error;
    }
    
    // Initialize audio module
    ret = hal_audio_init();
    if (ret != HAL_OK) {
        HAL_LOG_WARN("Audio initialization failed: %d (audio may be disabled)\n", ret);
        // Continue without audio
    }
    
    // Initialize AI module
    ret = hal_ai_init();
    if (ret != HAL_OK) {
        HAL_LOG_WARN("AI initialization failed: %d (AI may be disabled)\n", ret);
        // Continue without AI
    }
    
    // Initialize OSD module
    ret = hal_osd_init();
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("OSD initialization failed: %d\n", ret);
        g_last_error = ret;
        goto error;
    }
    
    // Initialize network module
    ret = hal_network_init();
    if (ret != HAL_OK) {
        HAL_LOG_ERROR("Network initialization failed: %d\n", ret);
        g_last_error = ret;
        goto error;
    }
    
    // Initialize peripheral module
    ret = hal_peripheral_init();
    if (ret != HAL_OK) {
        HAL_LOG_WARN("Peripheral initialization failed: %d\n", ret);
        // Continue without peripherals
    }
    
    HAL_LOG_INFO("HAL initialization completed successfully\n");
    g_initialized = true;
    return HAL_OK;
    
error:
    // Cleanup partially initialized modules
    hal_deinit_all();
    return ret;
}

// HAL deinitialization
int hal_deinit_all(void)
{
    if (!g_initialized) {
        return HAL_OK;
    }
    
    HAL_LOG_INFO("Deinitializing HAL\n");
    
    // Deinitialize in reverse order
    hal_peripheral_deinit();
    hal_network_deinit();
    hal_osd_deinit();
    hal_ai_deinit();
    hal_audio_deinit();
    hal_venc_deinit();
    hal_vpss_deinit();
    hal_vi_deinit();
    hal_system_deinit();
    
    // Deinitialize platform
    hal_platform_deinit();
    
    g_initialized = false;
    HAL_LOG_INFO("HAL deinitialization completed\n");
    return HAL_OK;
}

// HAL configuration
int hal_configure(const hal_global_config_t *config)
{
    if (config == NULL) {
        g_last_error = HAL_ERROR_PARAM;
        return HAL_ERROR_PARAM;
    }
    
    memcpy(&g_global_config, config, sizeof(hal_global_config_t));
    HAL_LOG_INFO("HAL configuration updated\n");
    return HAL_OK;
}

int hal_get_configuration(hal_global_config_t *config)
{
    if (config == NULL) {
        g_last_error = HAL_ERROR_PARAM;
        return HAL_ERROR_PARAM;
    }
    
    memcpy(config, &g_global_config, sizeof(hal_global_config_t));
    return HAL_OK;
}

// HAL status
int hal_get_global_status(hal_global_status_t *status)
{
    if (status == NULL) {
        g_last_error = HAL_ERROR_PARAM;
        return HAL_ERROR_PARAM;
    }
    
    memset(status, 0, sizeof(hal_global_status_t));
    
    // Get system status
    hal_get_system_status(&status->system);
    
    // Get video status (from first channel)
    hal_video_get_stats(0, &status->video_stats);
    
    // Get audio status (from first device)
    hal_audio_get_status(0, &status->audio_status);
    
    // Get AI status (from first session)
    hal_ai_stats_t ai_stats = {0};
    hal_ai_get_stats(NULL, &ai_stats);
    status->ai_stats = ai_stats;
    
    // Get OSD status
    hal_osd_get_status(&status->osd_status);
    
    // Get network status (from first interface)
    hal_network_status_t net_status = {0};
    hal_netif_get_status("eth0", &net_status);
    status->network_status = net_status;
    
    // Get peripheral status
    hal_peripheral_get_status(&status->peripheral_status);
    
    // Calculate total memory usage (simplified)
    status->total_memory_usage = status->system.total_memory - status->system.free_memory;
    status->cpu_usage = status->system.cpu_usage;
    status->temperature = status->system.temperature;
    
    return HAL_OK;
}

int hal_dump_status(void)
{
    hal_global_status_t status;
    int ret = hal_get_global_status(&status);
    if (ret != HAL_OK) {
        return ret;
    }
    
    printf("=== HAL Status Dump ===\n");
    printf("Platform: %s\n", HAL_PLATFORM_STR);
    printf("Version: %s\n", hal_get_version());
    printf("Initialized: %s\n", g_initialized ? "Yes" : "No");
    printf("\n");
    
    printf("System:\n");
    printf("  Memory: %u/%u KB used\n", status.total_memory_usage / 1024, status.system.total_memory / 1024);
    printf("  CPU: %u%%\n", status.cpu_usage);
    printf("  GPU: %u%%\n", status.system.gpu_usage);
    printf("  VPU: %u%%\n", status.system.vpu_usage);
    printf("  Temperature: %u°C\n", status.temperature);
    printf("  Uptime: %u seconds\n", status.system.uptime);
    printf("\n");
    
    printf("Video:\n");
    printf("  Frames: %u\n", status.video_stats.frame_count);
    printf("  Frame rate: %u fps\n", status.video_stats.frame_rate);
    printf("  Bitrate: %u bps\n", status.video_stats.bitrate);
    printf("  Encode time: %u us\n", status.video_stats.encode_time);
    printf("  Dropped frames: %u\n", status.video_stats.drop_frame_count);
    printf("\n");
    
    printf("Audio:\n");
    printf("  Frames: %u\n", status.audio_status.frame_count);
    printf("  Sample rate: %u Hz\n", status.audio_status.sample_rate);
    printf("  Bitrate: %u bps\n", status.audio_status.bitrate);
    printf("  Volume: %u%%\n", status.audio_status.volume);
    printf("  Mute: %s\n", status.audio_status.mute ? "Yes" : "No");
    printf("  Active: %s\n", status.audio_status.active ? "Yes" : "No");
    printf("\n");
    
    printf("Network (eth0):\n");
    printf("  IP: %s\n", status.network_status.ip_addr);
    printf("  MAC: %s\n", status.network_status.mac_addr);
    printf("  Link: %s\n", status.network_status.link_up ? "Up" : "Down");
    printf("  Speed: %u Mbps\n", status.network_status.link_speed);
    printf("  RX: %u bytes, TX: %u bytes\n", status.network_status.rx_bytes, status.network_status.tx_bytes);
    printf("\n");
    
    return HAL_OK;
}

// HAL version information
const char* hal_get_version(void)
{
    static char version_str[32];
    snprintf(version_str, sizeof(version_str), "%d.%d.%d", 
             HAL_VERSION_MAJOR, HAL_VERSION_MINOR, HAL_VERSION_PATCH);
    return version_str;
}

const char* hal_get_platform(void)
{
    return HAL_PLATFORM_STR;
}

const char* hal_get_build_date(void)
{
    return HAL_BUILD_DATE;
}

// HAL error handling
const char* hal_strerror(int error_code)
{
    return hal_error_to_string(error_code);
}

int hal_get_last_error(void)
{
    return g_last_error;
}

void hal_clear_last_error(void)
{
    g_last_error = HAL_OK;
}

void hal_set_error_handler(void (*handler)(int error_code, const char* error_msg))
{
    g_error_handler = handler;
}

// HAL logging control
void hal_set_log_level(int level)
{
    // This would typically set a global log level
    // For now, just set the HAL_LOG_LEVEL macro
    HAL_LOG_INFO("Log level set to %d\n", level);
}

void hal_set_log_file(const char* filename)
{
    HAL_LOG_INFO("Log file set to %s\n", filename);
}

void hal_set_log_callback(void (*callback)(int level, const char* message))
{
    HAL_LOG_INFO("Log callback set\n");
}

// HAL factory reset
int hal_factory_reset(void)
{
    HAL_LOG_INFO("Performing factory reset\n");
    
    // Reset configuration to defaults
    memset(&g_global_config, 0, sizeof(hal_global_config_t));
    
    // Platform-specific factory reset
    // ...
    
    HAL_LOG_INFO("Factory reset completed\n");
    return HAL_OK;
}

int hal_save_configuration(const char* filename)
{
    if (filename == NULL) {
        g_last_error = HAL_ERROR_PARAM;
        return HAL_ERROR_PARAM;
    }
    
    HAL_LOG_INFO("Saving configuration to %s\n", filename);
    // Implementation would save to file
    return HAL_OK;
}

int hal_load_configuration(const char* filename)
{
    if (filename == NULL) {
        g_last_error = HAL_ERROR_PARAM;
        return HAL_ERROR_PARAM;
    }
    
    HAL_LOG_INFO("Loading configuration from %s\n", filename);
    // Implementation would load from file
    return HAL_OK;
}

// HAL platform detection
int hal_detect_platform(void)
{
    // This function would auto-detect the platform
    // For now, return the compile-time platform
    
#ifdef PLATFORM_CVITEK
    return PLATFORM_CVITEK;
#elif defined(PLATFORM_INGENIC)
    return PLATFORM_INGENIC;
#elif defined(PLATFORM_RK)
    return PLATFORM_RK;
#else
    return 0;  // Unknown
#endif
}

int hal_get_platform_capabilities(void)
{
    // Return a bitmap of platform capabilities
    // This is platform-specific and would be implemented in platform code
    return 0;
}

// Internal functions
static int hal_platform_init(void)
{
    // Platform-specific initialization
    // This would call platform-specific init functions
    
#ifdef PLATFORM_CVITEK
    // CVITEK platform initialization
    HAL_LOG_DEBUG("Initializing CVITEK platform\n");
#elif defined(PLATFORM_INGENIC)
    // Ingenic platform initialization
    HAL_LOG_DEBUG("Initializing Ingenic platform\n");
#elif defined(PLATFORM_RK)
    // Rockchip platform initialization
    HAL_LOG_DEBUG("Initializing Rockchip platform\n");
#else
    HAL_LOG_ERROR("Unknown platform\n");
    return HAL_ERROR_NOT_SUPPORT;
#endif
    
    return HAL_OK;
}

static int hal_platform_deinit(void)
{
    // Platform-specific deinitialization
    
#ifdef PLATFORM_CVITEK
    // CVITEK platform deinitialization
    HAL_LOG_DEBUG("Deinitializing CVITEK platform\n");
#elif defined(PLATFORM_INGENIC)
    // Ingenic platform deinitialization
    HAL_LOG_DEBUG("Deinitializing Ingenic platform\n");
#elif defined(PLATFORM_RK)
    // Rockchip platform deinitialization
    HAL_LOG_DEBUG("Deinitializing Rockchip platform\n");
#endif
    
    return HAL_OK;
}

static const char* hal_error_to_string(int error_code)
{
    switch (error_code) {
        case HAL_OK:
            return "Success";
        case HAL_ERROR:
            return "General error";
        case HAL_ERROR_PARAM:
            return "Invalid parameter";
        case HAL_ERROR_MEMORY:
            return "Memory allocation error";
        case HAL_ERROR_TIMEOUT:
            return "Operation timeout";
        case HAL_ERROR_NOT_SUPPORT:
            return "Operation not supported";
        case HAL_ERROR_NOT_INIT:
            return "Module not initialized";
        case HAL_ERROR_BUSY:
            return "Resource busy";
        default:
            return "Unknown error";
    }
}

// Platform-specific function implementations (stubs)
// These would be implemented in platform-specific files

int hal_peripheral_init(void) { return HAL_OK; }
int hal_peripheral_deinit(void) { return HAL_OK; }
int hal_peripheral_get_status(hal_peripheral_status_t *status) { 
    memset(status, 0, sizeof(hal_peripheral_status_t));
    return HAL_OK; 
}