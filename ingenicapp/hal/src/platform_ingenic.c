/**
 * @file platform_ingenic.c
 * @brief Platform-specific implementation for Ingenic T31 platform
 * 
 * This file implements the HAL interface using Ingenic's IMP (Ingenic Media Platform) API.
 * 
 * @version 1.0.0
 * @date 2026-03-24
 */

#include "app_hal.h"
#include "hal_osd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/i2c-dev.h>

/*-----------------------------------------------------------------------------
 *  Ingenic IMP includes and definitions
 *----------------------------------------------------------------------------*/

#include <imp/imp_osd.h>
#include <imp/imp_common.h>
#include <imp/imp_log.h>

// IMP OSD constants
#define IMP_OSD_GROUP_ID 0  // Use group 0 for all HAL OSD regions
#define IMP_OSD_MAX_REGIONS 8

// IMP pixel format mapping
static IMPPixelFormat hal_to_imp_pixel_format(hal_pixel_format_t hal_format)
{
    switch (hal_format) {
        case HAL_PIXEL_FORMAT_ARGB_8888:
            return PIX_FMT_ARGB;
        case HAL_PIXEL_FORMAT_ABGR_8888:
            return PIX_FMT_ABGR;
        case HAL_PIXEL_FORMAT_RGB_888:
            return PIX_FMT_RGB24;
        case HAL_PIXEL_FORMAT_BGR_888:
            return PIX_FMT_BGR24;
        case HAL_PIXEL_FORMAT_YUV_SEMIPLANAR_420:
        case HAL_PIXEL_FORMAT_YUV_PLANAR_420:
            // IMP may not support YUV OSD directly, default to ARGB
            return PIX_FMT_ARGB;
        default:
            return PIX_FMT_ARGB;
    }
}

// Convert HAL rectangle to IMP rectangle
static void hal_to_imp_rect(const hal_rect_t *hal_rect, IMPRect *imp_rect)
{
    if (!hal_rect || !imp_rect) {
        return;
    }
    
    // HAL rect uses x,y,width,height
    // IMP rect uses p0 (top-left) and p1 (bottom-right)
    imp_rect->p0.x = hal_rect->x;
    imp_rect->p0.y = hal_rect->y;
    imp_rect->p1.x = hal_rect->x + hal_rect->width - 1;
    imp_rect->p1.y = hal_rect->y + hal_rect->height - 1;
}

// Convert IMP rectangle to HAL rectangle
static void __attribute__((unused)) imp_to_hal_rect(const IMPRect *imp_rect, hal_rect_t *hal_rect)
{
    if (!imp_rect || !hal_rect) {
        return;
    }
    
    hal_rect->x = imp_rect->p0.x;
    hal_rect->y = imp_rect->p0.y;
    hal_rect->width = imp_rect->p1.x - imp_rect->p0.x + 1;
    hal_rect->height = imp_rect->p1.y - imp_rect->p0.y + 1;
}

/*-----------------------------------------------------------------------------
 *  Platform-specific OSD implementation
 *----------------------------------------------------------------------------*/

// Platform-specific OSD region data
typedef struct {
    IMPRgnHandle imp_region;     // IMP region handle
    uint32_t region_id;          // HAL region ID
    IMPRect imp_rect;            // IMP rectangle
    IMPPixelFormat imp_format;   // IMP pixel format
    bool visible;                // Region visibility
    uint8_t alpha;               // Region alpha
    IMPOSDGrpRgnAttr grp_attr;   // Group region attributes
} ingenic_osd_region_t;

// Platform OSD module state
typedef struct {
    bool initialized;
    bool group_created;
    ingenic_osd_region_t *regions;
    uint32_t max_regions;
    uint32_t active_regions;
    pthread_mutex_t lock;
} ingenic_osd_module_t;

static ingenic_osd_module_t g_ingenic_osd = {
    .initialized = false,
    .group_created = false,
    .regions = NULL,
    .max_regions = 0,
    .active_regions = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER
};

