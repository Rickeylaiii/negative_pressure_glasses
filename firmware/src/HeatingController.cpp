/**
 * @file HeatingController.cpp
 * @brief 加热控制器实现
 */

#include "HeatingController.h"
#include "config.h"

HeatingController::HeatingController(uint8_t heating_pin, uint8_t pwm_channel)
    : heatingPin(heating_pin), pwmChannel(pwm_channel),
      targetTemp(TEMP_TARGET_DEFAULT), lastError(0.0f), integral(0.0f),
      currentOutput(0), enabled(false) {
}

void HeatingController::begin() {
    pinMode(heatingPin, OUTPUT);
    
    // 配置PWM
    ledcSetup(pwmChannel, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(heatingPin, pwmChannel);
    ledcWrite(pwmChannel, 0);
    
    Serial.println("加热控制器初始化完成");
}

void HeatingController::setTargetTemperature(float target) {
    if (target >= TEMP_MIN_LIMIT && target <= TEMP_MAX_LIMIT) {
        targetTemp = target;
        Serial.printf("设置目标温度: %.1f°C\n", target);
        
        // 重置PID
        integral = 0.0f;
        lastError = 0.0f;
    } else {
        Serial.printf("温度超出范围: %.1f°C\n", target);
    }
}

float HeatingController::calculatePID(float error, float dt) {
    // 比例项
    float p = kp * error;
    
    // 积分项（带限幅）
    integral += error * dt;
    if (integral > INTEGRAL_MAX) integral = INTEGRAL_MAX;
    if (integral < -INTEGRAL_MAX) integral = -INTEGRAL_MAX;
    float i = ki * integral;
    
    // 微分项
    float d = kd * (error - lastError) / dt;
    lastError = error;
    
    // 总输出
    float output = p + i + d;
    
    return output;
}

uint8_t HeatingController::update(float current_temp) {
    if (!enabled) {
        currentOutput = 0;
        ledcWrite(pwmChannel, 0);
        return 0;
    }
    
    // 安全检查
    if (current_temp >= TEMP_EMERGENCY_STOP) {
        emergencyStop();
        Serial.println("温度过高！紧急停止加热！");
        return 0;
    }
    
    // 计算误差
    float error = targetTemp - current_temp;
    
    // PID控制（假设更新周期为CONTROL_UPDATE_PERIOD_MS）
    float dt = CONTROL_UPDATE_PERIOD_MS / 1000.0f;
    float output = calculatePID(error, dt);
    
    // 限幅
    if (output < OUTPUT_MIN) output = OUTPUT_MIN;
    if (output > OUTPUT_MAX) output = OUTPUT_MAX;
    
    currentOutput = (uint8_t)output;
    ledcWrite(pwmChannel, currentOutput);
    
    return currentOutput;
}

void HeatingController::enable() {
    enabled = true;
    // 重置PID
    integral = 0.0f;
    lastError = 0.0f;
    Serial.println("加热已启用");
}

void HeatingController::disable() {
    enabled = false;
    currentOutput = 0;
    ledcWrite(pwmChannel, 0);
    Serial.println("加热已禁用");
}

void HeatingController::emergencyStop() {
    enabled = false;
    currentOutput = 0;
    ledcWrite(pwmChannel, 0);
    integral = 0.0f;
    lastError = 0.0f;
    Serial.println("紧急停止！");
}
