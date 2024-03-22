/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * TGSI program transformation utility.
 *
 * Authors:  Brian Paul
 */

#include "util/u_debug.h"
#include "util/log.h"

#include "tgsi_transform.h"

/**
 * Increments the next-token index if the tgsi_build_* succeeded, or extends the
 * token array and returns true to request a re-emit of the tgsi_build_* by the
 * caller.
 */
static bool
need_re_emit(struct tgsi_transform_context *ctx, uint32_t emitted, struct tgsi_header orig_header)
{
   if (emitted > 0) {
      ctx->ti += emitted;
      return false;
   } else {
      uint32_t new_len = ctx->max_tokens_out * 2;
      if (new_len < ctx->max_tokens_out) {
         ctx->fail = true;
         return false;
      }

      struct tgsi_token *new_tokens = tgsi_alloc_tokens(new_len);
      if (!new_tokens) {
         ctx->fail = true;
         return false;
      }
      memcpy(new_tokens, ctx->tokens_out, sizeof(struct tgsi_token) * ctx->ti);

      tgsi_free_tokens(ctx->tokens_out);
      ctx->tokens_out = new_tokens;
      ctx->max_tokens_out = new_len;

      /* Point the header at the resized tokens. */
      ctx->header = (struct tgsi_header *)new_tokens;
      /* The failing emit may have incremented header/body size, reset it to its state before our attempt. */
      *ctx->header = orig_header;

      return true;
   }
}

static void
emit_instruction(struct tgsi_transform_context *ctx,
                 const struct tgsi_full_instruction *inst)
{
   uint32_t emitted;
   struct tgsi_header orig_header = *ctx->header;

   do {
      emitted = tgsi_build_full_instruction(inst,
                                            ctx->tokens_out + ctx->ti,
                                            ctx->header,
                                            ctx->max_tokens_out - ctx->ti);
   } while (need_re_emit(ctx, emitted, orig_header));
}


static void
emit_declaration(struct tgsi_transform_context *ctx,
                 const struct tgsi_full_declaration *decl)
{
   uint32_t emitted;
   struct tgsi_header orig_header = *ctx->header;

   do {
      emitted = tgsi_build_full_declaration(decl,
                                            ctx->tokens_out + ctx->ti,
                                            ctx->header,
                                            ctx->max_tokens_out - ctx->ti);
   } while (need_re_emit(ctx, emitted, orig_header));
}


static void
emit_immediate(struct tgsi_transform_context *ctx,
               const struct tgsi_full_immediate *imm)
{
   uint32_t emitted;
   struct tgsi_header orig_header = *ctx->header;

   do {
      emitted = tgsi_build_full_immediate(imm,
                                          ctx->tokens_out + ctx->ti,
                                          ctx->header,
                                          ctx->max_tokens_out - ctx->ti);
   } while (need_re_emit(ctx, emitted, orig_header));
}


static void
emit_property(struct tgsi_transform_context *ctx,
              const struct tgsi_full_property *prop)
{
   uint32_t emitted;
   struct tgsi_header orig_header = *ctx->header;

   do {
      emitted = tgsi_build_full_property(prop,
                                         ctx->tokens_out + ctx->ti,
                                         ctx->header,
                                         ctx->max_tokens_out - ctx->ti);
   } while (need_re_emit(ctx, emitted, orig_header));
}


/**
 * Apply user-defined transformations to the input shader to produce
 * the output shader.
 * For example, a register search-and-replace operation could be applied
 * by defining a transform_instruction() callback that examined and changed
 * the instruction src/dest regs.
 *
 * \return new tgsi tokens, or NULL on failure
 */
struct tgsi_token *
tgsi_transform_shader(const struct tgsi_token *tokens_in,
                      unsigned initial_tokens_len,
                      struct tgsi_transform_context *ctx)
{
   bool first_instruction = true;
   bool epilog_emitted = false;
   int cond_stack = 0;
   int call_stack = 0;

   /* input shader */
   struct tgsi_parse_context parse;

   /* output shader */
   struct tgsi_processor *processor;

   /* Always include space for the header. */
   initial_tokens_len = MAX2(initial_tokens_len, 2);

   /**
    ** callback context init
    **/
   ctx->emit_instruction = emit_instruction;
   ctx->emit_declaration = emit_declaration;
   ctx->emit_immediate = emit_immediate;
   ctx->emit_property = emit_property;
   ctx->tokens_out = tgsi_alloc_tokens(initial_tokens_len);
   ctx->max_tokens_out = initial_tokens_len;
   ctx->fail = false;

   if (!ctx->tokens_out) {
      mesa_loge("failed to allocate %d tokens\n", initial_tokens_len);
      return NULL;
   }

   /**
    ** Setup to begin parsing input shader
    **/
   if (tgsi_parse_init( &parse, tokens_in ) != TGSI_PARSE_OK) {
      debug_printf("tgsi_parse_init() failed in tgsi_transform_shader()!\n");
      return NULL;
   }
   ctx->processor = parse.FullHeader.Processor.Processor;

