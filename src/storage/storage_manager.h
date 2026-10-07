#ifndef __STORAGE_MANAGER_H
#define __STORAGE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "co2_sensor.h"
#include "th_sensor.h"

/**
 * @brief 初始化 FATFS 与 SD/TF 卡挂载
 */
bool Storage_Init(void);

/**
 * @brief 将当前数据追加写入 CSV 文件 (格式: timestamp,co2_ppm,temp,humidity)
 */
bool Storage_Write_CSV(uint32_t timestamp, const CO2_Data_t *co2, const TH_Data_t *th);

#endif /* __STORAGE_MANAGER_H */
