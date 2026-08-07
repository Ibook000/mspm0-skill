---
title: "我给 Codex 和 Claude Code 补了一份 MSPM0 使用说明"
description: "面向 TI MSPM0、MSPG3507 和立创天猛星开发板的 AI Agent Skill，包含 SysConfig 工作流、DriverLib 规则、引脚检查和可复用示例。"
tags:
  - MSPM0
  - MSPG3507
  - MSPM0G3507
  - MSPM0G3519
  - Tianmengxing
  - Codex
  - Claude Code
  - SysConfig
  - DriverLib
  - Embedded
---

# 我给 Codex 和 Claude Code 补了一份 MSPM0 使用说明

> English version follows below.

前段时间我让 AI 帮忙改一个 MSPM0 工程。C 代码写得挺快，麻烦也来得挺快。

它会猜 DriverLib 宏名，会顺手改掉 `ti_msp_dl_config.c`，还可能挑中一个在芯片手册上能用、在开发板上早已接给晶振或下载接口的引脚。代码看着像那么回事，编译、烧录和上板运行却是三回事。

做过 MSPM0 的朋友应该很熟悉这种场面。一个简单的 PWM 或 UART，最后半天时间花在查 SysConfig、翻生成头文件、对开发板原理图。

所以我整理了 `mspm0-skill`。

它是一份给 Codex、Claude Code 等编程 Agent 使用的工程规则，也带了检查脚本、参考资料和示例项目。它不会替你造一套新的 MSPM0 开发方式，只负责提醒 Agent 按现有工程办事，先看配置，少改文件，做完再验证。

