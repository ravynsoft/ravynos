/*
 * Foundation.framework Objective-C SPI
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
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

#ifndef __clang__
#error Unsupported compiler!
#endif

#ifndef _FOUNDATION_OBJCRUNTIME_H
#define _FOUNDATION_OBJCRUNTIME_H

#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <objc/NSObjCRuntime.h>
#include <objc/runtime.h>
#include <objc/objc.h>
#include <objc_size_alignment.h>

#ifdef __cplusplus
#define FOUNDATION_EXPORT extern "C"
#else
#define FOUNDATION_EXPORT extern
#endif /* __cplusplus */

#define NS_INLINE static inline
#define NS_ROOT_CLASS __attribute__((objc_root_class))
#define NS_RETURNS_RETAINED __attribute__((ns_returns_retained))
#define NS_RETURNS_NOT_RETAINED __attribute__((ns_returns_not_retained))

#ifndef CF_RETURNS_RETAINED
#define CF_RETURNS_RETAINED __attribute__((cf_returns_retained))
#endif

#if __OBJC__
@class NSString;
const char *NSGetSizeAndAlignment(const char *type, NSUInteger *size, NSUInteger *alignment);
SEL NSSelectorFromString(NSString *selectorName);
Class NSClassFromString(NSString *className);
NSString *NSStringFromSelector(SEL selector);
NSString *NSStringFromClass(Class _class);
#endif /* __OBJC__ */
#endif /* _FOUNDATION_OBJCRUNTIME_H */