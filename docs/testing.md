# 测试与验收规范

## 测试层级

- Host unit tests：协议解析、CRC32、Buffer Pool 状态转移、重连退避。
- Target component tests：SCCB 读写、SDRAM/SRAM 测试、DCMI DMA、LCD 填充和 Ethernet loopback。
- Integration tests：Camera -> Buffer -> LCD，Camera -> TCP -> Server。
- Soak tests：30 分钟起步，后续提高到 8/24 小时。

## 每个变更必须记录

1. 测试步骤。
2. 预期结果。
3. 实际结果。
4. Debug 方法和所用观测点。
5. 验收结论及性能数据。

## 必要观测项

`capture_fps`、`display_fps` 、`send_fps`、`frame_size`、`frame_drop`、`camera_error`、`network_error`、`network_kbps`、堆栈高水位和 Buffer Pool 占用率。
