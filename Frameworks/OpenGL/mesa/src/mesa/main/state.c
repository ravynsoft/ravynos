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


/**
 * \file state.c
 * State management.
 * 
 * This file manages recalculation of derived values in struct gl_context.
 */


#include "util/glheader.h"
#include "mtypes.h"
#include "arrayobj.h"
#include "context.h"
#include "debug.h"
#include "macros.h"
#include "ffvertex_prog.h"
#include "framebuffer.h"
#include "light.h"
#include "matrix.h"
#include "pixel.h"
#include "program/program.h"
#include "program/prog_parameter.h"
#include "shaderobj.h"
#include "state.h"
#include "stencil.h"
#include "texenvprogram.h"
#include "texobj.h"
#include "texstate.h"
#include "varray.h"
#include "vbo/vbo.h"
#include "viewport.h"
#include "blend.h"

#include "state_tracker/st_context.h"
#include "state_tracker/st_util.h"

void
_mesa_update_allow_draw_out_of_order(struct gl_context *ctx)
{
   /* Out-of-order drawing is useful when vertex array draws and immediate
    * mode are interleaved.
    *
    * Example with 3 draws:
    *   glBegin();
    *      glVertex();
    *   glEnd();
    *   glDrawElements();
    *   glBegin();
    *      glVertex();
    *   glEnd();
    *
    * Out-of-order drawing changes the execution order like this:
    *   glDrawElements();
    *   glBegin();
    *      glVertex();
    *      glVertex();
    *   glEnd();
    *
    * If out-of-order draws are enabled, immediate mode vertices are not
    * flushed before glDrawElements, resulting in fewer draws and lower CPU
    * overhead. This helps workstation applications.
    *
    * This is a simplified version of out-of-order determination to catch
    * common cases.
    *
    * RadeonSI has a complete and more complicated out-of-order determination
    * for driver-internal reasons.
    */
   /* Only the compatibility profile with immediate mode needs this. */
   if (!ctx->Const.AllowDrawOutOfOrder)
      return;

   assert(_mesa_is_desktop_gl_compat(ctx));

   /* If all of these are NULL, GLSL is disabled. */
   struct gl_program *vs =
      ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX];
   struct gl_program *tcs =
      ctx->_Shader->CurrentProgram[MESA_SHADER_TESS_CTRL];
   struct gl_program *tes =
      ctx->_Shader->CurrentProgram[MESA_SHADER_TESS_EVAL];
   struct gl_program *gs =
      ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY];
   struct gl_program *fs =
      ctx->_Shader->CurrentProgram[MESA_SHADER_FRAGMENT];
   GLenum16 depth_func = ctx->Depth.Func;

   /* Z fighting and any primitives with equal Z shouldn't be reordered
    * with LESS/LEQUAL/GREATER/GEQUAL functions.
    *
    * When drawing 2 primitive with equal Z:
    * - with LEQUAL/GEQUAL, the last primitive wins the Z test.
    * - with LESS/GREATER, the first primitive wins the Z test.
    *
    * Here we ignore that on the basis that such cases don't occur in real
    * apps, and we they do occur, they occur with blending where out-of-order
    * drawing is always disabled.
    */
   bool previous_state = ctx->_AllowDrawOutOfOrder;
   ctx->_AllowDrawOutOfOrder =
         ctx->DrawBuffer &&
         ctx->DrawBuffer->Visual.depthBits &&
         ctx->Depth.Test &&
         ctx->Depth.Mask &&
         (depth_func == GL_NEVER ||
          depth_func == GL_LESS ||
          depth_func == GL_LEQUAL ||
          depth_func == GL_GREATER ||
          depth_func == GL_GEQUAL) &&
         (!ctx->DrawBuffer->Visual.stencilBits ||
          !ctx->Stencil.Enabled) &&
         (!ctx->Color.ColorMask ||
          (!ctx->Color.BlendEnabled &&
           (!ctx->Color.ColorLogicOpEnabled ||
            ctx->Color._LogicOp == COLOR_LOGICOP_COPY))) &&
         (!vs || !vs->info.writes_memory) &&
         (!tes || !tes->info.writes_memory) &&
         (!tcs || !tcs->info.writes_memory) &&
         (!gs || !gs->info.writes_memory) &&
         (!fs || !fs->info.writes_memory || !fs->info.fs.early_fragment_tests);

   /* If we are disabling out-of-order drawing, we need to flush queued
    * vertices.
    */
   if (previous_state && !ctx->_AllowDrawOutOfOrder)
      FLUSH_VERTICES(ctx, 0, 0);
}

