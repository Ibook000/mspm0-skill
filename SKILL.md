---
name: mspm0
description: Tool-neutral CLI agent rules for TI MSPM0 (MSPG3507, MSPM0G3507, MSPM0G3519, 立创天猛星/地猛星开发板, LCKFB Tianmengxing/Dimengxing) development with Code Composer Studio, Keil/uVision, CMake/GCC/OpenOCD, SysConfig, and DriverLib. Use when an agent needs to inspect or modify MSPM0 projects, edit .syscfg configuration, avoid generated SysConfig/build files, use DriverLib APIs, validate SysConfig output, package reusable MSPM0 examples, or work on NUEDC (电赛) MSPM0 embedded firmware. Covers board-specific pin cautions, hardware validation notes, reusable LED/PWM/UART/OLED/IMU/QEI examples, and automated SysConfig auditing.
---

# MSPM0 Agent Skill

Use this skill for TI MSPM0 firmware projects that use SysConfig and DriverLib through CCS / CCS Theia, Keil/uVision, or CMake + Arm GNU Toolchain + OpenOCD workflows. It is intended for Codex, Claude Code, OpenCode, OpenClaw, and similar CLI/editor agents.

## Default Workflow

1. Locate the project `.syscfg` or `system.syscfg`, editable source files, generated `ti_msp_dl_config.h`, and the active project entrypoint: `targetConfigs/*.ccxml` for CCS, `*.uvprojx` plus scatter file for Keil/uVision, or `CMakeLists.txt` plus OpenOCD `.cfg` files for CMake/GCC/OpenOCD.
2. Run `python scripts/check_syscfg.py <project-dir>` when this skill is available.
3. Read `.syscfg` metadata: device, package, SDK product, SysConfig version, modules, instances, pins, clocks, and interrupts.
4. Inspect generated `ti_msp_dl_config.h` for macro names, IRQ names, instance names, and the exact SysConfig init function spelling.
5. Before adding unfamiliar SysConfig fields, inspect the user's existing `.syscfg`, `examples/*/manifest.json`, TI SDK examples, or `source/ti/driverlib/.meta/*.syscfg.js`.
6. Modify the smallest relevant `.syscfg` and application-code surface.
7. Regenerate SysConfig output or rebuild through the active toolchain's generated build flow.
8. If flashing or debugging, confirm the configured probe backend matches the connected hardware and prefer a System Reset after programming.

## Core Rules

- Treat `.syscfg` as the source of truth for pinmux, peripheral setup, clocks, interrupts, DMA ownership, and generated initialization.
- Prefer SysConfig + DriverLib for GPIO, UART, PWM, Timer, ADC, I2C, SPI, DMA, and clock setup.
- Do not hand-edit generated outputs such as `Debug/ti_msp_dl_config.c`, `Debug/ti_msp_dl_config.h`, the project-root `ti_msp_dl_config.c` / `ti_msp_dl_config.h` pair in Keil layouts, `device_linker.cmd`, `Objects/`, `Listings/`, object files, maps, or `.out` files.
- Preserve `.syscfg` metadata such as `@cliArgs`, `@v2CliArgs`, `@versions`, `--device`, `--package`, and `--product`.
- Do not guess generated names. Read `ti_msp_dl_config.h` and use the local macros and the local init function spelling, such as `SYSCFG_DL_init()`.
- Do not invent SysConfig fields, enum values, device metadata, board names, package names, or tool versions. Validate against local examples, SDK metadata, or SysConfig CLI.
- Preserve unrelated user code, comments, copyright headers, project layout, and existing `.syscfg` settings. If a requested feature requires a larger rewrite, explain why before making it when possible.
- Do not change device, package, SDK, compiler, CCS version, board, or debug probe without user confirmation.
- If SysConfig emits warnings, report them separately from build/flash success. Do not call a warning-producing generation "clean".
- If hardware behavior is not verified on a connected board, say that validation stopped at source, SysConfig, or build level.

## Board-Specific Pin Caution

### Custom MSPM0G3519 Board (LQFP-64)

When the project targets the custom MSPM0G3519 development board (`--device "MSPM0G3519" --package "LQFP-64(PM)"`):

**Completely unusable — never assign to any peripheral:**
- PA2: frequency accuracy control, not routed to header
- PA5, PA6: 40 MHz HFXT crystal, system clock source
- PA19, PA20: SWDIO / SWCLK debug interface

**PA18 — BSL caution:**
- PA18 is the BSL entry pin and the onboard BACK button (PULL_DOWN in normal use).
- If PA18 is high at reset, the device enters BSL mode and user firmware will not run.
- Do not drive PA18 high at power-on. Warn the user before assigning PA18 to any output or external signal that could be high at reset.

