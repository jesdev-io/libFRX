# Contributing

Want/need something fixed/new? Get into gear and help `libFRX` to grow!

## Adding or changing Modules

Follow the existing FRX Module shape:

```text
lib/<module>/
├── <module>.h               # function/struct declarations with docstrings
├── <module>.cpp             # your smart driver code
├── <module>_default_cfg.h   # compile-time defaults and required macros
└── <module>_jccl.h          # (optional) CLI command names/messages, if the module has CLI control
```

Keep board-specific pins and product policy outside the Implementation. Required
hardware pins should be compile-time macros supplied by the consuming firmware or
PlatformIO environment. If you want to build CLI compatibility, its strings should live in `<your_module>_jccl.h`; configuration
macros should live in `<your_module>_default_cfg.h`.

## Testing

Embedded tests are PlatformIO environments in this repository. They build and may
upload firmware to attached hardware:

```bash
pio test -e frx_test_audio
pio test -e frx_test_audio --without-uploading --without-testing
```

Python CLI tests exercise jescore/jescorecli-facing behavior:

```bash
uv run -m pytest tests/test_sdcard_cli.py -v
```

## Documentation source vs rendered output

Docs source lives in `docs/` and `mkdocs.yml`. Some Markdown pages may contain
compact API markers such as:

```md
::: api audio_start
```

Those markers are not meant to render directly from the source tree. During docs
build, they are expanded from Doxygen-style comments in the C/C++ headers into a
temporary MkDocs tree under `.mkdocs-build/`.

Do not edit files under `.mkdocs-build/`; they are generated and ignored by git.

## Preview the wiki locally

Build the temporary expanded docs tree and serve that copy:

```bash
uv run python shared/scripts/build_mkdocs_site.py --build
uv run mkdocs serve -f .mkdocs-build/prepared/mkdocs.yml
```

Open <http://127.0.0.1:8000>.

## API reference generator

`shared/scripts/gen_api_docs.py` is a small dependency-free parser. It scans
Doxygen-style comments in public headers under `include/` and `lib/`, then emits
MkDocs-compatible Markdown API cards.

Normal docs builds call it inside the temporary `.mkdocs-build/` tree. Only run
it directly when you intentionally want to refresh tracked generated snapshots:

```bash
python3 shared/scripts/gen_api_docs.py
```

## JCCL string fixtures

`shared/scripts/jccl_macro_parser.py` scans `*_jccl.h` files and regenerates
Python regex fixtures in `tests/`. PlatformIO runs it as a pre-script, but if you
change JCCL macros and want to refresh tracked fixtures manually, run:

```bash
python3 shared/scripts/jccl_macro_parser.py
```