/**
 * @brief Allocate a new platform region
 * 
 * @return Pointer to allocated region or NULL if failed
 */
static ingenic_osd_region_t *allocate_platform_region(void)
{
    if (!g_ingenic_osd.regions) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < g_ingenic_osd.max_regions; i++) {
        if (g_ingenic_osd.regions[i].region_id == 0) {
            // Found free slot
            return &g_ingenic_osd.regions[i];
        }
    }
    
    return NULL;
}

/*-----------------------------------------------------------------------------
 *  Platform OSD API implementation
 *----------------------------------------------------------------------------*/

int hal_osd_platform_init(void)
{
    int ret = 0;
    
    pthread_mutex_lock(&g_ingenic_osd.lock);
    
    if (g_ingenic_osd.initialized) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return 0;
    }
    
    // Create IMP OSD group
    ret = IMP_OSD_CreateGroup(IMP_OSD_GROUP_ID);
    if (ret < 0) {
        printf("[INGENIC OSD] Failed to create IMP OSD group: %d\n", ret);
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return ret;
    }
    
    g_ingenic_osd.group_created = true;
    
    // Allocate memory for regions
    g_ingenic_osd.max_regions = IMP_OSD_MAX_REGIONS;
    g_ingenic_osd.regions = (ingenic_osd_region_t *)calloc(g_ingenic_osd.max_regions, sizeof(ingenic_osd_region_t));
    if (!g_ingenic_osd.regions) {
        printf("[INGENIC OSD] Failed to allocate region memory\n");
        IMP_OSD_DestroyGroup(IMP_OSD_GROUP_ID);
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return -1;
    }
    
    g_ingenic_osd.initialized = true;
    g_ingenic_osd.active_regions = 0;
    
    printf("[INGENIC OSD] Platform OSD initialized (group %d)\n", IMP_OSD_GROUP_ID);
    pthread_mutex_unlock(&g_ingenic_osd.lock);
    
    return 0;
}

int hal_osd_platform_deinit(void)
{
    pthread_mutex_lock(&g_ingenic_osd.lock);
    
    if (!g_ingenic_osd.initialized) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return 0;
    }
    
    // Destroy all active regions
    for (uint32_t i = 0; i < g_ingenic_osd.max_regions; i++) {
        if (g_ingenic_osd.regions[i].region_id != 0) {
            // Unregister region from group
            IMP_OSD_UnRegisterRgn(g_ingenic_osd.regions[i].imp_region, IMP_OSD_GROUP_ID);
            
            // Destroy IMP region
            IMP_OSD_DestroyRgn(g_ingenic_osd.regions[i].imp_region);
        }
    }
    
    // Free region memory
    if (g_ingenic_osd.regions) {
        free(g_ingenic_osd.regions);
        g_ingenic_osd.regions = NULL;
    }
    
    // Destroy IMP OSD group
    if (g_ingenic_osd.group_created) {
        IMP_OSD_DestroyGroup(IMP_OSD_GROUP_ID);
        g_ingenic_osd.group_created = false;
    }
    
    g_ingenic_osd.initialized = false;
    g_ingenic_osd.max_regions = 0;
    g_ingenic_osd.active_regions = 0;
    
    printf("[INGENIC OSD] Platform OSD deinitialized\n");
    pthread_mutex_unlock(&g_ingenic_osd.lock);
    
    return 0;
}

int platform_osd_init(void)
{
    return hal_osd_platform_init();
}

int platform_osd_deinit(void)
{
    return hal_osd_platform_deinit();
}