**Already occupied by board peripherals — do not reassign without user confirmation:**

| Pin(s) | Occupied by |
|--------|-------------|
| PA0, PA1 | Software I2C — OLED (SDA/SCL), board has 2.2 kΩ pull-ups; I2C bus can share additional devices |
| PA10, PA11 | UART0 TX/RX — onboard CH340; header pins can be shared |
| PA27, PA28 | Software I2C — LSM6DS3 IMU (SCL/SDA) |
| PB6, PB7, PB8, PB9 | SPI1 — W25Q128 Flash (CS/MISO/MOSI/SCLK) |
| PB17, PB18 | UART7 TX/RX — onboard 2.4 GHz wireless module |
| PB21 | User button ENTER (PULL_UP, active-low) |
| PB22 | Onboard LED (active-low, PULL_DOWN) |
| PB23 | Wireless link-status input (PULL_DOWN) |
| PB26 | TIMA1 CCP0 — WS2812 RGB LED PWM |
| PB27 | TIMG6 CCP1 — buzzer PWM |

**Optional peripherals — can be released if not used:**
- PA29, PA30: QEI encoder PHA/PHB (TIMG8 CCP0/CCP1)
- PA31: Encoder SW button (PULL_UP)

When the user asks to choose a free pin, prefer pins not listed above. If the user explicitly requests an occupied pin, explain the conflict and ask for confirmation before proceeding.

### LCKFB Tianmengxing MSPM0G3507

When the user explicitly says the board is LCKFB Tianmengxing MSPM0G3507:

- Avoid choosing A21/PA21, A23/PA23, A02/PA02, A18/PA18, A10/PA10, and A11/PA11 for ordinary user-requested pin assignments unless the user asks for those pins or the local project already deliberately uses them.
- If the user asks to drive or reuse one of those pins, remind them that the Tianmengxing documentation marks these as special pins and says they should not be used unless necessary.
- Do not silently move an existing project away from these pins. Explain the board caveat first, then ask or proceed according to the user's intent.

### LCKFB Dimengxing MSPM0G3507

When the user explicitly says the board is LCKFB Dimengxing MSPM0G3507 (立创·地猛星 MSPM0G3507 开发板):

**Board resources:**
- `MSPM0G3507`, 48-pin package (per official schematic: VCORE at pin 48; Tianmengxing is LQFP-64).
- CH340E USB-UART: PA10 (UART0_TX / BSLTX), PA11 (UART0_RX / BSLRX).
- SWD debug: PA19 (SWDIO), PA20 (SWCLK).
- 40 MHz HFXT on PA5/PA6; 32.768 kHz LFX on PA3/PA4.
- ROSC on PA2 with 100 kΩ resistor.
- BSL entry on PA18 (BSL button).
- Onboard SPI Flash W25Q32: PB6 (CS), PB7 (POCI/MISO), PB8 (PICO/MOSI), PB9 (SCLK).
- Onboard user LED: PA14, active-low (LED on when PA14 is low), 270 Ω series current-limiting resistor.
- Two 20-pin expansion headers (H3 and H5) expose most GPIO and analog pins.

**Expansion header pinout (from the official Dimengxing schematic):**

H3 (20-pin header):
`PA0, PA1, PA28, PA31, NRST, PA2/ROSC, PB24, PB20, PB19, PB18, PA7, PB2, PB3, PA8, PA9, PB6, PB7, +5V, 3V3, NC`

H5 (20-pin header):
`PA27, PA26, PA25, PA24, PA23/VREF+, PA22, PA21/VREF-, PB9, PB8, PA18, PA17, PA16, PA15, PA14, PA13, PA12, NC, +5V, 3V3, NC`

Use these lists when choosing free pins for the Dimengxing board. Pins already occupied by board peripherals (PA14 LED, PB6-PB9 flash, PA10/PA11 UART0, PA18 BSL, PA19/PA20 SWD, PA2 ROSC, PA5/PA6 HFXT, PA3/PA4 LFX) are not free.

**Pin cautions:**
- Never assign PA2, PA5, PA6, PA19, or PA20 to user peripherals; they are used by ROSC, HFXT crystal, and SWD debug.
- PA18 is the BSL entry pin. If PA18 is high at reset, the device enters BSL mode and user firmware will not run. Do not drive PA18 high at power-on.
- PA14 is the onboard user LED (active-low). Reusing PA14 for another function will disable the LED or conflict with the onboard LED circuit.
- PB6/PB7/PB8/PB9 are occupied by the onboard W25Q32 SPI Flash. Do not reassign without user confirmation.
- PA10/PA11 are connected to the onboard CH340E for UART0 and BSL; header pins can be shared for UART0 traffic.
- The Dimengxing board does not include the Tianmengxing onboard OLED, LSM6DS3 IMU, WS2812 RGB LEDs, buzzer, QEI encoder, wireless UART module, or ENTER button. Peripherals that relied on those Tianmengxing board resources must be adapted or wired externally.

