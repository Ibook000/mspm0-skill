# OLED UI Full Feature (MSPM0G3519)

Full-feature OLED UI project for custom MSPM0G3519 board: 80 MHz clock, OLED SSD1312, LSM6DS3 IMU, W25Q128 Flash, WS2812 RGB, buzzer, QEI encoder, UART0 debug, UART7 wireless, DMA, 5ms tick. Hardware validated.

This example was captured from a user project. It is a compact reference package, not a full CCS import project.

## Summary

- Board: Custom MSPM0G3519 LQFP-64
- Device: MSPM0G351X
- Complexity: advanced
- Validation: hardware
- Peripherals: Board, DMA, GPIO, PWM, ProjectConfig, QEI, SPI, SYSCTL, TIMER, UART
- Pins: PA0, PA1, PA10, PA11, PA18, PA19, PA20, PA29, PA30, PA31, PA5, PA6, PB17, PB18, PB21, PB22, PB23, PB26, PB27, PB6, PB7, PB8, PB9, pin:0, pin:1, pin:18, pin:21, pin:22, pin:23, pin:31, pin:6

## Files

- `example.syscfg`: captured SysConfig source
- `src/`: selected source files
- `manifest.json`: machine-readable summary for example selection

## Notes

- Rebuild the target project after copying patterns from this example.
- Inspect the target project's generated `ti_msp_dl_config.h`; generated names may differ.
- For advanced examples, copy only the relevant module pattern instead of transplanting the whole source tree.
