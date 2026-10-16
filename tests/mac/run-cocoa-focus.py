#!/usr/bin/env python3
"""Compile real keyboard and bounded production mouse fragments without windows."""
import argparse
import json
import os
from pathlib import Path
import re
import shlex
import subprocess


def method(source, name):
    match = re.search(r'^- \(void\)' + re.escape(name) + r'(?:\n|:)', source, re.M)
    if not match:
        raise RuntimeError('Missing production method: ' + name)
    start = match.start()
    opening = source.index('{', start)
    end, depth = opening + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build-dir', type=Path, required=True)
parser.add_argument('--mouse-only', action='store_true', help='No OIS library required')
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
args = parser.parse_args()
root = args.source_root.resolve()
build = args.build_dir.resolve()
build.mkdir(parents=True, exist_ok=True)
header = (root / 'includes/mac/CocoaMouse.h').read_text()
source = (root / 'src/mac/CocoaMouse.mm').read_text()
ivars = re.search(r'@interface CocoaMouseView : NSView \{\n(.*?)\n\}', header, re.S).group(1)
names = ('observeWindow', 'detach', 'dealloc', 'setOISMouseObj', 'focusLost', 'deliverButton', 'capture', 'mouseDown', 'mouseUp',
         'rightMouseDown', 'rightMouseUp', 'otherMouseDown', 'otherMouseUp', 'mouseMoved', 'scrollWheel',
         'mouseDragged', 'rightMouseDragged', 'otherMouseDragged', 'updateMotion', 'refreshCoordinates', 'seedCoordinates')
fixture = Path(__file__).with_name('cocoa-focus-fixture.mm.in').read_text()
fixture = fixture.replace('@PRODUCTION_IVARS@', ivars)
fixture = fixture.replace('@PRODUCTION_METHODS@', '\n\n'.join(method(source, name) for name in names))
generated = build / 'cocoa-focus-fixture.mm'
generated.write_text(fixture)
command = shlex.split(os.environ.get('CXX', 'clang++'))
command += shlex.split(os.environ.get('CXXFLAGS', ''))
command += ['-std=c++11', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', '-Wno-ignored-qualifiers', '-Wno-reorder-ctor',
            '-I', str(root / 'includes'), str(generated),
            '-framework', 'Cocoa', '-Wl,-rpath,@loader_path', '-o', str(build / 'cocoa-focus-fixture')]
if args.mouse_only:
    command += ['-DOIS_MOUSE_ONLY']
else:
    command += ['-L', str(build), '-lOIS']
(build / 'fixture-command.json').write_text(json.dumps(command, indent=2) + '\n')
subprocess.run(command, check=True)
subprocess.run([str(build / 'cocoa-focus-fixture')], check=True)
