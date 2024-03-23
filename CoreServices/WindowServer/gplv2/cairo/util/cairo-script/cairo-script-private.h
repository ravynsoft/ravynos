/*
 * Copyright © 2008 Chris Wilson <chris@chris-wilson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson.
 *
 * Contributor(s):
 *	Chris Wilson <chris@chris-wilson.co.uk>
 */

#ifndef CAIRO_SCRIPT_PRIVATE_H
#define CAIRO_SCRIPT_PRIVATE_H

#include "config.h"

#include "cairo-script-interpreter.h"

#include <setjmp.h>

#ifdef _MSC_VER
#undef inline
#define inline __inline
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#ifndef NULL
#define NULL (void *) 0
#endif

#if   HAVE_STDINT_H
# include <stdint.h>
#elif HAVE_INTTYPES_H
# include <inttypes.h>
#elif HAVE_SYS_INT_TYPES_H
# include <sys/int_types.h>
#elif defined(_MSC_VER)
  typedef __int8 int8_t;
  typedef unsigned __int8 uint8_t;
  typedef __int16 int16_t;
  typedef unsigned __int16 uint16_t;
  typedef __int32 int32_t;
  typedef unsigned __int32 uint32_t;
  typedef __int64 int64_t;
  typedef unsigned __int64 uint64_t;
# ifndef HAVE_UINT64_T
#  define HAVE_UINT64_T 1
# endif
#else
#error Cannot find definitions for fixed-width integral types (uint8_t, uint32_t, etc.)
#endif

#if HAVE_BYTESWAP_H
# include <byteswap.h>
#endif
#ifndef bswap_16
# define bswap_16(p) \
	(((((uint16_t)(p)) & 0x00ff) << 8) | \
	  (((uint16_t)(p))           >> 8))
#endif
#ifndef bswap_32
# define bswap_32(p) \
         (((((uint32_t)(p)) & 0x000000ff) << 24) | \
	  ((((uint32_t)(p)) & 0x0000ff00) << 8)  | \
	  ((((uint32_t)(p)) & 0x00ff0000) >> 8)  | \
	  ((((uint32_t)(p)))              >> 24))
#endif


#if __GNUC__ >= 3
#define csi_pure __attribute__((pure))
#define csi_const __attribute__((const))
#else
#define csi_pure
#define csi_const
#endif

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _CSI_BOOLEAN_EXPR(expr)                   \
 __extension__ ({                               \
   int _csi_boolean_var_;                         \
   if (expr)                                    \
      _csi_boolean_var_ = 1;                      \
   else                                         \
      _csi_boolean_var_ = 0;                      \
   _csi_boolean_var_;                             \
})
#define _csi_likely(expr) (__builtin_expect (_CSI_BOOLEAN_EXPR(expr), 1))
#define _csi_unlikely(expr) (__builtin_expect (_CSI_BOOLEAN_EXPR(expr), 0))
#else
#define _csi_likely(expr) (expr)
#define _csi_unlikely(expr) (expr)
#endif

#ifdef __GNUC__
#ifndef offsetof
#define offsetof(type, member) \
    ((char *) &((type *) 0)->member - (char *) 0)
#endif
#define csi_container_of(ptr, type, member) ({ \
    const typeof(((type *) 0)->member) *mptr__ = (ptr); \
    (type *) ((char *) mptr__ - offsetof (type, member)); \
})
#else
#define csi_container_of(ptr, type, member) \
    (type *)((char *) (ptr) - (char *) &((type *)0)->member)
#endif

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3)) && defined(__ELF__) && !defined(__sun)
#define csi_private_no_warn	__attribute__((__visibility__("hidden")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#define csi_private_no_warn	__hidden
#else /* not gcc >= 3.3 and not Sun Studio >= 8 */
#define csi_private_no_warn
#endif

#undef  ARRAY_LENGTH
#define ARRAY_LENGTH(__array) ((int) (sizeof (__array) / sizeof (__array[0])))

