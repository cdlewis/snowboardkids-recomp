#!/usr/bin/env python3
"""Give JP-generated native functions distinct linker names, preserving export strings."""
import argparse
from pathlib import Path
import re

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('sources', type=Path, nargs='+')
args = parser.parse_args()
names = {'get_entrypoint_address', 'get_rom_name', 'mm_patches_bin', 'mm_patches_bin_size'}
for directory in args.sources:
    for source in directory.glob('*.c'):
        names.update(re.findall(r'RECOMP_FUNC\s+void\s+(\w+)\s*\(', source.read_text()))
content = '#pragma once\n// Generated native names; ROM addresses and mod export names stay unchanged.\n'
content += ''.join(f'#define {name} jp_{name}\n' for name in sorted(names))
args.output.parent.mkdir(parents=True, exist_ok=True)
if not args.output.exists() or args.output.read_text() != content:
    args.output.write_text(content)
