/* -*- c++ -*- */
/*
 * Copyright © 2009 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef GLSL_TYPES_H
#define GLSL_TYPES_H

#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "shader_enums.h"
#include "c11/threads.h"
#include "util/blob.h"
#include "util/format/u_format.h"
#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct glsl_type glsl_type;
typedef struct glsl_struct_field glsl_struct_field;

extern void
glsl_type_singleton_init_or_ref(void);

extern void
glsl_type_singleton_decref(void);

void encode_type_to_blob(struct blob *blob, const glsl_type *type);

const glsl_type *decode_type_from_blob(struct blob_reader *blob);

typedef void (*glsl_type_size_align_func)(const glsl_type *type,
                                          unsigned *size, unsigned *alignment);

enum glsl_base_type {
   /* Note: GLSL_TYPE_UINT, GLSL_TYPE_INT, and GLSL_TYPE_FLOAT must be 0, 1,
    * and 2 so that they will fit in the 2 bits of glsl_type::sampled_type.
    */
   GLSL_TYPE_UINT = 0,
   GLSL_TYPE_INT,
   GLSL_TYPE_FLOAT,
   GLSL_TYPE_FLOAT16,
   GLSL_TYPE_DOUBLE,
   GLSL_TYPE_UINT8,
   GLSL_TYPE_INT8,
   GLSL_TYPE_UINT16,
   GLSL_TYPE_INT16,
   GLSL_TYPE_UINT64,
   GLSL_TYPE_INT64,
   GLSL_TYPE_BOOL,
   GLSL_TYPE_COOPERATIVE_MATRIX,
   GLSL_TYPE_SAMPLER,
   GLSL_TYPE_TEXTURE,
   GLSL_TYPE_IMAGE,
   GLSL_TYPE_ATOMIC_UINT,
   GLSL_TYPE_STRUCT,
   GLSL_TYPE_INTERFACE,
   GLSL_TYPE_ARRAY,
   GLSL_TYPE_VOID,
   GLSL_TYPE_SUBROUTINE,
   GLSL_TYPE_ERROR
};

/* Return the bit size of a type. Note that this differs from
 * glsl_get_bit_size in that it returns 32 bits for bools, whereas at
 * the NIR level we would want to return 1 bit for bools.
 */
static unsigned glsl_base_type_bit_size(enum glsl_base_type type)
{
   switch (type) {
   case GLSL_TYPE_BOOL:
   case GLSL_TYPE_INT:
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_FLOAT: /* TODO handle mediump */
   case GLSL_TYPE_SUBROUTINE:
      return 32;

   case GLSL_TYPE_FLOAT16:
   case GLSL_TYPE_UINT16:
   case GLSL_TYPE_INT16:
      return 16;

   case GLSL_TYPE_UINT8:
   case GLSL_TYPE_INT8:
      return 8;

   case GLSL_TYPE_DOUBLE:
   case GLSL_TYPE_INT64:
   case GLSL_TYPE_UINT64:
   case GLSL_TYPE_IMAGE:
   case GLSL_TYPE_TEXTURE:
   case GLSL_TYPE_SAMPLER:
      return 64;

   default:
      /* For GLSL_TYPE_STRUCT etc, it should be ok to return 0. This usually
       * happens when calling this method through is_64bit and is_16bit
       * methods
       */
      return 0;
   }

   return 0;
}

static inline bool glsl_base_type_is_16bit(enum glsl_base_type type)
{
   return glsl_base_type_bit_size(type) == 16;
}

static inline bool glsl_base_type_is_64bit(enum glsl_base_type type)
{
   return glsl_base_type_bit_size(type) == 64;
}

static inline bool glsl_base_type_is_integer(enum glsl_base_type type)
{
   return type == GLSL_TYPE_UINT8 ||
          type == GLSL_TYPE_INT8 ||
          type == GLSL_TYPE_UINT16 ||
          type == GLSL_TYPE_INT16 ||
          type == GLSL_TYPE_UINT ||
          type == GLSL_TYPE_INT ||
          type == GLSL_TYPE_UINT64 ||
          type == GLSL_TYPE_INT64 ||
          type == GLSL_TYPE_BOOL ||
          type == GLSL_TYPE_SAMPLER ||
          type == GLSL_TYPE_TEXTURE ||
          type == GLSL_TYPE_IMAGE;
}

static inline unsigned int
glsl_base_type_get_bit_size(const enum glsl_base_type base_type)
{
   switch (base_type) {
   case GLSL_TYPE_BOOL:
      return 1;

   case GLSL_TYPE_INT:
   case GLSL_TYPE_UINT:
   case GLSL_TYPE_FLOAT: /* TODO handle mediump */
   case GLSL_TYPE_SUBROUTINE:
   case GLSL_TYPE_COOPERATIVE_MATRIX:
      return 32;

   case GLSL_TYPE_FLOAT16:
   case GLSL_TYPE_UINT16:
   case GLSL_TYPE_INT16:
      return 16;

   case GLSL_TYPE_UINT8:
   case GLSL_TYPE_INT8:
      return 8;

   case GLSL_TYPE_DOUBLE:
   case GLSL_TYPE_INT64:
   case GLSL_TYPE_UINT64:
   case GLSL_TYPE_IMAGE:
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_TEXTURE:
      return 64;

   default:
      unreachable("unknown base type");
   }

   return 0;
}

