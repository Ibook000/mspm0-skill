<div align="center">
  <h1>⚡️ TI MSPM0 AI Agent Skill</h1>
  <p><strong>专为全国大学生电子设计竞赛（NUEDC）及立创·天猛星 / 地猛星 / 地正星开发板（MSPM0G3507 / L1306）与自定义 MSPM0G3519 打造的 AI 编程 Skill</strong></p>
  <p>让 Codex / Claude Code 替你接管 SysConfig 外设配置与 DriverLib 驱动，你只管专注算法与控制逻辑。</p>

  <p>
    <a href="https://github.com/Ibook000/mspm0-skill/stargazers"><img src="https://img.shields.io/github/stars/Ibook000/mspm0-skill?style=for-the-badge&logo=github" alt="Stars"></a>
    <a href="https://github.com/Ibook000/mspm0-skill/blob/main/LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg?style=for-the-badge" alt="License"></a>
    <a href="https://www.ti.com/microcontrollers-mcus-processors/microcontrollers/arm-based-microcontrollers/arm-cortex-m0-mcus/overview.html"><img src="https://img.shields.io/badge/Platform-TI_MSPM0-red.svg?style=for-the-badge" alt="Platform"></a>
    <a href="https://github.com/Ibook000/mspm0-skill/blob/main/SKILL.md"><img src="https://img.shields.io/badge/AI_Agent-SKILL-success?style=for-the-badge" alt="AI Ready"></a>
  </p>
</div>

