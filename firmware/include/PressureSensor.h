/**
 * @file PressureSensor.h
 * @brief XGZ6897d压力传感器驱动（±5kPa）
 */

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

class PressureSensor {
public:
    /**
     * @brief 构造函数
     * @param sda_pin I2C SDA引脚
     * @param scl_pin I2C SCL引脚
     * @param i2c_addr I2C地址（默认0x6D）
     */
    PressureSensor(uint8_t sda_pin, uint8_t scl_pin, uint8_t i2c_addr = 0x6D);
    
    /**
     * @brief 初始化传感器
     * @return true 初始化成功，false 失败
     */
    bool begin();
    
    /**
     * @brief 读取压力值
     * @return 压力值（kPa），出错返回NAN
     */
    float readPressure();
    
    /**
     * @brief 校准零点（在大气压下调用）
     */
    void calibrateZero();
    
    /**
     * @brief 检查传感器是否正常
     * @return true 正常，false 异常
     */
    bool isValid();
    
    /**
     * @brief 获取最后一次读取的压力
     * @return 最后压力值
     */
    float getLastPressure() const { return lastPressure; }
    
private:
    uint8_t sdaPin;
    uint8_t sclPin;
    uint8_t i2cAddr;
    float lastPressure;
    float zeroOffset;
    uint8_t errorCount;
    static const uint8_t MAX_ERROR_COUNT = 3;
    
    /**
     * @brief 从传感器读取原始数据
     * @return 原始ADC值
     */
    uint16_t readRawData();
    
    /**
     * @brief 将原始数据转换为压力值
     * @param raw 原始ADC值
     * @return 压力值（kPa）
     */
    float convertToPressure(uint16_t raw);
};

#endif // PRESSURE_SENSOR_H
