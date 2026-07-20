# TI MSPM0 Agent Skill

本技能包专为 AI Agent（如 Codex, Claude Code, Cursor 等）设计，提供 TI MSPM0 系列单片机开发的规则集与辅助脚本。

通过这个 Skill，AI 能够理解 MSPM0 的 SysConfig 配置流、DriverLib 底层库，并能够规避常见的硬件引脚坑，大幅提升 AI 协助开发嵌入式固件的准确率。

## ✨ 核心能力

- **⚙️ SysConfig 配置解析**：教会 AI 将 `.syscfg` 文件作为配置的唯一真相源（Source of Truth），并提供脚本检查其正确性，禁止 AI 手工乱改生成的 `ti_msp_dl_config.h` 等文件。
- **🔨 多工具链支持**：适配 Code Composer Studio (CCS)、Keil/uVision 以及 CMake/GCC/OpenOCD 开发流。
- **🛡️ 硬件防坑指南**：内置特定开发板（如 MSPM0G3519、MSPM0G3507 等）的引脚复用规则。比如明确标记出用于调试的 SWDIO/SWCLK 引脚、晶振引脚和 BSL 启动引脚，防止 AI 错误分配导致芯片锁死。
- **📚 DriverLib 规范**：确保 AI 生成的初始化代码和外设调用（GPIO、UART、PWM、ADC 等）符合 TI 官方的最佳实践。

## 📁 目录结构

```text
msp0-skill/
├── SKILL.md                 # Agent 核心激活规则与执行协议
├── assets/                  # 辅助文档资源
├── examples/                # JSON 配置或代码示例
├── scripts/                 # 提供给 Agent 验证和扫描 SysConfig 的脚本
└── references/              # 知识库
    ├── sysconfig_ccs_workflow.md    # CCS 和 SysConfig 工作流指南
    ├── driverlib_runtime_rules.md   # 运行时的 DriverLib 调用规范
    ├── hardware_validation_notes.md # 硬件调试与验证指南
    ├── sdk_schema_lookup.md         # SDK Schema 查询表
    ├── pin_occupation_table.md      # 开发板引脚占用表
    └── MSPM0G3507_Pinout_Mapping.md # G3507 引脚映射规范
```

## 🚀 Agent 工作流

当用户触发此技能时，Agent 将遵循以下标准流程：
1. 定位项目中的 `.syscfg` 文件以及工具链入口文件（如 `.ccxml`、`.uvprojx` 或 `CMakeLists.txt`）。
2. 调用 `scripts/check_syscfg.py` 脚本读取配置的元数据（设备型号、引脚分配、时钟、中断等）。
3. 分析生成的 `ti_msp_dl_config.h` 获取宏定义与中断名。
4. 安全地修改 `.syscfg` 及用户级应用代码，绝不触碰编译生成的中间文件。
5. 通过官方工具链重新生成配置和固件。

## ⚠️ 注意事项（Agent 铁律）

- **绝对禁止**手工修改以 `ti_msp_dl_config` 命名的文件。
- 在分配任何引脚之前，**必须**检查硬件防坑表，避开调试口 (PA19/PA20)、晶振口 (PA5/PA6) 以及 BSL 引脚 (PA18)。
- 初始化函数应始终依赖生成的配置（如 `SYSCFG_DL_init()`），不要自行发明名称。
