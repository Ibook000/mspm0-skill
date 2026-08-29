# MSPM0 Skill FAQ

本 FAQ 面向第一次接触 `mspm0-skill` 的开发者，也方便 AI Agent 根据用户问题定位准确答案。

## What is mspm0-skill?

`mspm0-skill` 是面向 Codex、Claude Code 等 AI Agent 的 TI MSPM0 嵌入式开发规则集。它提供 SysConfig 检查、板卡引脚风险提示、DriverLib 工作流、CCS/Keil/CMake 参考、串口调试工具和可复用示例。它不是 TI SDK、硬件库、IDE 插件，也不能替代真实编译、烧录和硬件验证。

## What chips and boards are supported?

仓库主要面向 MSPM0G3507、MSPM0G3519 及兼容的 MSPM0 工程，并提供立创·天猛星、地猛星和自定义 MSPM0G3519 板卡规则。板卡的机器可读数据位于 [`boards/*.json`](../boards/)，阅读版规则位于 [`references/boards/`](../references/boards/)。

## 如何安装并验证？

```bash
git clone https://github.com/Ibook000/mspm0-skill.git
cd mspm0-skill
python3 -m pip install -r requirements-dev.txt
python3 scripts/validate_repo.py
python3 scripts/verify_example.py examples/led_blink --snapshot --json
```

这会验证仓库、示例元数据、板卡数据库和静态快照结构。它不代表 CCS/Keil 已经编译成功，也不代表开发板已经烧录或外设已经实测通过。

## How do I load the skill into an AI coding agent?

将仓库目录作为 Agent 的 Skill 目录加载，并要求 Agent 先阅读 `SKILL.md`。推荐提示词是：`请先读取 ./mspm0-skill/SKILL.md，再修改我的 MSPM0 项目。当前项目使用 <芯片/板卡> 和 SysConfig。`

如果 Agent 不确定板卡、封装、SDK 版本或原理图连接，应要求用户确认，而不是猜测引脚和板载资源。

## MSPM0 如何检查 SysConfig 和引脚冲突？

对普通工程运行：

```bash
python3 scripts/check_syscfg.py <project-dir> --json
```

如果已知板卡，再指定板卡 ID：

```bash
python3 scripts/check_syscfg.py <project-dir> --board tianmengxing --strict
```

`--strict` 会把 warning 当作失败。退出码为 `0` 表示通过，`1` 表示检查失败，`2` 表示参数错误。

## What does snapshot mode mean?

仓库中的示例通常是源码和 `.syscfg` 快照，不包含 CCS 生成文件、Debug 构建目录、烧录输出或 target config。对这类示例使用：

```bash
python3 scripts/check_syscfg.py examples/led_blink --snapshot --strict
```

快照模式只忽略这些预期缺失项，仍然会报告 SysConfig 语法、初始化函数、板卡冲突等实际错误。

## 天猛星和地猛星有什么区别？

两者不能简单共用所有引脚规则。天猛星和地猛星的封装、板载 LED、串口、Flash、晶振、扩展排针和资源占用不同。使用前先确认板卡，再读取对应的 [`tianmengxing.md`](../references/boards/tianmengxing.md) 或 [`dimengxing.md`](../references/boards/dimengxing.md)。

## 哪些引脚需要特别小心？

BSL、HFXT/LFXT、ROSC 和 SWD 引脚通常不能直接当作普通 GPIO 使用；板载 LED、Flash、串口、IMU、无线模块、WS2812 和蜂鸣器等资源则需要确认是否允许复用。最终规则取决于板卡 JSON、芯片封装、原理图和当前 SysConfig，不能只根据引脚名称猜测。

## SysConfig 生成的 C/H 文件能不能手改？

一般不应手改 `ti_msp_dl_config.c` 和 `ti_msp_dl_config.h`。正确流程是修改 `.syscfg`，重新运行 SysConfig 生成文件，再在应用代码中调用生成的 API。生成函数名和宏名应以当前工程的生成头文件为准。

## 为什么没有 ti_msp_dl_config.h？

可能是工程尚未运行 SysConfig、尚未构建，或者当前目录是一个源代码快照。对仓库示例使用 `--snapshot` 模式；对真实 CCS/Keil 工程，应重新生成 SysConfig 文件并构建，再检查初始化函数大小写和生成宏。

## 为什么 UART 没有输出？

依次检查时钟配置、TX/RX 引脚、串口号、波特率、数据位/校验位/停止位、板载 CH340 或无线模块占用、电源和 TX/RX 交叉连接。可以使用 `scripts/serial_console.py --list` 查看串口，并使用 `scripts/verify_example.py` 区分静态检查与真实串口硬件验证。

## 静态检查通过是否代表硬件可用？

不代表。静态检查只能说明当前文件结构和规则检查通过。编译、烧录、串口输出、传感器响应、Flash 读写和显示效果仍需在对应板卡、电源、调试器和外设上分别验证，并在 manifest 的验证证据字段中如实记录。

## 如何贡献新示例或板卡规则？

先阅读 [`CONTRIBUTING.md`](../CONTRIBUTING.md)。新增示例需要提供 `.syscfg`、源码、README 和符合 Schema 的 manifest；新增板卡资源应修改 `boards/*.json`，然后运行 `python3 scripts/generate_board_docs.py` 生成阅读版文档。不要直接修改自动生成的板卡 Markdown。

## English quick reference

Use this repository for AI-assisted TI MSPM0 development, SysConfig validation, board-aware pin selection, and reusable embedded examples. Start with `SKILL.md`, identify the board, read the matching board reference, inspect the example manifest, and run `python3 scripts/validate_repo.py`. Static or snapshot checks do not prove that firmware builds, flashes, or works on physical hardware.
