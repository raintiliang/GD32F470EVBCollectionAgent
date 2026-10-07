#ifndef __TFT_DISPLAY_H
#define __TFT_DISPLAY_H

#include <stdint.h>

/**
 * @brief 初始化 5 寸 TFT 屏幕 (SPI/RGB)
 */
void TFT_Init(void);

/**
 * @brief 在特定位置绘制像素点 (供 LVGL 调用)
 */
void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief 填充矩形区域
 */
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif /* __TFT_DISPLAY_H */
