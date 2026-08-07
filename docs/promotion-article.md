---
title: "我用 AI 写 MSPM0 电赛代码，结果它把板子烧了：一个业余嵌入式玩家的自救指南"
description: "从芯片手册逐行查引脚到 SysConfig 自动生成，从手动移植驱动到 AI 一键接管——我如何用一份开源 Skill 让 Codex 和 Claude Code 听话地写 MSPM0 代码。"
tags:
  - MSPM0
  - 电赛
  - NUEDC
  - AI编程
  - Codex
  - Claude Code
  - 嵌入式
  - SysConfig
  - DriverLib
  - 立创
  - 天猛星
  - 地猛星
---

# 我用 AI 写 MSPM0 电赛代码，结果它把板子烧了：一个业余嵌入式玩家的自救指南

如果你参加过全国大学生电子设计竞赛，你大概见过这样的场景：

比赛第三天凌晨两点，队友已经趴在桌上睡着了。你盯着屏幕上的编译输出，一遍一遍地烧录，一遍一遍地等板子亮灯。但灯就是不亮。

你打开示波器，把探头怼在 GPIO 引脚上，发现波形完全不对。你开始怀疑是不是驱动写错了，又翻回 `ti_msp_dl_config.c` 看了一遍——你记得自己好像没改过这个文件，但这个时候已经记不清了。

然后你想起，下午让 AI 帮你配了一路 PWM。

---

## 我踩过的坑，希望你别再踩

故事要从我让 AI 改一个 MSPM0 工程说起。

一开始一切都很顺利。我在 Codex 里输入需求："帮我在 MSPM0G3507 上加一路 1kHz 的 PWM，驱动一个蜂鸣器。" AI 很听话，很快就生成了代码。C 文件写得漂漂亮亮，注释也齐全，我在心里给它点了个赞。

然后我编译，烧录，上板。

蜂鸣器没响。板子上的 LED 反而开始乱闪。

我蹲在满了零件的桌子前，把示波器、逻辑分析仪、万用表全翻出来，开始排查。最后发现的问题让我哭笑不得：

1. PA5 和 PA6 是板载晶振引脚，AI 把它们配成了 PWM 输出
2. `ti_msp_dl_config.c` 被 AI 手动修改过，下次 SysConfig 重新生成就会全部丢失
3. PA19 被配置成了 GPIO，直接和 SWD 调试接口冲突

这三个问题，任何一个都足以让代码"看起来对了"但"跑不起来"。而 AI 同时踩了三个。

这不是 AI 不聪明，而是我忘了告诉它：**这块板子上，不是所有芯片引脚都能用。**

---

## 为什么通用 AI 写不好嵌入式

如果你用过 ChatGPT 或 Claude 写代码，你会发现它们写 Python、写 JavaScript 都很溜。但换成嵌入式，错误率直线上升。原因很简单：

**通用 AI 的训练数据里，没有你手上这块板子的原理图。**

芯片数据手册说 PA5 可以复用为 PWM 输出，AI 就给你配了。但它不知道，你手上的天猛星开发板已经把 PA5 焊上了一颗 40MHz 晶振。你把它配成 PWM，不但 PWM 不工作，整个系统时钟都会崩。

**这就是嵌入式开发的"常识鸿沟"。**

Python 的 `print()` 在任何机器上都能跑。但 MSPM0 的 PA5 引脚，在芯片手册、天猛星原理图、地猛星原理图、你自己的最小系统板上，可能意味着完全不同的东西。

通用 AI 解决不了这个问题。它需要一个"懂这块板子"的上下文。

---

## 我的解决方案：把板级知识写进 Skill

大概在踩了三次坑之后，我决定把这些教训固化下来，做成一份给 AI 看的"行为规范"。

