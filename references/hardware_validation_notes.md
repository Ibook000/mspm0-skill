# Hardware Validation Notes

Use this for verified Tianmengxing MSPM0G3507 lessons and real-board caveats.

## Verified Environment

Validated combination:

- Board: LCKFB Tianmengxing MSPM0G3507
- IDE: CCS / CCS Theia
- SDK: MSPM0 SDK 2.10.00.04
- SysConfig: 1.26.2
- Compiler: TI Arm Clang 4.0.3 LTS
- Debug probe: J-Link through UniFlash / DSLite
- Validated peripherals: PB22 onboard LED, UART0 blocking TX, PB22 TIMG8 PWM breathing LED
- Validated clock: 80 MHz CPUCLK with MFCLK 4 MHz for UART work

Other boards, packages, SDK versions, CCS versions, probes, and pin maps may work, but they are not guaranteed by these notes.

## Tianmengxing Special Pin Caution

The LCKFB Tianmengxing documentation marks A21, A23, A02, A18, A10, and A11 as special pins and says they should not be used unless necessary. In SysConfig or generated headers these may appear as PA21, PA23, PA02, PA18, PA10, and PA11.

When the user asks the agent to choose pins for normal GPIO, PWM, UART, SPI, I2C, timer capture, or similar tasks on Tianmengxing, prefer other available pins first. If the user explicitly requests one of these pins, or an existing project already uses one, warn about the board note before changing or depending on it.

## PB22 LED Lessons

The LCKFB Tianmengxing onboard LED uses PB22. A verified GPIO blink used generated names similar to:

```c
LED_PORT
LED_PIN_22_PIN
SYSCFG_DL_init()
```

The original LED blink was a 32 MHz baseline using `delay_cycles(32000000)`.

## 80 MHz Clock Tree Lessons

The verified 80 MHz pattern uses HFXT 40 MHz on PA5/PA6, SYSPLL, ULPCLK divider 2, and MFCLK gate enabled.

SysConfig can generate successfully while warning:

```text
HFXT peripheral.$assign: Solution may have changed
HFXT peripheral.hfxInPin.$assign: Solution may have changed
HFXT peripheral.hfxOutPin.$assign: Solution may have changed
```

Do not hide this. Confirm generated `CPUCLK_FREQ`, `GPIO_HFXIN_*`, and `GPIO_HFXOUT_*`, or ask the user to inspect the clock tree GUI.

## UART0 Blocking TX Lessons

The verified UART smoke test used UART0 at 115200 8N1 with PA10/PA11 and a CH340 PC adapter. Treat it as a blocking transmit baseline, not a final DMA or variable-length receive design.

## PWM Breathing LED Lessons

The verified PB22 PWM example used TIMG8 CCP1, a period of 1000 counts, and generated macro `GPIO_PWM_0_C1_IDX`.

Successful runtime pattern:

- set the first compare value before starting the timer
- update CCP1, not channel 0
- avoid exact compare boundaries `0` and `period`
- use `1..999` for a period of `1000`
- at 80 MHz, `delay_cycles(800000)` is roughly 10 ms per step

Failed patterns included one-second delay per brightness step and exact boundary values that made the LED appear off or glitchy.

## Flash And Reset

Manual load-and-run after a clock-tree change can behave differently from a full reset. A verified 80 MHz test blinked at about 2.5 seconds immediately after plain flash, then about one second after board reset. DSLite `-r 2 -u` made automated flashing start correctly.

If J-Link connection fails after a previous attempt, stale `DSLite`, `JLink`, or `JLinkGUIServer` processes may need to be closed before retrying.

---

## MSPM0G3519 Custom Board Lessons

### Verified Environment

Validated combination:

- Board: Custom MSPM0G3519 development board (LQFP-64)
- IDE: CCS
- SDK: MSPM0 SDK 2.05.01.01
- SysConfig: 1.24.0+4110
- Device metadata: `--device "MSPM0G3519" --package "LQFP-64(PM)"`
- Debug probe: J-Link
- Validated peripherals: PB22 LED, UART0 debug TX/RX, UART7 wireless, SPI1 Flash (W25Q128), I2C software (OLED + LSM6DS3 IMU), TIMA1 PWM (WS2812), TIMG6 PWM (buzzer), TIMG8 QEI encoder, TIMA0 5 ms tick, DMA CH0

### System-Level Unusable Pins

These pins must never be assigned to user peripherals:

- **PA2**: Frequency accuracy control, not routed to header, system occupied
- **PA5**: HFXT crystal input (40 MHz), system occupied
- **PA6**: HFXT crystal output (40 MHz), system occupied
- **PA19**: SWDIO, debug interface, do not connect other devices
- **PA20**: SWCLK, debug interface, do not connect other devices

### PA18 BSL Caution

PA18 is the BSL (Bootstrap Loader) entry pin and also the BACK button on this board. It uses `PULL_DOWN` in normal operation. If PA18 is high at reset, the device enters BSL mode instead of running user firmware. Ensure PA18 is not driven high at power-on or reset.

### 80 MHz Clock Tree (Verified)

Clock tree configuration verified in the OLED_UI full-feature project:

