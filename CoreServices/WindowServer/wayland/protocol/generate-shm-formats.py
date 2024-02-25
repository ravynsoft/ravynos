#!/usr/bin/env python3

# This script synchronizes wayland.xml's wl_shm.format enum with drm_fourcc.h.
# Invoke it to update wayland.xml, then manually check the changes applied.
#
# Requires Python 3, python-lxml, a C compiler and pkg-config.

import os
import subprocess
import sys
import tempfile
# We need lxml instead of the standard library because we want
# Element.sourceline
from lxml import etree as ElementTree

proto_dir = os.path.dirname(os.path.realpath(__file__))
wayland_proto = proto_dir + "/wayland.xml"

cc = os.getenv("CC", "cc")
pkg_config = os.getenv("PKG_CONFIG", "pkg-config")

# Find drm_fourcc.h
version = subprocess.check_output([pkg_config, "libdrm",
    "--modversion"]).decode().strip()
cflags = subprocess.check_output([pkg_config, "libdrm",
    "--cflags-only-I"]).decode().strip().split()
libdrm_include = None
for include_flag in cflags:
    if not include_flag.startswith("-I"):
        raise Exception("Expected one include dir for libdrm")
    include_dir = include_flag[2:]
    if include_dir.endswith("/libdrm"):
        libdrm_include = include_dir
        fourcc_include = libdrm_include + "/drm_fourcc.h"
if libdrm_include == None:
    raise Exception("Failed to find libdrm include dir")

print("Using libdrm " + version, file=sys.stderr)

def drm_format_to_wl(ident):
    return ident.replace("DRM_FORMAT_", "").lower()

# Collect DRM format constant names
ident_list = []
descriptions = {}
prev_comment = None
with open(fourcc_include) as input_file:
    for l in input_file.readlines():
        l = l.strip()

        # Collect comments right before format definitions
        if l.startswith("/*") and l.endswith("*/"):
            prev_comment = l[2:-2]
            continue
        desc = prev_comment
        prev_comment = None

        # Recognize format definitions
        parts = l.split()
        if len(parts) < 3 or parts[0] != "#define":
            continue
        ident = parts[1]
        if not ident.startswith("DRM_FORMAT_") or ident.startswith(
                "DRM_FORMAT_MOD_"):
            continue

        ident_list.append(ident)

        # Prefer in-line comments
        if l.endswith("*/"):
            desc = l[l.rfind("/*") + 2:-2]
        if desc != None:
            descriptions[drm_format_to_wl(ident)] = desc.strip()

# Collect DRM format values
idents = {}
with tempfile.TemporaryDirectory() as work_dir:
    c_file_name = work_dir + "/print-formats.c"
    exe_file_name = work_dir + "/print-formats"

    with open(c_file_name, "w+") as c_file:
        c_file.write('#include <inttypes.h>\n')
        c_file.write('#include <stdint.h>\n')
        c_file.write('#include <stdio.h>\n')
        c_file.write('#include <drm_fourcc.h>\n')
        c_file.write('\n')
        c_file.write('int main(void) {\n')
        for ident in ident_list:
            c_file.write('printf("0x%" PRIX64 "\\n", (uint64_t)' + ident + ');\n')
        c_file.write('}\n')

    subprocess.check_call([cc, "-Wall", "-Wextra", "-o", exe_file_name,
        c_file_name] + cflags)
    output = subprocess.check_output([exe_file_name]).decode().strip()
    for i, val in enumerate(output.splitlines()):
        idents[ident_list[i]] = val

# We don't need those
del idents["DRM_FORMAT_BIG_ENDIAN"]
del idents["DRM_FORMAT_INVALID"]
del idents["DRM_FORMAT_RESERVED"]

# Convert from DRM constants to Wayland wl_shm.format entries
formats = {}
for ident, val in idents.items():
    formats[drm_format_to_wl(ident)] = val.lower()
# Special case for ARGB8888 and XRGB8888
formats["argb8888"] = "0"
formats["xrgb8888"] = "1"

print("Loaded {} formats from drm_fourcc.h".format(len(formats)), file=sys.stderr)

tree = ElementTree.parse("wayland.xml")
root = tree.getroot()
wl_shm_format = root.find("./interface[@name='wl_shm']/enum[@name='format']")
if wl_shm_format == None:
    raise Exception("wl_shm.format not found in wayland.xml")

# Remove formats we already know about
last_line = None
for node in wl_shm_format:
    if node.tag != "entry":
        continue
    fmt = node.attrib["name"]
    val = node.attrib["value"]
    if fmt not in formats:
        raise Exception("Format present in wl_shm.formats but not in "
            "drm_fourcc.h: " + fmt)
    if val != formats[fmt]:
        raise Exception("Format value in wl_shm.formats ({}) differs "
            "from value in drm_fourcc.h ({}) for format {}"
            .format(val, formats[fmt], fmt))
    del formats[fmt]
    last_line = node.sourceline
if last_line == None:
    raise Exception("Expected at least one existing wl_shm.format entry")

print("Adding {} formats to wayland.xml...".format(len(formats)), file=sys.stderr)

# Append new formats
new_wayland_proto = wayland_proto + ".new"
with open(new_wayland_proto, "w+") as output_file, \
        open(wayland_proto) as input_file:
    for i, l in enumerate(input_file.readlines()):
        output_file.write(l)
        if i + 1 == last_line:
            for fmt, val in formats.items():
                output_file.write('      <entry name="{}" value="{}"'
                    .format(fmt, val))
                if fmt in descriptions:
                    output_file.write(' summary="{}"'.format(descriptions[fmt]))
                output_file.write('/>\n')
os.rename(new_wayland_proto, wayland_proto)
