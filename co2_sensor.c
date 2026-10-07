#include "co2_sensor.h"
#include <string.h>
#include <stdio.h>

char rx_buffer[CO2_FRAME_MAX_LEN];
static uint8_t rx_index = 0;

/**
 * @brief Initialize UART3 for S8-005 CO2 Sensor (9600, 8N1)
 */
void co2_sensor_init(void) {
    /* Enable GPIO and UART clocks */
    rcu_periph_clock_enable(CO2_USART_GPIO_CLK);
    rcu_periph_clock_enable(CO2_USART_CLK);

    /* Configure UART Pins */
    gpio_af_set(CO2_USART_GPIO_PORT, CO2_USART_AF, CO2_USART_TX_PIN);
    gpio_af_set(CO2_USART_GPIO_PORT, CO2_USART_AF, CO2_USART_RX_PIN);
    gpio_mode_set(CO2_USART_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, CO2_USART_TX_PIN);
    gpio_output_options_set(CO2_USART_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CO2_USART_TX_PIN);
    gpio_mode_set(CO2_USART_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, CO2_USART_RX_PIN);
    gpio_output_options_set(CO2_USART_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CO2_USART_RX_PIN);

    /* UART Configuration */
    usart_deinit(CO2_USART);
    usart_baudrate_set(CO2_USART, 9600U);
    usart_receive_config(CO2_USART, USART_RECEIVE_ENABLE);
    usart_transmit_config(CO2_USART, USART_TRANSMIT_ENABLE);
    
    /* Enable RX Interrupt */
    usart_interrupt_enable(CO2_USART, USART_INT_RBNE);
    nvic_irq_enable(UART3_IRQn, 0, 0);
    
    usart_enable(CO2_USART);
}

/**
 * @brief Parse the active transmission frame from sensor
 * Sample: "425 850 26.3 65.2\r\n"
 */
int co2_sensor_parse(const char *frame, co2_data_t *data) {
    int co2;
    float temp, humid;
    /* Format according to SRS Detail: sscanf(frame, "%*d %d %f %f", co2, temp, humid) */
    int scanned = sscanf(frame, "%*d %d %f %f", &co2, &temp, &humid);
    if (scanned == 3) {
        data->co2_ppm = co2;
        data->temperature = temp;
        data->humidity = humid;
        data->data_ready = 1;
        return 0;
    }
    return -1;
}

/**
 * @brief Process character received from UART (Call in UART3_IRQHandler)
 */
void co2_sensor_rx_handler(uint8_t ch) {
    if (rx_index < CO2_FRAME_MAX_LEN - 1) {
        rx_buffer[rx_index++] = ch;
        if (ch == '\n') {
            rx_buffer[rx_index] = '\0';
            /* Frame completed, signal task would go here via Queue or Flag */
            /* For now we just reset for next frame */
            rx_index = 0;
        }
    } else {
        rx_index = 0; // Buffer overflow, reset
    }
}
