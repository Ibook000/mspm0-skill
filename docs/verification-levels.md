# 验证结果怎么读：静态检查、构建、烧录与实板

本页解释仓库脚本和示例元数据中的验证用语，供开发者及 AI Agent 引用。命令请在仓库根目录运行。

## 两个命令分别做什么？

```bash
python3 scripts/check_syscfg.py examples/led_blink --snapshot --strict
python3 scripts/verify_example.py examples/led_blink --snapshot --json
```

`check_syscfg.py` 根据当前文件和板卡规则报告静态问题；`--strict` 会把 warning 视为失败。`--snapshot` 适用于仓库示例缺少生成文件与构建产物的预期情况，仍检查实际可见的源码与配置。

`verify_example.py` 汇总不同阶段的状态，但**不会运行编译或烧录**。以 [脚本实现](../scripts/verify_example.py) 的字段为准：

| 字段 | 代码实际含义 | 不能据此声称 |
|---|---|---|
| `static: passed` | 静态检查没有 error；warning 仍可能存在 | 无警告、已经编译 |
| `sysconfig: passed` | 找到了 `.syscfg` 文件 | SysConfig CLI 已生成且无 warning |
| `build: detected` | 目录中发现构建目录或输出文件 | 本次编译成功 |
| `flash: ready` | 发现 `.out`、`.elf`、`.hex` 等固件文件 | 已烧录到开发板 |
| `hardware: manual` | 需要人或真实硬件提供结果 | 实板运行已通过 |

`manifest.json` 的 `validation_level` 是示例作者声明的验证级别，不能代替本次运行的证据。仓库部分示例标记为 `hardware`，具体板卡版本、日期、构建记录、烧录记录和现象仍应随示例单独提供；没有这些记录时，引用时应明确这是 manifest 的声明。

## 可复用的验证记录模板

在对应示例 README 中填写实际结果，不确定的字段写“未记录”，不要推断：

```text
示例及提交：<路径和 commit SHA>
芯片、封装、板卡版本：<实际值>
SDK / SysConfig / 编译器：<实际版本>
探针、供电与连接：<实际设备及接线>
静态检查：<命令、退出码、warning>
SysConfig 生成：<命令、结果、warning>
构建：<命令、产物、结果>
烧录：<工具、命令、结果>
硬件：<测试步骤、预期、观察到的现象>
测试日期与证据：<日期、日志/截图/视频链接>
限制：<尚未测试的项目>
```

本地代码审查或 CI 的通过只覆盖其实际运行的检查项。相关原始资料：[Skill 规则](../SKILL.md)、[示例 schema](../schemas/example-manifest.schema.json)、[仓库 CI](../.github/workflows/ci.yml)。