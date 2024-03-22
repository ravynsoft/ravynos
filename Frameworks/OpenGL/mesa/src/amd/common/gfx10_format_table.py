#
# Copyright 2017 Advanced Micro Devices, Inc.
#
# SPDX-License-Identifier: MIT
#
"""
Script that generates the mapping from Gallium PIPE_FORMAT_xxx to GFX10_FORMAT_xxx enums.
"""

import json
import mako.template
import os
import re
import sys

AMD_REGISTERS = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), "../registers"))
UTIL_FORMAT = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), "../../util/format"))
sys.path.extend([AMD_REGISTERS, UTIL_FORMAT])

from regdb import Object, RegisterDatabase
from u_format_parse import *

# ----------------------------------------------------------------------------
# Hard-coded mappings

def hardcoded_format(hw_enum):
    return Object(img_format=hw_enum, flags=[])

HARDCODED = {
    'PIPE_FORMAT_Z32_FLOAT_S8X24_UINT': hardcoded_format('X24_8_32_FLOAT'),
    'PIPE_FORMAT_Z24_UNORM_S8_UINT': hardcoded_format('8_24_UNORM'),
    'PIPE_FORMAT_S8_UINT_Z24_UNORM': hardcoded_format('24_8_UNORM'),
    'PIPE_FORMAT_Z32_UNORM': None,
    'PIPE_FORMAT_Z16_UNORM_S8_UINT': None,

    'PIPE_FORMAT_R9G9B9E5_FLOAT': hardcoded_format('5_9_9_9_FLOAT'),
    'PIPE_FORMAT_R11G11B10_FLOAT': hardcoded_format('10_11_11_FLOAT'), # NOTE: full set of int/unorm/etc. exists

    'PIPE_FORMAT_R8G8_B8G8_UNORM': hardcoded_format('GB_GR_UNORM'),
    'PIPE_FORMAT_G8R8_B8R8_UNORM': hardcoded_format('GB_GR_UNORM'),

    'PIPE_FORMAT_R8G8_R8B8_UNORM': hardcoded_format('BG_RG_UNORM'),
    'PIPE_FORMAT_G8R8_G8B8_UNORM': hardcoded_format('BG_RG_UNORM'),

    # These mixed channel types are not supported natively
    'PIPE_FORMAT_R8SG8SB8UX8U_NORM': None,
    'PIPE_FORMAT_R10SG10SB10SA2U_NORM': None,
    'PIPE_FORMAT_R5SG5SB6U_NORM': None,

    # Only R8G8_SRGB is supported, not L8A8_SRGB
    'PIPE_FORMAT_L8A8_SRGB': None,

    # S3TC
    'PIPE_FORMAT_DXT1_RGB': hardcoded_format('BC1_UNORM'),
    'PIPE_FORMAT_DXT1_RGBA': hardcoded_format('BC1_UNORM'),
    'PIPE_FORMAT_DXT1_SRGB': hardcoded_format('BC1_SRGB'),
    'PIPE_FORMAT_DXT1_SRGBA': hardcoded_format('BC1_SRGB'),
    'PIPE_FORMAT_DXT3_RGBA': hardcoded_format('BC2_UNORM'),
    'PIPE_FORMAT_DXT3_SRGBA': hardcoded_format('BC2_SRGB'),
    'PIPE_FORMAT_DXT5_RGBA': hardcoded_format('BC3_UNORM'),
    'PIPE_FORMAT_DXT5_SRGBA': hardcoded_format('BC3_SRGB'),

    # RGTC
    'PIPE_FORMAT_RGTC1_UNORM': hardcoded_format('BC4_UNORM'),
    'PIPE_FORMAT_RGTC1_SNORM': hardcoded_format('BC4_SNORM'),
    'PIPE_FORMAT_RGTC2_UNORM': hardcoded_format('BC5_UNORM'),
    'PIPE_FORMAT_RGTC2_SNORM': hardcoded_format('BC5_SNORM'),
    'PIPE_FORMAT_LATC1_UNORM': hardcoded_format('BC4_UNORM'),
    'PIPE_FORMAT_LATC1_SNORM': hardcoded_format('BC4_SNORM'),
    'PIPE_FORMAT_LATC2_UNORM': hardcoded_format('BC5_UNORM'),
    'PIPE_FORMAT_LATC2_SNORM': hardcoded_format('BC5_SNORM'),

    # BPTC
    'PIPE_FORMAT_BPTC_RGB_UFLOAT': hardcoded_format('BC6_UFLOAT'),
    'PIPE_FORMAT_BPTC_RGB_FLOAT': hardcoded_format('BC6_SFLOAT'),

    'PIPE_FORMAT_BPTC_RGBA_UNORM': hardcoded_format('BC7_UNORM'),
    'PIPE_FORMAT_BPTC_SRGBA': hardcoded_format('BC7_SRGB'),

    'PIPE_FORMAT_R64_UINT': hardcoded_format('32_32_UINT'),
    'PIPE_FORMAT_R64_SINT': hardcoded_format('32_32_SINT'),
}


