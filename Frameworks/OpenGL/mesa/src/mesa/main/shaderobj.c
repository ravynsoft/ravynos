/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2004-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009-2010  VMware, Inc.  All Rights Reserved.
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
 * \file shaderobj.c
 * \author Brian Paul
 *
 */


#include "compiler/glsl/string_to_uint_map.h"
#include "util/glheader.h"
#include "main/context.h"
#include "main/glspirv.h"
#include "main/hash.h"
#include "main/mtypes.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "program/program.h"
#include "program/prog_parameter.h"
#include "util/ralloc.h"
#include "util/u_atomic.h"

/**********************************************************************/
/*** Shader object functions                                        ***/
/**********************************************************************/


/**
 * Set ptr to point to sh.
 * If ptr is pointing to another shader, decrement its refcount (and delete
 * if refcount hits zero).
 * Then set ptr to point to sh, incrementing its refcount.
 */
static void
_reference_shader(struct gl_context *ctx, struct gl_shader **ptr,
                       struct gl_shader *sh, bool skip_locking)
{
   assert(ptr);
   if (*ptr == sh) {
      /* no-op */
      return;
   }
   if (*ptr) {
      /* Unreference the old shader */
      struct gl_shader *old = *ptr;

      assert(old->RefCount > 0);

      if (p_atomic_dec_zero(&old->RefCount)) {
         if (old->Name != 0) {
            if (skip_locking)
               _mesa_HashRemoveLocked(ctx->Shared->ShaderObjects, old->Name);
            else
               _mesa_HashRemove(ctx->Shared->ShaderObjects, old->Name);
         }
         _mesa_delete_shader(ctx, old);
      }

      *ptr = NULL;
   }
   assert(!*ptr);

   if (sh) {
      /* reference new */
      p_atomic_inc(&sh->RefCount);
      *ptr = sh;
   }
}

void
_mesa_reference_shader(struct gl_context *ctx, struct gl_shader **ptr,
                       struct gl_shader *sh)
{
   _reference_shader(ctx, ptr, sh, false);
}

static void
_mesa_init_shader(struct gl_shader *shader)
{
   shader->RefCount = 1;
   shader->info.Geom.VerticesOut = -1;
   shader->info.Geom.InputType = MESA_PRIM_TRIANGLES;
   shader->info.Geom.OutputType = MESA_PRIM_TRIANGLE_STRIP;
}

/**
 * Allocate a new gl_shader object, initialize it.
 */
struct gl_shader *
_mesa_new_shader(GLuint name, gl_shader_stage stage)
{
   struct gl_shader *shader;
   shader = rzalloc(NULL, struct gl_shader);
   if (shader) {
      shader->Stage = stage;
      shader->Name = name;
      _mesa_init_shader(shader);
   }
   return shader;
}


/**
 * Delete a shader object.
 */
void
_mesa_delete_shader(struct gl_context *ctx, struct gl_shader *sh)
{
   _mesa_shader_spirv_data_reference(&sh->spirv_data, NULL);
   free((void *)sh->Source);
   free((void *)sh->FallbackSource);
   free(sh->Label);
   ralloc_free(sh);
}


/**
 * Delete a shader object.
 */
void
_mesa_delete_linked_shader(struct gl_context *ctx,
                           struct gl_linked_shader *sh)
{
   _mesa_shader_spirv_data_reference(&sh->spirv_data, NULL);
   _mesa_reference_program(ctx, &sh->Program, NULL);
   ralloc_free(sh);
}


/**
 * Lookup a GLSL shader object.
 */
struct gl_shader *
_mesa_lookup_shader(struct gl_context *ctx, GLuint name)
{
   if (name) {
      struct gl_shader *sh = (struct gl_shader *)
         _mesa_HashLookup(ctx->Shared->ShaderObjects, name);
      /* Note that both gl_shader and gl_shader_program objects are kept
       * in the same hash table.  Check the object's type to be sure it's
       * what we're expecting.
       */
      if (sh && sh->Type == GL_SHADER_PROGRAM_MESA) {
         return NULL;
      }
      return sh;
   }
   return NULL;
}


/**
 * As above, but record an error if shader is not found.
 */
