/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 **************************************************************************/


/**
 * Implementation of glDrawTex() for GL_OES_draw_tex
 */




#include "main/image.h"
#include "main/macros.h"
#include "main/teximage.h"
#include "main/framebuffer.h"
#include "program/program.h"
#include "program/prog_print.h"

#include "st_context.h"
#include "st_atom.h"
#include "st_cb_bitmap.h"
#include "st_cb_drawtex.h"
#include "st_nir.h"
#include "st_util.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_shader_tokens.h"
#include "util/u_draw_quad.h"
#include "util/u_simple_shaders.h"
#include "util/u_upload_mgr.h"

#include "cso_cache/cso_context.h"


struct cached_shader
{
   void *handle;

   uint num_attribs;
   gl_varying_slot slots[2 + MAX_TEXTURE_UNITS];
};

#define MAX_SHADERS (2 * MAX_TEXTURE_UNITS)

/**
 * Simple linear list cache.
 * Most of the time there'll only be one cached shader.
 * XXX This should be per-st_context state.
 */
static struct cached_shader CachedShaders[MAX_SHADERS];
static GLuint NumCachedShaders = 0;

static gl_vert_attrib
slot_to_vert_attrib(gl_varying_slot slot)
{
   switch (slot) {
   case VARYING_SLOT_POS:
      return VERT_ATTRIB_POS;
   case VARYING_SLOT_COL0:
      return VERT_ATTRIB_COLOR0;
   case VARYING_SLOT_VAR0:
   case VARYING_SLOT_TEX0:
      return VERT_ATTRIB_GENERIC0;
   default:
      unreachable("unhandled slot");
   }
}

static void *
lookup_shader(struct st_context *st,
              uint num_attribs,
              const gl_varying_slot *slots)
{
   GLuint i, j;

   /* look for existing shader with same attributes */
   for (i = 0; i < NumCachedShaders; i++) {
      if (CachedShaders[i].num_attribs == num_attribs) {
         GLboolean match = GL_TRUE;
         for (j = 0; j < num_attribs; j++) {
            if (slots[j] != CachedShaders[i].slots[j]) {
               match = GL_FALSE;
               break;
            }
         }
         if (match)
            return CachedShaders[i].handle;
      }
   }

   /* not found - create new one now */
   if (NumCachedShaders >= MAX_SHADERS) {
      return NULL;
   }

   CachedShaders[i].num_attribs = num_attribs;
   for (j = 0; j < num_attribs; j++)
      CachedShaders[i].slots[j] = slots[j];

   unsigned inputs[2 + MAX_TEXTURE_UNITS];

   for (int j = 0; j < num_attribs; j++) {
      inputs[j] = slot_to_vert_attrib(slots[j]);
   }

   CachedShaders[i].handle =
      st_nir_make_passthrough_shader(st, "st/drawtex VS",
                                       MESA_SHADER_VERTEX,
                                       num_attribs, inputs,
                                       slots, NULL, 0);

   NumCachedShaders++;

   return CachedShaders[i].handle;
}


