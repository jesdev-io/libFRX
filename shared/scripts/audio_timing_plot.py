#!/usr/bin/env python3
"""Run/parse libFRX audio timing probes and draw an audio-stack timing budget.

The plot answers three questions:
1. When does the sampler run relative to the expected audio block period?
2. How does the worst observed processing span stack up?
3. How much timing headroom remains for user callbacks / DSP?
"""

from __future__ import annotations

import argparse
import datetime as dt
import pathlib
import re
import subprocess
from dataclasses import dataclass

import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec


ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_ENV = "frx_test_audio"
DEFAULT_TIMEOUT_S = 180


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
    internal_callback_count: int = 0
    internal_callback_last_us: int = 0
    internal_callback_max_us: int = 0
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


@dataclass
class FftBench:
    backend: str = ""
    n: int = 0
    callback_max_us: int = 0
    process_max_us: int = 0
    block_us: int = 0
    energy_milli: int = 0


@dataclass
class ParsedTiming:
    process: ProcessTiming
    banks: list[BankTiming]
    fft: FftBench | None


def _macro_int(text: str, name: str, fallback: int | None = None) -> int | None:
    m = re.search(rf"#define\s+{re.escape(name)}\s+\(?([0-9]+)\)?", text)
    return int(m.group(1)) if m else fallback


def _load_audio_config() -> dict[str, int | None]:
    cfg_text = (ROOT / "lib/audio/audio_default_cfg.h").read_text()
    pio_text = (ROOT / "platformio.ini").read_text()

    def pio_define(name: str) -> int | None:
        m = re.search(rf"-D{re.escape(name)}=([0-9]+)", pio_text)
        return int(m.group(1)) if m else None

    pingpong = pio_define("AUDIO_PINGPONG_SAMPLES") or _macro_int(cfg_text, "AUDIO_PINGPONG_SAMPLES", 1024)
    block = pio_define("AUDIO_BLOCK_SAMPLES") or _macro_int(cfg_text, "AUDIO_BLOCK_SAMPLES", None)
    if block is None and pingpong is not None:
        block = pingpong // 2

    sr_default = _macro_int(cfg_text, "AUDIO_SR_DEFAULT", None)
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


def _block_period_us(cfg: dict[str, int | None], parsed: ParsedTiming | None = None) -> float | None:
    if parsed and parsed.fft and parsed.fft.block_us:
        return float(parsed.fft.block_us)
    fs = cfg["fs"]
    block = cfg["block_samples"]
    return (block / fs * 1_000_000.0) if fs and block else None