uint64_t
_mesa_get_active_states(struct gl_context *ctx)
{
   struct gl_program *vp = ctx->VertexProgram._Current;
   struct gl_program *tcp = ctx->TessCtrlProgram._Current;
   struct gl_program *tep = ctx->TessEvalProgram._Current;
   struct gl_program *gp = ctx->GeometryProgram._Current;
   struct gl_program *fp = ctx->FragmentProgram._Current;
   struct gl_program *cp = ctx->ComputeProgram._Current;
   uint64_t active_shader_states = 0;

   if (vp)
      active_shader_states |= vp->affected_states;
   if (tcp)
      active_shader_states |= tcp->affected_states;
   if (tep)
      active_shader_states |= tep->affected_states;
   if (gp)
      active_shader_states |= gp->affected_states;
   if (fp)
      active_shader_states |= fp->affected_states;
   if (cp)
      active_shader_states |= cp->affected_states;

   /* Mark non-shader-resource shader states as "always active". */
   return active_shader_states | ~ST_ALL_SHADER_RESOURCES;
}

/**
 * Update the ctx->*Program._Current pointers to point to the
 * current/active programs.
 *
 * Programs may come from 3 sources: GLSL shaders, ARB/NV_vertex/fragment
 * programs or programs derived from fixed-function state.
 *
 * This function needs to be called after texture state validation in case
 * we're generating a fragment program from fixed-function texture state.
 *
 * \return bitfield which will indicate _NEW_PROGRAM state if a new vertex
 * or fragment program is being used.
 */
