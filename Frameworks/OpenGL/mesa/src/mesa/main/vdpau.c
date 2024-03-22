/**************************************************************************
 *
 * Copyright 2013 Advanced Micro Devices, Inc.
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
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * Authors:
 *      Christian König <christian.koenig@amd.com>
 *
 */

#include <stdbool.h>
#include "util/hash_table.h"
#include "util/set.h"
#include "util/u_memory.h"
#include "context.h"
#include "glformats.h"
#include "texobj.h"
#include "teximage.h"
#include "textureview.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_vdpau.h"

#define MAX_TEXTURES 4

struct vdp_surface
{
   GLenum target;
   struct gl_texture_object *textures[MAX_TEXTURES];
   GLenum access, state;
   GLboolean output;
   const GLvoid *vdpSurface;
};

void GLAPIENTRY
_mesa_VDPAUInitNV(const GLvoid *vdpDevice, const GLvoid *getProcAddress)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!vdpDevice) {
      _mesa_error(ctx, GL_INVALID_VALUE, "vdpDevice");
      return;
   }

   if (!getProcAddress) {
      _mesa_error(ctx, GL_INVALID_VALUE, "getProcAddress");
      return;
   }

   if (ctx->vdpDevice || ctx->vdpGetProcAddress || ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUInitNV");
      return;
   }

   ctx->vdpDevice = vdpDevice;
   ctx->vdpGetProcAddress = getProcAddress;
   ctx->vdpSurfaces = _mesa_set_create(NULL, _mesa_hash_pointer,
                                       _mesa_key_pointer_equal);
}

static void
unregister_surface(struct set_entry *entry)
{
   struct vdp_surface *surf = (struct vdp_surface *)entry->key;
   GET_CURRENT_CONTEXT(ctx);
   
   if (surf->state == GL_SURFACE_MAPPED_NV) {
      GLintptr surfaces[] = { (GLintptr)surf };
      _mesa_VDPAUUnmapSurfacesNV(1, surfaces);
   }

   _mesa_set_remove(ctx->vdpSurfaces, entry);
   FREE(surf);
}

void GLAPIENTRY
_mesa_VDPAUFiniNV(void)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUFiniNV");
      return;
   }

   _mesa_set_destroy(ctx->vdpSurfaces, unregister_surface);

   ctx->vdpDevice = 0;
   ctx->vdpGetProcAddress = 0;
   ctx->vdpSurfaces = NULL;
}

static GLintptr
register_surface(struct gl_context *ctx, GLboolean isOutput,
                 const GLvoid *vdpSurface, GLenum target,
                 GLsizei numTextureNames, const GLuint *textureNames)
{
   struct vdp_surface *surf;
   int i;

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAURegisterSurfaceNV");
      return (GLintptr)NULL;
   }

   if (target != GL_TEXTURE_2D && target != GL_TEXTURE_RECTANGLE) {
      _mesa_error(ctx, GL_INVALID_ENUM, "VDPAURegisterSurfaceNV");
      return (GLintptr)NULL;
   }

   if (target == GL_TEXTURE_RECTANGLE && !ctx->Extensions.NV_texture_rectangle) {
      _mesa_error(ctx, GL_INVALID_ENUM, "VDPAURegisterSurfaceNV");
      return (GLintptr)NULL;
   }

   surf = CALLOC_STRUCT( vdp_surface );
   if (surf == NULL) {
      _mesa_error_no_memory("VDPAURegisterSurfaceNV");
      return (GLintptr)NULL;
   }

   surf->vdpSurface = vdpSurface;
   surf->target = target;
   surf->access = GL_READ_WRITE;
   surf->state = GL_SURFACE_REGISTERED_NV;
   surf->output = isOutput;
   for (i = 0; i < numTextureNames; ++i) {
      struct gl_texture_object *tex;

      tex = _mesa_lookup_texture_err(ctx, textureNames[i],
                                     "VDPAURegisterSurfaceNV");
      if (tex == NULL) {
         free(surf);
         return (GLintptr)NULL;
      }

      _mesa_lock_texture(ctx, tex);

      if (tex->Immutable) {
         _mesa_unlock_texture(ctx, tex);
         free(surf);
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "VDPAURegisterSurfaceNV(texture is immutable)");
         return (GLintptr)NULL;
      }

      if (tex->Target == 0) {
         tex->Target = target;
         tex->TargetIndex = _mesa_tex_target_to_index(ctx, target);
      } else if (tex->Target != target) {
         _mesa_unlock_texture(ctx, tex);
         free(surf);
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "VDPAURegisterSurfaceNV(target mismatch)");
         return (GLintptr)NULL;
      }

      /* This will disallow respecifying the storage. */
      _mesa_set_texture_view_state(ctx, tex, target, 1);
      _mesa_unlock_texture(ctx, tex);

      _mesa_reference_texobj(&surf->textures[i], tex);
   }

   _mesa_set_add(ctx->vdpSurfaces, surf);

   return (GLintptr)surf;
}

