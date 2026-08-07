<div align="center">
  <h1>⚡️ TI MSPM0 AI Agent Skill</h1>
  <p><strong>专为全国大学生电子设计竞赛（NUEDC）及立创·天猛星 / 地猛星开发板（MSPM0G3507）与自定义 MSPM0G3519 开发板打造的 AI 编程神器与工程范式</strong></p>
  <p>还在四天三夜里死磕底层、查引脚冲突、移植驱动？让 Codex / Claude Code 替你接管 SysConfig 外设配置与 DriverLib 驱动，你只管专注算法与控制逻辑！</p>

  <p>
    <a href="https://github.com/Ibook000/mspm0-skill/stargazers"><img src="https://img.shields.io/github/stars/Ibook000/mspm0-skill?style=for-the-badge&logo=github" alt="Stars"></a>
    <a href="https://github.com/Ibook000/mspm0-skill/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge" alt="License"></a>
    <a href="https://www.ti.com/microcontrollers-mcus-processors/microcontrollers/arm-based-microcontrollers/arm-cortex-m0-mcus/overview.html"><img src="https://img.shields.io/badge/Platform-TI_MSPM0-red.svg?style=for-the-badge" alt="Platform"></a>
    <a href="https://github.com/Ibook000/mspm0-skill/actions"><img src="https://img.shields.io/badge/CI-Passing-brightgreen?style=for-the-badge" alt="CI"></a>
    <br>
    <a href="#"><img src="https://img.shields.io/badge/AI_Agent-Codex_%7C_Claude_Code-success?style=for-the-badge" alt="AI Ready"></a>
    <a href="#"><img src="https://img.shields.io/badge/SysConfig_Auditor-Verified-brightgreen?style=for-the-badge" alt="SysConfig Check"></a>
    <a href="https://github.com/Ibook000/mspm0-skill/issues"><img src="https://img.shields.io/github/issues/Ibook000/mspm0-skill?style=for-the-badge" alt="Issues"></a>
  </p>
</div>

---

## 📖 项目简介

**mspm0-skill** 是一套专为 TI MSPM0 微控制器（MSPG3507 / MSPM0G3507 / MSPM0G3519）设计的 AI Agent 工程规则集。它不是 SDK，不是库，也不是 IDE 插件——它是一份让 Codex、Claude Code 等编程 Agent 瞬间具备资深嵌入式工程师直觉的*行为规范*。

当 Agent 加载此 Skill 后，它会：
- 把 `.syscfg` 当作硬件配置的唯一信源，绝不动手修改生成文件
- 在修改引脚前自动检查板级约束（BSL、晶振、SWD、ROSC、板载外设）
- 根据当前板型（天猛星 / 地猛星 / 自定义 G3519）加载对应的引脚保护规则
- 使用已验证的 DriverLib 调用模式，而非猜测 API 名称
- 每次修改后运行静态检查脚本，在构建前发现风险

---

## 🎯 为什么电赛你需要它？

电赛四天三夜，时间就是生命。最怕**引脚配置冲突烧板子**、**误改生成代码程序跑飞**和**重头写外设库找 Bug**。

本 Skill 深度适配 **Codex**、**Claude Code** 等现代化终端与 IDE Agent，使其瞬间具备**资深 TI 嵌入式工程师的直觉与严谨**：

| 痛点 | 传统做法 | 有了 Skill 之后 |
| :--- | :--- | :--- |
| 引脚冲突 | 对着原理图手算，改错就是一次烧录 | Agent 自动读取 `.syscfg`，检查 BSL/晶振/SWD/ROSC 冲突 |
| 生成文件被改 | 编译通过了但跑飞，排查半天 | 强制规则：永远不碰 `ti_msp_dl_config.c/h` |
| 外设驱动从头写 | 抄数据手册自己撸，Bug 一个接一个 | 直接用仓库里的已验证示例移植 |
| 板型不匹配 | 天猛星的代码烧到地猛星上，灯不亮 | Agent 自动识别板型，适配对应引脚 |
| 焊接/接线问题 | 代码看起来没问题，就是跑不起来 | 检查清单提醒：电源、上拉、电平、TX/RX 交叉 |

---

## 📸 实战演示

<p align="center">
  <img src="assets/readme/msp0-installation.png" alt="Claude Code 加载 mspm0-skill 后，Skill 规则、参考资料、SysConfig 示例、工程示例与辅助脚本一目了然" width="48%">
  <img src="assets/readme/msp0-audit-report.png" alt="MSPM0 项目审查报告：Agent 自动检查 SysConfig 配置、引脚风险和代码健康度" width="48%">
</p>

---

## 🚀 快速开始

### 安装

```bash
# 克隆到你的项目旁边
git clone https://github.com/Ibook000/mspm0-skill.git
```

### 使用

在 Codex 或 Claude Code 中，对 Agent 说：

```text
请先读取 ./mspm0-skill/SKILL.md，再修改我的 MSPM0 项目。
```

