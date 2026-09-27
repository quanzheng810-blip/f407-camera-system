# Hardware

本目录存放项目自研硬件、转接板及其设计说明，与 `firmware/` 和仓库外部参考资料分开管理。

```text
hardware/
└─ camera-adapter/
   ├─ README.md
   ├─ docs/               设计、接线与测试指南
   ├─ bom/                器件清单
   ├─ pcb/                用户自行建立的立创 EDA/KiCad PCB 工程
   └─ manufacturing/      打样前检查清单
```

当前硬件项目：STM32F407 霸天虎 V2 的 2x10 摄像头座到 FD5640-500W-V11 2x9 摄像头模块的转接板。
