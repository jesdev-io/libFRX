#!/usr/bin/env python3
"""Build/flash/test PlatformIO environments for this repository.

The script discovers `[env:*]` sections from platformio.ini so new demo/test
environments are picked up automatically. Output is streamed live and written to
per-step logs under `.pio/build-all-logs/`.
"""

from __future__ import annotations

import argparse
import configparser
import datetime as dt
import fnmatch
import pathlib
import subprocess
import sys
import time
from dataclasses import dataclass
from enum import Enum

ROOT = pathlib.Path(__file__).resolve().parents[2]
PIO_INI = ROOT / "platformio.ini"
LOG_ROOT = ROOT / ".pio" / "build-all-logs"

# Runtime tests that disturb shared hardware state and must be requested explicitly.
# frx_test_sdcard_spi can leave the single SD-card socket in SPI/card state;
# power-cycle or reinsert the card before running SDMMC tests afterwards.
MANUAL_RUNTIME_TEST_ENVS = {"frx_test_sdcard_spi"}


class EnvKind(str, Enum):
    DEFAULT = "default"
    DEMO = "demo"
    TEST = "test"


@dataclass(frozen=True)
class EnvInfo:
    name: str
    section: configparser.SectionProxy
    kind: EnvKind


@dataclass(frozen=True)
class Step:
    label: str
    command: list[str]
    log: pathlib.Path
    settle_after_s: float = 0.0


@dataclass(frozen=True)
class StepResult:
    step: Step
    returncode: int
    seconds: float

    @property
    def ok(self) -> bool:
        return self.returncode == 0


def _load_envs() -> list[EnvInfo]:
    parser = configparser.ConfigParser(interpolation=None)
    parser.optionxform = str
    with PIO_INI.open() as f:
        parser.read_file(f)

    envs: list[EnvInfo] = []
    for section_name in parser.sections():
        if not section_name.startswith("env:"):
            continue
        name = section_name.removeprefix("env:")
        section = parser[section_name]
        if "test_filter" in section:
            kind = EnvKind.TEST
        elif "_demo_" in name:
            kind = EnvKind.DEMO
        else:
            kind = EnvKind.DEFAULT
        envs.append(EnvInfo(name=name, section=section, kind=kind))
    return envs


def _matches_any(name: str, patterns: list[str]) -> bool:
    return not patterns or any(fnmatch.fnmatch(name, pattern) for pattern in patterns)


def _demo_cli_test(env_name: str) -> pathlib.Path | None:
    stem = env_name.split("_demo_", 1)[1] if "_demo_" in env_name else env_name.removeprefix("frx_demo_")
    candidate = ROOT / "tests" / f"test_{stem}_cli.py"
    return candidate if candidate.exists() else None


def _log_dir() -> pathlib.Path:
    stamp = dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    log_dir = LOG_ROOT / stamp
    log_dir.mkdir(parents=True, exist_ok=True)
    return log_dir


def _slug(label: str) -> str:
    return "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in label)


