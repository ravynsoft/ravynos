/*
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * Vertex fetch/store/convert code.  This functionality is used in two places:
 * 1. Vertex fetch/convert - to grab vertex data from incoming vertex
 *    arrays and convert to format needed by vertex shaders.
 * 2. Vertex store/emit - to convert simple float[][4] vertex attributes
 *    (which is the organization used throughout the draw/prim pipeline) to
 *    hardware-specific formats and emit into hardware vertex buffers.
 *
 *
 * Authors:
 *    Keith Whitwell <keithw@vmware.com>
 */

#ifndef _TRANSLATE_H
#define _TRANSLATE_H


#include "util/compiler.h"
#include "util/format/u_formats.h"
#include "pipe/p_state.h"

/**
 * Translate has to work on two more attributes because
 * the draw module has to be able to pass a few fixed
 * function vertex shader outputs even if the fragment
 * shader already consumes PIPE_MAX_ATTRIBS inputs.
 *
 * These vertex shader outputs include:
 * - position
 * - bcolor (up to two)
 * - point-size
 * - viewport index
 * - layer
 */
#define TRANSLATE_MAX_ATTRIBS (PIPE_MAX_ATTRIBS + 6)

enum translate_element_type {
   TRANSLATE_ELEMENT_NORMAL,
   TRANSLATE_ELEMENT_INSTANCE_ID
};

struct translate_element 
{
   enum translate_element_type type;
   enum pipe_format input_format;
   enum pipe_format output_format;
   unsigned input_buffer:8;
   unsigned input_offset:24;
   unsigned instance_divisor;
   unsigned output_offset;
};


struct translate_key {
   unsigned output_stride;
   unsigned nr_elements;
   struct translate_element element[TRANSLATE_MAX_ATTRIBS];
};


struct translate;


typedef void (UTIL_CDECL *run_elts_func)(struct translate *,
                                         const unsigned *elts,
                                         unsigned count,
                                         unsigned start_instance,
                                         unsigned instance_id,
                                         void *output_buffer);

typedef void (UTIL_CDECL *run_elts16_func)(struct translate *,
                                           const uint16_t *elts,
                                           unsigned count,
                                           unsigned start_instance,
                                           unsigned instance_id,
                                           void *output_buffer);

typedef void (UTIL_CDECL *run_elts8_func)(struct translate *,
                                          const uint8_t *elts,
                                          unsigned count,
                                          unsigned start_instance,
                                          unsigned instance_id,
                                          void *output_buffer);

typedef void (UTIL_CDECL *run_func)(struct translate *,
                                    unsigned start,
                                    unsigned count,
                                    unsigned start_instance,
                                    unsigned instance_id,
                                    void *output_buffer);

struct translate {
   struct translate_key key;

   void (*release)( struct translate * );

   void (*set_buffer)( struct translate *,
		       unsigned i,
		       const void *ptr,
		       unsigned stride,
		       unsigned max_index );

   run_elts_func run_elts;
   run_elts16_func run_elts16;
   run_elts8_func run_elts8;
   run_func run;
};



struct translate *translate_create( const struct translate_key *key );

bool translate_is_output_format_supported(enum pipe_format format);

static inline int translate_keysize( const struct translate_key *key )
{
   assert(key->nr_elements <= TRANSLATE_MAX_ATTRIBS);
   return 2 * sizeof(int) + key->nr_elements * sizeof(struct translate_element);
}

static inline int translate_key_compare( const struct translate_key *a,
                                         const struct translate_key *b )
{
   int keysize_a = translate_keysize(a);
   int keysize_b = translate_keysize(b);

   if (keysize_a != keysize_b) {
      return keysize_a - keysize_b;
   }
   return memcmp(a, b, keysize_a);
}


static inline void translate_key_sanitize( struct translate_key *a )
{
   int keysize = translate_keysize(a);
   char *ptr = (char *)a;
   memset(ptr + keysize, 0, sizeof(*a) - keysize);
}


/*******************************************************************************
 *  Private:
 */
struct translate *translate_sse2_create( const struct translate_key *key );

struct translate *translate_generic_create( const struct translate_key *key );

bool translate_generic_is_output_format_supported(enum pipe_format format);

#endif