int platform_osd_create_region(void *region_ptr)
{
    osd_region_t *hal_region = (osd_region_t *)region_ptr;
    ingenic_osd_region_t *platform_region = NULL;
    int ret = 0;
    
    if (!hal_region) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ingenic_osd.lock);
    
    if (!g_ingenic_osd.initialized) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return -1;
    }
    
    // Allocate platform region
    platform_region = allocate_platform_region();
    if (!platform_region) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return -1;
    }
    
    // Initialize platform region
    platform_region->region_id = hal_region->region_id;
    platform_region->visible = false;
    platform_region->alpha = hal_region->config.alpha;
    
    // Convert HAL parameters to IMP
    hal_to_imp_rect(&hal_region->config.rect, &platform_region->imp_rect);
    platform_region->imp_format = hal_to_imp_pixel_format(hal_region->config.format);
    
    // Create IMP OSD region attribute
    IMPOSDRgnAttr rgn_attr;
    memset(&rgn_attr, 0, sizeof(IMPOSDRgnAttr));
    
    // For now, create a cover region (transparent rectangle)
    // This gives us a canvas that doesn't affect the underlying video
    rgn_attr.type = OSD_REG_COVER;
    rgn_attr.rect = platform_region->imp_rect;
    rgn_attr.fmt = PIX_FMT_ARGB;  // Cover uses ARGB format
    
    // Set cover color to transparent (alpha = 0)
    coverData cover_data;
    cover_data.color = 0x00000000;  // ARGB: alpha=0, RGB=0
    rgn_attr.data.coverData = cover_data;
    
    // Create IMP region
    platform_region->imp_region = IMP_OSD_CreateRgn(&rgn_attr);
    if (platform_region->imp_region == INVHANDLE) {
        printf("[INGENIC OSD] Failed to create IMP region\n");
        memset(platform_region, 0, sizeof(ingenic_osd_region_t));
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return -1;
    }
    
    // Set up group region attributes
    memset(&platform_region->grp_attr, 0, sizeof(IMPOSDGrpRgnAttr));
    platform_region->grp_attr.show = 0;  // Start hidden
    platform_region->grp_attr.offPos.x = 0;
    platform_region->grp_attr.offPos.y = 0;
    platform_region->grp_attr.scalex = 1.0;
    platform_region->grp_attr.scaley = 1.0;
    platform_region->grp_attr.gAlphaEn = (platform_region->alpha != 255);
    platform_region->grp_attr.fgAlhpa = platform_region->alpha;
    platform_region->grp_attr.bgAlhpa = 0;
    platform_region->grp_attr.layer = hal_region->config.z_order;
    
    // Register region to group
    ret = IMP_OSD_RegisterRgn(platform_region->imp_region, IMP_OSD_GROUP_ID, &platform_region->grp_attr);
    if (ret < 0) {
        printf("[INGENIC OSD] Failed to register region to group: %d\n", ret);
        IMP_OSD_DestroyRgn(platform_region->imp_region);
        memset(platform_region, 0, sizeof(ingenic_osd_region_t));
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return ret;
    }
    
    // Store platform private data in HAL region
    hal_region->platform_priv = platform_region;
    g_ingenic_osd.active_regions++;
    
    printf("[INGENIC OSD] Created platform region for HAL region %u (%dx%d)\n", 
           hal_region->region_id, hal_region->config.rect.width, hal_region->config.rect.height);
    
    pthread_mutex_unlock(&g_ingenic_osd.lock);
    return 0;
}

int platform_osd_destroy_region(void *region_ptr)
{
    osd_region_t *hal_region = (osd_region_t *)region_ptr;
    ingenic_osd_region_t *platform_region = NULL;
    
    if (!hal_region) {
        return -1;
    }
    
    platform_region = (ingenic_osd_region_t *)hal_region->platform_priv;
    if (!platform_region) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ingenic_osd.lock);
    
    if (!g_ingenic_osd.initialized) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return -1;
    }
    
    // Unregister region from group
    IMP_OSD_UnRegisterRgn(platform_region->imp_region, IMP_OSD_GROUP_ID);
    
    // Destroy IMP region
    IMP_OSD_DestroyRgn(platform_region->imp_region);
    
    // Clear platform region data
    memset(platform_region, 0, sizeof(ingenic_osd_region_t));
    hal_region->platform_priv = NULL;
    g_ingenic_osd.active_regions--;
    
    printf("[INGENIC OSD] Destroyed platform region for HAL region %u\n", hal_region->region_id);
    
    pthread_mutex_unlock(&g_ingenic_osd.lock);
    return 0;
}

