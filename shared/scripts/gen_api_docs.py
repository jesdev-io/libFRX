#!/usr/bin/env python3
"""Generate concise MkDocs API reference from Doxygen-style comments.

This is intentionally small and dependency-free. It does not try to replace
Doxygen; it extracts the public comment blocks we already maintain in headers
and writes Markdown snippets that can be included from handwritten MkDocs pages.
"""

from __future__ import annotations

import argparse
import dataclasses
import pathlib
import re
from collections import defaultdict

ROOT = pathlib.Path(__file__).resolve().parents[2]
DEFAULT_HEADERS = [ROOT / "include", ROOT / "lib"]
DEFAULT_OUT = ROOT / "docs" / "reference" / "generated"

EXCLUDE_SUFFIXES = ("_jccl.h",)
EXCLUDE_NAMES = {"audio_default_cfg.h", "sdcard_default_cfg.h"}


@dataclasses.dataclass
class DocItem:
    module: str
    name: str
    kind: str
    source: pathlib.Path
    signature: str
    brief: str = ""
    details: list[str] = dataclasses.field(default_factory=list)
    params: list[tuple[str, str]] = dataclasses.field(default_factory=list)
    returns: str = ""
    notes: list[str] = dataclasses.field(default_factory=list)


def _module_for(path: pathlib.Path) -> str:
    rel = path.relative_to(ROOT)
    if rel.parts[0] == "lib" and len(rel.parts) > 2:
        return rel.parts[1]
    if rel.parts[0] == "include":
        return "core"
    return rel.stem


def _clean_comment_line(line: str) -> str:
    line = line.strip()
    if line.startswith("///"):
        line = line[3:]
    elif line.startswith("//!"):
        line = line[3:]
    elif line.startswith("/**") or line.startswith("/*!"):
        line = line[3:]
    elif line.startswith("*/"):
        line = ""
    elif line.startswith("*"):
        line = line[1:]
    return line.strip()


def _parse_comment(lines: list[str]) -> tuple[str, list[str], list[tuple[str, str]], str, list[str]]:
    brief = ""
    details: list[str] = []
    params: list[tuple[str, str]] = []
    returns = ""
    notes: list[str] = []
    current: tuple[str, str] | None = None

    def append_continuation(text: str) -> None:
        nonlocal returns
        if current and params and params[-1][0] == current[1]:
            params[-1] = (params[-1][0], (params[-1][1] + " " + text).strip())
        elif current and current[0] == "return":
            returns = (returns + " " + text).strip()
        elif current and current[0] == "note" and notes:
            notes[-1] = (notes[-1] + " " + text).strip()
        elif text:
            details.append(text)

    for raw in lines:
        text = _clean_comment_line(raw)
        if not text:
            current = None
            continue
        if text.startswith(("@brief", "\\brief")):
            brief = re.sub(r"^[@\\]brief\s*", "", text).strip()
            current = ("brief", "")
        elif text.startswith(("@param", "\\param")):
            m = re.match(r"^[@\\]param(?:\[[^\]]+\])?\s+(\w+)\s*(.*)$", text)
            if m:
                params.append((m.group(1), m.group(2).strip()))
                current = ("param", m.group(1))
        elif text.startswith(("@return", "\\return", "@returns", "\\returns")):
            returns = re.sub(r"^[@\\]returns?\s*", "", text).strip()
            current = ("return", "")
        elif text.startswith(("@note", "\\note")):
            notes.append(re.sub(r"^[@\\]note\s*", "", text).strip())
            current = ("note", "")
        elif text.startswith("@") or text.startswith("\\"):
            current = None
        else:
            append_continuation(text)
    return brief, details, params, returns, notes


def _collect_declaration(lines: list[str], start: int) -> tuple[str, int]:
    decl_lines: list[str] = []
    brace_depth = 0
    i = start
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()
        if not decl_lines and (not stripped or stripped.startswith("#")):
            i += 1
            continue
        decl_lines.append(line.rstrip())
        brace_depth += line.count("{") - line.count("}")
        if stripped.endswith(";") and brace_depth <= 0:
            break
        if len(decl_lines) > 200:
            break
        i += 1
    return "\n".join(decl_lines).strip(), i


def _classify(decl: str) -> tuple[str, str] | None:
    one = re.sub(r"\s+", " ", decl).strip()
    if not one or one.startswith("#"):
        return None
    if "typedef enum" in one:
        m = re.search(r"}\s*(\w+)\s*;\s*$", one)
        return ("enum", m.group(1) if m else "anonymous_enum")
    if "typedef struct" in one:
        m = re.search(r"}\s*(\w+)\s*;\s*$", one)
        return ("struct", m.group(1) if m else "anonymous_struct")
    if "typedef union" in one:
        m = re.search(r"}\s*(\w+)\s*;\s*$", one)
        return ("union", m.group(1) if m else "anonymous_union")
    if one.startswith("typedef"):
        m = re.search(r"\(\s*\*\s*(\w+)\s*\)", one) or re.search(r"\b(\w+)\s*;\s*$", one)
        return ("typedef", m.group(1) if m else "typedef")
    if "(" in one and one.endswith(";") and not one.startswith(("if", "for", "while")):
        m = re.search(r"([A-Za-z_]\w*)\s*\([^;]*\)\s*;\s*$", one)
        if m:
            return ("function", m.group(1))
    if one.startswith("#define"):
        parts = one.split()
        if len(parts) >= 2:
            return ("macro", parts[1].split("(")[0])
    return None


