# Copyright © 2023 Intel Corporation
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

#ifndef _BUILTIN_TYPES_
#define _BUILTIN_TYPES_

%for t in BUILTIN_TYPES:
extern const struct glsl_type glsl_type_builtin_${t["name"]};
%endfor

#endif /* _BUILTIN_TYPES_ */"""

if len(sys.argv) < 2:
    print('Missing output argument', file=sys.stderr)
    sys.exit(1)

output = sys.argv[1]

with open(output, 'w', encoding='utf-8') as f:
    f.write(Template(template).render(BUILTIN_TYPES=BUILTIN_TYPES))
