# Phase 1 上板测试

## 测试步骤

1. 核对 LCD 为野火 4.3 寸 ILI9806G，正确安装 OV5640。
2. 如需复现该阶段，先切换到 Git tag `v0.1.0-phase1`，再打开 `firmware/Project/MDK-ARM/RVM_F407_Phase1.uvprojx`。
3. Build target，确认 0 errors / 0 warnings。
4. 选择实际 DAP/ST-Link 调试器，下载后复位。
5. 观察 LCD 初始化文字、OV5640 ID 检测结果和 320x240 实时画面。
6. 连续运行 30 分钟，观察帧率日志、花屏、擒获停止和 HardFault。

## 预期结果

- LCD 背光和初始化正常。
- 串口打印 OV5640 PIDH `0x56`。
- LCD 显示连续摄像头画面，无持续花屏、滚屏或中途卡死。
- Keil 无 HardFault，DCMI/DMA 帧计数持续增长。

## Debug 方法

- 黑屏：检查 LCD 控制器型号、FSMC 接触、RESET 和背光。
- 提示找不到摄像头：检查 SCCB PB8/PB9、PWDN PC0、RESET PF10 和排线方向。
- 画面花屏：检查 DCMI PCLK/HSYNC/VSYNC/数据线、LCD 扫描方向和 DMA 宽度。
- 无帧中断：在 `DCMI_IRQHandler` 和 DMA Stream1 中断处设断点，查看 DCMI/DMA 状态寄存器。

## 验收标准

- 30 分钟连续显示。
- 无 HardFault、无 DMA 错误累积、无长时间画面冻结。
- 记录实际 FPS、分辨率、LCD 型号和摄像头模组版本。
