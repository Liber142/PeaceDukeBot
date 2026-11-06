#!/usr/bin/env python3
"""One command, no downloads: python3 tests/run.py [--mode all|normal|address|undefined|thread]."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--mode", choices=["all", "normal", "address", "undefined", "thread"], default="all")
p.add_argument("--jobs", type=int, default=min(4, os.cpu_count() or 1))
a = p.parse_args()
results = {}
for mode in (["normal", "address", "undefined", "thread"] if a.mode == "all" else [a.mode]):
    build = root / "build" / ("tests-" + mode)
    build.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ)
    if mode == "undefined":
        env["UBSAN_OPTIONS"] = "halt_on_error=1:print_stacktrace=1"
    if mode == "address":
        env["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1"
    if mode == "thread":
        env["TSAN_OPTIONS"] = "halt_on_error=1"
    def run(args, log):
        print("+", " ".join(map(str, args)), flush=True)
        with (build / log).open("w") as f:
            result = subprocess.run(list(map(str, args)), cwd=root, env=env, stdout=f, stderr=subprocess.STDOUT)
        print((build / log).read_text()[-16000:], flush=True)
        return result.returncode
    for counter in build.rglob("*.gcda"):
        counter.unlink()
    rc = run(["cmake", "-S", root, "-B", build, "-DPEACE_TESTS_ONLY=ON", "-DCMAKE_BUILD_TYPE=Debug",
              "-DPEACE_COVERAGE=" + ("ON" if mode == "normal" else "OFF"),
              "-DPEACE_SANITIZER=" + ("" if mode == "normal" else mode)], "configure.log")
    if not rc:
        rc = run(["cmake", "--build", build, "-j", a.jobs], "build.log")
    if rc:
        results[mode] = {"build_exit": rc}
        continue
    # Remove stale counters before every measurement.
    for counter in build.rglob("*.gcda"):
        counter.unlink()
    rc = run(["ctest", "--test-dir", build, "--output-on-failure", "--output-junit", "results.xml", "-j", a.jobs], "ctest.log")
    tree = ET.parse(build / "results.xml")
    cases = tree.findall(".//testcase")
    failed = [c.attrib["name"] for c in cases if c.find("failure") is not None]
    results[mode] = {"exit": rc, "total": len(cases), "passed": len(cases) - len(failed), "failed": failed}
    if mode == "normal" and shutil.which("gcovr"):
        coverage_rc = run(["gcovr", "--root", root, "--filter", str(root / "src") + "/",
                          "--exclude", r".*/src/tests/.*", "--exclude-unreachable-branches", "--exclude-throw-branches",
                          "--json-summary", build / "coverage-summary.json", "--json", build / "coverage.json",
                          "--html-details", build / "coverage.html", "--print-summary", build], "coverage.log")
        results[mode]["coverage_exit"] = coverage_rc
    elif mode == "normal":
        results[mode]["coverage"] = "unrun: gcovr is not installed"
(root / "build" / ("summary-" + a.mode + ".json")).write_text(__import__("json").dumps(results, indent=2))
print(__import__("json").dumps(results, indent=2))
sys.exit(int(any(r.get("exit", r.get("build_exit", 1)) or r.get("coverage_exit", 0) for r in results.values())))
