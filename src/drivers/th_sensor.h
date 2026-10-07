#ifndef __TH_SENSOR_H
#define __TH_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 温湿度传感器数据结构体
 */
typedef struct {
    float temperature;      // 温度 (°C)
    float humidity;         // 湿度 (%)
    bool  is_valid;         // 数据是否有效
} TH_Data_t;

/**
 * @brief 初始化温湿度传感器 (I2C)
 */
void TH_Sensor_Init(void);

/**
 * @brief 读取最新的温湿度数据 (AHT10/SHT40)
 * @param data 输出数据结构体指针
 * @return true 读取成功, false 读取失败
 */
bool TH_Sensor_Read(TH_Data_t *data);

#endif /* __TH_SENSOR_H */
