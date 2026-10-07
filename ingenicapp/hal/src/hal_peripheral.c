/**
 * @file hal_peripheral.c
 * @brief HAL Peripheral Control Implementation
 * 
 * This file provides a unified interface for peripheral operations across
 * multiple platforms, including IR-CUT control, LED control, and light sensor
 * reading.
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

// GPIO pin definitions (default for Ingenic T31)
#define GPIO_IRCUT_DAY_PIN   101   // Default IR-CUT day mode control pin
#define GPIO_IRCUT_NIGHT_PIN 102   // Default IR-CUT night mode control pin  
#define GPIO_LED0_PIN        103   // Default LED 0 control pin
#define GPIO_LED1_PIN        104   // Default LED 1 control pin

// Light sensor I2C address (default)
#define LIGHT_SENSOR_I2C_ADDR 0x23  // Typical BH1750 light sensor address

// Module state
typedef struct {
    bool initialized;
    pthread_mutex_t lock;
    
    // IR-CUT state
    hal_ircut_mode_t ircut_mode;
    bool ircut_day_pin_exported;
    bool ircut_night_pin_exported;
    
    // LED states
    bool led_pins_exported[2];  // Support up to 2 LEDs
    hal_led_mode_t led_modes[2];
    
    // Light sensor state
    bool light_sensor_available;
    int light_sensor_fd;        // I2C file descriptor
    
    // Platform-specific private data
    void *platform_priv;
} peripheral_module_t;

/*-----------------------------------------------------------------------------
 *  Static variables
 *----------------------------------------------------------------------------*/

static peripheral_module_t g_peripheral_module = {
    .initialized = false,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .ircut_mode = HAL_IRCUT_AUTO_MODE,
    .ircut_day_pin_exported = false,
    .ircut_night_pin_exported = false,
    .led_pins_exported = {false, false},
    .led_modes = {HAL_LED_OFF, HAL_LED_OFF},
    .light_sensor_available = false,
    .light_sensor_fd = -1,
    .platform_priv = NULL
};

// Error code compatibility macros
#define HAL_ERR_PARAM      HAL_ERROR_PARAM
#define HAL_ERR_NOT_INIT   HAL_ERROR_NOT_INIT
#define HAL_ERR_IO         (-8)  // I/O error
#define HAL_ERR_UNSUPPORTED HAL_ERROR_NOT_SUPPORT

/*-----------------------------------------------------------------------------
 *  Private helper functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Validate LED ID
 * 
 * @param led_id LED ID
 * @return true if valid, false otherwise
 */
static bool validate_led_id(uint32_t led_id)
{
    if (led_id >= 2) {  // Support up to 2 LEDs
        HAL_LOG_ERROR("Peripheral: Invalid LED ID %u (max %u)", led_id, 1);
        return false;
    }
    return true;
}

/**
 * @brief Initialize platform-specific peripheral layer
 * 
 * @return int 0 on success, negative error code on failure
 */
static int platform_peripheral_init(void)
{
    // Default: platform layer not implemented
    // This should be overridden by platform-specific implementation
    HAL_LOG_WARN("Peripheral: Platform layer not implemented, using stub implementation");
    return 0;
}

/**
 * @brief Deinitialize platform-specific peripheral layer
 * 
 * @return int 0 on success, negative error code on failure
 */
static int platform_peripheral_deinit(void)
{
    HAL_LOG_WARN("Peripheral: Platform layer not implemented, using stub implementation");
    return 0;
}

/**
 * @brief Platform-specific IR-CUT control
 * 
 * @param mode IR-CUT mode
 * @return int 0 on success, negative error code on failure
 */
static int platform_peripheral_set_ircut(hal_ircut_mode_t mode)
{
    HAL_LOG_WARN("Peripheral: Platform IR-CUT control not implemented");
    return 0;
}

/**
 * @brief Platform-specific LED control
 * 
 * @param led_id LED ID
 * @param mode LED mode
 * @return int 0 on success, negative error code on failure
 */
static int platform_peripheral_set_led(uint32_t led_id, hal_led_mode_t mode)
{
    HAL_LOG_WARN("Peripheral: Platform LED control not implemented for LED %u", led_id);
    return 0;
}

