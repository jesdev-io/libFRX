#!/usr/bin/env python3
"""Prepare/build the MkDocs site from Doxygen inline comments and .md files.

This is the deployment-time expander for handwritten API reference pages.
Source docs may contain short reference lines such as:

    ::: api audio_start
    ::: api audio.audio_start

The copied build tree receives expanded Markdown cards generated from the
Doxygen-style blocks in headers. Tracked source docs are not modified.
"""

from __future__ import annotations

import argparse
import dataclasses
import pathlib
import re
import shutil
import subprocess
import sys
from collections import defaultdict

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD_ROOT = ROOT / ".mkdocs-build"
PREPARED_ROOT = BUILD_ROOT / "prepared"

sys.path.insert(0, str(ROOT / "shared" / "scripts"))
import gen_api_docs  # noqa: E402


API_REF_RE = re.compile(r"^(?P<indent>\s*):::\s*api\s+(?P<target>[A-Za-z_][\w.:-]*)(?:\s*)$")


def _collect_items() -> list[gen_api_docs.DocItem]:
    items: list[gen_api_docs.DocItem] = []
    for base in gen_api_docs.DEFAULT_HEADERS:
        candidates = [base] if base.is_file() else sorted(base.rglob("*.h"))
        for header in candidates:
            if header.name in gen_api_docs.EXCLUDE_NAMES or header.name.endswith(gen_api_docs.EXCLUDE_SUFFIXES):
                continue
            items.extend(gen_api_docs.extract_items(header))
    return items


def _item_index(items: list[gen_api_docs.DocItem]) -> dict[str, gen_api_docs.DocItem]:
    buckets: dict[str, list[gen_api_docs.DocItem]] = defaultdict(list)
    for item in items:
        buckets[item.name].append(item)
        buckets[f"{item.module}.{item.name}"].append(item)
        buckets[f"{item.module}:{item.name}"].append(item)
        buckets[f"{item.module}.{item.kind}.{item.name}"].append(item)
        buckets[f"{item.module}:{item.kind}:{item.name}"].append(item)

    index: dict[str, gen_api_docs.DocItem] = {}
    ambiguous = {key for key, vals in buckets.items() if len(vals) > 1}
    for key, vals in buckets.items():
        if key not in ambiguous:
            index[key] = vals[0]
    return index


def _render_item(item: gen_api_docs.DocItem) -> str:
    lines: list[str] = [
        f'<a id="api-{item.name}"></a>',
        f"### `{item.name}`",
        "",
        f"_Kind: {item.kind}. Source: `{item.source}`._",
        "",
        "```c",
        item.signature,
        "```",
        "",
    ]
    if item.brief:
        lines += [item.brief, ""]
    if item.details:
        lines += [" ".join(item.details), ""]
    if item.params:
        lines += ["| Parameter | Description |", "|---|---|"]
        for name, desc in item.params:
            lines.append(f"| `{name}` | {desc or '—'} |")
        lines.append("")
    if item.returns:
        lines += [f"**Returns:** {item.returns}", ""]
    for note in item.notes:
        lines += [f"> **Note:** {note}", ""]
    return "\n".join(lines).rstrip()


def _expand_api_refs(md: str, index: dict[str, gen_api_docs.DocItem], rel: pathlib.Path) -> tuple[str, list[str]]:
    unresolved: list[str] = []
    out: list[str] = []
    for line_no, line in enumerate(md.splitlines(), 1):
        m = API_REF_RE.match(line)
        if not m:
            out.append(line)
            continue
        target = m.group("target")
        item = index.get(target)
        if item is None:
            unresolved.append(f"{rel}:{line_no}: unresolved API reference {target!r}")
            out.append(line)
            continue
        out.append(_render_item(item))
    return "\n".join(out).rstrip() + "\n", unresolved


def _copy_source_tree(prepared_root: pathlib.Path) -> None:
    if prepared_root.exists():
        shutil.rmtree(prepared_root)
    prepared_root.mkdir(parents=True)
    shutil.copytree(ROOT / "docs", prepared_root / "docs")
    shutil.copy2(ROOT / "mkdocs.yml", prepared_root / "mkdocs.yml")


def prepare(prepared_root: pathlib.Path, check: bool = True) -> pathlib.Path:
    _copy_source_tree(prepared_root)

    docs_dir = prepared_root / "docs"
    items = _collect_items()
    index = _item_index(items)

    unresolved: list[str] = []
    for md_path in sorted(docs_dir.rglob("*.md")):
        rel = md_path.relative_to(docs_dir)
        expanded, errors = _expand_api_refs(md_path.read_text(), index, rel)
        unresolved.extend(errors)
        md_path.write_text(expanded)

    gen_api_docs.generate(gen_api_docs.DEFAULT_HEADERS, docs_dir / "reference" / "generated")

    if unresolved and check:
        raise SystemExit("\n".join(unresolved))
    return prepared_root / "mkdocs.yml"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--prepared-root", type=pathlib.Path, default=PREPARED_ROOT)
    parser.add_argument("--site-dir", type=pathlib.Path, default=BUILD_ROOT / "site")
    parser.add_argument("--no-check", action="store_true", help="Leave unresolved ::: api refs in place")
    parser.add_argument("--build", action="store_true", help="Run mkdocs build after preparing the copied tree")
    parser.add_argument("--strict", action="store_true", help="Pass --strict to mkdocs build")
    args = parser.parse_args()

    prepared_root = args.prepared_root if args.prepared_root.is_absolute() else ROOT / args.prepared_root
    site_dir = args.site_dir if args.site_dir.is_absolute() else ROOT / args.site_dir
    cfg = prepare(prepared_root, check=not args.no_check)
    print(f"prepared MkDocs tree: {prepared_root.relative_to(ROOT)}")
    print(f"config: {cfg.relative_to(ROOT)}")

    if args.build:
        cmd = ["mkdocs", "build", "-f", str(cfg), "-d", str(site_dir)]
        if args.strict:
            cmd.append("--strict")
        subprocess.run(cmd, cwd=ROOT, check=True)
        print(f"site: {site_dir.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
