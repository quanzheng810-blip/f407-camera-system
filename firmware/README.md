# Firmware

## 分层与依赖方向

```text
App -> Services -> BSP -> Platform
 |        |
 +------> Common
```

保持四层主链路即可；底层禁止反向引用 App 或业务状态。

## 目录责任

```text
App/            main、用例编排和主状态机
Services/       Camera、Display、System、Network 服务
Board/Drivers/  BSP 实现：开发板与器件驱动
Components/     Common：日志、指标、时间基准、Buffer Pool
Platform/       CMSIS、STM32F4 StdPeriph 与启动文件
Project/        Keil 工程文件
Config/         功能和产品参数
Middleware/     FreeRTOS、lwIP、LVGL 等第三方代码
Tests/          Host 与 Target 测试
```

## Keil 工程

- 打开 `Project/MDK-ARM/RVM_F407_Phase2.uvprojx`。
- 目标器件：STM32F407ZGTx。
- 编译器：ARM Compiler 5.06 update 6。
- 定义：`USE_STDPERIPH_DRIVER`、`STM32F40_41xxx`。
- 当前硬件：野火霸天虎 V2、OV5640、4.3 寸 ILI9806G。
- 已用 ARMCC5 全量重编译验证：0 errors, 0 warnings。

## 工程约定

- 不使用 CubeMX/HAL 生成或覆盖本标准库工程。
- App 只编排 Service；Service 通过 BSP/Common 完成工作。
- 新业务代码不得直接引用 GPIO、DCMI、DMA、FSMC 或野火 `bsp_*` 接口。
- 引脚、总线与板级资源逐步归一到 `Board/board_config.h`；产品参数放在 `Config/product_config.h`。
- 中断文件不得修改 App 全局变量，只调用明确的 `On*Irq` 入口。
- 图像内存最终由固定 Buffer Pool 管理，不在帧路径中使用频繁 `malloc/free`。

当前 Phase 2 已接入 FreeRTOS、lwIP、LAN8720 标准库 Ethernet BSP 和 LVGL。相机仍采用 DCMI DMA 直接写 LCD FSMC 数据寄存器；下一阶段再通过固定 Buffer Pool 解耦显示和网络发送。
