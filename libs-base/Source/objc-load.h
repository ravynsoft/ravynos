/* 
   objc-load.h - Dynamically load in Obj-C modules (Classes, Categories)
   
   Copyright (C) 1993, 2002 Free Software Foundation, Inc.

   Author: Adam Fedor
   Date: 1993
   
   This file is part of the GNUstep Objective-C Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   If you are interested in a warranty or support for this source code,
   contact Scott Christley <scottc@net-community.com> for more information.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/ 

#ifndef __objc_load_h_INCLUDE
#define __objc_load_h_INCLUDE

#include <stdio.h>
#include <Foundation/NSString.h>

#ifdef HAVE_DLADDR
#define LINKER_GETSYMBOL 1
#else
#define LINKER_GETSYMBOL 0
#endif

#endif /* __objc_load_h_INCLUDE */
