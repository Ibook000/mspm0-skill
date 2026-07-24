<div align="center">
  <h1>⚡️ TI MSPM0 AI Agent Skill (Codex & Claude Code 常用版)</h1>
  <p><strong>专为全国大学生电子设计竞赛（NUEDC）打造的 TI MSPM0 AI 编程神器与工程范式</strong></p>
  <p>还在四天三夜里死磕底层、查引脚冲突、移植驱动？让 Codex / Claude Code 替你接管 SysConfig 外设配置与 DriverLib 驱动，你只管专注算法与控制逻辑！</p>

  [![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
  [![Platform](https://img.shields.io/badge/Platform-TI_MSPM0-red.svg)](https://www.ti.com/microcontrollers-mcus-processors/microcontrollers/arm-based-microcontrollers/arm-cortex-m0-mcus/overview.html)
  [![AI Ready](https://img.shields.io/badge/AI_Agent-Codex%20%7C%20Claude%20Code-success)](#)
  [![SysConfig Check](https://img.shields.io/badge/SysConfig_Auditor-Verified-brightgreen)](#)
</div>

---

## 🎯 为什么电赛你需要它？

电赛四天三夜，时间就是生命。最怕**引脚配置冲突烧板子**、**误改生成代码程序跑飞**和**重头写外设库找 Bug**。
本 Skill 深度适配 **Codex**、**Claude Code** 等现代化主流终端与 IDE Agent，使其瞬间具备**资深 TI 嵌入式工程师的直觉与严谨**：

*   🔥 **拒绝踩坑，一发入魂**：内置 **[立创·天猛星 MSPM0G3507 / MSPM0G3519]** 引脚避坑指南，Agent 在修改配置前会主动规避 PA18 BSL、PA5/PA6 晶振、SWD 仿真引脚，杜绝“改完配置板子变砖”的惨剧。
*   🚀 **开箱即用，自带轮子**：内置丰富的工程示例，涵盖 OLED 显示驱动、六轴陀螺仪 (LSM6DS3) 解算、正交编码器 (QEI)、PWM 呼吸灯、串口通信及完整带 UI 的小游戏框架代码。
*   🧠 **底层绝对安全**：强制锁定 SysConfig 工作流，严禁 Agent 手动修改 `ti_msp_dl_config.h` 等生成文件，从源头切断“编译过了但程序跑飞”的底层逻辑灾难。
*   🛠️ **自带静态检查脚本**：随带工具链可一键扫描 `.syscfg`，检查外设冲突、时钟配置、缺失引脚与构建产物，让 Agent 在代码提交前自检保命。

---

## 📸 实战演示效果

有了 MSPM0 Skill 加持，你的 Codex / Claude Code 就是无情的干活与审查机器：

<p align="center">
  <img src="assets/readme/msp0-installation.png" alt="MSPM0 Agent Skill 安装与运行演示" width="48%">
  <img src="assets/readme/msp0-audit-report.png" alt="MSPM0 Agent 审查报告演示" width="48%">
</p>

---

## 📦 如何快捷安装与配置 (Codex / Claude Code)

### 1. 对于 Codex 用户 (推荐)
直接将本仓库 clone 到本地或项目中，或者在 Codex 中加载 Skill：
```bash
# 在终端中全局安装或放入项目目录
git clone https://github.com/Ibook000/mspm0-skill.git

# 在 Codex 中即可直接调用或关联 SKILL.md
```

### 2. 对于 Claude Code 用户
在项目根目录拉取仓库后，对 Claude Code 输入以下指令加载环境规则：
```bash
git clone https://github.com/Ibook000/mspm0-skill.git

# 终端提示词：
"Please load the MSPM0 Agent Skill from ./mspm0-skill/SKILL.md to setup my project."
```

### 3. 对于 Cursor / VS Code 用户
可将 [SKILL.md](SKILL.md) 的内容复制到项目根目录下的 `.cursorrules` 文件中使用。

---

## ✨ 核心功能与工作流规则

### 1. 强制 SysConfig / DriverLib 研发规范
告知 Agent 你的需求，Agent 会自动修改 `.syscfg` 文件生成硬件配置，而不会去手动涂改底层驱动代码。
> 🗣️ **“帮我配一下天猛星板载的 PB22 呼吸灯（PWM），顺便把 PA10/PA11 串口打通跑个 Hello World”**

### 2. 自动化引脚与配置防坑审计
利用 `scripts/check_syscfg.py` 静态诊断工具，Agent 能够在构建前自动发现关键问题：
- ⚠️ 避开 BSL (PA18)、晶振 (PA5/PA6)、调试接口 (PA20/PA19) 冲突。
- 🔍 自动核对 `SYSCFG_DL_init()` 调用与外设宏命名。
- 检查是否存在编译产物、Target CCXML 依赖及构建配置完整性。

### 3. 全平台工具链满血支持
不论你们团队习惯用 **Code Composer Studio (CCS / CCS Theia)**、**Keil/uVision**，还是极客风的 **CMake + GCC + OpenOCD**，本 Skill 通通原生适配。

---

## 📂 仓库目录结构

```text
mspm0-skill/
├── SKILL.md                          # Skill 核心规则文件（Codex / Claude Code 识别入口）
├── README.md                         # 项目主文档
├── scripts/                          # 辅助脚本工具箱
│   ├── check_syscfg.py               # SysConfig 静态检查与防坑审计工具
│   ├── list_examples.py              # 列出内置示例工程与外设映射
│   ├── index_syscfg_examples.py      # SysConfig 模块模式索引脚本
│   ├── capture_example.py            # 从现成工程中提取与打包 Example 脚本
│   ├── ccs_dss_debug.py              # CCS DSS 命令行调试与自动烧录集成
│   └── serial_console.py             # 嵌入式串口通信与日志控制台
├── references/                       # 硬件防坑与硬核参考文档
│   ├── MSPM0G3507_Pinout_Mapping.md  # 天猛星防砖引脚映射与避坑指南
│   ├── hardware_validation_notes.md  # 硬件调试玄学问题排查手册
│   ├── sysconfig_ccs_workflow.md     # 官方标准 SysConfig & CCS 工作流参考
│   ├── driverlib_runtime_rules.md    # TI DriverLib 运行时调用避坑法则
│   ├── pin_occupation_table.md       # 引脚占用与复用一览表
│   ├── ccs_dss_debug.md              # 自动化调试与自动化测试指南
│   └── sdk_schema_lookup.md          # SDK 语法与 SysConfig Schema 查找指南
├── examples/                         # 可直接移植/抄作业的开箱即用工程示例
│   ├── empty_project                 # 基础工程骨架 (SYSCTL + Board)
│   ├── led_blink                     # GPIO 基础控制 (PB22 LED)
│   ├── pwm_breath_led                # 80MHz 高阶 PWM 呼吸灯
│   ├── uart_blocking_tx              # 80MHz 串口阻塞式收发 (PA10/PA11)
│   └── oledui_full_g3519             # 80MHz 综合示例 (OLED UI + LSM6DS3 陀螺仪 + QEI 编码器 + 按键 + 游戏)
└── assets/                           # 常用配置代码片段 (Snippets) 与 README 媒体资源
    └── snippets/                     # .syscfg 常用外设配置速查片段
```

---

## 🛠️ 实用工具与脚本使用指南

### 1. SysConfig 配置静态检查 (`check_syscfg.py`)
检查当前项目是否存在引脚冲突、生成文件修改风险或配置文件缺失：
```bash
python3 scripts/check_syscfg.py examples/pwm_breath_led
```

### 2. 查看内置 Example 列表与外设占用 (`list_examples.py`)
快速检索当前仓库中可供参考的代码示例：
```bash
python3 scripts/list_examples.py
```

### 3. 串口调试助手 (`serial_console.py`)
在 Agent 命令行环境下直接监控单片机串口输出：
```bash
python3 scripts/serial_console.py --port /dev/tty.usbserial-xxx --baud 115200
```

---

## 📁 硬件保命与极客指路

在 `references/` 目录中，我们为你准备了电赛备赛的**防砖与玄学排查指南**：
- 📌 [MSPM0G3507_Pinout_Mapping.md](references/MSPM0G3507_Pinout_Mapping.md) —— **天猛星防砖引脚映射规范**
- 📌 [hardware_validation_notes.md](references/hardware_validation_notes.md) —— 硬件调试玄学问题排查手册
- 📌 [sysconfig_ccs_workflow.md](references/sysconfig_ccs_workflow.md) —— 官方标准工作流参考
- 📌 [driverlib_runtime_rules.md](references/driverlib_runtime_rules.md) —— DriverLib API 安全调用指南

在 `examples/` 目录下：
- 🎮 [oledui_full_g3519](examples/oledui_full_g3519/README.md) —— 直接可以抄的完整带 UI 和小游戏的大型工程参考！

---

<div align="center">
  <p>祝各位同学在国赛/省赛中：<b>一次过编，不冒白烟！顺利拿奖！🏆</b></p>
  <sub>如果觉得好用，请帮忙点个 <b>Star ⭐️</b> 让更多电赛战友看见！</sub>
</div>