static inline enum glsl_base_type
glsl_unsigned_base_type_of(enum glsl_base_type type)
{
   switch (type) {
   case GLSL_TYPE_INT:
      return GLSL_TYPE_UINT;
   case GLSL_TYPE_INT8:
      return GLSL_TYPE_UINT8;
   case GLSL_TYPE_INT16:
      return GLSL_TYPE_UINT16;
   case GLSL_TYPE_INT64:
      return GLSL_TYPE_UINT64;
   default:
      assert(type == GLSL_TYPE_UINT ||
             type == GLSL_TYPE_UINT8 ||
             type == GLSL_TYPE_UINT16 ||
             type == GLSL_TYPE_UINT64);
      return type;
   }
}

static inline enum glsl_base_type
glsl_signed_base_type_of(enum glsl_base_type type)
{
   switch (type) {
   case GLSL_TYPE_UINT:
      return GLSL_TYPE_INT;
   case GLSL_TYPE_UINT8:
      return GLSL_TYPE_INT8;
   case GLSL_TYPE_UINT16:
      return GLSL_TYPE_INT16;
   case GLSL_TYPE_UINT64:
      return GLSL_TYPE_INT64;
   default:
      assert(type == GLSL_TYPE_INT ||
             type == GLSL_TYPE_INT8 ||
             type == GLSL_TYPE_INT16 ||
             type == GLSL_TYPE_INT64);
      return type;
   }
}

enum glsl_sampler_dim {
   GLSL_SAMPLER_DIM_1D = 0,
   GLSL_SAMPLER_DIM_2D,
   GLSL_SAMPLER_DIM_3D,
   GLSL_SAMPLER_DIM_CUBE,
   GLSL_SAMPLER_DIM_RECT,
   GLSL_SAMPLER_DIM_BUF,
   GLSL_SAMPLER_DIM_EXTERNAL,
   GLSL_SAMPLER_DIM_MS,
   GLSL_SAMPLER_DIM_SUBPASS, /* for vulkan input attachments */
   GLSL_SAMPLER_DIM_SUBPASS_MS, /* for multisampled vulkan input attachments */
};

int
glsl_get_sampler_dim_coordinate_components(enum glsl_sampler_dim dim);

enum glsl_matrix_layout {
   /**
    * The layout of the matrix is inherited from the object containing the
    * matrix (the top level structure or the uniform block).
    */
   GLSL_MATRIX_LAYOUT_INHERITED,

   /**
    * Explicit column-major layout
    *
    * If a uniform block doesn't have an explicit layout set, it will default
    * to this layout.
    */
   GLSL_MATRIX_LAYOUT_COLUMN_MAJOR,

   /**
    * Row-major layout
    */
   GLSL_MATRIX_LAYOUT_ROW_MAJOR
};

enum {
   GLSL_PRECISION_NONE = 0,
   GLSL_PRECISION_HIGH,
   GLSL_PRECISION_MEDIUM,
   GLSL_PRECISION_LOW
};

enum glsl_cmat_use {
   GLSL_CMAT_USE_NONE = 0,
   GLSL_CMAT_USE_A,
   GLSL_CMAT_USE_B,
   GLSL_CMAT_USE_ACCUMULATOR,
};

struct glsl_cmat_description {
   /* MSVC can't merge bitfields of different types and also sign extend enums,
    * so use uint8_t for those cases.
    */
   uint8_t element_type:5; /* enum glsl_base_type */
   uint8_t scope:3; /* mesa_scope */
   uint8_t rows;
   uint8_t cols;
   uint8_t use; /* enum glsl_cmat_use */
};

const char *glsl_get_type_name(const glsl_type *type);

struct glsl_type {
   uint32_t gl_type;
   enum glsl_base_type base_type:8;

   enum glsl_base_type sampled_type:8; /**< Type of data returned using this
                                        * sampler or image.  Only \c
                                        * GLSL_TYPE_FLOAT, \c GLSL_TYPE_INT,
                                        * and \c GLSL_TYPE_UINT are valid.
                                        */

   unsigned sampler_dimensionality:4; /**< \see glsl_sampler_dim */
   unsigned sampler_shadow:1;
   unsigned sampler_array:1;
   unsigned interface_packing:2;
   unsigned interface_row_major:1;

   struct glsl_cmat_description cmat_desc;

   /**
    * For \c GLSL_TYPE_STRUCT this specifies if the struct is packed or not.
    *
    * Only used for Compute kernels
    */
   unsigned packed:1;

   unsigned has_builtin_name:1;

   /**
    * \name Vector and matrix element counts
    *
    * For scalars, each of these values will be 1.  For non-numeric types
    * these will be 0.
    */
   /*@{*/
   uint8_t vector_elements;    /**< 1, 2, 3, or 4 vector elements. */
   uint8_t matrix_columns;     /**< 1, 2, 3, or 4 matrix columns. */
   /*@}*/

   /**
    * For \c GLSL_TYPE_ARRAY, this is the length of the array.  For
    * \c GLSL_TYPE_STRUCT or \c GLSL_TYPE_INTERFACE, it is the number of
    * elements in the structure and the number of values pointed to by
    * \c fields.structure (below).
    */
   unsigned length;

   /**
    * Identifier to the name of the data type
    *
    * Use glsl_get_type_name() to access the actual name.
    */
   uintptr_t name_id;

   /**
    * Explicit array, matrix, or vector stride.  This is used to communicate
    * explicit array layouts from SPIR-V.  Should be 0 if the type has no
    * explicit stride.
    */
   unsigned explicit_stride;

   /**
    * Explicit alignment. This is used to communicate explicit alignment
    * constraints. Should be 0 if the type has no explicit alignment
    * constraint.
    */
   unsigned explicit_alignment;

   /**
    * Subtype of composite data types.
    */
   union {
      const glsl_type *array;            /**< Type of array elements. */
      const glsl_struct_field *structure;      /**< List of struct fields. */
   } fields;
};

