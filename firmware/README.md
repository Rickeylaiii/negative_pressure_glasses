# FSYX™ OPAP 带加热负压眼镜固件

## 项目概述

这是基于ESP32-C3的带加热负压眼镜固件，使用FreeRTOS多任务架构。

## 硬件配置

### 主控芯片
- **ESP32-C3 SuperMini** - 主控芯片

### 传感器
- **MAX31855 K型热电偶模块** x2 - 温度检测
- **TT-K-30 K型热电偶** - 温度探头
- **XGZ6897d 压力传感器** - 负压检测（±5kPa）

### 执行器
- **加热片（PET）** - 通过MOSFET驱动
- **370-3366 真空泵** - 通过L298N驱动
- **HY9055 无源蜂鸣器** - 音频反馈

### 用户界面
- **12mm金属自锁按键** - 电源/模式切换
- **12x12按键** x2 - 温度调节

## 软件架构

### FreeRTOS任务

1. **温度监控任务** (优先级: 高, 核心0)
   - 周期：500ms
   - 功能：读取热电偶温度，过温保护

2. **压力监控任务** (优先级: 高, 核心0)
   - 周期：100ms
   - 功能：读取压力传感器，控制真空泵

3. **加热控制任务** (优先级: 中, 核心1)
   - 周期：200ms
   - 功能：PID温度控制

4. **用户界面任务** (优先级: 低, 核心1)
   - 周期：50ms
   - 功能：按键处理，系统状态显示

### 安全特性

- **过温保护**：温度超过45°C自动停止加热
- **传感器故障检测**：连续3次读取失败触发警报
- **紧急停止**：长按电源键3秒立即停止所有加热和泵
- **软件互锁**：通过互斥锁保护共享资源

## 引脚定义

### SPI (温度传感器)
- SCK: GPIO2
- CS1: GPIO3
- CS2: GPIO4
- MISO: GPIO10

### I2C (压力传感器)
- SDA: GPIO8
- SCL: GPIO9

### PWM输出
- 加热片: GPIO5
- 真空泵: GPIO6
- 蜂鸣器: GPIO18

### 按键输入
- 电源/模式: GPIO19
- 增加: GPIO20
- 减少: GPIO21

## 编译和上传

### 使用PlatformIO

```bash
# 编译
pio run

# 上传
pio run --target upload

# 串口监视
pio device monitor
```

### 使用VS Code
1. 安装PlatformIO扩展
2. 打开项目文件夹
3. 点击底部状态栏的"Build"按钮
4. 点击"Upload"按钮

## 使用说明

### 首次使用

1. 上电后系统自动初始化
2. 听到一声提示音表示初始化成功
3. 默认目标温度：37°C
4. 默认目标压力：-2kPa

### 操作方法

- **启动/停止**：短按电源键
- **增加温度**：短按↑键（每次+1°C）
- **降低温度**：短按↓键（每次-1°C）
- **紧急停止**：长按电源键3秒

### 指示说明

- **单声提示音**：操作确认
- **三声提示音**：警告
- **长鸣**：错误或紧急停止

## 参数配置

在 `include/config.h` 中可以修改以下参数：

```cpp
// 温度范围
#define TEMP_TARGET_DEFAULT 37.0f   // 默认目标温度
#define TEMP_MAX_LIMIT      42.0f   // 最高温度限制
#define TEMP_MIN_LIMIT      30.0f   // 最低温度限制

// 压力范围
#define PRESSURE_TARGET_DEFAULT -2.0f  // 默认目标压力
#define PRESSURE_MAX_LIMIT  -0.5f      // 最小负压
#define PRESSURE_MIN_LIMIT  -5.0f      // 最大负压

// PID参数（在HeatingController.cpp中）
float kp = 25.0f;   // 比例系数
float ki = 0.5f;    // 积分系数
float kd = 5.0f;    // 微分系数
```

## 调试

### 串口输出

波特率：115200

系统会定期输出：
- 传感器读数
- 控制器状态
- 警告和错误信息

### 常见问题

1. **温度读取失败**
   - 检查热电偶连接
   - 确认SPI引脚配置正确

2. **压力读取失败**
   - 检查I2C地址（默认0x6D）
   - 确认I2C引脚连接

3. **加热不工作**
   - 检查MOSFET连接
   - 确认PWM输出正常
   - 检查目标温度设置

## 待改进功能

- [ ] 压力PID控制
- [ ] 数据记录和回放
- [ ] 蓝牙/WiFi远程控制
- [ ] OLED显示屏支持
- [ ] 多温区独立控制

## 许可证

本项目仅供学习和研究使用。

## 参考资料

- [FSYX™ OPAP 原理分析](../readme.md)
- [ESP32-C3技术文档](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [MAX31855数据手册](https://datasheets.maximintegrated.com/en/ds/MAX31855.pdf)
