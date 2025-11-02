/**
 * @file HeatingController.h
 * @brief 加热片控制器（PID控制）
 */

#ifndef HEATING_CONTROLLER_H
#define HEATING_CONTROLLER_H

#include <Arduino.h>

class HeatingController {
public:
    /**
     * @brief 构造函数
     * @param heating_pin 加热片控制引脚
     * @param pwm_channel PWM通道
     */
    HeatingController(uint8_t heating_pin, uint8_t pwm_channel);
    
    /**
     * @brief 初始化控制器
     */
    void begin();
    
    /**
     * @brief 设置目标温度
     * @param target 目标温度（°C）
     */
    void setTargetTemperature(float target);
    
    /**
     * @brief PID控制更新
     * @param current_temp 当前温度（°C）
     * @return 输出PWM占空比（0-255）
     */
    uint8_t update(float current_temp);
    
    /**
     * @brief 启用加热
     */
    void enable();
    
    /**
     * @brief 禁用加热
     */
    void disable();
    
    /**
     * @brief 紧急停止（过温保护）
     */
    void emergencyStop();
    
    /**
     * @brief 获取当前输出功率百分比
     * @return 功率百分比（0-100）
     */
    float getPowerPercent() const { return (currentOutput / 255.0f) * 100.0f; }
    
    /**
     * @brief 获取是否启用
     */
    bool isEnabled() const { return enabled; }
    
private:
    uint8_t heatingPin;
    uint8_t pwmChannel;
    float targetTemp;
    float lastError;
    float integral;
    uint8_t currentOutput;
    bool enabled;
    
    // PID参数
    float kp = 25.0f;   // 比例系数
    float ki = 0.5f;    // 积分系数
    float kd = 5.0f;    // 微分系数
    
    // 限幅
    static const float INTEGRAL_MAX = 100.0f;
    static const uint8_t OUTPUT_MIN = 0;
    static const uint8_t OUTPUT_MAX = 255;
    
    /**
     * @brief PID计算
     * @param error 误差
     * @param dt 时间间隔（秒）
     * @return 控制输出
     */
    float calculatePID(float error, float dt);
};

#endif // HEATING_CONTROLLER_H