# ----------------------------------------------------------------------------
# Main script

header_template = mako.template.Template("""\
% if header:
// DO NOT EDIT -- AUTOMATICALLY GENERATED

#include "gfx10_format_table.h"
#include "amdgfxregs.h"

% endif

#define FMT(_img_format, ...) \
   { .img_format = V_008F0C_${gfx.upper()}_FORMAT_##_img_format, \
     ##__VA_ARGS__ }

const struct gfx10_format ${gfx}_format_table[PIPE_FORMAT_COUNT] = {
% for pipe_format, args in formats:
 % if args is not None:
  [${pipe_format}] = FMT(${args}),
 % else:
/* ${pipe_format} is not supported */
 % endif
% endfor

#undef FMT
};
""")

class Gfx10Format(object):
    RE_plain_channel = re.compile(r'X?([0-9]+)')

    def __init__(self, enum_entry):
        self.img_format = enum_entry.name[13:]
        self.flags = getattr(enum_entry, 'flags', [])

        code = self.img_format.split('_')

        self.plain_chan_sizes = []
        for i, chan_code in enumerate(code):
            m = self.RE_plain_channel.match(chan_code)
            if m is None:
                break
            self.plain_chan_sizes.append(int(m.group(1)))
        # Keep the bit sizes in little-endian order
        self.plain_chan_sizes.reverse()

        self.code = code[i:]


