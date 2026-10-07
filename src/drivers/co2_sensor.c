#include "co2_sensor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static CO2_Data_t current_sensor_data = {0};

void CO2_Sensor_Init(void) {
    // TODO: 实现针对特定硬件(如GD32)的UART初始化代码
    // 配置 9600bps, 8N1
    current_sensor_data.is_valid = false;
}

bool CO2_Sensor_ParseFrame(const char *frame, CO2_Data_t *data) {
    if (frame == NULL || data == NULL) return false;

    // 协议示例: "ID CO2 TEMP HUM\r\n" -> "425 850 26.3 65.2\r\n"
    int id;
    float co2, temp, hum;
    
    int count = sscanf(frame, "%d %f %f %f", &id, &co2, &temp, &hum);
    
    if (count == 4) {
        data->co2_ppm = co2;
        data->temperature = temp;
        data->humidity = hum;
        data->is_valid = true;
        return true;
    }

    data->is_valid = false;
    return false;
}

void CO2_Sensor_GetData(CO2_Data_t *data) {
    if (data != NULL) {
        memcpy(data, &current_sensor_data, sizeof(CO2_Data_t));
    }
}
