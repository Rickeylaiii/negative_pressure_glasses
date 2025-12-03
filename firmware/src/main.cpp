/**
 * @file main.cpp
 * @brief 带加热负压眼镜主程序 - FreeRTOS多任务实现
 * 
 * 系统架构：
 * - 温度监控任务：读取 MAX31855 K型热电偶温度，PID控制维持40°C
 * - 压力监控任务：读取 XGZP6897D 气压传感器，PID控制维持15mmHg负压
 * - 用户界面任务：按键处理（UP/DOWN调节负压档位，STOP急停）
 * - 安全监控任务：异常报警（蜂鸣器）
 * 
 * 硬件连接：
 * - GPIO1: 加热片PWM
 * - GPIO2: 负压泵PWM
 * - GPIO4/5/7: MAX31855 (SCK/MISO/CS)
 * - GPIO6: 蜂鸣器PWM (2.731kHz)
 * - GPIO8/9: XGZP6897D (SDA/SCL)
 * - GPIO10: STOP按键（低电平触发急停）
 * - GPIO20: UP按键（增加负压档位）
 * - GPIO21: DOWN按键（减少负压档位）
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

#include "config.h"
#include "TemperatureSensor.h"
#include "PressureSensor.h"
#include "HeatingController.h"
#include "PumpController.h"
#include "Buzzer.h"
#include "Button.h"

// ============ 全局对象 ============
TemperatureSensor* tempSensor;      // MAX31855温度传感器
PressureSensor* pressureSensor;     // XGZP6897D压力传感器
HeatingController* heatingCtrl;     // 加热控制器（PID）
PumpController* pumpCtrl;           // 负压泵控制器（PID）
Buzzer* buzzer;                     // 蜂鸣器
Button* btnStop;                    // 急停按键
Button* btnUp;                      // 增加档位
Button* btnDown;                    // 减少档位

// ============ 互斥锁和信号量 ============
SemaphoreHandle_t xSerialMutex;      // 串口打印互斥锁
SemaphoreHandle_t xTempMutex;        // 温度数据互斥锁
SemaphoreHandle_t xPressureMutex;    // 压力数据互斥锁

// ============ 共享数据 ============
struct SystemState {
    float currentTemp;          // 当前温度 (°C)
    float targetTemp;           // 目标温度 (°C) - 固定40°C
    float currentPressure;      // 当前负压 (mmHg)
    float targetPressure;       // 目标负压 (mmHg)
    uint8_t pressureGear;       // 负压档位 (1-10)
    bool systemEnabled;         // 系统运行状态
    bool emergencyStop;         // 急停状态
    bool overTemp;              // 过温标志
} sysState;

// ============ 任务句柄 ============
TaskHandle_t xTaskTemperatureHandle = NULL;
TaskHandle_t xTaskPressureHandle = NULL;
TaskHandle_t xTaskUIHandle = NULL;
TaskHandle_t xTaskSafetyHandle = NULL;

// ============ 任务函数声明 ============
void taskTemperatureControl(void* parameter);
void taskPressureControl(void* parameter);
void taskUserInterface(void* parameter);
void taskSafetyMonitor(void* parameter);

// ============ 辅助函数 ============
void safePrint(const char* format, ...);
void initializeHardware();
void initializeSystem();

/**
 * @brief Arduino setup函数
 */
