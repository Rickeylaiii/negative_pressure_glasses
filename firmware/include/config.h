/**
 * @file config.h
 * @brief 硬件配置和系统参数定义
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============ GPIO引脚定义 ============
// 根据 README.md 更新的引脚映射

// 状态LED
#define LED1_PIN            0   // LED2 (GPIO0)
#define LED2_PIN            3   // LED1 (GPIO3)

// 加热片控制 (PWM)
#define HEATING_PAD_PIN     1   // GPIO1 PWM控制加热丝

// 真空泵控制 (PWM)
#define PUMP_PWM_PIN        2   // GPIO2 PWM控制负压泵

// MAX31855热电偶模块 (SPI)
#define THERMO_CLK_PIN      4   // GPIO4 SCK
#define THERMO_MISO_PIN     5   // GPIO5 MISO
#define THERMO_CS_PIN       7   // GPIO7 CS

// 蜂鸣器 (PWM, 2.731kHz)
#define BUZZER_PIN          6   // GPIO6 PWM输出

// 压力传感器 XGZP6897D (I2C)
#define PRESSURE_SDA_PIN    8   // GPIO8 I2C SDA
#define PRESSURE_SCL_PIN    9   // GPIO9 I2C SCL

// 按键
#define BUTTON_STOP_PIN     10  // GPIO10 急停按键 (原为GPIO11，但GPIO11保留，用GPIO10)
#define BUTTON_UP_PIN       20  // GPIO20 增加负压档位
#define BUTTON_DOWN_PIN     21  // GPIO21 减少负压档位

// ============ 系统参数 ============

// 温度控制参数
#define TEMP_TARGET_DEFAULT 40.0f   // 默认目标温度（°C）- 固定40度
#define TEMP_MAX_LIMIT      50.0f   // 最高温度限制
#define TEMP_MIN_LIMIT      35.0f   // 最低温度限制
#define TEMP_HYSTERESIS     0.5f    // 温度回差（°C）

// 压力控制参数（负压）
#define PRESSURE_TARGET_DEFAULT 15.0f  // 默认目标负压（mmHg）- 固定15mmHg
#define PRESSURE_MIN_GEAR   10.0f      // 最小档位负压 (mmHg)
#define PRESSURE_MAX_GEAR   100.0f     // 最大档位负压 (mmHg)
#define PRESSURE_GEAR_STEP  10.0f      // 每档增减 10%
#define PRESSURE_NUM_GEARS  10         // 总共10档

// PWM参数
#define PWM_FREQUENCY       5000    // PWM频率 (Hz) - 加热和泵
#define PWM_BUZZER_FREQ     2731    // 蜂鸣器频率 (Hz) - 2.731kHz
#define PWM_RESOLUTION      8       // PWM分辨率 (8位 = 0-255)
#define PWM_CHANNEL_HEAT    0       // 加热PWM通道
#define PWM_CHANNEL_PUMP    1       // 泵PWM通道
#define PWM_CHANNEL_BUZZER  2       // 蜂鸣器PWM通道

// 安全参数
#define TEMP_EMERGENCY_STOP 50.0f   // 紧急停机温度
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
