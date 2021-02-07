/*
 * Objective-C type encoding support
 *
 * Copyright (C) 2012-2013 Free Software Foundation, Inc.
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

#include "GNUstepBase/GSTypeEncoding.h"

#include <stdlib.h>
#include <string.h>

#undef  MAX
#define MAX(X, Y)                     \
({                                    \
  typeof (X) __x = (X), __y = (Y);    \
  (__x > __y ? __x : __y);            \
})

/*
 * We store here aligned sizes of primitive types
 * and bit-masks of type qualifiers
 */
static const int8_t typeInfoTable[] =
{
  /* types */
  [GSObjCTypeId]                  = sizeof(id),
  [GSObjCTypeClass]               = sizeof(Class),
  [GSObjCTypeSelector]            = sizeof(SEL),
  [GSObjCTypeChar]                = sizeof(char),
  [GSObjCTypeUnsignedChar]        = sizeof(unsigned char),
  [GSObjCTypeShort]               = sizeof(short),
  [GSObjCTypeUnsignedShort]       = sizeof(unsigned short),
  [GSObjCTypeInt]                 = sizeof(int),
  [GSObjCTypeUnsignedInt]         = sizeof(unsigned int),
  [GSObjCTypeLong]                = sizeof(long),
  [GSObjCTypeUnsignedLong]        = sizeof(unsigned long),
  [GSObjCTypeLongLong]            = sizeof(long long),
  [GSObjCTypeUnsignedLongLong]    = sizeof(unsigned long long),
  [GSObjCTypeFloat]               = sizeof(float),
  [GSObjCTypeDouble]              = sizeof(double),
  [GSObjCTypeBool]                = sizeof(_Bool),
  [GSObjCTypeVoid]                = sizeof(void),
  /* here would go Pointer, but in most cases it needs special treatment */
  [GSObjCTypeCharPointer]         = sizeof(char *),
  [GSObjCTypeAtom]                = sizeof(void *),
  /* type qualifiers (negated for distinctiveness) */
  [GSObjCQualifierConst]          = -GSObjCQualifierConstMask,
  [GSObjCQualifierIn]             = -GSObjCQualifierInMask,
  [GSObjCQualifierInOut]          = -GSObjCQualifierInOutMask,
  [GSObjCQualifierOut]            = -GSObjCQualifierOutMask,
  [GSObjCQualifierByCopy]         = -GSObjCQualifierByCopyMask,
  [GSObjCQualifierByRef]          = -GSObjCQualifierByRefMask,
  [GSObjCQualifierOneWay]         = -GSObjCQualifierOneWayMask,
  [GSObjCQualifierInvisible]      = -GSObjCQualifierInvisible,
  /* ensure an appropriate table size */
  [GSObjCTypeMax]                 = 0
};

/* all substripts of typeInfoTable are of char type */
#ifdef  __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#endif

GS_STATIC_INLINE uint8_t
RoundToThePowerOfTwo (uint8_t value)
{
  --value;
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  return ++value;
}

GS_STATIC_INLINE const char *
GetNumericValue (const char *cursor, int *value)
{
  *value = 0;
  while (*cursor >= '0' && *cursor <= '9')
    {
      *value = 10 * (*value) + (*cursor++ - '0');
    }
  return cursor;
}

GS_STATIC_INLINE const char *
SkipName (const char *cursor)
{
  if (*cursor == '"')
    {
      for (++cursor; *cursor++ != '"';);
    }
  return cursor;
}

GS_STATIC_INLINE const char *
SkipType (const char *cursor)
{
  unsigned depth = 0;
  do {
    while (typeInfoTable[(int)*cursor] < 0)
      {
        ++cursor;
      }
    
    if (!typeInfoTable[(int)*cursor])
      {
        switch (*cursor)
          {
            case GSObjCTypeArrayBegin:
            case GSObjCTypeStructureBegin:
            case GSObjCTypeUnionBegin:
              ++depth;
              break;
            case GSObjCTypeArrayEnd:
            case GSObjCTypeStructureEnd:
            case GSObjCTypeUnionEnd:
              --depth;
              break;
            case GSObjCTypePointer:
              ++cursor;
            default:
              break;
          }
      }
    cursor = SkipName(++cursor);
  } while (depth);
  return cursor;
}