void
st_DrawTex(struct gl_context *ctx, GLfloat x, GLfloat y, GLfloat z,
           GLfloat width, GLfloat height)
{
   struct st_context *st = ctx->st;
   struct pipe_context *pipe = st->pipe;
   struct cso_context *cso = st->cso_context;
   struct pipe_resource *vbuffer = NULL;
   GLuint i, numTexCoords, numAttribs;
   GLboolean emitColor;
   gl_varying_slot slots[2 + MAX_TEXTURE_UNITS];
   struct cso_velems_state velems;
   unsigned offset;

   st_flush_bitmap_cache(st);
   st_invalidate_readpix_cache(st);

   st_validate_state(st, ST_PIPELINE_META_STATE_MASK);

   /* determine if we need vertex color */
   if (ctx->FragmentProgram._Current->info.inputs_read & VARYING_BIT_COL0)
      emitColor = GL_TRUE;
   else
      emitColor = GL_FALSE;

   /* determine how many enabled sets of texcoords */
   numTexCoords = 0;
   for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
      if (ctx->Texture.Unit[i]._Current &&
          ctx->Texture.Unit[i]._Current->Target == GL_TEXTURE_2D) {
         numTexCoords++;
      }
   }

   /* total number of attributes per vertex */
   numAttribs = 1 + emitColor + numTexCoords;

   /* load vertex buffer */
   {
#define SET_ATTRIB(VERT, ATTR, X, Y, Z, W)                              \
      do {                                                              \
         GLuint k = (((VERT) * numAttribs + (ATTR)) * 4);               \
         assert(k < 4 * 4 * numAttribs);                                \
         vbuf[k + 0] = X;                                               \
         vbuf[k + 1] = Y;                                               \
         vbuf[k + 2] = Z;                                               \
         vbuf[k + 3] = W;                                               \
      } while (0)

      const GLfloat x0 = x, y0 = y, x1 = x + width, y1 = y + height;
      GLfloat *vbuf = NULL;
      GLuint tex_attr;

      u_upload_alloc(pipe->stream_uploader, 0,
                     numAttribs * 4 * 4 * sizeof(GLfloat), 4,
                     &offset, &vbuffer, (void **) &vbuf);
      if (!vbuffer) {
         return;
      }

      z = SATURATE(z);

      /* positions (in clip coords) */
      {
         const struct gl_framebuffer *fb = ctx->DrawBuffer;
         const GLfloat fb_width = (GLfloat)_mesa_geometric_width(fb);
         const GLfloat fb_height = (GLfloat)_mesa_geometric_height(fb);

         const GLfloat clip_x0 = (GLfloat)(x0 / fb_width * 2.0 - 1.0);
         const GLfloat clip_y0 = (GLfloat)(y0 / fb_height * 2.0 - 1.0);
         const GLfloat clip_x1 = (GLfloat)(x1 / fb_width * 2.0 - 1.0);
         const GLfloat clip_y1 = (GLfloat)(y1 / fb_height * 2.0 - 1.0);

         SET_ATTRIB(0, 0, clip_x0, clip_y0, z, 1.0f);   /* lower left */
         SET_ATTRIB(1, 0, clip_x1, clip_y0, z, 1.0f);   /* lower right */
         SET_ATTRIB(2, 0, clip_x1, clip_y1, z, 1.0f);   /* upper right */
         SET_ATTRIB(3, 0, clip_x0, clip_y1, z, 1.0f);   /* upper left */

         slots[0] = VARYING_SLOT_POS;
      }

      /* colors */
      if (emitColor) {
         const GLfloat *c = ctx->Current.Attrib[VERT_ATTRIB_COLOR0];
         SET_ATTRIB(0, 1, c[0], c[1], c[2], c[3]);
         SET_ATTRIB(1, 1, c[0], c[1], c[2], c[3]);
         SET_ATTRIB(2, 1, c[0], c[1], c[2], c[3]);
         SET_ATTRIB(3, 1, c[0], c[1], c[2], c[3]);
         slots[1] = VARYING_SLOT_COL0;
         tex_attr = 2;
      }
      else {
         tex_attr = 1;
      }

      /* texcoords */
      for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
         if (ctx->Texture.Unit[i]._Current &&
             ctx->Texture.Unit[i]._Current->Target == GL_TEXTURE_2D) {
            struct gl_texture_object *obj = ctx->Texture.Unit[i]._Current;
            const struct gl_texture_image *img = _mesa_base_tex_image(obj);
            const GLfloat wt = (GLfloat) img->Width;
            const GLfloat ht = (GLfloat) img->Height;
            const GLfloat s0 = obj->CropRect[0] / wt;
            const GLfloat t0 = obj->CropRect[1] / ht;
            const GLfloat s1 = (obj->CropRect[0] + obj->CropRect[2]) / wt;
            const GLfloat t1 = (obj->CropRect[1] + obj->CropRect[3]) / ht;

            /*printf("crop texcoords: %g, %g .. %g, %g\n", s0, t0, s1, t1);*/
            SET_ATTRIB(0, tex_attr, s0, t0, 0.0f, 1.0f);  /* lower left */
            SET_ATTRIB(1, tex_attr, s1, t0, 0.0f, 1.0f);  /* lower right */
            SET_ATTRIB(2, tex_attr, s1, t1, 0.0f, 1.0f);  /* upper right */
            SET_ATTRIB(3, tex_attr, s0, t1, 0.0f, 1.0f);  /* upper left */

            slots[tex_attr] = st->needs_texcoord_semantic ?
               VARYING_SLOT_TEX0 : VARYING_SLOT_VAR0;

            tex_attr++;
         }
      }

      u_upload_unmap(pipe->stream_uploader);