项目地址 [Ibook000/mspm0-skill](https://github.com/Ibook000/mspm0-skill)

## 实际跑起来是什么样

下面这张图是 Claude Code 加载 Skill 后的项目状态。规则、SysConfig 资料、示例和辅助脚本都能直接读取。

![Claude Code 已加载 mspm0-skill，包含 Skill、参考资料、SysConfig 示例、MSPM0 工程示例与辅助脚本。](https://raw.githubusercontent.com/Ibook000/mspm0-skill/main/assets/readme/msp0-installation.png)

再让 Agent 审查一个 MSPM0 工程，它会先检查配置和引脚风险，再看应用代码。这样做慢不了多少，却能少掉很多莫名其妙的硬件排查。

![MSPM0 项目审查报告，展示 SysConfig 配置、代码风险和接线或引脚问题。](https://raw.githubusercontent.com/Ibook000/mspm0-skill/main/assets/readme/msp0-audit-report.png)

## 我最想解决的几个坑

MSPM0 项目里，`.syscfg` 管着引脚复用、时钟、中断、DMA 和外设初始化。`ti_msp_dl_config.c` 与 `ti_msp_dl_config.h` 通常由 SysConfig 生成。直接改生成文件，眼前可能能编译，下次重新生成就全没了，配置和源码还会越走越远。

另一个坑是引脚。

芯片支持某个复用功能，不等于开发板上的这个引脚闲着。拿立创天猛星一类板卡来说，PA5 和 PA6 可能接了高频晶振，PA19 和 PA20 要留给 SWD，PA10 和 PA11 常用于板载 CH340 串口。OLED、IMU、SPI Flash、无线模块、按键、蜂鸣器和 WS2812 也都要占资源。

通用模型很难凭训练数据记准每块板子的细节。`mspm0-skill` 要求 Agent 从当前工程、本地生成文件和板级资料里找答案，找不到就说明缺什么，不靠猜。

这一点我挺在意。嵌入式开发里，猜错一个 API 名称只是编译报错，猜错一个板级约束可能让你对着示波器折腾一晚上。

## Skill 会怎样处理一个任务

假设你让 Codex 给 MSPM0G3507 工程加一路 1 kHz PWM。加载 Skill 后，它应该先找到 `.syscfg`、应用源码和 `ti_msp_dl_config.h`，确认当前设备、定时器实例、时钟和可用引脚。

接着运行静态检查，看看目标引脚有没有撞上晶振、SWD、BSL 或板载外设。确定方案后，它只改必要的 `.syscfg` 和应用代码，沿用项目原来的 CCS、Keil 或 CMake 构建方式。

最后，它还得把验证结果讲明白。

代码检查通过、工程编译成功、固件已经烧录、真实硬件运行正常，这四件事不能混着说。没有接触到板子，就不能写已经完成硬件验证。看起来有点较真，实际能避免不少误判。

## 仓库里放了什么

我把日常会用到的东西尽量放到了一起。

`scripts/check_syscfg.py` 用来扫描 `.syscfg`、生成文件、工程入口和常见引脚风险。`scripts/index_syscfg_examples.py` 可以从本地 TI SDK 里找 SysConfig 模块与示例。串口调试时可以用 `scripts/serial_console.py` 看日志。

仓库里还有 GPIO、PWM 呼吸灯、UART、OLED UI、LSM6DS3 IMU 和 QEI 编码器示例。示例的作用是给 Agent 提供已经落地的写法，它仍然需要结合你的芯片、SDK 版本和板子修改，不能整段硬抄。

目前重点照顾的关键词和环境包括 `MSPG3507`、`MSPM0G3507`、`MSPM0G3519`、立创天猛星、CCS、Keil、CMake、Arm GNU Toolchain 和 OpenOCD。

## 怎么用

先把仓库拉到本地或当前项目旁边。

```bash
git clone https://github.com/Ibook000/mspm0-skill.git
```

然后让 Codex 或 Claude Code 在动手前读取 `SKILL.md`。

```text
请先读取 ./mspm0-skill/SKILL.md，再修改我的 MSPM0 项目。
```

需求最好写得具体一点。下面这段可以直接改着用。

```text
读取 mspm0-skill/SKILL.md。当前项目使用 MSPM0G3507 和 SysConfig。
请增加一路 1 kHz PWM 输出，不要修改 ti_msp_dl_config.c 或 ti_msp_dl_config.h。
先检查时钟、引脚冲突和现有外设占用，只修改必要的 .syscfg 与应用代码。
完成后说明检查、编译、烧录和硬件运行分别验证到了哪一步。
```

这样交代以后，Agent 会少写一点看起来很完整、实际没法落地的代码，多花一点时间尊重当前工程。

## 它也有明确的边界

这个 Skill 能读配置、查风险、改工程、跑构建，也能整理烧录和调试步骤。它碰不到你的接线，测不到供电和逻辑电平，也看不到模块当前处于什么模式。

所以硬件结论最终还得上板确认。

仓库还在继续整理。如果你手里有跑通过的 `.syscfg` 片段、MSPG3507 或 MSPM0G3519 示例、天猛星引脚修正、CCS 与 Keil 的实战经验，欢迎提 Issue 或 PR。哪怕只是指出一个宏名或板载资源写错了，也很有用。

项目地址 [https://github.com/Ibook000/mspm0-skill](https://github.com/Ibook000/mspm0-skill)

如果它刚好帮你少查了一次原理图，欢迎点个 Star。然后回去把板子接上，跑一下。

---

# I Wrote an MSPM0 Field Guide for Codex and Claude Code

I recently asked an AI coding agent to modify an MSPM0 project. It produced C code quickly and created a different kind of work just as quickly.

It guessed DriverLib macros, edited `ti_msp_dl_config.c`, and selected pins that were valid on the MCU but already connected to board hardware. The code looked plausible. Building, flashing, and running it on a board were separate matters.

That experience led me to build `mspm0-skill`, a set of project rules, references, checks, and examples for Codex, Claude Code, and other coding agents working with TI MSPM0 firmware.

Repository [Ibook000/mspm0-skill](https://github.com/Ibook000/mspm0-skill)

## What it changes

The Skill asks an agent to inspect the current project before writing code. It treats `.syscfg` as the configuration source for pinmux, clocks, interrupts, DMA, and peripheral setup. It also protects generated files such as `ti_msp_dl_config.c` and `ti_msp_dl_config.h` from casual manual edits.

Board constraints matter too. A pin that is available in the device datasheet may already serve the crystal, SWD, BSL, CH340 UART, OLED, IMU, flash, radio, buttons, buzzer, or RGB LED on the actual board. The agent is expected to check local configuration and board references instead of guessing.

For a typical change, it finds the `.syscfg`, application sources, generated header, and build entry point. It checks pins and clocks, makes the smallest useful edit, then uses the project's existing CCS, Keil, or CMake workflow.

It must also report the validation level honestly. A source review, a successful build, a completed flash, and a test on real hardware are four different results.

## What is included

The repository includes a SysConfig checker, an SDK example indexer, serial console tooling, board pin references, and reusable examples for GPIO, PWM, UART, OLED UI, LSM6DS3 IMU, and QEI encoders.

The current focus covers `MSPG3507`, `MSPM0G3507`, `MSPM0G3519`, Lichuang Tianmengxing boards, CCS, Keil, CMake, Arm GNU Toolchain, and OpenOCD.

## Try it

```bash
git clone https://github.com/Ibook000/mspm0-skill.git
```

Ask your agent to read the Skill before changing the firmware.

```text
Read ./mspm0-skill/SKILL.md before modifying this MSPM0 project.
```

A more useful request looks like this.

```text
Read mspm0-skill/SKILL.md. This project uses MSPM0G3507 and SysConfig.
Add a 1 kHz PWM output without editing generated files.
Check clock, pin conflicts, and existing peripheral use first.
Change only the required .syscfg and application code, then report each validation step.
```

The Skill cannot verify wiring, power, logic levels, or a peripheral's behavior without access to the hardware. That boundary is intentional. It helps the agent say what it actually proved.

Verified SysConfig snippets, working examples, pin-map corrections, and toolchain notes are welcome. If the project saves you one long board-debugging session, consider leaving a Star.

Repository [https://github.com/Ibook000/mspm0-skill](https://github.com/Ibook000/mspm0-skill)

## Suggested publishing tags

`MSPM0`, `MSPG3507`, `MSPM0G3507`, `MSPM0G3519`, `Tianmengxing`, `立创天猛星`, `Codex`, `Claude Code`, `SysConfig`, `DriverLib`, `TI`, `Embedded`, `NUEDC`, `AI Agent`
