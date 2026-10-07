#include "storage_manager.h"
#include <stdio.h>
#include <string.h>

// 模拟 FATFS 相关变量，实际开发需集成 ff.h
// FATFS fs;
// FIL file;

bool Storage_Init(void) {
    // TODO: 实现 f_mount 代码逻辑
    return true;
}

bool Storage_Write_CSV(uint32_t timestamp, const CO2_Data_t *co2, const TH_Data_t *th) {
    if (co2 == NULL || th == NULL) return false;

    char line_buf[128];
    // 组装 CSV 行: timestamp,co2_ppm,temp,hum
    snprintf(line_buf, sizeof(line_buf), "%u,%.1f,%.1f,%.1f\n", 
             timestamp, co2->co2_ppm, th->temperature, th->humidity);

    /* 实际 FATFS 写入逻辑示例:
    f_open(&file, "data.csv", FA_OPEN_APPEND | FA_WRITE);
    f_write(&file, line_buf, strlen(line_buf), &bw);
    f_close(&file);
    */
    
    return true;
}