#ifndef WARN_UNUSED_RESULT
#define WARN_UNUSED_RESULT
#endif
/* Add attribute(warn_unused_result) if supported */
#define csi_warn	    WARN_UNUSED_RESULT
#define csi_private	    csi_private_no_warn csi_warn

#define CSI_BITSWAP8(c) ((((c) * 0x0802LU & 0x22110LU) | ((c) * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16)
#ifdef WORDS_BIGENDIAN
#define CSI_BITSWAP8_IF_LITTLE_ENDIAN(c) (c)
#else
#define CSI_BITSWAP8_IF_LITTLE_ENDIAN(c) CSI_BITSWAP8(c)
#endif

typedef enum _csi_status {
    CSI_STATUS_SUCCESS = CAIRO_STATUS_SUCCESS,
    CSI_STATUS_NO_MEMORY = CAIRO_STATUS_NO_MEMORY,
    CSI_STATUS_INVALID_RESTORE = CAIRO_STATUS_INVALID_RESTORE,
    CSI_STATUS_INVALID_POP_GROUP = CAIRO_STATUS_INVALID_POP_GROUP,
    CSI_STATUS_NO_CURRENT_POINT = CAIRO_STATUS_NO_CURRENT_POINT,
    CSI_STATUS_INVALID_MATRIX = CAIRO_STATUS_INVALID_MATRIX,
    CSI_STATUS_INVALID_STATUS = CAIRO_STATUS_INVALID_STATUS,
    CSI_STATUS_NULL_POINTER = CAIRO_STATUS_NULL_POINTER,
    CSI_STATUS_INVALID_STRING = CAIRO_STATUS_INVALID_STRING,
    CSI_STATUS_INVALID_PATH_DATA = CAIRO_STATUS_INVALID_PATH_DATA,
    CSI_STATUS_READ_ERROR = CAIRO_STATUS_READ_ERROR,
    CSI_STATUS_WRITE_ERROR = CAIRO_STATUS_WRITE_ERROR,
    CSI_STATUS_SURFACE_FINISHED = CAIRO_STATUS_SURFACE_FINISHED,
    CSI_STATUS_SURFACE_TYPE_MISMATCH = CAIRO_STATUS_SURFACE_TYPE_MISMATCH,
    CSI_STATUS_PATTERN_TYPE_MISMATCH = CAIRO_STATUS_PATTERN_TYPE_MISMATCH,
    CSI_STATUS_INVALID_CONTENT = CAIRO_STATUS_INVALID_CONTENT,
    CSI_STATUS_INVALID_FORMAT = CAIRO_STATUS_INVALID_FORMAT,
    CSI_STATUS_INVALID_VISUAL = CAIRO_STATUS_INVALID_VISUAL,
    CSI_STATUS_FILE_NOT_FOUND = CAIRO_STATUS_FILE_NOT_FOUND,
    CSI_STATUS_INVALID_DASH = CAIRO_STATUS_INVALID_DASH,
    CSI_STATUS_INVALID_DSC_COMMENT = CAIRO_STATUS_INVALID_DSC_COMMENT,
    CSI_STATUS_INVALID_INDEX = CAIRO_STATUS_INVALID_INDEX,
    CSI_STATUS_CLIP_NOT_REPRESENTABLE = CAIRO_STATUS_CLIP_NOT_REPRESENTABLE,
    CSI_STATUS_TEMP_FILE_ERROR = CAIRO_STATUS_TEMP_FILE_ERROR,
    CSI_STATUS_INVALID_STRIDE = CAIRO_STATUS_INVALID_STRIDE,
    CSI_STATUS_FONT_TYPE_MISMATCH = CAIRO_STATUS_FONT_TYPE_MISMATCH,
    CSI_STATUS_USER_FONT_IMMUTABLE = CAIRO_STATUS_USER_FONT_IMMUTABLE,
    CSI_STATUS_USER_FONT_ERROR = CAIRO_STATUS_USER_FONT_ERROR,
    CSI_STATUS_NEGATIVE_COUNT = CAIRO_STATUS_NEGATIVE_COUNT,
    CSI_STATUS_INVALID_CLUSTERS = CAIRO_STATUS_INVALID_CLUSTERS,
    CSI_STATUS_INVALID_SLANT = CAIRO_STATUS_INVALID_SLANT,
    CSI_STATUS_INVALID_WEIGHT = CAIRO_STATUS_INVALID_WEIGHT,
    CSI_STATUS_INVALID_SIZE = CAIRO_STATUS_INVALID_SIZE,
    CSI_STATUS_USER_FONT_NOT_IMPLEMENTED = CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED,
    CSI_STATUS_DEVICE_TYPE_MISMATCH = CAIRO_STATUS_DEVICE_TYPE_MISMATCH,
    CSI_STATUS_DEVICE_ERROR = CAIRO_STATUS_DEVICE_ERROR,
    CSI_STATUS_INVALID_MESH_CONSTRUCTION = CAIRO_STATUS_INVALID_MESH_CONSTRUCTION,
    CSI_STATUS_DEVICE_FINISHED = CAIRO_STATUS_DEVICE_FINISHED,
    CSI_STATUS_JBIG2_GLOBAL_MISSING = CAIRO_STATUS_JBIG2_GLOBAL_MISSING,
    CSI_STATUS_PNG_ERROR = CAIRO_STATUS_PNG_ERROR,
    CSI_STATUS_FREETYPE_ERROR = CAIRO_STATUS_FREETYPE_ERROR,
    CSI_STATUS_WIN32_GDI_ERROR = CAIRO_STATUS_WIN32_GDI_ERROR,

    /* cairo-script-interpreter specific errors */

    CSI_STATUS_INVALID_SCRIPT,
    CSI_STATUS_SCRIPT_INVALID_TYPE,
    CSI_STATUS_SCRIPT_INVALID_INDEX,
    CSI_STATUS_SCRIPT_UNDEFINED_NAME,
    CSI_STATUS_INTERPRETER_FINISHED,

    _CSI_STATUS_SCRIPT_LAST_ERROR,
    CSI_INT_STATUS_UNSUPPORTED
} csi_status_t;

