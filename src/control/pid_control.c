#include "pid_control.h"
#include "actuators.h"
#include <math.h>

static PID_Controller_t co2_pid;
static PID_Controller_t humid_pid;

/**
 * @brief 计算 PID 输出
 */
static float PID_Compute(PID_Controller_t *pid, float current_val) {
    float error = pid->target - current_val;
    pid->integral += error;

    // 抗积分饱和 (Integral Clamping)
    if (pid->integral > pid->integral_max) pid->integral = pid->integral_max;
    if (pid->integral < -pid->integral_max) pid->integral = -pid->integral_max;

    float derivative = error - pid->prev_error;
    float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

    pid->prev_error = error;

    // 限幅输出
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;

    return output;
}

void PID_Init(void) {
    /* CO2 PID 初始化: 默认参数 Kp=0.8, Ki=0.1, Kd=0.2 */
    co2_pid.Kp = 0.8f;
    co2_pid.Ki = 0.1f;
    co2_pid.Kd = 0.2f;
    co2_pid.target = 1000.0f; // 默认 1000ppm
    co2_pid.integral = 0;
    co2_pid.prev_error = 0;
    co2_pid.integral_max = 500.0f;
    co2_pid.output_max = 1.0f; // 0~1 逻辑输出
    co2_pid.output_min = 0;

    /* 湿度 PID 初始化 */
    humid_pid.Kp = 1.2f;
    humid_pid.Ki = 0.05f;
    humid_pid.Kd = 0.1f;
    humid_pid.target = 60.0f; // 默认 60%RH
    humid_pid.integral = 0;
    humid_pid.prev_error = 0;
    humid_pid.integral_max = 100.0f;
    humid_pid.output_max = 1.0f; // 1代表加湿, -1代表除湿
    humid_pid.output_min = -1.0f;
}

void PID_Set_Targets(float co2_ppm, float humidity_rh) {
    co2_pid.target = co2_ppm;
    humid_pid.target = humidity_rh;
}

void PID_Task_Update(const CO2_Data_t *co2_data, const TH_Data_t *th_data) {
    if (co2_data == NULL || th_data == NULL) return;

    /* 1. CO2 闭环控制 */
    if (co2_data->is_valid) {
        float co2_out = PID_Compute(&co2_pid, co2_data->co2_ppm);
        // PWM 逻辑映射 (这里采用简单的阈值控制，后续可优化为 PWM 时间切片)
        if (co2_out > 0.1f) {
            Actuators_Set_CO2_Valve(ACTUATOR_ON);
        } else {
            Actuators_Set_CO2_Valve(ACTUATOR_OFF);
        }
    }

    /* 2. 湿度闭环控制 (加湿/除湿互锁) */
    if (th_data->is_valid) {
        float hum_out = PID_Compute(&humid_pid, th_data->humidity);

        if (hum_out > 0.1f) {
            // 需要加湿
            Actuators_Set_Humidifier(ACTUATOR_ON);
            Actuators_Set_Dehumidifier(false, (uint16_t)humid_pid.target);
        } else if (hum_out < -0.1f) {
            // 需要除湿
            Actuators_Set_Humidifier(ACTUATOR_OFF);
            Actuators_Set_Dehumidifier(true, (uint16_t)humid_pid.target);
        } else {
            // 维持现状
            Actuators_Set_Humidifier(ACTUATOR_OFF);
            Actuators_Set_Dehumidifier(false, (uint16_t)humid_pid.target);
        }
        
        // 安全联锁: 湿度>90% 强制停止加湿
        if (th_data->humidity > 90.0f) {
            Actuators_Set_Humidifier(ACTUATOR_OFF);
        }
    }
}