GS_STATIC_INLINE const char *
GetQualifiers (const char *cursor, uint8_t *qualifiers)
{
  *qualifiers = 0;
  while (typeInfoTable[(int)*cursor] < 0)
    {
      *qualifiers |= (uint8_t) -typeInfoTable[(int)*cursor];
      ++cursor;
    }
  return cursor;
}

typedef struct ParserStackElement
{
  const char *cursor;
  size_t      size;
  size_t      count; /* for arrays */
  char        alignment;
  char        qualifiers;
} ParserStackElement;


typedef struct ParserOutput
{
  GSObjCTypeInfo      info;
  unsigned            parentDepth;
  BOOL                suppressed;
} ParserOutput;

typedef struct ParserState
{
  ParserStackElement  *stack;
  ParserOutput        *buffer;
  unsigned            stackSize;
  unsigned            bufferSize;
  unsigned            allocated;
  unsigned            stackSpace;
  unsigned            bufferSpace;
} ParserState;

/**
 * Complex type nesting level of 4 or greater is a rare case.
 * With initial size of 3 total memory footprint of stack and
 * buffer is 224 bytes (on machines with 64-bit word).
 *
 * Nesting depth of 16 will require circa 1K of memory, 64 - approximately 4K.
 * Maybe it would be better to place an upper bound on nesting depth and simply
 * allocate space on stack once. This will certainly be a performance again.
 */
static const unsigned ParserInitialStackSize = 3;

GS_STATIC_INLINE ParserStackElement *
ParserStackTop (ParserState *state)
{
  return state->stackSize ? &state->stack[state->stackSize - 1] : NULL;
}

