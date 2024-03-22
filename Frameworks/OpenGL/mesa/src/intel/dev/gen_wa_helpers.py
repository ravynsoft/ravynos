# Copyright © 2023 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse
import collections
import json
import os
import sys
from mako.template import Template

HEADER_TEMPLATE = Template("""\
/*
 * Copyright © 2023 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef INTEL_WA_H
#define INTEL_WA_H

#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

struct intel_device_info;
void intel_device_info_init_was(struct intel_device_info *devinfo);

enum intel_wa_steppings {
% for a in stepping_enum:
   INTEL_STEPPING_${a},
% endfor
   INTEL_STEPPING_RELEASE
};

enum intel_workaround_id {
% for a in wa_def:
   INTEL_WA_${a},
% endfor
   INTEL_WA_NUM
};

/* These defines apply workarounds to a subset of platforms within a graphics
 * generation.  They must be used in conjunction with intel_needs_workaround()
 * to check platform details.  Use these macros to compile out genxml code on
 * generations where it can never execute.  Whenever possible, prefer use of
 * INTEL_NEEDS_WA_{num} instead of INTEL_WA_{num}_GFX_VER
 */
% for a in wa_def:
#define INTEL_WA_${a}_GFX_VER ${wa_macro[a]}
% endfor

/* These defines may be used to compile out genxml workaround implementations
 * using #if guards.  If a definition has been 'poisoned' below, then it applies to a
 * subset of a graphics generation.  In that case, use INTEL_WA_{NUM}_GFX_VER macros
 * in conjunction with calls to intel_needs_workaround().
 */
% for a in partial_gens:
    % if partial_gens[a]:
PRAGMA_POISON(INTEL_NEEDS_WA_${a})
    % else:
#define INTEL_NEEDS_WA_${a} INTEL_WA_${a}_GFX_VER
    % endif
% endfor

#define INTEL_ALL_WA ${"\\\\"}
% for wa_id in wa_def:
  INTEL_WA(${wa_id}), ${"\\\\"}
% endfor

#ifdef __cplusplus
}
#endif

#endif /* INTEL_WA_H */
""")

IMPL_TEMPLATE = Template("""\
/*
 * Copyright © 2023 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "dev/intel_wa.h"
#include "dev/intel_device_info.h"
#include "util/bitset.h"

void intel_device_info_init_was(struct intel_device_info *devinfo)
{
   switch(devinfo->platform) {
% for platform in platform_bugs:
      case ${platform}:
% if platform in stepping_bugs:
         switch(intel_device_info_wa_stepping(devinfo)) {
% for stepping, ids in stepping_bugs[platform].items():
            case INTEL_STEPPING_${stepping}:
% for id in ids:
               BITSET_SET(devinfo->workarounds, INTEL_WA_${id});
% endfor
               break;
% endfor
            default:
               break;
         }
% endif
% for id in platform_bugs[platform]:
         BITSET_SET(devinfo->workarounds, INTEL_WA_${id});
% endfor
         break;
% endfor
      default:
         /* unsupported platform */
         break;
   };
}
""")

def stepping_enums(wa_def):
    """provide a sorted list of all known steppings"""
    stepping_enum = []
    for bug in wa_def.values():
        for platform_info in bug["mesa_platforms"].values():
            steppings = platform_info["steppings"]
            if steppings == "all":
                continue
            steppings = steppings.split("..")
            for stepping in steppings:
                stepping = stepping.upper()
                if stepping and stepping != "None" and stepping not in stepping_enum:
                    stepping_enum.append(stepping)
    return sorted(stepping_enum)

_PLATFORM_GFXVERS = {"INTEL_PLATFORM_BDW" : 80,
                     "INTEL_PLATFORM_CHV" : 80,
                     "INTEL_PLATFORM_SKL" : 90,
                     "INTEL_PLATFORM_BXT" : 90,
                     "INTEL_PLATFORM_KBL" : 90,
                     "INTEL_PLATFORM_GLK" : 90,
                     "INTEL_PLATFORM_CFL" : 90,
                     "INTEL_PLATFORM_ICL" : 110,
                     "INTEL_PLATFORM_EHL" : 110,
                     "INTEL_PLATFORM_TGL" : 120,
                     "INTEL_PLATFORM_RKL" : 120,
                     "INTEL_PLATFORM_DG1" : 120,
                     "INTEL_PLATFORM_ADL" : 120,
                     "INTEL_PLATFORM_RPL" : 120,
                     "INTEL_PLATFORM_DG2_G10" : 125,
                     "INTEL_PLATFORM_DG2_G11" : 125,
                     "INTEL_PLATFORM_DG2_G12" : 125,
                     "INTEL_PLATFORM_MTL_U" : 125,
                     "INTEL_PLATFORM_MTL_H" : 125,
                     }

