from __future__ import annotations

import contextlib
import io
import unittest

from scripts import serial_console


class SerialConsoleTests(unittest.TestCase):
    def test_format_bytes_text_and_hex(self) -> None:
        self.assertEqual(serial_console.format_bytes(b"A\xff", as_hex=False, encoding="utf-8"), "A�")
        self.assertEqual(serial_console.format_bytes(b"A\xff", as_hex=True, encoding="utf-8"), "41 FF")

    def test_parser_help_does_not_require_pyserial(self) -> None:
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            with self.assertRaises(SystemExit) as raised:
                serial_console.build_parser().parse_args(["--help"])
        self.assertEqual(raised.exception.code, 0)
        self.assertIn("Read and write a UART serial port", output.getvalue())

    def test_missing_dependency_is_deferred_until_operation(self) -> None:
        if serial_console.serial is not None:
            self.skipTest("pyserial is installed in this environment")
        with self.assertRaises(serial_console.SerialDependencyError):
            serial_console.require_serial()


if __name__ == "__main__":
    unittest.main()
