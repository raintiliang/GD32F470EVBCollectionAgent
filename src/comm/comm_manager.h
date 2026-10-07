#ifndef __COMM_MANAGER_H
#define __COMM_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "co2_sensor.h"
#include "th_sensor.h"

/**
 * @brief 初始化 4G 模块与 MQTT 协议栈
 */
void Comm_Init(void);

/**
 * @brief 发送遥测数据到云端 (MQTT Publish)
 */
void Comm_Publish_Telemetry(const CO2_Data_t *co2, const TH_Data_t *th);

#endif /* __COMM_MANAGER_H */
