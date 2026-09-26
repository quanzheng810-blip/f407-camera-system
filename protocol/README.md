# Device Protocol

线上数据采用明确字节序列化，不直接发送 C struct，避免对齐、padding 和大小端问题。

## 初始帧格式

```text
MAGIC | VERSION | TYPE | HEADER_LENGTH | DEVICE_ID | FRAME_ID
TIMESTAMP | WIDTH | HEIGHT | FORMAT | PAYLOAD_LENGTH | PAYLOAD | CRC32
```

- Magic：`0x56494430` (`VID0`)
- 网络字节序：big-endian
- 最大 payload 必须在编译期和服务器端同时设置上限。
- CRC32 覆盖范围和多项式在 Phase 4 实现前固定。
- 解析器必须支持粘包、拆包和错误 Magic 重同步。

## 消息类型

```text
0x01 DEVICE_LOGIN
0x02 HEARTBEAT
0x10 VIDEO_FRAME
0x11 IMAGE_FRAME
0x20 DEVICE_STATUS
0x30 COMMAND
0x31 COMMAND_ACK
```

协议正式实现时必须附带 golden test vectors，让 C 和 Python 实现使用同一组测试数据。
