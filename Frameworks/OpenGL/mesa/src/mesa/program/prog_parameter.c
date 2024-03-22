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
 * \file prog_parameter.c
 * Program parameter lists and functions.
 * \author Brian Paul
 */

#include "util/glheader.h"
#include "main/macros.h"
#include "main/errors.h"
#include "util/u_memory.h"
#include "prog_instruction.h"
#include "prog_parameter.h"
#include "prog_statevars.h"


/**
 * Look for a float vector in the given parameter list.  The float vector
 * may be of length 1, 2, 3 or 4.  If swizzleOut is non-null, we'll try
 * swizzling to find a match.
 * \param list  the parameter list to search
 * \param v  the float vector to search for
 * \param vSize  number of element in v
 * \param posOut  returns the position of the constant, if found
 * \param swizzleOut  returns a swizzle mask describing location of the
 *                    vector elements if found.
 * \return GL_TRUE if found, GL_FALSE if not found
 */
static GLboolean
lookup_parameter_constant(const struct gl_program_parameter_list *list,
                          const gl_constant_value v[], GLuint vSize,
                          GLint *posOut, GLuint *swizzleOut)
{
   GLuint i;

   assert(vSize >= 1);
   assert(vSize <= 4);

   if (!list) {
      *posOut = -1;
      return GL_FALSE;
   }

   for (i = 0; i < list->NumParameters; i++) {
      if (list->Parameters[i].Type == PROGRAM_CONSTANT) {
         unsigned offset = list->Parameters[i].ValueOffset;

         if (!swizzleOut) {
            /* swizzle not allowed */
            GLuint j, match = 0;
            for (j = 0; j < vSize; j++) {
               if (v[j].u == list->ParameterValues[offset + j].u)
                  match++;
            }
            if (match == vSize) {
               *posOut = i;
               return GL_TRUE;
            }
         }
         else {
            /* try matching w/ swizzle */
             if (vSize == 1) {
                /* look for v[0] anywhere within float[4] value */
                GLuint j;
                for (j = 0; j < list->Parameters[i].Size; j++) {
                   if (list->ParameterValues[offset + j].u == v[0].u) {
                      /* found it */
                      *posOut = i;
                      *swizzleOut = MAKE_SWIZZLE4(j, j, j, j);
                      return GL_TRUE;
                   }
                }
             }
             else if (vSize <= list->Parameters[i].Size) {
                /* see if we can match this constant (with a swizzle) */
                GLuint swz[4];
                GLuint match = 0, j, k;
                for (j = 0; j < vSize; j++) {
                   if (v[j].u == list->ParameterValues[offset + j].u) {
                      swz[j] = j;
                      match++;
                   }
                   else {
                      for (k = 0; k < list->Parameters[i].Size; k++) {
                         if (v[j].u == list->ParameterValues[offset + k].u) {
                            swz[j] = k;
                            match++;
                            break;
                         }
                      }
                   }
                }
                /* smear last value to remaining positions */
                for (; j < 4; j++)
                   swz[j] = swz[j-1];

                if (match == vSize) {
                   *posOut = i;
                   *swizzleOut = MAKE_SWIZZLE4(swz[0], swz[1], swz[2], swz[3]);
                   return GL_TRUE;
                }
             }
         }
      }
   }

   *posOut = -1;
   return GL_FALSE;
}


struct gl_program_parameter_list *
_mesa_new_parameter_list(void)
{
   struct gl_program_parameter_list *list =
      CALLOC_STRUCT(gl_program_parameter_list);
   if (!list)
      return NULL;

   list->UniformBytes = 0;
   list->FirstStateVarIndex = INT_MAX;
   list->LastStateVarIndex = 0;
   return list;
}


struct gl_program_parameter_list *
_mesa_new_parameter_list_sized(unsigned size)
{
   struct gl_program_parameter_list *p = _mesa_new_parameter_list();


   if ((p != NULL) && (size != 0)) {
      _mesa_reserve_parameter_storage(p, size, size);

      if ((p->Parameters == NULL) || (p->ParameterValues == NULL)) {
         free(p->Parameters);
         align_free(p->ParameterValues);
         free(p);
         p = NULL;
      }
   }

   return p;
}


