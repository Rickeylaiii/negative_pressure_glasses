/**
 * @file hardware_test.cpp
 * @brief 硬件测试程序 - 用于逐个测试各个模块
 * 
 * 使用方法：
 * 1. 将此文件重命名为main.cpp（备份原main.cpp）
 * 2. 编译并上传
 * 3. 打开串口监视器（115200波特率）
 * 4. 输入命令测试各模块
 */

#include <Arduino.h>
#include "config.h"
#include "TemperatureSensor.h"
#include "PressureSensor.h"
#include "HeatingController.h"
#include "PumpController.h"
#include "Buzzer.h"
#include "Button.h"

// 测试对象
TemperatureSensor* tempSensor;
PressureSensor* pressureSensor;
HeatingController* heatingCtrl;
PumpController* pumpCtrl;
Buzzer* buzzer;
Button* btnTest;

void printMenu();
void testTemperature();
void testPressure();
void testHeating();
void testPump();
void testBuzzer();
void testButton();
void testAll();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n========================================");
    Serial.println("FSYX OPAP 硬件测试程序");
    Serial.println("========================================\n");
    
    // 初始化对象
    tempSensor = new TemperatureSensor(THERMO_1_CLK_PIN, THERMO_1_CS_PIN, THERMO_1_MISO_PIN);
    pressureSensor = new PressureSensor(PRESSURE_SDA_PIN, PRESSURE_SCL_PIN);
    heatingCtrl = new HeatingController(HEATING_PAD_PIN, PWM_CHANNEL_HEAT);
    pumpCtrl = new PumpController(PUMP_PWM_PIN, PWM_CHANNEL_PUMP);
    buzzer = new Buzzer(BUZZER_PIN, PWM_CHANNEL_BUZZER);
    btnTest = new Button(BUTTON_POWER_PIN);
    
    // 初始化
    Serial.println("初始化模块...");
    heatingCtrl->begin();
    pumpCtrl->begin();
    buzzer->begin();
    btnTest->begin();
    
    buzzer->beep();
    Serial.println("初始化完成！\n");
    
    printMenu();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch(cmd) {
            case '1':
                testTemperature();
                break;
            case '2':
                testPressure();
                break;
            case '3':
                testHeating();
                break;
            case '4':
                testPump();
                break;
            case '5':
                testBuzzer();
                break;
            case '6':
                testButton();
                break;
            case '7':
                testAll();
                break;
            case 'm':
            case 'M':
                printMenu();
                break;
            default:
                break;
        }
    }
    
    delay(100);
}

void printMenu() {
    Serial.println("\n========== 测试菜单 ==========");
    Serial.println("1 - 测试温度传感器");
    Serial.println("2 - 测试压力传感器");
    Serial.println("3 - 测试加热控制");
    Serial.println("4 - 测试真空泵");
    Serial.println("5 - 测试蜂鸣器");
    Serial.println("6 - 测试按键");
    Serial.println("7 - 测试所有模块");
    Serial.println("M - 显示菜单");
    Serial.println("==============================\n");
    Serial.print("> ");
}

void testTemperature() {
    Serial.println("\n[测试] 温度传感器");
    Serial.println("初始化传感器...");
    
    if (tempSensor->begin()) {
        Serial.println("✓ 初始化成功");
        
        Serial.println("读取温度（10次）...");
        for (int i = 0; i < 10; i++) {
            float temp = tempSensor->readTemperature();
            float internal = tempSensor->readInternalTemperature();
            
            if (!isnan(temp)) {
                Serial.printf("  [%d] 温度: %.2f°C, 内部温度: %.2f°C\n", 
                             i+1, temp, internal);
            } else {
                Serial.printf("  [%d] ✗ 读取失败\n", i+1);
            }
            
            delay(500);
        }
        
        Serial.printf("\n状态: %s\n", tempSensor->isValid() ? "正常" : "异常");
    } else {
        Serial.println("✗ 初始化失败");
        Serial.println("  检查项：");
        Serial.println("  - SPI连接是否正确");
        Serial.println("  - 热电偶是否连接");
        Serial.println("  - 电源是否正常");
    }
    
    buzzer->beep();
    printMenu();
}

void testPressure() {
    Serial.println("\n[测试] 压力传感器");
    Serial.println("初始化传感器...");
    
    if (pressureSensor->begin()) {
        Serial.println("✓ 初始化成功");
        
        Serial.println("读取压力（10次）...");
        for (int i = 0; i < 10; i++) {
            float pressure = pressureSensor->readPressure();
            
            if (!isnan(pressure)) {
                Serial.printf("  [%d] 压力: %.3f kPa\n", i+1, pressure);
            } else {
                Serial.printf("  [%d] ✗ 读取失败\n", i+1);
            }
            
            delay(500);
        }
        
        Serial.printf("\n状态: %s\n", pressureSensor->isValid() ? "正常" : "异常");
        
        // 可选：零点校准
        Serial.println("\n是否进行零点校准？(y/n)");
        while(!Serial.available()) delay(10);
        if (Serial.read() == 'y') {
            pressureSensor->calibrateZero();
        }
    } else {
        Serial.println("✗ 初始化失败");
        Serial.println("  检查项：");
        Serial.println("  - I2C连接是否正确");
        Serial.println("  - I2C地址是否正确");
        Serial.println("  - 电源是否正常");
    }
    
    buzzer->beep();
    printMenu();
}

