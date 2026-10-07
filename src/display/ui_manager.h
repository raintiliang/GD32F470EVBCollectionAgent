#ifndef __UI_MANAGER_H
#define __UI_MANAGER_H

#include <stdint.h>
#include "co2_sensor.h"
#include "th_sensor.h"

/**
 * @brief 初始化 LVGL 与 UI 组件
 */
void UI_Init(void);

/**
 * @brief 更新 UI 显示的数据
 */
void UI_Update_Data(float co2, float temp, float hum, const char* state);

/**
 * @brief LVGL 节拍处理 (应在 1ms 中断或任务中调用)
 */
void UI_Tick(void);

#endif /* __UI_MANAGER_H */
