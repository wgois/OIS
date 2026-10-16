#!/usr/bin/env python3
"""Run bounded Cocoa motion production fragments without NSView or cursor APIs."""

import argparse
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile


def method(source, name):
    signature = "- (void)" + name
    if name != "capture":
        signature += ":(NSEvent*)theEvent"
    marker = signature + "\n{"
    if source.count(marker) != 1:
        raise RuntimeError("Expected one production method: " + signature)
    start = source.index(marker)
    opening = start + len(marker) - 1
    depth = 1
    end = opening + 1
    while depth and end < len(source):
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    if depth:
        raise RuntimeError("Unterminated production method: " + signature)
    return source[start:end]


def run(root, output):
    header = (root / "includes/mac/CocoaMouse.h").read_text()
    source = (root / "src/mac/CocoaMouse.mm").read_text()
    match = re.search(r"@interface CocoaMouseView : NSView \{\n(.*?)\n\}",
                      header, re.DOTALL)
    if not match or header.count("@interface CocoaMouseView : NSView {") != 1:
        raise RuntimeError("Unexpected production responder declaration")
    names = ("capture", "mouseMoved", "mouseDragged", "rightMouseDragged",
             "otherMouseDragged", "scrollWheel", "mouseEntered")
    fragments = "\n\n".join(method(source, name) for name in names)
    template = Path(__file__).with_name("cocoa-motion-fixture.mm.in").read_text()
    fixture = template.replace("@PRODUCTION_IVARS@", match.group(1))
    fixture = fixture.replace("@PRODUCTION_METHODS@", fragments)
    generated = output / "cocoa-motion-fixture.mm"
    executable = output / "cocoa-motion-fixture"
    generated.write_text(fixture)
    command = shlex.split(os.environ.get("CXX", "clang++"))
    command += shlex.split(os.environ.get("CXXFLAGS", ""))
    command += ["-std=c++11", "-Wall", "-Wextra", "-Werror",
                "-Wno-unused-parameter", "-I", str(root / "includes"),
                str(generated), "-framework", "Foundation", "-o", str(executable)]
    subprocess.run(command, check=True)
    subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path,
                        default=Path(__file__).resolve().parents[2])
    parser.add_argument("--output-dir", type=Path)
    args = parser.parse_args()
    root = args.source_root.resolve()
    if not (root / "includes/OISPrereqs.h").exists():
        parser.error("Configure OIS with CMake first to generate OISPrereqs.h")
    try:
        if args.output_dir:
            args.output_dir.mkdir(parents=True, exist_ok=True)
            run(root, args.output_dir.resolve())
        else:
            with tempfile.TemporaryDirectory(prefix="ois-cocoa-motion-") as directory:
                run(root, Path(directory))
    except subprocess.CalledProcessError as error:
        raise SystemExit(error.returncode)
