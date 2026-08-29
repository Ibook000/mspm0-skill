#!/usr/bin/env python3
"""Validate repository metadata, packaged examples, and local Markdown links."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
EXAMPLES = ROOT / "examples"
MARKDOWN_LINK_RE = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
REQUIRED_MANIFEST_FIELDS = {
    "schema",
    "name",
    "title",
    "description",
    "board",
    "device",
    "package",
    "validated",
    "validation_level",
    "complexity",
    "peripherals",
    "pins",
    "source_files",
    "syscfg",
}


def fail(message: str) -> None:
    print(f"ERROR: {message}")


def validate_manifests() -> list[str]:
    errors: list[str] = []
    manifests = sorted(EXAMPLES.glob("*/manifest.json"))
    if not manifests:
        return ["no examples/*/manifest.json files found"]

    names: set[str] = set()
    for manifest_path in manifests:
        example_dir = manifest_path.parent
        try:
            data = json.loads(manifest_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"{manifest_path.relative_to(ROOT)}: invalid JSON: {exc}")
            continue

        missing = REQUIRED_MANIFEST_FIELDS - data.keys()
        if missing:
            errors.append(
                f"{manifest_path.relative_to(ROOT)}: missing fields: {', '.join(sorted(missing))}"
            )
        for legacy_key in ("product", "sysconfig_versions"):
            if legacy_key in data:
                errors.append(
                    f"{manifest_path.relative_to(ROOT)}: legacy field {legacy_key!r}; use the canonical field instead"
                )
        for canonical_key in ("sdk", "sysconfig"):
            if canonical_key not in data:
                errors.append(
                    f"{manifest_path.relative_to(ROOT)}: missing canonical field {canonical_key!r}"
                )
        name = data.get("name")
        if not isinstance(name, str) or not name.strip():
            errors.append(f"{manifest_path.relative_to(ROOT)}: name must be a non-empty string")
        elif name in names:
            errors.append(f"{manifest_path.relative_to(ROOT)}: duplicate example name {name!r}")
        else:
            names.add(name)

        syscfg = data.get("syscfg")
        if isinstance(syscfg, str) and not (example_dir / syscfg).is_file():
            errors.append(f"{manifest_path.relative_to(ROOT)}: missing syscfg file {syscfg!r}")

        source_files = data.get("source_files", [])
        if not isinstance(source_files, list):
            errors.append(f"{manifest_path.relative_to(ROOT)}: source_files must be a list")
        else:
            for source_file in source_files:
                if not isinstance(source_file, str) or not (example_dir / source_file).is_file():
                    errors.append(
                        f"{manifest_path.relative_to(ROOT)}: missing source file {source_file!r}"
                    )

        if not (example_dir / "README.md").is_file():
            errors.append(f"{manifest_path.relative_to(ROOT)}: missing README.md")

    return errors


def validate_markdown_links() -> list[str]:
    errors: list[str] = []
    for markdown_path in sorted(ROOT.rglob("*.md")):
        if ".git" in markdown_path.parts:
            continue
        try:
            text = markdown_path.read_text(encoding="utf-8")
        except OSError as exc:
            errors.append(f"{markdown_path.relative_to(ROOT)}: cannot read file: {exc}")
            continue
        for raw_target in MARKDOWN_LINK_RE.findall(text):
            target = raw_target.strip().strip("<>").split()[0]
            parsed = urlsplit(target)
            if parsed.scheme or target.startswith("//") or target.startswith("#"):
                continue
            relative_target = unquote(parsed.path)
            if not relative_target:
                continue
            resolved = (markdown_path.parent / relative_target).resolve()
            try:
                resolved.relative_to(ROOT.resolve())
            except ValueError:
                errors.append(
                    f"{markdown_path.relative_to(ROOT)}: link escapes repository: {target!r}"
                )
                continue
            if not resolved.exists():
                errors.append(f"{markdown_path.relative_to(ROOT)}: missing link target {target!r}")
    return errors


def main() -> int:
    errors = validate_manifests() + validate_markdown_links()
    if errors:
        for error in errors:
            fail(error)
        print(f"Repository validation failed with {len(errors)} error(s).")
        return 1
    print("Repository validation passed: manifests, example files, and local Markdown links.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