struct gl_shader *
_mesa_lookup_shader_err(struct gl_context *ctx, GLuint name, const char *caller)
{
   if (!name) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s", caller);
      return NULL;
   }
   else {
      struct gl_shader *sh = (struct gl_shader *)
         _mesa_HashLookup(ctx->Shared->ShaderObjects, name);
      if (!sh) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s", caller);
         return NULL;
      }
      if (sh->Type == GL_SHADER_PROGRAM_MESA) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s", caller);
         return NULL;
      }
      return sh;
   }
}



/**********************************************************************/
/*** Shader Program object functions                                ***/
/**********************************************************************/

void
_mesa_reference_shader_program_data(struct gl_shader_program_data **ptr,
                                    struct gl_shader_program_data *data)
{
   if (*ptr == data)
      return;

   if (*ptr) {
      struct gl_shader_program_data *oldData = *ptr;

      assert(oldData->RefCount > 0);

      if (p_atomic_dec_zero(&oldData->RefCount)) {
         assert(oldData->NumUniformStorage == 0 ||
                oldData->UniformStorage);

         for (unsigned i = 0; i < oldData->NumUniformStorage; ++i)
            _mesa_uniform_detach_all_driver_storage(&oldData->UniformStorage[i]);

         ralloc_free(oldData);
      }

      *ptr = NULL;
   }

   if (data)
      p_atomic_inc(&data->RefCount);

   *ptr = data;
}

/**
 * Set ptr to point to shProg.
 * If ptr is pointing to another object, decrement its refcount (and delete
 * if refcount hits zero).
 * Then set ptr to point to shProg, incrementing its refcount.
 */
void
_mesa_reference_shader_program_(struct gl_context *ctx,
                                struct gl_shader_program **ptr,
                                struct gl_shader_program *shProg)
{
   assert(ptr);
   if (*ptr == shProg) {
      /* no-op */
      return;
   }
   if (*ptr) {
      /* Unreference the old shader program */
      struct gl_shader_program *old = *ptr;

      assert(old->RefCount > 0);

      if (p_atomic_dec_zero(&old->RefCount)) {
         _mesa_HashLockMutex(ctx->Shared->ShaderObjects);
         if (old->Name != 0)
	         _mesa_HashRemoveLocked(ctx->Shared->ShaderObjects, old->Name);
         _mesa_delete_shader_program(ctx, old);
         _mesa_HashUnlockMutex(ctx->Shared->ShaderObjects);
      }

      *ptr = NULL;
   }
   assert(!*ptr);

   if (shProg) {
      p_atomic_inc(&shProg->RefCount);
      *ptr = shProg;
   }
}

struct gl_shader_program_data *
_mesa_create_shader_program_data()
{
   struct gl_shader_program_data *data;
   data = rzalloc(NULL, struct gl_shader_program_data);
   if (data) {
      data->RefCount = 1;
      data->InfoLog = ralloc_strdup(data, "");
   }

   return data;
}

static void
init_shader_program(struct gl_shader_program *prog)
{
   prog->Type = GL_SHADER_PROGRAM_MESA;
   prog->RefCount = 1;

   prog->AttributeBindings = string_to_uint_map_ctor();
   prog->FragDataBindings = string_to_uint_map_ctor();
   prog->FragDataIndexBindings = string_to_uint_map_ctor();

   prog->Geom.UsesEndPrimitive = false;
   prog->Geom.ActiveStreamMask = 0;

   prog->TransformFeedback.BufferMode = GL_INTERLEAVED_ATTRIBS;

   exec_list_make_empty(&prog->EmptyUniformLocations);
}

/**
 * Allocate a new gl_shader_program object, initialize it.
 */
struct gl_shader_program *
_mesa_new_shader_program(GLuint name)
{
   struct gl_shader_program *shProg;
   shProg = rzalloc(NULL, struct gl_shader_program);
   if (shProg) {
      shProg->Name = name;
      shProg->data = _mesa_create_shader_program_data();
      if (!shProg->data) {
         ralloc_free(shProg);
         return NULL;
      }
      init_shader_program(shProg);
   }
   return shProg;
}


/**
 * Clear (free) the shader program state that gets produced by linking.
 */
void
_mesa_clear_shader_program_data(struct gl_context *ctx,
                                struct gl_shader_program *shProg)
{
   for (gl_shader_stage sh = 0; sh < MESA_SHADER_STAGES; sh++) {
      if (shProg->_LinkedShaders[sh] != NULL) {
         _mesa_delete_linked_shader(ctx, shProg->_LinkedShaders[sh]);
         shProg->_LinkedShaders[sh] = NULL;
      }
   }

