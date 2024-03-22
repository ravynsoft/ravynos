/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#define DEBUG_PARSING 0
#define DEBUG_VP 0
#define DEBUG_FP 0

/**
 * \file arbprogparse.c
 * ARB_*_program parser core
 * \author Karl Rasche
 */

/**
Notes on program parameters, etc.

The instructions we emit will use six kinds of source registers:

  PROGRAM_INPUT      - input registers
  PROGRAM_TEMPORARY  - temp registers
  PROGRAM_ADDRESS    - address/indirect register
  PROGRAM_CONSTANT   - indexes into program->Parameters, a known constant/literal
  PROGRAM_STATE_VAR  - indexes into program->Parameters, and may actually be:
                       + a state variable, like "state.fog.color", or
                       + a pointer to a "program.local[k]" parameter, or
                       + a pointer to a "program.env[k]" parameter

Basically, all the program.local[] and program.env[] values will get mapped
into the unified gl_program->Parameters array.  This solves the problem of
having three separate program parameter arrays.
*/


#include "util/glheader.h"

#include "main/context.h"
#include "arbprogparse.h"
#include "prog_parameter.h"
#include "prog_statevars.h"
#include "prog_instruction.h"
#include "prog_print.h"
#include "program_parser.h"


void
_mesa_parse_arb_fragment_program(struct gl_context* ctx, GLenum target,
                                 const GLvoid *str, GLsizei len,
                                 struct gl_program *program)
{
   struct gl_program prog;
   struct asm_parser_state state;
   GLuint i;

   assert(target == GL_FRAGMENT_PROGRAM_ARB);

   memset(&prog, 0, sizeof(prog));
   memset(&state, 0, sizeof(state));
   state.prog = &prog;
   state.mem_ctx = program;

   if (!_mesa_parse_arb_program(ctx, target, (const GLubyte*) str, len,
				&state)) {
      /* Error in the program. Just return. */
      return;
   }

   ralloc_free(program->String);

   /* Copy the relevant contents of the arb_program struct into the
    * fragment_program struct.
    */
   program->String          = prog.String;
   program->arb.NumInstructions = prog.arb.NumInstructions;
   program->arb.NumTemporaries  = prog.arb.NumTemporaries;
   program->arb.NumParameters   = prog.arb.NumParameters;
   program->arb.NumAttributes   = prog.arb.NumAttributes;
   program->arb.NumAddressRegs  = prog.arb.NumAddressRegs;
   program->arb.NumNativeInstructions = prog.arb.NumNativeInstructions;
   program->arb.NumNativeTemporaries = prog.arb.NumNativeTemporaries;
   program->arb.NumNativeParameters = prog.arb.NumNativeParameters;
   program->arb.NumNativeAttributes = prog.arb.NumNativeAttributes;
   program->arb.NumNativeAddressRegs = prog.arb.NumNativeAddressRegs;
   program->arb.NumAluInstructions   = prog.arb.NumAluInstructions;
   program->arb.NumTexInstructions   = prog.arb.NumTexInstructions;
   program->arb.NumTexIndirections   = prog.arb.NumTexIndirections;
   program->arb.NumNativeAluInstructions = prog.arb.NumAluInstructions;
   program->arb.NumNativeTexInstructions = prog.arb.NumTexInstructions;
   program->arb.NumNativeTexIndirections = prog.arb.NumTexIndirections;
   program->info.inputs_read      = prog.info.inputs_read;
   program->info.outputs_written = prog.info.outputs_written;
   program->arb.IndirectRegisterFiles = prog.arb.IndirectRegisterFiles;
   for (i = 0; i < MAX_TEXTURE_IMAGE_UNITS; i++) {
      program->TexturesUsed[i] = prog.TexturesUsed[i];
      if (prog.TexturesUsed[i])
         program->SamplersUsed |= (1 << i);
   }
   program->ShadowSamplers = prog.ShadowSamplers;
   program->info.fs.origin_upper_left = state.option.OriginUpperLeft;
   program->info.fs.pixel_center_integer = state.option.PixelCenterInteger;

   program->info.fs.uses_discard = state.fragment.UsesKill;
   program->arb.Fog = state.option.Fog;

   ralloc_free(program->arb.Instructions);
   program->arb.Instructions = prog.arb.Instructions;

   if (program->Parameters)
      _mesa_free_parameter_list(program->Parameters);
   program->Parameters    = prog.Parameters;

#if DEBUG_FP
   printf("____________Fragment program %u ________\n", program->Id);
   _mesa_print_program(program);
#endif
}



/**
 * Parse the vertex program string.  If success, update the given
 * vertex_program object with the new program.  Else, leave the vertex_program
 * object unchanged.
 */
void
_mesa_parse_arb_vertex_program(struct gl_context *ctx, GLenum target,
			       const GLvoid *str, GLsizei len,
			       struct gl_program *program)
{
   struct gl_program prog;
   struct asm_parser_state state;

   assert(target == GL_VERTEX_PROGRAM_ARB);

   memset(&prog, 0, sizeof(prog));
   memset(&state, 0, sizeof(state));
   state.prog = &prog;
   state.mem_ctx = program;

   if (!_mesa_parse_arb_program(ctx, target, (const GLubyte*) str, len,
				&state)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glProgramString(bad program)");
      return;
   }

   ralloc_free(program->String);

   /* Copy the relevant contents of the arb_program struct into the
    * vertex_program struct.
    */
   program->String          = prog.String;
   program->arb.NumInstructions = prog.arb.NumInstructions;
   program->arb.NumTemporaries  = prog.arb.NumTemporaries;
   program->arb.NumParameters   = prog.arb.NumParameters;
   program->arb.NumAttributes   = prog.arb.NumAttributes;
   program->arb.NumAddressRegs  = prog.arb.NumAddressRegs;
   program->arb.NumNativeInstructions = prog.arb.NumNativeInstructions;
   program->arb.NumNativeTemporaries = prog.arb.NumNativeTemporaries;
   program->arb.NumNativeParameters = prog.arb.NumNativeParameters;
   program->arb.NumNativeAttributes = prog.arb.NumNativeAttributes;
   program->arb.NumNativeAddressRegs = prog.arb.NumNativeAddressRegs;
   program->info.inputs_read     = prog.info.inputs_read;
   program->info.outputs_written = prog.info.outputs_written;
   program->arb.IndirectRegisterFiles = prog.arb.IndirectRegisterFiles;
   program->arb.IsPositionInvariant = (state.option.PositionInvariant)
      ? GL_TRUE : GL_FALSE;

   ralloc_free(program->arb.Instructions);
   program->arb.Instructions = prog.arb.Instructions;

   if (program->Parameters)
      _mesa_free_parameter_list(program->Parameters);
   program->Parameters = prog.Parameters;

#if DEBUG_VP
   printf("____________Vertex program %u __________\n", program->Id);
   _mesa_print_program(program);
#endif
}
