// HAL Peripheral Abstraction
// Unified interface for GPIO, I2C, SPI, UART, PWM, and other peripherals

#ifndef _HAL_PERIPHERAL_H_
#define _HAL_PERIPHERAL_H_

#include "hal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// GPIO configuration
typedef struct {
    uint32_t pin;                   // GPIO pin number
    uint32_t direction;             // 0=input, 1=output
    uint32_t pull;                  // 0=none, 1=pull-up, 2=pull-down
    uint32_t drive_strength;        // Drive strength (mA)
    uint32_t slew_rate;             // Slew rate control
    bool interrupt_enable;          // Enable interrupt
    uint32_t interrupt_mode;        // 0=rising, 1=falling, 2=both
    void (*isr_handler)(uint32_t pin, void *priv); // ISR handler
    void *isr_priv;                 // ISR private data
} hal_gpio_config_t;

// I2C configuration
typedef struct {
    uint32_t bus;                   // I2C bus number
    uint32_t speed;                 // I2C speed (Hz)
    uint8_t slave_addr;             // Slave address
    uint32_t timeout_ms;            // Timeout in milliseconds
    bool ten_bit_addr;              // 10-bit addressing
} hal_i2c_config_t;

// SPI configuration
typedef struct {
    uint32_t bus;                   // SPI bus number
    uint32_t speed;                 // SPI speed (Hz)
    uint32_t mode;                  // SPI mode (0-3)
    uint32_t bits_per_word;         // Bits per word (usually 8)
    uint32_t cs_pin;                // Chip select GPIO pin
    bool cs_active_low;             // Chip select active low
    bool lsb_first;                 // LSB first
} hal_spi_config_t;

// UART configuration
typedef struct {
    uint32_t port;                  // UART port number
    uint32_t baudrate;              // Baud rate
    uint32_t data_bits;             // Data bits (5,6,7,8)
    uint32_t stop_bits;             // Stop bits (1,2)
    uint32_t parity;                // Parity: 0=none, 1=odd, 2=even
    uint32_t flow_control;          // Flow control: 0=none, 1=RTS/CTS
    bool rs485_mode;                // RS485 mode
    uint32_t rs485_direction_pin;   // RS485 direction control pin
} hal_uart_config_t;

// PWM configuration
typedef struct {
    uint32_t pwm_id;                // PWM channel ID
    uint32_t period_ns;             // Period in nanoseconds
    uint32_t duty_ns;               // Duty cycle in nanoseconds
    bool polarity;                  // Polarity: 0=normal, 1=inverted
    bool enable;                    // Enable PWM output
} hal_pwm_config_t;

// ADC configuration
typedef struct {
    uint32_t channel;               // ADC channel number
    uint32_t sample_rate;           // Sample rate (Hz)
    uint32_t resolution;            // Resolution in bits
    uint32_t reference_voltage;     // Reference voltage (mV)
    uint32_t scale_factor;          // Scaling factor (mV/count)
} hal_adc_config_t;

// IR-CUT control
typedef struct {
    uint32_t cut_pin;               // IR-CUT control pin
    uint32_t led_pin;               // IR-LED control pin
    bool auto_switch;               // Auto switch based on light level
    uint32_t light_threshold;       // Light threshold for auto switch
    uint32_t delay_ms;              // Switch delay in milliseconds
} hal_ircut_config_t;

// LED control
typedef struct {
    uint32_t led_id;                // LED identifier
    uint32_t pin;                   // GPIO pin for LED
    bool active_low;                // Active low
    uint32_t blink_pattern;         // Blink pattern (bitmask)
    uint32_t blink_interval_ms;     // Blink interval in milliseconds
} hal_led_config_t;

// Motor control (for PTZ)
typedef struct {
    uint32_t motor_id;              // Motor identifier (0=pan, 1=tilt, 2=zoom)
    uint32_t step_pin;              // Step control pin
    uint32_t dir_pin;               // Direction control pin
    uint32_t enable_pin;            // Enable pin
    uint32_t steps_per_rev;         // Steps per revolution
    uint32_t max_speed;             // Maximum speed (steps/second)
    uint32_t acceleration;          // Acceleration (steps/second^2)
    uint32_t microsteps;            // Microstepping resolution
} hal_motor_config_t;

// Temperature sensor
typedef struct {
    uint32_t sensor_id;             // Sensor identifier
    uint32_t bus_type;              // 0=I2C, 1=SPI, 2=OneWire
    uint32_t bus_id;                // Bus identifier
    uint8_t address;                // Device address
    float temperature;              // Current temperature (°C)
    float humidity;                 // Current humidity (%)
} hal_temp_sensor_t;

