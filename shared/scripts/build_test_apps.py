#!/usr/bin/env python3
"""Compile all PlatformIO test apps without uploading/running them."""

from __future__ import annotations

import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]

if __name__ == "__main__":
    raise SystemExit(
        subprocess.call(
            [sys.executable, str(ROOT / "shared" / "scripts" / "run_pio_env_suite.py"), "--suite", "build-tests", *sys.argv[1:]],
            cwd=ROOT,
        )
    )
