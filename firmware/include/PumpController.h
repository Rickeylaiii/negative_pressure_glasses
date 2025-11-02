/**
 * @file PumpController.h
 * @brief 真空泵控制器
 */

#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include <Arduino.h>

class PumpController {
public:
    /**
     * @brief 构造函数
     * @param pwm_pin PWM控制引脚
     * @param pwm_channel PWM通道
     */
    PumpController(uint8_t pwm_pin, uint8_t pwm_channel);
    
    /**
     * @brief 初始化控制器
     */
    void begin();
    
    /**
     * @brief 设置泵速（0-100%）
     * @param speed 速度百分比
     */
    void setSpeed(uint8_t speed);
    
    /**
     * @brief 启动泵
     */
    void start();
    
    /**
     * @brief 停止泵
     */
    void stop();
    
    /**
     * @brief 获取当前速度
     * @return 速度百分比（0-100）
     */
    uint8_t getSpeed() const { return currentSpeed; }
    
    /**
     * @brief 获取是否运行
     */
    bool isRunning() const { return running; }
    
private:
    uint8_t pwmPin;
    uint8_t pwmChannel;
    uint8_t currentSpeed;
    bool running;
};

#endif // PUMP_CONTROLLER_H