def macro_versions(wa_def):
    """provide a map of workaround id -> GFX_VERx10 macro test"""
    wa_macro = {}
    for bug_id, bug in wa_def.items():
        platforms = set()
        for platform in bug["mesa_platforms"]:
            gfxver = _PLATFORM_GFXVERS[platform]
            if gfxver not in platforms:
                platforms.add(gfxver)
        if not platforms:
            continue
        ver_cmps = [f"(GFX_VERx10 == {platform})" for platform in sorted(platforms)]
        wa_macro[bug_id] = ver_cmps[0]
        if len(ver_cmps) > 1:
            wa_macro[bug_id] = f"({' || '.join(ver_cmps)})"
    return wa_macro

def partial_gens(wa_def):
    """provide a map of workaround id -> true/false, indicating whether the wa
    applies to a subset of platforms in a generation"""
    wa_partial_gen = {}

    # map of gfxver -> set(all platforms for gfxver)
    generations = collections.defaultdict(set)
    for platform, gfxver in _PLATFORM_GFXVERS.items():
        generations[gfxver].add(platform)

    # map of platform -> set(all required platforms for gen completeness)
    required_platforms = collections.defaultdict(set)
    for gen_set in generations.values():
        for platform in gen_set:
            required_platforms[platform] = gen_set

    for bug_id, bug in wa_def.items():
        # for the given wa, create a set which includes all platforms that
        # match any of the affected gfxver.
        wa_required_for_completeness = set()
        for platform in bug["mesa_platforms"]:
            wa_required_for_completeness.update(required_platforms[platform])

        # eliminate each platform specifically indicated by the WA, to see if
        # are left over.
        for platform, desc in bug["mesa_platforms"].items():
            if desc["steppings"] == "all":
                wa_required_for_completeness.remove(platform)

        # if any platform remains in the required set, then this wa *partially*
        # applies to one of the gfxvers.
        wa_partial_gen[bug_id] = bool(wa_required_for_completeness)
    return wa_partial_gen

def platform_was(wa_def):
    """provide a map of platform -> list of workarounds"""
    platform_bugs = collections.defaultdict(list)
    for workaround, bug in wa_def.items():
        for platform, desc in bug["mesa_platforms"].items():
            if desc["steppings"] != "all":
                # stepping-specific workaround, not platform-wide
                continue
            platform_bugs[platform].append(workaround)
    return platform_bugs

def stepping_was(wa_def, all_steppings):
    """provide a map of wa[platform][stepping] -> [ids]"""
    stepping_bugs = collections.defaultdict(lambda: collections.defaultdict(list))
    for workaround, bug in wa_def.items():
        for platform, desc in bug["mesa_platforms"].items():
            if desc["steppings"] == "all":
                continue
            first_stepping, fixed_stepping = desc["steppings"].split("..")
            first_stepping = first_stepping.upper()
            fixed_stepping = fixed_stepping.upper()
            steppings = []
            for step in all_steppings:
                if step <first_stepping:
                    continue
                if step >= fixed_stepping:
                    break
                steppings.append(step)
            for step in steppings:
                u_step = step.upper()
                stepping_bugs[platform][u_step].append(workaround)
                stepping_bugs[platform][u_step].sort()
    return stepping_bugs

def main():
    """writes c/h generated files to outdir"""
    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("wa_file", type=str,
                        help="json data file with workaround definitions")
    parser.add_argument("header_file", help="include file to generate")
    parser.add_argument("impl_file", help="implementation file to generate")
    args = parser.parse_args()
    if not os.path.exists(args.wa_file):
        print(f"Error: workaround definition not found: {args.wa_file}")
        sys.exit(-1)

    # json dictionary of workaround definitions
    wa_def = {}
    with open(args.wa_file, encoding='utf8') as wa_fh:
        wa_def = json.load(wa_fh)

    # detect unknown platforms
    unknown_platforms = set()
    for wa in wa_def.values():
        for p in wa['mesa_platforms']:
            if p not in _PLATFORM_GFXVERS:
                unknown_platforms.add(p)
    if unknown_platforms:
        abbrev = map(lambda s: s.replace('INTEL_PLATFORM_', ''),
                     unknown_platforms)
        raise Exception(f'warning: unknown platforms in {args.wa_file}: '
                        f'{", ".join(abbrev)}')

    steppings = stepping_enums(wa_def)
    with open(args.header_file, 'w', encoding='utf8') as header:
        header.write(HEADER_TEMPLATE.render(wa_def=wa_def,
                                            stepping_enum=steppings,
                                            wa_macro=macro_versions(wa_def),
                                            partial_gens=partial_gens(wa_def)))
    with open(args.impl_file, 'w', encoding='utf8') as impl:
        impl.write(IMPL_TEMPLATE.render(platform_bugs=platform_was(wa_def),
                                        stepping_bugs=stepping_was(wa_def, steppings)))

main()
