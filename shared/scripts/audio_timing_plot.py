#!/usr/bin/env python3
"""Run the audio timing probe and save a diagnostic plot to docs/media."""

from __future__ import annotations

import argparse
import datetime as dt
import pathlib
import re
import subprocess
from dataclasses import dataclass

import matplotlib.pyplot as plt


ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_ENV = "frx_test_audio"


@dataclass
class ProcessTiming:
    count: int = 0
    last_us: int = 0
    min_us: int = 0
    max_us: int = 0
    runtime_max_us: int = 0
    read_count: int = 0
    read_last_us: int = 0
    read_max_us: int = 0
    callback_count: int = 0
    callback_last_us: int = 0
    callback_max_us: int = 0
    write_count: int = 0
    write_last_us: int = 0
    write_max_us: int = 0
    read_err: int = 0
    write_err: int = 0


@dataclass
class BankTiming:
    bank: int
    count: int
    rx: int
    tx: int
    last_us: int
    min_us: int
    max_us: int


def _macro_int(text: str, name: str, fallback: int | None = None) -> int | None:
    m = re.search(rf"#define\s+{re.escape(name)}\s+\(?([0-9]+)\)?", text)
    if m:
        return int(m.group(1))
    return fallback


def _load_audio_config() -> dict[str, int | None]:
    cfg_text = (ROOT / "lib/audio/audio_default_cfg.h").read_text()
    pio_text = (ROOT / "platformio.ini").read_text()

    def pio_define(name: str) -> int | None:
        m = re.search(rf"-D{re.escape(name)}=([0-9]+)", pio_text)
        return int(m.group(1)) if m else None

    pingpong = pio_define("AUDIO_PINGPONG_SAMPLES") or _macro_int(
        cfg_text, "AUDIO_PINGPONG_SAMPLES", 1024
    )
    block = pio_define("AUDIO_BLOCK_SAMPLES") or _macro_int(
        cfg_text, "AUDIO_BLOCK_SAMPLES", None
    )
    if block is None and pingpong is not None:
        block = pingpong // 2

    sr_default = _macro_int(cfg_text, "AUDIO_SR_DEFAULT", None)
    # AUDIO_SR_DEFAULT may point to AUDIO_SR_48000 rather than a literal.
    if sr_default is None:
        m = re.search(r"#define\s+AUDIO_SR_DEFAULT\s+(AUDIO_SR_[0-9]+)", cfg_text)
        if m:
            sr_default = _macro_int(cfg_text, m.group(1), None)

    return {
        "fs": pio_define("AUDIO_SR_DEFAULT") or sr_default,
        "bps": pio_define("AUDIO_BPS_DEFAULT") or _macro_int(cfg_text, "AUDIO_BPS_DEFAULT", 32),
        "pingpong_samples": pingpong,
        "block_samples": block,
    }


