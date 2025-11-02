/**
 * @file TemperatureSensor.cpp
 * @brief 温度传感器实现
 */

#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t miso_pin)
    : lastTemp(0.0f), errorCount(0) {
    thermocouple = new Adafruit_MAX31855(sck_pin, cs_pin, miso_pin);
}

bool TemperatureSensor::begin() {
    delay(500); // MAX31855需要启动时间
    
    // 尝试读取一次测试
    float temp = thermocouple->readCelsius();
    if (isnan(temp)) {
        Serial.println("温度传感器初始化失败！");
        return false;
    }
    
    lastTemp = temp;
    Serial.printf("温度传感器初始化成功，当前温度: %.2f°C\n", temp);
    return true;
}

float TemperatureSensor::readTemperature() {
    float temp = thermocouple->readCelsius();
    
    if (isnan(temp)) {
        errorCount++;
        Serial.println("温度读取错误！");
        
        // 如果连续多次错误，返回NAN
        if (errorCount >= MAX_ERROR_COUNT) {
            return NAN;
        }
        // 否则返回上次有效值
        return lastTemp;
    }
    
    errorCount = 0;
    lastTemp = temp;
    return temp;
}

float TemperatureSensor::readInternalTemperature() {
    return thermocouple->readInternal();
}

bool TemperatureSensor::isValid() {
    return (errorCount < MAX_ERROR_COUNT) && !isnan(lastTemp);
}
