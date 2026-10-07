#ifndef CO2_SENSOR_H
#define CO2_SENSOR_H

#include "gd32f4xx.h"
#include <stdint.h>

/* S8-005 CO2 Sensor Configuration */
#define CO2_USART               UART3
#define CO2_USART_CLK           RCU_UART3
#define CO2_USART_TX_PIN        GPIO_PIN_10
#define CO2_USART_RX_PIN        GPIO_PIN_11
#define CO2_USART_GPIO_PORT     GPIOC
#define CO2_USART_GPIO_CLK      RCU_GPIOC
#define CO2_USART_AF            GPIO_AF_8

#define CO2_FRAME_MAX_LEN       64

typedef struct {
    int co2_ppm;
    float temperature;
    float humidity;
    uint8_t data_ready;
} co2_data_t;

void co2_sensor_init(void);
int co2_sensor_parse(const char *frame, co2_data_t *data);
void co2_sensor_rx_handler(uint8_t ch);
extern char rx_buffer[];

#endif /* CO2_SENSOR_H */