int platform_osd_set_region_visibility(void *region_ptr, bool visible)
{
    osd_region_t *hal_region = (osd_region_t *)region_ptr;
    ingenic_osd_region_t *platform_region = NULL;
    int ret = 0;
    
    if (!hal_region) {
        return -1;
    }
    
    platform_region = (ingenic_osd_region_t *)hal_region->platform_priv;
    if (!platform_region) {
        return -1;
    }
    
    pthread_mutex_lock(&g_ingenic_osd.lock);
    
    if (!g_ingenic_osd.initialized) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return -1;
    }
    
    if (platform_region->visible == visible) {
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return 0;
    }
    
    // Update group region attribute
    platform_region->grp_attr.show = visible ? 1 : 0;
    ret = IMP_OSD_SetGrpRgnAttr(platform_region->imp_region, IMP_OSD_GROUP_ID, &platform_region->grp_attr);
    if (ret < 0) {
        printf("[INGENIC OSD] Failed to set region visibility: %d\n", ret);
        pthread_mutex_unlock(&g_ingenic_osd.lock);
        return ret;
    }
    
    platform_region->visible = visible;
    printf("[INGENIC OSD] Region %u visibility set to %s\n", 
           hal_region->region_id, visible ? "visible" : "hidden");
    
    pthread_mutex_unlock(&g_ingenic_osd.lock);
    return 0;
}

/*-----------------------------------------------------------------------------
 *  HAL System Functions (stubs for now)
 *----------------------------------------------------------------------------*/

hal_err_t hal_sys_init(const hal_sys_config_t *config)
{
    printf("[INGENIC] System initialization (stub)\n");
    return HAL_OK;
}

hal_err_t hal_sys_deinit(void)
{
    printf("[INGENIC] System deinitialization (stub)\n");
    return HAL_OK;
}

hal_err_t hal_sys_get_platform(char *platform_name, uint32_t buf_size)
{
    if (!platform_name || buf_size == 0) {
        return HAL_ERR_PARAM;
    }
    
    const char *name = "Ingenic T31";
    size_t name_len = strlen(name);
    
    if (name_len >= buf_size) {
        return HAL_ERR_PARAM;
    }
    
    strncpy(platform_name, name, buf_size - 1);
    platform_name[buf_size - 1] = '\0';
    
    return HAL_OK;
}

// Add other HAL function stubs as needed...

/*-----------------------------------------------------------------------------
 *  Platform-specific Peripheral Implementation
 *----------------------------------------------------------------------------*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

// GPIO sysfs paths
#define GPIO_SYSFS_PATH "/sys/class/gpio"
#define GPIO_EXPORT_PATH GPIO_SYSFS_PATH "/export"
#define GPIO_UNEXPORT_PATH GPIO_SYSFS_PATH "/unexport"
#define GPIO_PREFIX GPIO_SYSFS_PATH "/gpio"

// GPIO pin definitions for Ingenic T31 (example pins)
#define IMP_GPIO_IRCUT_DAY    101  // GPIO pin for IR-CUT day mode
#define IMP_GPIO_IRCUT_NIGHT  102  // GPIO pin for IR-CUT night mode
#define IMP_GPIO_LED0         103  // GPIO pin for LED 0
#define IMP_GPIO_LED1         104  // GPIO pin for LED 1

// Light sensor I2C configuration
#define LIGHT_SENSOR_I2C_BUS  0    // I2C bus 0
#define LIGHT_SENSOR_I2C_ADDR 0x23 // BH1750 address

// Platform peripheral state
typedef struct {
    bool initialized;
    bool gpio_available;
    bool i2c_available;
    
    // GPIO pin states
    bool gpio_ircut_day_exported;
    bool gpio_ircut_night_exported;
    bool gpio_led0_exported;
    bool gpio_led1_exported;
    
    // I2C state
    int light_sensor_fd;
} ingenic_peripheral_t;

static ingenic_peripheral_t g_ingenic_peripheral = {
    .initialized = false,
    .gpio_available = false,
    .i2c_available = false,
    .gpio_ircut_day_exported = false,
    .gpio_ircut_night_exported = false,
    .gpio_led0_exported = false,
    .gpio_led1_exported = false,
    .light_sensor_fd = -1
};

/*-----------------------------------------------------------------------------
 *  GPIO Helper Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Export GPIO pin via sysfs
 * 
 * @param gpio_num GPIO pin number
 * @return int 0 on success, negative error code on failure
 */