> **来源与版权**：本仓库基于 [mc3545dada/mspm0-skill](https://github.com/mc3545dada/mspm0-skill) 的历史版本修改而来，相关原始内容受 MIT License 许可，并保留版权声明 `Copyright (c) 2026 mc3545dada`。本仓库在此基础上重写了定位、补充了板卡资料与示例、调整了工具脚本。具体以提交历史为准。

---

## 这是什么？

**mspm0-skill** 是一套面向 AI Agent（Codex / Claude Code / 兼容工具）的 TI MSPM0 嵌入式开发**规则集**。它不是 SDK、不是库、也不是 IDE 插件，而是一份让编程 Agent 具备资深嵌入式工程师直觉的*行为规范*。

Agent 加载本 Skill 后会：

- 把 `.syscfg` 当作硬件配置的唯一信源，**绝不手工修改** `ti_msp_dl_config.c/.h` 等生成文件
- 修改引脚前自动检查板级约束（BSL、晶振、SWD、ROSC、板载外设）
- 按当前板型（天猛星 / 地猛星 / 地正星 / 自定义 G3519）加载对应引脚保护规则
- 用已验证的 DriverLib 调用模式，而非猜测 API 名称
- 每次修改后跑静态检查，在构建前发现风险

**支持平台**：MSPM0G3507、MSPM0G3519（custom-mspm0g3519）、MSPM0L1306（地正星）；板卡规则位于 `boards/*.json`（机器可读唯一数据源）。

---

## ✨ 核心亮点

| 能力 | 解决的问题 | 对应实现（本仓库均有） |
| :--- | :--- | :--- |
| 🧠 **直接理解 SysConfig** | 检查/修改 `.syscfg`，保留 metadata/时钟/PinMux/DMA/中断，**不篡改**生成文件 | `scripts/check_syscfg.py` + `SKILL.md` |
| 🔧 **识别工程与工具链** | 区分 CCS / Keil / CMake+GCC+OpenOCD / FreeRTOS 工程并给对应路径 | `SKILL.md`「Project Shape」 |
| ⚡ **构建与烧录指引** | CCS/Keil/CMake 构建与烧录入口、DSLite 与 OpenOCD 示例 | `references/sysconfig_ccs_workflow.md` |
| 🖥️ **CCS-DSS 调试** | 通过 CCS DSS 连真实硬件，做探针探测/断点/寄存器/复位 | `scripts/ccs_dss_debug.py` |
| 📡 **串口收发** | Python 串口控制台，文本/十六进制收发、时间戳 | `scripts/serial_console.py` |
| 📚 **例程与 SDK 检索** | GPIO/PWM/Timer/UART 等示例 + 检索本地 TI SDK 例程 | `scripts/list_examples.py` + `index_syscfg_examples.py` + `examples/` |

---

## 📦 快速安装

一行安装（需 Node.js 环境）：

```bash
npx skills add Ibook000/mspm0-skill
```

也可以只复制本仓库根目录（**本仓库根目录即 skill 本体**）到对应 Agent 的 skills 目录：

| Agent | 安装位置 |
| :--- | :--- |
| Claude Code | `~/.claude/skills/mspm0-skill/` |
| Codex 等 | `~/.agents/skills/mspm0-skill/` |
| OpenClaw | `~/.openclaw/skills/mspm0-skill/` |
| CodeBuddy Code | `~/.codebuddy/skills/mspm0-skill/` |

> 安装后的 skill 目录名为 `mspm0-skill`。装好可在 Agent 中用 `/skills`（或等价命令）确认已加载。**规则与参考文档是纯 Markdown，不装依赖也能用**；只有要运行 `check_syscfg.py` 等辅助脚本时才需要 `pip install -r requirements-dev.txt`。

Windows PowerShell 手动安装：

```powershell
New-Item -ItemType Directory -Force "$env:USERPROFILE\.claude\skills" | Out-Null
Copy-Item -Recurse -Force .\mspm0-skill "$env:USERPROFILE\.claude\skills\mspm0-skill"
```

---

## 🤖 让 AI 使用这个 Skill

本仓库是 AI Agent Skill，**真正的规则入口是 [`SKILL.md`](SKILL.md)**。Agent 被加载后的标准动作：

1. 先读 `SKILL.md`（权威入口：工作流、修改边界、验证口径）
2. 识别板卡（`.syscfg` 的 `--device/--package` 或用户说明）→ 读取对应 `boards/*.json` 与 `references/boards/<board>.md`
3. 以 `.syscfg` 为唯一信源，只改 `.syscfg` / 应用代码，**绝不手工改生成文件**
4. 改前跑 `python scripts/check_syscfg.py <dir> --board <board-id>` 检查引脚冲突
5. 把「源码 / SysConfig 校验 / 编译 / 烧录 / 真实硬件」**分层**报告；没真板子不得称“硬件已验证”

在 Agent 中直接说：

```text
读取 mspm0-skill/SKILL.md，按里面的规则工作。
当前项目：MSPM0G3507，板卡是立创·天猛星。
需求：用 PB22 板载 LED 做 1 kHz 呼吸灯；不要改 ti_msp_dl_config.c/.h。
步骤：先跑 python scripts/check_syscfg.py <项目目录> --board tianmengxing 检查引脚冲突，
      只改 .syscfg 与应用代码，改完再跑一次检查。
最后把「源码/校验/编译/烧录/硬件」各自验证到哪一步分开说明。
```

按问题查资料：[`docs/topic-guide.md`](docs/topic-guide.md) · 验证状态释义：[`docs/verification-levels.md`](docs/verification-levels.md)。

机器可读索引：[`llms.txt`](llms.txt) · [`docs/project-index.json`](docs/project-index.json)。

---

## 🧩 电路板对比（速览）

| 特性 | 天猛星 (tianmengxing) | 地猛星 (dimengxing) | 地正星 (dizhengxing) | 自定义 MSPM0G3519 (custom-mspm0g3519) |
| :--- | :--- | :--- | :--- | :--- |
| **MCU** | MSPM0G3507 LQFP-64 | MSPM0G3507 48-pin | MSPM0L1306 VQFN-32 | MSPM0G3519 LQFP-64 |
| **板载 LED** | PB22（低电平亮） | PA14（低电平亮，270Ω） | PA14（高电平亮） | PB22（低有效，示例） |
| **USB-UART** | CH340E — PA10/PA11 | CH340E — PA10/PA11 | CH340E — PA22/PA23 | CH340E — PA10/PA11（示例） |
| **SPI Flash** | 板载 SPI Flash (PB6~PB9) | W25Q32 (PB6~PB9) | 无，需外接 | W25Q128 (PB6~PB9，示例） |
| **OLED** | 板载 OLED（引脚以原理图/板卡资料为准） | 需外接 | 需外接 | I2C (PA0/PA1，示例) |
| **IMU** | LSM6DS3 (PA27/PA28) | 需外接 | 需外接 | LSM6DS3 (PA27/PA28，示例) |
| **WS2812 RGB** | PB26 (TIMA1 CCP0) | 需外接 | 需外接 | PB26（示例） |

> 完整引脚占用、默认电平与避坑说明见 [`references/boards/`](references/boards/)。地猛星/地正星/自定义 MSPM0G3519 的板载外设多为示例连接，在外接板上需适配到对应引脚；地正星仅 GPIOA 端口且 LED 高电平点亮，跨板移植务必核对极性与可用引脚。自定义 MSPM0G3519 为自定义/示例板，板载资源以用户原理图为准。

---

## 📚 详细文档（按需取用，链接即全文）

| 文档 | 内容 |
| :--- | :--- |
| [**SKILL.md**](SKILL.md) | 🔥 Agent 规则入口：工作流、修改边界、验证口径、脚本命令全集 |
| [**boards/**](boards/) | 机器可读的板卡识别 / 引脚 / 板载资源（唯一数据源） |
| [**references/boards/**](references/boards/) | 各板卡引脚与板载资源规则（由 JSON 生成） |
| [MSPM0G3507_Pinout_Mapping](references/MSPM0G3507_Pinout_Mapping.md) | 天猛星/地猛星引脚映射与避坑 |
| [sysconfig_ccs_workflow](references/sysconfig_ccs_workflow.md) | SysConfig & CCS/Keil/CMake 工作流 |
| [driverlib_runtime_rules](references/driverlib_runtime_rules.md) | DriverLib 安全调用法则 |
| [hardware_validation_notes](references/hardware_validation_notes.md) | 硬件调试经验（HFXT/Flash/复位/干扰） |
| [ccs_dss_debug](references/ccs_dss_debug.md) | CCS DSS 调试指南 |
| [sdk_schema_lookup](references/sdk_schema_lookup.md) | SDK Schema / 例程检索方法 |
| [pin_occupation_table](references/pin_occupation_table.md) | G3519 引脚占用表 |
| [docs/faq.md](docs/faq.md) | MSPM0 / SysConfig / 板卡 / 安装常见问题 |
| [docs/topic-guide.md](docs/topic-guide.md) | 按板卡、配置、示例和排障问题寻找原始资料 |
| [docs/verification-levels.md](docs/verification-levels.md) | 解释静态检查、构建、烧录、实板状态及证据边界 |
| [examples/](examples/) | led_blink / pwm_breath_led / uart_blocking_tx / oledui_full_g3519 等 |

`verify_example.py` 的 `build: detected` 仅表示发现构建目录或输出文件，`flash: ready` 仅表示发现固件文件；两者均不表示本次成功编译或已烧录。示例 manifest 的 `validation_level` 是作者声明，实测条件与证据请参照[验证状态说明](docs/verification-levels.md)。

常用脚本（完整参数见 `SKILL.md`「Examples and Tools」）：

```bash
python scripts/check_syscfg.py <project-dir> --board tianmengxing   # 引脚/生成物/工程结构审计
python scripts/verify_example.py <project-dir> --snapshot --json     # 静态/SysConfig/构建/烧录/硬件 各阶段状态
python scripts/list_examples.py                                      # 列出内置示例
```

---

## 🤝 贡献 / License / 致谢

- **贡献**：欢迎提交 Issue / PR / Discussions（见仓库顶部链接）；即使只是指出一个宏名或板载资源写错也很有用。
- **License**：[MIT](LICENSE)。使用、复制、修改或再发布本仓库及其来自上游的实质性内容时，请保留 `Copyright (c) 2026 mc3545dada`。
- **致谢**：上游作者 **mc3545dada**、**TI**（DriverLib / SysConfig）、**嘉立创**（天猛星/地猛星/地正星/天巧星开发板）。

<div align="center">
  <p>祝各位电赛战友：<b>一次过编，不冒白烟！顺利拿奖！🏆</b></p>
</div>
