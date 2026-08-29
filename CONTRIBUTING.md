# 贡献指南

感谢参与 MSPM0 Skill。提交代码、示例或板卡资料前，请先确认修改范围和内容来源，并保持机器可读数据与生成文档同步。

## 新增或修改示例

每个示例应包含 `README.md`、`manifest.json`、`.syscfg` 文件和最小必要源码。manifest 必须符合 [`schemas/example-manifest.schema.json`](schemas/example-manifest.schema.json)，并填写 SDK、SysConfig、板卡、验证等级和已知限制。推荐使用 `scripts/capture_example.py` 生成示例，再运行：

```bash
python -m unittest discover -s tests -v
python scripts/validate_repo.py
python scripts/check_syscfg.py examples/<name> --snapshot --strict
```

未经真实硬件验证的示例不得标记为 `hardware` 或 `hardware_serial`。如果使用板卡专属引脚，应在 `boards/*.json` 中更新数据，并运行 `python scripts/generate_board_docs.py`，不要直接手改生成的 `references/boards/*.md`。

## 修改板卡数据库

板卡 JSON 是板卡识别、引脚占用、板载资源和风险等级的主要数据源。新增 GPIO 时必须说明 `severity`：`blocked` 表示不可分配，`confirm` 表示需要用户确认，`release` 表示先确认板载功能已经释放。提交前必须运行板卡文档漂移检查。

## Pull Request 要求

Pull Request 应说明修改原因、影响范围、验证命令和未验证部分。CI 必须通过；如果修改硬件规则，还应说明依据、适用芯片/封装和板卡版本。请不要提交 `.DS_Store`、构建输出、Python 缓存或含有个人路径和凭据的文件。