When the user asks to choose a free pin on Dimengxing, prefer pins not listed above. If the user explicitly requests an occupied pin, explain the conflict and ask for confirmation before proceeding.

## Project Shape Checks

- Simple projects usually keep most logic in `main.c`, `empty.c`, or a small number of files. It is acceptable to make narrowly scoped edits there.
- Framework projects often have multiple source directories such as `app/`, `bsp/`, `components/`, `core/`, `drivers/`, `hal/`, `middleware/`, or `tasks/`. First identify ownership boundaries before adding peripherals or changing control logic.
- Do not assume every MSPM0 project is CCS-like or single-file. A framework project can still use CCS, Keil, or CMake/GCC/OpenOCD.
- For control code, confirm whether timing comes from a timer ISR, RTOS task delay, hardware PWM/ADC trigger chain, or a main-loop poll before changing periods or priorities.

## Keil Project Checks

- Treat `system.syscfg` and `ti_msp_dl_config.c` / `ti_msp_dl_config.h` as the configuration source surface for Keil-based MSPM0 projects that keep SysConfig outputs at the project root.
- Treat a Keil `.uvprojx` as the project entrypoint, the scatter file as the linker source of truth, and `Objects/`, `Listings/`, `*.uvoptx`, build logs, and generated outputs as inspection-only unless a request explicitly targets them.
- For a project's application code, follow its own source layout rather than assuming CCS defaults.

## CMake / GCC / OpenOCD Checks

- Treat `CMakeLists.txt`, toolchain files, and OpenOCD `.cfg` files as the project entrypoints for CMake/GCC/OpenOCD projects.
- Build through the existing CMake build directory when present, for example `cmake --build cmake-build-debug --target <target>`.
- MSPM0 OpenOCD flashing usually requires a TI MSPM0-capable OpenOCD build or TI extension branch. If OpenOCD reports `unable to find a matching CMSIS-DAP device`, report that as probe discovery failure rather than firmware failure.
- Do not require CCS `targetConfigs/*.ccxml` when the active project uses OpenOCD instead of DSLite.

## FreeRTOS Checks

- If `FreeRTOSConfig.h`, `FreeRTOS.h`, `task.h`, `xTaskCreate`, or `vTaskStartScheduler` are present, treat the project as RTOS-aware.
- Keep RTOS handling lightweight: respect existing task, queue, ISR, and blocking-call boundaries; do not impose a specific framework architecture unless the user asks.

## Ambiguous Requests

If the user omits important hardware parameters, do not silently choose risky values.

- For low-risk defaults, use this skill's `examples/` or local TI SDK examples, then tell the user which defaults were applied.
- For important parameters, ask before editing and offer a concrete recommendation.
- Important missing parameters include pin, peripheral instance, UART baud/data/parity/stop bits, Timer period, PWM frequency/duty/polarity, ADC channel/reference/sample time, DMA direction/source/destination, interrupt priority, and external-module power/logic levels.

Example: if the user asks "add a timer interrupt", ask which timer and period they want, and recommend a starter such as TIMG at 1 ms or 10 ms if they are unsure.

## External Modules And Hardware Debugging

When asked to drive an external module, sensor, motor driver, servo, display, radio, or custom board:

- Ask for the module datasheet, schematic, pin map, supply voltage, logic level, communication protocol, and key parameters when they are not available.
- Verify wiring assumptions before blaming code: power, ground, pull-ups, level shifting, reset/enable pins, boot pins, chip select, UART TX/RX crossover, I2C address, SPI mode, PWM polarity, and shared pins.
- If repeated attempts fail and SysConfig, build, flash, and code logic look correct, explicitly raise the possibility of wiring, power, module mode, datasheet mismatch, damaged hardware, or wrong test procedure.
- Separate "firmware looks correct" from "hardware proved correct".

## Reference Selection

Read references only when needed:

- `references/sysconfig_ccs_workflow.md`: `.syscfg` editing, CCS / Keil / CMake project layout, SysConfig CLI, gmake, CMake build, DSLite/J-Link, and OpenOCD.
- `references/driverlib_runtime_rules.md`: DriverLib usage, interrupts, clock tree, delays, and common runtime mistakes.
- `references/sdk_schema_lookup.md`: how to find official SysConfig fields and examples in the local MSPM0 SDK.
- `references/hardware_validation_notes.md`: verified Tianmengxing MSPM0G3507 lessons, HFXT warnings, flash/reset behavior, and real-board caveats.
- `references/ccs_dss_debug.md`: CCS Debug Server Scripting (`ccs-dss`) debug workflow, breakpoints, register reads, and current limitations.