typedef enum {
    CSI_OBJECT_TYPE_NULL = 0,

    /* atomics */
    CSI_OBJECT_TYPE_BOOLEAN,
    CSI_OBJECT_TYPE_INTEGER,
    CSI_OBJECT_TYPE_MARK,
    CSI_OBJECT_TYPE_NAME,
    CSI_OBJECT_TYPE_OPERATOR,
    CSI_OBJECT_TYPE_REAL,

    /* compound */
    CSI_OBJECT_TYPE_ARRAY = 0x8,
    CSI_OBJECT_TYPE_DICTIONARY,
    CSI_OBJECT_TYPE_FILE,
    CSI_OBJECT_TYPE_MATRIX,
    CSI_OBJECT_TYPE_STRING,

    /* cairo */
    CSI_OBJECT_TYPE_CONTEXT = 0x10,
    CSI_OBJECT_TYPE_FONT,
    CSI_OBJECT_TYPE_PATTERN,
    CSI_OBJECT_TYPE_SCALED_FONT,
    CSI_OBJECT_TYPE_SURFACE
} csi_object_type_t;

#define CSI_OBJECT_IS_ATOM(OBJ) (((OBJ)->type & CSI_OBJECT_TYPE_MASK) < 0x08)
#define CSI_OBJECT_IS_COMPOUND(OBJ) ((OBJ)->type & 0x08)
#define CSI_OBJECT_IS_CAIRO(OBJ) ((OBJ)->type & 0x10)

enum { /* attributes */
    CSI_OBJECT_ATTR_EXECUTABLE = 1 << 6,
    CSI_OBJECT_ATTR_WRITABLE   = 1 << 7
};
#define CSI_OBJECT_ATTR_MASK (CSI_OBJECT_ATTR_EXECUTABLE | \
			      CSI_OBJECT_ATTR_WRITABLE)
#define CSI_OBJECT_TYPE_MASK (~CSI_OBJECT_ATTR_MASK)