def _signature(decl: str, kind: str) -> str:
    # Keep compound declarations readable but compact enough for reference cards.
    if kind in {"function", "typedef", "macro"}:
        return re.sub(r"\s+", " ", decl).strip()
    lines = [ln.rstrip() for ln in decl.splitlines()]
    if len(lines) <= 80:
        return "\n".join(lines)
    return "\n".join(lines[:77] + ["    // ...", lines[-1]])


def extract_items(header: pathlib.Path) -> list[DocItem]:
    lines = header.read_text(errors="replace").splitlines()
    items: list[DocItem] = []
    pending: list[str] = []
    in_block = False
    i = 0
    while i < len(lines):
        stripped = lines[i].strip()
        if stripped.startswith("///") or stripped.startswith("//!"):
            pending.append(lines[i])
            i += 1
            continue
        if stripped.startswith("/**") or stripped.startswith("/*!"):
            in_block = True
            pending.append(lines[i])
            i += 1
            continue
        if in_block:
            pending.append(lines[i])
            if "*/" in stripped:
                in_block = False
            i += 1
            continue
        if pending and (not stripped or stripped.startswith("#if") or stripped.startswith("#endif") or stripped.startswith("#else")):
            i += 1
            continue
        if pending:
            decl, end = _collect_declaration(lines, i)
            classified = _classify(decl)
            if classified:
                kind, name = classified
                brief, details, params, returns, notes = _parse_comment(pending)
                items.append(
                    DocItem(
                        module=_module_for(header),
                        name=name,
                        kind=kind,
                        source=header.relative_to(ROOT),
                        signature=_signature(decl, kind),
                        brief=brief,
                        details=details,
                        params=params,
                        returns=returns,
                        notes=notes,
                    )
                )
                i = end + 1
            else:
                i += 1
            pending = []
            continue
        i += 1
    return items


def _write_module(module: str, items: list[DocItem], out_dir: pathlib.Path) -> pathlib.Path:
    out = out_dir / f"{module}_api.md"
    out.parent.mkdir(parents=True, exist_ok=True)
    groups = defaultdict(list)
    for item in items:
        groups[item.kind].append(item)

    kind_order = ["struct", "union", "enum", "typedef", "function", "macro"]
    md: list[str] = [
        "## API Reference",
        "",
        "<!-- Generated by shared/scripts/gen_api_docs.py; do not edit by hand. -->",
        "",
    ]
    for kind in kind_order:
        group = sorted(groups.get(kind, []), key=lambda x: x.name)
        if not group:
            continue
        md += [f"## {kind.title()}s", ""]
        for item in group:
            md += [f"### `{item.name}`", "", f"_Source: `{item.source}`_", "", "```cpp", item.signature, "```", ""]
            if item.brief:
                md += [item.brief, ""]
            if item.details:
                md += [" ".join(item.details), ""]
            if item.params:
                md += ["| Parameter | Description |", "|---|---|"]
                for name, desc in item.params:
                    md.append(f"| `{name}` | {desc or '—'} |")
                md.append("")
            if item.returns:
                md += [f"**Returns:** {item.returns}", ""]
            for note in item.notes:
                md += [f"> **Note:** {note}", ""]
    out.write_text("\n".join(md).rstrip() + "\n")
    return out


def generate(headers: list[pathlib.Path], out_dir: pathlib.Path) -> list[pathlib.Path]:
    by_module: dict[str, list[DocItem]] = defaultdict(list)
    for base in headers:
        candidates = [base] if base.is_file() else sorted(base.rglob("*.h"))
        for header in candidates:
            if header.name in EXCLUDE_NAMES or header.name.endswith(EXCLUDE_SUFFIXES):
                continue
            for item in extract_items(header):
                by_module[item.module].append(item)
    written = []
    for module, items in sorted(by_module.items()):
        if items:
            written.append(_write_module(module, items, out_dir))
    index = out_dir / "index.md"
    index.parent.mkdir(parents=True, exist_ok=True)
    index.write_text(
        "# Generated API Reference\n\n"
        "<!-- Generated by shared/scripts/gen_api_docs.py; do not edit by hand. -->\n\n"
        + "\n".join(f"- [{p.stem.replace('_api', '').title()}]({p.name})" for p in written)
        + "\n"
    )
    written.append(index)
    return written


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=pathlib.Path, default=DEFAULT_OUT)
    parser.add_argument("headers", nargs="*", type=pathlib.Path, default=DEFAULT_HEADERS)
    args = parser.parse_args()
    headers = [p if p.is_absolute() else ROOT / p for p in args.headers]
    out = args.out if args.out.is_absolute() else ROOT / args.out
    written = generate(headers, out)
    print(f"generated {len(written)} markdown files in {out.relative_to(ROOT)}")
    for path in written:
        print(path.relative_to(ROOT))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