static GLbitfield
update_program(struct gl_context *ctx)
{
   struct gl_program *vsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX];
   struct gl_program *tcsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_TESS_CTRL];
   struct gl_program *tesProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_TESS_EVAL];
   struct gl_program *gsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY];
   struct gl_program *fsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_FRAGMENT];
   struct gl_program *csProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_COMPUTE];
   const struct gl_program *prevVP = ctx->VertexProgram._Current;
   const struct gl_program *prevFP = ctx->FragmentProgram._Current;
   const struct gl_program *prevGP = ctx->GeometryProgram._Current;
   const struct gl_program *prevTCP = ctx->TessCtrlProgram._Current;
   const struct gl_program *prevTEP = ctx->TessEvalProgram._Current;
   const struct gl_program *prevCP = ctx->ComputeProgram._Current;
   uint64_t prev_vp_affected_states = prevVP ? prevVP->affected_states : 0;
   uint64_t prev_tcp_affected_states = prevTCP ? prevTCP->affected_states : 0;
   uint64_t prev_tep_affected_states = prevTEP ? prevTEP->affected_states : 0;
   uint64_t prev_gp_affected_states = prevGP ? prevGP->affected_states : 0;
   uint64_t prev_fp_affected_states = prevFP ? prevFP->affected_states : 0;
   uint64_t prev_cp_affected_states = prevCP ? prevCP->affected_states : 0;

   /*
    * Set the ctx->VertexProgram._Current and ctx->FragmentProgram._Current
    * pointers to the programs that should be used for rendering.  If either
    * is NULL, use fixed-function code paths.
    *
    * These programs may come from several sources.  The priority is as
    * follows:
    *   1. OpenGL 2.0/ARB vertex/fragment shaders
    *   2. ARB/NV vertex/fragment programs
    *   3. ATI fragment shader
    *   4. Programs derived from fixed-function state.
    *
    * Note: it's possible for a vertex shader to get used with a fragment
    * program (and vice versa) here, but in practice that shouldn't ever
    * come up, or matter.
    */

   if (fsProg) {
      /* Use GLSL fragment shader */
      _mesa_reference_program(ctx, &ctx->FragmentProgram._Current, fsProg);
      _mesa_reference_program(ctx, &ctx->FragmentProgram._TexEnvProgram,
                              NULL);
   } else if (_mesa_arb_fragment_program_enabled(ctx)) {
      /* Use user-defined fragment program */
      _mesa_reference_program(ctx, &ctx->FragmentProgram._Current,
                              ctx->FragmentProgram.Current);
      _mesa_reference_program(ctx, &ctx->FragmentProgram._TexEnvProgram,
			      NULL);
   } else if (_mesa_ati_fragment_shader_enabled(ctx) &&
            ctx->ATIFragmentShader.Current->Program) {
       /* Use the enabled ATI fragment shader's associated program */
      _mesa_reference_program(ctx, &ctx->FragmentProgram._Current,
                              ctx->ATIFragmentShader.Current->Program);
      _mesa_reference_program(ctx, &ctx->FragmentProgram._TexEnvProgram,
                              NULL);
   } else {
      /* Use fragment program generated from fixed-function state */
      _mesa_reference_program(ctx, &ctx->FragmentProgram._Current,
                              _mesa_get_fixed_func_fragment_program(ctx));
      _mesa_reference_program(ctx, &ctx->FragmentProgram._TexEnvProgram,
                              ctx->FragmentProgram._Current);
   }

   /* Examine vertex program after fragment program as
    * _mesa_get_fixed_func_vertex_program() needs to know active
    * fragprog inputs.
    */
   if (vsProg) {
      /* Use GLSL vertex shader */
      assert(VP_MODE_SHADER == ctx->VertexProgram._VPMode);
      _mesa_reference_program(ctx, &ctx->VertexProgram._Current, vsProg);
   } else if (_mesa_arb_vertex_program_enabled(ctx)) {
      /* Use user-defined vertex program */
      assert(VP_MODE_SHADER == ctx->VertexProgram._VPMode);
      _mesa_reference_program(ctx, &ctx->VertexProgram._Current,
                              ctx->VertexProgram.Current);
   } else {
      /* Use vertex program generated from fixed-function state */
      assert(VP_MODE_FF == ctx->VertexProgram._VPMode);
      _mesa_reference_program(ctx, &ctx->VertexProgram._Current,
                              _mesa_get_fixed_func_vertex_program(ctx));
      _mesa_reference_program(ctx, &ctx->VertexProgram._TnlProgram,
                              ctx->VertexProgram._Current);
   }

   /* Bind or unbind these shaders. (NULL = unbind) */
   _mesa_reference_program(ctx, &ctx->GeometryProgram._Current, gsProg);
   _mesa_reference_program(ctx, &ctx->TessEvalProgram._Current, tesProg);
   _mesa_reference_program(ctx, &ctx->TessCtrlProgram._Current, tcsProg);
   _mesa_reference_program(ctx, &ctx->ComputeProgram._Current, csProg);

   bool vp_changed = ctx->VertexProgram._Current != prevVP;
   bool tcp_changed = ctx->TessCtrlProgram._Current != prevTCP;
   bool tep_changed = ctx->TessEvalProgram._Current != prevTEP;
   bool gp_changed = ctx->GeometryProgram._Current != prevGP;
   bool fp_changed = ctx->FragmentProgram._Current != prevFP;
   bool cp_changed = ctx->ComputeProgram._Current != prevCP;

   /* Set NewDriverState depending on which shaders have changed. */
   uint64_t dirty = 0;

   /* Flag states used by both new and old shaders to rebind shader resources
    * (because shaders pack them and reorder them) and to unbind shader
    * resources properly when transitioning to shaders that don't use them.
    */
   if (vp_changed) {
      ctx->Array.NewVertexElements = true;
      dirty |= prev_vp_affected_states;
      if (ctx->VertexProgram._Current)
         dirty |= ST_NEW_VERTEX_PROGRAM(ctx, ctx->VertexProgram._Current);
   }

   if (tcp_changed) {
      dirty |= prev_tcp_affected_states;
      if (ctx->TessCtrlProgram._Current)
         dirty |= ctx->TessCtrlProgram._Current->affected_states;
   }

   if (tep_changed) {
      dirty |= prev_tep_affected_states;
      if (ctx->TessEvalProgram._Current)
         dirty |= ctx->TessEvalProgram._Current->affected_states;
   }

   if (gp_changed) {
      dirty |= prev_gp_affected_states;
      if (ctx->GeometryProgram._Current)
         dirty |= ctx->GeometryProgram._Current->affected_states;
   }

   if (fp_changed) {
      dirty |= prev_fp_affected_states;
      if (ctx->FragmentProgram._Current)
         dirty |= ctx->FragmentProgram._Current->affected_states;

      if (!ctx->st->needs_texcoord_semantic)
         dirty |= ST_NEW_RASTERIZER;
   }

   if (cp_changed) {
      dirty |= prev_cp_affected_states;
      if (ctx->ComputeProgram._Current)
         dirty |= ctx->ComputeProgram._Current->affected_states;
   }

   struct gl_program *last_vertex_stage;
   bool last_vertex_stage_dirty;

   if (ctx->GeometryProgram._Current) {
      last_vertex_stage = ctx->GeometryProgram._Current;
      last_vertex_stage_dirty = gp_changed;
   } else if (ctx->TessEvalProgram._Current) {
      last_vertex_stage = ctx->TessEvalProgram._Current;
      last_vertex_stage_dirty = gp_changed | tep_changed;
   } else {
      last_vertex_stage = ctx->VertexProgram._Current;
      last_vertex_stage_dirty = gp_changed | tep_changed | vp_changed;
   }

   /* Find out the number of viewports. This determines how many scissors
    * and viewport states we need to update.
    */
   struct st_context *st = ctx->st;
   unsigned num_viewports = 1;

   if (last_vertex_stage &&
       last_vertex_stage->info.outputs_written & (
             VARYING_BIT_VIEWPORT | VARYING_BIT_VIEWPORT_MASK))
      num_viewports = ctx->Const.MaxViewports;

   if (st->state.num_viewports != num_viewports) {
      st->state.num_viewports = num_viewports;
      dirty |= ST_NEW_VIEWPORT;

      if (ctx->Scissor.EnableFlags & u_bit_consecutive(0, num_viewports))
         dirty |= ST_NEW_SCISSOR;
   }

   if (st->lower_point_size && last_vertex_stage_dirty &&
       !ctx->VertexProgram.PointSizeEnabled && !ctx->PointSizeIsSet) {
      if (ctx->GeometryProgram._Current) {
         ctx->NewDriverState |= ST_NEW_GS_CONSTANTS;
      } else if (ctx->TessEvalProgram._Current) {
         ctx->NewDriverState |= ST_NEW_TES_CONSTANTS;
      } else {
         ctx->NewDriverState |= ST_NEW_VS_CONSTANTS;
      }
   }

   ctx->NewDriverState |= dirty;

   /* Let the driver know what's happening: */
   if (fp_changed || vp_changed || gp_changed || tep_changed ||
       tcp_changed || cp_changed) {
      /* This will mask out unused shader resources. */
      st->active_states = _mesa_get_active_states(ctx);

      /* Some drivers need to clean up previous states too */
      if (st->validate_all_dirty_states)
         st->active_states |= dirty;

      return _NEW_PROGRAM;
   }

   return 0;
}


