# 系统架构与模块边界

## 1. 系统边界

```text
OV5640 -> STM32F407 -> TCP Server -> Stream Hub -> Web / Qt / Flutter
```

STM32 不直接管理观看客户端。服务器对外提供 REST 和 WebSocket，对设备提供持久 TCP 连接。

## 2. Firmware 分层

| 逻辑层 | 对应目录 | 职责 | 禁止 |
| --- | --- | --- | --- |
| `App` | `App`、`Config` | main、用例编排和主状态机 | 写寄存器、调用野火 `bsp_*` |
| `Services` | `Services` | Camera/Display/System/Network 服务 | 依赖 App |
| `BSP` | `Board`、`Drivers`、`Core` | 板级初始化、器件驱动和中断转发 | 承载业务流程 |
| `Common` | `Components` | 日志、指标、Buffer Pool 等通用能力 | 访问 GPIO/StdPeriph |
| `Platform` | `Platform` | CMSIS、标准外设库和启动文件 | 反向依赖项目业务 |

`Project`、`Middleware`、`Tests` 是辅助目录，不再作为运行时业务层级。

依赖方向：

```text
App -> Services -> BSP -> Platform
 |        |
 +------> Common
```

依赖只能沿箭头向下。野火 `bsp_*` API 是 BSP 内部实现细节，不向 Service/App 泄漏。

## 3. Phase 1 模块定义

### Camera Driver

- 目标：配置 OV5640，启停 DCMI + DMA 采集。
- 输入：分辨率、像素格式、JPEG 质量、目标 Buffer。
- 输出：初始化结果、帧完成事件、错误码。
- 依赖：SCCB、DCMI、DMA、XCLK、`board_config.h` 和野火底层驱动。
- 边界：不负责 LCD 绘制、网络发送或 RTOS 调度。

### Video Buffer

- 目标：使用固定内存池管理帧所有权。
- 输入：申请状态、帧元数据、释放请求。
- 输出：Buffer Handle 或明确错误。
- 依赖：临界区抽象，不依赖相机或 LCD。
- 边界：不动态分配图像内存，不允许无所有者的裸指针长期流转。

### Camera Service

- 目标：编排 Camera BSP 和 Video Buffer，实现相机状态机。
- 输入：Init/Start/Stop/Recover 请求，DMA 完成/错误事件。
- 输出：Ready Frame 事件和相机状态。
- 依赖：Camera BSP、Video Buffer、时间源。
- 边界：不直接访问 GPIO/StdPeriph。

### Display Driver / Display Service

- 目标：把 Ready Frame 呈现到 LCD。
- 输入：帧句柄和显示区域。
- 输出：显示完成/失败事件。
- 依赖：LCD Driver，必要时使用 JPEG 解码组件。
- 边界：不改写正在 CAPTURING/SENDING 的 Buffer。

## 4. 核心数据结构

- `VideoFrameMeta`：帧 ID、时间戳、宽高、格式、有效长度。
- `VideoBufferHandle`：索引和 generation，避免旧句柄误释放新帧。
- `VideoBufferState`：`FREE/CAPTURING/READY/DISPLAYING/SENDING/ERROR`。
- `CameraConfig`：分辨率、格式和 JPEG 质量。

## 5. 状态机

```text
Camera:  UNINITIALIZED -> IDLE -> STREAMING -> ERROR
Network: DISABLED -> INIT -> CONNECTING -> LOGIN -> CONNECTED -> BACKOFF
System:  BOOT -> INIT -> READY -> STREAMING -> ERROR
```

状态转移只能由显式事件驱动，不通过多个模块共享可写全局变量隐式触发。

## 6. Server 分层（后续）

```text
transport    HTTP / WebSocket / device TCP adapters
application  use cases and orchestration
domain       Device, Stream, Command entities and policies
infrastructure sockets, clocks, persistence and metrics adapters
```

Domain 不引用 FastAPI、WebSocket 或数据库对象。

## 7. Phase 1 过渡边界

当前已完成代码依赖分层和 Keil 分组重构，但为保持野火 4.3 寸例程的上板行为，采集数据仍由 DCMI DMA 直接写入 LCD FSMC 数据寄存器。因此 Phase 1 尚未具备真正的 Frame Buffer 所有权流转；`Components/buffer_pool` 是后续采集、显示、网络解耦的接口位置。完成硬件稳定性验收后，再将数据路径升级为：

```text
DCMI DMA -> Fixed Buffer Pool -> Display/Network consumers
```
