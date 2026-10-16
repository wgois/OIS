#!/usr/bin/env python3
"""Run the shared Cocoa mouse/focus/coordinate production-fragment regression."""
import argparse
from pathlib import Path
import subprocess
import sys
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
parser.add_argument('--output-dir', type=Path)
args = parser.parse_args()
root = args.source_root.resolve()
if not (root / 'includes/OISPrereqs.h').exists():
    parser.error('Configure OIS with CMake first to generate OISPrereqs.h')
def run(output):
    subprocess.run([sys.executable, str(Path(__file__).with_name('run-cocoa-focus.py')),
                    '--source-root', str(root), '--build-dir', str(output), '--mouse-only'], check=True)
try:
    if args.output_dir:
        run(args.output_dir)
    else:
        with tempfile.TemporaryDirectory(prefix='ois-cocoa-motion-') as directory:
            run(directory)
except subprocess.CalledProcessError as error:
    raise SystemExit(error.returncode)
