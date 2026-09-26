# Keil MDK-ARM

## 打开

双击 `RVM_F407_Phase2.uvprojx`，或在 Keil uVision 中选择 **Project -> Open Project**。

## 配置

- Device：STM32F407ZGTx
- Toolchain：ARM Compiler 5.06 update 6
- Device Pack：Keil STM32F4xx DFP
- Board：野火 STM32F407 霸天虎 V2
- Camera：OV5640
- LCD：野火 4.3 寸 ILI9806G，480x800，FSMC 16-bit
- Capture path：OV5640 -> DCMI -> DMA -> LCD FSMC data register
- Camera output：RGB565，320x240（当前 bring-up 默认）
- RTOS：FreeRTOS 9.0.0
- Network：lwIP 2.1.2 + LAN8720 RMII + DHCP
- GUI：LVGL 8.3.11，横屏逻辑分辨率 800x480

## 编译

1. 选中 `RVM_F407_Phase2` target。
2. 点击 **Build** 或按 `F7`。
3. 预期生成 `Objects/RVM_F407_Phase2.axf` 和 `.hex`。

已在本机 Keil MDK 5 上执行 **Rebuild All**：0 errors, 0 warnings。

构建占用：Code=201012，RO-data=25488，RW-data=5964，ZI-data=84420 bytes。

## Keil 分组

```text
00_PLATFORM
01_BSP
02_MIDDLEWARE_FREERTOS
02_MIDDLEWARE_LWIP
02_MIDDLEWARE_LVGL
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

## 内存布局

- FreeRTOS heap：内部 SRAM，32 KiB。
- LVGL heap：外部 SRAM `0x6C000000` 起始，32 KiB。
- LVGL draw buffer：外部 SRAM `0x6C010000` 起始，800 x 8 x RGB565。
- Ethernet DMA descriptors/buffers：内部 SRAM，6 个 RX、4 个 TX。

## 当前边界

此 target 已完成 RTOS、GUI、TCP/IP 协议栈和标准库 Ethernet BSP 的工程集成。相机仍沿用 Phase 1 的 DCMI DMA 直达 LCD 路径；尚未把视频帧送入 TCP，也尚未完成目标板实机联调。下一步是固定帧池、JPEG 单帧上传和持续视频传输。
