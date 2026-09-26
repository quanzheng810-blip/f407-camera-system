# ADR-0001：使用分层 Monorepo

- 状态：Accepted
- 日期：2026-09-26

## 背景

项目同时包含 MCU 固件、服务器和多客户端，但开发必须按 Phase 推进，不能让后期功能污染当前 Camera Bring-up。

## 决策

使用单一 Monorepo 管理协议和端到端版本，各产品保持独立的构建、测试和依赖边界。Firmware 使用 `App -> Services -> BSP -> Platform` 主链路，通用能力放在 `Common`；`Middleware` 只存放第三方中间件。

## 影响

- 协议变更可在一个提交中同时更新设备和客户端。
- 大型厂商资料不进 Git，只记录来源和选用版本。
- 任何跨层调用需要先修改本 ADR 或新增 ADR。
