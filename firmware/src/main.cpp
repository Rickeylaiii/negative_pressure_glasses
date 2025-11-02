/**
 * @file main.cpp
 * @brief 带加热负压眼镜主程序 - FreeRTOS多任务实现
 * 
 * 系统架构：
 * - 温度监控任务：读取热电偶温度，检测过温
 * - 压力监控任务：读取压力传感器，调节真空泵
 * - 加热控制任务：PID温度控制
 * - 用户界面任务：按键处理和蜂鸣器反馈
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
TemperatureSensor* tempSensor1;
TemperatureSensor* tempSensor2; // 可选第二个温度传感器
PressureSensor* pressureSensor;
HeatingController* heatingCtrl;
PumpController* pumpCtrl;
Buzzer* buzzer;
Button* btnPower;
Button* btnUp;
Button* btnDown;

// ============ 互斥锁和信号量 ============
SemaphoreHandle_t xSerialMutex;      // 串口打印互斥锁
SemaphoreHandle_t xTempMutex;        // 温度数据互斥锁
SemaphoreHandle_t xPressureMutex;    // 压力数据互斥锁

// ============ 共享数据 ============
struct SystemState {
    float currentTemp;
    float targetTemp;
    float currentPressure;
    float targetPressure;
    bool heatingEnabled;
    bool pumpEnabled;
    bool emergencyStop;
} sysState;

// ============ 任务句柄 ============
TaskHandle_t xTaskTemperatureHandle = NULL;
TaskHandle_t xTaskPressureHandle = NULL;
TaskHandle_t xTaskHeatingHandle = NULL;
TaskHandle_t xTaskUIHandle = NULL;

// ============ 任务函数声明 ============
void taskTemperatureMonitor(void* parameter);
void taskPressureMonitor(void* parameter);
void taskHeatingControl(void* parameter);
void taskUserInterface(void* parameter);

// ============ 辅助函数 ============
void safePrint(const char* format, ...);
void initializeHardware();
void initializeSystem();

/**
 * @brief Arduino setup函数
 */
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n========================================");
    Serial.println("FSYX OPAP 带加热负压眼镜系统启动");
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
        taskTemperatureMonitor,           // 任务函数
        "Temperature",                     // 任务名称
        TASK_STACK_SIZE_MEDIUM,           // 栈大小
        NULL,                              // 参数
        TASK_PRIORITY_HIGH,               // 优先级（高）
        &xTaskTemperatureHandle,          // 任务句柄
        0                                  // CPU核心0
    );
    
    xTaskCreatePinnedToCore(
        taskPressureMonitor,
        "Pressure",
        TASK_STACK_SIZE_MEDIUM,
        NULL,
        TASK_PRIORITY_HIGH,
        &xTaskPressureHandle,
        0
    );
    
    xTaskCreatePinnedToCore(
        taskHeatingControl,
        "Heating",
        TASK_STACK_SIZE_MEDIUM,
        NULL,
        TASK_PRIORITY_NORMAL,
        &xTaskHeatingHandle,
        1                                  // CPU核心1
    );
    
    xTaskCreatePinnedToCore(
        taskUserInterface,
        "UI",
        TASK_STACK_SIZE_LARGE,
        NULL,
        TASK_PRIORITY_LOW,
        &xTaskUIHandle,
        1
    );
    
    Serial.println("所有任务已创建，系统运行中...\n");
    buzzer->beep();
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
    
    // 创建传感器对象
    tempSensor1 = new TemperatureSensor(THERMO_1_CLK_PIN, THERMO_1_CS_PIN, THERMO_1_MISO_PIN);
    tempSensor2 = new TemperatureSensor(THERMO_2_CLK_PIN, THERMO_2_CS_PIN, THERMO_2_MISO_PIN);
    pressureSensor = new PressureSensor(PRESSURE_SDA_PIN, PRESSURE_SCL_PIN);
    
    // 创建控制器对象
    heatingCtrl = new HeatingController(HEATING_PAD_PIN, PWM_CHANNEL_HEAT);
    pumpCtrl = new PumpController(PUMP_PWM_PIN, PWM_CHANNEL_PUMP);
    buzzer = new Buzzer(BUZZER_PIN, PWM_CHANNEL_BUZZER);
    
    // 创建按键对象
    btnPower = new Button(BUTTON_POWER_PIN);
    btnUp = new Button(BUTTON_UP_PIN);
    btnDown = new Button(BUTTON_DOWN_PIN);
    
    // 初始化
    if (!tempSensor1->begin()) {
        Serial.println("警告：温度传感器1初始化失败！");
    }
    
    if (!pressureSensor->begin()) {
        Serial.println("警告：压力传感器初始化失败！");
    }
    
    heatingCtrl->begin();
    pumpCtrl->begin();
    buzzer->begin();
    
    btnPower->begin();
    btnUp->begin();
    btnDown->begin();
    
    Serial.println("硬件初始化完成\n");
}

/**
 * @brief 初始化系统状态
 */