static int gpio_export(int gpio_num)
{
    char buffer[32];
    int fd, len;
    
    // Check if GPIO is already exported
    char gpio_path[64];
    snprintf(gpio_path, sizeof(gpio_path), "%s%d", GPIO_PREFIX, gpio_num);
    if (access(gpio_path, F_OK) == 0) {
        return 0;  // Already exported
    }
    
    fd = open(GPIO_EXPORT_PATH, O_WRONLY);
    if (fd < 0) {
        printf("[INGENIC PERIPHERAL] Failed to open GPIO export: %s\n", strerror(errno));
        return -1;
    }
    
    len = snprintf(buffer, sizeof(buffer), "%d", gpio_num);
    if (write(fd, buffer, len) != len) {
        printf("[INGENIC PERIPHERAL] Failed to export GPIO %d: %s\n", gpio_num, strerror(errno));
        close(fd);
        return -1;
    }
    
    close(fd);
    usleep(100000);  // Wait for sysfs to create files
    
    printf("[INGENIC PERIPHERAL] Exported GPIO %d\n", gpio_num);
    return 0;
}

/**
 * @brief Unexport GPIO pin via sysfs
 * 
 * @param gpio_num GPIO pin number
 * @return int 0 on success, negative error code on failure
 */
static int gpio_unexport(int gpio_num)
{
    char buffer[32];
    int fd, len;
    
    fd = open(GPIO_UNEXPORT_PATH, O_WRONLY);
    if (fd < 0) {
        printf("[INGENIC PERIPHERAL] Failed to open GPIO unexport: %s\n", strerror(errno));
        return -1;
    }
    
    len = snprintf(buffer, sizeof(buffer), "%d", gpio_num);
    if (write(fd, buffer, len) != len) {
        printf("[INGENIC PERIPHERAL] Failed to unexport GPIO %d: %s\n", gpio_num, strerror(errno));
        close(fd);
        return -1;
    }
    
    close(fd);
    printf("[INGENIC PERIPHERAL] Unexported GPIO %d\n", gpio_num);
    return 0;
}

/**
 * @brief Set GPIO direction
 * 
 * @param gpio_num GPIO pin number
 * @param direction "in" or "out"
 * @return int 0 on success, negative error code on failure
 */
