# Phase 2 集成说明

## 软件基线

| 模块 | 版本 | 说明 |
| --- | --- | --- |
| FreeRTOS | 9.0.0 | ARM_CM4F RVDS/ARMCC5 port，heap_4 |
| lwIP | 2.1.2 | FreeRTOS sys_arch、DHCP、TCP/UDP、Socket/Netconn |
| LVGL | 8.3.11 | RGB565，ILI9806G 800x480 display port |
| Ethernet | STM32F4x7 legacy driver | LAN8720 RMII，STM32 标准外设库 |

## 运行时关系

```text
FreeRTOS scheduler
├─ ETH-RX (priority 6) -> Ethernet DMA RX -> lwIP tcpip_input
├─ lwIP  (priority 5) -> timers / DHCP / TCP-IP core
└─ RVM-App (priority 4)
   ├─ camera service
   ├─ network link/status service
   └─ LVGL timer handler
```

Ethernet 中断优先级为 6，低于 `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5` 的屏蔽边界，因此可以安全调用 FreeRTOS FromISR API。

## 首次上板检查

1. 在 Keil 下载 `RVM_F407_Phase2`。
2. 确认 LCD 显示 `RVM Phase 2` 和三套中间件名称。
3. 确认 OV5640 预览区域持续刷新。
4. 接入带 DHCP 的路由器或交换网络。
5. 等待 LCD 从 `link down` / `DHCP...` 更新为 IPv4 地址。
6. 在 PC 上 ping LCD 显示的地址，并连续拔插一次网线验证恢复。

若设备上电时未连接网线，网络服务会每 2 秒重试 PHY 初始化；后续插入网线后应自动进入 DHCP 流程，无需重启设备。

## 已知边界

- 当前 MAC 地址是本地管理地址 `02:40:7A:10:00:01`；多块设备同时接入前必须生成唯一地址。
- 当前视频数据仍由 DCMI DMA 直接写 LCD，网络栈尚未发送视频帧。
- UI 刷新时会短暂停止并恢复相机 DMA，以避免 LCD FSMC 写入竞争。
- 编译验证不能替代目标板电气、PHY 时钟、网线和 DHCP 环境验证。