/**
 * Free a parameter list and all its parameters
 */
void
_mesa_free_parameter_list(struct gl_program_parameter_list *paramList)
{
   GLuint i;
   for (i = 0; i < paramList->NumParameters; i++) {
      free((void *)paramList->Parameters[i].Name);
   }
   free(paramList->Parameters);
   align_free(paramList->ParameterValues);
   FREE(paramList);
}


/**
 * Make sure there are enough unused parameter slots. Reallocate the list
 * if needed.
 *
 * \param paramList        where to reserve parameter slots
 * \param reserve_params   number of parameter description slots
 * \param reserve_values   number of parameter vec4 slots
 */
void
_mesa_reserve_parameter_storage(struct gl_program_parameter_list *paramList,
                                unsigned reserve_params,
                                unsigned reserve_values)
{
   const GLuint oldNum = paramList->NumParameters;
   const unsigned oldValNum = paramList->NumParameterValues;
   const unsigned needSizeValues = oldValNum + reserve_values * 4;

   if (paramList->DisallowRealloc &&
       (oldNum + reserve_params > paramList->Size ||
        needSizeValues > paramList->SizeValues)) {
      _mesa_problem(NULL, "Parameter storage reallocation disallowed.\n"
                          "This is a Mesa bug.\n"
                          "Increase the reservation size in the code (wanted bytes %u, have %u || wanted values %u have %u).",
                          oldNum + reserve_params, paramList->Size, needSizeValues, paramList->SizeValues);
      abort();
   }

   if (oldNum + reserve_params > paramList->Size) {
      /* Need to grow the parameter list array (alloc some extra) */
      paramList->Size += 4 * reserve_params;

      /* realloc arrays */
      paramList->Parameters =
         realloc(paramList->Parameters,
                 paramList->Size * sizeof(struct gl_program_parameter));
   }

   if (needSizeValues > paramList->SizeValues) {
      paramList->SizeValues = needSizeValues + 16; /* alloc some extra */

      paramList->ParameterValues = (gl_constant_value *)
         align_realloc(paramList->ParameterValues,         /* old buf */
                       oldValNum * sizeof(gl_constant_value),/* old sz */
                       /* Overallocate the size by 12 because matrix rows can
                        * be allocated partially but fetch_state always writes
                        * 4 components (16 bytes).
                        */
                       paramList->SizeValues * sizeof(gl_constant_value) +
                       12, 16);
      /* The values are written to the shader cache, so clear them. */
      memset(paramList->ParameterValues + oldValNum, 0,
             (paramList->SizeValues - oldValNum) * sizeof(gl_constant_value));
   }
}


/**
 * Disallow reallocating the parameter storage, so that uniform storage
 * can have pointers pointing to it.
 */
void
_mesa_disallow_parameter_storage_realloc(struct gl_program_parameter_list *paramList)
{
   paramList->DisallowRealloc = true;
}


/**
 * Add a new parameter to a parameter list.
 * Note that parameter values are usually 4-element GLfloat vectors.
 * When size > 4 we'll allocate a sequential block of parameters to
 * store all the values (in blocks of 4).
 *
 * \param paramList  the list to add the parameter to
 * \param type  type of parameter, such as
 * \param name  the parameter name, will be duplicated/copied!
 * \param size  number of elements in 'values' vector (1..4, or more)
 * \param datatype  GL_FLOAT, GL_FLOAT_VECx, GL_INT, GL_INT_VECx or GL_NONE.
 * \param values  initial parameter value, up to 4 gl_constant_values, or NULL
 * \param state  state indexes, or NULL
 * \return  index of new parameter in the list, or -1 if error (out of mem)
 */