#undef SET_ATTRIB
   }

   cso_save_state(cso, (CSO_BIT_VIEWPORT |
                        CSO_BIT_STREAM_OUTPUTS |
                        CSO_BIT_VERTEX_SHADER |
                        CSO_BIT_TESSCTRL_SHADER |
                        CSO_BIT_TESSEVAL_SHADER |
                        CSO_BIT_GEOMETRY_SHADER |
                        CSO_BIT_VERTEX_ELEMENTS));

   {
      void *vs = lookup_shader(st, numAttribs, slots);
      cso_set_vertex_shader_handle(cso, vs);
   }
   cso_set_tessctrl_shader_handle(cso, NULL);
   cso_set_tesseval_shader_handle(cso, NULL);
   cso_set_geometry_shader_handle(cso, NULL);

   for (i = 0; i < numAttribs; i++) {
      velems.velems[i].src_offset = i * 4 * sizeof(float);
      velems.velems[i].instance_divisor = 0;
      velems.velems[i].vertex_buffer_index = 0;
      velems.velems[i].src_format = PIPE_FORMAT_R32G32B32A32_FLOAT;
      velems.velems[i].dual_slot = false;
      velems.velems[i].src_stride = numAttribs * 4 * sizeof(float);
   }
   velems.count = numAttribs;

   cso_set_vertex_elements(cso, &velems);
   cso_set_stream_outputs(cso, 0, NULL, NULL);

   /* viewport state: viewport matching window dims */
   {
      const struct gl_framebuffer *fb = ctx->DrawBuffer;
      const GLboolean invert = (_mesa_fb_orientation(fb) == Y_0_TOP);
      const GLfloat width = (GLfloat)_mesa_geometric_width(fb);
      const GLfloat height = (GLfloat)_mesa_geometric_height(fb);
      struct pipe_viewport_state vp;
      vp.scale[0] =  0.5f * width;
      vp.scale[1] = height * (invert ? -0.5f : 0.5f);
      vp.scale[2] = 1.0f;
      vp.translate[0] = 0.5f * width;
      vp.translate[1] = 0.5f * height;
      vp.translate[2] = 0.0f;
      vp.swizzle_x = PIPE_VIEWPORT_SWIZZLE_POSITIVE_X;
      vp.swizzle_y = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Y;
      vp.swizzle_z = PIPE_VIEWPORT_SWIZZLE_POSITIVE_Z;
      vp.swizzle_w = PIPE_VIEWPORT_SWIZZLE_POSITIVE_W;
      cso_set_viewport(cso, &vp);
   }

   util_draw_vertex_buffer(pipe, cso, vbuffer,
                           offset,  /* offset */
                           MESA_PRIM_TRIANGLE_FAN,
                           4,  /* verts */
                           numAttribs); /* attribs/vert */
   st->last_num_vbuffers = MAX2(st->last_num_vbuffers, 1);

   pipe_resource_reference(&vbuffer, NULL);

   /* restore state */
   cso_restore_state(cso, 0);
   ctx->Array.NewVertexElements = true;
   ctx->NewDriverState |= ST_NEW_VERTEX_ARRAYS;
}

/**
 * Free any cached shaders
 */
void
st_destroy_drawtex(struct st_context *st)
{
   GLuint i;
   for (i = 0; i < NumCachedShaders; i++) {
      st->pipe->delete_vs_state(st->pipe, CachedShaders[i].handle);
   }
   NumCachedShaders = 0;
}
