# Windows Qt 串口图像客户端

这是远程视频监控系统 PC 客户端的第一阶段版本。当前通过串口接收 STM32F407
输出的日志与图像帧，后续可在保留 UI 的基础上加入 TCP 和实时视频功能。

## 当前功能

- 自动扫描 Windows 串口并显示设备描述
- 串口连接与断开
- 常用波特率选择，默认 `115200`
- 文本或十六进制收发
- 可选 `LF`、`CRLF` 行尾
- 接收时间戳、收发字节计数与日志清空
- 串口异常和设备拔出提示
- JPEG、RGB565 图像帧解析与显示
- 图像帧 CRC32 校验、断包/粘包处理和错误重同步
- 当前帧保存为 PNG 或 JPEG
- JPEG 回环测试画面

串口固定使用 `8-N-1`、无流控，与现有固件调试串口设置一致。

## 开发环境

已验证的环境为：

- Qt 6.8.3 MinGW 64-bit
- MinGW 13.1
- Qt Serial Port
- CMake
- Ninja

本机自动安装目录为 `D:\Qt`。也可以安装 Qt Creator，或者直接使用下面的命令行构建。

编译器必须与 Qt 套件匹配，不能用 MinGW 编译器链接 MSVC 版 Qt。

## 使用 Qt Creator 构建

1. 在 Qt Creator 中选择“打开项目”。
2. 打开本目录的 `CMakeLists.txt`。
3. 选择安装了 `Qt Serial Port` 的 Desktop Qt 6 Kit。
4. 点击“配置项目”，然后点击左下角绿色运行按钮。

## 使用命令行构建

当前电脑可以直接使用一键脚本完成 Release 构建、测试和打包：

```powershell
cd "D:\Remote Video Monitoring System\f407-camera-system\pc-client"
.\build.ps1 -Configuration Release -Test -Package
```

生成的独立程序位于：

```text
dist\Release\bin\RvmSerialClient.exe
```

如果 PowerShell 禁止执行本地脚本，可以仅对当前进程临时放行：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
```

通用 CMake 构建方式如下。需先打开 Qt 对应编译器的终端，或者把 Qt、CMake、Ninja
加入当前进程的 `PATH`：

```powershell
cd "D:\Remote Video Monitoring System\f407-camera-system\pc-client"
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
.\build\RvmSerialClient.exe
```

如果 CMake 找不到 Qt，可显式指定 Qt 安装目录：

```powershell
cmake -S . -B build -G Ninja `
  -DCMAKE_PREFIX_PATH="D:\Qt\6.8.3\mingw_64" `
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

运行解析器测试：

```powershell
ctest --test-dir build --output-on-failure
```

生成可分发目录：

```powershell
cmake --install build --prefix dist
```

## STM32F407 接线

现有固件的调试串口是 USART1：

| STM32F407 | USB 转串口模块 |
| --- | --- |
| PA9 / USART1_TX | RX |
| PA10 / USART1_RX | TX |
| GND | GND |

使用 **3.3 V TTL 电平**，TX/RX 交叉连接并确保共地。不要把 RS-232 电平直接接到
STM32，也不要把 5 V TTL 信号接入 PA9/PA10。

串口参数：`115200 / 8 数据位 / 无校验 / 1 停止位 / 无流控`。

连接开发板后按一下复位键，当前固件会主动通过 USART1 输出启动日志。正常情况下，
客户端接收区会看到类似内容：

```text
[INFO][System] ... boot
[INFO][Camera] OV5640 detected: ...
[INFO][System] streaming ... RGB565
```

如果摄像头尚未连接，出现 `[ERROR][Camera] OV5640 initialization failed: ...` 也说明
PC 串口接收链路是正常的；它表示摄像头初始化失败，而不是串口失败。

当前 STM32 固件尚未实现串口图像发送和命令回显，因此现有固件只能在日志区输出文本，
不会让左侧图框自动出图。固件后续需要按照 [串口图像协议](SERIAL_IMAGE_PROTOCOL.md)
封装并发送图像。

## 无开发板时的自检

可以先验证 PC 客户端和 USB 转串口模块：

1. 将 USB 转串口模块的 `TX` 与 `RX` 短接。
2. 插入电脑，在客户端刷新并连接对应 COM 口。
3. 输入 `hello` 并发送，日志中应先显示 `TX hello`，随后显示 `RX hello`。
4. 点击“发送测试帧”。等待完整帧回传后，左侧应显示带时间的测试图像。

完成测试后先断开串口，再移除 TX/RX 短接线。

在 `115200` 波特率下图像传输较慢，约为 11.5 KB/s。正式传图建议优先使用 JPEG，
并在固件和客户端两端验证后提高到 `921600` 或更高波特率。未经压缩的
`320 × 240 RGB565` 单帧为 153600 字节，不适合在 115200 下连续传输。
