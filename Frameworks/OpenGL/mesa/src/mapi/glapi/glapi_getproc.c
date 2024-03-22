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
 * \file glapi_getproc.c
 *
 * Code for implementing glXGetProcAddress(), etc.
 * This was originally in glapi.c but refactored out.
 */


#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "glapi/glapi_priv.h"
#include "glapitable.h"


/**********************************************************************
 * Static function management.
 */


#if !defined(DISPATCH_FUNCTION_SIZE)
# define NEED_FUNCTION_POINTER
#endif
#include "glprocs.h"


/**
 * Search the table of static entrypoint functions for the named function
 * and return the corresponding glprocs_table_t entry.
 */
static const glprocs_table_t *
get_static_proc(const char *n)
{
   GLuint i;
   for (i = 0; static_functions[i].Name_offset >= 0; i++) {
      const char *testName = gl_string_table + static_functions[i].Name_offset;
      if (strcmp(testName, n) == 0)
      {
         return &static_functions[i];
      }
   }
   return NULL;
}


/**
 * Return dispatch table offset of the named static (built-in) function.
 * Return -1 if function not found.
 */
static GLint
get_static_proc_offset(const char *funcName)
{
   const glprocs_table_t *const f = get_static_proc(funcName);
   if (f == NULL) {
      return -1;
   }

   return f->Offset;
}


/**********************************************************************
 * Extension function management.
 */

/**
 * Initializes the glapi relocs table, and returns the offset of the given
 * function in the dispatch table.
 */
int
_glapi_add_dispatch(const char *funcName)
{
   init_glapi_relocs_once();

   return get_static_proc_offset(funcName);
}

/**
 * Return offset of entrypoint for named function within dispatch table.
 */
GLint
_glapi_get_proc_offset(const char *funcName)
{
   /* search static functions */
   return get_static_proc_offset(funcName);
}



/**
 * Return dispatch function address for the named static (built-in) function.
 * Return NULL if function not found.
 */
_glapi_proc
_glapi_get_proc_address(const char *funcName)
{
   init_glapi_relocs_once();

   if (!funcName || funcName[0] != 'g' || funcName[1] != 'l')
      return NULL;

   const glprocs_table_t *const f = get_static_proc(funcName);
   if (f == NULL) {
      return NULL;
   }

#if defined(DISPATCH_FUNCTION_SIZE) && defined(GLX_INDIRECT_RENDERING)
   return (f->Address == NULL)
      ? get_entrypoint_address(f->Offset)
      : f->Address;
#elif defined(DISPATCH_FUNCTION_SIZE)
   return get_entrypoint_address(f->Offset);
#else
   return f->Address;
#endif
}



/**
 * Return the name of the function at the given dispatch offset.
 * This is only intended for debugging.
 */
const char *
_glapi_get_proc_name(GLuint offset)
{
   GLuint i;
   for (i = 0; static_functions[i].Name_offset >= 0; i++) {
      if (static_functions[i].Offset == offset) {
         return gl_string_table + static_functions[i].Name_offset;
      }
   }
   return NULL;
}



/**********************************************************************
 * GL API table functions.
 */


/**
 * Return size of dispatch table struct as number of functions (or
 * slots).
 */
GLuint
_glapi_get_dispatch_table_size(void)
{
   return sizeof(struct _glapi_table) / sizeof(void *);
}
