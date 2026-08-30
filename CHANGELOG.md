# 变更日志

本文件记录对外可见的功能、规则、示例和验证流程变化。

## 未发布

### Added
- 新增立创·地正星 MSPM0L1306 板卡数据库：PA14 LED（高电平点亮，与地猛星极性相反）、PA18 BSL 按键（47 kΩ 下拉）、CH340E UART0（PA22/PA23）。
- 增加立创（LCKFB）官方 wiki 资源索引：模块移植手册、入门手册与串口/Keil 下载报错排查。
- 增加机器可读的板卡数据库和板卡文档生成流程。
- 增加示例 manifest JSON Schema、板卡 JSON Schema 和 CI 校验。
- 增加 `capture_example.py`、`serial_console.py`、板卡冲突检查和示例快照模式的回归测试。
- 增加贡献规范和验证证据字段约定。

### Changed

- `manifest.json` 统一使用 `sdk`、`sysconfig` 和 `syscfg` 字段。
- `check_syscfg.py` 的警告、严格模式、板卡模式和快照模式行为更加明确。
- 板卡 Markdown 参考文档改为由 `boards/*.json` 自动生成。