GLintptr GLAPIENTRY
_mesa_VDPAURegisterVideoSurfaceNV(const GLvoid *vdpSurface, GLenum target,
                                  GLsizei numTextureNames,
                                  const GLuint *textureNames)
{
   GET_CURRENT_CONTEXT(ctx);

   if (numTextureNames != 4) {
      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAURegisterVideoSurfaceNV");
      return (GLintptr)NULL;
   }

   return register_surface(ctx, false, vdpSurface, target,
                           numTextureNames, textureNames);
}

GLintptr GLAPIENTRY
_mesa_VDPAURegisterOutputSurfaceNV(const GLvoid *vdpSurface, GLenum target,
                                   GLsizei numTextureNames,
                                   const GLuint *textureNames)
{
   GET_CURRENT_CONTEXT(ctx);

   if (numTextureNames != 1) {
      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAURegisterVideoSurfaceNV");
      return (GLintptr)NULL;
   }

   return register_surface(ctx, true, vdpSurface, target,
                           numTextureNames, textureNames);
}

GLboolean GLAPIENTRY
_mesa_VDPAUIsSurfaceNV(GLintptr surface)
{
   struct vdp_surface *surf = (struct vdp_surface *)surface;
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUIsSurfaceNV");
      return false;
   }

   if (!_mesa_set_search(ctx->vdpSurfaces, surf)) {
      return false;
   }

   return true;
}

void GLAPIENTRY
_mesa_VDPAUUnregisterSurfaceNV(GLintptr surface)
{
   struct vdp_surface *surf = (struct vdp_surface *)surface;
   struct set_entry *entry;
   int i;
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUUnregisterSurfaceNV");
      return;
   }

   /* according to the spec it's ok when this is zero */
   if (surface == 0)
      return;

   entry = _mesa_set_search(ctx->vdpSurfaces, surf);
   if (!entry) {
      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUUnregisterSurfaceNV");
      return;
   }

   for (i = 0; i < MAX_TEXTURES; i++) {
      if (surf->textures[i]) {
         surf->textures[i]->Immutable = GL_FALSE;
         _mesa_reference_texobj(&surf->textures[i], NULL);
      }
   }

   _mesa_set_remove(ctx->vdpSurfaces, entry);
   free(surf);
}

void GLAPIENTRY
_mesa_VDPAUGetSurfaceivNV(GLintptr surface, GLenum pname, GLsizei bufSize,
                          GLsizei *length, GLint *values)
{
   struct vdp_surface *surf = (struct vdp_surface *)surface;
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUGetSurfaceivNV");
      return;
   }

   if (!_mesa_set_search(ctx->vdpSurfaces, surf)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUGetSurfaceivNV");
      return;
   }

   if (pname != GL_SURFACE_STATE_NV) {
      _mesa_error(ctx, GL_INVALID_ENUM, "VDPAUGetSurfaceivNV");
      return;
   }

   if (bufSize < 1) {
      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUGetSurfaceivNV");
      return;
   }

   values[0] = surf->state;

   if (length != NULL)
      *length = 1;
}

