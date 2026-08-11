#!/usr/bin/env python3
"""Build, flash, and CLI-test all demo PlatformIO apps."""

from __future__ import annotations

import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]

if __name__ == "__main__":
    raise SystemExit(
        subprocess.call(
            [sys.executable, str(ROOT / "shared" / "scripts" / "run_pio_env_suite.py"), "--suite", "flash-cli-demos", *sys.argv[1:]],
            cwd=ROOT,
        )
    )
