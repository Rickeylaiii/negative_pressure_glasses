/**
 * @file Button.h
 * @brief 按键处理（带消抖和长按检测）
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
public:
    /**
     * @brief 构造函数
     * @param pin 按键引脚
     * @param pull_up 是否使用内部上拉（默认true）
     */
    Button(uint8_t pin, bool pull_up = true);
    
    /**
     * @brief 初始化
     */
    void begin();
    
    /**
     * @brief 更新按键状态（需要在主循环中定期调用）
     */
    void update();
    
    /**
     * @brief 是否按下（单次触发）
     * @return true 按键被按下
     */
    bool wasPressed();
    
    /**
     * @brief 是否释放
     * @return true 按键被释放
     */
    bool wasReleased();
    
    /**
     * @brief 是否正在按下
     * @return true 按键保持按下状态
     */
    bool isPressed();
    
    /**
     * @brief 是否长按
     * @param threshold 长按阈值（ms），默认1000ms
     * @return true 长按
     */
    bool isLongPressed(uint32_t threshold = 1000);
    
    /**
     * @brief 获取按下持续时间
     * @return 持续时间（ms）
     */
    uint32_t getPressedDuration();
    
private:
    uint8_t buttonPin;
    bool pullUp;
    bool currentState;
    bool lastState;
    bool pressed;
    bool released;
    uint32_t pressedTime;
    uint32_t lastDebounceTime;
    static const uint32_t DEBOUNCE_DELAY = 50; // 消抖延迟（ms）
};

#endif // BUTTON_H