static int gpio_set_direction(int gpio_num, const char *direction)
{
    char path[64];
    int fd;
    
    snprintf(path, sizeof(path), "%s%d/direction", GPIO_PREFIX, gpio_num);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[INGENIC PERIPHERAL] Failed to open direction for GPIO %d: %s\n", gpio_num, strerror(errno));
        return -1;
    }
    
    if (write(fd, direction, strlen(direction)) != (ssize_t)strlen(direction)) {
        printf("[INGENIC PERIPHERAL] Failed to set direction for GPIO %d: %s\n", gpio_num, strerror(errno));
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/**
 * @brief Set GPIO value
 * 
 * @param gpio_num GPIO pin number
 * @param value 0 for low, 1 for high
 * @return int 0 on success, negative error code on failure
 */
static int gpio_set_value(int gpio_num, int value)
{
    char path[64];
    int fd;
    
    snprintf(path, sizeof(path), "%s%d/value", GPIO_PREFIX, gpio_num);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf("[INGENIC PERIPHERAL] Failed to open value for GPIO %d: %s\n", gpio_num, strerror(errno));
        return -1;
    }
    
    char val_char = value ? '1' : '0';
    if (write(fd, &val_char, 1) != 1) {
        printf("[INGENIC PERIPHERAL] Failed to set value for GPIO %d: %s\n", gpio_num, strerror(errno));
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/*-----------------------------------------------------------------------------
 *  I2C Helper Functions
 *----------------------------------------------------------------------------*/

/**
 * @brief Open I2C device
 * 
 * @param bus I2C bus number
 * @param addr I2C device address
 * @return int File descriptor on success, -1 on failure
 */
static int i2c_open(int bus, int addr)
{
    char filename[32];
    int fd;
    
    snprintf(filename, sizeof(filename), "/dev/i2c-%d", bus);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("[INGENIC PERIPHERAL] Failed to open I2C bus %d: %s\n", bus, strerror(errno));
        return -1;
    }
    
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        printf("[INGENIC PERIPHERAL] Failed to set I2C address 0x%02x: %s\n", addr, strerror(errno));
        close(fd);
        return -1;
    }
    
    return fd;
}

/**
 * @brief Read light sensor value (simplified for BH1750)
 * 
 * @param fd I2C file descriptor
 * @param value Output light level (lux)
 * @return int 0 on success, negative error code on failure
 */
static int read_bh1750_light_sensor(int fd, float *value)
{
    // Simplified implementation for BH1750
    // Real implementation would send commands and read measurements
    
    if (!value) {
        return -1;
    }
    
    // For now, return a dummy value
    // In real implementation:
    // 1. Send power on command (0x01)
    // 2. Send continuous high res mode (0x10)
    // 3. Wait 180ms
    // 4. Read 2 bytes
    // 5. Convert to lux
    
    *value = 300.0f;  // Dummy value: 300 lux
    
    return 0;
}

/*-----------------------------------------------------------------------------
 *  Platform Peripheral API Implementation
 *----------------------------------------------------------------------------*/

int platform_peripheral_init(void)
{
    printf("[INGENIC PERIPHERAL] Platform peripheral initialization\n");
    
    // Check if GPIO sysfs is available
    if (access(GPIO_SYSFS_PATH, F_OK) == 0) {
        g_ingenic_peripheral.gpio_available = true;
        printf("[INGENIC PERIPHERAL] GPIO sysfs available\n");
    } else {
        printf("[INGENIC PERIPHERAL] GPIO sysfs not available: %s\n", strerror(errno));
        g_ingenic_peripheral.gpio_available = false;
    }
    
    // Check if I2C device is available
    char i2c_dev[32];
    snprintf(i2c_dev, sizeof(i2c_dev), "/dev/i2c-%d", LIGHT_SENSOR_I2C_BUS);
    if (access(i2c_dev, F_OK) == 0) {
        g_ingenic_peripheral.i2c_available = true;
        printf("[INGENIC PERIPHERAL] I2C bus %d available\n", LIGHT_SENSOR_I2C_BUS);
    } else {
        printf("[INGENIC PERIPHERAL] I2C bus %d not available: %s\n", LIGHT_SENSOR_I2C_BUS, strerror(errno));
        g_ingenic_peripheral.i2c_available = false;
    }
    
    g_ingenic_peripheral.initialized = true;
    return 0;
}

