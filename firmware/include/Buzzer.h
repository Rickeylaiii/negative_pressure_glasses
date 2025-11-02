/**
 * @file Buzzer.h
 * @brief 蜂鸣器控制
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
public:
    /**
     * @brief 构造函数
     * @param pin 蜂鸣器引脚
     * @param pwm_channel PWM通道
     */
    Buzzer(uint8_t pin, uint8_t pwm_channel);
    
    /**
     * @brief 初始化
     */
    void begin();
    
    /**
     * @brief 播放音调
     * @param frequency 频率（Hz）
     * @param duration 持续时间（ms），0表示持续播放
     */
    void tone(uint16_t frequency, uint32_t duration = 0);
    
    /**
     * @brief 停止播放
     */
    void noTone();
    
    /**
     * @brief 短促提示音
     */
    void beep();
    
    /**
     * @brief 警告音
     */
    void warning();
    
    /**
     * @brief 错误音
     */
    void error();
    
private:
    uint8_t buzzerPin;
    uint8_t pwmChannel;
};

#endif // BUZZER_H
