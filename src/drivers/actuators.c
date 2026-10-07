#include "actuators.h"
#include "gd32f4xx.h"

/* GPIO 定义 */
#define VALVE_GPIO_PORT      GPIOC
#define VALVE_GPIO_PIN       GPIO_PIN_0
#define VALVE_GPIO_CLK       RCU_GPIOC

#define HUMID_GPIO_PORT      GPIOC
#define HUMID_GPIO_PIN       GPIO_PIN_1
#define HUMID_GPIO_CLK       RCU_GPIOC

/* RS485 (USART2) 定义 */
#define RS485_UART           USART2
#define RS485_UART_CLK       RCU_USART2
#define RS485_GPIO_PORT      GPIOB
#define RS485_GPIO_CLK       RCU_GPIOB
#define RS485_TX_PIN         GPIO_PIN_10
#define RS485_RX_PIN         GPIO_PIN_11
#define RS485_AF             GPIO_AF_7

/* 485 收发方向切换引脚 (DE) */
#define RS485_DE_PORT        GPIOB
#define RS485_DE_PIN         GPIO_PIN_12
#define RS485_DE_CLK         RCU_GPIOB

void Actuators_Init(void) {
    /* 1. 初始化 GPIO (电磁阀 & 加湿机) */
    rcu_periph_clock_enable(VALVE_GPIO_CLK);
    gpio_mode_set(VALVE_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, VALVE_GPIO_PIN);
    gpio_output_options_set(VALVE_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, VALVE_GPIO_PIN);
    gpio_bit_reset(VALVE_GPIO_PORT, VALVE_GPIO_PIN); // 默认关闭

    rcu_periph_clock_enable(HUMID_GPIO_CLK);
    gpio_mode_set(HUMID_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, HUMID_GPIO_PIN);
    gpio_output_options_set(HUMID_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, HUMID_GPIO_PIN);
    gpio_bit_reset(HUMID_GPIO_PORT, HUMID_GPIO_PIN); // 默认关闭

    /* 2. 初始化 RS485 (USART2) */
    rcu_periph_clock_enable(RS485_GPIO_CLK);
    rcu_periph_clock_enable(RS485_UART_CLK);
    rcu_periph_clock_enable(RS485_DE_CLK);

    gpio_af_set(RS485_GPIO_PORT, RS485_AF, RS485_TX_PIN);
    gpio_af_set(RS485_GPIO_PORT, RS485_AF, RS485_RX_PIN);
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS485_TX_PIN);
    gpio_mode_set(RS485_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, RS485_RX_PIN);

    /* 方向控制引脚 DE */
    gpio_mode_set(RS485_DE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS485_DE_PIN);
    gpio_bit_reset(RS485_DE_PORT, RS485_DE_PIN); // 默认为接收模式

    usart_deinit(RS485_UART);
    usart_baudrate_set(RS485_UART, 9600U); // Modbus 常用 9600
    usart_receive_config(RS485_UART, USART_RECEIVE_ENABLE);
    usart_transmit_config(RS485_UART, USART_TRANSMIT_ENABLE);
    usart_enable(RS485_UART);
}

void Actuators_Set_CO2_Valve(Actuator_State_t state) {
    if (state == ACTUATOR_ON) {
        gpio_bit_set(VALVE_GPIO_PORT, VALVE_GPIO_PIN);
    } else {
        gpio_bit_reset(VALVE_GPIO_PORT, VALVE_GPIO_PIN);
    }
}

void Actuators_Set_Humidifier(Actuator_State_t state) {
    if (state == ACTUATOR_ON) {
        gpio_bit_set(HUMID_GPIO_PORT, HUMID_GPIO_PIN);
    } else {
        gpio_bit_reset(HUMID_GPIO_PORT, HUMID_GPIO_PIN);
    }
}

/**
 * @brief 计算 Modbus CRC16
 */
static uint16_t Modbus_CRC16(uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (uint16_t j = 8; j != 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void Actuators_Set_Dehumidifier(bool enable, uint16_t humidity_target) {
    uint8_t frame[8];
    frame[0] = 0x01; // Slave Addr
    frame[1] = 0x06; // Write Single Register
    frame[2] = 0x00; // Reg Addr High
    frame[3] = 0x01; // Reg Addr Low (假设0x01为开关/目标设定寄存器)
    frame[4] = (enable ? 0x01 : 0x00); 
    frame[5] = (uint8_t)humidity_target;
    
    uint16_t crc = Modbus_CRC16(frame, 6);
    frame[6] = (uint8_t)(crc & 0xFF);
    frame[7] = (uint8_t)(crc >> 8);

    /* 切换为发送模式 */
    gpio_bit_set(RS485_DE_PORT, RS485_DE_PIN);
    
    for (int i = 0; i < 8; i++) {
        usart_data_transmit(RS485_UART, frame[i]);
        while (usart_flag_get(RS485_UART, USART_FLAG_TBE) == RESET);
    }
    while (usart_flag_get(RS485_UART, USART_FLAG_TC) == RESET); // 等待发送完成

    /* 切回接收模式 */
    gpio_bit_reset(RS485_DE_PORT, RS485_DE_PIN);
}