int platform_peripheral_deinit(void)
{
    printf("[INGENIC PERIPHERAL] Platform peripheral deinitialization\n");
    
    if (!g_ingenic_peripheral.initialized) {
        return 0;
    }
    
    // Unexport all GPIO pins
    if (g_ingenic_peripheral.gpio_ircut_day_exported) {
        gpio_unexport(IMP_GPIO_IRCUT_DAY);
    }
    if (g_ingenic_peripheral.gpio_ircut_night_exported) {
        gpio_unexport(IMP_GPIO_IRCUT_NIGHT);
    }
    if (g_ingenic_peripheral.gpio_led0_exported) {
        gpio_unexport(IMP_GPIO_LED0);
    }
    if (g_ingenic_peripheral.gpio_led1_exported) {
        gpio_unexport(IMP_GPIO_LED1);
    }
    
    // Close I2C device
    if (g_ingenic_peripheral.light_sensor_fd >= 0) {
        close(g_ingenic_peripheral.light_sensor_fd);
        g_ingenic_peripheral.light_sensor_fd = -1;
    }
    
    g_ingenic_peripheral.initialized = false;
    g_ingenic_peripheral.gpio_available = false;
    g_ingenic_peripheral.i2c_available = false;
    
    return 0;
}

int platform_peripheral_set_ircut(hal_ircut_mode_t mode)
{
    int ret = 0;
    
    if (!g_ingenic_peripheral.initialized || !g_ingenic_peripheral.gpio_available) {
        printf("[INGENIC PERIPHERAL] Peripheral not initialized or GPIO not available\n");
        return -1;
    }
    
    // Export GPIO pins if not already exported
    if (!g_ingenic_peripheral.gpio_ircut_day_exported) {
        ret = gpio_export(IMP_GPIO_IRCUT_DAY);
        if (ret == 0) {
            g_ingenic_peripheral.gpio_ircut_day_exported = true;
            gpio_set_direction(IMP_GPIO_IRCUT_DAY, "out");
        }
    }
    
    if (!g_ingenic_peripheral.gpio_ircut_night_exported) {
        ret = gpio_export(IMP_GPIO_IRCUT_NIGHT);
        if (ret == 0) {
            g_ingenic_peripheral.gpio_ircut_night_exported = true;
            gpio_set_direction(IMP_GPIO_IRCUT_NIGHT, "out");
        }
    }
    
    // Set IR-CUT mode
    switch (mode) {
        case HAL_IRCUT_DAY_MODE:
            // Day mode: enable day pin, disable night pin
            gpio_set_value(IMP_GPIO_IRCUT_DAY, 1);
            gpio_set_value(IMP_GPIO_IRCUT_NIGHT, 0);
            printf("[INGENIC PERIPHERAL] IR-CUT set to DAY mode\n");
            break;
            
        case HAL_IRCUT_NIGHT_MODE:
            // Night mode: disable day pin, enable night pin
            gpio_set_value(IMP_GPIO_IRCUT_DAY, 0);
            gpio_set_value(IMP_GPIO_IRCUT_NIGHT, 1);
            printf("[INGENIC PERIPHERAL] IR-CUT set to NIGHT mode\n");
            break;
            
        case HAL_IRCUT_AUTO_MODE:
            // Auto mode: disable both pins (let sensor decide)
            gpio_set_value(IMP_GPIO_IRCUT_DAY, 0);
            gpio_set_value(IMP_GPIO_IRCUT_NIGHT, 0);
            printf("[INGENIC PERIPHERAL] IR-CUT set to AUTO mode\n");
            break;
            
        default:
            printf("[INGENIC PERIPHERAL] Invalid IR-CUT mode: %d\n", mode);
            return -1;
    }
    
    return 0;
}

