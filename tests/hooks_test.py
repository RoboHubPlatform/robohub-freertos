#!/usr/bin/env python3
"""Builds and runs tests/hooks_test.c on the host (see that file for what it checks).

It compiles src/freertos_hooks.c with the real config/FreeRTOSConfig.h and the
stand-ins in tests/stubs, against robohub-common's console header, and fails when
the compile does (a default that is not what RTOS-0007 says is an #error) or the
program reports a failure.
"""

# @verifies RTOS-0006, RTOS-0007

import os
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORKSPACE = os.environ.get("ROBOHUB_WORKSPACE", os.path.dirname(ROOT))


def main():
    compiler = shutil.which(os.environ.get("CC", "cc")) or shutil.which("gcc")
    if not compiler:
        print("FAIL: no C compiler on PATH")
        return 1
    common = os.path.join(WORKSPACE, "robohub-common", "include")
    if not os.path.isdir(common):
        print(f"FAIL: robohub-common's headers are not at {common} (set ROBOHUB_WORKSPACE)")
        return 1
    with tempfile.TemporaryDirectory() as work:
        binary = os.path.join(work, "hooks_test")
        command = [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror", "-DRH_FREERTOS_KERNEL_PRESENT=1",
                   "-I", os.path.join(ROOT, "tests", "stubs"),
                   "-I", os.path.join(ROOT, "config"),
                   "-I", os.path.join(ROOT, "include"),
                   "-I", common,
                   os.path.join(ROOT, "tests", "hooks_test.c"),
                   os.path.join(ROOT, "src", "freertos_hooks.c"),
                   "-o", binary]
        built = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        if built.returncode != 0:
            print("FAIL: the hooks test does not build:\n" + built.stdout)
            return 1
        try:
            return subprocess.run([binary], timeout=20).returncode
        except subprocess.TimeoutExpired:
            print("FAIL: a fatal path never returned to the test's halt: it spins where the "
                  "halt should be RH_FREERTOS_HALT(), so its order cannot be checked")
            return 1


if __name__ == "__main__":
    sys.exit(main())
