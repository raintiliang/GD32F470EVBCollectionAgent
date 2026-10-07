// HAL System Abstraction
// Provides unified system initialization and resource management

#ifndef _HAL_SYSTEM_H_
#define _HAL_SYSTEM_H_

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// System configuration
typedef struct {
    uint32_t vb_pool_count;           // Video buffer pool count
    uint32_t vb_block_size;           // Video buffer block size
    uint32_t vb_block_count;          // Video buffer block count per pool
    uint32_t ai_pool_size;            // AI buffer pool size
    uint32_t max_vi_channels;         // Max video input channels
    uint32_t max_venc_channels;       // Max video encoder channels
    uint32_t max_ai_channels;         // Max AI inference channels
    bool enable_hdr;                  // Enable HDR support
    bool enable_3d_nr;                // Enable 3D noise reduction
    bool enable_wdr;                  // Enable WDR
    char sensor_name[32];             // Primary sensor name
    void *priv;                       // Platform private configuration
} hal_system_config_t;

// Memory pool info
typedef struct {
    uint32_t id;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t free_blocks;
} hal_mempool_info_t;

// System initialization and control
int hal_system_init(const hal_system_config_t *config);
int hal_system_deinit(void);
int hal_system_reset(void);

// Memory buffer management
int hal_buffer_pool_create(uint32_t block_size, uint32_t block_count, uint32_t *pool_id);
int hal_buffer_pool_destroy(uint32_t pool_id);
int hal_buffer_alloc(uint32_t pool_id, hal_buffer_t *buffer);
int hal_buffer_free(const hal_buffer_t *buffer);
int hal_buffer_get_info(uint32_t pool_id, hal_mempool_info_t *info);

// Clock and timer
uint64_t hal_get_time_us(void);
uint64_t hal_get_time_ms(void);
void hal_msleep(uint32_t ms);
void hal_usleep(uint32_t us);

// Interrupt and event
typedef void (*hal_isr_handler_t)(int irq_num, void *priv);
int hal_register_isr(int irq_num, hal_isr_handler_t handler, void *priv);
int hal_unregister_isr(int irq_num);
int hal_enable_irq(int irq_num);
int hal_disable_irq(int irq_num);

// Power management
int hal_power_on_sensor(void);
int hal_power_off_sensor(void);
int hal_power_on_isp(void);
int hal_power_off_isp(void);
int hal_power_on_venc(void);
int hal_power_off_venc(void);
int hal_power_on_ai(void);
int hal_power_off_ai(void);

// Temperature monitoring
int hal_get_temperature_sensor(float *temp);
int hal_get_temperature_isp(float *temp);
int hal_get_temperature_venc(float *temp);
int hal_get_temperature_ai(float *temp);

// System status
typedef struct {
    uint32_t total_memory;
    uint32_t free_memory;
    uint32_t cpu_usage;           // 0-100%
    uint32_t gpu_usage;           // 0-100%
    uint32_t vpu_usage;           // 0-100%
    uint32_t temperature;         // Celsius
    uint32_t uptime;              // Seconds
} hal_system_status_t;

int hal_get_system_status(hal_system_status_t *status);

// Debug and logging
int hal_set_log_level(int level);
int hal_dump_memory_info(void);
int hal_dump_buffer_pools(void);

// Platform specific initialization (called internally)
int hal_platform_init(void);
int hal_platform_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // _HAL_SYSTEM_H_