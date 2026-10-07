#include "th_sensor.h"
#include "gd32f4xx.h"
#include "gd32f4xx_i2c.h"
#include <string.h>

#define TH_I2C              I2C0
#define TH_I2C_CLK          RCU_I2C0
#define TH_I2C_SCL_PIN      GPIO_PIN_6
#define TH_I2C_SDA_PIN      GPIO_PIN_7
#define TH_I2C_GPIO_PORT    GPIOB
#define TH_I2C_GPIO_CLK     RCU_GPIOB
#define TH_I2C_AF           GPIO_AF_4

#define AHT10_ADDR          0x70  // 7-bit address

void TH_Sensor_Init(void) {
    /* 1. 开启 GPIO 和 I2C 时钟 */
    rcu_periph_clock_enable(TH_I2C_GPIO_CLK);
    rcu_periph_clock_enable(TH_I2C_CLK);

    /* 2. 配置 I2C 引脚 (PB6-SCL, PB7-SDA) */
    gpio_af_set(TH_I2C_GPIO_PORT, TH_I2C_AF, TH_I2C_SCL_PIN);
    gpio_af_set(TH_I2C_GPIO_PORT, TH_I2C_AF, TH_I2C_SDA_PIN);
    
    gpio_mode_set(TH_I2C_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TH_I2C_SCL_PIN);
    gpio_output_options_set(TH_I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, TH_I2C_SCL_PIN);
    
    gpio_mode_set(TH_I2C_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, TH_I2C_SDA_PIN);
    gpio_output_options_set(TH_I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, TH_I2C_SDA_PIN);

    /* 3. I2C 参数配置: 100kHz 标准模式 */
    i2c_deinit(TH_I2C);
    i2c_clock_config(TH_I2C, 100000U, I2C_DTCY_2);
    i2c_mode_addr_config(TH_I2C, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, AHT10_ADDR);
    i2c_enable(TH_I2C);
    i2c_ack_config(TH_I2C, I2C_ACK_ENABLE);
}

/**
 * @brief 简单的 I2C 等待超时宏
 */
#define I2C_TIMEOUT  10000
static bool I2C_Wait_Flag(uint32_t i2c_periph, i2c_flag_enum flag, FlagStatus status) {
    uint32_t timeout = I2C_TIMEOUT;
    while(i2c_flag_get(i2c_periph, flag) != status) {
        if((timeout--) == 0) return false;
    }
    return true;
}

bool TH_Sensor_Read(TH_Data_t *data) {
    if (data == NULL) return false;

    // AHT10 触发测量指令: 0xAC, 0x33, 0x00
    uint8_t cmd[] = {0xAC, 0x33, 0x00};
    
    // 1. 发送起始位
    i2c_start_on_bus(TH_I2C);
    if(!I2C_Wait_Flag(TH_I2C, I2C_FLAG_SBSEND, SET)) return false;

    // 2. 发送从机地址 (写)
    i2c_master_addressing(TH_I2C, AHT10_ADDR << 1, I2C_TRANSMITTER);
    if(!I2C_Wait_Flag(TH_I2C, I2C_FLAG_ADDSEND, SET)) return false;
    i2c_flag_clear(TH_I2C, I2C_FLAG_ADDSEND);

    // 3. 发送数据
    for(int i=0; i<3; i++) {
        i2c_data_transmit(TH_I2C, cmd[i]);
        if(!I2C_Wait_Flag(TH_I2C, I2C_FLAG_TBE, SET)) return false;
    }

    // 4. 发送停止位
    i2c_stop_on_bus(TH_I2C);

    // 5. 等待测量完成 (约 80ms)
    // 注意：在实际 RTOS 环境中应使用 vTaskDelay
    for(volatile int i=0; i<1000000; i++); 

    // 6. 读取数据 (6 字节)
    uint8_t buf[6];
    i2c_start_on_bus(TH_I2C);
    if(!I2C_Wait_Flag(TH_I2C, I2C_FLAG_SBSEND, SET)) return false;
    
    i2c_master_addressing(TH_I2C, AHT10_ADDR << 1, I2C_RECEIVER);
    if(!I2C_Wait_Flag(TH_I2C, I2C_FLAG_ADDSEND, SET)) return false;
    i2c_flag_clear(TH_I2C, I2C_FLAG_ADDSEND);

    for(int i=0; i<6; i++) {
        if(i == 5) i2c_ack_config(TH_I2C, I2C_ACK_DISABLE); // 最后字节不应答
        if(!I2C_Wait_Flag(TH_I2C, I2C_FLAG_RBNE, SET)) return false;
        buf[i] = i2c_data_receive(TH_I2C);
    }
    i2c_stop_on_bus(TH_I2C);
    i2c_ack_config(TH_I2C, I2C_ACK_ENABLE);

    // 7. 转换数据
    // 状态位检查buf[0]...此处略过，直接转换
    uint32_t hum_raw = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | (buf[3] >> 4);
    uint32_t temp_raw = (((uint32_t)buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5];

    data->humidity = (float)hum_raw * 100.0f / 1048576.0f;
    data->temperature = (float)temp_raw * 200.0f / 1048576.0f - 50.0f;
    data->is_valid = true;

    return true;
}
