#!/usr/bin/env python3
"""Validate repository metadata, packaged examples, and local Markdown links."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

from jsonschema import Draft202012Validator
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
EXAMPLES = ROOT / "examples"
BOARDS = ROOT / "boards"
SCHEMA_PATH = ROOT / "schemas" / "example-manifest.schema.json"
MARKDOWN_LINK_RE = re.compile(r"!?\[[^\]]*\]\(([^)]+)\)")
def fail(message: str) -> None:
    print(f"ERROR: {message}")


def schema_error_path(error: object) -> str:
    path = getattr(error, "absolute_path", ())
    return ".".join(str(part) for part in path) or "<root>"


def validate_boards() -> list[str]:
    errors: list[str] = []
    required = {"schema", "id", "name", "aliases", "device", "package", "identification", "description", "system_pins", "peripherals", "expansion_headers", "validation"}
    seen_ids: set[str] = set()
    severities = {"blocked", "confirm", "release"}
    files = sorted(BOARDS.glob("*.json"))
    if not files:
        return ["no boards/*.json files found"]
    for path in files:
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            errors.append(f"{path.relative_to(ROOT)}: invalid JSON: {exc}")
            continue
        missing = required - data.keys()
        if missing:
            errors.append(f"{path.relative_to(ROOT)}: missing fields: {', '.join(sorted(missing))}")
            continue
        board_id = data["id"]
        if not isinstance(board_id, str) or not re.fullmatch(r"[a-z0-9][a-z0-9-]*", board_id):
            errors.append(f"{path.relative_to(ROOT)}: id must be lowercase kebab-case")
        elif board_id in seen_ids:
            errors.append(f"{path.relative_to(ROOT)}: duplicate board id {board_id!r}")
        else:
            seen_ids.add(board_id)
        board_pins: set[str] = set()
        for group in ("system_pins", "peripherals"):
            if not isinstance(data[group], list):
                errors.append(f"{path.relative_to(ROOT)}: {group} must be a list")
                continue
            for item in data[group]:
                if not isinstance(item, dict):
                    errors.append(f"{path.relative_to(ROOT)}: each {group} item must be an object")
                    continue
                if group == "system_pins" and isinstance(item.get("pin"), str):
                    pins = [item["pin"]]
                elif group == "peripherals" and isinstance(item.get("pins"), list):
                    pins = item["pins"]
                else:
                    errors.append(f"{path.relative_to(ROOT)}: each {group} item must define pins")
                    continue
                if item.get("severity") not in severities:
                    errors.append(f"{path.relative_to(ROOT)}: invalid severity {item.get('severity')!r}")
                for pin in pins:
                    if not isinstance(pin, str) or not re.fullmatch(r"P[AB][0-9]+", pin):
                        errors.append(f"{path.relative_to(ROOT)}: invalid GPIO pin {pin!r}")
                    elif pin in board_pins:
                        errors.append(f"{path.relative_to(ROOT)}: duplicate pin entry {pin!r}")
                    else:
                        board_pins.add(pin)
    return errors


def validate_manifests() -> list[str]:
    errors: list[str] = []
    try:
        schema = json.loads(SCHEMA_PATH.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        return [f"{SCHEMA_PATH.relative_to(ROOT)}: cannot load schema: {exc}"]
    validator = Draft202012Validator(schema)
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

        for schema_error in sorted(validator.iter_errors(data), key=lambda item: list(item.absolute_path)):
            errors.append(
                f"{manifest_path.relative_to(ROOT)}: {schema_error_path(schema_error)}: {schema_error.message}"
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
    errors = validate_boards() + validate_manifests() + validate_markdown_links()
    if errors:
        for error in errors:
            fail(error)
        print(f"Repository validation failed with {len(errors)} error(s).")
        return 1
    print("Repository validation passed: manifests, example files, and local Markdown links.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
