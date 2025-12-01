/**
 * @file Buzzer.cpp
 * @brief 蜂鸣器实现
 */

#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin, uint8_t pwm_channel)
    : buzzerPin(pin), pwmChannel(pwm_channel) {
}

void Buzzer::begin() {
    pinMode(buzzerPin, OUTPUT);
    ledcSetup(pwmChannel, 2731, 8); // HY9055谐振频率2731Hz
    ledcAttachPin(buzzerPin, pwmChannel);
    ledcWrite(pwmChannel, 0);
    Serial.println("Buzzer Init Success");
}

void Buzzer::tone(uint16_t frequency, uint32_t duration) {
    ledcSetup(pwmChannel, frequency, 8);
    ledcWrite(pwmChannel, 128); // 50%占空比
    
    if (duration > 0) {
        delay(duration);
        noTone();
    }
}

void Buzzer::noTone() {
    ledcWrite(pwmChannel, 0);
}

void Buzzer::beep() {
    tone(2731, 100);
}

void Buzzer::warning() {
    for (int i = 0; i < 3; i++) {
        tone(2731, 200);
        delay(100);
    }
}

void Buzzer::error() {
    tone(2000, 1000);
}