typedef struct _cairo_script_interpreter csi_t;

typedef cairo_bool_t csi_boolean_t;
typedef csi_status_t (*csi_operator_t) (csi_t *);
typedef float csi_real_t;
typedef long csi_integer_t;
typedef intptr_t csi_name_t;
typedef struct _csi_array csi_array_t;
typedef struct _csi_buffer csi_buffer_t;
typedef struct _csi_compound_object csi_compound_object_t;
typedef struct _csi_dictionary csi_dictionary_t;
typedef struct _csi_file csi_file_t;
typedef struct _csi_hash_entry csi_hash_entry_t;
typedef struct _csi_hash_table csi_hash_table_t;
typedef struct _csi_hash_table_arrangement csi_hash_table_arrangement_t;
typedef struct _csi_list csi_list_t;
typedef struct _csi_matrix csi_matrix_t;
typedef struct _csi_object csi_object_t;
typedef struct _csi_scanner csi_scanner_t;
typedef struct _csi_stack csi_stack_t;
typedef struct _csi_string csi_string_t;

typedef cairo_bool_t
(*csi_hash_predicate_func_t) (void *entry);

typedef void
(*csi_hash_callback_func_t) (void *entry,
			     void *closure);

typedef cairo_bool_t
(*csi_hash_keys_equal_func_t) (const void *key_a, const void *key_b);

struct _csi_object {
    csi_object_type_t type;
    union {
	cairo_t *cr;
	cairo_font_face_t *font_face;
	cairo_pattern_t *pattern;
	cairo_scaled_font_t *scaled_font;
	cairo_surface_t *surface;
	csi_array_t *array;
	csi_boolean_t boolean;
	csi_compound_object_t *object;
	csi_dictionary_t *dictionary;
	csi_file_t *file;
	csi_integer_t integer;
	csi_matrix_t *matrix;
	csi_operator_t op;
	csi_name_t name;
	csi_real_t real;
	csi_string_t *string;
	void *ptr;
    } datum;
};

struct _csi_compound_object {
    csi_object_type_t type;
    unsigned int ref;
};

struct _csi_hash_entry {
    unsigned long hash;
};

struct _csi_hash_table_arrangement {
    unsigned long high_water_mark;
    unsigned long size;
    unsigned long rehash;
};

struct _csi_hash_table {
    csi_hash_keys_equal_func_t keys_equal;

    const csi_hash_table_arrangement_t *arrangement;
    csi_hash_entry_t **entries;

    unsigned long live_entries;
    unsigned long used_entries;
    unsigned long iterating;   /* Iterating, no insert, no resize */
};


/* simple, embedded doubly-linked links */
struct _csi_list {
    struct _csi_list *next, *prev;
};

struct _csi_buffer {
    char *base, *ptr, *end;
    unsigned int size;
};

struct _csi_stack {
    csi_object_t *objects;
    csi_integer_t len;
    csi_integer_t size;
};

struct _csi_array {
    csi_compound_object_t base;
    csi_stack_t stack;
};

typedef struct _csi_dictionary_entry {
    csi_hash_entry_t hash_entry;
    csi_object_t value;
} csi_dictionary_entry_t;

struct _csi_dictionary {
    csi_compound_object_t base;
    csi_hash_table_t hash_table;
};

struct _csi_matrix {
    csi_compound_object_t base;
    cairo_matrix_t matrix;
};

struct _csi_string {
    csi_compound_object_t base;
    csi_integer_t len;
    csi_integer_t deflate;
    enum {
	NONE,
	ZLIB,
	LZO,
    } method;
    char *string;
};

typedef struct _csi_filter_funcs {
    int (*filter_getc) (csi_file_t *);
    void (*filter_putc) (csi_file_t *, int);
    int (*filter_read) (csi_file_t *, uint8_t *, int);
    void (*filter_destroy) (csi_t *, void *);
} csi_filter_funcs_t;

struct _csi_file {
    csi_compound_object_t base;
    enum {
	STDIO,
	BYTES,
	PROCEDURE,
	FILTER
    } type;
    unsigned int flags;
    void *src;
    void *data;
    uint8_t *bp;
    int rem;
    const csi_filter_funcs_t *filter;
};