const char *
GSObjCParseTypeSpecification (const char *cursor,
                              GSObjCTypeParserDelegate delegate,
                              void *context,
                              unsigned options)
{
  ParserState state;
  unsigned suppressionDepth = 0;
  unsigned bitFieldSpaceAvailable = 0;
  ParserStackElement el;
  unsigned index;

  state.stackSize = state.bufferSize = 0;
  state.stackSpace = sizeof(ParserStackElement) * ParserInitialStackSize;
  state.bufferSpace = sizeof(ParserOutput) * (ParserInitialStackSize + 1);
  state.stack = malloc(state.stackSpace + state.bufferSpace);
  state.buffer = (void *)state.stack + state.stackSpace;
  state.allocated = ParserInitialStackSize;
  
  do {
    GSObjCTypeInfo info = {cursor, 0, 1, 0};
    BOOL pushStack = NO;
    BOOL popStack = NO;
    BOOL pushBuffer = YES;
    BOOL suppress = suppressionDepth != 0;
    unsigned parentDepth;

    cursor = GetQualifiers(cursor, &info.qualifiers);
    cursor = SkipName(cursor);
    parentDepth = state.stackSize;
    
    /* is it a primitive type? */
    if (typeInfoTable[(int)*cursor])
      {
        info.size = info.alignment = typeInfoTable[(int)*cursor];
        cursor = SkipName(++cursor);
      }
    else
      {
        switch (*cursor) {
          case GSObjCTypeBitField:
            {
              int totalBitCount = -bitFieldSpaceAvailable;
              int bitCount = 0;

              info.alignment = 1;
              while (totalBitCount < 8 && *cursor == GSObjCTypeBitField)
              /* can we emit token */
                {
                  cursor = GetNumericValue(++cursor, &bitCount);
                  totalBitCount += bitCount;
                  /* round bitCount to the nearest power of 2 */
                  info.alignment
                    = MAX(info.alignment, RoundToThePowerOfTwo(bitCount) / 8);
                }
              info.size = totalBitCount / 8
                + ((totalBitCount & 7 /* mod 8 */) != 0);
              if (*cursor == GSObjCTypeBitField)
                {
                  bitFieldSpaceAvailable
                    = (unsigned)info.alignment * 8 - totalBitCount;
                }
              else
                bitFieldSpaceAvailable = 0;
              break;
            }
          case GSObjCTypePointer:
            {
              info.size = info.alignment = sizeof(void *);
              cursor = SkipType(++cursor);
              break;
            }
          case GSObjCTypeComplex:
            {
              info.size = 2 * (info.alignment = typeInfoTable[(int)*++cursor]);
              ++cursor;
              break;
            }
          case GSObjCTypeArrayBegin:
            {
              int length;

              cursor = GetNumericValue(++cursor, &length);
              el = (ParserStackElement){
                cursor, 0, length - 1, 1, info.qualifiers
              };
              pushStack = YES;
              suppressionDepth += (options & GSObjCReportArrayOnceMask) != 0;
              break;
            }
          case GSObjCTypeStructureBegin:
            {
              el = (ParserStackElement){cursor, 0, 0, 1, info.qualifiers};
              /* skip typename annotation */
              while (*cursor != GSObjCTypeStructureEnd && *cursor++ != '=');
              pushStack = YES;
              break;
            }
          case GSObjCTypeUnionBegin:
            {
              el = (ParserStackElement){cursor, 0, 0, 1, info.qualifiers};
              /* skip typename annotation */
              while (*cursor != GSObjCTypeUnionEnd && *cursor++ != '=');
              ++suppressionDepth;
              pushStack = YES;
              break;
            }
          case GSObjCTypeUnionEnd:
          case GSObjCTypeArrayEnd:
          case GSObjCTypeStructureEnd:
            {
              popStack = YES;
              break;
            }
            
          default:
            abort();
        }
      }
    
    if (pushStack)
      {
        if (state.stackSize == state.allocated)
          {
            unsigned stackSpace;
            unsigned bufferSpace;
            void *data;

            state.allocated *= 2;
            stackSpace = sizeof(ParserStackElement) * state.allocated;
            bufferSpace = sizeof(ParserOutput) * (state.allocated + 1);
            data = malloc(stackSpace + bufferSpace);
            memcpy(data, state.stack, state.stackSpace);
            memcpy(data + stackSpace, state.buffer, state.bufferSpace);
            free(state.stack);
            state.stack = data;
            state.buffer = (void *)data + stackSpace;
            state.stackSpace = stackSpace;
            state.bufferSpace = bufferSpace;
          }
        state.stack[state.stackSize] = el;
        ++state.stackSize;
      }
    else
      {
        /* we can safely flush the buffer */
        for (index = 0; index < state.bufferSize; ++index)
          {
            GSObjCTypeInfo output = state.buffer[index].info;
            unsigned depth = state.buffer[index].parentDepth;

            if (depth)
              {
                ParserStackElement *parent = &state.stack[depth - 1];

                if (*parent->cursor != GSObjCTypeUnionBegin)
                  {
                    size_t alignedSize;

                    /* in array and structure we should align data according
                     * to the element that triggered flushing (it may be the
                     * next real member of the data-structure or a closing
                     * tag)
                     */
                    output.alignment = MAX(output.alignment, info.alignment);
                    alignedSize = GSObjCPadSize(output.size, output.alignment);
                    parent->size += alignedSize;
                  }
                else
                  {
                    parent->size = MAX(parent->size, output.size);
                  }
                parent->alignment = MAX(parent->alignment, output.alignment);
              }
            if (!state.buffer[index].suppressed)
              {
                delegate(context, output);
              }
          }
        state.bufferSize = 0;
      }
    
    if (popStack)
      {
        ParserStackElement *element = ParserStackTop(&state);

        switch (*cursor)
          {
            case GSObjCTypeUnionEnd:
              --suppressionDepth;       /* FALLTHROUGH */
            case GSObjCTypeArrayEnd:
              {
                if (element->count)
                  {
                    if (options & GSObjCReportArrayOnceMask)
                      {
                        /* we need to compensate "length - 1" */
                        element->size *= (element->count + 1);
                        --suppressionDepth;
                      }
                    else
                      {
                        /* decrease length and rewind */
                        --element->count;
                        cursor = element->cursor;
                        pushBuffer = NO;
                        break;
                      }
                  }
              }         /* FALLTHROUGH */
            case GSObjCTypeStructureEnd:
              {
                info.qualifiers = element->qualifiers;
                info.size = element->size;
                info.alignment = element->alignment;
                parentDepth = --state.stackSize;
                ++cursor;
                break;
              }
          }
      }
    
    if (pushBuffer)
      {
        /* outermost GSObjCTypeUnionBegin and GSObjCTypeUnionEnd
         * should be reported
         */
        state.buffer[state.bufferSize].suppressed
          = suppress && suppressionDepth != 0;
        state.buffer[state.bufferSize].parentDepth  = parentDepth;
        state.buffer[state.bufferSize].info         = info;
        ++state.bufferSize;
      }
  } while (state.stackSize);
  
  for (index = 0; index < state.bufferSize; ++index)
    {
      if (!state.buffer[index].suppressed)
        {
          delegate(context, state.buffer[index].info);
        }
    }
  
  free(state.stack);
  
  return cursor;
}