#include "builtin_types.h"

struct glsl_struct_field {
   const glsl_type *type;
   const char *name;

   /**
    * For interface blocks, gl_varying_slot corresponding to the input/output
    * if this is a built-in input/output (i.e. a member of the built-in
    * gl_PerVertex interface block); -1 otherwise.
    *
    * Ignored for structs.
    */
   int location;

   /**
    * For interface blocks, members may explicitly assign the component used
    * by a varying. Ignored for structs.
    */
   int component;

   /**
    * For interface blocks, members may have an explicit byte offset
    * specified; -1 otherwise. Also used for xfb_offset layout qualifier.
    *
    * Unless used for xfb_offset this field is ignored for structs.
    */
   int offset;

   /**
    * For interface blocks, members may define a transform feedback buffer;
    * -1 otherwise.
    */
   int xfb_buffer;

   /**
    * For interface blocks, members may define a transform feedback stride;
    * -1 otherwise.
    */
   int xfb_stride;

   /**
    * Layout format, applicable to image variables only.
    */
   enum pipe_format image_format;

   union {
      struct {
         /**
          * For interface blocks, the interpolation mode (as in
          * ir_variable::interpolation).  0 otherwise.
          */
         unsigned interpolation:3;

         /**
          * For interface blocks, 1 if this variable uses centroid interpolation (as
          * in ir_variable::centroid).  0 otherwise.
          */
         unsigned centroid:1;

         /**
          * For interface blocks, 1 if this variable uses sample interpolation (as
          * in ir_variable::sample). 0 otherwise.
          */
         unsigned sample:1;

         /**
          * Layout of the matrix.  Uses glsl_matrix_layout values.
          */
         unsigned matrix_layout:2;

         /**
          * For interface blocks, 1 if this variable is a per-patch input or output
          * (as in ir_variable::patch). 0 otherwise.
          */
         unsigned patch:1;

         /**
          * Precision qualifier
          */
         unsigned precision:2;

         /**
          * Memory qualifiers, applicable to buffer variables defined in shader
          * storage buffer objects (SSBOs)
          */
         unsigned memory_read_only:1;
         unsigned memory_write_only:1;
         unsigned memory_coherent:1;
         unsigned memory_volatile:1;
         unsigned memory_restrict:1;

         /**
          * Any of the xfb_* qualifiers trigger the shader to be in transform
          * feedback mode so we need to keep track of whether the buffer was
          * explicitly set or if its just been assigned the default global value.
          */
         unsigned explicit_xfb_buffer:1;

         unsigned implicit_sized_array:1;
      };
      unsigned flags;
   };
#ifdef __cplusplus
#define DEFAULT_CONSTRUCTORS(_type, _name)                  \
   type(_type), name(_name), location(-1), component(-1), offset(-1), \
   xfb_buffer(0),  xfb_stride(0), image_format(PIPE_FORMAT_NONE), flags(0) \

   glsl_struct_field(const glsl_type *_type,
                     int _precision,
                     const char *_name)
      : DEFAULT_CONSTRUCTORS(_type, _name)
   {
      matrix_layout = GLSL_MATRIX_LAYOUT_INHERITED;
      precision = _precision;
   }

   glsl_struct_field(const glsl_type *_type, const char *_name)
      : DEFAULT_CONSTRUCTORS(_type, _name)
   {
      matrix_layout = GLSL_MATRIX_LAYOUT_INHERITED;
      precision = GLSL_PRECISION_NONE;
   }

   glsl_struct_field()
      : DEFAULT_CONSTRUCTORS(NULL, NULL)
   {
      matrix_layout = GLSL_MATRIX_LAYOUT_INHERITED;
      precision = GLSL_PRECISION_NONE;
   }
#undef DEFAULT_CONSTRUCTORS
#endif
};

static inline enum glsl_base_type glsl_get_base_type(const glsl_type *t) { return t->base_type; }

static inline unsigned
glsl_get_bit_size(const glsl_type *t)
{
   return glsl_base_type_get_bit_size(glsl_get_base_type(t));
}

static inline bool glsl_type_is_boolean(const glsl_type *t) { return t->base_type == GLSL_TYPE_BOOL; }
static inline bool glsl_type_is_sampler(const glsl_type *t) { return t->base_type == GLSL_TYPE_SAMPLER; }
static inline bool glsl_type_is_texture(const glsl_type *t) { return t->base_type == GLSL_TYPE_TEXTURE; }
static inline bool glsl_type_is_image(const glsl_type *t) { return t->base_type == GLSL_TYPE_IMAGE; }
static inline bool glsl_type_is_atomic_uint(const glsl_type *t) { return t->base_type == GLSL_TYPE_ATOMIC_UINT; }
static inline bool glsl_type_is_struct(const glsl_type *t) { return t->base_type == GLSL_TYPE_STRUCT; }
static inline bool glsl_type_is_interface(const glsl_type *t) { return t->base_type == GLSL_TYPE_INTERFACE; }
static inline bool glsl_type_is_array(const glsl_type *t) { return t->base_type == GLSL_TYPE_ARRAY; }
static inline bool glsl_type_is_cmat(const glsl_type *t) { return t->base_type == GLSL_TYPE_COOPERATIVE_MATRIX; }
static inline bool glsl_type_is_void(const glsl_type *t) { return t->base_type == GLSL_TYPE_VOID; }
static inline bool glsl_type_is_subroutine(const glsl_type *t) { return t->base_type == GLSL_TYPE_SUBROUTINE; }
static inline bool glsl_type_is_error(const glsl_type *t) { return t->base_type == GLSL_TYPE_ERROR; }
static inline bool glsl_type_is_double(const glsl_type *t) { return t->base_type == GLSL_TYPE_DOUBLE; }
static inline bool glsl_type_is_float(const glsl_type *t) { return t->base_type == GLSL_TYPE_FLOAT; }

