#!/usr/bin/env python3
# coding=utf-8
##########################################################################
#
# enums2names - Parse and convert enums to translator code
# (C) Copyright 2021 Matti 'ccr' Hämäläinen <ccr@tnsp.org>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sub license, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice (including the
# next paragraph) shall be included in all copies or substantial portions
# of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
# IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##########################################################################

import sys
import os.path
import re
import signal
import argparse
import textwrap

assert sys.version_info >= (3, 6)


#
# List of enums we wish to include in output.
# NOTE: This needs to be updated if such enums are added.
#
lst_enum_include = [
    "pipe_texture_target",
    "pipe_shader_cap",
    "pipe_shader_ir",
    "pipe_map_flags",
    "pipe_cap",
    "pipe_capf",
    "pipe_compute_cap",
    "pipe_video_cap",
    "pipe_video_profile",
    "pipe_video_entrypoint",
    "pipe_video_vpp_orientation",
    "pipe_video_vpp_blend_mode",
    "pipe_resource_param",
    "pipe_fd_type",
    "pipe_blendfactor",
    "pipe_blend_func",
    "pipe_logicop",
]


###
### Utility functions
###
## Fatal error handler
def pkk_fatal(smsg):
    print("ERROR: "+ smsg)
    sys.exit(1)


## Handler for SIGINT signals
def pkk_signal_handler(signal, frame):
    print("\nQuitting due to SIGINT / Ctrl+C!")
    sys.exit(1)


## Argument parser subclass
class PKKArgumentParser(argparse.ArgumentParser):
    def print_help(self):
        print("enums2names - Parse and convert enums to translator code\n"
        "(C) Copyright 2021 Matti 'ccr' Hämäläinen <ccr@tnsp.org>\n")
        super().print_help()

    def error(self, msg):
        self.print_help()
        print(f"\nERROR: {msg}", file=sys.stderr)
        sys.exit(2)


def pkk_get_argparser():
    optparser = PKKArgumentParser(
        usage="%(prog)s [options] <infile|->\n"
        "example: %(prog)s ../../include/pipe/p_defines.h -C tr_util.c -H tr_util.h"
        )

    optparser.add_argument("in_files",
        type=str,
        metavar="infiles",
        nargs="+",
        help="path to input header files (or '-' for stdin)")

    optparser.add_argument("-C",
        type=str,
        metavar="outfile",
        dest="out_source",
        help="output C source file")

    optparser.add_argument("-H",
        type=str,
        metavar="outfile",
        dest="out_header",
        help="output C header file")

    optparser.add_argument("-I",
        type=str,
        metavar="include",
        dest="include_file",
        help="include file / path used for C source output")

    return optparser


class PKKHeaderParser:

    def __init__(self, nfilename):
        self.filename = nfilename
        self.enums = {}
        self.state = 0
        self.nline = 0
        self.mdata = []
        self.start = 0
        self.name = None
        self.in_multiline_comment = False

    def error(self, msg):
        pkk_fatal(f"{self.filename}:{self.nline} : {msg}")

    def parse_line(self, sline: str):
        start = sline.find('/*')
        end = sline.find('*/')
        if not self.in_multiline_comment and start >= 0:
            if end >= 0:
                assert end > start
                sline = sline[:start] + sline[end + 2:]
            else:
                sline = sline[:start]
                self.in_multiline_comment = True
        elif self.in_multiline_comment and end >= 0:
            self.in_multiline_comment = False
            sline = sline[end + 2:]
        elif self.in_multiline_comment:
            return
        # A kingdom for Py3.8 := operator ...
        smatch = re.match(r'^enum\s+([A-Za-z0-9_]+)\s+.*;', sline)
        if smatch:
            pass
        else:
            smatch = re.match(r'^enum\s+([A-Za-z0-9_]+)', sline)
            if smatch:
                stmp = smatch.group(1)

                if self.state != 0:
                    self.error(f"enum '{stmp}' starting inside another enum '{self.name}'")

                self.name = stmp
                self.state = 1
                self.start = self.nline
                self.mdata = []
            else:
                smatch = re.match(r'^}(\s*|\s*[A-Z][A-Z_]+\s*);', sline)
                if smatch:
                    if self.state == 1:
                        if self.name in self.enums:
                            self.error("duplicate enum definition '{}', lines {} - {} vs {} - {}".format(
                            self.name, self.enums[self.name]["start"], self.enums[self.name]["end"],
                            self.start, self.nline))

                        self.enums[self.name] = {
                            "data": self.mdata,
                            "start": self.start,
                            "end": self.nline
                        }

                    self.state = 0

                elif self.state == 1:
                    smatch = re.match(r'([A-Za-z0-9_]+)\s*=\s*(.+)\s*,?', sline)
                    if smatch:
                        self.mdata.append(smatch.group(1))
                    else:
                        smatch = re.match(r'([A-Za-z0-9_]+)\s*,?', sline)
                        if smatch:
                            self.mdata.append(smatch.group(1))

    def parse_file(self, fh):
        self.nline = 0
        for line in fh:
            self.nline += 1
            self.parse_line(line.strip())

        return self.enums