GLint
_mesa_add_parameter(struct gl_program_parameter_list *paramList,
                    gl_register_file type, const char *name,
                    GLuint size, GLenum datatype,
                    const gl_constant_value *values,
                    const gl_state_index16 state[STATE_LENGTH],
                    bool pad_and_align)
{
   assert(0 < size);
   const int oldNum = paramList->NumParameters;
   unsigned oldValNum = paramList->NumParameterValues;
   const unsigned padded_size = pad_and_align ? align(size, 4) : size;

   if (pad_and_align)
      oldValNum = align(oldValNum, 4); /* pad start to a vec4 boundary */
   else if (_mesa_gl_datatype_is_64bit(datatype))
      oldValNum = align(oldValNum, 2); /* pad start to 64-bit */

   unsigned elements = (oldValNum - paramList->NumParameterValues) + padded_size;
   _mesa_reserve_parameter_storage(paramList, 1, DIV_ROUND_UP(elements, 4));

   if (!paramList->Parameters ||
       !paramList->ParameterValues) {
      /* out of memory */
      paramList->NumParameters = 0;
      paramList->Size = 0;
      paramList->SizeValues = 0;
      return -1;
   }

   paramList->NumParameters = oldNum + 1;

   paramList->NumParameterValues = oldValNum + padded_size;

   memset(&paramList->Parameters[oldNum], 0,
          sizeof(struct gl_program_parameter));
   memset(&paramList->ParameterValues[oldValNum], 0, padded_size);

   struct gl_program_parameter *p = paramList->Parameters + oldNum;
   p->Name = strdup(name ? name : "");
   p->Type = type;
   p->Size = size;
   p->Padded = pad_and_align;
   p->DataType = datatype;

   paramList->Parameters[oldNum].ValueOffset = oldValNum;
   if (values) {
      if (size >= 4) {
         memcpy(paramList->ParameterValues + oldValNum, values,
                size * sizeof(values[0]));
      } else {
         /* copy 1, 2 or 3 values */
         assert(size < 4);
         unsigned j;
         for (j = 0; j < size; j++) {
            paramList->ParameterValues[oldValNum + j].f = values[j].f;
         }

         /* Zero out padding (if any) to avoid valgrind errors */
         for (; j < padded_size; j++) {
            paramList->ParameterValues[oldValNum + j].f = 0;
         }
      }
   } else {
      for (unsigned j = 0; j < padded_size; j++) {
         paramList->ParameterValues[oldValNum + j].f = 0;
      }
   }

   if (state) {
      for (unsigned i = 0; i < STATE_LENGTH; i++)
         paramList->Parameters[oldNum].StateIndexes[i] = state[i];
   } else {
      paramList->Parameters[oldNum].StateIndexes[0] = STATE_NOT_STATE_VAR;
   }

   if (type == PROGRAM_UNIFORM || type == PROGRAM_CONSTANT) {
      paramList->UniformBytes =
         MAX2(paramList->UniformBytes,
              (paramList->Parameters[oldNum].ValueOffset +
               paramList->Parameters[oldNum].Size) * 4);
   } else if (type == PROGRAM_STATE_VAR) {
      paramList->FirstStateVarIndex =
         MIN2(paramList->FirstStateVarIndex, oldNum);
      paramList->LastStateVarIndex =
         MAX2(paramList->LastStateVarIndex, oldNum);
   } else {
      unreachable("invalid parameter type");
   }

   assert(paramList->NumParameters <= paramList->Size);
   assert(paramList->NumParameterValues <= paramList->SizeValues);

   return (GLint) oldNum;
}


/**
 * Add a new unnamed constant to the parameter list.  This will be used
 * when a fragment/vertex program contains something like this:
 *    MOV r, { 0, 1, 2, 3 };
 * If swizzleOut is non-null we'll search the parameter list for an
 * existing instance of the constant which matches with a swizzle.
 *
 * \param paramList  the parameter list
 * \param values  four float values
 * \param swizzleOut  returns swizzle mask for accessing the constant
 * \return index/position of the new parameter in the parameter list.
 */
