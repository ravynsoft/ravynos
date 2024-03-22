# Copyright © 2013 Intel Corporation
# SPDX-License-Identifier: MIT

import sys

from builtin_types import BUILTIN_TYPES
from mako.template import Template

template = """\
/*
 * Copyright 2023 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

/* This is an automatically generated file. */

#include "glsl_types.h"
#include "util/glheader.h"

const char glsl_type_builtin_names[] =
%for n in NAME_ARRAY:
   "${n}"
%endfor
;

%for t in BUILTIN_TYPES:
const struct glsl_type glsl_type_builtin_${t["name"]} = {
   %for k, v in t.items():
       %if v is None or k == "name":
          <% continue %>
       %elif k == "name_id":
          .name_id = ${v},
          .has_builtin_name = 1,
       %else:
          .${k} = ${v},
       %endif
   %endfor
};

%endfor"""

if len(sys.argv) < 2:
    print('Missing output argument', file=sys.stderr)
    sys.exit(1)

output = sys.argv[1]

# Add padding to make sure zero is an invalid name.
invalid = "INVALID"
NAME_ARRAY = [invalid + "\\0"]
id = len(invalid) + 1

for t in BUILTIN_TYPES:
    name = t["name"]
    NAME_ARRAY.append(name + "\\0")
    t["name_id"] = id
    id += len(name) + 1

with open(output, 'w', encoding='utf-8') as f:
    f.write(Template(template).render(BUILTIN_TYPES=BUILTIN_TYPES, NAME_ARRAY=NAME_ARRAY))