union _csi_union_object {
    void *ptr[2];
    csi_stack_t stack;
    csi_array_t arry;
    csi_dictionary_t dictionary;
    csi_matrix_t matrix;
    csi_string_t string;
    csi_file_t file;
    csi_object_t object;
};

struct _csi_scanner {
    jmp_buf jump_buffer;
    int depth;

    int bind;
    csi_status_t (*push) (csi_t *ctx, csi_object_t *obj);
    csi_status_t (*execute) (csi_t *ctx, csi_object_t *obj);
    void *closure;

    csi_buffer_t buffer;
    csi_stack_t procedure_stack;
    csi_object_t build_procedure;

    unsigned int accumulator;
    unsigned int accumulator_count;

    unsigned int line_number;
};

typedef cairo_script_interpreter_hooks_t csi_hooks_t;

typedef struct _csi_chunk {
    struct _csi_chunk *next;
    int rem;
    char *ptr;
} csi_chunk_t;

struct _cairo_script_interpreter {
    int ref_count;
    csi_status_t status;

    unsigned int finished : 1;

    csi_hooks_t hooks;

    csi_hash_table_t strings;

    csi_stack_t ostack;
    csi_stack_t dstack;

    csi_scanner_t scanner;

    csi_chunk_t *perm_chunk;
    struct {
	csi_chunk_t *chunk;
	void *free_list;
    } slabs[16];
    csi_array_t *free_array;
    csi_dictionary_t *free_dictionary;
    csi_string_t *free_string;

    csi_operator_t opcode[256];

    /* caches of live data */
    csi_list_t *_images;
    csi_list_t *_faces;
};

typedef struct _csi_operator_def {
    const char *name;
    csi_operator_t op;
} csi_operator_def_t;

typedef struct _csi_integer_constant_def {
    const char *name;
    csi_integer_t value;
} csi_integer_constant_def_t;

typedef struct _csi_real_constant_def {
    const char *name;
    csi_real_t value;
} csi_real_constant_def_t;

/* cairo-script-file.c */

csi_private csi_status_t
csi_file_new (csi_t *ctx,
	      csi_object_t *obj,
	      const char *path, const char *mode);

csi_private csi_status_t
csi_file_new_for_stream (csi_t *ctx,
	                 csi_object_t *obj,
			 FILE *stream);

csi_private csi_status_t
csi_file_new_for_bytes (csi_t *ctx,
			csi_object_t *obj,
			const char *bytes,
			unsigned int length);

csi_private csi_status_t
csi_file_new_from_string (csi_t *ctx,
			  csi_object_t *obj,
			  csi_string_t *src);

csi_private csi_status_t
csi_file_new_ascii85_decode (csi_t *ctx,
			     csi_object_t *obj,
			     csi_dictionary_t *dict,
			     csi_object_t *src);

csi_private csi_status_t
csi_file_new_deflate_decode (csi_t *ctx,
			     csi_object_t *obj,
			     csi_dictionary_t *dict,
			     csi_object_t *src);

csi_private csi_status_t
_csi_file_execute (csi_t *ctx, csi_file_t *obj);

csi_private int
csi_file_getc (csi_file_t *obj);

csi_private int
csi_file_read (csi_file_t *obj, void *buf, int len);

csi_private void
csi_file_putc (csi_file_t *obj, int c);

csi_private void
csi_file_flush (csi_file_t *obj);

csi_private void
csi_file_close (csi_t *ctx, csi_file_t *obj);

csi_private void
_csi_file_free (csi_t *ctx, csi_file_t *obj);

csi_private csi_status_t
_csi_file_as_string (csi_t *ctx,
		     csi_file_t *file,
		     csi_object_t *obj);

/* cairo-script-hash.c */

csi_private csi_status_t
_csi_hash_table_init (csi_hash_table_t *hash_table,
		      csi_hash_keys_equal_func_t keys_equal);