void GLAPIENTRY
_mesa_VDPAUSurfaceAccessNV(GLintptr surface, GLenum access)
{
   struct vdp_surface *surf = (struct vdp_surface *)surface;
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUSurfaceAccessNV");
      return;
   }

   if (!_mesa_set_search(ctx->vdpSurfaces, surf)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUSurfaceAccessNV");
      return;
   }

   if (access != GL_READ_ONLY && access != GL_WRITE_ONLY &&
       access != GL_READ_WRITE) {

      _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUSurfaceAccessNV");
      return;
   }

   if (surf->state == GL_SURFACE_MAPPED_NV) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUSurfaceAccessNV");
      return;
   }

   surf->access = access;
}

void GLAPIENTRY
_mesa_VDPAUMapSurfacesNV(GLsizei numSurfaces, const GLintptr *surfaces)
{
   GET_CURRENT_CONTEXT(ctx);
   int i;

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUUnmapSurfacesNV");
      return;
   }

   for (i = 0; i < numSurfaces; ++i) {
      struct vdp_surface *surf = (struct vdp_surface *)surfaces[i];

      if (!_mesa_set_search(ctx->vdpSurfaces, surf)) {
         _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUSurfaceAccessNV");
         return;
      }

      if (surf->state == GL_SURFACE_MAPPED_NV) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUSurfaceAccessNV");
         return;
      }
   }

   for (i = 0; i < numSurfaces; ++i) {
      struct vdp_surface *surf = (struct vdp_surface *)surfaces[i];
      unsigned numTextureNames = surf->output ? 1 : 4;
      unsigned j;

      for (j = 0; j < numTextureNames; ++j) {
         struct gl_texture_object *tex = surf->textures[j];
         struct gl_texture_image *image;

         _mesa_lock_texture(ctx, tex);
         image = _mesa_get_tex_image(ctx, tex, surf->target, 0);
         if (!image) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "VDPAUMapSurfacesNV");
            _mesa_unlock_texture(ctx, tex);
            return;
         }

         st_FreeTextureImageBuffer(ctx, image);

         st_vdpau_map_surface(ctx, surf->target, surf->access,
                              surf->output, tex, image,
                              surf->vdpSurface, j);

         _mesa_unlock_texture(ctx, tex);
      }
      surf->state = GL_SURFACE_MAPPED_NV;
   }
}

void GLAPIENTRY
_mesa_VDPAUUnmapSurfacesNV(GLsizei numSurfaces, const GLintptr *surfaces)
{
   GET_CURRENT_CONTEXT(ctx);
   int i;

   if (!ctx->vdpDevice || !ctx->vdpGetProcAddress || !ctx->vdpSurfaces) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUUnmapSurfacesNV");
      return;
   }

   for (i = 0; i < numSurfaces; ++i) {
      struct vdp_surface *surf = (struct vdp_surface *)surfaces[i];

      if (!_mesa_set_search(ctx->vdpSurfaces, surf)) {
         _mesa_error(ctx, GL_INVALID_VALUE, "VDPAUSurfaceAccessNV");
         return;
      }

      if (surf->state != GL_SURFACE_MAPPED_NV) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "VDPAUSurfaceAccessNV");
         return;
      }
   }

   for (i = 0; i < numSurfaces; ++i) {
      struct vdp_surface *surf = (struct vdp_surface *)surfaces[i];
      unsigned numTextureNames = surf->output ? 1 : 4;
      unsigned j;

      for (j = 0; j < numTextureNames; ++j) {
         struct gl_texture_object *tex = surf->textures[j];
         struct gl_texture_image *image;

         _mesa_lock_texture(ctx, tex);

         image = _mesa_select_tex_image(tex, surf->target, 0);

         st_vdpau_unmap_surface(ctx, surf->target, surf->access,
                                surf->output, tex, image,
                                surf->vdpSurface, j);

         if (image)
            st_FreeTextureImageBuffer(ctx, image);

         _mesa_unlock_texture(ctx, tex);
      }
      surf->state = GL_SURFACE_REGISTERED_NV;
   }
}
