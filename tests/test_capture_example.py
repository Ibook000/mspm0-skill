from __future__ import annotations

import json
import shutil
import tempfile
import unittest
from pathlib import Path

from jsonschema import Draft202012Validator

from scripts import capture_example


ROOT = Path(__file__).resolve().parents[1]
SCHEMA = json.loads(
    (ROOT / "schemas" / "example-manifest.schema.json").read_text(encoding="utf-8")
)


class CaptureExampleTests(unittest.TestCase):
    def make_project(self) -> Path:
        temp = Path(tempfile.mkdtemp())
        self.addCleanup(shutil.rmtree, temp)
        (temp / "src").mkdir()
        (temp / "Debug").mkdir()
        (temp / "example.syscfg").write_text(
            '@cliArgs --device "MSPM0G3507" --package "LQFP-64" --product "MSPM0 SDK 2.10.00.04"\n'
            '@versions {"tool":"1.26.2"}\n'
            'scripting.addModule("/ti/driverlib/GPIO");\n'
            'GPIOA.$assign = "PA14";\n',
            encoding="utf-8",
        )
        (temp / "src" / "main.c").write_text("int main(void) { return 0; }\n", encoding="utf-8")
        (temp / "Debug" / "generated.out").write_bytes(b"not source")
        return temp

    def test_auto_capture_generates_schema_valid_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as output_dir:
            project = self.make_project()
            code = capture_example.main(
                [
                    str(project),
                    "--name",
                    "captured_test",
                    "--auto",
                    "--examples-dir",
                    output_dir,
                ]
            )
            self.assertEqual(code, 0)
            dest = Path(output_dir) / "captured_test"
            manifest = json.loads((dest / "manifest.json").read_text(encoding="utf-8"))
            self.assertEqual(manifest["sdk"], "MSPM0 SDK 2.10.00.04")
            self.assertEqual(manifest["sysconfig"], "{\"tool\":\"1.26.2\"}")
            self.assertEqual(list(Draft202012Validator(SCHEMA).iter_errors(manifest)), [])
            self.assertTrue((dest / "src" / "main.c").exists())
            self.assertFalse((dest / "src" / "Debug" / "generated.out").exists())

    def test_no_selected_sources_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as output_dir:
            project = self.make_project()
            with self.assertRaises(SystemExit) as raised:
                capture_example.main(
                    [
                        str(project),
                        "--name",
                        "empty_capture",
                        "--include",
                        "missing/**/*.c",
                        "--examples-dir",
                        output_dir,
                    ]
                )
            self.assertIn("No source files selected", str(raised.exception))

    def test_invalid_validation_level_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as output_dir:
            project = self.make_project()
            with self.assertRaises(SystemExit):
                capture_example.main(
                    [
                        str(project),
                        "--name",
                        "bad_level",
                        "--auto",
                        "--validation-level",
                        "unverified",
                        "--examples-dir",
                        output_dir,
                    ]
                )


if __name__ == "__main__":
    unittest.main()