def _run_probe(env: str) -> str:
    proc = subprocess.run(
        ["pio", "test", "-e", env, "-v"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    return proc.stdout


def _platformio_with_block_overrides(text: str, env: str, block: int, pingpong_factor: int) -> str:
    pingpong = block * pingpong_factor
    lines = text.splitlines()
    out: list[str] = []
    in_env = False
    inserted = False
    env_header = f"[env:{env}]"
    for line in lines:
        if line.startswith("[env:"):
            if in_env and not inserted:
                out.append(f"    -DAUDIO_BLOCK_SAMPLES={block}")
                out.append(f"    -DAUDIO_PINGPONG_SAMPLES={pingpong}")
                inserted = True
            in_env = line.strip() == env_header
        if in_env and ("-DAUDIO_BLOCK_SAMPLES=" in line or "-DAUDIO_PINGPONG_SAMPLES=" in line):
            continue
        out.append(line)
        if in_env and line.strip() == "-DAUDIO_TIMING_ENABLE=1":
            out.append(f"    -DAUDIO_BLOCK_SAMPLES={block}")
            out.append(f"    -DAUDIO_PINGPONG_SAMPLES={pingpong}")
            inserted = True
    if in_env and not inserted:
        out.append(f"    -DAUDIO_BLOCK_SAMPLES={block}")
        out.append(f"    -DAUDIO_PINGPONG_SAMPLES={pingpong}")
    if not inserted:
        raise SystemExit(f"Could not find env {env!r} with AUDIO_TIMING_ENABLE=1 in platformio.ini")
    return "\n".join(out) + "\n"


def _run_block_sweep(env: str, blocks: list[int], pingpong_factor: int, clean: bool) -> int:
    pio = ROOT / "platformio.ini"
    original = pio.read_text()
    try:
        for block in blocks:
            pingpong = block * pingpong_factor
            pio.write_text(_platformio_with_block_overrides(original, env, block, pingpong_factor))
            if clean:
                subprocess.run(["pio", "run", "-e", env, "-t", "clean"], cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
            output = _run_probe(env)
            log_path = ROOT / "docs/media" / f"audio_timing_block_{block}.log"
            log_path.write_text(output)
            process, banks = _parse_timing(output)
            status = "PASSED" if "[PASSED]" in output else ("FAILED" if "[FAILED]" in output else "ERRORED")
            out = ROOT / "docs/media" / f"audio_timing_block_{block}.png"
            if process.count or banks:
                _plot(process, banks, _load_audio_config(), out)
                print(f"block={block} pingpong={pingpong} status={status} plot={out}")
            else:
                print(f"block={block} pingpong={pingpong} status={status} no AUDIO_TIMING lines")
        return 0
    finally:
        pio.write_text(original)


def _parse_timing(output: str) -> tuple[ProcessTiming, list[BankTiming]]:
    proc_re = re.compile(
        r"AUDIO_TIMING process count=(\d+) delta_us last=(\d+) min=(\d+) max=(\d+) "
        r"runtime_max=(\d+) read_count=(\d+) read_last=(\d+) read_max=(\d+) "
        r"cb_count=(\d+) cb_last=(\d+) cb_max=(\d+) "
        r"write_count=(\d+) write_last=(\d+) write_max=(\d+) "
        r"read_err=(\d+) write_err=(\d+)"
    )
    bank_re = re.compile(
        r"AUDIO_TIMING bank=(\d+) count=(\d+) rx=(\d+) tx=(\d+) "
        r"delta_us last=(\d+) min=(\d+) max=(\d+)"
    )

    p = ProcessTiming()
    banks: list[BankTiming] = []
    for line in output.splitlines():
        if m := proc_re.search(line):
            p = ProcessTiming(*(int(x) for x in m.groups()))
        elif m := bank_re.search(line):
            banks.append(BankTiming(*(int(x) for x in m.groups())))
    return p, banks


def _plot(
    process: ProcessTiming,
    banks: list[BankTiming],
    cfg: dict[str, int | None],
    output_path: pathlib.Path,
) -> None:
    fs = cfg["fs"]
    block = cfg["block_samples"]
    expected_us = (block / fs * 1_000_000) if fs and block else None
    if process.min_us > 1_000_000_000:
        process.min_us = 0
    if process.max_us == 0 and process.count == 0:
        process.runtime_max_us = 0
    for b in banks:
        if b.min_us > 1_000_000_000:
            b.min_us = 0

    fig, axes = plt.subplots(2, 1, figsize=(12, 10))
    fig.subplots_adjust(left=0.08, right=0.98, top=0.90, bottom=0.30, hspace=0.45)
    fig.suptitle("libFRX Audio Timing Probe", fontsize=15)

    effective_overhead_us = process.callback_max_us + process.write_max_us
    effective_overhead_pct = (effective_overhead_us / expected_us * 100.0) if expected_us else None

    labels = [
        "block period",
        "service span\n(includes idle)",
        "read wait\n(mostly idle)",
        "callback CPU",
        "write/pack",
        "effective\noverhead",
    ]
    vals = [
        process.max_us,
        process.runtime_max_us,
        process.read_max_us,
        process.callback_max_us,
        process.write_max_us,
        effective_overhead_us,
    ]
    colors = ["#4c78a8", "#9ecae9", "#d9d9d9", "#f58518", "#54a24b", "#e45756"]
    axes[0].bar(labels, vals, color=colors)
    if expected_us:
        axes[0].axhline(expected_us, linestyle="--", label=f"expected block {expected_us:.1f} us")
        axes[0].legend()
    axes[0].set_ylabel("microseconds")
    axes[0].set_title("Effective overhead; idle-blocking read time is not counted as consuming overhead")
    axes[0].grid(axis="y", alpha=0.3)

    bank_labels: list[str] = []
    bank_vals: list[int] = []
    for b in banks:
        bank_labels.extend([f"b{b.bank} last", f"b{b.bank} min", f"b{b.bank} max"])
        bank_vals.extend([b.last_us, b.min_us, b.max_us])
    if bank_vals:
        axes[1].bar(bank_labels, bank_vals)
    if expected_us:
        axes[1].axhline(expected_us, linestyle="--", label=f"expected RX/TX pair cadence {expected_us:.1f} us")
        axes[1].legend()
    axes[1].set_ylabel("microseconds")
    axes[1].set_title("I2S event intervals")
    axes[1].grid(axis="y", alpha=0.3)

    stamp = dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    bank_summary = "\n".join(
        f"bank {b.bank}: events={b.count}, rx={b.rx}, tx={b.tx}, last/min/max={b.last_us}/{b.min_us}/{b.max_us} us"
        for b in banks
    ) or "no bank timing captured"
    text = (
        f"Date: {stamp}\n"
        f"fs={cfg['fs']} Hz, bps={cfg['bps']}, "
        f"pingpong={cfg['pingpong_samples']} samples, block={cfg['block_samples']} samples\n"
        f"process: count={process.count}, last/min/max={process.last_us}/{process.min_us}/{process.max_us} us, "
        f"runtime_max={process.runtime_max_us} us\n"
        f"effective overhead: {effective_overhead_us} us"
        f"{f' ({effective_overhead_pct:.2f}% of block)' if effective_overhead_pct is not None else ''}; "
        f"read wait is shown but excluded because idle-blocking is not consuming overhead\n"
        f"breakdown: read last/max={process.read_last_us}/{process.read_max_us} us, "
        f"callback last/max={process.callback_last_us}/{process.callback_max_us} us, "
        f"write last/max={process.write_last_us}/{process.write_max_us} us, "
        f"read_err={process.read_err}, write_err={process.write_err}\n"
        f"{bank_summary}"
    )
    fig.text(0.02, 0.03, text, fontsize=9, family="monospace", va="bottom")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--env", default=DEFAULT_ENV)
    parser.add_argument("--input", type=pathlib.Path, help="Parse existing pio output instead of running test")
    parser.add_argument("--output", type=pathlib.Path, help="Output PNG path")
    parser.add_argument("--blocks", nargs="+", type=int, help="Run a sweep for these AUDIO_BLOCK_SAMPLES values")
    parser.add_argument("--pingpong-factor", type=int, default=2, help="AUDIO_PINGPONG_SAMPLES = block * factor")
    parser.add_argument("--no-clean", action="store_true", help="Do not clean PlatformIO build between block sizes")
    args = parser.parse_args()

    if args.blocks:
        if args.input or args.output:
            raise SystemExit("--blocks cannot be combined with --input or --output")
        return _run_block_sweep(args.env, args.blocks, args.pingpong_factor, not args.no_clean)

    output = args.input.read_text() if args.input else _run_probe(args.env)
    process, banks = _parse_timing(output)
    if process.count == 0 and not banks:
        raise SystemExit("No AUDIO_TIMING lines found. Enable AUDIO_TIMING_ENABLE and run test_audio.")

    stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    out = args.output or (ROOT / "docs/media" / f"audio_timing_{stamp}.png")
    _plot(process, banks, _load_audio_config(), out)
    print(out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
