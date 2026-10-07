#ifndef __PID_CONTROL_H
#define __PID_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "co2_sensor.h"
#include "th_sensor.h"

/**
 * @brief PID 参数结构体
 */
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float target;          // 目标设定值
    float prev_error;      // 上一次误差
    float integral;        // 积分累加项
    float integral_max;    // 抗积分饱和上限
    float output_max;      // 输出上限
    float output_min;      // 输出下限
} PID_Controller_t;

/**
 * @brief 初始化 PID 控制器
 */
void PID_Init(void);

/**
 * @brief 设置目标 CO2 浓度与目标湿度
 */
void PID_Set_Targets(float co2_ppm, float humidity_rh);

/**
 * @brief 执行一次 PID 闭环控制计算与执行器输出
 * @param co2_data 当前 CO2 数据
 * @param th_data 当前温湿度数据
 */
void PID_Task_Update(const CO2_Data_t *co2_data, const TH_Data_t *th_data);

#endif /* __PID_CONTROL_H */