// GPIO operations
int hal_gpio_init(void);
int hal_gpio_deinit(void);
int hal_gpio_set_config(const hal_gpio_config_t *config);
int hal_gpio_get_config(uint32_t pin, hal_gpio_config_t *config);
int hal_gpio_set_value(uint32_t pin, uint32_t value);
int hal_gpio_get_value(uint32_t pin, uint32_t *value);
int hal_gpio_toggle(uint32_t pin);
int hal_gpio_enable_interrupt(uint32_t pin);
int hal_gpio_disable_interrupt(uint32_t pin);
int hal_gpio_clear_interrupt(uint32_t pin);

// I2C operations
int hal_i2c_init(void);
int hal_i2c_deinit(void);
int hal_i2c_open(const hal_i2c_config_t *config, void **handle);
int hal_i2c_close(void *handle);
int hal_i2c_read(void *handle, uint8_t *data, uint32_t len);
int hal_i2c_write(void *handle, const uint8_t *data, uint32_t len);
int hal_i2c_read_reg(void *handle, uint8_t reg_addr, uint8_t *value);
int hal_i2c_write_reg(void *handle, uint8_t reg_addr, uint8_t value);
int hal_i2c_read_block(void *handle, uint8_t reg_addr, uint8_t *data, uint32_t len);
int hal_i2c_write_block(void *handle, uint8_t reg_addr, const uint8_t *data, uint32_t len);

// SPI operations
int hal_spi_init(void);
int hal_spi_deinit(void);
int hal_spi_open(const hal_spi_config_t *config, void **handle);
int hal_spi_close(void *handle);
int hal_spi_transfer(void *handle, const uint8_t *tx_data, uint8_t *rx_data, uint32_t len);
int hal_spi_write(void *handle, const uint8_t *data, uint32_t len);
int hal_spi_read(void *handle, uint8_t *data, uint32_t len);

// UART operations
int hal_uart_init(void);
int hal_uart_deinit(void);
int hal_uart_open(const hal_uart_config_t *config, void **handle);
int hal_uart_close(void *handle);
int hal_uart_write(void *handle, const uint8_t *data, uint32_t len);
int hal_uart_read(void *handle, uint8_t *data, uint32_t len, uint32_t timeout_ms);
int hal_uart_get_rx_available(void *handle, uint32_t *available);
int hal_uart_get_tx_available(void *handle, uint32_t *available);
int hal_uart_flush(void *handle);
int hal_uart_set_baudrate(void *handle, uint32_t baudrate);
int hal_uart_set_mode(void *handle, uint32_t data_bits, uint32_t stop_bits, uint32_t parity);

// PWM operations
int hal_pwm_init(void);
int hal_pwm_deinit(void);
int hal_pwm_open(const hal_pwm_config_t *config, void **handle);
int hal_pwm_close(void *handle);
int hal_pwm_start(void *handle);
int hal_pwm_stop(void *handle);
int hal_pwm_set_period(void *handle, uint32_t period_ns);
int hal_pwm_set_duty(void *handle, uint32_t duty_ns);
int hal_pwm_set_frequency(void *handle, uint32_t frequency_hz);
int hal_pwm_set_duty_ratio(void *handle, float duty_ratio);  // 0.0 to 1.0

// ADC operations
int hal_adc_init(void);
int hal_adc_deinit(void);
int hal_adc_open(const hal_adc_config_t *config, void **handle);
int hal_adc_close(void *handle);
int hal_adc_read(void *handle, uint32_t *value);
int hal_adc_read_voltage(void *handle, uint32_t *voltage_mv);
int hal_adc_read_multiple(void *handle, uint32_t *values, uint32_t count);
int hal_adc_set_sample_rate(void *handle, uint32_t sample_rate);

// IR-CUT operations
int hal_ircut_init(const hal_ircut_config_t *config);
int hal_ircut_deinit(void);
int hal_ircut_set_mode(bool day_mode);  // true=day mode (IR-CUT filter out), false=night mode (IR-CUT filter in)
int hal_ircut_get_mode(bool *day_mode);
int hal_ircut_set_irled(bool enable);   // Enable/disable IR-LED
int hal_ircut_get_irled(bool *enabled);
int hal_ircut_auto_switch(bool enable);
int hal_ircut_set_light_threshold(uint32_t threshold);

