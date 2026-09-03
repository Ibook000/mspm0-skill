"""Smoke tests for scripts/verify_example.py."""

import unittest

from scripts import verify_example


class VerifyExampleTests(unittest.TestCase):
    def test_module_importable_as_package(self):
        # Regression: verify_example.py does a bare `from check_syscfg import ...`.
        # It must be importable both as `python scripts/verify_example.py` and as
        # `from scripts import verify_example` (the previous version raised
        # ModuleNotFoundError: No module named 'check_syscfg' here).
        self.assertTrue(hasattr(verify_example, "main"))
        self.assertTrue(hasattr(verify_example, "verify"))
        self.assertTrue(hasattr(verify_example, "check_project"))
        self.assertTrue(hasattr(verify_example, "EXIT_CHECK_FAILED"))

    def test_main_returns_int_for_example(self):
        # main() should run to completion and return an int exit code (it must
        # not raise). Either 0 or non-zero is acceptable for a smoke test.
        rc = verify_example.main(["examples/led_blink", "--snapshot"])
        self.assertIsInstance(rc, int)


if __name__ == "__main__":
    unittest.main()
