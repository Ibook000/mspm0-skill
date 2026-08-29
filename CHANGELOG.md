# 变更日志

本文件记录对外可见的功能、规则、示例和验证流程变化。

## 未发布

### Added

- 增加机器可读的板卡数据库和板卡文档生成流程。
- 增加示例 manifest JSON Schema、板卡 JSON Schema 和 CI 校验。
- 增加 `capture_example.py`、`serial_console.py`、板卡冲突检查和示例快照模式的回归测试。
- 增加贡献规范和验证证据字段约定。

### Changed

- `manifest.json` 统一使用 `sdk`、`sysconfig` 和 `syscfg` 字段。
- `check_syscfg.py` 的警告、严格模式、板卡模式和快照模式行为更加明确。
- 板卡 Markdown 参考文档改为由 `boards/*.json` 自动生成。