void initializeSystem() {
    sysState.currentTemp = 0.0f;
    sysState.targetTemp = TEMP_TARGET_DEFAULT;
    sysState.currentPressure = 0.0f;
    sysState.targetPressure = PRESSURE_TARGET_DEFAULT;
    sysState.heatingEnabled = false;
    sysState.pumpEnabled = false;
    sysState.emergencyStop = false;
    
    heatingCtrl->setTargetTemperature(sysState.targetTemp);
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
 * @brief 温度监控任务
 */
void taskTemperatureMonitor(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        // 读取温度
        float temp = tempSensor1->readTemperature();
        
        if (!isnan(temp)) {
            // 更新共享数据
            if (xSemaphoreTake(xTempMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                sysState.currentTemp = temp;
                xSemaphoreGive(xTempMutex);
            }
            
            // 安全检查
            if (temp >= TEMP_EMERGENCY_STOP) {
                sysState.emergencyStop = true;
                heatingCtrl->emergencyStop();
                buzzer->error();
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
 * @brief 压力监控任务
 */
void taskPressureMonitor(void* parameter) {
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
            
            // 简单的压力控制（可改进为PID）
            if (sysState.pumpEnabled) {
                float error = sysState.targetPressure - pressure;
                
                if (error < -0.5f) {
                    // 压力太高（负压太小），增加泵速
                    pumpCtrl->setSpeed(80);
                } else if (error > 0.5f) {
                    // 压力太低（负压太大），减小泵速
                    pumpCtrl->setSpeed(40);
                } else {
                    // 维持当前压力
                    pumpCtrl->setSpeed(60);
                }
            }
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PRESSURE_SAMPLE_PERIOD_MS));
    }
}

/**
 * @brief 加热控制任务
 */
void taskHeatingControl(void* parameter) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        if (sysState.heatingEnabled && !sysState.emergencyStop) {
            float temp;
            
            // 读取当前温度
            if (xSemaphoreTake(xTempMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                temp = sysState.currentTemp;
                xSemaphoreGive(xTempMutex);
            } else {
                temp = 0.0f;
            }
            
            // 执行PID控制
            uint8_t output = heatingCtrl->update(temp);
            
            // 定期打印控制状态
            static uint32_t lastPrintTime = 0;
            if (millis() - lastPrintTime > 5000) {
                safePrint("[加热] 当前: %.1f°C, 目标: %.1f°C, 功率: %.0f%%\n",
                         temp, sysState.targetTemp, heatingCtrl->getPowerPercent());
                lastPrintTime = millis();
            }
        }
        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_UPDATE_PERIOD_MS));
    }
}

/**
 * @brief 用户界面任务（按键处理）
 */
void taskUserInterface(void* parameter) {
    while (1) {
        // 更新按键状态
        btnPower->update();
        btnUp->update();
        btnDown->update();
        
        // 电源/模式按键
        if (btnPower->wasPressed()) {
            sysState.heatingEnabled = !sysState.heatingEnabled;
            sysState.pumpEnabled = !sysState.pumpEnabled;
            
            if (sysState.heatingEnabled) {
                heatingCtrl->enable();
                pumpCtrl->start();
                pumpCtrl->setSpeed(60);
                buzzer->beep();
                safePrint("[系统] 加热和泵已启动\n");
            } else {
                heatingCtrl->disable();
                pumpCtrl->stop();
                buzzer->beep();
                safePrint("[系统] 加热和泵已停止\n");
            }
        }
        
        // 增加温度
        if (btnUp->wasPressed()) {
            sysState.targetTemp += 1.0f;
            if (sysState.targetTemp > TEMP_MAX_LIMIT) {
                sysState.targetTemp = TEMP_MAX_LIMIT;
            }
            heatingCtrl->setTargetTemperature(sysState.targetTemp);
            buzzer->beep();
            safePrint("[设置] 目标温度: %.1f°C\n", sysState.targetTemp);
        }
        
        // 减少温度
        if (btnDown->wasPressed()) {
            sysState.targetTemp -= 1.0f;
            if (sysState.targetTemp < TEMP_MIN_LIMIT) {
                sysState.targetTemp = TEMP_MIN_LIMIT;
            }
            heatingCtrl->setTargetTemperature(sysState.targetTemp);
            buzzer->beep();
            safePrint("[设置] 目标温度: %.1f°C\n", sysState.targetTemp);
        }
        
        // 长按电源键 - 紧急停止
        if (btnPower->isLongPressed(3000)) {
            sysState.emergencyStop = true;
            heatingCtrl->emergencyStop();
            pumpCtrl->stop();
            buzzer->warning();
            safePrint("[系统] 紧急停止触发\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        // 定期打印系统状态
        static uint32_t lastStatusTime = 0;
        if (millis() - lastStatusTime > 10000) {
            safePrint("\n=== 系统状态 ===\n");
            safePrint("温度: %.1f°C (目标: %.1f°C)\n", sysState.currentTemp, sysState.targetTemp);
            safePrint("压力: %.2f kPa (目标: %.2f kPa)\n", sysState.currentPressure, sysState.targetPressure);
            safePrint("加热: %s, 泵: %s\n", 
                     sysState.heatingEnabled ? "ON" : "OFF",
                     sysState.pumpEnabled ? "ON" : "OFF");
            safePrint("================\n\n");
            lastStatusTime = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 按键扫描周期
    }
}