/**
 * @brief Platform-specific light sensor reading
 * 
 * @param value Output light level (0-100%)
 * @return int 0 on success, negative error code on failure
 */
static int platform_peripheral_read_light_sensor(float *value)
{
    HAL_LOG_WARN("Peripheral: Platform light sensor reading not implemented");
    if (value) {
        *value = 50.0f;  // Default dummy value
    }
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Public API implementation
 *----------------------------------------------------------------------------*/

int hal_peripheral_init(void)
{
    int ret = 0;
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (g_peripheral_module.initialized) {
        HAL_LOG_WARN("Peripheral: Module already initialized");
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return 0;
    }
    
    // Initialize platform layer first
    ret = platform_peripheral_init();
    if (ret < 0) {
        HAL_LOG_ERROR("Peripheral: Platform initialization failed: %d", ret);
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return ret;
    }
    
    g_peripheral_module.initialized = true;
    
    HAL_LOG_INFO("Peripheral: Module initialized successfully");
    pthread_mutex_unlock(&g_peripheral_module.lock);
    
    return 0;
}

int hal_peripheral_deinit(void)
{
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return 0;
    }
    
    // Turn off all LEDs before deinit
    for (uint32_t i = 0; i < 2; i++) {
        if (g_peripheral_module.led_pins_exported[i]) {
            platform_peripheral_set_led(i, HAL_LED_OFF);
        }
    }
    
    // Set IR-CUT to auto mode (default)
    if (g_peripheral_module.ircut_day_pin_exported || 
        g_peripheral_module.ircut_night_pin_exported) {
        platform_peripheral_set_ircut(HAL_IRCUT_AUTO_MODE);
    }
    
    // Deinitialize platform layer
    platform_peripheral_deinit();
    
    g_peripheral_module.initialized = false;
    g_peripheral_module.ircut_day_pin_exported = false;
    g_peripheral_module.ircut_night_pin_exported = false;
    g_peripheral_module.led_pins_exported[0] = false;
    g_peripheral_module.led_pins_exported[1] = false;
    g_peripheral_module.light_sensor_available = false;
    g_peripheral_module.light_sensor_fd = -1;
    
    HAL_LOG_INFO("Peripheral: Module deinitialized");
    pthread_mutex_unlock(&g_peripheral_module.lock);
    
    return 0;
}

int hal_peripheral_set_ircut(hal_ircut_mode_t mode)
{
    int ret = 0;
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        HAL_LOG_ERROR("Peripheral: Module not initialized");
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Validate mode
    switch (mode) {
        case HAL_IRCUT_DAY_MODE:
        case HAL_IRCUT_NIGHT_MODE:
        case HAL_IRCUT_AUTO_MODE:
            break;
        default:
            HAL_LOG_ERROR("Peripheral: Invalid IR-CUT mode: %d", mode);
            pthread_mutex_unlock(&g_peripheral_module.lock);
            return HAL_ERR_PARAM;
    }
    
    // Call platform-specific implementation
    ret = platform_peripheral_set_ircut(mode);
    if (ret < 0) {
        HAL_LOG_ERROR("Peripheral: Platform IR-CUT control failed: %d", ret);
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return ret;
    }
    
    // Update module state
    g_peripheral_module.ircut_mode = mode;
    
    const char *mode_str = "unknown";
    switch (mode) {
        case HAL_IRCUT_DAY_MODE:    mode_str = "day"; break;
        case HAL_IRCUT_NIGHT_MODE:  mode_str = "night"; break;
        case HAL_IRCUT_AUTO_MODE:   mode_str = "auto"; break;
    }
    
    HAL_LOG_DEBUG("Peripheral: IR-CUT set to %s mode", mode_str);
    pthread_mutex_unlock(&g_peripheral_module.lock);
    
    return 0;
}

