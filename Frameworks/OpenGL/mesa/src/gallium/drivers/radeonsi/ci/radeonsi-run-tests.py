#!/usr/bin/python3
#
# Copyright 2021 Advanced Micro Devices, Inc.
#
# SPDX-License-Identifier: MIT
#

import os
import sys
import argparse
import subprocess
import shutil
from datetime import datetime
import tempfile
import itertools
import filecmp
import multiprocessing
import csv


def print_red(txt, end_line=True, prefix=None):
    if prefix:
        print(prefix, end="")
    print("\033[0;31m{}\033[0m".format(txt), end="\n" if end_line else " ")


def print_yellow(txt, end_line=True, prefix=None):
    if prefix:
        print(prefix, end="")
    print("\033[1;33m{}\033[0m".format(txt), end="\n" if end_line else " ")


def print_green(txt, end_line=True, prefix=None):
    if prefix:
        print(prefix, end="")
    print("\033[1;32m{}\033[0m".format(txt), end="\n" if end_line else " ")


parser = argparse.ArgumentParser(
    description="radeonsi tester",
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
parser.add_argument(
    "--jobs",
    "-j",
    type=int,
    help="Number of processes/threads to use.",
    default=multiprocessing.cpu_count(),
)

# The path to above the mesa directory, i.e. ../../../../../..
path_above_mesa = os.path.realpath(os.path.join(os.path.dirname(__file__), *['..'] * 6))

parser.add_argument("--piglit-path", type=str, help="Path to piglit source folder.")
parser.add_argument("--glcts-path", type=str, help="Path to GLCTS source folder.")
parser.add_argument("--escts-path", type=str, help="Path to GLES CTS source folder.")
parser.add_argument("--deqp-path", type=str, help="Path to dEQP source folder.")
parser.add_argument(
    "--parent-path",
    type=str,
    help="Path to folder containing piglit/GLCTS and dEQP source folders.",
    default=os.getenv('MAREKO_BUILD_PATH', path_above_mesa),
)
parser.add_argument("--verbose", "-v", action="count", default=0)
parser.add_argument(
    "--include-tests",
    "-t",
    action="append",
    dest="include_tests",
    default=[],
    help="Only run the test matching this expression. This can only be a filename containing a list of failing tests to re-run.",
)
parser.add_argument(
    "--baseline",
    dest="baseline",
    help="Folder containing expected results files",
    default=os.path.dirname(__file__),
)
parser.add_argument(
    "--no-piglit", dest="piglit", help="Disable piglit tests", action="store_false"
)
parser.add_argument(
    "--no-glcts", dest="glcts", help="Disable GLCTS tests", action="store_false"
)
parser.add_argument(
    "--no-escts", dest="escts", help="Disable GLES CTS tests", action="store_false"
)
parser.add_argument(
    "--no-deqp", dest="deqp", help="Disable dEQP tests", action="store_false"
)
parser.add_argument(
    "--slow", dest="slow", help="Include slowest glcts tests", action="store_true"
)
parser.add_argument(
    "--no-deqp-egl",
    dest="deqp_egl",
    help="Disable dEQP-EGL tests",
    action="store_false",
)
parser.add_argument(
    "--no-deqp-gles2",
    dest="deqp_gles2",
    help="Disable dEQP-gles2 tests",
    action="store_false",
)
parser.add_argument(
    "--no-deqp-gles3",
    dest="deqp_gles3",
    help="Disable dEQP-gles3 tests",
    action="store_false",
)
parser.add_argument(
    "--no-deqp-gles31",
    dest="deqp_gles31",
    help="Disable dEQP-gles31 tests",
    action="store_false",
)
parser.set_defaults(piglit=True)
parser.set_defaults(glcts=True)
parser.set_defaults(escts=True)
parser.set_defaults(deqp=True)
parser.set_defaults(deqp_egl=True)
parser.set_defaults(deqp_gles2=True)
parser.set_defaults(deqp_gles3=True)
parser.set_defaults(deqp_gles31=True)
parser.set_defaults(slow=False)

parser.add_argument(
    "output_folder",
    nargs="?",
    help="Output folder (logs, etc)",
    default=os.path.join(
        # Default is ../../../../../../test-results/datetime
        os.path.join(path_above_mesa, 'test-results',
                     datetime.now().strftime("%Y-%m-%d-%H-%M-%S"))
    ),
)

available_gpus = []
for f in os.listdir("/dev/dri/by-path"):
    idx = f.find("-render")
    if idx < 0:
        continue
    # gbm name is the full path, but DRI_PRIME expects a different
    # format
    available_gpus += [
        (
            os.path.join("/dev/dri/by-path", f),
            f[:idx].replace(":", "_").replace(".", "_"),
        )
    ]

parser.add_argument(
    "--gpu",
    type=int,
    dest="gpu",
    default=0,
    help="Select GPU (0..{})".format(len(available_gpus) - 1),
)

args = parser.parse_args(sys.argv[1:])
piglit_path = args.piglit_path
glcts_path = args.glcts_path
escts_path = args.escts_path
deqp_path = args.deqp_path

if args.parent_path:
    if args.piglit_path or args.glcts_path or args.deqp_path:
        parser.print_help()
        sys.exit(0)
    piglit_path = os.path.join(args.parent_path, "piglit")
    glcts_path = os.path.join(args.parent_path, "glcts")
    escts_path = os.path.join(args.parent_path, "escts")
    deqp_path = os.path.join(args.parent_path, "deqp")
else:
    if not args.piglit_path or not args.glcts_path or not args.escts_path or not args.deqp_path:
        parser.print_help()
        sys.exit(0)

base = args.baseline
skips = os.path.join(os.path.dirname(__file__), "skips.csv")

env = os.environ.copy()

if "DISPLAY" not in env:
    print_red("DISPLAY environment variable missing.")
    sys.exit(1)
p = subprocess.run(
    ["deqp-runner", "--version"], capture_output="True", check=True, env=env
)
for line in p.stdout.decode().split("\n"):
    if line.find("deqp-runner") >= 0:
        s = line.split(" ")[1].split(".")
        if args.verbose > 1:
            print("Checking deqp-version ({})".format(s))
        # We want at least 0.9.0
        if not (int(s[0]) > 0 or int(s[1]) >= 9):
            print("Expecting deqp-runner 0.9.0+ version (got {})".format(".".join(s)))
            sys.exit(1)

env["PIGLIT_PLATFORM"] = "gbm"

if "DRI_PRIME" in env:
    print("Don't use DRI_PRIME. Instead use --gpu N")
    del env["DRI_PRIME"]

assert "gpu" in args, "--gpu defaults to 0"

gpu_device = available_gpus[args.gpu][1]
env["DRI_PRIME"] = gpu_device
env["WAFFLE_GBM_DEVICE"] = available_gpus[args.gpu][0]

# Use piglit's glinfo to determine the GPU name
gpu_name = "unknown"
gpu_name_full = ""
gfx_level = -1

env["AMD_DEBUG"] = "info"
p = subprocess.run(
    ["./glinfo"],
    capture_output="True",
    cwd=os.path.join(piglit_path, "bin"),
    check=True,
    env=env,
)
del env["AMD_DEBUG"]
for line in p.stdout.decode().split("\n"):
    if "GL_RENDER" in line:
        line = line.split("=")[1]
        gpu_name_full = "(".join(line.split("(")[:-1]).strip()
        gpu_name = line.replace("(TM)", "").split("(")[1].split(",")[0].lower()
        break
    elif "gfx_level" in line:
        gfx_level = int(line.split("=")[1])

output_folder = args.output_folder
print_green("Tested GPU: '{}' ({}) {}".format(gpu_name_full, gpu_name, gpu_device))
print_green("Output folder: '{}'".format(output_folder))

count = 1
while os.path.exists(output_folder):
    output_folder = "{}.{}".format(os.path.abspath(args.output_folder), count)
    count += 1

os.makedirs(output_folder, exist_ok=True)

logfile = open(os.path.join(output_folder, "{}-run-tests.log".format(gpu_name)), "w")

spin = itertools.cycle("-\\|/")

shutil.copy(skips, output_folder)
skips = os.path.join(output_folder, "skips.csv")
if not args.slow:
    # Exclude these 4 tests slow tests
    with open(skips, "a") as f:
        print("KHR-GL46.copy_image.functional", file=f)
        print("KHR-GL46.texture_swizzle.smoke", file=f)
        print(
            "KHR-GL46.tessellation_shader.tessellation_control_to_tessellation_evaluation.gl_MaxPatchVertices_Position_PointSize",
            file=f,
        )
        print("KHR-Single-GL46.arrays_of_arrays_gl.AtomicUsage", file=f)


def gfx_level_to_str(cl):
    supported = ["gfx6", "gfx7", "gfx8", "gfx9", "gfx10", "gfx10_3", "gfx11"]
    if 8 <= cl and cl < 8 + len(supported):
        return supported[cl - 8]
    return supported[-1]


def run_cmd(args, verbosity):
    if verbosity > 1:
        print_yellow(
            "| Command line argument '"
            + " ".join(['"{}"'.format(a) for a in args])
            + "'"
        )
    start = datetime.now()
    proc = subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env
    )
    while True:
        line = proc.stdout.readline().decode()
        if verbosity > 0:
            if "ERROR" in line:
                print_red(line.strip(), prefix="| ")
            else:
                print("| " + line.strip())
        else:
            sys.stdout.write(next(spin))
            sys.stdout.flush()
            sys.stdout.write("\b")

        logfile.write(line)

        if proc.poll() is not None:
            break
    proc.wait()
    end = datetime.now()

    if verbosity == 0:
        sys.stdout.write(" ... ")

    print_yellow(
        "Completed in {} seconds".format(int((end - start).total_seconds())),
        prefix="└ " if verbosity > 0 else None,
    )