static inline bool
glsl_type_is_numeric(const glsl_type *t)
{
   return t->base_type >= GLSL_TYPE_UINT &&
          t->base_type <= GLSL_TYPE_INT64;
}

static inline bool
glsl_type_is_integer(const glsl_type *t)
{
   return glsl_base_type_is_integer(t->base_type);
}

static inline bool
glsl_type_is_struct_or_ifc(const glsl_type *t)
{
   return glsl_type_is_struct(t) || glsl_type_is_interface(t);
}

static inline bool
glsl_type_is_packed(const glsl_type *t)
{
   return t->packed;
}

static inline bool
glsl_type_is_16bit(const glsl_type *t)
{
   return glsl_base_type_is_16bit(t->base_type);
}

static inline bool
glsl_type_is_32bit(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_UINT ||
          t->base_type == GLSL_TYPE_INT ||
          t->base_type == GLSL_TYPE_FLOAT;
}

static inline bool
glsl_type_is_64bit(const glsl_type *t)
{
   return glsl_base_type_is_64bit(t->base_type);
}

static inline bool
glsl_type_is_integer_16(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_UINT16 || t->base_type == GLSL_TYPE_INT16;
}

static inline bool
glsl_type_is_integer_32(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_UINT || t->base_type == GLSL_TYPE_INT;
}

static inline bool
glsl_type_is_integer_64(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_UINT64 || t->base_type == GLSL_TYPE_INT64;
}

static inline bool
glsl_type_is_integer_32_64(const glsl_type *t)
{
   return glsl_type_is_integer_32(t) || glsl_type_is_integer_64(t);
}

static inline bool
glsl_type_is_integer_16_32(const glsl_type *t)
{
   return glsl_type_is_integer_16(t) || glsl_type_is_integer_32(t);
}

static inline bool
glsl_type_is_integer_16_32_64(const glsl_type *t)
{
   return glsl_type_is_integer_16(t) || glsl_type_is_integer_32(t) || glsl_type_is_integer_64(t);
}

static inline bool
glsl_type_is_float_16_32(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_FLOAT16 || glsl_type_is_float(t);
}

static inline bool
glsl_type_is_float_16_32_64(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_FLOAT16 || glsl_type_is_float(t) || glsl_type_is_double(t);
}

static inline bool
glsl_type_is_float_32_64(const glsl_type *t)
{
   return glsl_type_is_float(t) || glsl_type_is_double(t);
}

static inline bool
glsl_type_is_int_16_32_64(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_INT16 ||
          t->base_type == GLSL_TYPE_INT ||
          t->base_type == GLSL_TYPE_INT64;
}

static inline bool
glsl_type_is_uint_16_32_64(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_UINT16 ||
          t->base_type == GLSL_TYPE_UINT ||
          t->base_type == GLSL_TYPE_UINT64;
}

static inline bool
glsl_type_is_int_16_32(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_INT ||
          t->base_type == GLSL_TYPE_INT16;
}

static inline bool
glsl_type_is_uint_16_32(const glsl_type *t)
{
   return t->base_type == GLSL_TYPE_UINT ||
          t->base_type == GLSL_TYPE_UINT16;
}

static inline bool
glsl_type_is_unsized_array(const glsl_type *t)
{
   return glsl_type_is_array(t) && t->length == 0;
}

static inline bool
glsl_type_is_array_of_arrays(const glsl_type *t)
{
   return glsl_type_is_array(t) && glsl_type_is_array(t->fields.array);
}

static inline bool
glsl_type_is_bare_sampler(const glsl_type *t)
{
   return glsl_type_is_sampler(t) && t->sampled_type == GLSL_TYPE_VOID;
}

bool glsl_type_is_vector(const glsl_type *t);
bool glsl_type_is_scalar(const glsl_type *t);
bool glsl_type_is_vector_or_scalar(const glsl_type *t);
bool glsl_type_is_matrix(const glsl_type *t);
bool glsl_type_is_array_or_matrix(const glsl_type *t);

/**
 * Query whether a 64-bit type takes two slots.
 */
bool glsl_type_is_dual_slot(const glsl_type *t);

bool glsl_type_is_leaf(const glsl_type *type);

static inline bool
glsl_matrix_type_is_row_major(const glsl_type *t)
{
   assert((glsl_type_is_matrix(t) && t->explicit_stride) || glsl_type_is_interface(t));
   return t->interface_row_major;
}

static inline bool
glsl_sampler_type_is_shadow(const glsl_type *t)
{
   assert(glsl_type_is_sampler(t));
   return t->sampler_shadow;
}

static inline bool
glsl_sampler_type_is_array(const glsl_type *t)
{
   assert(glsl_type_is_sampler(t) ||
          glsl_type_is_texture(t) ||
          glsl_type_is_image(t));
   return t->sampler_array;
}

static inline bool
glsl_struct_type_is_packed(const glsl_type *t)
{
   assert(glsl_type_is_struct(t));
   return t->packed;
}

/**
 * Gets the "bare" type without any decorations or layout information.
 */
const glsl_type *glsl_get_bare_type(const glsl_type *t);

/**
 * Get the basic scalar type which this type aggregates.
 *
 * If the type is a numeric or boolean scalar, vector, or matrix, or an
 * array of any of those, this function gets the scalar type of the
 * individual components.  For structs and arrays of structs, this function
 * returns the struct type.  For samplers and arrays of samplers, this
 * function returns the sampler type.
 */