void setup() {
    // 使用 USB CDC 串口
    Serial.begin(115200);
    
    // 等待串口连接（最多3秒）
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000)) {
        delay(10);
    }
    delay(500);
    
    Serial.println("\n\n========================================");
    Serial.println("负压眼镜加热系统启动");
    Serial.println("========================================");
    Serial.printf("芯片: %s @ %dMHz\n", ESP.getChipModel(), ESP.getCpuFreqMHz());
    Serial.printf("内存: %d KB\n", ESP.getFreeHeap() / 1024);
    Serial.println("========================================\n");
    
    // 初始化硬件
    initializeHardware();
    
    // 初始化系统状态
    initializeSystem();
    
    // 创建互斥锁
    xSerialMutex = xSemaphoreCreateMutex();
    xTempMutex = xSemaphoreCreateMutex();
    xPressureMutex = xSemaphoreCreateMutex();
    
    // 创建FreeRTOS任务
    xTaskCreatePinnedToCore(
        taskTemperatureControl,           // 温度控制任务（包含PID）
        "Temperature",
        TASK_STACK_SIZE_MEDIUM,
        NULL,
        TASK_PRIORITY_HIGH,
        &xTaskTemperatureHandle,
        0
    );
    
    xTaskCreatePinnedToCore(
        taskPressureControl,              // 压力控制任务（包含PID）
        "Pressure",
        TASK_STACK_SIZE_MEDIUM,
        NULL,
        TASK_PRIORITY_HIGH,
        &xTaskPressureHandle,
        0
    );
    
    xTaskCreatePinnedToCore(
        taskUserInterface,                // 用户界面任务
        "UI",
        TASK_STACK_SIZE_LARGE,
        NULL,
        TASK_PRIORITY_NORMAL,
        &xTaskUIHandle,
        1
    );
    
    xTaskCreatePinnedToCore(
        taskSafetyMonitor,                // 安全监控任务
        "Safety",
        TASK_STACK_SIZE_SMALL,
        NULL,
        TASK_PRIORITY_HIGH,
        &xTaskSafetyHandle,
        1
    );
    
    Serial.println("✓ 所有任务已创建");
    Serial.println("✓ 系统运行中...\n");
    buzzer->beep();  // 启动提示音
}

/**
 * @brief Arduino loop函数（FreeRTOS接管后基本不用）
 */
void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief 初始化所有硬件
 */
void initializeHardware() {
    Serial.println("初始化硬件...");
    
    // 创建传感器对象（使用新的引脚定义）
    tempSensor = new TemperatureSensor(THERMO_CLK_PIN, THERMO_CS_PIN, THERMO_MISO_PIN);
    pressureSensor = new PressureSensor(PRESSURE_SDA_PIN, PRESSURE_SCL_PIN);
    
    // 创建控制器对象
    heatingCtrl = new HeatingController(HEATING_PAD_PIN, PWM_CHANNEL_HEAT);
    pumpCtrl = new PumpController(PUMP_PWM_PIN, PWM_CHANNEL_PUMP);
    buzzer = new Buzzer(BUZZER_PIN, PWM_CHANNEL_BUZZER);
    
    // 创建按键对象
    btnStop = new Button(BUTTON_STOP_PIN);
    btnUp = new Button(BUTTON_UP_PIN);
    btnDown = new Button(BUTTON_DOWN_PIN);
    
    // 初始化传感器
    if (!tempSensor->begin()) {
        Serial.println("⚠ 警告：MAX31855温度传感器初始化失败！");
    } else {
        Serial.println("✓ MAX31855温度传感器就绪");
    }
    
    if (!pressureSensor->begin()) {
        Serial.println("⚠ 警告：XGZP6897D压力传感器初始化失败！");
    } else {
        Serial.println("✓ XGZP6897D压力传感器就绪");
    }
    
    // 初始化控制器
    heatingCtrl->begin();
    pumpCtrl->begin();
    buzzer->begin();
    
    Serial.println("✓ 加热控制器就绪");
    Serial.println("✓ 负压泵控制器就绪");
    Serial.println("✓ 蜂鸣器就绪");
    
    // 初始化按键
    btnStop->begin();
    btnUp->begin();
    btnDown->begin();
    
    Serial.println("✓ 按键初始化完成");
    Serial.println("硬件初始化完成\n");
}

/**
 * @brief 初始化系统状态
 */
void initializeSystem() {
    sysState.currentTemp = 0.0f;
    sysState.targetTemp = TEMP_TARGET_DEFAULT;  // 固定40°C
    sysState.currentPressure = 0.0f;
    sysState.targetPressure = PRESSURE_TARGET_DEFAULT;  // 默认15mmHg
    sysState.pressureGear = 5;  // 默认档位5 (中档)
    sysState.systemEnabled = true;  // 系统默认启动
    sysState.emergencyStop = false;
    sysState.overTemp = false;
    
    heatingCtrl->setTargetTemperature(sysState.targetTemp);
    
    Serial.printf("目标温度: %.1f°C\n", sysState.targetTemp);
    Serial.printf("目标负压: %.1f mmHg\n", sysState.targetPressure);
    Serial.printf("当前档位: %d/10\n", sysState.pressureGear);
}

