#include "ui_manager.h"
#include <stdio.h>

/* 这里模拟 LVGL 对象，实际需要包含 lvgl.h */
// lv_obj_t * label_co2;
// lv_obj_t * label_temp;
// lv_obj_t * label_hum;

void UI_Init(void) {
    // lv_init();
    // 注册显示驱动
    // 创建仪表盘布局、标题、数据卡片、实时曲线图等
}

void UI_Update_Data(float co2, float temp, float hum, const char* state) {
    char buf[32];
    
    // 更新 CO2 标签
    snprintf(buf, sizeof(buf), "CO2: %.0f ppm", co2);
    // lv_label_set_text(label_co2, buf);

    // 更新温湿度
    snprintf(buf, sizeof(buf), "T: %.1f C  H: %.1f %%", temp, hum);
    // lv_label_set_text(label_temp, buf);
    
    // 更新状态图表...
}

void UI_Tick(void) {
    // lv_timer_handler();
}