然后直接提出你的需求，Agent 就会遵循 Skill 规则来工作：

```text
读取 mspm0-skill/SKILL.md。当前项目使用 MSPM0G3507 和 SysConfig。
请增加一路 1 kHz PWM 输出，不要修改 ti_msp_dl_config.c 或 ti_msp_dl_config.h。
先检查时钟、引脚冲突和现有外设占用，只修改必要的 .syscfg 与应用代码。
完成后说明检查、编译、烧录和硬件运行分别验证到了哪一步。
```

---

## ✨ 核心功能

### 1. 强制 SysConfig / DriverLib 研发规范

告知 Agent 你的需求，Agent 会自动修改 `.syscfg` 文件生成硬件配置，而不会去手动涂改底层驱动代码。

> 🗣️ **"帮我配一下天猛星板载的 PB22 呼吸灯（PWM），顺便把 PA10/PA11 串口打通跑个 Hello World"**
>
> 🗣️ **"我用地猛星，配一下 PA14 板载灯闪烁，再在 H3 扩展座上开一路 UART"**

### 2. 自动化引脚与配置防坑审计

利用 `scripts/check_syscfg.py` 静态诊断工具，Agent 能够在构建前自动发现关键问题：

- ⚠️ 避开 BSL (PA18)、晶振 (PA5/PA6)、调试接口 (PA20/PA19)、ROSC (PA2) 冲突
- 🔍 自动识别当前板型（天猛星 / 地猛星 / 自定义 G3519），加载对应引脚保护规则
- 📋 检查是否存在编译产物、Target CCXML 依赖及构建配置完整性

### 3. 全平台工具链支持

| 工具链 | 项目入口 | 烧录方式 |
| :--- | :--- | :--- |
| **CCS / CCS Theia** | `targetConfigs/*.ccxml` | DSLite + J-Link / XDS110 |
| **Keil / uVision** | `*.uvprojx` | J-Link / CMSIS-DAP |
| **CMake + GCC + OpenOCD** | `CMakeLists.txt` + `openocd.cfg` | OpenOCD (TI 分支) |
| **串口烧录 (免调试器)** | `.syscfg` + `main.c` | CH340E + BSL |

> ⚠️ **严禁使用 ST-LINK 进行下载或调试！** ST-LINK 会锁死 MSPM0 芯片。

---

## 📂 仓库结构

```text
mspm0-skill/
├── SKILL.md                          # 🔥 核心规则文件（Agent 入口）
├── README.md                         # 项目文档
├── scripts/                          # 辅助脚本工具箱
│   ├── check_syscfg.py               # SysConfig 静态检查与防坑审计
│   ├── list_examples.py              # 示例工程索引
│   ├── index_syscfg_examples.py      # SDK SysConfig 模块检索
│   ├── capture_example.py            # 从工程中提取 Example 包
│   ├── ccs_dss_debug.py              # CCS DSS 命令行调试
│   └── serial_console.py             # 串口调试终端
├── references/                       # 硬件参考文档
│   ├── MSPM0G3507_Pinout_Mapping.md  # 天猛星/地猛星引脚映射与避坑指南
│   ├── hardware_validation_notes.md  # 硬件调试经验手册
│   ├── sysconfig_ccs_workflow.md     # SysConfig & CCS 工作流
│   ├── driverlib_runtime_rules.md    # DriverLib 安全调用法则
│   ├── pin_occupation_table.md       # 引脚占用与复用表
│   ├── ccs_dss_debug.md              # 自动化调试指南
│   └── sdk_schema_lookup.md          # SDK Schema 查找指南
├── examples/                         # 可直接参考的工程示例
│   ├── empty_project                 # 基础工程骨架
│   ├── led_blink                     # GPIO 点灯（天猛星 PB22）
│   ├── pwm_breath_led                # 80MHz PWM 呼吸灯
│   ├── uart_blocking_tx              # 串口收发
│   └── oledui_full_g3519             # 综合示例：OLED UI + 陀螺仪 + 游戏
└── assets/                           # 媒体资源与配置代码片段
    ├── readme/                       # README 图片
    └── snippets/                     # .syscfg 外设配置速查片段
```

---

## 🛠️ 实用脚本一览

### SysConfig 静态检查

```bash
python3 scripts/check_syscfg.py examples/pwm_breath_led
```

一键扫描 `.syscfg`、生成文件、引脚冲突、工程入口和构建产物完整性。

### 列出内置示例

```bash
python3 scripts/list_examples.py
```

快速检索仓库中所有可参考的工程示例，含外设占用概览。

### 串口调试

```bash
# 列出可用端口
python3 scripts/serial_console.py --list

# 连接监控
python3 scripts/serial_console.py -p /dev/tty.usbserial-xxx -b 115200 --timestamp --duration 10
```

### 从用户工程提取 Example

```bash
python3 scripts/capture_example.py <project-dir> --name my-example --include "src/*.c"
```

---

## 🧩 电路板对比

