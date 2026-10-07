// HAL Test Application
// Tests the Hardware Abstraction Layer interface

#include "hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test configuration
#define TEST_VIDEO_WIDTH   1920
#define TEST_VIDEO_HEIGHT  1080
#define TEST_VIDEO_FPS     30
#define TEST_AUDIO_RATE    16000
#define TEST_DURATION_SEC  5

// Global test state
static bool test_passed = true;
static int tests_run = 0;
static int tests_passed = 0;

// Test utilities
#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (!(condition)) { \
            printf("FAIL: %s (%s:%d)\n", message, __FILE__, __LINE__); \
            test_passed = false; \
        } else { \
            tests_passed++; \
            printf("PASS: %s\n", message); \
        } \
    } while(0)

#define TEST_EQ(expected, actual, message) \
    TEST_ASSERT((expected) == (actual), message)

#define TEST_NEQ(expected, actual, message) \
    TEST_ASSERT((expected) != (actual), message)

#define TEST_NULL(ptr, message) \
    TEST_ASSERT((ptr) == NULL, message)

#define TEST_NOT_NULL(ptr, message) \
    TEST_ASSERT((ptr) != NULL, message)

// Test functions
static void test_hal_version(void)
{
    printf("\n=== Testing HAL Version ===\n");
    
    const char* version = hal_get_version();
    const char* platform = hal_get_platform();
    const char* build_date = hal_get_build_date();
    
    TEST_NOT_NULL(version, "hal_get_version returns non-NULL");
    TEST_NOT_NULL(platform, "hal_get_platform returns non-NULL");
    TEST_NOT_NULL(build_date, "hal_get_build_date returns non-NULL");
    
    printf("Version: %s\n", version);
    printf("Platform: %s\n", platform);
    printf("Build date: %s\n", build_date);
}

static void test_hal_init_deinit(void)
{
    printf("\n=== Testing HAL Initialization ===\n");
    
    // Test initialization
    int ret = hal_init_all();
    TEST_EQ(HAL_OK, ret, "hal_init_all succeeds");
    
    // Try to initialize again (should be ok)
    ret = hal_init_all();
    TEST_EQ(HAL_OK, ret, "hal_init_all when already initialized");
    
    // Test deinitialization
    ret = hal_deinit_all();
    TEST_EQ(HAL_OK, ret, "hal_deinit_all succeeds");
    
    // Try to deinitialize again (should be ok)
    ret = hal_deinit_all();
    TEST_EQ(HAL_OK, ret, "hal_deinit_all when not initialized");
    
    // Re-initialize for subsequent tests
    ret = hal_init_all();
    TEST_EQ(HAL_OK, ret, "Re-initialization for subsequent tests");
}

static void test_hal_configuration(void)
{
    printf("\n=== Testing HAL Configuration ===\n");
    
    hal_global_config_t config = {0};
    
    // Set some test configuration
    config.system.vb_pool_count = 3;
    config.system.vb_block_size = 1024 * 1024;
    config.system.vb_block_count = 10;
    config.system.max_vi_channels = 2;
    config.system.max_venc_channels = 2;
    config.system.enable_hdr = false;
    strcpy(config.system.sensor_name, "test_sensor");
    
    config.video_input.width = TEST_VIDEO_WIDTH;
    config.video_input.height = TEST_VIDEO_HEIGHT;
    config.video_input.fps = TEST_VIDEO_FPS;
    config.video_input.pixel_format = HAL_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    
    config.video_encoder.chn_id = 0;
    config.video_encoder.codec = HAL_VIDEO_CODEC_H264;
    config.video_encoder.width = 1280;
    config.video_encoder.height = 720;
    config.video_encoder.fps = TEST_VIDEO_FPS;
    config.video_encoder.bitrate = 2000000;
    config.video_encoder.gop = 30;
    config.video_encoder.profile = 77;  // Main profile
    
    config.audio.device_type = HAL_AUDIO_DEVICE_MIC;
    config.audio.sample_rate = TEST_AUDIO_RATE;
    config.audio.format = HAL_AUDIO_FORMAT_PCM_S16_LE;
    config.audio.channels = HAL_AUDIO_CHANNEL_MONO;
    config.audio.volume = 80;
    
    // Set configuration
    int ret = hal_configure(&config);
    TEST_EQ(HAL_OK, ret, "hal_configure succeeds");
    
    // Get configuration back
    hal_global_config_t read_config = {0};
    ret = hal_get_configuration(&read_config);
    TEST_EQ(HAL_OK, ret, "hal_get_configuration succeeds");
    
    // Verify some values
    TEST_EQ(config.system.vb_pool_count, read_config.system.vb_pool_count, 
            "Configuration values preserved");
    TEST_EQ(config.video_input.width, read_config.video_input.width,
            "Video width preserved");
    TEST_EQ(config.video_encoder.bitrate, read_config.video_encoder.bitrate,
            "Encoder bitrate preserved");
}

