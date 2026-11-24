#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# The web editor in one file: web/src/editor.css, body.html and the scripts, inlined into web/editor.html (a page
# that works opened from the disk, with nothing else). Written for this project.
import os, sys
SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'web', 'src')
JS = ['core.js', 'live.js', 'sound.js', 'seq.js', 'mixer.js', 'library.js', 'files.js', 'kit.js', 'banks.js', 'main.js']
def rd(n):
    with open(os.path.join(SRC, n), encoding='utf-8') as f: return f.read()
def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(SRC, '..', 'editor.html')
    js = '\n'.join(rd(n) for n in JS)
    for bad in ('</script', '<!--'):
        assert bad not in js.lower(), bad
    html = ('<!DOCTYPE html>\n<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">\n'
            '<title>SLOOP 3 editor</title>\n<style>\n' + rd('editor.css') + '</style></head>\n<body>\n' + rd('body.html') +
            '<script>\n"use strict";\n' + js + '</script></body></html>\n')
    with open(out, 'w', encoding='utf-8') as f: f.write(html)
    print('%s: %d bytes' % (out, len(html.encode())))
if __name__ == '__main__': main()
