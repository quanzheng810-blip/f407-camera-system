# Phase 0 硬件核对清单

只有来自原理图、IO 分配表或实物丝印的结论才能写入 `board_config.h`。

## 必须确认

- [x] 开发板确认为野火 STM32F407 霸天虎 V2，Keil 目标器件为 STM32F407ZGTx。
- [ ] OV5640 模块版本、排线方向、供电和电平。
- [ ] OV5640 SCCB、RESET、PWDN、XCLK 引脚。
- [ ] DCMI D0..D7、PCLK、HSYNC、VSYNC 引脚及 AF 映射。
- [ ] DCMI DMA controller/stream/channel 与中断优先级。
- [ ] LCD 实物丝印最终核对；当前 Keil 目标为野火 4.3 寸 ILI9806G（480x800）。
- [ ] LCD 总线、复位、背光和触摸控制器连接。
- [ ] 外部存储器类型、容量、基地址、总线宽度和时序。
- [ ] Ethernet PHY 型号、RMII 引脚、PHY 地址、RESET 和 REF_CLK 来源。
- [ ] 调试串口和 LED，用于最小板级 smoke test。
- [x] 工具链：Keil MDK 5 + ARM Compiler 5.06 update 6 + STM32F4 DFP。

## 本地资料位置

- 开发板 V2：`../project_references/00_hardware/F407_霸天虎V2/`
- OV5640：`../project_references/07_ov5640/`
- HAL 例程：`../project_references/02_hal_examples/`
- FreeRTOS：`../project_references/03_freertos_f407_examples/` 和 `../project_references/04_freertos_source/`
- lwIP：`../project_references/05_lwip_f407_examples/` 和 `../project_references/06_lwip_source/`
- LCD：`../project_references/08_lcd_4.3_4.5/` 和 `../project_references/09_lcd_2.8_3.2/`