   if (shProg->UniformRemapTable) {
      ralloc_free(shProg->UniformRemapTable);
      shProg->NumUniformRemapTable = 0;
      shProg->UniformRemapTable = NULL;
   }

   if (shProg->UniformHash) {
      string_to_uint_map_dtor(shProg->UniformHash);
      shProg->UniformHash = NULL;
   }

   if (shProg->data)
      _mesa_program_resource_hash_destroy(shProg);

   _mesa_reference_shader_program_data(&shProg->data, NULL);
}


/**
 * Free all the data that hangs off a shader program object, but not the
 * object itself.
 * Must be called with shared->ShaderObjects locked.
 */
void
_mesa_free_shader_program_data(struct gl_context *ctx,
                               struct gl_shader_program *shProg)
{
   GLuint i;

   assert(shProg->Type == GL_SHADER_PROGRAM_MESA);

   _mesa_clear_shader_program_data(ctx, shProg);

   if (shProg->AttributeBindings) {
      string_to_uint_map_dtor(shProg->AttributeBindings);
      shProg->AttributeBindings = NULL;
   }

   if (shProg->FragDataBindings) {
      string_to_uint_map_dtor(shProg->FragDataBindings);
      shProg->FragDataBindings = NULL;
   }

   if (shProg->FragDataIndexBindings) {
      string_to_uint_map_dtor(shProg->FragDataIndexBindings);
      shProg->FragDataIndexBindings = NULL;
   }

   /* detach shaders */
   for (i = 0; i < shProg->NumShaders; i++) {
      _reference_shader(ctx, &shProg->Shaders[i], NULL, true);
   }
   shProg->NumShaders = 0;

   free(shProg->Shaders);
   shProg->Shaders = NULL;

   /* Transform feedback varying vars */
   for (i = 0; i < shProg->TransformFeedback.NumVarying; i++) {
      free(shProg->TransformFeedback.VaryingNames[i]);
   }
   free(shProg->TransformFeedback.VaryingNames);
   shProg->TransformFeedback.VaryingNames = NULL;
   shProg->TransformFeedback.NumVarying = 0;

   free(shProg->Label);
   shProg->Label = NULL;
}


/**
 * Free/delete a shader program object.
 */
void
_mesa_delete_shader_program(struct gl_context *ctx,
                            struct gl_shader_program *shProg)
{
   _mesa_free_shader_program_data(ctx, shProg);
   ralloc_free(shProg);
}


/**
 * Lookup a GLSL program object.
 */
struct gl_shader_program *
_mesa_lookup_shader_program(struct gl_context *ctx, GLuint name)
{
   struct gl_shader_program *shProg;
   if (name) {
      shProg = (struct gl_shader_program *)
         _mesa_HashLookup(ctx->Shared->ShaderObjects, name);
      /* Note that both gl_shader and gl_shader_program objects are kept
       * in the same hash table.  Check the object's type to be sure it's
       * what we're expecting.
       */
      if (shProg && shProg->Type != GL_SHADER_PROGRAM_MESA) {
         return NULL;
      }
      return shProg;
   }
   return NULL;
}


/**
 * As above, but record an error if program is not found.
 */
struct gl_shader_program *
_mesa_lookup_shader_program_err_glthread(struct gl_context *ctx, GLuint name,
                                         bool glthread, const char *caller)
{
   if (!name) {
      _mesa_error_glthread_safe(ctx, GL_INVALID_VALUE, glthread, "%s", caller);
      return NULL;
   }
   else {
      struct gl_shader_program *shProg = (struct gl_shader_program *)
         _mesa_HashLookup(ctx->Shared->ShaderObjects, name);
      if (!shProg) {
         _mesa_error_glthread_safe(ctx, GL_INVALID_VALUE, glthread,
                                   "%s", caller);
         return NULL;
      }
      if (shProg->Type != GL_SHADER_PROGRAM_MESA) {
         _mesa_error_glthread_safe(ctx, GL_INVALID_OPERATION, glthread,
                                   "%s", caller);
         return NULL;
      }
      return shProg;
   }
}


struct gl_shader_program *
_mesa_lookup_shader_program_err(struct gl_context *ctx, GLuint name,
                                const char *caller)
{
   return _mesa_lookup_shader_program_err_glthread(ctx, name, false, caller);
}