def verify_results(results):
    with open(results) as file:
        if len(file.readlines()) == 0:
            return True
    print_red("New results (fails or pass). Check {}".format(results))
    return False


def parse_test_filters(include_tests):
    cmd = []
    for t in include_tests:
        if os.path.exists(t):
            with open(t, "r") as file:
                for row in csv.reader(file, delimiter=","):
                    if not row or row[0][0] == "#":
                        continue
                    cmd += ["-t", row[0]]
        else:
            cmd += ["-t", t]
    return cmd


def select_baseline(basepath, gfx_level, gpu_name):
    gfx_level_str = gfx_level_to_str(gfx_level)

    # select the best baseline we can find
    # 1. exact match
    exact = os.path.join(base, "{}-{}-fail.csv".format(gfx_level_str, gpu_name))
    if os.path.exists(exact):
        return exact
    # 2. any baseline with the same gfx_level
    while gfx_level >= 8:
        for subdir, dirs, files in os.walk(basepath):
            for file in files:
                if file.find(gfx_level_str) == 0 and file.endswith("-fail.csv"):
                    return os.path.join(base, file)
        # No match. Try an earlier class
        gfx_level = gfx_level - 1
        gfx_level_str = gfx_level_to_str(gfx_level)

    return exact


success = True
filters_args = parse_test_filters(args.include_tests)
baseline = select_baseline(base, gfx_level, gpu_name)
flakes = [
    f
    for f in (
        os.path.join(base, g)
        for g in [
            "radeonsi-flakes.csv",
            "{}-{}-flakes.csv".format(gfx_level_to_str(gfx_level), gpu_name),
        ]
    )
    if os.path.exists(f)
]
flakes_args = []
for f in flakes:
    flakes_args += ["--flakes", f]

