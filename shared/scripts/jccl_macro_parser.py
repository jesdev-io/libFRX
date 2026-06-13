#!/usr/bin/env python3
"""
JCCL Macro Parser
-----------------
Scans for lib files ending with _jccl.h and translates macros into Python regexes.
Generates Python files for CLI unit tests.
"""

import re
from pathlib import Path


def parse_jccl_file(file_path):
    with open(file_path, 'r') as f:
        content = f.read()
    macros = re.findall(r'#define\s+(\w+)\s+"([^"]+)"', content)
    return {name: value for name, value in macros}


def macro_to_regex(value):
    value = value.replace('%d', '__PERCENT_D__')
    value = value.replace('%s', '__PERCENT_S__')
    escaped = re.escape(value)
    escaped = escaped.replace('__PERCENT_D__', r'\d+')
    escaped = escaped.replace('__PERCENT_S__', r'.*?')
    escaped = escaped.replace(r'\ ', ' ')
    return escaped


def generate_python_file(macros, module_name, output_dir):
    """Generate a Python file with regex patterns."""
    python_file = output_dir / f"{module_name}_strings.py"
    with open(python_file, 'w') as f:
        f.write(f"# Auto-generated from {module_name}_jccl.h\n")
        f.write("# Do not edit manually!\n\n")
        for name, value in macros.items():
            var_name = name
            regex_value = macro_to_regex(value)
            f.write(f'{var_name} = r"{regex_value}"\n')


def main():
    input_dir = Path("lib")
    output_dir = Path("tests")
    output_dir.mkdir(exist_ok=True)

    jccl_files = list(input_dir.rglob('*_jccl.h'))
    if not jccl_files:
        print(f'No _jccl.h files found in {input_dir}')
        return

    for jccl_file in jccl_files:
        module_name = jccl_file.stem.replace('_jccl', '')
        macros = parse_jccl_file(jccl_file)
        generate_python_file(macros, module_name, output_dir)
        print(f'Generated {output_dir}/{module_name}_strings.py from {jccl_file}')

main()