项目叫 [mspm0-skill](https://github.com/Ibook000/mspm0-skill)，目前已经在 GitHub 开源。

它不是一个 SDK，也不是一个库。它是一份 `SKILL.md` 文件，外加一些辅助脚本和参考文档。当 Codex 或 Claude Code 加载它之后，AI 会遵守一套规则：

**第一，永远把 `.syscfg` 当作硬件配置的唯一信源。**

MSPM0 的引脚复用、时钟、中断、DMA 和外设初始化，全部由 SysConfig 生成到 `ti_msp_dl_config.c` 和 `ti_msp_dl_config.h`。这两个文件是生成物，不是给人手改的。AI 之前直接改生成文件，编译能过，但下次 SysConfig 一跑就全没了。

现在 Skill 里明确写了：**永远不要手改生成文件。**

**第二，改引脚前先查板级约束。**

天猛星和地猛星虽然是同一颗 MSPM0G3507，但引脚占用完全不同：

- 天猛星：PB22 是板载 LED，PA5/PA6 是 40MHz 晶振，PB26 是 WS2812 RGB，PA27/PA28 是 LSM6DS3 IMU
- 地猛星：PA14 是板载 LED，PB6-PB9 是 W25Q32 Flash，PA18 是 BSL 按键，PA2 是 ROSC

这些信息，AI 的预训练数据里大概率没有。但 Skill 文件里有。

**第三，每次修改后运行静态检查。**

仓库里带了一个 `check_syscfg.py` 脚本，会扫描 `.syscfg` 文件，检查引脚冲突、时钟配置、生成文件风险、构建产物完整性。AI 在给代码之前先跑一遍，能在构建前发现问题。

---

## 实际用起来是什么感觉？

我让 Claude Code 加载这个 Skill 后，再让它配置 PWM，它的行为完全变了：

1. 先找到当前工程的 `.syscfg` 文件
2. 读取 `ti_msp_dl_config.h`，确认外设宏名
3. 对照引脚避坑表，排除 PA5/PA6/PA19/PA20 等特殊引脚
4. 只修改 `.syscfg` 和应用代码，不碰生成文件
5. 运行静态检查脚本，确认没有冲突
6. 最后告诉我：检查通过了，但烧录和硬件验证需要我自己做

整个过程就像请了一个懂行的嵌入式工程师当助手，而不是一个只会抄代码的 ChatGPT。

---

## 这个 Skill 里到底有什么？

目前仓库里包含了：

**参考文档 7 份**
- 天猛星/地猛星引脚映射与避坑指南
- 硬件调试经验手册（含实测坑位）
- SysConfig & CCS 工作流
- DriverLib 安全调用法则
- 引脚占用与复用表
- 自动化调试指南
- SDK Schema 查找指南

**辅助脚本 6 个**
- SysConfig 静态检查器
- 示例工程索引器
- SDK 示例检索器
- 示例提取打包器
- CCS DSS 命令行调试器
- 串口调试终端

**可复用示例 5 个**
- 空工程骨架
- GPIO 点灯（天猛星 PB22）
- PWM 呼吸灯（80MHz）
- 串口收发
- OLED UI + 陀螺仪 + 小游戏综合工程

特别是 `oledui_full_g3519` 这个示例，是一个完整的 MSPM0G3519 工程：80MHz 时钟、OLED UI 菜单、LSM6DS3 姿态解算、WS2812 RGB、蜂鸣器、正交编码器、无线通信、5ms 定时器任务。电赛里可能用到的外设，基本都覆盖了。

---

## 它能帮你什么？

**如果你是电赛选手：**

四天三夜，最宝贵的是时间。与其让 AI 帮你生成一段"看起来对"但"跑不起来"的代码，再花两个小时排查，不如让它从一开始就遵守正确的规则。

让 AI 帮你配引脚、写驱动、提炼代码，你腾出时间去调试算法、优化控制逻辑、准备答辩。

**如果你是嵌入式初学者：**

你可能还不熟悉 SysConfig 的工作方式，也不知道为什么要分 `ti_msp_dl_config.c` 和 `main.c`。这个 Skill 里的规则会强迫 AI 按正确的方式工作，你跟着看一遍，也就学会了正确的工程习惯。

**如果你只是好奇 AI 写嵌入式代码靠不靠谱：**

你可以直接 clone 下来玩一下。让 Codex 加载 Skill，然后给它提一个需求，看看它和"裸奔"的 AI 有什么区别。

---

## 怎么用？

```bash
git clone https://github.com/Ibook000/mspm0-skill.git
```

然后在 Codex 或 Claude Code 里说：

```text
请先读取 ./mspm0-skill/SKILL.md，再修改我的 MSPM0 项目。
```

就这么简单。

---

## 最后说点掏心窝的话

我一开始做这个项目，纯粹是被 AI 坑怕了。后来做了才发现，价值不在"防止 AI 犯错"，而在"定义一套正确的工程规范"。

嵌入式开发看着门槛高，其实很多坑都是重复的。晶振引脚不能复用、SWD 不能乱配、生成文件不能手改、上电时序要考虑、外设初始化要按顺序……这些经验，每个踩过坑的人都懂，但没人好好整理过。

我把它们整理成了 Skill，分享给所有参加电赛的同学们。

如果你有跑通过的 `.syscfg` 片段、工程示例、引脚修正或者其他 MSPM0 经验，欢迎提 Issue 或 PR。就算只是指出一个宏名写错了，也是在帮全国的嵌入式玩家少踩一个坑。

项目地址：[https://github.com/Ibook000/mspm0-skill](https://github.com/Ibook000/mspm0-skill)

如果它刚好帮你少查了一次原理图，欢迎点个 Star。然后回去把板子接上，跑一下。

祝各位同学在国赛/省赛中：一次过编，不冒白烟，顺利拿奖。

---

## 英文版 / English Version

# I Told AI to Write MSPM0 Code for a Competition, and It Killed My Board

If you've ever competed in NUEDC (National Undergraduate Electronic Design Contest), you know the scene:

It's 3 AM on day three. Your teammate is asleep at the desk. You're staring at the compiler output, flashing the board again and again, waiting for the LED to light up. It doesn't.

You grab the oscilloscope, probe the GPIO pin, and the waveform is wrong. You open `ti_msp_dl_config.c` and check if you accidentally edited it. You're not sure anymore.

Then you remember: the AI you asked to configure the PWM this afternoon.

## The Problem With Generic AI for Embedded

GPT-style models write great Python. They struggle with embedded firmware because **their training data doesn't include the schematic of your specific development board**.

The datasheet says PA5 can be a PWM output. The AI configures it. But your LCKFB Tianmengxing board has a 40 MHz crystal on PA5. The board stops working.

This gap is not the AI's fault. It's a knowledge gap that can be fixed with the right context.

## The Fix: A Skill That Encodes Board Knowledge

[mspm0-skill](https://github.com/Ibook000/mspm0-skill) is an open-source Skill for Codex and Claude Code. When loaded, it forces AI agents to follow these rules:

1. Treat `.syscfg` as the source of truth for hardware configuration
2. Never hand-edit generated files like `ti_msp_dl_config.c` or `ti_msp_dl_config.h`
3. Check board-specific pin constraints before assigning pins
4. Run static validation scripts before reporting success
5. Report validation level honestly: source vs build vs flash vs hardware

The repo includes:
- 7 reference documents (pin maps, debug notes, SysConfig workflow, DriverLib rules)
- 6 helper scripts (SysConfig checker, serial console, CCS DSS debugger)
- 5 reusable examples (LED, PWM, UART, OLED UI + IMU + games)

## Try It

```bash
git clone https://github.com/Ibook000/mspm0-skill.git
```

Then tell your agent:

```text
Read ./mspm0-skill/SKILL.md before modifying this MSPM0 project.
```

If it saves you one late-night debugging session, leave a Star.
