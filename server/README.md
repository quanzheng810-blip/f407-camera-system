# Server

计划技术栈：Python、FastAPI、asyncio、WebSocket。Phase 4 之前不引入数据库、账号系统或复杂权限。

目标分层：

```text
src/rvm_server/domain/          设备、流、命令实体与规则
src/rvm_server/application/     用例和编排
src/rvm_server/infrastructure/  TCP、时钟、指标和持久化适配
src/rvm_server/transport/       FastAPI/REST/WebSocket 边界
tests/                          单元与集成测试
```
