from __future__ import annotations

import contextlib
import io
import json
import tempfile
import unittest
from pathlib import Path

from scripts import list_examples


class ListExamplesTests(unittest.TestCase):
    def test_json_output_lists_examples(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "examples"
            example = root / "blink"
            example.mkdir(parents=True)
            (example / "manifest.json").write_text(json.dumps({"name": "blink"}), encoding="utf-8")
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                code = list_examples.main(["--examples-dir", str(root), "--json", "--strict"])
            self.assertEqual(code, 0)
            self.assertIn('"name": "blink"', output.getvalue())

    def test_strict_mode_rejects_invalid_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / "examples"
            example = root / "broken"
            example.mkdir(parents=True)
            (example / "manifest.json").write_text("{broken", encoding="utf-8")
            with contextlib.redirect_stdout(io.StringIO()):
                code = list_examples.main(["--examples-dir", str(root), "--strict"])
            self.assertEqual(code, 1)


if __name__ == "__main__":
    unittest.main()