- HFXT: 40 MHz external crystal on PA5/PA6, `HFXTStartup = 10`, `HFCLKMonitor = true`
- EXHFMUX: `XTAL`
- SYSPLLMUX: `HFCLK`
- PLL_QDIV multiply: 4 (40 MHz × 4 / 2 = 80 MHz)
- HSCLKMUX: `SYSPLL0`
- UDIV: 2
- Result: `CPUCLK_FREQ = 80000000`
- Flash wait state: 2 cycles (required at 80 MHz; suppress the `DL_FlashCTL_executeClearStatus` warning via `scripting.suppress(...)` in `.syscfg`)

Generated macros to verify after SysConfig regeneration:

```c
#define CPUCLK_FREQ        80000000
#define GPIO_HFXIN_PIN     DL_GPIO_PIN_5
#define GPIO_HFXOUT_PIN    DL_GPIO_PIN_6
```

### UART0 Debug (Verified)

- Instance: `UART0`, pins PA10 (TX) / PA11 (RX), baud 9600, 8N1
- Board has onboard CH340 connected to PA10/PA11; header pins can also be used simultaneously
- Generated macros: `UART_DEBUG_INST`, `GPIO_UART_DEBUG_TX_PIN`, `GPIO_UART_DEBUG_RX_PIN`, `UART_DEBUG_BAUD_RATE`
- UART clock source runs at 40 MHz (`UART_DEBUG_INST_FREQUENCY = 40000000`)

### UART7 Wireless (Verified)

- Instance: `UART7`, pins PB17 (TX) / PB18 (RX), baud 9600, 8N1
- Connected to onboard 2.4 GHz wireless UART module
- PB23 is the link-status input (low = connected, high = disconnected), `PULL_DOWN`
- Generated macros: `UART_WIRELESS_INST`, `GPIO_UART_WIRELESS_TX_PIN`, `GPIO_UART_WIRELESS_RX_PIN`

### SPI1 Flash W25Q128 (Verified)

- Instance: `SPI1`, 20 MHz, MOTO3 frame format
- Pins: PB9 (SCLK), PB8 (MOSI/PICO), PB7 (MISO/POCI), PB6 (CS via GPIO)
- CS is a plain GPIO output with `PULL_DOWN`, not a hardware SPI CS
- DMA CH0 used for burst transfers (24-byte, subscriber index 0, `FSUB_0` trigger)
- Generated macros: `SPI_FLASH_INST`, `GPIO_SPI_FLASH_SCLK_PIN`, `GPIO_SPI_FLASH_PICO_PIN`, `GPIO_SPI_FLASH_POCI_PIN`, `FLASH_CS_PIN`, `FLASH_PORT`

### I2C (Software, Verified)

- OLED (SSD1312 128×64): PA0 (SDA), PA1 (SCL), open-drain, initial HIGH, board has 2.2 kΩ pull-ups
- IMU (LSM6DS3, I2C addr 0x6A): PA28 (SDA), PA27 (SCL), software I2C
- Both buses use bit-bang software I2C, not hardware I2C peripheral
- Generated macros: `OLED_SDA_PIN`, `OLED_SCL_PIN`, `OLED_PORT`

### WS2812 RGB LED (Verified)

- Instance: `TIMA1`, CCP0, pin PB26, 80 MHz clock, period 100 counts
- 4 addressable RGB LEDs, single-wire PWM protocol
- Generated macros: `WS2812_INST`, `GPIO_WS2812_C0_PIN`, `GPIO_WS2812_C0_IDX`, `WS2812_INST_CLK_FREQ`

### Buzzer PWM (Verified)

- Instance: `TIMG6`, CCP1, pin PB27, prescale 80 → 1 MHz clock, period 1 count (frequency set at runtime)
- Generated macros: `BUZZER_INST`, `GPIO_BUZZER_C1_PIN`, `GPIO_BUZZER_C1_IDX`, `BUZZER_INST_CLK_FREQ`

### QEI Encoder (Verified)

- Instance: `TIMG8`, CCP0 = PA29 (PHA), CCP1 = PA30 (PHB)
- Encoder SW button: PA31, `PULL_UP`, GPIO input
- Dual backend: hardware QEI (`USE_QEI_ENCODER = 1`) or software GPIO fallback
- Generated macros: `QEI_ENCODER_INST`, `GPIO_QEI_ENCODER_PHA_PIN`, `GPIO_QEI_ENCODER_PHB_PIN`, `KEY_ENCODER_SW_PIN`
- PA29/PA30/PA31 can be released if encoder is not used

### System Tick Timer (Verified)

- Instance: `TIMA0`, periodic, 5 ms period, interrupt priority 3
- Clock: 80 MHz / div 8 / prescale 10 = 1 MHz → load value 4999
- Generated macros: `TIMER_TICK_INST`, `TIMER_TICK_INST_IRQHandler`, `TIMER_TICK_INST_LOAD_VALUE`

### PB22 LED (Verified)

- Active-low (high = off, low = on), `PULL_DOWN`
- Generated macros: `DEBUG_LED_PORT` (GPIOB), `DEBUG_LED_PIN_22_PIN` (DL_GPIO_PIN_22)