int hal_peripheral_set_led(uint32_t led_id, hal_led_mode_t mode)
{
    int ret = 0;
    
    if (!validate_led_id(led_id)) {
        return HAL_ERR_PARAM;
    }
    
    // Validate mode
    switch (mode) {
        case HAL_LED_OFF:
        case HAL_LED_ON:
        case HAL_LED_BLINK_SLOW:
        case HAL_LED_BLINK_FAST:
            break;
        default:
            HAL_LOG_ERROR("Peripheral: Invalid LED mode: %d", mode);
            return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        HAL_LOG_ERROR("Peripheral: Module not initialized");
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Call platform-specific implementation
    ret = platform_peripheral_set_led(led_id, mode);
    if (ret < 0) {
        HAL_LOG_ERROR("Peripheral: Platform LED control failed for LED %u: %d", led_id, ret);
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return ret;
    }
    
    // Update module state
    g_peripheral_module.led_modes[led_id] = mode;
    g_peripheral_module.led_pins_exported[led_id] = true;
    
    const char *mode_str = "unknown";
    switch (mode) {
        case HAL_LED_OFF:         mode_str = "off"; break;
        case HAL_LED_ON:          mode_str = "on"; break;
        case HAL_LED_BLINK_SLOW:  mode_str = "slow blink"; break;
        case HAL_LED_BLINK_FAST:  mode_str = "fast blink"; break;
    }
    
    HAL_LOG_DEBUG("Peripheral: LED %u set to %s", led_id, mode_str);
    pthread_mutex_unlock(&g_peripheral_module.lock);
    
    return 0;
}

int hal_peripheral_read_light_sensor(float *value)
{
    int ret = 0;
    
    if (!value) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        HAL_LOG_ERROR("Peripheral: Module not initialized");
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    // Call platform-specific implementation
    ret = platform_peripheral_read_light_sensor(value);
    if (ret < 0) {
        HAL_LOG_ERROR("Peripheral: Platform light sensor reading failed: %d", ret);
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return ret;
    }
    
    // Validate value range (0-100%)
    if (*value < 0.0f) {
        *value = 0.0f;
    } else if (*value > 100.0f) {
        *value = 100.0f;
    }
    
    HAL_LOG_DEBUG("Peripheral: Light sensor reading: %.1f%%", *value);
    pthread_mutex_unlock(&g_peripheral_module.lock);
    
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Additional utility functions (not in HAL spec)
 *----------------------------------------------------------------------------*/

/**
 * @brief Get current IR-CUT mode
 * 
 * @param mode Output IR-CUT mode
 * @return int 0 on success, negative error code on failure
 */
int hal_peripheral_get_ircut_mode(hal_ircut_mode_t *mode)
{
    if (!mode) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    *mode = g_peripheral_module.ircut_mode;
    
    pthread_mutex_unlock(&g_peripheral_module.lock);
    return 0;
}

/**
 * @brief Get current LED mode
 * 
 * @param led_id LED ID
 * @param mode Output LED mode
 * @return int 0 on success, negative error code on failure
 */
int hal_peripheral_get_led_mode(uint32_t led_id, hal_led_mode_t *mode)
{
    if (!mode || !validate_led_id(led_id)) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    *mode = g_peripheral_module.led_modes[led_id];
    
    pthread_mutex_unlock(&g_peripheral_module.lock);
    return 0;
}

/**
 * @brief Check if light sensor is available
 * 
 * @param available Output availability flag
 * @return int 0 on success, negative error code on failure
 */
int hal_peripheral_check_light_sensor(bool *available)
{
    if (!available) {
        return HAL_ERR_PARAM;
    }
    
    pthread_mutex_lock(&g_peripheral_module.lock);
    
    if (!g_peripheral_module.initialized) {
        pthread_mutex_unlock(&g_peripheral_module.lock);
        return HAL_ERR_NOT_INIT;
    }
    
    *available = g_peripheral_module.light_sensor_available;
    
    pthread_mutex_unlock(&g_peripheral_module.lock);
    return 0;
}

/**
 * @brief Platform-specific peripheral initialization (called by platform layer)
 * 
 * @return int 0 on success, negative error code on failure
 */
int hal_peripheral_platform_init(void)
{
    return platform_peripheral_init();
}

/**
 * @brief Platform-specific peripheral deinitialization (called by platform layer)
 * 
 * @return int 0 on success, negative error code on failure
 */
int hal_peripheral_platform_deinit(void)
{
    return platform_peripheral_deinit();
}