#ifdef  __clang__
#pragma GCC diagnostic pop
#endif

typedef struct InfoAccumulator
{
  size_t  size;
  char    alignment;
} InfoAccumulator;

static void
InfoAccumulatorAddInfo (InfoAccumulator *this, GSObjCTypeInfo info)
{
  /* if it's the end of structure, accumulate only padding */
  if (*info.type == GSObjCTypeStructureEnd)
      this->size += GSObjCGetPadding (info.size, info.alignment);
  else
      this->size += GSObjCPadSize (info.size, info.alignment);
  this->alignment = MAX(this->alignment, info.alignment);
}

const char *
GSGetSizeAndAlignment (const char *type, size_t *sizep, uint8_t *alignp)
{
  InfoAccumulator accumulator = {0, 0};
  type = GSObjCParseTypeSpecification (type,
    (GSObjCTypeParserDelegate)&InfoAccumulatorAddInfo,
    &accumulator,
    GSObjCReportArrayOnceMask);
  if (sizep)
    {
      *sizep = accumulator.size;
    }
  if (alignp)
    {
      *alignp = accumulator.alignment;
    }
  return type;
}


#if defined (NeXT_RUNTIME)

/* emulate GNU API */

typedef struct SizeInfoAccumulator
{
  size_t      size;
  unsigned    depth;
} SizeInfoAccumulator;

static void
SizeInfoAccumulatorAddInfo (SizeInfoAccumulator *this, GSObjCTypeInfo info)
{
  /* we wait until typespec's last element and save it's unaligned size */
  switch (*info.type)
  {
    case GSObjCTypeArrayBegin:
    case GSObjCTypeStructureBegin:
    case GSObjCTypeUnionBegin:
      ++this->depth;
      break;
    case GSObjCTypeArrayEnd:
    case GSObjCTypeStructureEnd:
    case GSObjCTypeUnionEnd:
      --this->depth;
      break;
    default:
      break;
  }
  if (!this->depth)
    {
      this->size += info.size;
    }
}

int
objc_sizeof_type (const char* type)
{
  SizeInfoAccumulator accumulator = {0, 0};
  GSObjCParseTypeSpecification (type,
    (GSObjCTypeParserDelegate)&SizeInfoAccumulatorAddInfo,
    &accumulator,
    GSObjCReportArrayOnceMask);
  return (int)accumulator.size;
}

int
objc_alignof_type (const char* type)
{
  uint8_t alignment;
  GSGetSizeAndAlignment (type, NULL, &alignment);
  return (int)alignment;
}

int
objc_aligned_size (const char* type)
{
  size_t size;
  GSGetSizeAndAlignment (type, &size, NULL);
  return (int)size;
}