static GLbitfield
update_single_program_constants(struct gl_context *ctx,
                                struct gl_program *prog,
                                gl_shader_stage stage)
{
   if (prog) {
      const struct gl_program_parameter_list *params = prog->Parameters;
      if (params && params->StateFlags & ctx->NewState) {
         if (ctx->DriverFlags.NewShaderConstants[stage])
            ctx->NewDriverState |= ctx->DriverFlags.NewShaderConstants[stage];
         else
            return _NEW_PROGRAM_CONSTANTS;
      }
   }
   return 0;
}


/**
 * This updates fixed-func state constants such as gl_ModelViewMatrix.
 * Examine shader constants and return either _NEW_PROGRAM_CONSTANTS or 0.
 */
static GLbitfield
update_program_constants(struct gl_context *ctx)
{
   GLbitfield new_state =
      update_single_program_constants(ctx, ctx->VertexProgram._Current,
                                      MESA_SHADER_VERTEX) |
      update_single_program_constants(ctx, ctx->FragmentProgram._Current,
                                      MESA_SHADER_FRAGMENT);

   if (_mesa_is_desktop_gl_compat(ctx) &&
       ctx->Const.GLSLVersionCompat >= 150) {
      new_state |=
         update_single_program_constants(ctx, ctx->GeometryProgram._Current,
                                         MESA_SHADER_GEOMETRY);

      if (_mesa_has_ARB_tessellation_shader(ctx)) {
         new_state |=
            update_single_program_constants(ctx, ctx->TessCtrlProgram._Current,
                                            MESA_SHADER_TESS_CTRL) |
            update_single_program_constants(ctx, ctx->TessEvalProgram._Current,
                                            MESA_SHADER_TESS_EVAL);
      }
   }

   return new_state;
}


