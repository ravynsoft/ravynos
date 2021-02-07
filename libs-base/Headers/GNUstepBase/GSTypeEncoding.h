/*
 * Objective-C type encoding support
 *
 * Copyright (C) 2012-2014 Free Software Foundation, Inc.
 *
 * Written by Marat Ibadinov <ibadinov@me.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef GS_TYPE_ENCODING_H
#define GS_TYPE_ENCODING_H

#include <GNUstepBase/GSVersionMacros.h>

#if defined(__MINGW__)
/* On MINGW we need to get the boolean type from the runtime at this point.
 */
#define _NO_BOOL_TYPEDEF
#endif

#if defined (NeXT_RUNTIME)
#  include <objc/objc-runtime.h>
#else
#  include <objc/objc.h>
#  if defined (__GNU_LIBOBJC__)
#    include <objc/runtime.h>
#  else
#    include <objc/objc-api.h>
#    include <objc/encoding.h>
#  endif
#endif

#if !defined(GS_STATIC_INLINE)
#  if defined(__GNUC__)
#    define GS_STATIC_INLINE static __inline__ __attribute__((always_inline))
#  else
#    define GS_STATIC_INLINE static inline
#  endif
#endif

/* type mangling is compiler independent so we can safely define this by hand */

typedef enum GSObjCTypeQualifier
{
  GSObjCQualifierConst        = 'r',
  GSObjCQualifierIn           = 'n',
  GSObjCQualifierInOut        = 'N',
  GSObjCQualifierOut          = 'o',
  GSObjCQualifierByCopy       = 'O',
  GSObjCQualifierByRef        = 'R',
  GSObjCQualifierOneWay       = 'V',
  GSObjCQualifierInvisible    = '!'
} GSObjCTypeQualifier;

typedef enum GSObjCType
{
  GSObjCTypeId                = '@',
  GSObjCTypeClass             = '#',
  GSObjCTypeSelector          = ':',
  GSObjCTypeChar              = 'c',
  GSObjCTypeUnsignedChar      = 'C',
  GSObjCTypeShort             = 's',
  GSObjCTypeUnsignedShort     = 'S',
  GSObjCTypeInt               = 'i',
  GSObjCTypeUnsignedInt       = 'I',
  GSObjCTypeLong              = 'l',
  GSObjCTypeUnsignedLong      = 'L',
  GSObjCTypeLongLong          = 'q',
  GSObjCTypeUnsignedLongLong  = 'Q',
  GSObjCTypeFloat             = 'f',
  GSObjCTypeDouble            = 'd',
  GSObjCTypeComplex           = 'j',
  GSObjCTypeBitField          = 'b',
  GSObjCTypeBool              = 'B',
  GSObjCTypeVoid              = 'v',
  GSObjCTypePointer           = '^',
  GSObjCTypeCharPointer       = '*',
  GSObjCTypeAtom              = '%',
  GSObjCTypeArrayBegin        = '[',
  GSObjCTypeArrayEnd          = ']',
  GSObjCTypeStructureBegin    = '{',
  GSObjCTypeStructureEnd      = '}',
  GSObjCTypeUnionBegin        = '(',
  GSObjCTypeUnionEnd          = ')',
  GSObjCTypeUnknown           = '?'
} GSObjCType;

/* maximum and minimum char values in a type specification */
typedef enum GSObjCTypeBound
{
  GSObjCTypeMin               = ' ',
  GSObjCTypeMax               = '~'
} GSObjCTypeBound;

#if defined (NeXT_RUNTIME)
typedef enum GSObjCTypeQualifierMask
{
  GSObjCQualifierConstMask        = 0x01,
  GSObjCQualifierInMask           = 0x01,
  GSObjCQualifierOutMask          = 0x02,
  GSObjCQualifierInOutMask        = 0x03,
  GSObjCQualifierByCopyMask       = 0x04,
  GSObjCQualifierByRefMask        = 0x08,
  GSObjCQualifierOneWayMask       = 0x10,
  GSObjCQualifierInvisibleMask    = 0x20
} GSObjCTypeQualifierMask;
#else
typedef enum GSObjCTypeQualifierMask
{
  GSObjCQualifierConstMask        = _F_CONST,
  GSObjCQualifierInMask           = _F_IN,
  GSObjCQualifierOutMask          = _F_OUT,
  GSObjCQualifierInOutMask        = _F_INOUT,
  GSObjCQualifierByCopyMask       = _F_BYCOPY,
  GSObjCQualifierByRefMask        = _F_BYREF,
  GSObjCQualifierOneWayMask       = _F_ONEWAY,
  GSObjCQualifierInvisibleMask    = _F_GCINVISIBLE
} GSObjCTypeQualifierMask;
#endif

/*
 * parser-related stuff
 */

typedef struct GSObjCTypeInfo {
  /* store pointer to allow recursive parsing of pointer types, e.g. ^{^[2*]} */
  const char  *type;
  size_t      size;
  uint8_t     alignment;
  uint8_t     qualifiers;
} GSObjCTypeInfo;

typedef void (*GSObjCTypeParserDelegate)(void *context, GSObjCTypeInfo type);

typedef enum GSObjCParserOptions {
  GSObjCReportArrayOnceMask = 1
} GSObjCParserOptions;

const char *
GSObjCParseTypeSpecification (const char *cursor,
                              GSObjCTypeParserDelegate delegate,
                              void *context,
                              unsigned options);

GS_STATIC_INLINE size_t
GSObjCPadSize (size_t size, uint8_t alignment)
{
  return alignment * ((size + alignment - 1) / alignment);
}

GS_STATIC_INLINE size_t
GSObjCGetPadding (size_t size, uint8_t alignment)
{
  return (alignment - (size & (alignment - 1))) & (alignment - 1);
}

const char *
GSGetSizeAndAlignment (const char *type, size_t *sizep, uint8_t *alignp);


#if defined (NeXT_RUNTIME)

/* GNU API support for NeXT runtime */

int
objc_sizeof_type (const char* type);
int
objc_alignof_type (const char* type);
int
objc_aligned_size (const char* type);
int
objc_promoted_size (const char* type);

unsigned
objc_get_type_qualifiers (const char* type);

const char *
objc_skip_typespec (const char* type);
const char *
objc_skip_offset (const char* type);
const char *
objc_skip_argspec (const char* type);
const char *
objc_skip_type_qualifiers (const char* type);

struct objc_struct_layout
{
  GSObjCTypeInfo  *info;
  long            position;
  unsigned        count;
  unsigned        allocated;
  unsigned        depth;
  unsigned        offset;
  unsigned        alignment;
};

void
objc_layout_structure (const char *type,
                       struct objc_struct_layout *layout);

BOOL
objc_layout_structure_next_member (struct objc_struct_layout *layout);

void
objc_layout_structure_get_info (struct objc_struct_layout *layout,
                                unsigned int *offset,
                                unsigned int *align,
                                const char **type);

void
objc_layout_finish_structure (struct objc_struct_layout *layout,
                              unsigned int *size,
                              unsigned int *align);

#endif /* NeXT_RUNTIME */
#endif /* GS_TYPE_ENCODING_H */
