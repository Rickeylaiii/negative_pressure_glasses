/**
 * @file PressureSensor.cpp
 * @brief CPS610DSD003DH01 压力传感器实现
 * 
 * 通信协议:
 * 1. 向0x30寄存器写入0x0A触发采集
 * 2. 等待5-10ms
 * 3. 从0x06-0x08读取24位数据
 * 4. 转换公式: P(kPa) = 7.5 * (raw/8388608) - 3.75
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
    
    Serial.printf("Initializing CPS610DSD003DH01 at address 0x%02X...\n", i2cAddr);
    
    // 检查I2C设备是否存在
    Wire.beginTransmission(i2cAddr);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.printf("X I2C device not found! Error code: %d\n", error);
        Serial.println("  Possible issues:");
        Serial.println("  1. Check I2C wiring (SDA/SCL)");
        Serial.println("  2. Check power supply");
        Serial.println("  3. Verify I2C address is 0x7F");
        return false;
    }
    
    Serial.println("OK I2C device detected");
    
    // 尝试触发一次采集并读取
    if (!startMeasurement()) {
        Serial.println("X Failed to start measurement");
        return false;
    }
    
    delay(10); // 等待采集完成
    
    // 读取初始值
    float pressure = readPressure();
    if (isnan(pressure)) {
        Serial.println("X Failed to read pressure during initialization");
        return false;
    }
    
    Serial.printf("OK CPS610DSD003DH01 initialized, current: %.3f kPa\n", pressure);
    return true;
}

bool PressureSensor::startMeasurement() {
    // 向0x30寄存器写入0x0A触发采集
    Wire.beginTransmission(i2cAddr);
    Wire.write(CMD_REG);      // 寄存器地址 0x30
    Wire.write(CMD_START);    // 命令 0x0A
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        Serial.printf("X Failed to start measurement, I2C error: %d\n", error);
        return false;
    }
    
    return true;
}

int32_t PressureSensor::readRaw24bit() {
    // 从0x06开始读取3个字节
    Wire.beginTransmission(i2cAddr);
    Wire.write(DATA_REG_H);  // 从0x06开始
    uint8_t error = Wire.endTransmission(false);
    
    if (error != 0) {
        Serial.printf("X Failed to set read pointer, error: %d\n", error);
        return 0x7FFFFFFF; // 错误标记
    }
    
    // 请求3个字节: 0x06(H), 0x07(M), 0x08(L)
    Wire.requestFrom(i2cAddr, (uint8_t)3);
    
    if (Wire.available() < 3) {
        Serial.printf("X Not enough data available: %d bytes\n", Wire.available());
        return 0x7FFFFFFF;
    }
    
    uint8_t byteH = Wire.read();  // 高字节 [23:16]
    uint8_t byteM = Wire.read();  // 中字节 [15:8]
    uint8_t byteL = Wire.read();  // 低字节 [7:0]
    
    // 拼接24位数据
    int32_t raw24 = ((int32_t)byteH << 16) | ((int32_t)byteM << 8) | byteL;
    
    // 处理24位有符号数的符号扩展
    // 如果最高位(bit 23)为1,说明是负数,需要扩展符号位
    if (raw24 & 0x800000) {
        raw24 |= 0xFF000000;  // 符号扩展到32位
    }
    
    return raw24;
}

float PressureSensor::convertToPressure(int32_t raw24) {
    if (raw24 == 0x7FFFFFFF) {
        return NAN;  // 错误标记
    }
    
    // CPS610DSD003DH01 转换公式:
    // Code = raw24 / 8388608.0
    // P(kPa) = 7.5 * Code - 3.75
    float code = (float)raw24 / DIVISOR;
    float pressure = COEF_A * code + COEF_B;
    
    return pressure - zeroOffset;
}

float PressureSensor::readPressure() {
    // 触发采集
    if (!startMeasurement()) {
        errorCount++;
        if (errorCount >= MAX_ERROR_COUNT) {
            return NAN;
        }
        return lastPressure;
    }
    
    // 等待采集完成 (5-10ms)
    delay(8);
    
    // 读取原始数据
    int32_t raw24 = readRaw24bit();
    
    // 转换为压力值
    float pressure = convertToPressure(raw24);
    
    if (isnan(pressure)) {
        errorCount++;
        Serial.println("X Pressure read error");
        
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
    Serial.println("Starting zero calibration...");
    Serial.println("Ensure sensor is at atmospheric pressure (no pressure difference)");
    
    float sum = 0.0f;
    int count = 0;
    
    // 采集10个样本并求平均
    for (int i = 0; i < 10; i++) {
        if (!startMeasurement()) {
            Serial.printf("  Sample %d: Failed to trigger\n", i+1);
            continue;
        }
        
        delay(10);
        
        int32_t raw24 = readRaw24bit();
        float pressure = convertToPressure(raw24);
        
        if (!isnan(pressure)) {
            sum += pressure + zeroOffset; // 加回之前的偏移
            count++;
            Serial.printf("  Sample %d: %.3f kPa (raw=%d)\n", i+1, pressure + zeroOffset, raw24);
        } else {
            Serial.printf("  Sample %d: Read failed\n", i+1);
        }
        
        delay(100);
    }
    
    if (count > 0) {
        zeroOffset = sum / count;
        Serial.printf("OK Zero calibration completed\n");
        Serial.printf("   Offset: %.3f kPa (based on %d samples)\n", zeroOffset, count);
        Serial.printf("   Expected: ~0 kPa at atmospheric pressure\n");
    } else {
        Serial.println("X Zero calibration failed - no valid samples");
    }
}

bool PressureSensor::isValid() {
    return (errorCount < MAX_ERROR_COUNT) && !isnan(lastPressure);
}