// LED operations
int hal_led_init(void);
int hal_led_deinit(void);
int hal_led_set_config(const hal_led_config_t *config);
int hal_led_on(uint32_t led_id);
int hal_led_off(uint32_t led_id);
int hal_led_toggle(uint32_t led_id);
int hal_led_blink(uint32_t led_id, uint32_t pattern, uint32_t interval_ms);
int hal_led_set_brightness(uint32_t led_id, uint32_t brightness);  // 0-100%

// Motor (PTZ) operations
int hal_motor_init(void);
int hal_motor_deinit(void);
int hal_motor_set_config(const hal_motor_config_t *config);
int hal_motor_enable(uint32_t motor_id, bool enable);
int hal_motor_move(uint32_t motor_id, int32_t steps, uint32_t speed);
int hal_motor_move_to(uint32_t motor_id, int32_t position, uint32_t speed);
int hal_motor_stop(uint32_t motor_id);
int hal_motor_get_position(uint32_t motor_id, int32_t *position);
int hal_motor_set_home(uint32_t motor_id);
int hal_motor_go_home(uint32_t motor_id);
int hal_motor_set_limit(uint32_t motor_id, int32_t min_pos, int32_t max_pos);

// Temperature sensor operations
int hal_temp_sensor_init(void);
int hal_temp_sensor_deinit(void);
int hal_temp_sensor_probe(hal_temp_sensor_t *sensors, uint32_t max_count, uint32_t *count);
int hal_temp_sensor_read(uint32_t sensor_id, float *temperature, float *humidity);
int hal_temp_sensor_set_resolution(uint32_t sensor_id, uint32_t resolution);
int hal_temp_sensor_set_sample_rate(uint32_t sensor_id, uint32_t sample_rate);

// Watchdog timer
int hal_watchdog_init(uint32_t timeout_ms);
int hal_watchdog_deinit(void);
int hal_watchdog_feed(void);
int hal_watchdog_enable(bool enable);
int hal_watchdog_get_timeout(uint32_t *timeout_ms);
int hal_watchdog_set_timeout(uint32_t timeout_ms);

// Real Time Clock (RTC)
int hal_rtc_init(void);
int hal_rtc_deinit(void);
int hal_rtc_set_time(uint64_t timestamp);
int hal_rtc_get_time(uint64_t *timestamp);
int hal_rtc_set_alarm(uint64_t alarm_time);
int hal_rtc_get_alarm(uint64_t *alarm_time);
int hal_rtc_enable_alarm(bool enable);
int hal_rtc_read_reg(uint8_t reg_addr, uint8_t *value);
int hal_rtc_write_reg(uint8_t reg_addr, uint8_t value);

// Power management
int hal_power_set_voltage(uint32_t rail_id, uint32_t voltage_mv);
int hal_power_get_voltage(uint32_t rail_id, uint32_t *voltage_mv);
int hal_power_set_current_limit(uint32_t rail_id, uint32_t current_ma);
int hal_power_get_current(uint32_t rail_id, uint32_t *current_ma);
int hal_power_enable_rail(uint32_t rail_id, bool enable);
int hal_power_get_rail_status(uint32_t rail_id, bool *enabled, uint32_t *voltage_mv, uint32_t *current_ma);

// Fan control
int hal_fan_init(uint32_t fan_id, uint32_t pwm_channel, uint32_t tach_pin);
int hal_fan_deinit(uint32_t fan_id);
int hal_fan_set_speed(uint32_t fan_id, uint32_t speed_rpm);
int hal_fan_get_speed(uint32_t fan_id, uint32_t *speed_rpm);
int hal_fan_set_pwm(uint32_t fan_id, uint32_t duty_cycle);  // 0-100%
int hal_fan_enable(uint32_t fan_id, bool enable);
int hal_fan_set_temp_control(uint32_t fan_id, uint32_t temp_sensor_id, uint32_t min_temp, uint32_t max_temp);

// Platform-specific peripheral operations
int hal_peripheral_platform_init(void);
int hal_peripheral_platform_deinit(void);

// Peripheral status monitoring
typedef struct {
    uint32_t gpio_count;
    uint32_t i2c_bus_count;
    uint32_t spi_bus_count;
    uint32_t uart_port_count;
    uint32_t pwm_channel_count;
    uint32_t adc_channel_count;
    uint32_t motor_count;
    uint32_t led_count;
    uint32_t watchdog_active;
    uint32_t rtc_present;
} hal_peripheral_status_t;

int hal_peripheral_get_status(hal_peripheral_status_t *status);

#ifdef __cplusplus
}
#endif

#endif // _HAL_PERIPHERAL_H_