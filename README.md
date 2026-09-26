# STM32F407 Embedded Remote Video Monitoring System

基于 STM32F407、STM32 标准外设库、OV5640、FreeRTOS、lwIP 的嵌入式远程视频监控系统。设备端只与服务器保持一条持久 TCP 连接，服务器负责设备管理、视频帧缓存和多客户端分发。

## 当前阶段

Phase 1：Camera Bring-up。

首个可验收目标是：

```text
OV5640 -> DCMI -> DMA -> Frame Buffer -> LCD
```

当前已建立可直接使用 Keil MDK 打开和编译的 Phase 1 工程，硬件基线为野火 STM32F407 霸天虎 V2 + OV5640 + 野火 4.3 寸 ILI9806G。

Keil 工程：`firmware/Project/MDK-ARM/RVM_F407_Phase1.uvprojx`

## 仓库结构

```text
docs/           架构、决策、测试与阶段计划
firmware/       STM32F407 设备端
protocol/       设备与服务器的线上协议规范
server/         FastAPI/asyncio 服务器（后续 Phase）
web/            Web 管理端（后续 Phase）
pc-client/      Qt Windows 客户端（后续 Phase）
mobile-app/     Flutter 移动端（后续 Phase）
tools/          协议测试、抓帧和开发工具
```

`../project_references/` 和 `../摄像头0V5640资料/` 是工程外部的本地参考资料，不属于 Git 仓库。

## 开发原则

- 禁止业务层直接使用 GPIO 寄存器或标准库外设句柄。
- 图像路径不使用频繁 `malloc/free`，使用固定 Frame Buffer Pool。
- DCMI 采集必须由 DMA 完成，ISR 只发事件，不做解码、显示和网络发送。
- 任务之间通过队列、事件或明确所有权的 Buffer Handle 通信。
- TCP 协议必须处理拆包、粘包、部分收发和断线重连。
- 每个 Phase 必须包含测试步骤、预期结果、Debug 方法和验收标准。

详细边界见 [docs/architecture.md](docs/architecture.md)，开发顺序见 [docs/development-plan.md](docs/development-plan.md)。
