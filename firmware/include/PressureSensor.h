/**
 * @file PressureSensor.h
 * @brief CPS610DSD003DH01压力传感器驱动（-3kPa ~ +3kPa）
 * 
 * 通信协议:
 * - I2C地址: 0x7F
 * - 命令寄存器: 0x30
 * - 数据寄存器: 0x06-0x08 (24位)
 * - 转换公式: P(kPa) = 7.5 * Code - 3.75
 *   其中 Code = P_raw / 8388608.0
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
     * @param i2c_addr I2C地址（默认0x7F - CPS610DSD003DH01）
     */
    PressureSensor(uint8_t sda_pin, uint8_t scl_pin, uint8_t i2c_addr = 0x7F);
    
    /**
     * @brief 初始化传感器
     * @return true 初始化成功，false 失败
     */
    bool begin();
    
    /**
     * @brief 读取压力值
     * @return 压力值（kPa），出错返回NAN
     * @note 量程: -3.0 ~ +3.0 kPa
     */
    float readPressure();
    
    /**
     * @brief 触发单次采集
     * @return true 成功，false 失败
     */
    bool startMeasurement();
    
    /**
     * @brief 读取原始24位数据
     * @return 原始压力值（有符号24位整数）
     */
    int32_t readRaw24bit();
    
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
    
    // CPS610DSD003DH01 寄存器地址
    static const uint8_t CMD_REG = 0x30;      // 命令/状态寄存器
    static const uint8_t DATA_REG_H = 0x06;   // 数据高字节
    static const uint8_t DATA_REG_M = 0x07;   // 数据中字节
    static const uint8_t DATA_REG_L = 0x08;   // 数据低字节
    
    // CPS610DSD003DH01 命令
    static const uint8_t CMD_START = 0x0A;    // 启动采集
    static const uint8_t CMD_DONE = 0x02;     // 采集完成标志
    
    // CPS610DSD003DH01 计算参数
    static constexpr float COEF_A = 7.5f;      // 传递函数系数A
    static constexpr float COEF_B = -3.75f;    // 传递函数系数B
    static constexpr float DIVISOR = 8388608.0f; // 2^23
    
    /**
     * @brief 将24位原始数据转换为压力值
     * @param raw24 原始24位数据
     * @return 压力值（kPa）
     */
    float convertToPressure(int32_t raw24);
};

#endif // PRESSURE_SENSOR_H
