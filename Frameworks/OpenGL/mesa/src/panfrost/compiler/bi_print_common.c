/*
 * Copyright (C) 2019 Connor Abbott <cwabbott0@gmail.com>
 * Copyright (C) 2019 Lyude Paul <thatslyude@gmail.com>
 * Copyright (C) 2019 Ryan Houdek <Sonicadvance1@gmail.com>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Prints shared with the disassembler */

#include "bi_print_common.h"

const char *
bi_message_type_name(enum bifrost_message_type T)
{
   switch (T) {
   case BIFROST_MESSAGE_NONE:
      return "";
   case BIFROST_MESSAGE_VARYING:
      return "vary";
   case BIFROST_MESSAGE_ATTRIBUTE:
      return "attr";
   case BIFROST_MESSAGE_TEX:
      return "tex";
   case BIFROST_MESSAGE_VARTEX:
      return "vartex";
   case BIFROST_MESSAGE_LOAD:
      return "load";
   case BIFROST_MESSAGE_STORE:
      return "store";
   case BIFROST_MESSAGE_ATOMIC:
      return "atomic";
   case BIFROST_MESSAGE_BARRIER:
      return "barrier";
   case BIFROST_MESSAGE_BLEND:
      return "blend";
   case BIFROST_MESSAGE_TILE:
      return "tile";
   case BIFROST_MESSAGE_Z_STENCIL:
      return "z_stencil";
   case BIFROST_MESSAGE_ATEST:
      return "atest";
   case BIFROST_MESSAGE_JOB:
      return "job";
   case BIFROST_MESSAGE_64BIT:
      return "64";
   default:
      return "XXX reserved";
   }
}

const char *
bi_flow_control_name(enum bifrost_flow mode)
{
   switch (mode) {
   case BIFROST_FLOW_END:
      return "eos";
   case BIFROST_FLOW_NBTB_PC:
      return "nbb br_pc";
   case BIFROST_FLOW_NBTB_UNCONDITIONAL:
      return "nbb r_uncond";
   case BIFROST_FLOW_NBTB:
      return "nbb";
   case BIFROST_FLOW_BTB_UNCONDITIONAL:
      return "bb r_uncond";
   case BIFROST_FLOW_BTB_NONE:
      return "bb";
   case BIFROST_FLOW_WE_UNCONDITIONAL:
      return "we r_uncond";
   case BIFROST_FLOW_WE:
      return "we";
   default:
      return "XXX";
   }
}