int platform_peripheral_set_led(uint32_t led_id, hal_led_mode_t mode)
{
    int gpio_pin;
    static bool led_blinking[2] = {false, false};
    
    if (!g_ingenic_peripheral.initialized || !g_ingenic_peripheral.gpio_available) {
        printf("[INGENIC PERIPHERAL] Peripheral not initialized or GPIO not available\n");
        return -1;
    }
    
    // Select GPIO pin based on LED ID
    switch (led_id) {
        case 0:
            gpio_pin = IMP_GPIO_LED0;
            if (!g_ingenic_peripheral.gpio_led0_exported) {
                if (gpio_export(gpio_pin) == 0) {
                    g_ingenic_peripheral.gpio_led0_exported = true;
                    gpio_set_direction(gpio_pin, "out");
                }
            }
            break;
            
        case 1:
            gpio_pin = IMP_GPIO_LED1;
            if (!g_ingenic_peripheral.gpio_led1_exported) {
                if (gpio_export(gpio_pin) == 0) {
                    g_ingenic_peripheral.gpio_led1_exported = true;
                    gpio_set_direction(gpio_pin, "out");
                }
            }
            break;
            
        default:
            printf("[INGENIC PERIPHERAL] Invalid LED ID: %u\n", led_id);
            return -1;
    }
    
    // Stop any existing blinking
    led_blinking[led_id] = false;
    
    // Set LED mode
    switch (mode) {
        case HAL_LED_OFF:
            gpio_set_value(gpio_pin, 0);
            printf("[INGENIC PERIPHERAL] LED %u set to OFF\n", led_id);
            break;
            
        case HAL_LED_ON:
            gpio_set_value(gpio_pin, 1);
            printf("[INGENIC PERIPHERAL] LED %u set to ON\n", led_id);
            break;
            
        case HAL_LED_BLINK_SLOW:
            led_blinking[led_id] = true;
            printf("[INGENIC PERIPHERAL] LED %u set to SLOW BLINK (simulated)\n", led_id);
            // In real implementation, would start a blinking thread
            // For now, just turn on
            gpio_set_value(gpio_pin, 1);
            break;
            
        case HAL_LED_BLINK_FAST:
            led_blinking[led_id] = true;
            printf("[INGENIC PERIPHERAL] LED %u set to FAST BLINK (simulated)\n", led_id);
            // In real implementation, would start a blinking thread
            // For now, just turn on
            gpio_set_value(gpio_pin, 1);
            break;
            
        default:
            printf("[INGENIC PERIPHERAL] Invalid LED mode: %d\n", mode);
            return -1;
    }
    
    return 0;
}

int platform_peripheral_read_light_sensor(float *value)
{
    if (!value) {
        return -1;
    }
    
    if (!g_ingenic_peripheral.initialized) {
        printf("[INGENIC PERIPHERAL] Peripheral not initialized\n");
        return -1;
    }
    
    if (!g_ingenic_peripheral.i2c_available) {
        printf("[INGENIC PERIPHERAL] I2C not available\n");
        return -1;
    }
    
    // Open I2C device if not already open
    if (g_ingenic_peripheral.light_sensor_fd < 0) {
        g_ingenic_peripheral.light_sensor_fd = i2c_open(LIGHT_SENSOR_I2C_BUS, LIGHT_SENSOR_I2C_ADDR);
        if (g_ingenic_peripheral.light_sensor_fd < 0) {
            printf("[INGENIC PERIPHERAL] Failed to open I2C device\n");
            return -1;
        }
    }
    
    // Read light sensor
    float lux_value;
    int ret = read_bh1750_light_sensor(g_ingenic_peripheral.light_sensor_fd, &lux_value);
    if (ret < 0) {
        printf("[INGENIC PERIPHERAL] Failed to read light sensor\n");
        return -1;
    }
    
    // Convert lux to percentage (simplified)
    // Assuming max 1000 lux = 100%
    float percentage = (lux_value / 1000.0f) * 100.0f;
    if (percentage > 100.0f) {
        percentage = 100.0f;
    } else if (percentage < 0.0f) {
        percentage = 0.0f;
    }
    
    *value = percentage;
    printf("[INGENIC PERIPHERAL] Light sensor reading: %.1f lux (%.1f%%)\n", lux_value, percentage);
    
    return 0;
}