/**
 * @file Button.cpp
 * @brief 按键实现
 */

#include "Button.h"

Button::Button(uint8_t pin, bool pull_up)
    : buttonPin(pin), pullUp(pull_up), currentState(false), lastState(false),
      pressed(false), released(false), pressedTime(0), lastDebounceTime(0) {
}

void Button::begin() {
    if (pullUp) {
        pinMode(buttonPin, INPUT_PULLUP);
    } else {
        pinMode(buttonPin, INPUT);
    }
    
    currentState = digitalRead(buttonPin) == LOW; // 假设低电平为按下
    lastState = currentState;
}

void Button::update() {
    bool reading = digitalRead(buttonPin) == LOW;
    
    // 消抖
    if (reading != lastState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // 状态稳定
        if (reading != currentState) {
            currentState = reading;
            
            if (currentState) {
                // 按下
                pressed = true;
                pressedTime = millis();
            } else {
                // 释放
                released = true;
            }
        }
    }
    
    lastState = reading;
}

bool Button::wasPressed() {
    if (pressed) {
        pressed = false;
        return true;
    }
    return false;
}

bool Button::wasReleased() {
    if (released) {
        released = false;
        return true;
    }
    return false;
}

bool Button::isPressed() {
    return currentState;
}

bool Button::isLongPressed(uint32_t threshold) {
    if (currentState && (millis() - pressedTime) >= threshold) {
        return true;
    }
    return false;
}

uint32_t Button::getPressedDuration() {
    if (currentState) {
        return millis() - pressedTime;
    }
    return 0;
}
