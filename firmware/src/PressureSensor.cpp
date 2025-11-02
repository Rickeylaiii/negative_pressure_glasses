/**
 * @file PressureSensor.cpp
 * @brief 压力传感器实现
 */

#include "PressureSensor.h"

PressureSensor::PressureSensor(uint8_t sda_pin, uint8_t scl_pin, uint8_t i2c_addr)
    : sdaPin(sda_pin), sclPin(scl_pin), i2cAddr(i2c_addr), 
      lastPressure(0.0f), zeroOffset(0.0f), errorCount(0) {
}

bool PressureSensor::begin() {
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(100000); // 100kHz I2C时钟
    
    delay(100);
    
    // 尝试读取传感器
    Wire.beginTransmission(i2cAddr);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.printf("压力传感器初始化失败！I2C错误代码: %d\n", error);
        return false;
    }
    
    // 读取初始值
    float pressure = readPressure();
    if (isnan(pressure)) {
        Serial.println("压力传感器读取失败！");
        return false;
    }
    
    Serial.printf("压力传感器初始化成功，当前压力: %.2f kPa\n", pressure);
    return true;
}

uint16_t PressureSensor::readRawData() {
    Wire.beginTransmission(i2cAddr);
    Wire.write(0x00); // 读取命令（根据实际传感器调整）
    uint8_t error = Wire.endTransmission(false);
    
    if (error != 0) {
        return 0xFFFF; // 错误标记
    }
    
    Wire.requestFrom(i2cAddr, (uint8_t)2);
    
    if (Wire.available() >= 2) {
        uint16_t raw = Wire.read() << 8;
        raw |= Wire.read();
        return raw;
    }
    
    return 0xFFFF; // 错误标记
}

float PressureSensor::convertToPressure(uint16_t raw) {
    if (raw == 0xFFFF) {
        return NAN;
    }
    
    // XGZ6897d 转换公式（根据实际数据手册调整）
    // 假设：0-16384对应-5kPa到+5kPa
    // 实际公式需要根据传感器数据手册调整
    float pressure = ((float)raw / 16384.0f) * 10.0f - 5.0f;
    
    return pressure - zeroOffset;
}

float PressureSensor::readPressure() {
    uint16_t raw = readRawData();
    float pressure = convertToPressure(raw);
    
    if (isnan(pressure)) {
        errorCount++;
        Serial.println("压力读取错误！");
        
        if (errorCount >= MAX_ERROR_COUNT) {
            return NAN;
        }
        return lastPressure;
    }
    
    errorCount = 0;
    lastPressure = pressure;
    return pressure;
}

void PressureSensor::calibrateZero() {
    Serial.println("开始压力传感器零点校准...");
    
    float sum = 0.0f;
    int count = 0;
    
    // 采集10次取平均
    for (int i = 0; i < 10; i++) {
        uint16_t raw = readRawData();
        float pressure = convertToPressure(raw);
        
        if (!isnan(pressure)) {
            sum += pressure + zeroOffset; // 加回之前的偏移
            count++;
        }
        delay(100);
    }
    
    if (count > 0) {
        zeroOffset = sum / count;
        Serial.printf("零点校准完成，偏移值: %.3f kPa\n", zeroOffset);
    } else {
        Serial.println("零点校准失败！");
    }
}

bool PressureSensor::isValid() {
    return (errorCount < MAX_ERROR_COUNT) && !isnan(lastPressure);
}
