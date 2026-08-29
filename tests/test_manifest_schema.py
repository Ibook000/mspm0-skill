from __future__ import annotations

import copy
import json
import sys
import unittest
from pathlib import Path

from jsonschema import Draft202012Validator

ROOT = Path(__file__).resolve().parents[1]
SCHEMA_PATH = ROOT / "schemas" / "example-manifest.schema.json"
sys.path.insert(0, str(ROOT))


class ManifestSchemaTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
        cls.validator = Draft202012Validator(cls.schema)
        cls.manifest = json.loads(
            (ROOT / "examples" / "pwm_breath_led" / "manifest.json").read_text(
                encoding="utf-8"
            )
        )

    def assert_invalid(self, manifest: dict, fragment: str) -> None:
        errors = list(self.validator.iter_errors(manifest))
        self.assertTrue(errors)
        self.assertTrue(any(fragment in error.message for error in errors), errors)

    def test_packaged_manifest_is_valid(self) -> None:
        self.assertEqual(list(self.validator.iter_errors(self.manifest)), [])

    def test_validated_must_be_boolean(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        manifest["validated"] = "true"
        self.assert_invalid(manifest, "not of type 'boolean'")

    def test_validation_level_is_controlled(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        manifest["validation_level"] = "maybe"
        self.assert_invalid(manifest, "is not one of")

    def test_sdk_version_has_expected_format(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        manifest["sdk"] = "SDK latest"
        self.assert_invalid(manifest, "does not match")

    def test_unknown_fields_are_rejected(self) -> None:
        manifest = copy.deepcopy(self.manifest)
        manifest["unexpected"] = True
        self.assert_invalid(manifest, "Additional properties are not allowed")


if __name__ == "__main__":
    unittest.main()
