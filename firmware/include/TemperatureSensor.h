/**
 * @file TemperatureSensor.h
 * @brief MAX31855/MAX6675热电偶温度传感器驱动
 */

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <Adafruit_MAX31855.h>

class TemperatureSensor {
public:
    /**
     * @brief 构造函数
     * @param sck_pin SCK引脚
     * @param cs_pin CS引脚
     * @param miso_pin MISO引脚
     */
    TemperatureSensor(uint8_t sck_pin, uint8_t cs_pin, uint8_t miso_pin);
    
    /**
     * @brief 初始化传感器
     * @return true 初始化成功，false 失败
     */
    bool begin();
    
    /**
     * @brief 读取温度
     * @return 温度值（°C），出错返回NAN
     */
    float readTemperature();
    
    /**
     * @brief 读取内部温度（冷端补偿温度）
     * @return 内部温度（°C）
     */
    float readInternalTemperature();
    
    /**
     * @brief 检查传感器是否正常
     * @return true 正常，false 异常
     */
    bool isValid();
    
    /**
     * @brief 获取最后一次读取的温度
     * @return 最后温度值
     */
    float getLastTemperature() const { return lastTemp; }
    
private:
    Adafruit_MAX31855* thermocouple;
    float lastTemp;
    uint8_t errorCount;
    static const uint8_t MAX_ERROR_COUNT = 3;
};

#endif // TEMPERATURE_SENSOR_H