int
objc_promoted_size (const char* type)
{
  size_t size;
  GSGetSizeAndAlignment (type, &size, NULL);
  return (int)GSObjCPadSize (size, sizeof(void *));
}

/* we should not instantiate this function more than once */
static const char *
GetQualifiersInst (const char *cursor, uint8_t *qualifiers)
{
  return GetQualifiers (cursor, qualifiers);
}

/* we should not instantiate this function more than once */
static const char *
SkipTypeInst (const char *cursor)
{
  return SkipType(cursor);
}

unsigned
objc_get_type_qualifiers (const char* type)
{
  uint8_t qualifiers;
  GetQualifiersInst (type, &qualifiers);
  return qualifiers;
}

const char *
objc_skip_type_qualifiers (const char* type)
{
  uint8_t qualifiers;
  return GetQualifiersInst (type, &qualifiers);
}

const char *
objc_skip_typespec (const char* type)
{
  uint8_t qualifiers;
  type = GetQualifiersInst (type, &qualifiers);
  return SkipTypeInst (type);
}

const char *
objc_skip_offset (const char* type)
{
  if (*type == '+' || *type == '-')
    {
      type++;
    }
  while (*type >= '0' && *type <= '9')
    {
      type++;
    }
  return type;
}

const char *
objc_skip_argspec (const char* type)
{
  type = SkipTypeInst (type);
  return objc_skip_offset (type);
}

static void
objc_layout_structure_append_info(struct objc_struct_layout *this,
                                  GSObjCTypeInfo info)
{
  if (this->count == this->allocated)
    {
      this->info = realloc(this->info, sizeof(GSObjCTypeInfo) * (this->allocated *= 2));
    }
  this->info[this->count] = info;
  ++this->count;
}

static void
objc_layout_structure_parser_delegate(struct objc_struct_layout *this,
                                      GSObjCTypeInfo info)
{
  unsigned initialDepth = this->depth;
  switch (*info.type)
  {
    case GSObjCTypeArrayEnd:
    case GSObjCTypeStructureEnd:
    case GSObjCTypeUnionEnd:
    {
      if (--this->depth == 1)
        {
          this->info[this->count - 1].size = info.size;
          this->info[this->count - 1].alignment = info.alignment;
        }
      break;
    }
    case GSObjCTypeArrayBegin:
    case GSObjCTypeStructureBegin:
    case GSObjCTypeUnionBegin:
      ++this->depth;
    default:
      if (initialDepth == 1)
        {
          objc_layout_structure_append_info(this, info);
        }
      break;
  }
}

void
objc_layout_structure (const char *type,
                       struct objc_struct_layout *layout)
{
  *layout = (struct objc_struct_layout)
  {
    malloc(8 * sizeof(GSObjCTypeInfo)),
    -1, 0, 8, 0, 0, 0
  };
  GSObjCParseTypeSpecification(type,
                               (GSObjCTypeParserDelegate)&objc_layout_structure_parser_delegate,
                               layout,
                               GSObjCReportArrayOnceMask);
}

BOOL
objc_layout_structure_next_member (struct objc_struct_layout *layout)
{
  return ++layout->position < layout->count;
}

void
objc_layout_structure_get_info (struct objc_struct_layout *layout,
                                unsigned int *offset,
                                unsigned int *align,
                                const char **type)
{
  GSObjCTypeInfo info = layout->info[layout->position];
  
  if (offset)
    {
      *offset = layout->offset;
    }
  if (align)
    {
      *align = info.alignment;
    }
  if (type)
    {
      *type = info.type;
    }
  
  layout->offset += GSObjCPadSize(info.size, info.alignment);
  layout->alignment = MAX(layout->alignment, info.alignment);
}

void
objc_layout_finish_structure (struct objc_struct_layout *layout,
                              unsigned int *size,
                              unsigned int *align)
{
  if (size)
    {
      *size = (unsigned int) GSObjCPadSize(layout->offset, layout->alignment);
    }
  if (align)
    {
      *align = layout->alignment;
    }
  free(layout->info);
}

#endif /* NeXT_RUNTIME */