Use `examples/` as the main source for reusable tested patterns. Prefer `scripts/list_examples.py` to inspect available examples before opening individual example files.

## Examples

Each reusable example should contain:

```text
examples/<name>/
├─ example.syscfg
├─ README.md
├─ manifest.json
└─ src/
   └─ source files copied from the minimal relevant project surface
```

Do not require users to drop full CCS projects into `examples/`. Use `scripts/capture_example.py` to extract a compact example package from a real project.

## Tools

- `python scripts/check_syscfg.py <project-dir>`: static project check for `.syscfg`, generated files, pins, init spelling, project shape, CCS/Keil/CMake/OpenOCD clues, build output, target config, and validation hints.
- `python scripts/list_examples.py`: list packaged examples from `examples/*/manifest.json`.
- `python scripts/capture_example.py <project-dir> --name <example-name> --include <glob>`: package selected source files and `.syscfg` from a user project into `examples/<example-name>/`.
- `python scripts/index_syscfg_examples.py <mspm0-sdk-root> --board LP_MSPM0G3507 --module UART`: search local TI SDK examples and module metadata.
- `python scripts/serial_console.py --list`: list serial ports.
- `python scripts/ccs_dss_debug.py <project-dir> probe --leave-running`: connect through CCS Debug Server Scripting, read reset/register state, verify the configured `.ccxml` debug path, and continue the target before disconnecting.

For the verified CH340 setup, use `python scripts/serial_console.py -p COM6 -b 115200 --timestamp --duration 10` after closing other serial tools such as VOFA+.

## Flash Backends

> ⚠️ **严禁使用 ST-LINK 进行下载或调试！**
> ST-LINK 会锁死 MSPM0 芯片，下载时显示 PDSC 错误。天猛星 / 地猛星 / 自定义 G3519 板仅支持以下烧录方式：
> - **J-Link**（推荐，经 CCS / UniFlash 验证）
> - **DSLite**（CCS 自带，配合 J-Link 或 XDS110 使用）
> - **CH340 + BSL**（串口烧录，无需额外调试器）
> - **OpenOCD**（仅限 CMake/GCC 项目，需 TI 分支 OpenOCD）

The verified CCS flash path is DSLite / UniFlash with J-Link. For automated flashing after clock-tree changes, prefer DSLite System Reset:

```text
dslite -c <target.ccxml> -e -r 2 -u <project.out>
```

For CMake/GCC/OpenOCD projects, use the project's existing flash target or explicit OpenOCD config. Keep the backend explicit and report probe-discovery errors separately from build success.

## Debug Backends

The currently packaged automated debug helper is the CCS Debug Server Scripting backend (`ccs-dss`):

```text
python scripts/ccs_dss_debug.py <project-dir> probe --leave-running
python scripts/ccs_dss_debug.py <project-dir> run-to-symbol --symbol main --load --reset "System Reset"
```

Use it only for CCS / CCS Theia / UniFlash-style projects with a valid `targetConfigs/*.ccxml`. The physical probe is selected by `.ccxml`, so the backend is not inherently J-Link-only; it can also work with CCS-supported probes such as XDS110 when the project configuration matches the hardware.

Do not treat `ccs-dss` as the OpenOCD path. For CMake/GCC/OpenOCD projects, keep future debugging under a separate `openocd-gdb` backend. Debug actions can halt the CPU, so report that risk before using breakpoints or register inspection on real-time control hardware.

## 助力全国大学生电子设计竞赛 (NUEDC)

本 Skill 同时深入适配 **立创·天猛星**（MSPM0G3507，板载 OLED/IMU/WS2812/无线模块）与 **立创·地猛星**（MSPM0G3507，精简版，板载 W25Q32 Flash、PA14 LED），两款开发板共用同一 MCU 型号，但天猛星为 LQFP-64（64 脚）、地猛星为 48 脚封装，引脚映射与避坑指南高度互通。
- 内置 `references/MSPM0G3507_Pinout_Mapping.md` 提供完整引脚复用和避坑指南，覆盖两款板卡。
- 引导 Agent 自动识别当前板型，避开特殊系统引脚（PA18 BSL、PA5/PA6 HFXT、PA19/PA20 SWD、PA2 ROSC 等），并根据板载资源分配通信总线。
- 天猛星示例（PB22 LED、OLED、IMU、WS2812）与地猛星建议（PA14 LED、W25Q32 Flash）均有独立保护规则，避免误配。
- 加速外设配置验证速度，加油，电赛人！