const glsl_type *glsl_get_scalar_type(const glsl_type *t);

/**
 * For numeric and boolean derived types returns the basic scalar type
 *
 * If the type is a numeric or boolean scalar, vector, or matrix type,
 * this function gets the scalar type of the individual components.  For
 * all other types, including arrays of numeric or boolean types, the
 * error type is returned.
 */
const glsl_type *glsl_get_base_glsl_type(const glsl_type *t);

unsigned glsl_get_length(const glsl_type *t);

static inline unsigned
glsl_get_vector_elements(const glsl_type *t)
{
   return t->vector_elements;
}

/**
 * Query the total number of scalars that make up a scalar, vector or matrix
 */
static inline unsigned
glsl_get_components(const glsl_type *t)
{
   return t->vector_elements * t->matrix_columns;
}

static inline unsigned
glsl_get_matrix_columns(const glsl_type *t)
{
   return t->matrix_columns;
}

const glsl_type *glsl_type_wrap_in_arrays(const glsl_type *t,
                                                 const glsl_type *arrays);

/**
 * Query the number of elements in an array type
 *
 * \return
 * The number of elements in the array for array types or -1 for non-array
 * types.  If the number of elements in the array has not yet been declared,
 * zero is returned.
 */
static inline int
glsl_array_size(const glsl_type *t)
{
   return glsl_type_is_array(t) ? t->length : -1;
}

/**
 * Return the total number of elements in an array including the elements
 * in arrays of arrays.
 */
unsigned glsl_get_aoa_size(const glsl_type *t);

const glsl_type *glsl_get_array_element(const glsl_type *t);

/**
 * Get the type stripped of any arrays
 *
 * \return
 * Pointer to the type of elements of the first non-array type for array
 * types, or pointer to itself for non-array types.
 */
const glsl_type *glsl_without_array(const glsl_type *t);

const glsl_type *glsl_without_array_or_matrix(const glsl_type *t);
const glsl_type *glsl_type_wrap_in_arrays(const glsl_type *t,
                                                 const glsl_type *arrays);

const glsl_type *glsl_get_cmat_element(const glsl_type *t);
const struct glsl_cmat_description *glsl_get_cmat_description(const glsl_type *t);

/**
 * Return the amount of atomic counter storage required for a type.
 */
unsigned glsl_atomic_size(const glsl_type *type);

/**
 * Type A contains type B if A is B or A is a composite type (struct,
 * interface, array) that has an element that contains B.
 */
bool glsl_type_contains_64bit(const glsl_type *t);
bool glsl_type_contains_image(const glsl_type *t);
bool glsl_contains_atomic(const glsl_type *t);
bool glsl_contains_double(const glsl_type *t);
bool glsl_contains_integer(const glsl_type *t);
bool glsl_contains_opaque(const glsl_type *t);
bool glsl_contains_sampler(const glsl_type *t);
bool glsl_contains_array(const glsl_type *t);
bool glsl_contains_subroutine(const glsl_type *t);

static inline enum glsl_sampler_dim
glsl_get_sampler_dim(const glsl_type *t)
{
   assert(glsl_type_is_sampler(t) ||
          glsl_type_is_texture(t) ||
          glsl_type_is_image(t));
   return (enum glsl_sampler_dim)t->sampler_dimensionality;
}

static inline enum glsl_base_type
glsl_get_sampler_result_type(const glsl_type *t)
{
   assert(glsl_type_is_sampler(t) ||
          glsl_type_is_texture(t) ||
          glsl_type_is_image(t));
   return (enum glsl_base_type)t->sampled_type;
}

/**
 * Return the number of coordinate components needed for this
 * sampler or image type.
 *
 * This is based purely on the sampler's dimensionality.  For example, this
 * returns 1 for sampler1D, and 3 for sampler2DArray.
 *
 * Note that this is often different than actual coordinate type used in
 * a texturing built-in function, since those pack additional values (such
 * as the shadow comparator or projector) into the coordinate type.
 */
int glsl_get_sampler_coordinate_components(const glsl_type *t);

/**
 * Compares whether this type matches another type without taking into
 * account the precision in structures.
 *
 * This is applied recursively so that structures containing structure
 * members can also ignore the precision.
 */
bool glsl_type_compare_no_precision(const glsl_type *a, const glsl_type *b);

/**
 * Compare a record type against another record type.
 *
 * This is useful for matching record types declared on the same shader
 * stage as well as across different shader stages.
 * The option to not match name is needed for matching record types
 * declared across different shader stages.
 * The option to not match locations is to deal with places where the
 * same struct is defined in a block which has a location set on it.
 */
bool glsl_record_compare(const glsl_type *a, const glsl_type *b,
                         bool match_name, bool match_locations,
                         bool match_precision);

const glsl_type *glsl_get_struct_field(const glsl_type *t, unsigned index);
const glsl_struct_field *glsl_get_struct_field_data(const glsl_type *t, unsigned index);

/**
 * Calculate offset between the base location of the struct in
 * uniform storage and a struct member.
 * For the initial call, length is the index of the member to find the
 * offset for.
 */
unsigned glsl_get_struct_location_offset(const glsl_type *t, unsigned length);

/**
 * Get the location of a field within a record type
 */
int glsl_get_field_index(const glsl_type *t, const char *name);

/**
 * Get the type of a structure field
 *
 * \return
 * Pointer to the type of the named field.  If the type is not a structure
 * or the named field does not exist, \c &glsl_type_builtin_error is returned.
 */
const glsl_type *glsl_get_field_type(const glsl_type *t, const char *name);

static inline int
glsl_get_struct_field_offset(const glsl_type *t, unsigned index)
{
   return t->fields.structure[index].offset;
}

