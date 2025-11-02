/**
 * @file PumpController.cpp
 * @brief 真空泵控制器实现
 */

#include "PumpController.h"
#include "config.h"

PumpController::PumpController(uint8_t pwm_pin, uint8_t pwm_channel)
    : pwmPin(pwm_pin), pwmChannel(pwm_channel), currentSpeed(0), running(false) {
}

void PumpController::begin() {
    pinMode(pwmPin, OUTPUT);
    
    // 配置PWM
    ledcSetup(pwmChannel, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(pwmPin, pwmChannel);
    ledcWrite(pwmChannel, 0);
    
    Serial.println("泵控制器初始化完成");
}

void PumpController::setSpeed(uint8_t speed) {
    if (speed > 100) speed = 100;
    
    currentSpeed = speed;
    
    if (running) {
        uint8_t pwm_value = map(speed, 0, 100, 0, 255);
        ledcWrite(pwmChannel, pwm_value);
        Serial.printf("泵速度设置为: %d%%\n", speed);
    }
}

void PumpController::start() {
    running = true;
    uint8_t pwm_value = map(currentSpeed, 0, 100, 0, 255);
    ledcWrite(pwmChannel, pwm_value);
    Serial.printf("泵启动，速度: %d%%\n", currentSpeed);
}

void PumpController::stop() {
    running = false;
    ledcWrite(pwmChannel, 0);
    Serial.println("泵停止");
}
