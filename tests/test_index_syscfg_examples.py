"""Smoke/unit tests for scripts/index_syscfg_examples.py.

These tests cover the pure helper functions and importability. They do not
require an actual MSPM0 SDK on disk.
"""

import tempfile
import unittest
from pathlib import Path

from scripts import index_syscfg_examples as idx


class IndexSyscfgExamplesTests(unittest.TestCase):
    def test_module_importable_as_package(self):
        self.assertTrue(hasattr(idx, "find_examples"))
        self.assertTrue(hasattr(idx, "find_metadata"))
        self.assertTrue(hasattr(idx, "main"))

    def test_split_filters_handles_csv_and_spaces(self):
        self.assertEqual(idx.split_filters(None), set())
        self.assertEqual(idx.split_filters(["uart, gpio", "DMA"]), {"UART", "GPIO", "DMA"})

    def test_modules_from_syscfg_parses_addmodule(self):
        text = (
            'scripting.addModule("/ti/driverlib/gpio.js");\n'
            'scripting.addModule("/ti/driverlib/uart.js");\n'
        )
        with tempfile.NamedTemporaryFile("w", suffix=".syscfg", delete=False, encoding="utf-8") as tmp:
            tmp.write(text)
            tmp_path = Path(tmp.name)
        try:
            self.assertEqual(idx.modules_from_syscfg(tmp_path), ["GPIO.JS", "UART.JS"])
        finally:
            tmp_path.unlink(missing_ok=True)

    def test_filter_examples_by_module_and_board(self):
        example = idx.SyscfgExample(
            path="examples/LP_MSPM0G3507/uart/uart.syscfg",
            board="LP_MSPM0G3507",
            modules=["UART", "GPIO"],
        )
        self.assertTrue(idx.matches_module(example, {"UART"}))
        self.assertFalse(idx.matches_module(example, {"SPI"}))
        self.assertTrue(idx.matches_board(example, "LP_MSPM0G3507"))
        self.assertFalse(idx.matches_board(example, "LP_MSPM0L1306"))

    def test_infer_board_from_path(self):
        root = Path("/sdk")
        self.assertEqual(idx.infer_board(root / "examples" / "LP_MSPM0G3507" / "uart" / "u.syscfg", root), "LP_MSPM0G3507")
        self.assertIsNone(idx.infer_board(root / "examples" / "nested" / "u.syscfg", root))

    def test_find_examples_empty_for_missing_sdk(self):
        self.assertEqual(idx.find_examples(Path("/nonexistent-sdk-xyz-123")), [])


if __name__ == "__main__":
    unittest.main()