class Gfx10FormatMapping(object):
    def __init__(self, pipe_formats, gfx10_formats):
        self.pipe_formats = pipe_formats
        self.gfx10_formats = gfx10_formats

        self.plain_gfx10_formats = dict(
            (tuple(['_'.join(fmt.code)] + fmt.plain_chan_sizes), fmt)
            for fmt in gfx10_formats if fmt.plain_chan_sizes
        )

    def map(self, fmt):
        if fmt.layout == PLAIN:
            chan_type = set([chan.type for chan in fmt.le_channels if chan.type != VOID])
            chan_norm = set([chan.norm for chan in fmt.le_channels if chan.type != VOID])
            chan_pure = set([chan.pure for chan in fmt.le_channels if chan.type != VOID])
            if len(chan_type) > 1 or len(chan_norm) > 1 or len(chan_pure) > 1:
                print(('Format {fmt.name} has inconsistent channel types: ' +
                        '{chan_type} {chan_norm} {chan_pure}')
                      .format(**locals()),
                      file=sys.stderr)
                return None

            chan_type = chan_type.pop()
            chan_norm = chan_norm.pop()
            chan_pure = chan_pure.pop()
            chan_sizes = [chan.size for chan in fmt.le_channels if chan.size != 0]

            extra_flags = []

            if fmt.colorspace == SRGB:
                assert chan_type == UNSIGNED and chan_norm
                num_format = 'SRGB'
            else:
                if chan_type == UNSIGNED:
                    if chan_pure:
                        num_format = 'UINT'
                    elif chan_sizes[0] == 32:
                        # Shader-based work-around for 32-bit non-pure-integer
                        num_format = 'UINT'
                        extra_flags.append('buffers_only')
                    elif chan_norm:
                        num_format = 'UNORM'
                    else:
                        num_format = 'USCALED'
                        extra_flags.append('buffers_only')
                elif chan_type == SIGNED:
                    if chan_pure:
                        num_format = 'SINT'
                    elif chan_sizes[0] == 32:
                        # Shader-based work-around for 32-bit non-pure-integer
                        num_format = 'SINT'
                        extra_flags.append('buffers_only')
                    elif chan_norm:
                        num_format = 'SNORM'
                    else:
                        num_format = 'SSCALED'
                        extra_flags.append('buffers_only')
                elif chan_type == FLOAT:
                    num_format = 'FLOAT'

                    if chan_sizes[0] == 64:
                        # Shader-based work-around for doubles
                        if len(chan_sizes) % 2 == 1:
                            # 1 or 3 loads for 1 or 3 double channels
                            chan_sizes = [32, 32]
                        else:
                            # 1 or 2 loads for 2 or 4 double channels
                            chan_sizes = [32, 32, 32, 32]
                        extra_flags.append('buffers_only')
                else:
                    # Shader-based work-around
                    assert chan_type == FIXED
                    assert chan_sizes[0] == 32
                    num_format = 'SINT'
                    extra_flags.append('buffers_only')

            # These are not supported as render targets, so we don't support
            # them as images either.
            if (len(chan_sizes) == 3 and chan_sizes[0] in (8, 16, 32) and
                chan_sizes[0] == chan_sizes[1]):
                extra_flags.append('buffers_only')
                if chan_sizes[0] in (8, 16):
                    # Shader-based work-around: one load per channel
                    chan_sizes = [chan_sizes[0]]

            # Don't expose SRGB buffer formats
            if 'buffers_only' in extra_flags and fmt.colorspace == SRGB:
                return None

            # Don't support 4_4 because it's not supported as render targets
            # and it's useless in other cases.
            if len(chan_sizes) == 2 and chan_sizes[0] == 4:
                return None

            key = tuple([num_format] + chan_sizes)
            if key not in self.plain_gfx10_formats:
                return None

            gfx10_fmt = self.plain_gfx10_formats[key]
            return Object(
                img_format=gfx10_fmt.img_format,
                flags=gfx10_fmt.flags + extra_flags,
            )

        return None

def pipe_formats_to_formats(pipe_formats, mapping):
    formats = []
    for fmt in pipe_formats:
        if fmt.name in HARDCODED:
            obj = HARDCODED[fmt.name]
        else:
            obj = mapping.map(fmt)

        if obj is not None:
            args = obj.img_format
            if 'buffers_only' in obj.flags:
                args += ', .buffers_only = 1'
        else:
            args = None
        formats.append((fmt.name, args))

    return formats

if __name__ == '__main__':
    pipe_formats = parse(sys.argv[1])

    # gfx10
    with open(sys.argv[2], 'r') as filp:
        db = RegisterDatabase.from_json(json.load(filp))

    gfx10_formats = [Gfx10Format(entry) for entry in db.enum('GFX10_FORMAT').entries]
    mapping = Gfx10FormatMapping(pipe_formats, gfx10_formats)
    formats = pipe_formats_to_formats(pipe_formats, mapping)
    print(header_template.render(header=True, gfx='gfx10', formats=formats))

    # gfx11
    with open(sys.argv[3], 'r') as filp:
        db = RegisterDatabase.from_json(json.load(filp))

    gfx11_formats = [Gfx10Format(entry) for entry in db.enum('GFX11_FORMAT').entries]
    mapping = Gfx10FormatMapping(pipe_formats, gfx11_formats)
    formats = pipe_formats_to_formats(pipe_formats, mapping)
    print(header_template.render(header=False, gfx='gfx11', formats=formats))