   /**
    **  Setup output shader
    **/
   ctx->header = (struct tgsi_header *)ctx->tokens_out;
   *ctx->header = tgsi_build_header();

   processor = (struct tgsi_processor *) (ctx->tokens_out + 1);
   *processor = tgsi_build_processor( ctx->processor, ctx->header );

   ctx->ti = 2;


   /**
    ** Loop over incoming program tokens/instructions
    */
   while( !tgsi_parse_end_of_tokens( &parse ) ) {

      tgsi_parse_token( &parse );

      switch( parse.FullToken.Token.Type ) {
      case TGSI_TOKEN_TYPE_INSTRUCTION:
         {
            struct tgsi_full_instruction *fullinst
               = &parse.FullToken.FullInstruction;
            enum tgsi_opcode opcode = fullinst->Instruction.Opcode;

            if (first_instruction && ctx->prolog) {
               ctx->prolog(ctx);
            }

            /*
             * XXX Note: we handle the case of ret in main.
             * However, the output redirections done by transform
             * have their limits with control flow and will generally
             * not work correctly. e.g.
             * if (cond) {
             *    oColor = x;
             *    ret;
             * }
             * oColor = y;
             * end;
             * If the color output is redirected to a temp and modified
             * by a transform, this will not work (the oColor assignment
             * in the conditional will never make it to the actual output).
             */
            if ((opcode == TGSI_OPCODE_END || opcode == TGSI_OPCODE_RET) &&
                 call_stack == 0 && ctx->epilog && !epilog_emitted) {
               if (opcode == TGSI_OPCODE_RET && cond_stack != 0) {
                  assert(!"transform ignoring RET in main");
               } else {
                  assert(cond_stack == 0);
                  /* Emit caller's epilog */
                  ctx->epilog(ctx);
                  epilog_emitted = true;
               }
               /* Emit END (or RET) */
               ctx->emit_instruction(ctx, fullinst);
            }
            else {
               switch (opcode) {
               case TGSI_OPCODE_IF:
               case TGSI_OPCODE_UIF:
               case TGSI_OPCODE_SWITCH:
               case TGSI_OPCODE_BGNLOOP:
                  cond_stack++;
                  break;
               case TGSI_OPCODE_CAL:
                  call_stack++;
                  break;
               case TGSI_OPCODE_ENDIF:
               case TGSI_OPCODE_ENDSWITCH:
               case TGSI_OPCODE_ENDLOOP:
                  assert(cond_stack > 0);
                  cond_stack--;
                  break;
               case TGSI_OPCODE_ENDSUB:
                  assert(call_stack > 0);
                  call_stack--;
                  break;
               case TGSI_OPCODE_BGNSUB:
               case TGSI_OPCODE_RET:
               default:
                  break;
               }
               if (ctx->transform_instruction)
                  ctx->transform_instruction(ctx, fullinst);
               else
                  ctx->emit_instruction(ctx, fullinst);
            }

            first_instruction = false;
         }
         break;

      case TGSI_TOKEN_TYPE_DECLARATION:
         {
            struct tgsi_full_declaration *fulldecl
               = &parse.FullToken.FullDeclaration;

            if (ctx->transform_declaration)
               ctx->transform_declaration(ctx, fulldecl);
            else
               ctx->emit_declaration(ctx, fulldecl);
         }
         break;

      case TGSI_TOKEN_TYPE_IMMEDIATE:
         {
            struct tgsi_full_immediate *fullimm
               = &parse.FullToken.FullImmediate;

            if (ctx->transform_immediate)
               ctx->transform_immediate(ctx, fullimm);
            else
               ctx->emit_immediate(ctx, fullimm);
         }
         break;
      case TGSI_TOKEN_TYPE_PROPERTY:
         {
            struct tgsi_full_property *fullprop
               = &parse.FullToken.FullProperty;

            if (ctx->transform_property)
               ctx->transform_property(ctx, fullprop);
            else
               ctx->emit_property(ctx, fullprop);
         }
         break;

      default:
         assert( 0 );
      }
   }
   assert(call_stack == 0);

   tgsi_parse_free (&parse);

   if (ctx->fail) {
      tgsi_free_tokens(ctx->tokens_out);
      return NULL;
   }

   return ctx->tokens_out;
}


#include "tgsi_text.h"

extern int tgsi_transform_foo( struct tgsi_token *tokens_out,
                               unsigned max_tokens_out );

/* This function exists only so that tgsi_text_translate() doesn't get
 * magic-ed out of the libtgsi.a archive by the build system.  Don't
 * remove unless you know this has been fixed - check on mingw/scons
 * builds as well.
 */
int
tgsi_transform_foo( struct tgsi_token *tokens_out,
                    unsigned max_tokens_out )
{
   const char *text = 
      "FRAG\n"
      "DCL IN[0], COLOR, CONSTANT\n"
      "DCL OUT[0], COLOR\n"
      "  0: MOV OUT[0], IN[0]\n"
      "  1: END";
        
   return tgsi_text_translate( text,
                               tokens_out,
                               max_tokens_out );
}