static void
update_fixed_func_program_usage(struct gl_context *ctx)
{
   ctx->FragmentProgram._UsesTexEnvProgram =
      !ctx->_Shader->CurrentProgram[MESA_SHADER_FRAGMENT] && /* GLSL*/
      !_mesa_arb_fragment_program_enabled(ctx) &&
      !(_mesa_ati_fragment_shader_enabled(ctx) &&
        ctx->ATIFragmentShader.Current->Program);

   ctx->VertexProgram._UsesTnlProgram =
      !ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX] && /* GLSL */
      !_mesa_arb_vertex_program_enabled(ctx);
}


/**
 * Compute derived GL state.
 * If __struct gl_contextRec::NewState is non-zero then this function \b must
 * be called before rendering anything.
 *
 * Calls dd_function_table::UpdateState to perform any internal state
 * management necessary.
 * 
 * \sa _mesa_update_modelview_project(), _mesa_update_texture(),
 * _mesa_update_buffer_bounds(),
 * _mesa_update_lighting() and _mesa_update_tnl_spaces().
 */
void
_mesa_update_state_locked( struct gl_context *ctx )
{
   GLbitfield new_state = ctx->NewState;
   GLbitfield new_prog_state = 0x0;
   const GLbitfield checked_states =
      _NEW_BUFFERS | _NEW_MODELVIEW | _NEW_PROJECTION | _NEW_TEXTURE_MATRIX |
      _NEW_TEXTURE_OBJECT | _NEW_TEXTURE_STATE | _NEW_PROGRAM |
      _NEW_LIGHT_CONSTANTS | _NEW_POINT | _NEW_FF_VERT_PROGRAM |
      _NEW_FF_FRAG_PROGRAM | _NEW_TNL_SPACES;

   /* we can skip a bunch of state validation checks if the dirty
    * state matches one or more bits in 'computed_states'.
    */
   if (!(new_state & checked_states))
      goto out;

   if (MESA_VERBOSE & VERBOSE_STATE)
      _mesa_print_state("_mesa_update_state", new_state);

   if (new_state & _NEW_BUFFERS)
      _mesa_update_framebuffer(ctx, ctx->ReadBuffer, ctx->DrawBuffer);

   /* Handle Core and Compatibility contexts separately. */
   if (_mesa_is_desktop_gl_compat(ctx) ||
       _mesa_is_gles1(ctx)) {
      /* Update derived state. */
      if (new_state & (_NEW_MODELVIEW|_NEW_PROJECTION))
         _mesa_update_modelview_project( ctx, new_state );

      if (new_state & _NEW_TEXTURE_MATRIX)
         new_state |= _mesa_update_texture_matrices(ctx);

      if (new_state & (_NEW_TEXTURE_OBJECT | _NEW_TEXTURE_STATE | _NEW_PROGRAM))
         new_state |= _mesa_update_texture_state(ctx);

      if (new_state & _NEW_LIGHT_CONSTANTS)
         new_state |= _mesa_update_lighting(ctx);

      /* ctx->_NeedEyeCoords is determined here.
       *
       * If the truth value of this variable has changed, update for the
       * new lighting space and recompute the positions of lights and the
       * normal transform.
       *
       * If the lighting space hasn't changed, may still need to recompute
       * light positions & normal transforms for other reasons.
       */
      if (new_state & (_NEW_TNL_SPACES | _NEW_LIGHT_CONSTANTS |
                       _NEW_MODELVIEW)) {
         if (_mesa_update_tnl_spaces(ctx, new_state))
            new_state |= _NEW_FF_VERT_PROGRAM;
      }

      if (new_state & _NEW_PROGRAM)
         update_fixed_func_program_usage(ctx);

      /* Determine which states affect fixed-func vertex/fragment program. */
      GLbitfield prog_flags = _NEW_PROGRAM;

      if (ctx->FragmentProgram._UsesTexEnvProgram) {
         prog_flags |= _NEW_BUFFERS | _NEW_TEXTURE_OBJECT |
                       _NEW_FF_FRAG_PROGRAM | _NEW_TEXTURE_STATE;
      }

      if (ctx->VertexProgram._UsesTnlProgram)
         prog_flags |= _NEW_FF_VERT_PROGRAM;

      if (new_state & prog_flags) {
         /* When we generate programs from fixed-function vertex/fragment state
          * this call may generate/bind a new program.  If so, we need to
          * propogate the _NEW_PROGRAM flag to the driver.
          */
         new_prog_state |= update_program(ctx);
      }
   } else {
      /* GL Core and GLES 2/3 contexts */
      if (new_state & (_NEW_TEXTURE_OBJECT | _NEW_PROGRAM))
         _mesa_update_texture_state(ctx);

      if (new_state & _NEW_PROGRAM)
         update_program(ctx);
   }

 out:
   new_prog_state |= update_program_constants(ctx);

   ctx->NewState |= new_prog_state;

   /*
    * Give the driver a chance to act upon the new_state flags.
    * The driver might plug in different span functions, for example.
    * Also, this is where the driver can invalidate the state of any
    * active modules (such as swrast_setup, swrast, tnl, etc).
    */
   st_invalidate_state(ctx);
   ctx->NewState = 0;
}


