# OV5640 Camera Adapter

当前版本：**AssemblyFixed v2**。PCB 布线与装配位置已整理，等待实板焊接、供电和摄像头测试。

- [最新嘉立创 EDA 专业版工程](pcb/routed/assembly-fixed-v2/STM32F407_OV5640_Adapter_AssemblyFixed_v2.epro2)
- [版本说明及验证结果](pcb/routed/assembly-fixed-v2/README.md)
- [采购与装配 BOM](bom/bom.csv)
- [从当前 PCB 提取的逐焊盘网络表](docs/pin-mapping.csv)
- [硬件说明](docs/hardware-design-guide.md)
- [装配与上电检查清单](manufacturing/preflight-checklist.md)

板框约 30.047 × 26 mm，双层板。J1 为底面 2×10 排针，连接用户指定的霸天虎 V2 J7；J2 为顶面 2×9 排母，连接 OV5640 模块。两者间距均为 2.54 mm。实物接口方向和供电脚需在上电前测量核对；网络表记录工程连接，不代表实物测量结论。

采购时 U1 使用 **LDL1117S33R（SOT-223，3.3V）**，以配合 C1/C2 陶瓷电容。工程内原器件属性仍为 LD1117S33TR；这是明确记录的同封装、同引脚装配替代，自动导出的 BOM 需按采购清单修正。

原始工程、旧布线版、中间文件及依赖旧输入的脚本保留在本地但不纳入 Git；仓库只保存最新工程及配套资料。验证报告不等同于实物功能测试，也不代表最终 v2 的原生 DRC 已归档。