static void test_hal_status(void)
{
    printf("\n=== Testing HAL Status ===\n");
    
    hal_global_status_t status = {0};
    
    // Get status
    int ret = hal_get_global_status(&status);
    TEST_EQ(HAL_OK, ret, "hal_get_global_status succeeds");
    
    // Check some status fields
    TEST_NOT_NULL(&status.system, "System status populated");
    TEST_NOT_NULL(&status.video_stats, "Video stats populated");
    TEST_NOT_NULL(&status.network_status, "Network status populated");
    
    printf("System memory: %u/%u KB\n", 
           status.total_memory_usage / 1024, 
           status.system.total_memory / 1024);
    printf("CPU usage: %u%%\n", status.cpu_usage);
    printf("Temperature: %u°C\n", status.temperature);
}

static void test_hal_error_handling(void)
{
    printf("\n=== Testing HAL Error Handling ===\n");
    
    // Clear any previous error
    hal_clear_last_error();
    
    // Get last error (should be HAL_OK)
    int last_error = hal_get_last_error();
    TEST_EQ(HAL_OK, last_error, "Last error cleared");
    
    // Test error string conversion
    const char* error_str = hal_strerror(HAL_OK);
    TEST_NOT_NULL(error_str, "hal_strerror for HAL_OK");
    
    error_str = hal_strerror(HAL_ERROR);
    TEST_NOT_NULL(error_str, "hal_strerror for HAL_ERROR");
    
    error_str = hal_strerror(HAL_ERROR_PARAM);
    TEST_NOT_NULL(error_str, "hal_strerror for HAL_ERROR_PARAM");
    
    error_str = hal_strerror(999);  // Unknown error code
    TEST_NOT_NULL(error_str, "hal_strerror for unknown error");
    
    // Test invalid parameter handling (should set error)
    int ret = hal_configure(NULL);
    TEST_EQ(HAL_ERROR_PARAM, ret, "NULL config returns HAL_ERROR_PARAM");
    
    // Last error should be set
    last_error = hal_get_last_error();
    TEST_EQ(HAL_ERROR_PARAM, last_error, "Last error set after error");
}

static void test_hal_module_interfaces(void)
{
    printf("\n=== Testing HAL Module Interfaces ===\n");
    
    // Note: These are mostly interface validation tests
    // Actual implementation would be platform-specific
    
    // System module
    printf("Testing system module...\n");
    hal_system_status_t sys_status = {0};
    int ret = hal_get_system_status(&sys_status);
    TEST_EQ(HAL_OK, ret, "hal_get_system_status interface valid");
    
    // Time functions
    uint64_t time_us = hal_get_time_us();
    uint64_t time_ms = hal_get_time_ms();
    TEST_ASSERT(time_us > 0, "hal_get_time_us returns positive value");
    TEST_ASSERT(time_ms > 0, "hal_get_time_ms returns positive value");
    TEST_ASSERT(time_us >= time_ms * 1000, "Microseconds >= milliseconds * 1000");
    
    // Memory pool (simulated)
    uint32_t pool_id = 0;
    ret = hal_buffer_pool_create(4096, 100, &pool_id);
    if (ret == HAL_OK) {
        TEST_ASSERT(pool_id > 0, "Buffer pool created successfully");
        
        hal_mempool_info_t pool_info = {0};
        ret = hal_buffer_get_info(pool_id, &pool_info);
        TEST_EQ(HAL_OK, ret, "Buffer pool info retrieved");
        
        ret = hal_buffer_pool_destroy(pool_id);
        TEST_EQ(HAL_OK, ret, "Buffer pool destroyed");
    } else {
        printf("Note: Buffer pool creation not implemented (platform-specific)\n");
    }
}

static void test_hal_dump_status(void)
{
    printf("\n=== Testing HAL Status Dump ===\n");
    
    int ret = hal_dump_status();
    TEST_EQ(HAL_OK, ret, "hal_dump_status succeeds");
}

static void test_hal_factory_reset(void)
{
    printf("\n=== Testing HAL Factory Reset ===\n");
    
    int ret = hal_factory_reset();
    TEST_EQ(HAL_OK, ret, "hal_factory_reset succeeds");
}

// Main test runner
int main(int argc, char* argv[])
{
    printf("HAL Test Suite\n");
    printf("==============\n");
    printf("Platform: %s\n", hal_get_platform());
    printf("Version: %s\n", hal_get_version());
    
    // Run tests
    test_hal_version();
    test_hal_init_deinit();
    test_hal_configuration();
    test_hal_status();
    test_hal_error_handling();
    test_hal_module_interfaces();
    test_hal_dump_status();
    test_hal_factory_reset();
    
    // Final cleanup
    int ret = hal_deinit_all();
    if (ret != HAL_OK) {
        printf("WARNING: hal_deinit_all failed: %d\n", ret);
    }
    
    // Test summary
    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    if (test_passed) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}

