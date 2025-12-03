/**
 * @file test_buzzer.cpp
 * @brief Electronic Keyboard Simulator - 电子琴模拟器
 * 
 * 功能：
 * - 用A,S,D,F,G,H,J,K模拟钢琴音阶 (Do Re Mi Fa Sol La Si Do)
 * - 实时按键发声，松开停止
 * - 支持持续按住发声
 * - LED随按键闪烁
 * 
 * 按键映射：
 * - A: Do  (C4 - 262Hz)
 * - S: Re  (D4 - 294Hz)
 * - D: Mi  (E4 - 330Hz)
 * - F: Fa  (F4 - 349Hz)
 * - G: Sol (G4 - 392Hz)
 * - H: La  (A4 - 440Hz)
 * - J: Si  (B4 - 494Hz)
 * - K: Do' (C5 - 523Hz)
 * - Space: 停止发声
 * - Q: 退出电子琴模式
 * 
 * 使用方法：
 * 1. 重命名此文件为 main.cpp
 * 2. 备份原 main.cpp
 * 3. 编译上传
 * 4. 打开串口监视器，输入字母弹奏
 */

#include <Arduino.h>
#include "config.h"
#include "Buzzer.h"

// LED 引脚
#define LED_PIN 8

// 蜂鸣器对象
Buzzer* buzzer;

// 音符频率定义（简易音阶）
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440 
#define NOTE_B4  494
#define NOTE_C5  523

// 蜂鸣器默认频率（如果 config.h 中没有定义）
#ifndef BUZZER_FREQUENCY
#define BUZZER_FREQUENCY 2731
#endif

void playMelody();
void playStartupSound();

void setup() {
    // 初始化串口
    Serial.begin(115200);
    
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000)) {
        delay(10);
    }
    delay(500);
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("\n\n========================================");
    Serial.println("Buzzer Test Program");
    Serial.println("========================================");
    Serial.printf("Chip: %s @ %dMHz\n", ESP.getChipModel(), ESP.getCpuFreqMHz());
    Serial.println("========================================\n");
    
    // 显示蜂鸣器配置
    Serial.println("Buzzer Config:");
    Serial.printf("  Pin: GPIO%d\n", BUZZER_PIN);
    Serial.printf("  PWM Channel: %d\n", PWM_CHANNEL_BUZZER);
    Serial.printf("  Default Frequency: %d Hz\n", BUZZER_FREQUENCY);
    Serial.println();
    
    // 初始化蜂鸣器
    buzzer = new Buzzer(BUZZER_PIN, PWM_CHANNEL_BUZZER);
    buzzer->begin();
    
    Serial.println("OK Buzzer initialized");
    Serial.println();
    
    Serial.println("=== ELECTRONIC KEYBOARD MODE ===");
    Serial.println();
    Serial.println("Keyboard Mapping:");
    Serial.println("  A - Do  (C4 - 262Hz)");
    Serial.println("  S - Re  (D4 - 294Hz)");
    Serial.println("  D - Mi  (E4 - 330Hz)");
    Serial.println("  F - Fa  (F4 - 349Hz)");
    Serial.println("  G - Sol (G4 - 392Hz)");
    Serial.println("  H - La  (A4 - 440Hz)");
    Serial.println("  J - Si  (B4 - 494Hz)");
    Serial.println("  K - Do' (C5 - 523Hz)");
    Serial.println();
    Serial.println("Special Commands:");
    Serial.println("  Space - Stop sound");
    Serial.println("  0 - Play scale (Do Re Mi Fa Sol La Si Do)");
    Serial.println("  9 - Demo song");
    Serial.println("  Q - Quit (show all commands)");
    Serial.println();
    Serial.println("================================");
    Serial.println();
    
    // 启动提示音
    Serial.println("Playing startup sound...");
    playStartupSound();
    Serial.println();
    
    // LED 闪烁表示就绪
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    
    Serial.println("Ready! Start playing music...\n");
}

