#!/usr/bin/env python3
"""Compiles this module's sources the way a build without a kernel does.

With RH_FREERTOS_KERNEL_PRESENT undefined and no FreeRTOS headers on the include
path — only robohub-common's and this module's own — every source must compile,
and define nothing, so robohub-common's weak bare-metal OSAL defaults stand
(RTOS-0003). The sources are the ones module.yml lists.
"""

# @verifies RTOS-0003

import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORKSPACE = os.environ.get("ROBOHUB_WORKSPACE", os.path.dirname(ROOT))


def sources():
    """The `build.sources` list of module.yml."""
    found, inside = [], False
    with open(os.path.join(ROOT, "module.yml")) as manifest:
        for line in manifest:
            if re.match(r"^  sources:\s*$", line) and not found:
                inside = True
                continue
            if inside:
                m = re.match(r"^    - (\S+)\s*$", line)
                if not m:
                    break
                found.append(m.group(1))
    return found


def main():
    compiler = shutil.which(os.environ.get("CC", "cc")) or shutil.which("gcc")
    nm = shutil.which("nm")
    if not compiler or not nm:
        print("FAIL: no C compiler or nm on PATH")
        return 1
    common = os.path.join(WORKSPACE, "robohub-common", "include")
    if not os.path.isdir(common):
        print(f"FAIL: robohub-common's headers are not at {common} (set ROBOHUB_WORKSPACE)")
        return 1
    listed = sources()
    if not listed:
        print("FAIL: module.yml lists no build sources")
        return 1
    failed = 0
    with tempfile.TemporaryDirectory() as work:
        for source in listed:
            obj = os.path.join(work, os.path.basename(source) + ".o")
            command = [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-c",
                       "-I", os.path.join(ROOT, "include"),
                       "-I", common,
                       "-I", os.path.join(common, "robohub"),
                       os.path.join(ROOT, source), "-o", obj]
            built = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            if built.returncode != 0:
                print(f"FAIL: {source} does not compile without a kernel:\n{built.stdout}")
                failed += 1
                continue
            symbols = subprocess.run([nm, "--defined-only", obj], stdout=subprocess.PIPE, text=True).stdout
            defined = [l.split()[-1] for l in symbols.splitlines()
                       if len(l.split()) == 3 and l.split()[1] in "TtDdBbRrWwVv"]
            if defined:
                print(f"FAIL: {source} defines {', '.join(defined)} without a kernel")
                failed += 1
                continue
            print(f"ok   {source} compiles to nothing without a kernel")
    if failed:
        print(f"{failed} failed")
        return 1
    print("all passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