GLint
_mesa_add_typed_unnamed_constant(struct gl_program_parameter_list *paramList,
                           const gl_constant_value *values, GLuint size,
                           GLenum datatype, GLuint *swizzleOut)
{
   GLint pos;
   assert(size >= 1);
   assert(size <= 4);

   if (swizzleOut &&
       lookup_parameter_constant(paramList, values, size, &pos, swizzleOut)) {
      return pos;
   }

   /* Look for empty space in an already unnamed constant parameter
    * to add this constant.  This will only work for single-element
    * constants because we rely on smearing (i.e. .yyyy or .zzzz).
    */
   if (size == 1 && swizzleOut) {
      for (pos = 0; pos < (GLint) paramList->NumParameters; pos++) {
         struct gl_program_parameter *p = paramList->Parameters + pos;
         unsigned offset = paramList->Parameters[pos].ValueOffset;
         if (p->Type == PROGRAM_CONSTANT && p->Size + size <= 4) {
            /* ok, found room */
            gl_constant_value *pVal = paramList->ParameterValues + offset;
            GLuint swz = p->Size; /* 1, 2 or 3 for Y, Z, W */
            pVal[p->Size] = values[0];
            p->Size++;
            *swizzleOut = MAKE_SWIZZLE4(swz, swz, swz, swz);
            return pos;
         }
      }
   }

   /* add a new parameter to store this constant */
   pos = _mesa_add_parameter(paramList, PROGRAM_CONSTANT, NULL,
                             size, datatype, values, NULL, true);
   if (pos >= 0 && swizzleOut) {
      if (size == 1)
         *swizzleOut = SWIZZLE_XXXX;
      else
         *swizzleOut = SWIZZLE_NOOP;
   }
   return pos;
}

GLint
_mesa_add_sized_state_reference(struct gl_program_parameter_list *paramList,
                                const gl_state_index16 stateTokens[STATE_LENGTH],
                                const unsigned size, bool pad_and_align)
{
   char *name;
   GLint index;

   /* Check if the state reference is already in the list */
   for (index = 0; index < (GLint) paramList->NumParameters; index++) {
      if (!memcmp(paramList->Parameters[index].StateIndexes,
                  stateTokens,
                  sizeof(paramList->Parameters[index].StateIndexes))) {
         return index;
      }
   }

   name = _mesa_program_state_string(stateTokens);
   index = _mesa_add_parameter(paramList, PROGRAM_STATE_VAR, name,
                               size, GL_NONE, NULL, stateTokens,
                               pad_and_align);
   paramList->StateFlags |= _mesa_program_state_flags(stateTokens);

   /* free name string here since we duplicated it in add_parameter() */
   free(name);

   return index;
}


/**
 * Add a new state reference to the parameter list.
 * This will be used when the program contains something like this:
 *    PARAM ambient = state.material.front.ambient;
 *
 * \param paramList  the parameter list
 * \param stateTokens  an array of STATE_LENGTH state tokens
 * \return index of the new parameter.
 */
GLint
_mesa_add_state_reference(struct gl_program_parameter_list *paramList,
                          const gl_state_index16 stateTokens[STATE_LENGTH])
{
   return _mesa_add_sized_state_reference(paramList, stateTokens, 4, true);
}

void
_mesa_recompute_parameter_bounds(struct gl_program_parameter_list *list)
{
   list->UniformBytes = 0;
   list->FirstStateVarIndex = INT_MAX;
   list->LastStateVarIndex = 0;

   for (int i = 0; i < (int)list->NumParameters; i++) {
      if (list->Parameters[i].Type == PROGRAM_STATE_VAR) {
         list->FirstStateVarIndex = MIN2(list->FirstStateVarIndex, i);
         list->LastStateVarIndex = MAX2(list->LastStateVarIndex, i);
      } else {
         list->UniformBytes = MAX2(list->UniformBytes,
                                   (list->Parameters[i].ValueOffset +
                                    list->Parameters[i].Size) * 4);
      }
   }
}
