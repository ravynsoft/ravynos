template = """/*
 * Copyright 2021 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_opcodes.h"

<%
def make_encoding(encoding):
   if encoding is None:
      return "{ 0 }"

   return "{{ {}, {}, {} }}".format(hex(encoding.exact), encoding.length_short, int(encoding.extensible))
%>

const struct agx_opcode_info agx_opcodes_info[AGX_NUM_OPCODES] = {
% for opcode in opcodes:
<%
   op = opcodes[opcode]
   imms = ["AGX_IMMEDIATE_" + imm.name.upper() for imm in op.imms]
   if len(imms) == 0:
      imms = ["0"]
%>
   [AGX_OPCODE_${opcode.upper()}] = {
      "${opcode}", ${op.srcs}, ${op.dests}, ${" | ".join(imms)},
      ${make_encoding(op.encoding_32)},
      ${make_encoding(op.encoding_16)},
      AGX_SCHEDULE_CLASS_${op.schedule_class.upper()},
      ${int(op.is_float)},
      ${int(op.can_eliminate)},
      ${int(op.can_reorder)},
   },
% endfor
};
"""

from mako.template import Template
from agx_opcodes import opcodes

print(Template(template).render(opcodes=opcodes))
