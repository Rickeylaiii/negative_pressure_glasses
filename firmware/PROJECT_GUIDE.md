# 项目文件说明

## 固件目录结构

```
firmware/
├── platformio.ini          # PlatformIO配置文件
├── README.md              # 项目说明文档
├── HARDWARE.md            # 硬件连接指南
├── include/               # 头文件目录
│   ├── config.h          # 系统配置和引脚定义
│   ├── TemperatureSensor.h
│   ├── PressureSensor.h
│   ├── HeatingController.h
│   ├── PumpController.h
│   ├── Buzzer.h
│   └── Button.h
├── src/                   # 源文件目录
│   ├── main.cpp          # 主程序（FreeRTOS多任务）
│   ├── hardware_test.cpp # 硬件测试程序
│   ├── TemperatureSensor.cpp
│   ├── PressureSensor.cpp
│   ├── HeatingController.cpp
│   ├── PumpController.cpp
│   ├── Buzzer.cpp
│   └── Button.cpp
└── lib/                   # 第三方库（自动下载）
```

## 文件功能说明

### 配置文件

- **platformio.ini**: PlatformIO构建配置
  - 开发板型号：esp32-c3-devkitm-1
  - 框架：Arduino
  - 依赖库：MAX31855, MAX6675

- **config.h**: 系统参数配置
  - GPIO引脚映射
  - 温度/压力范围
  - PWM参数
  - FreeRTOS任务参数

### 硬件抽象层（HAL）

每个模块包含头文件和实现文件：

1. **TemperatureSensor**: MAX31855热电偶接口
   - SPI通讯
   - 温度读取和故障检测
   - 支持多个传感器（不同CS）

2. **PressureSensor**: XGZ6897d压力传感器
   - I2C通讯
   - 压力读取和转换
   - 零点校准功能

3. **HeatingController**: 加热片PID控制
   - PWM输出控制
   - PID温度调节
   - 过温保护

4. **PumpController**: 真空泵速度控制
   - PWM速度调节
   - 启停控制

5. **Buzzer**: 蜂鸣器音频反馈
   - 多种提示音
   - 频率可调

6. **Button**: 按键处理
   - 消抖算法
   - 短按/长按检测

### 主程序

- **main.cpp**: 生产固件
  - FreeRTOS多任务架构
  - 4个独立任务（温度/压力/加热/UI）
  - 互斥锁保护共享资源
  - 完整的安全保护

- **hardware_test.cpp**: 测试固件
  - 逐模块测试
  - 串口交互式菜单
  - 用于硬件调试和验证

## 开发流程

### 1. 硬件准备

1. 按照 HARDWARE.md 连接硬件
2. 检查所有连接
3. 上电前确认无短路

### 2. 首次调试

使用测试固件：

```bash
# 重命名文件
mv src/main.cpp src/main_backup.cpp
mv src/hardware_test.cpp src/main.cpp

# 编译上传
pio run --target upload

# 串口监视
pio device monitor
```

按菜单提示逐个测试模块。

### 3. 正式运行

恢复主程序：

```bash
mv src/main.cpp src/hardware_test.cpp
mv src/main_backup.cpp src/main.cpp

pio run --target upload
```

### 4. 参数调整

根据实际情况修改 `config.h` 中的参数：

- 温度范围
- 压力范围
- PID系数（在HeatingController.cpp中）
- 任务优先级和周期

## 常用命令

```bash
# 编译
pio run

# 编译并上传
pio run --target upload

# 清理编译
pio run --target clean

# 串口监视
pio device monitor

# 查看依赖
pio lib list

# 更新依赖
pio lib update
```

## 调试技巧

### 1. 串口调试

系统会输出详细的调试信息：

```
[温度] 当前: 36.5°C, 目标: 37.0°C
[加热] 功率: 45%
[压力] 当前: -1.8kPa, 目标: -2.0kPa
```

### 2. 任务监控

FreeRTOS提供任务统计功能（需开启）：

```cpp
// 在platformio.ini中添加
build_flags = 
    -DCONFIG_FREERTOS_USE_TRACE_FACILITY
    -DCONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
```

### 3. 性能分析

观察任务运行情况：

```cpp
char statsBuffer[512];
vTaskGetRunTimeStats(statsBuffer);
Serial.println(statsBuffer);
```

## 安全注意事项

### 代码层面

- ✓ 温度上限保护（45°C紧急停止）
- ✓ 传感器故障检测
- ✓ 看门狗定时器（可选）
- ✓ 互斥锁防止竞态条件

### 硬件层面

- ⚠️ 加热片必须有温度监控
- ⚠️ MOSFET需要散热
- ⚠️ 电源功率需充足
- ⚠️ 首次测试使用低功率

### 使用层面

- ⚠️ 不要长时间高温运行
- ⚠️ 定期检查传感器
- ⚠️ 异常立即断电
- ⚠️ 不要在无人情况下运行

## 扩展功能建议

### 短期改进

- [ ] 添加OLED显示屏
- [ ] 数据记录到SD卡
- [ ] 更精细的压力PID控制
- [ ] WiFi/蓝牙配置界面

### 长期规划

- [ ] 手机APP控制
- [ ] 多温区独立控制
- [ ] 治疗程序预设
- [ ] 云端数据分析
- [ ] OTA固件升级

## 故障排除

### 编译错误

**错误**: `fatal error: config.h: No such file or directory`

**解决**: 确保 include 目录在项目根目录

**错误**: `Multiple libraries found for "MAX31855.h"`

**解决**: 在 platformio.ini 指定具体库

### 运行错误

**现象**: 温度始终显示NAN

**检查**:
- SPI连接
- 热电偶极性
- CS引脚定义

**现象**: 系统频繁重启

**检查**:
- 电源电流是否充足
- 是否触发看门狗
- 是否有内存溢出

**现象**: 加热不工作

**检查**:
- MOSFET连接
- PWM输出（示波器）
- 加热片电阻

## 技术支持

### 相关文档

- [ESP32-C3文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/)
- [FreeRTOS文档](https://www.freertos.org/Documentation/RTOS_book.html)
- [PlatformIO文档](https://docs.platformio.org/)

### 问题反馈

遇到问题时，提供以下信息：

1. 硬件版本和BOM
2. 完整的串口输出
3. 问题复现步骤
4. 修改过的代码

## 版本历史

### v1.0.0 (2025-11-02)

- ✓ 基础FreeRTOS架构
- ✓ 温度/压力传感器支持
- ✓ PID加热控制
- ✓ 真空泵PWM控制
- ✓ 按键和蜂鸣器
- ✓ 硬件测试程序
- ✓ 完整文档

---

**注意**: 本项目为实验性质，使用前请充分测试并评估风险。