| 特性 | 天猛星 (Tianmengxing) | 地猛星 (Dimengxing) |
| :--- | :--- | :--- |
| **MCU** | MSPM0G3507 LQFP-64 | MSPM0G3507 48-pin |
| **板载 LED** | PB22 (低电平亮) | PA14 (低电平亮，270 Ω 限流) |
| **USB-UART** | CH340E — PA10/PA11 | CH340E — PA10/PA11 |
| **SPI Flash** | — | W25Q32 (PB6~PB9) |
| **OLED** | 0.96/1.3寸 SPI (PB8/PB9/PB10/PB11/PB14/PB26) | 无板载，需外接 |
| **IMU** | LSM6DS3 (I2C — PA27/PA28) | 无板载，需外接 |
| **WS2812 RGB** | PB26 (TIMA1 CCP0) | 无板载，需外接 |
| **蜂鸣器** | PB27 (TIMG6 CCP1) | 无板载，需外接 |
| **无线模块** | UART7 (PB17/PB18) | 无板载，需外接 |
| **QEI 编码器** | PA29/PA30 + PA31 按键 | 无板载，需外接 |
| **扩展接口** | 双排扩展排针 | 双排 20pin 扩展座 (H3/H5) |
| **典型场景** | OLED 显示、IMU 姿态、无线通信、游戏机 | 精简控制、外接传感器、电机驱动 |

> 提示：天猛星示例（PB22 LED、OLED、IMU、WS2812、编码器）在地猛星上需适配到对应外接引脚使用。地猛星的 PA14 LED 与天猛星 PB22 LED 的驱动代码仅需修改 GPIO 引脚号和极性。

---

## 📚 参考文档一览

| 文档 | 内容 |
| :--- | :--- |
| [引脚映射与避坑指南](references/MSPM0G3507_Pinout_Mapping.md) | 天猛星 / 地猛星 通用引脚映射规范，含 BSL/晶振/SWD 避坑 |
| [硬件调试经验手册](references/hardware_validation_notes.md) | 实测硬件问题排查：HFXT、Flash、复位、高频干扰 |
| [SysConfig & CCS 工作流](references/sysconfig_ccs_workflow.md) | 官方标准工作流参考，含 CCS/Keil/CMake 布局 |
| [DriverLib 安全调用法则](references/driverlib_runtime_rules.md) | DriverLib API 使用规范、中断、时钟树、常见运行时错误 |
| [引脚占用与复用表](references/pin_occupation_table.md) | G3519 自定义板引脚占用一览 |
| [自动化调试指南](references/ccs_dss_debug.md) | CCS DSS 调试、断点、寄存器读写 |
| [SDK Schema 查找指南](references/sdk_schema_lookup.md) | SysConfig Schema 字段与 SDK 示例查找方法 |

---

## 🤝 如何参与贡献

这个项目还在持续完善中。如果你有跑通过的 `.syscfg` 片段、MSPG3507 或 MSPM0G3519 示例、天猛星/地猛星引脚修正、CCS 与 Keil 的实战经验，欢迎参与：

- **提交 Issue**：发现错误、建议新功能、报告问题 → [New Issue](https://github.com/Ibook000/mspm0-skill/issues/new)
- **提交 PR**：修复 Bug、新增示例、完善文档 → [Pull Requests](https://github.com/Ibook000/mspm0-skill/pulls)
- **分享经验**：在 [Discussions](https://github.com/Ibook000/mspm0-skill/discussions) 里分享你的电赛经验

即使只是指出一个宏名或板载资源写错了，也很有用。

---

## 📢 推广与交流

如果你觉得这个项目对你有帮助，欢迎：

- ⭐ 点个 **Star** 让更多电赛战友看见
- 🔗 分享给你的队友和同学
- 📝 在知乎、CSDN、掘金等平台写文章推荐
- 💬 在 [Discussions](https://github.com/Ibook000/mspm0-skill/discussions) 里交流使用心得

---

## 📄 License

[MIT](LICENSE)

---

## 🏆 致谢

- 感谢 **TI** 提供的 MSPM0 系列微控制器和完善的 DriverLib / SysConfig 工具链
- 感谢 **嘉立创** 推出的天猛星 / 地猛星开发板，降低了 MSPM0 的入门门槛
- 感谢所有参与电赛的同学们，你们的拼搏精神是这个项目存在的意义

---

<div align="center">
  <p>祝各位同学在国赛/省赛中：<b>一次过编，不冒白烟！顺利拿奖！🏆</b></p>
  <p>加油，电赛人！</p>
  <br>
  <sub>
    <a href="https://github.com/Ibook000/mspm0-skill">GitHub</a> ·
    <a href="https://github.com/Ibook000/mspm0-skill/issues">反馈</a> ·
    <a href="https://github.com/Ibook000/mspm0-skill/discussions">讨论</a>
  </sub>
  <br>
  <sub>如果觉得好用，请点个 <b>⭐ Star</b></sub>
</div>