static inline const char *
glsl_get_struct_elem_name(const glsl_type *t, unsigned index)
{
   return t->fields.structure[index].name;
}

static inline const glsl_type *glsl_void_type(void) { return &glsl_type_builtin_void; }
static inline const glsl_type *glsl_float_type(void) { return &glsl_type_builtin_float; }
static inline const glsl_type *glsl_float16_t_type(void) { return &glsl_type_builtin_float16_t; }
static inline const glsl_type *glsl_double_type(void) { return &glsl_type_builtin_double; }
static inline const glsl_type *glsl_vec2_type(void) { return &glsl_type_builtin_vec2; }
static inline const glsl_type *glsl_dvec2_type(void) { return &glsl_type_builtin_dvec2; }
static inline const glsl_type *glsl_uvec2_type(void) { return &glsl_type_builtin_uvec2; }
static inline const glsl_type *glsl_ivec2_type(void) { return &glsl_type_builtin_ivec2; }
static inline const glsl_type *glsl_bvec2_type(void) { return &glsl_type_builtin_bvec2; }
static inline const glsl_type *glsl_vec4_type(void) { return &glsl_type_builtin_vec4; }
static inline const glsl_type *glsl_dvec4_type(void) { return &glsl_type_builtin_dvec4; }
static inline const glsl_type *glsl_uvec4_type(void) { return &glsl_type_builtin_uvec4; }
static inline const glsl_type *glsl_ivec4_type(void) { return &glsl_type_builtin_ivec4; }
static inline const glsl_type *glsl_bvec4_type(void) { return &glsl_type_builtin_bvec4; }
static inline const glsl_type *glsl_int_type(void) { return &glsl_type_builtin_int; }
static inline const glsl_type *glsl_uint_type(void) { return &glsl_type_builtin_uint; }
static inline const glsl_type *glsl_int64_t_type(void) { return &glsl_type_builtin_int64_t; }
static inline const glsl_type *glsl_uint64_t_type(void) { return &glsl_type_builtin_uint64_t; }
static inline const glsl_type *glsl_int16_t_type(void) { return &glsl_type_builtin_int16_t; }
static inline const glsl_type *glsl_uint16_t_type(void) { return &glsl_type_builtin_uint16_t; }
static inline const glsl_type *glsl_int8_t_type(void) { return &glsl_type_builtin_int8_t; }
static inline const glsl_type *glsl_uint8_t_type(void) { return &glsl_type_builtin_uint8_t; }
static inline const glsl_type *glsl_bool_type(void) { return &glsl_type_builtin_bool; }
static inline const glsl_type *glsl_atomic_uint_type(void) { return &glsl_type_builtin_atomic_uint; }

static inline const glsl_type *
glsl_floatN_t_type(unsigned bit_size)
{
   switch (bit_size) {
   case 16: return &glsl_type_builtin_float16_t;
   case 32: return &glsl_type_builtin_float;
   case 64: return &glsl_type_builtin_double;
   default:
      unreachable("Unsupported bit size");
   }
}

static inline const glsl_type *
glsl_intN_t_type(unsigned bit_size)
{
   switch (bit_size) {
   case 8:  return &glsl_type_builtin_int8_t;
   case 16: return &glsl_type_builtin_int16_t;
   case 32: return &glsl_type_builtin_int;
   case 64: return &glsl_type_builtin_int64_t;
   default:
      unreachable("Unsupported bit size");
   }
}

static inline const glsl_type *
glsl_uintN_t_type(unsigned bit_size)
{
   switch (bit_size) {
   case 8:  return &glsl_type_builtin_uint8_t;
   case 16: return &glsl_type_builtin_uint16_t;
   case 32: return &glsl_type_builtin_uint;
   case 64: return &glsl_type_builtin_uint64_t;
   default:
      unreachable("Unsupported bit size");
   }
}

const glsl_type *glsl_vec_type(unsigned components);
const glsl_type *glsl_f16vec_type(unsigned components);
const glsl_type *glsl_dvec_type(unsigned components);
const glsl_type *glsl_ivec_type(unsigned components);
const glsl_type *glsl_uvec_type(unsigned components);
const glsl_type *glsl_bvec_type(unsigned components);
const glsl_type *glsl_i64vec_type(unsigned components);
const glsl_type *glsl_u64vec_type(unsigned components);
const glsl_type *glsl_i16vec_type(unsigned components);
const glsl_type *glsl_u16vec_type(unsigned components);
const glsl_type *glsl_i8vec_type(unsigned components);
const glsl_type *glsl_u8vec_type(unsigned components);

const glsl_type *glsl_simple_explicit_type(unsigned base_type, unsigned rows,
                                                  unsigned columns,
                                                  unsigned explicit_stride,
                                                  bool row_major,
                                                  unsigned explicit_alignment);

static inline const glsl_type *
glsl_simple_type(unsigned base_type, unsigned rows, unsigned columns)
{
   return glsl_simple_explicit_type(base_type, rows, columns, 0, false, 0);
}

const glsl_type *glsl_sampler_type(enum glsl_sampler_dim dim,
                                          bool shadow,
                                          bool array,
                                          enum glsl_base_type type);
const glsl_type *glsl_bare_sampler_type(void);
const glsl_type *glsl_bare_shadow_sampler_type(void);
const glsl_type *glsl_texture_type(enum glsl_sampler_dim dim,
                                          bool array,
                                          enum glsl_base_type type);
const glsl_type *glsl_image_type(enum glsl_sampler_dim dim,
                                        bool array, enum glsl_base_type type);
