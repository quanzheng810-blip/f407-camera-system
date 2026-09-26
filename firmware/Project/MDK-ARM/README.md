# Keil MDK-ARM

## 打开

双击 `RVM_F407_Phase1.uvprojx`，或在 Keil uVision 中选择 **Project -> Open Project**。

## 配置

- Device：STM32F407ZGTx
- Toolchain：ARM Compiler 5.06 update 6
- Device Pack：Keil STM32F4xx DFP
- Board：野火 STM32F407 霸天虎 V2
- Camera：OV5640
- LCD：野火 4.3 寸 ILI9806G，480x800，FSMC 16-bit
- Capture path：OV5640 -> DCMI -> DMA -> LCD FSMC data register
- Camera output：RGB565，320x240（当前 bring-up 默认）

## 编译

1. 选中 `RVM_F407_Phase1` target。
2. 点击 **Build** 或按 `F7`。
3. 预期生成 `Objects/RVM_F407_Phase1.axf` 和 `.hex`。

已在本机 Keil MDK 5 上验证：0 errors, 0 warnings。

## Keil 分组

```text
00_PLATFORM
01_BSP
02_COMMON
03_SERVICES
04_APP
05_DOC
```

`04_APP/main.c` 是唯一程序入口；底层驱动不得反向依赖 Service/App。

## 下载前检查

- 确认实物是 4.3 寸 ILI9806G；如屏幕背面标注 NT35510，不要直接使用此 LCD 驱动。
- 确认 OV5640 排线方向和供电。
- 确认 ST-Link/DAP 下载器连接。
- 在 **Options for Target -> Debug** 中选择实际下载器。

## 当前边界

此 target 是 Phase 1 上板硬件基线，直接使用野火已验证的 DCMI DMA -> LCD 路径。它还不是最终 FreeRTOS 双缓冲架构；完成上板验收后，再将采集、显示和网络发送通过 Frame Buffer Pool 解耦。