def _build_steps(suite: str, patterns: list[str], settle_after_upload_s: float) -> list[Step]:
    log_dir = _log_dir()
    envs = [env for env in _load_envs() if _matches_any(env.name, patterns)]
    steps: list[Step] = []

    def add(label: str, command: list[str], settle_after_s: float = 0.0) -> None:
        steps.append(Step(label=label, command=command, log=log_dir / f"{_slug(label)}.log", settle_after_s=settle_after_s))

    def add_default_builds() -> None:
        for env in envs:
            if env.kind == EnvKind.DEFAULT:
                add(f"{env.name}:build", ["pio", "run", "-e", env.name])

    def add_demo_builds() -> None:
        for env in envs:
            if env.kind == EnvKind.DEMO:
                add(f"{env.name}:build", ["pio", "run", "-e", env.name])

    def add_demo_flash_cli() -> None:
        for env in envs:
            if env.kind != EnvKind.DEMO:
                continue
            cli_test = _demo_cli_test(env.name)
            add(f"{env.name}:build", ["pio", "run", "-e", env.name])
            add(f"{env.name}:flash", ["pio", "run", "-e", env.name, "-t", "upload"], settle_after_upload_s)
            if cli_test is not None:
                add(f"{env.name}:cli", [sys.executable, "-m", "pytest", str(cli_test.relative_to(ROOT)), "-v"])
            else:
                add(f"{env.name}:cli-missing", [sys.executable, "-c", f"raise SystemExit('missing CLI test for {env.name}')"])

    def add_test_builds() -> None:
        for env in envs:
            if env.kind == EnvKind.TEST:
                add(f"{env.name}:build", ["pio", "test", "-e", env.name, "--without-uploading", "--without-testing"])

    def add_test_runs() -> None:
        explicit_patterns = bool(patterns)
        for env in envs:
            if env.kind != EnvKind.TEST:
                continue
            if env.name in MANUAL_RUNTIME_TEST_ENVS and not explicit_patterns:
                continue
            add(f"{env.name}:test", ["pio", "test", "-e", env.name])

    if suite == "all":
        add_default_builds()
        add_demo_builds()
        add_demo_flash_cli()
        add_test_builds()
        add_test_runs()
    elif suite == "build-demos":
        add_demo_builds()
    elif suite == "flash-cli-demos":
        add_demo_flash_cli()
    elif suite == "build-tests":
        add_test_builds()
    elif suite == "test-tests":
        add_test_runs()
    elif suite == "build-defaults":
        add_default_builds()
    else:
        raise ValueError(f"unknown suite: {suite}")

    return steps


def _run_one(step: Step) -> StepResult:
    print(f"\n=== {step.label} ===")
    print("$ " + " ".join(step.command))
    print(f"log: {step.log.relative_to(ROOT)}")

    start = time.monotonic()
    with step.log.open("w", encoding="utf-8", errors="replace") as log:
        proc = subprocess.Popen(
            step.command,
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )
        assert proc.stdout is not None
        for line in proc.stdout:
            print(line, end="")
            log.write(line)
        returncode = proc.wait()

    if returncode == 0 and step.settle_after_s > 0:
        print(f"settling for {step.settle_after_s:.1f}s after upload...")
        time.sleep(step.settle_after_s)

    seconds = time.monotonic() - start
    status = "PASS" if returncode == 0 else "FAIL"
    print(f"=== {step.label}: {status} in {seconds:.1f}s ===")
    return StepResult(step=step, returncode=returncode, seconds=seconds)


def _print_summary(results: list[StepResult]) -> None:
    print("\n=== build/test summary ===")
    width = max((len(r.step.label) for r in results), default=3)
    for result in results:
        status = "PASS" if result.ok else f"FAIL({result.returncode})"
        print(f"{result.step.label:<{width}}  {status:<8}  {result.seconds:6.1f}s  {result.step.log.relative_to(ROOT)}")

    failed = [r for r in results if not r.ok]
    print(f"\npassed: {len(results) - len(failed)} / {len(results)}")
    if failed:
        print("failed steps: " + ", ".join(r.step.label for r in failed))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "patterns",
        nargs="*",
        help="Optional env name globs, e.g. 'frx_test_*' or 'frx_demo_*'. Default: envs selected by suite.",
    )
    parser.add_argument(
        "--suite",
        choices=["all", "build-demos", "flash-cli-demos", "build-tests", "test-tests", "build-defaults"],
        default="all",
        help="Workload to run. Default: all.",
    )
    parser.add_argument("--list", action="store_true", help="List discovered commands without running them.")
    parser.add_argument("--fail-fast", action="store_true", help="Stop at the first failing step.")
    parser.add_argument("--settle-after-upload", type=float, default=2.0, help="Seconds to wait after flashing before CLI tests.")
    args = parser.parse_args()

    steps = _build_steps(args.suite, args.patterns, args.settle_after_upload)
    if not steps:
        print("No matching steps found", file=sys.stderr)
        return 2

    if args.list:
        for step in steps:
            print(f"{step.label}: {' '.join(step.command)}")
        return 0

    results: list[StepResult] = []
    for step in steps:
        result = _run_one(step)
        results.append(result)
        if args.fail_fast and not result.ok:
            break

    _print_summary(results)
    return 1 if any(not r.ok for r in results) else 0


if __name__ == "__main__":
    raise SystemExit(main())