const glsl_type *glsl_array_type(const glsl_type *element,
                                        unsigned array_size,
                                        unsigned explicit_stride);
const glsl_type *glsl_cmat_type(const struct glsl_cmat_description *desc);
const glsl_type *glsl_struct_type_with_explicit_alignment(const glsl_struct_field *fields,
                                                                 unsigned num_fields,
                                                                 const char *name,
                                                                 bool packed,
                                                                 unsigned explicit_alignment);

static inline const glsl_type *
glsl_struct_type(const glsl_struct_field *fields, unsigned num_fields,
                 const char *name, bool packed)
{
   return glsl_struct_type_with_explicit_alignment(fields, num_fields, name, packed, 0);
}

const glsl_type *glsl_interface_type(const glsl_struct_field *fields,
                                            unsigned num_fields,
                                            enum glsl_interface_packing packing,
                                            bool row_major,
                                            const char *block_name);
const glsl_type *glsl_subroutine_type(const char *subroutine_name);

/**
 * Query the full type of a matrix row
 *
 * \return
 * If the type is not a matrix, \c glsl_type::error_type is returned.
 * Otherwise a type matching the rows of the matrix is returned.
 */
const glsl_type *glsl_get_row_type(const glsl_type *t);

/**
 * Query the full type of a matrix column
 *
 * \return
 * If the type is not a matrix, \c glsl_type::error_type is returned.
 * Otherwise a type matching the columns of the matrix is returned.
 */
const glsl_type *glsl_get_column_type(const glsl_type *t);

/** Returns an explicitly laid out type given a type and size/align func
 *
 * The size/align func is only called for scalar and vector types and the
 * returned type is otherwise laid out in the natural way as follows:
 *
 *  - Arrays and matrices have a stride of align(elem_size, elem_align).
 *
 *  - Structure types have their elements in-order and as tightly packed as
 *    possible following the alignment required by the size/align func.
 *
 *  - All composite types (structures, matrices, and arrays) have an
 *    alignment equal to the highest alignment of any member of the composite.
 *
 * The types returned by this function are likely not suitable for most UBO
 * or SSBO layout because they do not add the extra array and substructure
 * alignment that is required by std140 and std430.
 */
const glsl_type *glsl_get_explicit_type_for_size_align(const glsl_type *type,
                                                              glsl_type_size_align_func type_info,
                                                              unsigned *size, unsigned *alignment);

const glsl_type *glsl_type_replace_vec3_with_vec4(const glsl_type *type);

/**
 * Gets the float16 version of this type.
 */
const glsl_type *glsl_float16_type(const glsl_type *t);

/**
 * Gets the int16 version of this type.
 */
const glsl_type *glsl_int16_type(const glsl_type *t);

/**
 * Gets the uint16 version of this type.
 */
const glsl_type *glsl_uint16_type(const glsl_type *t);

const glsl_type *glsl_type_to_16bit(const glsl_type *old_type);

static inline const glsl_type *
glsl_scalar_type(enum glsl_base_type base_type)
{
   return glsl_simple_type(base_type, 1, 1);
}

static inline const glsl_type *
glsl_vector_type(enum glsl_base_type base_type, unsigned components)
{
   const glsl_type *t = glsl_simple_type(base_type, components, 1);
   assert(t != &glsl_type_builtin_error);
   return t;
}

static inline const glsl_type *
glsl_matrix_type(enum glsl_base_type base_type,
                 unsigned rows, unsigned columns)
{
   const glsl_type *t = glsl_simple_type(base_type, rows, columns);
   assert(t != &glsl_type_builtin_error);
   return t;
}

static inline const glsl_type *
glsl_explicit_matrix_type(const glsl_type *mat, unsigned stride,
                          bool row_major) {
   assert(stride > 0);
   const glsl_type *t = glsl_simple_explicit_type(mat->base_type,
                                                         mat->vector_elements,
                                                         mat->matrix_columns,
                                                         stride, row_major, 0);
   assert(t != &glsl_type_builtin_error);
   return t;

}

static inline const glsl_type *
glsl_transposed_type(const glsl_type *t)
{
   assert(glsl_type_is_matrix(t));
   return glsl_simple_type(t->base_type, t->matrix_columns, t->vector_elements);
}

static inline const glsl_type *
glsl_texture_type_to_sampler(const glsl_type *t, bool is_shadow)
{
   assert(glsl_type_is_texture(t));
   return glsl_sampler_type((enum glsl_sampler_dim)t->sampler_dimensionality,
                            is_shadow, t->sampler_array,
                            (enum glsl_base_type)t->sampled_type);
}

static inline const glsl_type *
glsl_sampler_type_to_texture(const glsl_type *t)
{
   assert(glsl_type_is_sampler(t) && !glsl_type_is_bare_sampler(t));
   return glsl_texture_type((enum glsl_sampler_dim)t->sampler_dimensionality,
                            t->sampler_array,
                            (enum glsl_base_type)t->sampled_type);
}

const glsl_type *glsl_replace_vector_type(const glsl_type *t, unsigned components);
const glsl_type *glsl_channel_type(const glsl_type *t);

/**
 * Get the type resulting from a multiplication of \p type_a * \p type_b
 */
const glsl_type *glsl_get_mul_type(const glsl_type *type_a, const glsl_type *type_b);

unsigned glsl_type_get_sampler_count(const glsl_type *t);
unsigned glsl_type_get_texture_count(const glsl_type *t);
unsigned glsl_type_get_image_count(const glsl_type *t);

/**
 * Calculate the number of vec4 slots required to hold this type.
 *
 * This is the underlying recursive type_size function for
 * count_attribute_slots() (vertex inputs and varyings) but also for
 * gallium's !PIPE_CAP_PACKED_UNIFORMS case.
 */