csi_private void
_csi_hash_table_fini (csi_hash_table_t *hash_table);

csi_private void *
_csi_hash_table_lookup (csi_hash_table_t  *hash_table,
			csi_hash_entry_t  *key);

csi_private csi_status_t
_csi_hash_table_insert (csi_hash_table_t *hash_table,
			csi_hash_entry_t *entry);

csi_private void
_csi_hash_table_remove (csi_hash_table_t *hash_table,
			csi_hash_entry_t *key);

csi_private void
_csi_hash_table_foreach (csi_hash_table_t	      *hash_table,
			 csi_hash_callback_func_t  hash_callback,
			 void			      *closure);

/* cairo-script-interpreter.c */

csi_private void *
_csi_alloc (csi_t *ctx, int size);

csi_private void *
_csi_alloc0 (csi_t *ctx, int size);

csi_private void *
_csi_realloc (csi_t *ctx, void *ptr, int size);

csi_private void
_csi_free (csi_t *ctx, void *ptr);

csi_private void *
_csi_slab_alloc (csi_t *ctx, int size);

csi_private void *
_csi_perm_alloc (csi_t *ctx, int size);

csi_private void
_csi_slab_free (csi_t *ctx, void *ptr, int size);

csi_private csi_status_t
csi_push_ostack (csi_t *ctx, csi_object_t *obj);

csi_private csi_status_t
_csi_name_define (csi_t *ctx, csi_name_t name, csi_object_t *obj);

csi_private csi_status_t
_csi_name_lookup (csi_t *ctx, csi_name_t name, csi_object_t *obj);

csi_private csi_status_t
_csi_name_undefine (csi_t *ctx, csi_name_t name);

csi_private csi_status_t
_csi_intern_string (csi_t *ctx, const char **str_inout, int len);

csi_private csi_status_t
_csi_error (csi_status_t status);

/* cairo-script-objects.c */

csi_private csi_status_t
csi_array_new (csi_t *ctx,
	       csi_integer_t initial_size,
	       csi_object_t *obj);

csi_private csi_status_t
_csi_array_execute (csi_t *ctx, csi_array_t *array);

csi_private csi_status_t
csi_array_get (csi_t *ctx,
	       csi_array_t *array,
	       long elem,
	       csi_object_t *value);

csi_private csi_status_t
csi_array_put (csi_t *ctx,
	       csi_array_t *array,
	       csi_integer_t elem,
	       csi_object_t *value);

csi_private csi_status_t
csi_array_append (csi_t *ctx,
		  csi_array_t *array,
		  csi_object_t *obj);

csi_private void
csi_array_free (csi_t *ctx, csi_array_t *array);

static inline void
csi_boolean_new (csi_object_t *obj,
		 csi_boolean_t v)
{
    obj->type = CSI_OBJECT_TYPE_BOOLEAN;
    obj->datum.boolean = v;
}

csi_private csi_status_t
csi_dictionary_new (csi_t *ctx,
		    csi_object_t *obj);

csi_private csi_status_t
csi_dictionary_put (csi_t *ctx,
		    csi_dictionary_t *dict,
		    csi_name_t name,
		    csi_object_t *value);

csi_private csi_status_t
csi_dictionary_get (csi_t *ctx,
		    csi_dictionary_t *dict,
		    csi_name_t name,
		    csi_object_t *value);

csi_private csi_boolean_t
csi_dictionary_has (csi_dictionary_t *dict,
		    csi_name_t name);

csi_private void
csi_dictionary_remove (csi_t *ctx,
		       csi_dictionary_t *dict,
		       csi_name_t name);

csi_private void
csi_dictionary_free (csi_t *ctx,
		     csi_dictionary_t *dict);

static inline void
csi_integer_new (csi_object_t *obj,
		 csi_integer_t v)
{
    obj->type = CSI_OBJECT_TYPE_INTEGER;
    obj->datum.integer = v;
}


csi_private csi_status_t
csi_matrix_new (csi_t *ctx,
		csi_object_t *obj);