def pkk_output_header(fh):
    prototypes = [f"const char *\n"
        f"tr_util_{name}_name(enum {name} value);\n" for name in lst_enum_include]

    print(textwrap.dedent("""\
        /*
         * File generated with {program}, please do not edit manually.
         */
        #ifndef {include_header_guard}
        #define {include_header_guard}


        #include "pipe/p_defines.h"
        #include "pipe/p_video_enums.h"


        #ifdef __cplusplus
        extern "C" {{
        #endif

        {prototypes}

        #ifdef __cplusplus
        }}
        #endif

        #endif /* {include_header_guard} */\
        """).format(
            program=pkk_progname,
            include_header_guard=re.sub(r'[^A-Z]', '_', os.path.basename(pkk_cfg.out_header).upper()),
            prototypes="".join(prototypes)
            ), file=fh)


def pkk_output_source(fh):
    if pkk_cfg.include_file == None:
        pkk_fatal("Output C source enabled, but include file is not set (-I option).")

    print(textwrap.dedent("""\
        /*
         * File generated with {program}, please do not edit manually.
         */
        #include "{include_file}"
        """).format(
            program=pkk_progname,
            include_file=pkk_cfg.include_file,
            ), file=fh)

    for name in lst_enum_include:
        cases = [f"      case {eid}: return \"{eid}\";\n"
            for eid in enums[name]["data"]]

        print(textwrap.dedent("""\

            const char *
            tr_util_{name}_name(enum {name} value)
            {{
               switch (value) {{
            {cases}
                  default: return "{ucname}_UNKNOWN";
               }}
            }}
            """).format(
                name=name,
                ucname=name.upper(),
                cases="".join(cases)
                ), file=fh)

###
### Main program starts
###
if __name__ == "__main__":
    signal.signal(signal.SIGINT, pkk_signal_handler)

    ### Parse arguments
    pkk_progname = sys.argv[0]
    optparser = pkk_get_argparser()
    pkk_cfg = optparser.parse_args()

    ### Parse input files
    enums = {}
    for file in pkk_cfg.in_files:
        hdrparser = PKKHeaderParser(file)

        try:
            if file != "-":
                with open(file, "r", encoding="UTF-8") as fh:
                    enums.update(hdrparser.parse_file(fh))
            else:
                enums.update(hdrparser.parse_file(sys.stdin))
                break
        except OSError as e:
            pkk_fatal(str(e))

    ### Check if any of the required enums are missing
    errors = False
    for name in lst_enum_include:
        if name not in enums:
            print(f"ERROR: Missing enum '{name}'!")
            errors = True

    if errors:
        pkk_fatal(f"Errors in input. Edit this script ({pkk_progname}) to add/remove included enums.")

    ### Perform output
    if pkk_cfg.out_header:
        with open(pkk_cfg.out_header, "w", encoding="UTF-8") as fh:
            pkk_output_header(fh)

    if pkk_cfg.out_source:
        with open(pkk_cfg.out_source, "w", encoding="UTF-8") as fh:
            pkk_output_source(fh)
