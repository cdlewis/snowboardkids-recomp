"""Check copied functions against decomp and execute real GBI load macros."""
import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GROUPS = {
    'menu/renderer/menu_render_utils.c': ['drawAssetTableSprite', 'drawPulsingAssetTableSprite',
        'drawAssetTableSpriteWithDefaultPalette', 'drawAssetTableSprite8bpp',
        'drawAssetTableSpriteWithExplicitPalette', 'drawMenuAsciiCharImpl'],
    'menu/renderer/menu_renderer.c': ['drawMenuSpriteWithAlphaClipped', 'drawMenuSpriteSubrect',
        'drawMenuSpriteTileClipped', 'drawMenuGlyph', 'drawMenuColoredGlyph', 'drawMenuAsciiGlyph'],
    'menu/splitscreen_select/race_splitscreen_select_ui.c': ['drawMenuPanelBackdrop'],
}


def no_comments(text):
    # C joins physical line continuations before interpreting comments or tokens.
    text = text.replace("\\\n", "")
    return re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)


def function(text, name):
    text = no_comments(text)
    match = re.search(r'^(?:RECOMP_PATCH )?void '+name+r'\s*\(', text, re.M)
    start = text.index('{', match.end()); end = start + 1; depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}'); end += 1
    return text[match.start():end].replace('RECOMP_PATCH ', '')


def loads(text):
    for match in re.finditer(r'\bgDPLoadTextureTile(_4b)?\s*\(', text):
        start = match.end(); index = start; depth = 0; args = []
        while True:
            char = text[index]
            if char == '(':
                depth += 1
            elif char == ')':
                if not depth:
                    args.append((start, index)); break
                depth -= 1
            elif char == ',' and not depth:
                args.append((start, index)); start = index + 1
            index += 1
        yield bool(match.group(1)), args


class LoadBoundsTests(unittest.TestCase):
    def test_new_replacements_only_change_endpoints(self):
        patch = (ROOT/'patches/sprite_texture_sampling.c').read_text()
        count = 0
        for source, names in GROUPS.items():
            decomp = (ROOT/'lib/sk1-decomp/src'/source).read_text()
            for name in names:
                old, new = function(decomp, name), function(patch, name)
                for four_bit, args in reversed(list(loads(old))):
                    endpoint = 7 if four_bit else 8
                    for a, b in reversed(args[endpoint:endpoint+2]):
                        old = old[:a] + old[a:b].rstrip() + ' - 1' + old[b:]
                    count += 1
                self.assertEqual(re.sub(r'\s+', '', old), re.sub(r'\s+', '', new), name)
        self.assertEqual(count, 14)

    def test_tilemap_and_existing_patch_ownership(self):
        names = [name for group in GROUPS.values() for name in group] + ['drawMenuTilemapSprite']
        all_patches = '\n'.join(p.read_text() for p in (ROOT/'patches').glob('*.c'))
        for name in names:
            self.assertEqual(len(re.findall(r'RECOMP_PATCH\s+void\s+'+name+r'\s*\(', all_patches)), 1, name)
        code = function((ROOT/'patches/menu_background_widescreen.c').read_text(), 'drawMenuTilemapSprite')
        calls = list(loads(code)); self.assertEqual(len(calls), 2)
        for four_bit, args in calls:
            endpoint = 7 if four_bit else 8
            self.assertEqual(code[slice(*args[endpoint])].strip(), 'render->tileWidth - 1')
            self.assertEqual(code[slice(*args[endpoint+1])].strip(), 'render->tileHeight - 1')

    def test_emitted_gbi_bounds_and_stride(self):
        compiler = shutil.which('clang')
        if not compiler:
            self.skipTest('clang required to execute project GBI macros')
        source = '''#include <stdio.h>
#include <PR/mbi.h>
#include <PR/gbi.h>
int main(void) {
Gfx commands[32]; Gfx *p;
'''
        cases = []
        for bits, fmt, width, height in [(4,'CI',16,16),(4,'CI',32,32),(4,'CI',64,32),
                                       (8,'CI',32,32),(8,'CI',16,32),(4,'I',16,8)]:
            cases.append((bits,width,height))
            macro = 'gDPLoadTextureTile_4b' if bits == 4 else 'gDPLoadTextureTile'
            size = '' if bits == 4 else 'G_IM_SIZ_8b,'
            source += f'''p=commands;
{macro}(p++,(void*)0,G_IM_FMT_{fmt},{size}{width},{height},0,0,{width-1},{height-1},0,
G_TX_CLAMP,G_TX_CLAMP,G_TX_NOMASK,G_TX_NOMASK,G_TX_NOLOD,G_TX_NOLOD);
printf("%u %u %u\\n",(unsigned)commands[5].words.w0,(unsigned)commands[6].words.w0,(unsigned)commands[6].words.w1);
'''
        source += 'return 0;}\n'
        with tempfile.TemporaryDirectory() as directory:
            c = Path(directory)/'check.c'; binary = Path(directory)/'check'; c.write_text(source)
            subprocess.run([compiler, '-w', '-DF3DEX_GBI', '-D_LANGUAGE_C', '-I',
                            str(ROOT/'lib/sk1-decomp/include'), str(c), '-o', str(binary)], check=True)
            lines = subprocess.check_output([str(binary)], text=True).splitlines()
        self.assertEqual(len(lines), len(cases))
        for (bits,width,height), line in zip(cases, lines):
            set_tile, size0, size1 = map(int, line.split())
            self.assertEqual(size0 >> 24, 0xF2)
            self.assertEqual((size1 >> 12) & 0xFFF, (width-1)*4)
            self.assertEqual(size1 & 0xFFF, (height-1)*4)
            self.assertEqual((set_tile >> 9) & 0x1FF, (width*bits+63)//64)
