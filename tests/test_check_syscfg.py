from __future__ import annotations

import contextlib
import io
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from scripts import check_syscfg


FIXTURES = ROOT / "tests" / "fixtures"


class CheckSyscfgTests(unittest.TestCase):
    def run_main(self, *args: str) -> tuple[int, str]:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            code = check_syscfg.main(list(args))
        return code, output.getvalue()

    def test_valid_project_has_no_errors(self) -> None:
        messages, details = check_syscfg.check_project(FIXTURES / "valid_project")
        self.assertFalse([message for message in messages if message.level == "error"])
        self.assertEqual(details["header_init_names"], ["SYSCFG_DL_init"])
        self.assertEqual(details["assigned_pins:example.syscfg"][0]["assignedPin"], "PB22")

    def test_init_name_mismatch_is_an_error(self) -> None:
        messages, _ = check_syscfg.check_project(FIXTURES / "mismatch_project")
        errors = [message.text for message in messages if message.level == "error"]
        self.assertTrue(any("不存在的初始化函数" in text for text in errors))
        code, output = self.run_main(str(FIXTURES / "mismatch_project"))
        self.assertEqual(code, check_syscfg.EXIT_CHECK_FAILED)
        self.assertIn("ERROR", output)

    def test_warnings_are_non_fatal_by_default(self) -> None:
        code, output = self.run_main(str(FIXTURES / "warning_project"))
        self.assertEqual(code, check_syscfg.EXIT_OK)
        self.assertIn("WARNING", output)

    def test_strict_mode_treats_warnings_as_failure(self) -> None:
        code, _ = self.run_main("--strict", str(FIXTURES / "warning_project"))
        self.assertEqual(code, check_syscfg.EXIT_CHECK_FAILED)

    def test_board_database_reports_occupied_pin(self) -> None:
        messages, _ = check_syscfg.check_project(
            FIXTURES / "valid_project", board_id="tianmengxing"
        )
        warnings = [message.text for message in messages if message.level == "warning"]
        self.assertTrue(any("PB22" in text and "板卡 tianmengxing" in text for text in warnings))

    def test_unknown_board_is_an_error(self) -> None:
        messages, _ = check_syscfg.check_project(
            FIXTURES / "valid_project", board_id="not-a-board"
        )
        errors = [message.text for message in messages if message.level == "error"]
        self.assertTrue(any("无法加载板卡数据库" in text for text in errors))

    def test_json_output_is_machine_readable(self) -> None:
        code, output = self.run_main("--json", str(FIXTURES / "valid_project"))
        self.assertEqual(code, check_syscfg.EXIT_OK)
        self.assertIn('"messages"', output)
        self.assertIn('"details"', output)

    def test_invalid_argument_uses_usage_exit_code(self) -> None:
        with self.assertRaises(SystemExit) as raised:
            check_syscfg.main(["--unknown-option"])
        self.assertEqual(raised.exception.code, check_syscfg.EXIT_USAGE)


if __name__ == "__main__":
    unittest.main()
