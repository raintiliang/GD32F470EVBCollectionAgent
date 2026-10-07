#include "comm_manager.h"
#include <stdio.h>
#include <string.h>

void Comm_Init(void) {
    // TODO: 4G 模块 (UART/AT指令) 初始化
}

void Comm_Publish_Telemetry(const CO2_Data_t *co2, const TH_Data_t *th) {
    if (co2 == NULL || th == NULL) return;

    char json_payload[256];
    // 组装 JSON 格式负载
    snprintf(json_payload, sizeof(json_payload), 
             "{\"co2\":%.1f, \"temp\":%.1f, \"hum\":%.1f}",
             co2->co2_ppm, th->temperature, th->humidity);

    // TODO: 调用 MQTTClient_publish 发送数据
    // 主题: maats/{device_id}/telemetry
}
