# 快速开始指南

## 5分钟上手

### 前置要求

- ✓ Windows系统（你的当前环境）
- ✓ VS Code + PlatformIO扩展
- ✓ ESP32-C3开发板
- ✓ USB Type-C数据线

### 步骤1：安装PlatformIO

1. 打开VS Code
2. 进入扩展商店（Ctrl+Shift+X）
3. 搜索"PlatformIO IDE"
4. 点击安装
5. 重启VS Code

### 步骤2：打开项目

```powershell
# 在VS Code中打开firmware文件夹
cd d:\git_ck\positive_glasses\firmware
code .
```

或者：
- File → Open Folder → 选择 `d:\git_ck\positive_glasses\firmware`

### 步骤3：首次测试（推荐）

使用硬件测试程序验证连接：

1. 在VS Code中重命名文件：
   - `src/main.cpp` → `src/main_production.cpp`
   - `src/hardware_test.cpp` → `src/main.cpp`

2. 连接ESP32-C3到电脑

3. 点击底部状态栏的"PlatformIO: Build"（✓图标）

4. 编译成功后点击"PlatformIO: Upload"（→图标）

5. 点击"PlatformIO: Serial Monitor"（🔌图标）

6. 按菜单提示测试各模块

### 步骤4：运行生产固件

测试完成后：

1. 恢复文件名：
   - `src/main.cpp` → `src/hardware_test.cpp`
   - `src/main_production.cpp` → `src/main.cpp`

2. 重新编译上传

3. 系统自动运行FreeRTOS多任务程序

## 基本操作

### 按键功能

- **POWER键（短按）**：启动/停止系统
- **UP键**：温度+1°C
- **DOWN键**：温度-1°C
- **POWER键（长按3秒）**：紧急停止

### 串口输出

连接串口监视器（115200波特率）可看到：

```
========================================
FSYX OPAP 带加热负压眼镜系统启动
========================================

初始化硬件...
温度传感器初始化成功，当前温度: 25.30°C
压力传感器初始化成功，当前压力: 0.02 kPa
加热控制器初始化完成
泵控制器初始化完成
蜂鸣器初始化完成
硬件初始化完成

所有任务已创建，系统运行中...

=== 系统状态 ===
温度: 25.3°C (目标: 37.0°C)
压力: 0.02 kPa (目标: -2.00 kPa)
加热: OFF, 泵: OFF
================
```

## 常见问题快速解答

### Q: 找不到设备？

**A**: 
1. 确认USB线连接正确
2. 安装CH340驱动（ESP32-C3常用）
3. 检查设备管理器中的COM口

### Q: 编译错误？

**A**:
1. 等待PlatformIO自动下载依赖库
2. 检查 `platformio.ini` 配置
3. 清理项目：PlatformIO → Clean

### Q: 上传失败？

**A**:
1. 按住ESP32-C3的BOOT键
2. 点击RESET键
3. 松开BOOT键
4. 立即点击Upload

### Q: 温度读不到？

**A**:
1. 检查SPI引脚连接（SCK=GPIO2, CS=GPIO3, MISO=GPIO10）
2. 确认3.3V供电
3. 运行硬件测试程序诊断

### Q: 加热不工作？

**A**:
1. 检查MOSFET连接
2. 用万用表测量PWM输出（GPIO5）
3. 确认加热片电源正常

## 下一步

完成基本测试后，你可以：

1. **调整参数**：修改 `include/config.h` 中的温度/压力范围

2. **优化PID**：修改 `src/HeatingController.cpp` 中的PID系数
   ```cpp
   float kp = 25.0f;   // 比例系数
   float ki = 0.5f;    // 积分系数
   float kd = 5.0f;    // 微分系数
   ```

3. **添加功能**：参考 `PROJECT_GUIDE.md` 扩展系统

4. **硬件改进**：查看 `HARDWARE.md` 优化电路

## 技术支持

- 📖 **完整文档**: `README.md`
- 🔧 **硬件指南**: `HARDWARE.md`
- 📘 **项目说明**: `PROJECT_GUIDE.md`
- 🧪 **测试程序**: `src/hardware_test.cpp`

## 安全提醒 ⚠️

- 首次使用低功率测试（PWM<50%）
- 加热时必须监控温度
- 温度超过40°C时手动停止检查
- 异常情况立即断电
- 不要无人看管运行

---

祝你开发顺利！遇到问题请参考详细文档或反馈问题。
