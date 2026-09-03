"""Smoke tests for scripts/ccs_dss_debug.py.

These tests exercise the importable, pure helpers and argument parser only.
They never spawn the CCS DSS runtime, so they run without any hardware or CCS
installation.
"""

import unittest
from pathlib import Path

from scripts import ccs_dss_debug


class CcsDssDebugTests(unittest.TestCase):
    def test_module_importable_as_package(self):
        # Regression: the module must be importable both as a script
        # (python scripts/ccs_dss_debug.py) and as a package member.
        self.assertTrue(hasattr(ccs_dss_debug, "build_arg_parser"))
        self.assertTrue(hasattr(ccs_dss_debug, "make_probe_js"))
        self.assertTrue(hasattr(ccs_dss_debug, "make_load_js"))
        self.assertTrue(hasattr(ccs_dss_debug, "main"))

    def test_build_arg_parser_returns_parser(self):
        parser = ccs_dss_debug.build_arg_parser()
        self.assertTrue(hasattr(parser, "parse_args"))

    def test_make_probe_js_contains_expected_markers(self):
        js = ccs_dss_debug.make_probe_js(1000, Path("/tmp/target.ccxml"), leave_running=False)
        self.assertIsInstance(js, str)
        self.assertIn("initScripting", js)
        self.assertIn("/tmp/target.ccxml", js)
        self.assertIn("configured", js)

    def test_make_probe_js_leave_running_toggles_run(self):
        default = ccs_dss_debug.make_probe_js(1000, Path("/tmp/t.ccxml"), leave_running=False)
        running = ccs_dss_debug.make_probe_js(1000, Path("/tmp/t.ccxml"), leave_running=True)
        self.assertNotIn("left_target_running", default)
        self.assertIn("left_target_running", running)


if __name__ == "__main__":
    unittest.main()
