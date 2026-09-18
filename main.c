#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "systick.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

/* Task Priorities */
#define KEY_SCAN_TASK_PRIO    ( tskIDLE_PRIORITY + 5 )
#define DISPLAY_TASK_PRIO     ( tskIDLE_PRIORITY + 5 )
#define SENSOR_TASK_PRIO      ( tskIDLE_PRIORITY + 3 )
#define ACTUATOR_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )
#define CONTROL_TASK_PRIO     ( tskIDLE_PRIORITY + 1 )
#define STORAGE_TASK_PRIO     ( tskIDLE_PRIORITY + 4 )
#define COMM_TASK_PRIO        ( tskIDLE_PRIORITY + 2 )

/* Task Handles */
TaskHandle_t KeyScanTaskHandle = NULL;
TaskHandle_t DisplayTaskHandle = NULL;
TaskHandle_t SensorTaskHandle = NULL;
TaskHandle_t ActuatorTaskHandle = NULL;
TaskHandle_t ControlTaskHandle = NULL;
TaskHandle_t StorageTaskHandle = NULL;
TaskHandle_t CommTaskHandle = NULL;

/* Task Prototypes */
void vKeyScanTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void vSensorTask(void *pvParameters);
void vActuatorTask(void *pvParameters);
void vControlTask(void *pvParameters);
void vStorageTask(void *pvParameters);
void vCommTask(void *pvParameters);

void hardware_init(void) {
    /* TODO: Init GPIO, UART, I2C, SPI, RS485 */
}

int main(void) {
    hardware_init();

    xTaskCreate(vKeyScanTask, "KeyScan", 256, NULL, KEY_SCAN_TASK_PRIO, &KeyScanTaskHandle);
    xTaskCreate(vDisplayTask, "Display", 1024, NULL, DISPLAY_TASK_PRIO, &DisplayTaskHandle);
    xTaskCreate(vSensorTask, "Sensor", 512, NULL, SENSOR_TASK_PRIO, &SensorTaskHandle);
    xTaskCreate(vActuatorTask, "Actuator", 256, NULL, ACTUATOR_TASK_PRIO, &ActuatorTaskHandle);
    xTaskCreate(vControlTask, "Control", 512, NULL, CONTROL_TASK_PRIO, &ControlTaskHandle);
    xTaskCreate(vStorageTask, "Storage", 1024, NULL, STORAGE_TASK_PRIO, &StorageTaskHandle);
    xTaskCreate(vCommTask, "Comm", 1024, NULL, COMM_TASK_PRIO, &CommTaskHandle);

    vTaskStartScheduler();

    while(1);
}

/* Task Definitions (Stubs) */
void vKeyScanTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vDisplayTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vSensorTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vActuatorTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vControlTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vStorageTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

void vCommTask(void *pvParameters) {
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