/* This is the usual entrypoint for state updates:
 */
void
_mesa_update_state( struct gl_context *ctx )
{
   _mesa_lock_context_textures(ctx);
   _mesa_update_state_locked(ctx);
   _mesa_unlock_context_textures(ctx);
}

/* This is the usual entrypoint for state updates in glClear calls:
 */
void
_mesa_update_clear_state( struct gl_context *ctx )
{
   GLbitfield new_state = ctx->NewState;

   if (MESA_VERBOSE & VERBOSE_STATE)
      _mesa_print_state("_mesa_update_clear_state", new_state);

   if (new_state & _NEW_BUFFERS) {
      _mesa_update_framebuffer(ctx, ctx->ReadBuffer, ctx->DrawBuffer);

      st_invalidate_buffers(st_context(ctx));
      ctx->NewState &= ~_NEW_BUFFERS;
   }
}

/**
 * Used by drivers to tell core Mesa that the driver is going to
 * install/ use its own vertex program.  In particular, this will
 * prevent generated fragment programs from using state vars instead
 * of ordinary varyings/inputs.
 */
void
_mesa_set_vp_override(struct gl_context *ctx, GLboolean flag)
{
   if (ctx->VertexProgram._Overriden != flag) {
      ctx->VertexProgram._Overriden = flag;

      /* Set one of the bits which will trigger fragment program
       * regeneration:
       */
      ctx->NewState |= _NEW_PROGRAM;
   }
}


