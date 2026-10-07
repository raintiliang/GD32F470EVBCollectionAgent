#include "gd32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "co2_sensor.h"
#include "th_sensor.h"
#include "actuators.h"
#include "pid_control.h"
#include "storage_manager.h"
#include "comm_manager.h"

/* ------------------ 任务句柄 ------------------ */
TaskHandle_t SensorTask_Handler;
TaskHandle_t ControlTask_Handler;
TaskHandle_t StorageTask_Handler;
TaskHandle_t CommTask_Handler;
TaskHandle_t DisplayTask_Handler;

/* ------------------ 队列 & 互斥量 ------------------ */
QueueHandle_t SensorDataQueue;     // 用于将传感器采集数据传递给存储和通讯任务
SemaphoreHandle_t I2C_Mutex;        // 保护 I2C 总线访问的安全

/* ------------------ 联合数据打包 ------------------ */
typedef struct {
    uint32_t timestamp;
    CO2_Data_t co2;
    TH_Data_t th;
} System_Telemetry_t;

/**
 * @brief 传感器采集任务 (周期 1000ms, 优先级 3)
 */
void SensorTask(void *pvParameters) {
    System_Telemetry_t telemetry;
    char mock_uart_buf[64] = "425 850 26.3 65.2\r\n";
    uint32_t simulated_time = 1700000000;

    for (;;) {
        telemetry.timestamp = simulated_time++;

        /* 1. 解析 CO2 传感器数据 */
        CO2_Sensor_ParseFrame(mock_uart_buf, &telemetry.co2);

        /* 2. 读取 I2C 温湿度传感器 (使用 Mutex 保护总线) */
        if (xSemaphoreTake(I2C_Mutex, portMAX_DELAY) == pdTRUE) {
            TH_Sensor_Read(&telemetry.th);
            xSemaphoreGive(I2C_Mutex);
        }

        /* 3. 广播给传感器数据队列 (非阻塞覆盖写入) */
        xQueueOverwrite(SensorDataQueue, &telemetry);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief PID 闭环控制任务 (周期 200ms, 优先级 4 - 高实时性)
 */
void ControlTask(void *pvParameters) {
    System_Telemetry_t current_data;

    for (;;) {
        /* 从队列获取最新传感器数据进行控制计算 */
        if (xQueuePeek(SensorDataQueue, &current_data, 0) == pdTRUE) {
            PID_Task_Update(&current_data.co2, &current_data.th);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief 数据存储任务 (周期 60s / 队列触发, 优先级 1)
 */
void StorageTask(void *pvParameters) {
    System_Telemetry_t data_to_store;
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        // 每 60 秒获取当前最新帧存入 SD/TF 卡
        if (xQueuePeek(SensorDataQueue, &data_to_store, portMAX_DELAY) == pdTRUE) {
            Storage_Write_CSV(data_to_store.timestamp, &data_to_store.co2, &data_to_store.th);
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(60000));
    }
}

/**
 * @brief 4G 云端通讯上报任务 (周期 5s, 优先级 2)
 */
void CommTask(void *pvParameters) {
    System_Telemetry_t data_to_send;

    for (;;) {
        if (xQueuePeek(SensorDataQueue, &data_to_send, portMAX_DELAY) == pdTRUE) {
            Comm_Publish_Telemetry(&data_to_send.co2, &data_to_send.th);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/**
 * @brief 本地屏幕 UI 刷新任务 (周期 500ms, 优先级 2)
 */
void DisplayTask(void *pvParameters) {
    UI_Init();

    for (;;) {
        System_Telemetry_t current_data;
        if (xQueuePeek(SensorDataQueue, &current_data, 0) == pdTRUE) {
            UI_Update_Data(current_data.co2.co2_ppm, 
                           current_data.th.temperature, 
                           current_data.th.humidity, 
                           "REACTING");
        }
        
        UI_Tick();
        vTaskDelay(pdMS_TO_TICKS(50)); // LVGL 推荐 10~50ms 的刷新周期
    }
}

int main(void) {
    /* 1. 底层硬件与驱动初始化 */
    nvic_priority_group_config(NVIC_PRIGROUP_PRE4_SUB0);
    CO2_Sensor_Init();
    TH_Sensor_Init();
    Actuators_Init();
    PID_Init();
    Storage_Init();
    Comm_Init();

    /* 2. 创建 RTOS 同步原语 */
    SensorDataQueue = xQueueCreate(1, sizeof(System_Telemetry_t)); // 长度为1的最新值队列
    I2C_Mutex = xSemaphoreCreateMutex();

    /* 3. 创建 FreeRTOS 任务 */
    xTaskCreate(SensorTask,  "SensorTask",  512, NULL, 3, &SensorTask_Handler);
    xTaskCreate(ControlTask, "ControlTask", 512, NULL, 4, &ControlTask_Handler);
    xTaskCreate(StorageTask, "StorageTask", 1024, NULL, 1, &StorageTask_Handler);
    xTaskCreate(CommTask,    "CommTask",    1024, NULL, 2, &CommTask_Handler);
    xTaskCreate(DisplayTask, "DisplayTask", 1024, NULL, 2, &DisplayTask_Handler);

    /* 4. 启动 RTOS 调度器 */
    vTaskStartScheduler();

    while (1) {
        // 永远不会执行到这里
    }
}