def _run_probe(env: str, timeout_s: int) -> str:
    proc = subprocess.run(
        ["pio", "test", "-e", env, "-v"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
        timeout=timeout_s,
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
        if in_env and "-DAUDIO_TIMING_ENABLE" in line:
            out.append(f"    -DAUDIO_BLOCK_SAMPLES={block}")
            out.append(f"    -DAUDIO_PINGPONG_SAMPLES={pingpong}")
            inserted = True
    if in_env and not inserted:
        out.append(f"    -DAUDIO_BLOCK_SAMPLES={block}")
        out.append(f"    -DAUDIO_PINGPONG_SAMPLES={pingpong}")
    if not inserted:
        raise SystemExit(f"Could not find env {env!r} in platformio.ini")
    return "\n".join(out) + "\n"


def _run_block_sweep(env: str, blocks: list[int], pingpong_factor: int, clean: bool, timeout_s: int) -> int:
    pio = ROOT / "platformio.ini"
    original = pio.read_text()
    try:
        for block in blocks:
            pingpong = block * pingpong_factor
            pio.write_text(_platformio_with_block_overrides(original, env, block, pingpong_factor))
            if clean:
                subprocess.run(["pio", "run", "-e", env, "-t", "clean"], cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
            output = _run_probe(env, timeout_s)
            log_path = ROOT / "docs/media" / f"audio_timing_block_{block}.log"
            log_path.write_text(output)
            parsed = _parse_timing(output)
            status = "PASSED" if "[PASSED]" in output else ("FAILED" if "[FAILED]" in output else "ERRORED")
            out = ROOT / "docs/media" / f"audio_timing_block_{block}.png"
            if parsed.process.count or parsed.banks:
                _plot(parsed, _load_audio_config(), out)
                print(_summary(parsed, _load_audio_config(), out, status=f"block={block} pingpong={pingpong} {status}"))
            else:
                print(f"block={block} pingpong={pingpong} status={status} no AUDIO_TIMING lines")
        return 0
    finally:
        pio.write_text(original)


def _parse_timing(output: str) -> ParsedTiming:
    proc_short_re = re.compile(r"AUDIO_TIMING proc c=(\d+) l=(\d+) n=(\d+) x=(\d+) r=(\d+)")
    read_short_re = re.compile(r"AUDIO_TIMING read c=(\d+) l=(\d+) x=(\d+) e=(\d+)")
    cb_short_re = re.compile(r"AUDIO_TIMING cb c=(\d+) l=(\d+) x=(\d+)")
    intcb_short_re = re.compile(r"AUDIO_TIMING intcb c=(\d+) l=(\d+) x=(\d+)")
    write_short_re = re.compile(r"AUDIO_TIMING write c=(\d+) l=(\d+) x=(\d+) e=(\d+)")
    bank_short_re = re.compile(r"AUDIO_TIMING bank=(\d+) c=(\d+) rx=(\d+) tx=(\d+) l=(\d+) n=(\d+) x=(\d+)")
    fft_re = re.compile(
        r"AUDIO_FFT_BENCH\s+backend=([^\s]+)\s+n=(\d+)\s+cb_max=(\d+)\s+"
        r"process_max=(\d+)\s+block_us=(\d+)\s+energy=(-?\d+)"
    )
    fft_short_re = re.compile(
        r"AUDIO_FFT_BENCH\s+b=([^\s]+)\s+n=(\d+)\s+c=(\d+)\s+p=(\d+)\s+u=(\d+)\s+e=(-?\d+)"
    )

    p = ProcessTiming()
    banks: list[BankTiming] = []
    fft: FftBench | None = None
    for line in output.splitlines():
        if m := proc_short_re.search(line):
            p.count, p.last_us, p.min_us, p.max_us, p.runtime_max_us = (int(x) for x in m.groups())
        elif m := read_short_re.search(line):
            p.read_count, p.read_last_us, p.read_max_us, p.read_err = (int(x) for x in m.groups())
        elif m := cb_short_re.search(line):
            p.callback_count, p.callback_last_us, p.callback_max_us = (int(x) for x in m.groups())
        elif m := intcb_short_re.search(line):
            p.internal_callback_count, p.internal_callback_last_us, p.internal_callback_max_us = (int(x) for x in m.groups())
        elif m := write_short_re.search(line):
            p.write_count, p.write_last_us, p.write_max_us, p.write_err = (int(x) for x in m.groups())
        elif m := bank_short_re.search(line):
            banks.append(BankTiming(*(int(x) for x in m.groups())))
        elif m := fft_re.search(line):
            backend, n, cb_max, proc_max, block_us, energy = m.groups()
            fft = FftBench(backend, int(n), int(cb_max), int(proc_max), int(block_us), int(energy))
        elif m := fft_short_re.search(line):
            backend, n, cb_max, proc_max, block_us, energy = m.groups()
            fft = FftBench(backend, int(n), int(cb_max), int(proc_max), int(block_us), int(energy))
    return ParsedTiming(p, banks, fft)


def _pct(v: float, total: float | None) -> str:
    return f"{(100.0 * v / total):.1f}%" if total else "n/a"


def _summary(parsed: ParsedTiming, cfg: dict[str, int | None], output_path: pathlib.Path | None, status: str = "") -> str:
    p = parsed.process
    block_us = _block_period_us(cfg, parsed)
    measured = float(p.runtime_max_us)
    stack = p.read_max_us + p.callback_max_us + p.internal_callback_max_us + p.write_max_us
    other = max(0, p.runtime_max_us - stack)
    headroom = max(0.0, (block_us or 0.0) - measured) if block_us else 0.0
    lines = []
    if status:
        lines.append(status)
    lines.append("Audio timing summary")
    lines.append(f"  block period:      {block_us:.1f} us" if block_us else "  block period:      unknown")
    lines.append(f"  process cadence:   last/min/max = {p.last_us}/{p.min_us}/{p.max_us} us")
    lines.append(f"  process max span:  {p.runtime_max_us} us ({_pct(measured, block_us)} of block)")
    lines.append(f"  stack max:         read {p.read_max_us} + callback {p.callback_max_us} + internal {p.internal_callback_max_us} + write {p.write_max_us} + other {other} = {p.runtime_max_us} us")
    lines.append(f"  headroom:          {headroom:.1f} us ({_pct(headroom, block_us)})")
    lines.append(f"  errors:            read={p.read_err}, write={p.write_err}")
    if parsed.fft:
        lines.append(f"  FFT bench:         {parsed.fft.backend}, N={parsed.fft.n}, callback max={parsed.fft.callback_max_us} us, process max={parsed.fft.process_max_us} us")
    if output_path:
        lines.append(f"  plot:              {output_path}")
    return "\n".join(lines)


def _plot(parsed: ParsedTiming, cfg: dict[str, int | None], output_path: pathlib.Path) -> None:
    p = parsed.process
    block_us = _block_period_us(cfg, parsed)
    if p.min_us > 1_000_000_000:
        p.min_us = 0
    for b in parsed.banks:
        if b.min_us > 1_000_000_000:
            b.min_us = 0

    read = p.read_max_us
    callback = p.callback_max_us
    internal = p.internal_callback_max_us
    write = p.write_max_us
    known_stack = read + callback + internal + write
    other = max(0, p.runtime_max_us - known_stack)
    headroom = max(0.0, (block_us or 0.0) - p.runtime_max_us) if block_us else 0.0

    fig = plt.figure(figsize=(15, 10), constrained_layout=True)
    gs = GridSpec(3, 2, figure=fig, height_ratios=[1.25, 1.0, 1.0])
    fig.suptitle("libFRX audio timing budget", fontsize=16, weight="bold")

    ax_budget = fig.add_subplot(gs[0, :])
    segments = [
        ("read + unpack", read, "#4c78a8"),
        ("user callback", callback, "#f58518"),
        ("internal gain", internal, "#b279a2"),
        ("pack + write", write, "#54a24b"),
        ("scheduler/queue/other", other, "#bab0ab"),
        ("headroom", headroom, "#59a14f"),
    ]
    left = 0.0
    for label, value, color in segments:
        if value <= 0:
            continue
        ax_budget.barh([0], [value], left=left, color=color, edgecolor="white", label=label)
        if value > max((block_us or p.runtime_max_us) * 0.035, 40):
            ax_budget.text(left + value / 2, 0, f"{label}\n{value:.0f} us", ha="center", va="center", fontsize=9)
        left += value
    if block_us:
        ax_budget.axvline(block_us, color="black", linestyle="--", linewidth=1.2, label=f"block deadline {block_us:.0f} us")
        ax_budget.set_xlim(0, max(block_us * 1.05, left * 1.05))
    else:
        ax_budget.set_xlim(0, left * 1.10)
    ax_budget.set_yticks([])
    ax_budget.set_xlabel("microseconds")
    ax_budget.set_title("Worst observed critical path: everything left of the deadline must finish before next audio block")
    ax_budget.grid(axis="x", alpha=0.25)
    ax_budget.legend(ncols=3, loc="upper right")

    ax_stack = fig.add_subplot(gs[1, 0])
    labels = ["read\nmax", "callback\nmax", "internal\nmax", "write\nmax", "other", "process\nmax", "headroom"]
    vals = [read, callback, internal, write, other, p.runtime_max_us, headroom]
    colors = ["#4c78a8", "#f58518", "#b279a2", "#54a24b", "#bab0ab", "#e45756", "#59a14f"]
    ax_stack.bar(labels, vals, color=colors)
    if block_us:
        ax_stack.axhline(block_us, color="black", linestyle="--", label=f"block {block_us:.0f} us")
        ax_stack.legend()
    ax_stack.set_ylabel("microseconds")
    ax_stack.set_title("Max timings by role")
    ax_stack.grid(axis="y", alpha=0.25)

    ax_cadence = fig.add_subplot(gs[1, 1])
    cadence_labels = ["process\nmin", "process\nlast", "process\nmax"]
    cadence_vals = [p.min_us, p.last_us, p.max_us]
    for b in parsed.banks:
        cadence_labels += [f"bank {b.bank}\nmin", f"bank {b.bank}\nlast", f"bank {b.bank}\nmax"]
        cadence_vals += [b.min_us, b.last_us, b.max_us]
    ax_cadence.bar(cadence_labels, cadence_vals, color="#9ecae9")
    if block_us:
        ax_cadence.axhline(block_us, color="black", linestyle="--", label=f"expected block {block_us:.0f} us")
        ax_cadence.legend()
    ax_cadence.set_ylabel("microseconds")
    ax_cadence.set_title("Cadence / jitter. Bank min may be RX↔TX pair spacing; bank max is block-to-block.")
    ax_cadence.grid(axis="y", alpha=0.25)

    ax_text = fig.add_subplot(gs[2, :])
    ax_text.axis("off")
    overhead_pct = _pct(p.runtime_max_us, block_us)
    headroom_pct = _pct(headroom, block_us)
    fft_line = "none captured"
    if parsed.fft:
        fft_line = (
            f"{parsed.fft.backend}, N={parsed.fft.n}, "
            f"callback max={parsed.fft.callback_max_us} us, process max={parsed.fft.process_max_us} us"
        )
    bank_lines = "; ".join(
        f"bank {b.bank}: events={b.count}, rx={b.rx}, tx={b.tx}, interval last/min/max={b.last_us}/{b.min_us}/{b.max_us} us"
        for b in parsed.banks
    ) or "no bank timing captured"
    text = (
        f"Generated: {dt.datetime.now():%Y-%m-%d %H:%M:%S}\n"
        f"Configuration: fs={cfg['fs']} Hz, bps={cfg['bps']}, block={cfg['block_samples']} samples, pingpong={cfg['pingpong_samples']} samples\n"
        f"Definitions: overhead = max process span from ready queues through read/unpack → user callback → internal gain → pack/write. "
        f"headroom = block period - max process span.\n"
        f"Observed: process_count={p.count}, process last/min/max={p.last_us}/{p.min_us}/{p.max_us} us, "
        f"max span={p.runtime_max_us} us ({overhead_pct}), headroom={headroom:.0f} us ({headroom_pct}).\n"
        f"Stack: read={read} us, callback={callback} us, internal={internal} us, write={write} us, other={other} us; errors read={p.read_err}, write={p.write_err}.\n"
        f"FFT benchmark: {fft_line}\n"
        f"I2S events: {bank_lines}"
    )
    ax_text.text(0.01, 0.98, text, va="top", ha="left", family="monospace", fontsize=10)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output_path, dpi=150)
    plt.close(fig)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--env", default=DEFAULT_ENV)
    parser.add_argument("--input", type=pathlib.Path, help="Parse existing pio output instead of running test")
    parser.add_argument("--output", type=pathlib.Path, help="Output PNG path")
    parser.add_argument("--blocks", nargs="+", type=int, help="Run a sweep for these AUDIO_BLOCK_SAMPLES values")
    parser.add_argument("--pingpong-factor", type=int, default=2, help="AUDIO_PINGPONG_SAMPLES = block * factor")
    parser.add_argument("--no-clean", action="store_true", help="Do not clean PlatformIO build between block sizes")
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT_S, help="pio test timeout in seconds")
    args = parser.parse_args()

    if args.blocks:
        if args.input or args.output:
            raise SystemExit("--blocks cannot be combined with --input or --output")
        return _run_block_sweep(args.env, args.blocks, args.pingpong_factor, not args.no_clean, args.timeout)

    output = args.input.read_text() if args.input else _run_probe(args.env, args.timeout)
    parsed = _parse_timing(output)
    if parsed.process.count == 0 and not parsed.banks:
        raise SystemExit("No AUDIO_TIMING lines found. Enable AUDIO_TIMING_ENABLE and run test_audio.")

    stamp = dt.datetime.now().strftime("%Y%m%d_%H%M%S")
    out = args.output or (ROOT / "docs/media" / f"audio_timing_{stamp}.png")
    cfg = _load_audio_config()
    _plot(parsed, cfg, out)
    print(_summary(parsed, cfg, out, status="PASSED" if "[PASSED]" in output else "not-passed/unknown"))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
