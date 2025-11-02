/**
 * @file config.h
 * @brief 硬件配置和系统参数定义
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============ GPIO引脚定义 ============

// MAX31855热电偶模块 (SPI)
#define THERMO_1_CLK_PIN    2   // SCK
#define THERMO_1_CS_PIN     3   // CS
#define THERMO_1_MISO_PIN   10  // MISO

// 第二个热电偶（如果需要）
#define THERMO_2_CLK_PIN    2   // 共享SCK
#define THERMO_2_CS_PIN     4   // 独立CS
#define THERMO_2_MISO_PIN   10  // 共享MISO

// 加热片控制 (PWM)
#define HEATING_PAD_PIN     5   // 使用MOSFET驱动

// 真空泵控制 (通过L298N)
#define PUMP_PWM_PIN        6   // PWM控制
#define PUMP_DIR_PIN        7   // 方向控制（可选）

// 压力传感器 XGZ6897d (I2C或模拟)
#define PRESSURE_SDA_PIN    8   // I2C数据
#define PRESSURE_SCL_PIN    9   // I2C时钟

// 蜂鸣器
#define BUZZER_PIN          18  // PWM输出

// 按键
#define BUTTON_POWER_PIN    19  // 电源/模式按键（带自锁）
#define BUTTON_UP_PIN       20  // 增加按键
#define BUTTON_DOWN_PIN     21  // 减少按键

// 状态LED（如果有）
#define LED_STATUS_PIN      1   // 内置LED或外部LED

// ============ 系统参数 ============

// 温度控制参数
#define TEMP_TARGET_DEFAULT 37.0f   // 默认目标温度（°C）
#define TEMP_MAX_LIMIT      42.0f   // 最高温度限制
#define TEMP_MIN_LIMIT      30.0f   // 最低温度限制
#define TEMP_HYSTERESIS     0.5f    // 温度回差（°C）

// 压力控制参数
#define PRESSURE_TARGET_DEFAULT -2.0f  // 默认目标压力（kPa，负值表示负压）
#define PRESSURE_MAX_LIMIT  -0.5f      // 最小负压（安全）
#define PRESSURE_MIN_LIMIT  -5.0f      // 最大负压（不超过传感器量程）

// PWM参数
#define PWM_FREQUENCY       5000    // PWM频率 (Hz)
#define PWM_RESOLUTION      8       // PWM分辨率 (8位 = 0-255)
#define PWM_CHANNEL_HEAT    0       // 加热PWM通道
#define PWM_CHANNEL_PUMP    1       // 泵PWM通道
#define PWM_CHANNEL_BUZZER  2       // 蜂鸣器PWM通道

// 安全参数
#define TEMP_EMERGENCY_STOP 45.0f   // 紧急停机温度
#define OVERHEAT_TIMEOUT_MS 3000    // 过热超时时间(ms)

// FreeRTOS任务优先级
#define TASK_PRIORITY_HIGH      3
#define TASK_PRIORITY_NORMAL    2
#define TASK_PRIORITY_LOW       1

// FreeRTOS任务栈大小
#define TASK_STACK_SIZE_LARGE   4096
#define TASK_STACK_SIZE_MEDIUM  2048
#define TASK_STACK_SIZE_SMALL   1024

// 采样周期
#define TEMP_SAMPLE_PERIOD_MS   500    // 温度采样周期
#define PRESSURE_SAMPLE_PERIOD_MS 100  // 压力采样周期
#define CONTROL_UPDATE_PERIOD_MS 200   // 控制更新周期

#endif // CONFIG_H