/**
 * @brief 线程安全的串口打印
 */
void safePrint(const char* format, ...) {
    if (xSemaphoreTake(xSerialMutex, portMAX_DELAY) == pdTRUE) {
        va_list args;
        va_start(args, format);
        char buffer[256];
        vsnprintf(buffer, sizeof(buffer), format, args);
        Serial.print(buffer);
        va_end(args);
        xSemaphoreGive(xSerialMutex);
    }
}

/**
 * @brief 温度控制任务（读取+PID控制）
 */
void taskTemperatureControl(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        // 读取温度
        float temp = tempSensor->readTemperature();
        
        if (!isnan(temp)) {
            // 更新共享数据
            if (xSemaphoreTake(xTempMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                sysState.currentTemp = temp;
                xSemaphoreGive(xTempMutex);
            }
            
            // PID温度控制（如果系统运行且未急停）
            if (sysState.systemEnabled && !sysState.emergencyStop) {
                uint8_t output = heatingCtrl->update(temp);
                
                // 定期打印控制状态
                static uint32_t lastPrintTime = 0;
                if (millis() - lastPrintTime > 5000) {
                    safePrint("[温度] 当前: %.1f°C, 目标: %.1f°C, 功率: %.0f%%\n",
                             temp, sysState.targetTemp, heatingCtrl->getPowerPercent());
                    lastPrintTime = millis();
                }
            } else {
                // 系统停止，关闭加热
                heatingCtrl->disable();
            }
            
            // 过温检测
            if (temp >= TEMP_EMERGENCY_STOP) {
                sysState.overTemp = true;
                sysState.emergencyStop = true;
                heatingCtrl->emergencyStop();
                safePrint("[紧急] 温度过高！%.2f°C\n", temp);
            }
        } else {
            safePrint("[错误] 温度读取失败\n");
        }
        
        // 周期性休眠
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TEMP_SAMPLE_PERIOD_MS));
    }
}

/**
 * @brief 压力控制任务（读取+PID控制）
 */
void taskPressureControl(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        // 读取压力
        float pressure = pressureSensor->readPressure();
        
        if (!isnan(pressure)) {
            // 更新共享数据
            if (xSemaphoreTake(xPressureMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                sysState.currentPressure = pressure;
                xSemaphoreGive(xPressureMutex);
            }
            
            // PID压力控制（如果系统运行且未急停）
            if (sysState.systemEnabled && !sysState.emergencyStop) {
                // 根据档位计算目标压力 (10% - 100%)
                float gearPercent = (float)sysState.pressureGear / (float)PRESSURE_NUM_GEARS;
                sysState.targetPressure = PRESSURE_TARGET_DEFAULT * gearPercent;
                
                // 简单的压力控制（可改进为PID）
                float error = sysState.targetPressure - pressure;
                
                if (error > 2.0f) {
                    // 实际压力小于目标，需要增加泵速
                    pumpCtrl->setSpeed(80);
                } else if (error < -2.0f) {
                    // 实际压力大于目标，减小泵速
                    pumpCtrl->setSpeed(40);
                } else {
                    // 维持当前压力
                    pumpCtrl->setSpeed(60);
                }
                
                // 定期打印压力状态
                static uint32_t lastPrintTime = 0;
                if (millis() - lastPrintTime > 5000) {
                    safePrint("[压力] 当前: %.1f mmHg, 目标: %.1f mmHg, 档位: %d\n",
                             pressure, sysState.targetPressure, sysState.pressureGear);
                    lastPrintTime = millis();
                }
            } else {
                // 系统停止，关闭泵
                pumpCtrl->stop();
            }
        } else {
            safePrint("[错误] 压力读取失败\n");
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PRESSURE_SAMPLE_PERIOD_MS));
    }
}