unsigned glsl_count_vec4_slots(const glsl_type *t, bool is_gl_vertex_input, bool is_bindless);

/**
 * Calculate the number of vec4 slots required to hold this type.
 *
 * This is the underlying recursive type_size function for
 * gallium's PIPE_CAP_PACKED_UNIFORMS case.
 */
unsigned glsl_count_dword_slots(const glsl_type *t, bool is_bindless);

/**
 * Calculate the number of components slots required to hold this type
 *
 * This is used to determine how many uniform or varying locations a type
 * might occupy.
 */
unsigned glsl_get_component_slots(const glsl_type *t);

unsigned glsl_get_component_slots_aligned(const glsl_type *t, unsigned offset);

/**
 * Used to count the number of varyings contained in the type ignoring
 * innermost array elements.
 */
unsigned glsl_varying_count(const glsl_type *t);

/**
 * Calculate the number of unique values from glGetUniformLocation for the
 * elements of the type.
 *
 * This is used to allocate slots in the UniformRemapTable, the amount of
 * locations may not match with actual used storage space by the driver.
 */
unsigned glsl_type_uniform_locations(const glsl_type *t);

/**
 * Calculate the number of attribute slots required to hold this type
 *
 * This implements the language rules of GLSL 1.50 for counting the number
 * of slots used by a vertex attribute.  It also determines the number of
 * varying slots the type will use up in the absence of varying packing
 * (and thus, it can be used to measure the number of varying slots used by
 * the varyings that are generated by lower_packed_varyings).
 *
 * For vertex shader attributes - doubles only take one slot.
 * For inter-shader varyings - dvec3/dvec4 take two slots.
 *
 * Vulkan doesn’t make this distinction so the argument should always be
 * false.
 */
static inline unsigned
glsl_count_attribute_slots(const glsl_type *t, bool is_gl_vertex_input)
{
   return glsl_count_vec4_slots(t, is_gl_vertex_input, true);
}

/**
 * Size in bytes of this type in OpenCL memory
 */
unsigned glsl_get_cl_size(const glsl_type *t);

/**
 * Alignment in bytes of the start of this type in OpenCL memory.
 */
unsigned glsl_get_cl_alignment(const glsl_type *t);

void glsl_get_cl_type_size_align(const glsl_type *t,
                                 unsigned *size, unsigned *align);

/**
 * Get the type interface packing used internally. For shared and packing
 * layouts this is implementation defined.
 */
enum glsl_interface_packing glsl_get_internal_ifc_packing(const glsl_type *t, bool std430_supported);

static inline enum glsl_interface_packing
glsl_get_ifc_packing(const glsl_type *t)
{
   return (enum glsl_interface_packing)t->interface_packing;
}

/**
 * Alignment in bytes of the start of this type in a std140 uniform
 * block.
 */
unsigned glsl_get_std140_base_alignment(const glsl_type *t, bool row_major);

/** Size in bytes of this type in a std140 uniform block.
 *
 * Note that this is not GL_UNIFORM_SIZE (which is the number of
 * elements in the array)
 */
unsigned glsl_get_std140_size(const glsl_type *t, bool row_major);

/**
 * Calculate array stride in bytes of this type in a std430 shader storage
 * block.
 */
unsigned glsl_get_std430_array_stride(const glsl_type *t, bool row_major);

/**
 * Alignment in bytes of the start of this type in a std430 shader
 * storage block.
 */
unsigned glsl_get_std430_base_alignment(const glsl_type *t, bool row_major);

/**
 * Size in bytes of this type in a std430 shader storage block.
 *
 * Note that this is not GL_BUFFER_SIZE
 */
unsigned glsl_get_std430_size(const glsl_type *t, bool row_major);

/**
 * Size in bytes of this type based on its explicit data.
 *
 * When using SPIR-V shaders (ARB_gl_spirv), memory layouts are expressed
 * through explicit offset, stride and matrix layout, so the size
 * can/should be computed used those values.
 *
 * Note that the value returned by this method is only correct if such
 * values are set, so only with SPIR-V shaders. Should not be used with
 * GLSL shaders.
 */
unsigned glsl_get_explicit_size(const glsl_type *t, bool align_to_stride);

static inline unsigned
glsl_get_explicit_stride(const glsl_type *t)
{
   return t->explicit_stride;
}

static inline unsigned
glsl_get_explicit_alignment(const glsl_type *t)
{
   return t->explicit_alignment;
}

/**
 * Gets an explicitly laid out type with the std140 layout.
 */
const glsl_type *glsl_get_explicit_std140_type(const glsl_type *t, bool row_major);

/**
 * Gets an explicitly laid out type with the std430 layout.
 */
const glsl_type *glsl_get_explicit_std430_type(const glsl_type *t, bool row_major);

/**
 * Gets an explicitly laid out interface type.
 */
static inline const glsl_type *
glsl_get_explicit_interface_type(const glsl_type *t, bool supports_std430)
{
   enum glsl_interface_packing packing = glsl_get_internal_ifc_packing(t, supports_std430);
   if (packing == GLSL_INTERFACE_PACKING_STD140) {
      return glsl_get_explicit_std140_type(t, t->interface_row_major);
   } else {
      assert(packing == GLSL_INTERFACE_PACKING_STD430);
      return glsl_get_explicit_std430_type(t, t->interface_row_major);
   }
}

void glsl_get_natural_size_align_bytes(const glsl_type *t, unsigned *size, unsigned *align);
void glsl_get_vec4_size_align_bytes(const glsl_type *type, unsigned *size, unsigned *align);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GLSL_TYPES_H */
