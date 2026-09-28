# MSPM0 开发问题导航

本页按开发者的实际问题指向仓库内的原始资料。开始改工程前，先阅读 [Agent 工作规则](../SKILL.md)，确认芯片、封装、板卡和当前 `.syscfg`；不要把某块板的引脚表直接用于另一块板。

## 我用的是哪块板，哪些引脚不能直接复用？

| 板卡 | 芯片 | 板卡数据 | 阅读版规则 |
|---|---|---|---|
| 立创·天猛星 | MSPM0G3507 | [tianmengxing.json](../boards/tianmengxing.json) | [天猛星引脚与资源](../references/boards/tianmengxing.md) |
| 立创·地猛星 | MSPM0G3507 | [dimengxing.json](../boards/dimengxing.json) | [地猛星引脚与资源](../references/boards/dimengxing.md) |
| 立创·地正星 | MSPM0L1306 | [dizhengxing.json](../boards/dizhengxing.json) | [地正星引脚与资源](../references/boards/dizhengxing.md) |
| 自定义示例板 | MSPM0G3519 | [custom-mspm0g3519.json](../boards/custom-mspm0g3519.json) | [G3519 示例规则](../references/boards/custom-mspm0g3519.md) |

自定义 G3519 的连接仅适用于对应示例板。真实项目请以板卡版本、原理图及当前 `.syscfg` 为准；板卡无法确认时，不应套用专属规则。

## SysConfig 怎么检查？能直接修改生成文件吗？

- [检查脚本](../scripts/check_syscfg.py)：在仓库根目录运行 `python3 scripts/check_syscfg.py <project-dir> --board tianmengxing --json`；`--board` 须改为实际板卡 ID。对于仓库内的源码快照，增加 `--snapshot`。
- [SysConfig 与 CCS/Keil/CMake 工作流](../references/sysconfig_ccs_workflow.md)：按当前工程的工具链生成、构建；不要手改 `ti_msp_dl_config.c/.h`。
- [DriverLib 规则](../references/driverlib_runtime_rules.md) 与 [SDK Schema 查找](../references/sdk_schema_lookup.md)：确认当前生成头文件的宏和初始化函数，不猜 API。
- [常见问题](faq.md)：没有生成头文件、引脚占用和 snapshot 模式的说明。

## 有哪些可参考的功能示例？

| 任务 | 源码快照 | 先核对什么 |
|---|---|---|
| 天猛星 PB22 LED 闪烁 | [led_blink](../examples/led_blink/) | LED 极性、时钟、生成宏 |
| 天猛星 PWM 呼吸灯 | [pwm_breath_led](../examples/pwm_breath_led/) | 定时器实例、频率、引脚占用 |
| UART 阻塞发送 | [uart_blocking_tx](../examples/uart_blocking_tx/) | 板卡串口连接、波特率、TX/RX |
| 自定义 G3519 OLED UI | [oledui_full_g3519](../examples/oledui_full_g3519/) | 实际原理图、SDK 版本、各外设连接 |

示例通常只提供 `.syscfg`、源码和 manifest，不等于完整可导入的 CCS 工程。使用前阅读示例 README 与 manifest，按自己的工程重新生成并测试。

## 编译、烧录或串口没反应，如何定位？

1. 先看 [硬件验证笔记](../references/hardware_validation_notes.md) 与 [FAQ](faq.md)，确认电源、接线、探针和工程配置。
2. 用 `python3 scripts/serial_console.py --list` 查可用串口；按当前板卡核对 CH340 所连引脚。
3. 使用 [CCS DSS 指南](../references/ccs_dss_debug.md) 时，先确认探针与硬件相符。
4. 用 [验证状态说明](verification-levels.md) 分别记录静态检查、构建、烧录和实板结果。静态检查通过不能证明硬件可用。

## 资料来源与范围

本仓库基于 [mc3545dada/mspm0-skill](https://github.com/mc3545dada/mspm0-skill) 的历史版本修改，保留原作者 MIT 版权声明。这里的板卡规则应与 [机器可读数据](../boards/)及实际板卡资料交叉核对。