#ifndef __CO2_SENSOR_H
#define __CO2_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief CO2 传感器数据结构体
 */
typedef struct {
    float co2_ppm;          // CO2 浓度 (ppm)
    float temperature;      // 温度 (°C)
    float humidity;         // 湿度 (%)
    bool  is_valid;         // 数据是否有效
} CO2_Data_t;

/**
 * @brief 初始化 CO2 传感器 (UART)
 */
void CO2_Sensor_Init(void);

/**
 * @brief 解析接收到的数据帧
 * @param frame 接收到的字符串帧 (例如: "425 850 26.3 65.2\r\n")
 * @param data 输出解析后的数据
 * @return true 解析成功, false 解析失败
 */
bool CO2_Sensor_ParseFrame(const char *frame, CO2_Data_t *data);

/**
 * @brief 获取最新的 CO2 传感器数据
 * @param data 输出数据结构体指针
 */
void CO2_Sensor_GetData(CO2_Data_t *data);

#endif /* __CO2_SENSOR_H */
