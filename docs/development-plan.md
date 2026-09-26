# 分阶段开发计划

## Phase 0 - 工程基线（已完成）

- 建立 Monorepo、分层目录、编码规则和接口契约。
- 核对 F407 霸天虎 V2、OV5640 和实际 LCD 型号。
- 从原理图和 IO 分配表产出唯一 `board_config.h`。
- 工具链已确定：Keil MDK 5 + ARM Compiler 5 + STM32F4 StdPeriph。不同时维护 CubeIDE/HAL 工程。

## Phase 1 - Camera Bring-up

1. 建立最小板级工程：时钟、串口日志、LED 心跳。
2. SCCB 读取 OV5640 Chip ID，验证复位和寄存器访问。
3. 配置 DCMI + DMA，先采集固定尺寸帧。
4. 实现 Double Buffer 所有权转移。
5. LCD 持续显示，统计 capture FPS 和错帧。

### Phase 1 验收

- 连续运行 30 分钟，无 HardFault、越界和 Buffer 所有权冲突。
- LCD 画面稳定，无明显撕裂或 DMA 覆盖。
- 串口可输出 OV5640 ID、分辨率、帧率和错误计数。
- 拔插摄像头或制造采集错误时，系统进入可观测的 ERROR 状态，不静默卡死。

## Phase 2 - RTOS / Network / GUI Integration（当前）

- FreeRTOS 9.0.0 接管 SysTick、SVC 和 PendSV。
- lwIP 2.1.2 使用 FreeRTOS sys_arch，支持 DHCP、TCP/UDP、Netconn 和 Socket API。
- LAN8720 通过 STM32F407 RMII MAC 接入；底层为 STM32 标准库驱动，不引入 HAL。
- LVGL 8.3.11 接入 4.3 寸 ILI9806G，heap 和 draw buffer 放到板载外部 SRAM。
- UI 显示相机统计、网线状态和 DHCP 地址。

### Phase 2 验收

- Keil ARMCC5 Rebuild All 为 0 errors / 0 warnings。
- 启动后可进入 FreeRTOS 调度，LVGL UI 持续响应，无 malloc/stack overflow hook。
- 插入网线后可从 DHCP 获取地址，LCD 显示地址，PC 可 ping 通设备。
- 拔插网线后链路状态可恢复，相机预览不发生 HardFault。

## 后续 Phase

固定 Frame Buffer Pool -> JPEG 单帧上传 -> 连续流 -> WebSocket Viewer -> Device Manager -> Remote Control -> Qt -> Flutter -> 性能优化。
