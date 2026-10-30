#!/usr/bin/env python3
"""Generate both regional native backends from user-supplied ROMs."""
from pathlib import Path
import hashlib
import os
import subprocess

root = Path(__file__).resolve().parents[1]
suffix = '.exe' if os.name == 'nt' else ''
roms = {}
for region in ('us', 'jp'):
    stem = 'snowboardkids' + ('.jp' if region == 'jp' else '')
    roms[region] = (root / (stem + '.z64')).read_bytes()
    expected = (root / 'lib/sk1-decomp' / (stem + '.sha1')).read_text().split()[0]
    if hashlib.sha1(roms[region]).hexdigest() != expected:
        raise SystemExit(f'{stem}.z64 does not match the supported original ROM.')
# The two regions share the same aspMain instructions, despite different ROM offsets.
if roms['us'][0xB1AA0:0xB28C0] != roms['jp'][0xB1390:0xB21B0]:
    raise SystemExit('Regional audio microcode differs; generate a separate JP RSP backend.')
for region in ('us', 'jp'):
    subprocess.run([str(root / ('N64Recomp' + suffix)), f'{region}.toml'], cwd=root, check=True)
subprocess.run([str(root / ('RSPRecomp' + suffix)), 'aspMain.us.toml'], cwd=root, check=True)
