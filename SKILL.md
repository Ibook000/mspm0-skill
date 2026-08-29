---
name: mspm0
description: Tool-neutral CLI agent rules for TI MSPM0 development with SysConfig, DriverLib, CCS, Keil/uVision, CMake/GCC/OpenOCD, and supported board references. Use when an agent needs to inspect or modify MSPM0 projects, validate SysConfig output, package examples, or work on NUEDC embedded firmware.
---

# MSPM0 Agent Skill

本文件是 MSPM0 Agent 的**通用入口**。它负责定义工程工作流、修改边界、验证口径和工具链处理方式；具体板卡的引脚占用、默认电平和板载资源放在独立参考文件中，只有识别到对应板卡后才读取。

## Board Selection

在修改工程前，先根据 `.syscfg` 的 `--device`、`--package`、项目 README、原理图或用户明确说明识别板卡。识别完成后，仅加载需要的板卡资料：

| 板卡 | 参考文件 | 适用场景 |
|---|---|---|
| 立创·天猛星 MSPM0G3507 | [`references/boards/tianmengxing.md`](references/boards/tianmengxing.md) | LQFP-64、板载 OLED/IMU/WS2812/无线模块 |
| 立创·地猛星 MSPM0G3507 | [`references/boards/dimengxing.md`](references/boards/dimengxing.md) | 48-pin、PA14 LED、W25Q32、H3/H5 扩展座 |
| 自定义 MSPM0G3519 | [`references/boards/custom-mspm0g3519.md`](references/boards/custom-mspm0g3519.md) | LQFP-64、自定义板级资源 |

如果无法确认板卡，不要把任一板卡的占用信息当作通用规则；应先询问用户，或仅依据当前工程和芯片资料进行低风险检查。

## Default Workflow

1. 定位 `.syscfg` 或 `system.syscfg`、可编辑源码、生成的 `ti_msp_dl_config.h` 和工程入口。CCS 使用 `targetConfigs/*.ccxml`，Keil 使用 `*.uvprojx` 和 scatter 文件，CMake/GCC/OpenOCD 使用 `CMakeLists.txt`、toolchain 文件和 OpenOCD 配置。
2. 若本 Skill 可用，运行 `python scripts/check_syscfg.py <project-dir>`；需要机器可读结果时使用 `--json`，需要将 warning 作为失败时使用 `--strict`。
3. 读取 `.syscfg` 元数据，包括设备、封装、SDK、SysConfig 版本、模块、实例、引脚、时钟和中断；随后按 Board Selection 加载对应板卡资料。
4. 检查生成的 `ti_msp_dl_config.h`，确认宏名、IRQ 名、实例名和确切的 SysConfig 初始化函数拼写。
5. 对陌生 SysConfig 字段，优先查当前 `.syscfg`、`examples/*/manifest.json`、TI SDK 示例或 `source/ti/driverlib/.meta/*.syscfg.js`，不要凭记忆猜测。
6. 只修改最小必要的 `.syscfg` 和应用代码表面；重新生成 SysConfig 输出或通过项目现有工具链构建。
7. 若执行烧录或调试，确认探针后端与实际硬件一致，并单独报告源码、SysConfig、编译、烧录和真实硬件验证结果。

## Core Rules

- 将 `.syscfg` 作为引脚复用、外设、时钟、中断、DMA 和生成初始化的唯一配置来源。
- GPIO、UART、PWM、Timer、ADC、I2C、SPI、DMA 和时钟配置优先使用 SysConfig + DriverLib。
- 禁止手工修改 `ti_msp_dl_config.c/.h`、`device_linker.cmd`、`Objects/`、`Listings/`、目标文件、map 文件和 `.out` 等生成物或构建产物。
- 保留 `.syscfg` 元数据，包括 `@cliArgs`、`@v2CliArgs`、`@versions`、`--device`、`--package` 和 `--product`。
- 不要猜生成名称；必须从当前 `ti_msp_dl_config.h` 和应用源码确认宏、IRQ 和初始化函数。
- 不要凭空创造 SysConfig 字段、枚举值、设备元数据、板卡名称、封装名称或工具版本。
- 保留无关代码、注释、版权头、工程布局和已有 `.syscfg` 设置；需要大范围重构时先解释原因。
- 未经用户确认，不要更改设备、封装、SDK、编译器、CCS 版本、板卡或调试探针。
- SysConfig warning 必须和 build/flash 成功分开报告；有 warning 时不能称为 clean。
- 没有真实连接的板卡时，不能声称已经完成硬件验证。

## Project Shape and Toolchain Rules