static void
set_vertex_processing_mode(struct gl_context *ctx, gl_vertex_processing_mode m)
{
   if (ctx->VertexProgram._VPMode == m)
      return;

   /* On change we may get new maps into the current values */
   ctx->NewDriverState |= ST_NEW_VERTEX_ARRAYS;
   ctx->Array.NewVertexElements = true;

   /* Finally memorize the value */
   ctx->VertexProgram._VPMode = m;

   /* The gl_context::VertexProgram._VaryingInputs value is only used when in
    * VP_MODE_FF mode and the fixed-func pipeline is emulated by shaders.
    */
   ctx->VertexProgram._VPModeOptimizesConstantAttribs =
      m == VP_MODE_FF;

   /* Set a filter mask for the net enabled vao arrays.
    * This is to mask out arrays that would otherwise supersede required current
    * values for the fixed function shaders for example.
    */
   switch (m) {
   case VP_MODE_FF:
      /* When no vertex program is active (or the vertex program is generated
       * from fixed-function state).  We put the material values into the
       * generic slots.  Since the vao has no material arrays, mute these
       * slots from the enabled arrays so that the current material values
       * are pulled instead of the vao arrays.
       */
      ctx->VertexProgram._VPModeInputFilter = VERT_BIT_FF_ALL;
      break;

   case VP_MODE_SHADER:
      /* There are no shaders in OpenGL ES 1.x, so this code path should be
       * impossible to reach.  The meta code is careful to not use shaders in
       * ES1.
       */
      assert(ctx->API != API_OPENGLES);

      /* Other parts of the code assume that inputs[VERT_ATTRIB_POS] through
       * inputs[VERT_ATTRIB_GENERIC0-1] will be non-NULL.  However, in OpenGL
       * ES 2.0+ or OpenGL core profile, none of these arrays should ever
       * be enabled.
       */
      if (_mesa_is_desktop_gl_compat(ctx))
         ctx->VertexProgram._VPModeInputFilter = VERT_BIT_ALL;
      else
         ctx->VertexProgram._VPModeInputFilter = VERT_BIT_GENERIC_ALL;
      break;

   default:
      assert(0);
   }

   _mesa_set_varying_vp_inputs(ctx, ctx->VertexProgram._VPModeInputFilter &
                               ctx->Array._DrawVAO->_EnabledWithMapMode);
}


/**
 * Update ctx->VertexProgram._VPMode.
 * This is to distinguish whether we're running
 *   a vertex program/shader,
 *   a fixed-function TNL program or
 *   a fixed function vertex transformation without any program.
 */
void
_mesa_update_vertex_processing_mode(struct gl_context *ctx)
{
   if (ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX])
      set_vertex_processing_mode(ctx, VP_MODE_SHADER);
   else if (_mesa_arb_vertex_program_enabled(ctx))
      set_vertex_processing_mode(ctx, VP_MODE_SHADER);
   else
      set_vertex_processing_mode(ctx, VP_MODE_FF);
}


void
_mesa_reset_vertex_processing_mode(struct gl_context *ctx)
{
   ctx->VertexProgram._VPMode = -1; /* force the update */
   _mesa_update_vertex_processing_mode(ctx);
}