void testHeating() {
    Serial.println("\n[测试] 加热控制");
    Serial.println("⚠️  警告：加热片会发热，注意安全！");
    Serial.println("继续测试？(y/n)");
    
    while(!Serial.available()) delay(10);
    if (Serial.read() != 'y') {
        Serial.println("取消测试");
        printMenu();
        return;
    }
    
    Serial.println("\n测试PWM输出...");
    heatingCtrl->enable();
    
    uint8_t pwm_values[] = {0, 64, 128, 192, 255};
    for (int i = 0; i < 5; i++) {
        Serial.printf("  PWM: %d (%.0f%%)\n", pwm_values[i], 
                     (pwm_values[i]/255.0f)*100.0f);
        ledcWrite(PWM_CHANNEL_HEAT, pwm_values[i]);
        delay(2000);
    }
    
    Serial.println("\n关闭加热...");
    heatingCtrl->disable();
    
    Serial.println("✓ PWM测试完成");
    Serial.println("  检查项：");
    Serial.println("  - 用万用表测量MOSFET栅极电压");
    Serial.println("  - 用手感受加热片温度变化");
    
    buzzer->beep();
    printMenu();
}

void testPump() {
    Serial.println("\n[测试] 真空泵");
    Serial.println("测试不同速度...");
    
    pumpCtrl->start();
    
    uint8_t speeds[] = {20, 40, 60, 80, 100};
    for (int i = 0; i < 5; i++) {
        Serial.printf("  速度: %d%%\n", speeds[i]);
        pumpCtrl->setSpeed(speeds[i]);
        delay(2000);
    }
    
    Serial.println("\n停止泵...");
    pumpCtrl->stop();
    
    Serial.println("✓ 泵测试完成");
    Serial.println("  检查项：");
    Serial.println("  - 泵是否运转");
    Serial.println("  - 转速是否随PWM变化");
    Serial.println("  - 是否有异常噪音");
    
    buzzer->beep();
    printMenu();
}

void testBuzzer() {
    Serial.println("\n[测试] 蜂鸣器");
    
    Serial.println("短促提示音...");
    buzzer->beep();
    delay(1000);
    
    Serial.println("警告音...");
    buzzer->warning();
    delay(1000);
    
    Serial.println("错误音...");
    buzzer->error();
    delay(1000);
    
    Serial.println("不同频率测试...");
    uint16_t freqs[] = {1000, 1500, 2000, 2500, 3000};
    for (int i = 0; i < 5; i++) {
        Serial.printf("  频率: %d Hz\n", freqs[i]);
        buzzer->tone(freqs[i], 500);
        delay(700);
    }
    
    Serial.println("✓ 蜂鸣器测试完成");
    printMenu();
}

void testButton() {
    Serial.println("\n[测试] 按键");
    Serial.println("请按POWER键（10秒内）...");
    Serial.println("支持：");
    Serial.println("  - 短按检测");
    Serial.println("  - 长按检测（1秒）");
    
    uint32_t startTime = millis();
    bool pressed = false;
    
    while (millis() - startTime < 10000) {
        btnTest->update();
        
        if (btnTest->wasPressed()) {
            Serial.println("✓ 短按检测成功");
            buzzer->beep();
            pressed = true;
        }
        
        if (btnTest->isLongPressed(1000)) {
            Serial.println("✓ 长按检测成功");
            buzzer->warning();
        }
        
        if (btnTest->wasReleased()) {
            Serial.printf("  按下持续时间: %lu ms\n", 
                         btnTest->getPressedDuration());
        }
        
        delay(10);
    }
    
    if (!pressed) {
        Serial.println("✗ 未检测到按键");
        Serial.println("  检查项：");
        Serial.println("  - 按键连接是否正确");
        Serial.println("  - 上拉电阻是否有效");
    }
    
    printMenu();
}

void testAll() {
    Serial.println("\n[测试] 全部模块");
    Serial.println("开始综合测试...\n");
    
    testTemperature();
    delay(1000);
    
    testPressure();
    delay(1000);
    
    testBuzzer();
    delay(1000);
    
    // 加热和泵需要用户确认
    Serial.println("\n是否测试加热和泵？(y/n)");
    while(!Serial.available()) delay(10);
    if (Serial.read() == 'y') {
        testHeating();
        delay(1000);
        testPump();
    }
    
    Serial.println("\n========================================");
    Serial.println("全部测试完成！");
    Serial.println("========================================\n");
    
    buzzer->warning();
    printMenu();
}
