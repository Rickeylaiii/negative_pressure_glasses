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
    
    Serial.println("Heating Controller Init Success");
}

void HeatingController::setTargetTemperature(float target) {
    if (target >= TEMP_MIN_LIMIT && target <= TEMP_MAX_LIMIT) {
        targetTemp = target;
        Serial.printf("Set Target Temperature: %.1fC\n", target);
        
        // Reset PID
        integral = 0.0f;
        lastError = 0.0f;
    } else {
        Serial.printf("Temperature out of range: %.1fC\n", target);
    }
}

float HeatingController::calculatePID(float error, float dt) {
    // Proportional term
    float p = kp * error;
    
    // Integral term (with clamping)
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
        Serial.println("Temperature too high! Emergency stop heating!");
        return 0;
    }
    
    // Calculate error
    float error = targetTemp - current_temp;
    
    // 修正: 使用实际调用间隔,而不是配置中的固定值
    static uint32_t lastUpdateTime = 0;
    uint32_t currentTime = millis();
    float dt = (currentTime - lastUpdateTime) / 1000.0f;
    
    // 首次调用,使用默认1秒
    if (lastUpdateTime == 0) {
        dt = 1.0f;
    }
    lastUpdateTime = currentTime;
    
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
    Serial.println("Heating Enabled");
}

void HeatingController::disable() {
    enabled = false;
    currentOutput = 0;
    ledcWrite(pwmChannel, 0);
    Serial.println("Heating Disabled");
}

void HeatingController::emergencyStop() {
    enabled = false;
    currentOutput = 0;
    ledcWrite(pwmChannel, 0);
    integral = 0.0f;
    lastError = 0.0f;
    Serial.println("Emergency Stop!");
}

void HeatingController::reset() {
    integral = 0.0f;
    lastError = 0.0f;
    currentOutput = 0;
    Serial.println("PID reset");
}

void HeatingController::setPID(float p, float i, float d) {
    kp = p;
    ki = i;
    kd = d;
    Serial.printf("PID updated: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", kp, ki, kd);
    // Reset integral when changing parameters
    integral = 0.0f;
}
