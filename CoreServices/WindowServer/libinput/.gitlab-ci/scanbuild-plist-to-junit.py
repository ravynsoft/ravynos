#!/usr/bin/env python3
#
# SPDX-License-Identifier: MIT
#
# Usage:
#   $ scanbuild-plist-to-junit.py /path/to/meson-logs/scanbuild/ > junit-report.xml
#
# Converts the plist output from scan-build into a JUnit-compatible XML.
#
# For use with meson, use a wrapper script with this content:
#   scan-build -v --status-bugs -plist-html "$@"
# then build with
#  SCANBUILD="/abs/path/to/wrapper.sh" ninja -C builddir scan-build
#
# For file context, $PWD has to be the root source directory.
#
# Note that the XML format is tailored towards being useful in the gitlab
# CI, the JUnit format supports more features.
#
# This file is formatted with Python Black

import argparse
import plistlib
import re
import sys
from pathlib import Path

errors = []


class Error(object):
    pass


parser = argparse.ArgumentParser(
    description="This tool convers scan-build's plist format to JUnit XML"
)
parser.add_argument(
    "directory", help="Path to a scan-build output directory", type=Path
)
args = parser.parse_args()

if not args.directory.exists():
    print(f"Invalid directory: {args.directory}", file=sys.stderr)
    sys.exit(1)

# Meson places scan-build runs into a timestamped directory. To make it
# easier to invoke this script, we just glob everything on the assumption
# that there's only one scanbuild/$timestamp/ directory anyway.
for file in Path(args.directory).glob("**/*.plist"):
    with open(file, "rb") as fd:
        plist = plistlib.load(fd, fmt=plistlib.FMT_XML)
        try:
            sources = plist["files"]
            for elem in plist["diagnostics"]:
                e = Error()
                e.type = elem["type"]  # Human-readable error type
                e.description = elem["description"]  # Longer description
                e.func = elem["issue_context"]  # function name
                e.lineno = elem["location"]["line"]
                filename = sources[elem["location"]["file"]]
                # Remove the ../../../ prefix from the file
                e.file = re.sub(r"^(\.\./)*", "", filename)
                errors.append(e)
        except KeyError:
            print(
                "Failed to access plist content, incompatible format?", file=sys.stderr
            )
            sys.exit(1)


# Add a few lines of context for each error that we can print in the xml
# output. Note that e.lineno is 1-indexed.
#
# If one of the files fail, we stop doing this, we're probably in the wrong
# directory.
try:
    current_file = None
    lines = []
    for e in sorted(errors, key=lambda x: x.file):
        if current_file != e.file:
            current_file = e.file
            lines = open(current_file).readlines()

        # e.lineno is 1-indexed, lineno is our 0-indexed line number
        lineno = e.lineno - 1
        start = max(0, lineno - 4)
        end = min(len(lines), lineno + 5)  # end is exclusive
        e.context = [
            f"{'>' if line == e.lineno else ' '} {line}: {content}"
            for line, content in zip(range(start + 1, end), lines[start:end])
        ]
except FileNotFoundError:
    pass

print('<?xml version="1.0" encoding="utf-8"?>')
print("<testsuites>")
if errors:
    suites = sorted(set([s.type for s in errors]))
    # Use a counter to ensure test names are unique, otherwise the CI
    # display ignores duplicates.
    counter = 0
    for suite in suites:
        errs = [e for e in errors if e.type == suite]
        # Note: the grouping by suites doesn't actually do anything in gitlab. Oh well
        print(f'<testsuite name="{suite}" failures="{len(errs)}" tests="{len(errs)}">')
        for error in errs:
            print(
                f"""\
<testcase name="{counter}. {error.type} - {error.file}:{error.lineno}" classname="{error.file}">
<failure message="{error.description}">
<![CDATA[
In function {error.func}(),
{error.description}

{error.file}:{error.lineno}
---
{"".join(error.context)}
]]>
</failure>
</testcase>"""
            )
            counter += 1
        print("</testsuite>")
else:
    # In case of success, add one test case so that registers in the UI
    # properly
    print('<testsuite name="scanbuild" failures="0" tests="1">')
    print('<testcase name="scanbuild" classname="scanbuild"/>')
    print("</testsuite>")
print("</testsuites>")
