#include "tft_display.h"
#include "gd32f4xx.h"

/* 假设使用 SPI 接口驱动 5 寸屏控制器 (如 ILI9488/SSD1963) */
#define TFT_SPI              SPI0
#define TFT_GPIO_PORT        GPIOA
#define TFT_CS_PIN           GPIO_PIN_4
#define TFT_DC_PIN           GPIO_PIN_2
#define TFT_RST_PIN          GPIO_PIN_3

void TFT_Init(void) {
    /* 1. 初始化 GPIO 引脚 (CS, DC, RST) */
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(TFT_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TFT_CS_PIN | TFT_DC_PIN | TFT_RST_PIN);
    gpio_output_options_set(TFT_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, TFT_CS_PIN | TFT_DC_PIN | TFT_RST_PIN);

    /* 2. 复位屏幕 */
    gpio_bit_reset(TFT_GPIO_PORT, TFT_RST_PIN);
    // Delay...
    gpio_bit_set(TFT_GPIO_PORT, TFT_RST_PIN);

    /* 3. 配置 SPI 外设 (GD32F4xx) */
    // rcu_periph_clock_enable(RCU_SPI0);
    // spi_parameter_struct spi_init_struct;
    // spi_init(SPI0, &spi_init_struct);
    // spi_enable(SPI0);
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    // 设置地址窗并发送颜色数据
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // 快速填充逻辑
}
