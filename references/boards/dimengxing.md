# 立创·地猛星 MSPM0G3507

## 识别条件

当项目明确使用 LCKFB Dimengxing / 立创·地猛星 MSPM0G3507，或 `.syscfg`、封装信息和板卡资料能够相互印证时，读取本文件。地猛星使用 MSPM0G3507 48-pin 封装；不要将天猛星 LQFP-64 的引脚资源表直接套用到本板。

## 板载资源与特殊引脚

| 引脚 | 板载功能 | 注意事项 |
|---|---|---|
| PA2 | ROSC，100 kΩ 电阻 | 不得分配给普通外设 |
| PA3、PA4 | 32.768 kHz LFXT | 使用低频晶振时保留 |
| PA5、PA6 | 40 MHz HFXT 晶振 | 不得分配给普通外设 |
| PA10、PA11 | CH340E UART0 / BSLTX/BSLRX | 板载 USB-UART 共享引脚 |
| PA14 | 板载 LED，低有效，270 Ω 限流 | 低电平点亮；复用会影响 LED |
| PA18 | BSL 入口按键 | 复位时拉高会进入 BSL，不要让外部输出在上电时拉高 |
| PA19、PA20 | SWDIO / SWCLK | 保留调试接口 |
| PB6、PB7、PB8、PB9 | 板载 W25Q32 SPI Flash | 复用前确认 Flash 是否仍需使用 |

## 扩展排针

H3（20-pin）：`PA0, PA1, PA28, PA31, NRST, PA2/ROSC, PB24, PB20, PB19, PB18, PA7, PB2, PB3, PA8, PA9, PB6, PB7, +5V, 3V3, NC`

H5（20-pin）：`PA27, PA26, PA25, PA24, PA23/VREF+, PA22, PA21/VREF-, PB9, PB8, PA18, PA17, PA16, PA15, PA14, PA13, PA12, NC, +5V, 3V3, NC`

扩展排针暴露的引脚不等于空闲引脚。PA14、PB6/PB7/PB8/PB9、PA18、PA2、PA5/PA6、PA19/PA20 仍受板载功能约束。

## 与天猛星的差异

地猛星没有天猛星的板载 OLED、LSM6DS3 IMU、WS2812、蜂鸣器、QEI 编码器、无线 UART 模块和 ENTER 按键。移植天猛星示例时，应把这些资源改为外接模块或删除对应功能；尤其不能把 PB22 板载 LED 假定为地猛星的板载 LED，地猛星板载 LED 是 PA14。

## 选择引脚的规则

用户询问空闲引脚时，先排除 PA2、PA3、PA4、PA5、PA6、PA18、PA19、PA20、PA14、PA10、PA11 和 PB6-PB9，再结合当前 `.syscfg`、H3/H5 和原理图确认。用户明确指定已占用引脚时，先说明冲突并请求确认。

## 验证要求

修改引脚后至少运行 `python scripts/check_syscfg.py <project-dir>`，检查封装、HFXT、LFXT、ROSC、SWD、BSL、UART、LED 和 SPI Flash 冲突。没有实际接入地猛星硬件时，只能报告源码、SysConfig 或编译级别结果。
