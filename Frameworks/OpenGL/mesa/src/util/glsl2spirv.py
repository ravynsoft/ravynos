# Copyright © 2022 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# Converts GLSL shader to SPIR-V library 

import argparse
import subprocess
import sys
import os
import typing as T

if T.TYPE_CHECKING:
    class Arguments(T.Protocol):
        input: str
        output: str
        glslang: str
        create_entry: T.Optional[str]
        glsl_ver: T.Optional[str]
        Olib: bool
        extra: T.Optional[str]
        vn: str
        stage: str


def get_args() -> 'Arguments':
    parser = argparse.ArgumentParser()
    parser.add_argument('input', help="Name of input file.")
    parser.add_argument('output', help="Name of output file.")
    parser.add_argument('glslang', help="path to glslangValidator")

    parser.add_argument("--create-entry",
                        dest="create_entry",
                        help="Create a new entry point and put to the end of a file.")

    parser.add_argument('--glsl-version',
                        dest="glsl_ver",
                        choices=['100', '110', '120', '130', '140', '150', '300es', '310es', '330', '400', '410', '420', '430', '440', '450', '460'],
                        help="Override GLSL #version declaration in source.")

    parser.add_argument("-Olib",
                        action='store_true',
                        help="Any optimizations are disabled and unused functions are saved.")

    parser.add_argument("--extra-flags",
                        dest="extra",
                        help="Pass additional flags to glslangValidator.")

    parser.add_argument("--vn",
                        dest="vn",
                        required=True,
                        help="Variable name. Creates a C header file that contains a uint32_t array.")

    parser.add_argument("--stage",
                        default="vert",
                        choices=['vert', 'tesc', 'tese', 'geom', 'frag', 'comp'],
                        help="Uses specified stage rather than parsing the file extension")

    parser.add_argument("-I",
                        dest="includes",
                        default=[],
                        action='append',
                        help="Include directory")

    parser.add_argument("-D",
                        dest="defines",
                        default=[],
                        action='append',
                        help="Defines")

    args = parser.parse_args()
    return args


def create_include_guard(lines: T.List[str], filename: str) -> T.List[str]:
    filename = filename.replace('.', '_')
    upper_name = filename.upper()

    guard_head = [f"#ifndef {upper_name}\n",
                  f"#define {upper_name}\n"]
    guard_tail = [f"\n#endif // {upper_name}\n"]

    # remove '#pragma once'
    for idx, l in enumerate(lines):
        if '#pragma once' in l:
            lines.pop(idx)
            break

    return guard_head + lines + guard_tail


def convert_to_static_variable(lines: T.List[str], varname: str) -> T.List[str]:
    for idx, l in enumerate(lines):
        if varname in l:
            lines[idx] = f"static {l}"
            return lines
    raise RuntimeError(f'Did not find {varname}, this is unexpected')


def override_version(lines: T.List[str], glsl_version: str) -> T.List[str]:
    for idx, l in enumerate(lines):
        if '#version ' in l:
            lines[idx] = f"#version {glsl_version}\n"
            return lines
    raise RuntimeError('Did not find #version directive, this is unexpected')


def postprocess_file(args: 'Arguments') -> None:
    with open(args.output, "r") as r:
        lines = r.readlines()

    # glslangValidator creates a variable without the static modifier.
    lines = convert_to_static_variable(lines, args.vn)

    # '#pragma once' is unstandardised.
    lines = create_include_guard(lines, os.path.basename(r.name))

    with open(args.output, "w") as w:
        w.writelines(lines)


def preprocess_file(args: 'Arguments', origin_file: T.TextIO, directory: os.PathLike) -> str:
    with open(os.path.join(directory, os.path.basename(origin_file.name)), "w") as copy_file:
        lines = origin_file.readlines()

        if args.create_entry is not None:
            lines.append(f"\nvoid {args.create_entry}() {{}}\n")

        if args.glsl_ver is not None:
            override_version(lines, args.glsl_ver)

        copy_file.writelines(lines)

    return copy_file.name


def process_file(args: 'Arguments') -> None:
    with open(args.input, "r") as infile:
        copy_file = preprocess_file(args, infile,
                                    os.path.dirname(args.output))

    cmd_list = [args.glslang]

    if args.Olib:
        cmd_list.append("--keep-uncalled")

    if args.vn is not None:
        cmd_list.extend(["--variable-name", args.vn])

    if args.extra is not None:
        cmd_list.append(args.extra)

    if args.create_entry is not None:
        cmd_list.extend(["--entry-point", args.create_entry])

    for f in args.includes:
        cmd_list.append('-I' + f)

    for d in args.defines:
        cmd_list.append('-D' + d)

    cmd_list.extend([
        '-V',
        '-o', args.output,
        '-S', args.stage,
        copy_file,
    ])

    ret = subprocess.run(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
    if ret.returncode != 0:
        print(ret.stdout)
        print(ret.stderr, file=sys.stderr)
        sys.exit(1)

    if args.vn is not None:
        postprocess_file(args)

    if args.create_entry is not None:
        os.remove(copy_file)


def main() -> None:
    args = get_args()
    process_file(args)


if __name__ == "__main__":
    main()
