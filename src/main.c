#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "co2_sensor.h"
#include "th_sensor.h"
#include "actuators.h"
#include "pid_control.h"

/* 任务句柄定义 */
TaskHandle_t SensorTask_Handler;
TaskHandle_t ControlTask_Handler;
TaskHandle_t DisplayTask_Handler;

/* 全局数据队列/互斥量 (预留) */
QueueHandle_t SensorDataQueue;

/**
 * @brief 传感器采集任务 (周期 1s)
 */
void SensorTask(void *pvParameters) {
    CO2_Data_t co2_data;
    TH_Data_t th_data;
    char uart_rx_buf[64] = "425 850 26.3 65.2\r\n"; // 模拟输入

    for (;;) {
        /* 1. 采集 CO2 传感器数据 */
        CO2_Sensor_ParseFrame(uart_rx_buf, &co2_data);

        /* 2. 采集温湿度传感器数据 */
        TH_Sensor_Read(&th_data);

        /* 3. 闭环 PID 控制更新 */
        PID_Task_Update(&co2_data, &th_data);

        /* 挂起 1 秒 */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief 显示刷新任务 (周期 500ms)
 */
void DisplayTask(void *pvParameters) {
    for (;;) {
        // TODO: 刷新本地 5 寸屏 UI (LVGL/TFT)
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void) {
    /* 1. 硬件外设初始化 */
    nvic_priority_group_config(NVIC_PRIGROUP_PRE4_SUB0);
    CO2_Sensor_Init();
    TH_Sensor_Init();
    Actuators_Init();
    PID_Init();

    /* 2. 创建 FreeRTOS 任务 */
    xTaskCreate((TaskFunction_t )SensorTask,
                (const char*    )"SensorTask",
                (uint16_t       )512,
                (void*          )NULL,
                (UBaseType_t    )3,
                (TaskHandle_t*  )&SensorTask_Handler);

    xTaskCreate((TaskFunction_t )DisplayTask,
                (const char*    )"DisplayTask",
                (uint16_t       )512,
                (void*          )NULL,
                (UBaseType_t    )2,
                (TaskHandle_t*  )&DisplayTask_Handler);

    /* 3. 启动任务调度器 */
    vTaskStartScheduler();

    while (1) {
        // 不应该运行到这里
    }
}