csi_private csi_status_t
csi_matrix_new_from_array (csi_t *ctx,
			   csi_object_t *obj,
			   csi_array_t *array);

csi_private csi_status_t
csi_matrix_new_from_matrix (csi_t *ctx,
			    csi_object_t *obj,
			    const cairo_matrix_t *m);

csi_private csi_status_t
csi_matrix_new_from_values (csi_t *ctx,
			    csi_object_t *obj,
			    double v[6]);

csi_private void
csi_matrix_free (csi_t *ctx,
		 csi_matrix_t *obj);

csi_private csi_status_t
csi_name_new (csi_t *ctx,
	      csi_object_t *obj,
	      const char *str,
	      int len);

csi_private csi_status_t
csi_name_new_static (csi_t *ctx,
		     csi_object_t *obj,
		     const char *str);

static inline void
csi_operator_new (csi_object_t *obj,
		  csi_operator_t op)
{
    obj->type = CSI_OBJECT_TYPE_OPERATOR | CSI_OBJECT_ATTR_EXECUTABLE;
    obj->datum.op = op;
}

static inline void
csi_real_new (csi_object_t *obj,
	      csi_real_t v)
{
    obj->type = CSI_OBJECT_TYPE_REAL;
    obj->datum.real = v;
}

csi_private csi_status_t
csi_string_new (csi_t *ctx,
		csi_object_t *obj,
		const char *str,
		int len);

csi_private csi_status_t
csi_string_deflate_new (csi_t *ctx,
			csi_object_t *obj,
			void *bytes,
			int in_len,
			int out_len);

csi_private csi_status_t
csi_string_new_from_bytes (csi_t *ctx,
	                   csi_object_t *obj,
			   char *bytes,
			   unsigned int len);

csi_private void
csi_string_free (csi_t *ctx, csi_string_t *string);

csi_private csi_status_t
csi_object_execute (csi_t *ctx, csi_object_t *obj);

csi_private csi_object_t *
csi_object_reference (csi_object_t *obj);

csi_private void
csi_object_free (csi_t *ctx,
		 csi_object_t *obj);

csi_private csi_status_t
csi_object_as_file (csi_t *ctx,
		    csi_object_t *src,
		    csi_object_t *file);

csi_private csi_boolean_t
csi_object_eq (csi_object_t *a,
	       csi_object_t *b);

csi_private csi_status_t
csi_object_compare (csi_object_t *a,
		    csi_object_t *b,
		    int          *out_cmp);

/* cairo-script-operators.c */

csi_private const csi_operator_def_t *
_csi_operators (void);

csi_private const csi_integer_constant_def_t *
_csi_integer_constants (void);

csi_private const csi_real_constant_def_t *
_csi_real_constants (void);

/* cairo-script-scanner.c */

csi_private csi_status_t
_csi_scanner_init (csi_t *ctx, csi_scanner_t *scanner);

csi_private csi_status_t
_csi_scan_file (csi_t *ctx, csi_file_t *src);

csi_private csi_status_t
_csi_translate_file (csi_t *ctx,
	             csi_file_t *file,
		     cairo_write_func_t write_func,
		     void *closure);

csi_private void
_csi_scanner_fini (csi_t *ctx, csi_scanner_t *scanner);

csi_private csi_boolean_t
_csi_parse_number (csi_object_t *obj, const char *s, int len);

/* cairo-script-stack.c */

csi_private csi_status_t
_csi_stack_init (csi_t *ctx, csi_stack_t *stack, csi_integer_t size);

csi_private void
_csi_stack_fini (csi_t *ctx, csi_stack_t *stack);

csi_private csi_status_t
_csi_stack_roll (csi_t *ctx,
		 csi_stack_t *stack,
		 csi_integer_t mod,
		 csi_integer_t n);

csi_private csi_status_t
_csi_stack_grow (csi_t *ctx, csi_stack_t *stack, csi_integer_t cnt);

csi_private csi_status_t
_csi_stack_push_internal (csi_t *ctx, csi_stack_t *stack,
			  const csi_object_t *obj);

