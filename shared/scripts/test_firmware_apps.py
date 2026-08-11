#!/usr/bin/env python3
"""Build, upload, and run all PlatformIO unit-test firmware apps."""

from __future__ import annotations

import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]

if __name__ == "__main__":
    raise SystemExit(
        subprocess.call(
            [sys.executable, str(ROOT / "shared" / "scripts" / "run_pio_env_suite.py"), "--suite", "test-tests", *sys.argv[1:]],
            cwd=ROOT,
        )
    )