if os.path.exists(baseline):
    print_yellow("Baseline: {}".format(baseline))
if flakes_args:
    print_yellow("Flakes: {}".format(flakes_args))

# piglit test
if args.piglit:
    out = os.path.join(output_folder, "piglit")
    print_yellow("Running piglit tests", args.verbose > 0)
    cmd = [
        "piglit-runner",
        "run",
        "--piglit-folder",
        piglit_path,
        "--profile",
        "quick",
        "--output",
        out,
        "--process-isolation",
        "--timeout",
        "300",
        "--jobs",
        str(args.jobs),
        "--skips",
        skips,
        "--skips",
        os.path.join(path_above_mesa, "mesa", ".gitlab-ci", "gbm-skips.txt")
    ] + filters_args + flakes_args

    if os.path.exists(baseline):
        cmd += ["--baseline", baseline]

    run_cmd(cmd, args.verbose)

    if not verify_results(os.path.join(out, "failures.csv")):
        success = False

deqp_args = "-- --deqp-surface-width=256 --deqp-surface-height=256 --deqp-gl-config-name=rgba8888d24s8ms0 --deqp-visibility=hidden".split(
    " "
)

# glcts test
if args.glcts:
    out = os.path.join(output_folder, "glcts")
    print_yellow("Running  GLCTS tests", args.verbose > 0)
    os.mkdir(os.path.join(output_folder, "glcts"))

    cmd = [
        "deqp-runner",
        "run",
        "--tests-per-group",
        "100",
        "--deqp",
        "{}/build/external/openglcts/modules/glcts".format(glcts_path),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gl/khronos_mustpass/4.6.1.x/gl46-master.txt".format(
            glcts_path
        ),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gl/khronos_mustpass_single/4.6.1.x/gl46-khr-single.txt".format(
            glcts_path
        ),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gl/khronos_mustpass/4.6.1.x/gl46-gtf-master.txt".format(
            glcts_path
        ),
        "--output",
        out,
        "--skips",
        skips,
        "--jobs",
        str(args.jobs),
        "--timeout",
        "1000"
    ] + filters_args + flakes_args

    if os.path.exists(baseline):
        cmd += ["--baseline", baseline]
    cmd += deqp_args

    run_cmd(cmd, args.verbose)

    if not verify_results(os.path.join(out, "failures.csv")):
        success = False

