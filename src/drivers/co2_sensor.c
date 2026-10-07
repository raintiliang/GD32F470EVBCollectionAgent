#include "co2_sensor.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static CO2_Data_t current_sensor_data = {0};

void CO2_Sensor_Init(void) {
    /* 1. 开启 GPIO 和 USART 时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    /* 2. 配置 USART0 TX (PA9) 和 RX (PA10) 引脚 */
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);
    gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);
    
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* 3. USART 参数配置: 9600bps, 8N1 */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 9600U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    
    usart_enable(USART0);

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