void loop() {
    static uint32_t lastHeartbeat = 0;
    static bool isPlaying = false;
    static int currentNote = 0;
    
    // 心跳指示（每10秒）
    if (!isPlaying && (millis() - lastHeartbeat > 10000)) {
        lastHeartbeat = millis();
        digitalWrite(LED_PIN, HIGH);
        delay(30);
        digitalWrite(LED_PIN, LOW);
    }
    
    // 处理串口按键
    if (Serial.available()) {
        char key = Serial.read();
        
        // 转换为大写
        if (key >= 'a' && key <= 'z') {
            key = key - 32;
        }
        
        int frequency = 0;
        const char* noteName = "";
        int noteDuration = 300; // 默认音符时长
        
        // 按键映射到音符
        switch (key) {
            case 'A':
                frequency = NOTE_C4;
                noteName = "Do (C4)";
                break;
            case 'S':
                frequency = NOTE_D4;
                noteName = "Re (D4)";
                break;
            case 'D':
                frequency = NOTE_E4;
                noteName = "Mi (E4)";
                break;
            case 'F':
                frequency = NOTE_F4;
                noteName = "Fa (F4)";
                break;
            case 'G':
                frequency = NOTE_G4;
                noteName = "Sol (G4)";
                break;
            case 'H':
                frequency = NOTE_A4;
                noteName = "La (A4)";
                break;
            case 'J':
                frequency = NOTE_B4;
                noteName = "Si (B4)";
                break;
            case 'K':
                frequency = NOTE_C5;
                noteName = "Do' (C5)";
                break;
            case ' ':
                // 空格键停止发声
                buzzer->noTone();
                digitalWrite(LED_PIN, LOW);
                isPlaying = false;
                Serial.println("[Stop]");
                break;
            case '0':
                // 播放音阶
                Serial.println("\n[Playing Scale: Do Re Mi Fa Sol La Si Do]\n");
                playMelody();
                Serial.println();
                break;
            case '9':
                // 播放示例曲子
                Serial.println("\n[Playing Demo Song]\n");
                playStartupSound();
                delay(500);
                playMelody();
                Serial.println();
                break;
            case 'Q':
                // 显示所有命令
                Serial.println("\n=== ALL COMMANDS ===");
                Serial.println("Keyboard Mode:");
                Serial.println("  A,S,D,F,G,H,J,K - Play notes");
                Serial.println("  Space - Stop");
                Serial.println("  0 - Play scale");
                Serial.println("  9 - Demo song");
                Serial.println("\nTest Commands:");
                Serial.println("  1 - Short beep");
                Serial.println("  2 - Warning");
                Serial.println("  3 - Error alarm");
                Serial.println("  T - Test mode (sweep)");
                Serial.println("====================\n");
                break;
            case '1':
                Serial.println("[Beep]");
                buzzer->beep();
                break;
            case '2':
                Serial.println("[Warning]");
                buzzer->warning();
                break;
            case '3':
                Serial.println("[Error]");
                buzzer->error();
                break;
            case 'T':
                // 频率扫描测试
                Serial.println("\n[Frequency Sweep Test]");
                for (int freq = 200; freq <= 1000; freq += 100) {
                    Serial.printf("  %d Hz\n", freq);
                    digitalWrite(LED_PIN, HIGH);
                    buzzer->tone(freq, 150);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                }
                Serial.println();
                break;
            case '\n':
            case '\r':
                // 忽略换行符
                break;
            default:
                // 其他按键提示
                if (key >= 33 && key <= 126) {
                    Serial.printf("[Key '%c' - Not mapped]\n", key);
                }
                break;
        }
        
        // 如果是有效音符，播放声音
        if (frequency > 0) {
            Serial.printf("♪ %s - %d Hz\n", noteName, frequency);
            digitalWrite(LED_PIN, HIGH);
            buzzer->tone(frequency, noteDuration);
            isPlaying = true;
            
            // 等待音符播放完成
            delay(noteDuration);
            buzzer->noTone();
            digitalWrite(LED_PIN, LOW);
            isPlaying = false;
        }
    }
    
    delay(10);
}

/**
 * @brief 播放简单旋律（Do Re Mi Fa Sol La Si Do）
 */
void playMelody() {
    int melody[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};
    int noteDuration = 300;
    
    for (int i = 0; i < 8; i++) {
        Serial.printf("   Note %d: %d Hz\n", i+1, melody[i]);
        digitalWrite(LED_PIN, HIGH);
        buzzer->tone(melody[i], noteDuration);
        delay(noteDuration + 50);
        digitalWrite(LED_PIN, LOW);
        delay(50);
    }
}

/**
 * @brief 播放启动音效
 */
void playStartupSound() {
    // 三音上升音阶
    int notes[] = {NOTE_C4, NOTE_E4, NOTE_G4};
    int durations[] = {100, 100, 200};
    
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        buzzer->tone(notes[i], durations[i]);
        delay(durations[i] + 50);
        digitalWrite(LED_PIN, LOW);
        delay(50);
    }
}