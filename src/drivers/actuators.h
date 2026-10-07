#ifndef __ACTUATORS_H
#define __ACTUATORS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 电磁阀/加湿机状态
 */
typedef enum {
    ACTUATOR_OFF = 0,
    ACTUATOR_ON  = 1
} Actuator_State_t;

/**
 * @brief 初始化所有执行器 (GPIO + RS485)
 */
void Actuators_Init(void);

/**
 * @brief 控制 CO2 电磁阀
 */
void Actuators_Set_CO2_Valve(Actuator_State_t state);

/**
 * @brief 控制加湿机
 */
void Actuators_Set_Humidifier(Actuator_State_t state);

/**
 * @brief 控制除湿机模组 (RS485/Modbus)
 * @param humidity_target 目标湿度 (通过RS485下发给除湿控制器)
 */
void Actuators_Set_Dehumidifier(bool enable, uint16_t humidity_target);

#endif /* __ACTUATORS_H */