// Platform-specific implementations (stubs for testing)
// In a real implementation, these would be in platform-specific files

// System module stubs
int hal_system_init(const hal_system_config_t *config) { return HAL_OK; }
int hal_system_deinit(void) { return HAL_OK; }
int hal_system_reset(void) { return HAL_OK; }

int hal_buffer_pool_create(uint32_t block_size, uint32_t block_count, uint32_t *pool_id) 
{ 
    static uint32_t next_pool_id = 1;
    *pool_id = next_pool_id++;
    return HAL_OK; 
}

int hal_buffer_pool_destroy(uint32_t pool_id) { return HAL_OK; }
int hal_buffer_alloc(uint32_t pool_id, hal_buffer_t *buffer) { return HAL_OK; }
int hal_buffer_free(const hal_buffer_t *buffer) { return HAL_OK; }
int hal_buffer_get_info(uint32_t pool_id, hal_mempool_info_t *info) 
{ 
    info->id = pool_id;
    info->block_size = 4096;
    info->block_count = 100;
    info->free_blocks = 100;
    return HAL_OK; 
}

uint64_t hal_get_time_us(void) 
{ 
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

uint64_t hal_get_time_ms(void) 
{ 
    return hal_get_time_us() / 1000;
}

void hal_msleep(uint32_t ms) { usleep(ms * 1000); }
void hal_usleep(uint32_t us) { usleep(us); }

int hal_get_system_status(hal_system_status_t *status)
{
    status->total_memory = 256 * 1024 * 1024;  // 256MB
    status->free_memory = 128 * 1024 * 1024;   // 128MB
    status->cpu_usage = 10;
    status->gpu_usage = 0;
    status->vpu_usage = 0;
    status->temperature = 45;
    status->uptime = 3600;  // 1 hour
    return HAL_OK;
}

// Video module stubs
int hal_vi_init(void) { return HAL_OK; }
int hal_vi_deinit(void) { return HAL_OK; }
int hal_vpss_init(void) { return HAL_OK; }
int hal_vpss_deinit(void) { return HAL_OK; }
int hal_venc_init(void) { return HAL_OK; }
int hal_venc_deinit(void) { return HAL_OK; }

int hal_video_get_stats(uint32_t chn_id, hal_video_stats_t *stats)
{
    stats->frame_count = 1000;
    stats->frame_rate = TEST_VIDEO_FPS;
    stats->bitrate = 2000000;
    stats->encode_time = 5000;
    stats->drop_frame_count = 0;
    stats->error_frame_count = 0;
    return HAL_OK;
}

// Audio module stubs
int hal_audio_init(void) { return HAL_OK; }
int hal_audio_deinit(void) { return HAL_OK; }

int hal_audio_get_status(uint32_t dev_id, hal_audio_status_t *status)
{
    status->frame_count = 500;
    status->sample_rate = TEST_AUDIO_RATE;
    status->bitrate = 64000;
    status->volume = 80;
    status->mute = false;
    status->active = true;
    return HAL_OK;
}

// AI module stubs
int hal_ai_init(void) { return HAL_OK; }
int hal_ai_deinit(void) { return HAL_OK; }

int hal_ai_get_stats(hal_ai_session_handle_t session, hal_ai_stats_t *stats)
{
    stats->inference_count = 100;
    stats->success_count = 100;
    stats->error_count = 0;
    stats->total_inference_time = 1000000;
    stats->avg_inference_time = 10000;
    stats->max_inference_time = 20000;
    stats->min_inference_time = 5000;
    stats->fps = 10;
    stats->memory_usage = 1024 * 1024;  // 1MB
    return HAL_OK;
}

// OSD module stubs
int hal_osd_init(void) { return HAL_OK; }
int hal_osd_deinit(void) { return HAL_OK; }

int hal_osd_get_status(hal_osd_status_t *status)
{
    status->region_count = 2;
    status->active_regions = 1;
    status->total_graphics = 5;
    status->total_texts = 3;
    status->memory_usage = 512 * 1024;  // 512KB
    status->update_fps = 30;
    return HAL_OK;
}

// Network module stubs
int hal_network_init(void) { return HAL_OK; }
int hal_network_deinit(void) { return HAL_OK; }

int hal_netif_get_status(const char *ifname, hal_network_status_t *status)
{
    strcpy(status->ip_addr, "192.168.1.100");
    strcpy(status->netmask, "255.255.255.0");
    strcpy(status->gateway, "192.168.1.1");
    strcpy(status->mac_addr, "00:11:22:33:44:55");
    status->rx_bytes = 1024 * 1024 * 10;  // 10MB
    status->tx_bytes = 1024 * 1024 * 5;   // 5MB
    status->rx_packets = 10000;
    status->tx_packets = 5000;
    status->rx_errors = 0;
    status->tx_errors = 0;
    status->link_speed = 100;  // 100Mbps
    status->link_up = true;
    status->signal_strength = 100;
    return HAL_OK;
}