- 简单工程通常把逻辑放在 `main.c`、`empty.c` 或少量文件中，可以做窄范围修改。
- 多模块工程常见 `app/`、`bsp/`、`components/`、`core/`、`drivers/`、`hal/`、`middleware/`、`platform/` 或 `tasks/`；先确认模块边界再添加外设。
- 若存在 `FreeRTOSConfig.h`、`FreeRTOS.h`、`task.h`、`xTaskCreate` 或 `vTaskStartScheduler`，按 FreeRTOS 工程处理，尊重现有任务、队列、中断和阻塞调用边界。
- Keil 工程以 `system.syscfg`、`*.uvprojx` 和 scatter 文件为入口；CMake/GCC/OpenOCD 工程以 `CMakeLists.txt`、toolchain 和 `.cfg` 为入口，不要强行套用 CCS 流程。
- OpenOCD 报告 `unable to find a matching CMSIS-DAP device` 时，应报告为探针发现失败，而不是固件构建失败。

## Ambiguous Requests and External Hardware

用户缺少重要参数时，不要静默选择有风险的值。重要参数包括引脚、外设实例、UART 格式、Timer 周期、PWM 频率/占空比/极性、ADC 参考与采样时间、DMA 方向、IRQ 优先级及外部模块供电和逻辑电平。低风险默认值应来自当前工程、`examples/` 或 TI SDK，并明确告知用户。

使用外部模块时，若资料不足，应请求数据手册、原理图、引脚图、供电电压、逻辑电平、通信协议和关键参数。排障时依次核对电源、地、上拉、电平转换、复位/使能、启动脚、片选、TX/RX 交叉、I2C 地址、SPI 模式、PWM 极性和共享引脚，并把“固件看起来正确”和“硬件已证明正确”严格分开。

## References

按任务需要读取以下资料：

- [`references/boards/`](references/boards/)：板卡专属引脚、板载资源、默认电平和风险。
- [`references/MSPM0G3507_Pinout_Mapping.md`](references/MSPM0G3507_Pinout_Mapping.md)：天猛星/地猛星引脚映射总览。
- [`references/sysconfig_ccs_workflow.md`](references/sysconfig_ccs_workflow.md)：SysConfig、CCS、Keil、CMake 和烧录工作流。
- [`references/driverlib_runtime_rules.md`](references/driverlib_runtime_rules.md)：DriverLib、时钟、中断和运行时规则。
- [`references/sdk_schema_lookup.md`](references/sdk_schema_lookup.md)：SysConfig Schema 和 SDK 示例查找方法。
- [`references/hardware_validation_notes.md`](references/hardware_validation_notes.md)：板级验证和硬件排障经验。
- [`references/ccs_dss_debug.md`](references/ccs_dss_debug.md)：CCS DSS 调试工作流和限制。

## Examples and Tools

每个可复用示例应包含 `example.syscfg`、`README.md`、`manifest.json` 和最小相关源码。`manifest.json` 使用统一字段：`sdk` 表示 SDK 产品，`sysconfig` 表示 SysConfig 工具版本，`syscfg` 表示示例配置文件路径；不要再使用旧字段 `product` 或 `sysconfig_versions`。使用 `scripts/capture_example.py` 从真实工程提取示例，避免把完整 CCS 工程直接塞进示例目录。

- `python scripts/check_syscfg.py <project-dir>`：检查 SysConfig、生成物、引脚、初始化函数、工程结构、构建输出和验证提示。
- `python scripts/check_syscfg.py <project-dir> --json`：输出机器可读 JSON。
- `python scripts/check_syscfg.py <project-dir> --strict`：将 warning 视为检查失败。
- `python -m unittest discover -s tests -v`：运行检查器回归测试。
- `python scripts/list_examples.py`：列出 `examples/*/manifest.json` 中的示例。
- `python scripts/capture_example.py <project-dir> --name <example-name> --include <glob>`：提取示例。
- `python scripts/index_syscfg_examples.py <mspm0-sdk-root> --board LP_MSPM0G3507 --module UART`：检索本地 TI SDK 示例。
- `python scripts/serial_console.py --list`：列出串口。
- `python scripts/ccs_dss_debug.py <project-dir> probe --leave-running`：通过 CCS DSS 检查探针和目标。

检查器退出码约定为：`0` 表示没有失败，`1` 表示发现 error（或 `--strict` 下发现 warning），`2` 表示命令行参数错误。

## Flash and Debug Backends

> **重要**：不要使用未经当前芯片和板卡资料确认的烧录后端。仓库已有工作流中常见的后端包括 J-Link、DSLite、CH340 + BSL，以及适用于 CMake/GCC 项目的 TI 扩展版 OpenOCD。

CCS/UniFlash 项目的示例路径：

```text
dslite -c <target.ccxml> -e -r 2 -u <project.out>
```

CCS DSS 示例路径：

```text
python scripts/ccs_dss_debug.py <project-dir> probe --leave-running
python scripts/ccs_dss_debug.py <project-dir> run-to-symbol --symbol main --load --reset "System Reset"
```

`ccs-dss` 不是 OpenOCD 调试后端；CMake/GCC/OpenOCD 项目应使用项目已有的 flash/debug target。调试动作可能暂停实时控制 CPU，在真实硬件上执行断点或寄存器检查前必须先说明影响。
