#!/usr/bin/env python3
"""Report verification stages for an MSPM0 project or packaged example."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from check_syscfg import EXIT_CHECK_FAILED, check_project

BUILD_NAMES = {"Debug", "Release", "Objects", "Listings", "build", "out"}
OUTPUT_SUFFIXES = {".out", ".axf", ".elf", ".hex", ".bin"}


def find_outputs(root: Path) -> list[str]:
    return sorted(
        path.relative_to(root).as_posix()
        for path in root.rglob("*")
        if path.is_file() and (path.suffix.lower() in OUTPUT_SUFFIXES or any(part in BUILD_NAMES for part in path.parts))
    )


def verify(root: Path, board: str | None, snapshot: bool) -> dict[str, Any]:
    messages, details = check_project(root, board_id=board, snapshot=snapshot)
    errors = [message.text for message in messages if message.level == "error"]
    warnings = [message.text for message in messages if message.level == "warning"]
    outputs = find_outputs(root)
    return {
        "project": str(root),
        "static": {"status": "failed" if errors else "passed", "errors": errors, "warnings": warnings},
        "sysconfig": {"status": "passed" if details.get("syscfg_files") else "failed"},
        "build": {"status": "detected" if outputs else "not_detected", "files": outputs[:50]},
        "flash": {"status": "ready" if any(Path(item).suffix.lower() in OUTPUT_SUFFIXES for item in outputs) else "not_ready"},
        "hardware": {"status": "manual", "note": "真实硬件验证需要用户提供板卡、电源、探针和外设结果。"},
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project", type=Path, help="Project or packaged example directory.")
    parser.add_argument("--board", help="Board database id for board-specific pin checks.")
    parser.add_argument("--snapshot", action="store_true", help="Use packaged source-snapshot warning policy.")
    parser.add_argument("--json", action="store_true", help="Print machine-readable JSON.")
    args = parser.parse_args(argv)
    root = args.project.resolve()
    if not root.is_dir():
        parser.error(f"project directory not found: {root}")
    report = verify(root, args.board, args.snapshot)
    if args.json:
        print(json.dumps(report, ensure_ascii=False, indent=2))
    else:
        print(f"MSPM0 verification: {root}")
        for stage, result in report.items():
            if stage == "project":
                continue
            print(f"{stage:10} {result['status']}")
            if result.get("errors"):
                for error in result["errors"]:
                    print(f"  ERROR: {error}")
    return EXIT_CHECK_FAILED if report["static"]["status"] == "failed" else 0


if __name__ == "__main__":
    raise SystemExit(main())