/**
 * @brief 用户界面任务（按键处理）
 */
void taskUserInterface(void* parameter) {
    while (1) {
        // 更新按键状态
        btnStop->update();
        btnUp->update();
        btnDown->update();
        
        // STOP按键 - 急停（低电平触发）
        if (btnStop->isPressed()) {
            if (!sysState.emergencyStop) {
                sysState.emergencyStop = true;
                sysState.systemEnabled = false;
                heatingCtrl->emergencyStop();
                pumpCtrl->stop();
                buzzer->warning();
                safePrint("[系统] 急停触发！\n");
            }
        } else {
            // STOP按键松开，可以恢复运行
            if (sysState.emergencyStop && !sysState.overTemp) {
                sysState.emergencyStop = false;
                sysState.systemEnabled = true;
                heatingCtrl->enable();
                pumpCtrl->start();
                buzzer->beep();
                safePrint("[系统] 急停解除，系统恢复运行\n");
            }
        }
        
        // UP按键 - 增加负压档位
        if (btnUp->wasPressed()) {
            if (sysState.pressureGear < PRESSURE_NUM_GEARS) {
                sysState.pressureGear++;
                buzzer->beep();
                safePrint("[设置] 档位增加: %d/10 (%.0f%%)\n", 
                         sysState.pressureGear, 
                         (float)sysState.pressureGear * 10.0f);
            } else {
                buzzer->warning();  // 已达最大档位
                safePrint("[设置] 已达最大档位: %d/10\n", sysState.pressureGear);
            }
        }
        
        // DOWN按键 - 减少负压档位
        if (btnDown->wasPressed()) {
            if (sysState.pressureGear > 1) {
                sysState.pressureGear--;
                buzzer->beep();
                safePrint("[设置] 档位减少: %d/10 (%.0f%%)\n", 
                         sysState.pressureGear,
                         (float)sysState.pressureGear * 10.0f);
            } else {
                buzzer->warning();  // 已达最小档位
                safePrint("[设置] 已达最小档位: %d/10\n", sysState.pressureGear);
            }
        }
        
        // 定期打印系统状态
        static uint32_t lastStatusTime = 0;
        if (millis() - lastStatusTime > 10000) {
            safePrint("\n=== 系统状态 ===\n");
            safePrint("温度: %.1f°C (目标: %.1f°C)\n", sysState.currentTemp, sysState.targetTemp);
            safePrint("负压: %.1f mmHg (目标: %.1f mmHg)\n", sysState.currentPressure, sysState.targetPressure);
            safePrint("档位: %d/10 (%.0f%%)\n", sysState.pressureGear, (float)sysState.pressureGear * 10.0f);
            safePrint("状态: %s\n", sysState.systemEnabled ? "运行中" : "已停止");
            safePrint("急停: %s\n", sysState.emergencyStop ? "是" : "否");
            safePrint("================\n\n");
            lastStatusTime = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 按键扫描周期
    }
}

/**
 * @brief 安全监控任务
 */
void taskSafetyMonitor(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        // 检查过温状态
        if (sysState.overTemp) {
            // 持续报警
            buzzer->error();
            safePrint("[报警] 系统过温！当前温度: %.1f°C\n", sysState.currentTemp);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        // 检查急停状态
        if (sysState.emergencyStop && !sysState.overTemp) {
            // 急停状态下短促报警
            static uint32_t lastBeep = 0;
            if (millis() - lastBeep > 2000) {
                buzzer->beep();
                lastBeep = millis();
            }
        }
        
        // 检查传感器异常
        if (isnan(sysState.currentTemp) || isnan(sysState.currentPressure)) {
            static uint32_t lastWarn = 0;
            if (millis() - lastWarn > 5000) {
                buzzer->warning();
                safePrint("[警告] 传感器读取异常\n");
                lastWarn = millis();
            }
        }
        
        // TODO: 添加更多安全检查
        // - 压力传感器断线检测
        // - 加热片短路检测
        // - 泵电流异常检测
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}
