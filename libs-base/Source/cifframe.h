/* cifframe - Wrapper/Objective-C interface for ffi function interface

   Copyright (C) 1999, Free Software Foundation, Inc.
   
   Written by:  Adam Fedor <fedor@gnu.org>
   Created: Feb 2000
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02111 USA.
   */ 

#ifndef cifframe_h_INCLUDE
#define cifframe_h_INCLUDE

#include <ffi.h>

#if	defined(_WIN32)
/*
 * Avoid conflicts when other headers try to define UINT32 and UINT64
 */
#if	defined(UINT32)
#undef UINT32
#endif
#if	defined(UINT64)
#undef UINT64
#endif
#endif

#import "Foundation/NSMethodSignature.h"
#import "GNUstepBase/DistributedObjects.h"
#import "GSPrivate.h"

typedef struct _cifframe_t {
  ffi_cif cif;
  int nargs;
  ffi_type **arg_types;
  void **values;
} cifframe_t;

@class	NSMutableData;

extern NSMutableData *cifframe_from_signature (NSMethodSignature *info);

extern GSCodeBuffer* cifframe_closure (NSMethodSignature *sig, void (*func)());

extern void cifframe_set_arg(cifframe_t *cframe, int index, void *buffer, 
			     int size);
extern void cifframe_get_arg(cifframe_t *cframe, int index, void *buffer,
			     int size);
extern void *cifframe_arg_addr(cifframe_t *cframe, int index);
extern BOOL cifframe_decode_arg (const char *type, void* buffer);
extern BOOL cifframe_encode_arg (const char *type, void* buffer);

#endif