# escts test
if args.escts:
    out = os.path.join(output_folder, "escts")
    print_yellow("Running  ESCTS tests", args.verbose > 0)
    os.mkdir(out)

    cmd = [
        "deqp-runner",
        "run",
        "--tests-per-group",
        "100",
        "--deqp",
        "{}/build/external/openglcts/modules/glcts".format(escts_path),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gles/khronos_mustpass/3.2.6.x/gles2-khr-master.txt".format(
            escts_path
        ),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gles/khronos_mustpass/3.2.6.x/gles3-khr-master.txt".format(
            escts_path
        ),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gles/khronos_mustpass/3.2.6.x/gles31-khr-master.txt".format(
            escts_path
        ),
        "--caselist",
        "{}/build/external/openglcts/modules/gl_cts/data/mustpass/gles/khronos_mustpass/3.2.6.x/gles32-khr-master.txt".format(
            escts_path
        ),
        "--output",
        out,
        "--skips",
        skips,
        "--jobs",
        str(args.jobs),
        "--timeout",
        "1000"
    ] + filters_args + flakes_args

    if os.path.exists(baseline):
        cmd += ["--baseline", baseline]
    cmd += deqp_args

    run_cmd(cmd, args.verbose)

    if not verify_results(os.path.join(out, "failures.csv")):
        success = False

if args.deqp:
    print_yellow("Running   dEQP tests", args.verbose > 0)

    # Generate a test-suite file
    out = os.path.join(output_folder, "deqp")
    suite_filename = os.path.join(output_folder, "deqp-suite.toml")
    suite = open(suite_filename, "w")
    os.mkdir(out)

    deqp_tests = {
        "egl": args.deqp_egl,
        "gles2": args.deqp_gles2,
        "gles3": args.deqp_gles3,
        "gles31": args.deqp_gles31,
    }

    for k in deqp_tests:
        if not deqp_tests[k]:
            continue

        suite.write("[[deqp]]\n")
        suite.write(
            'deqp = "{}"\n'.format(
                "{}/build/modules/{subtest}/deqp-{subtest}".format(deqp_path, subtest=k)
            )
        )
        suite.write(
            'caselists = ["{}"]\n'.format(
                "{}/android/cts/main/{}-master.txt".format(deqp_path, k)
            )
        )
        if os.path.exists(baseline):
            suite.write('baseline = "{}"\n'.format(baseline))
        suite.write('skips = ["{}"]\n'.format(skips))
        suite.write("deqp_args = [\n")
        for a in deqp_args[1:-1]:
            suite.write('    "{}",\n'.format(a))
        suite.write('    "{}"\n'.format(deqp_args[-1]))
        suite.write("]\n")

    suite.close()

    cmd = [
        "deqp-runner",
        "suite",
        "--jobs",
        str(args.jobs),
        "--output",
        os.path.join(output_folder, "deqp"),
        "--suite",
        suite_filename,
    ] + filters_args + flakes_args

    run_cmd(cmd, args.verbose)

    if not verify_results(os.path.join(out, "failures.csv")):
        success = False

sys.exit(0 if success else 1)