csi_private csi_object_t *
_csi_stack_peek (csi_stack_t *stack, csi_integer_t i);

csi_private void
_csi_stack_pop (csi_t *ctx, csi_stack_t *stack, csi_integer_t count);

csi_private csi_status_t
_csi_stack_exch (csi_stack_t *stack);

static inline csi_object_type_t
csi_object_get_type (const csi_object_t *obj)
{
    return obj->type & CSI_OBJECT_TYPE_MASK;
}

static inline csi_boolean_t
csi_object_is_procedure (const csi_object_t *obj)
{
    return obj->type == (CSI_OBJECT_TYPE_ARRAY | CSI_OBJECT_ATTR_EXECUTABLE);
}

static inline csi_boolean_t
csi_object_is_number (const csi_object_t *obj)
{
    int type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN:
    case CSI_OBJECT_TYPE_INTEGER:
    case CSI_OBJECT_TYPE_REAL:
	return 1;
    default:
	return 0;
    }
}

static inline double
csi_number_get_value (const csi_object_t *obj)
{
    int type = csi_object_get_type (obj);
    switch (type) {
    case CSI_OBJECT_TYPE_BOOLEAN: return obj->datum.boolean;
    case CSI_OBJECT_TYPE_INTEGER: return obj->datum.integer;
    case CSI_OBJECT_TYPE_REAL: return obj->datum.real;
    default: return 0.;
    }
}

csi_private csi_status_t
_csi_stack_push (csi_t *ctx, csi_stack_t *stack,
		 const csi_object_t *obj);

static inline csi_boolean_t
_csi_check_ostack (csi_t *ctx, csi_integer_t count)
{
    return ctx->ostack.len >= count;
}

static inline csi_object_t *
_csi_peek_ostack (csi_t *ctx, csi_integer_t i)
{
    return &ctx->ostack.objects[ctx->ostack.len - i -1];
}

static inline void
_csi_pop_ostack (csi_t *ctx, csi_integer_t count)
{
    do
	csi_object_free (ctx, &ctx->ostack.objects[--ctx->ostack.len]);
    while (--count);
}

static inline csi_status_t
_csi_push_ostack_copy (csi_t *ctx, csi_object_t *obj)
{
    return _csi_stack_push (ctx, &ctx->ostack, csi_object_reference (obj));
}

static inline csi_status_t
_csi_push_ostack (csi_t *ctx, csi_object_t *obj)
{
    return _csi_stack_push (ctx, &ctx->ostack, obj);
}

static inline csi_status_t
_csi_push_ostack_boolean (csi_t *ctx, csi_boolean_t v)
{
    csi_object_t obj;
    obj.type = CSI_OBJECT_TYPE_BOOLEAN;
    obj.datum.boolean = v;
    return _csi_stack_push (ctx, &ctx->ostack, &obj);
}
static inline csi_status_t
_csi_push_ostack_integer (csi_t *ctx, csi_integer_t v)
{
    csi_object_t obj;
    obj.type = CSI_OBJECT_TYPE_INTEGER;
    obj.datum.integer = v;
    return _csi_stack_push (ctx, &ctx->ostack, &obj);
}
static inline csi_status_t
_csi_push_ostack_mark (csi_t *ctx)
{
    csi_object_t obj;
    obj.type = CSI_OBJECT_TYPE_MARK;
    return _csi_stack_push (ctx, &ctx->ostack, &obj);
}
static inline csi_status_t
_csi_push_ostack_null (csi_t *ctx)
{
    csi_object_t obj;
    obj.type = CSI_OBJECT_TYPE_NULL;
    return _csi_stack_push (ctx, &ctx->ostack, &obj);
}
static inline csi_status_t
_csi_push_ostack_real (csi_t *ctx, csi_real_t v)
{
    csi_object_t obj;
    obj.type = CSI_OBJECT_TYPE_REAL;
    obj.datum.real = v;
    return _csi_stack_push (ctx, &ctx->ostack, &obj);
}

#endif /* CAIRO_SCRIPT_PRIVATE_H */
