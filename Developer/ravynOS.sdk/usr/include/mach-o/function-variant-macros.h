/*
 * Copyright (c) 2024 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _FUNCTION_VARIANT_MACROS_H_
#define _FUNCTION_VARIANT_MACROS_H_

#include <stddef.h>
#include <stdint.h>
#include <os/base.h>    // for OS_STRINGIFY

/*
 * These macros allow you to define function-variants without any compiler support.
 * A function-variant is when you have multiple implementation of the "same" function
 * each optimized for different runtime environments (such as processor kind or
 * security settings, etc).
 *
 * For instance, if you have three assembly implementation of strcpy() optimized for
 * different x86_64 processors, the variant table could be:
 *
 * FUNCTION_VARIANT_TABLE_EXPORTED(strcpy,
 *      { strcpy$Rosetta,  "rosetta" },
 *      { strcpy$Haswell,  "haswell" },
 *      { strcpy$Base,     "default" } );
 *
 * The first field in each line is the symbol name of a particular implementation function.
 * The convention is to name the variants starting with the generic name followed by something
 * identifying the variant. The second field is a condition string of when that matching
 * implementation function may be used.
 *
 * The order of rows is important. At runtime, each row will be evaluated in order (top to bottom).
 * The first row where the condition string is true will be used. Therefore, it is important to sort
 * the rows to match the way you want to prioritize implementations.  The last row must always
 * be "default" and its implementation must work in all environments.
 *
 * The condition strings may use "+" to specify multiple conditions that all must be true.
 * For example "foo+bar" means both "foo" and "bar" must evaluate to true for the implememtation
 * to be used.  There can be at most four conditions (three plus signs).
 *
 * There are four namespaces for conditions strings: arm64, x86_64, system-wide, per-process.
 * All condition strings in a table must be in the same namespace.
 *
 */


struct FunctionVariantTableEntry
{
    const void*                         func;
    __attribute__((aligned(8))) char    condition[56];  // alignment keeps layout same for 32-bit and 64-bit archs
};

#ifdef __cplusplus
static_assert(offsetof(FunctionVariantTableEntry, condition) == 8, "conditional field should be 8-bytes into struct");
static_assert(sizeof(FunctionVariantTableEntry) == 64, "struct should be 64-bytes for all arches");
#endif

/*
 * FUNCTION_VARIANT_TABLE_EXPORTED() for use in dylibs when the function-variant symbol will be exported.
 */
 #define FUNCTION_VARIANT_TABLE_EXPORTED(_name, ...) \
    extern const struct FunctionVariantTableEntry OS_CONCAT(fvtemp_, _name)[] __asm("_" OS_STRINGIFY(_name)); \
    __attribute__((section("__LD,__func_variants")))  \
    __attribute__((no_sanitize("address"))) \
    const struct FunctionVariantTableEntry OS_CONCAT(fvtemp_, _name)[] = { \
    __VA_ARGS__  \
};

/*
 * FUNCTION_VARIANT_TABLE() for use when the function-variant is for internal use (not exported).
 */
#define FUNCTION_VARIANT_TABLE(_name, ...) \
    extern const struct FunctionVariantTableEntry OS_CONCAT(fvtemp_, _name)[] __asm("_" OS_STRINGIFY(_name)); \
    __attribute__((visibility("hidden")))  \
    __attribute__((no_sanitize("address"))) \
    __attribute__((section("__LD,__func_variants")))  \
    const struct FunctionVariantTableEntry OS_CONCAT(fvtemp_, _name)[] = { \
    __VA_ARGS__  \
};



#endif // _FUNCTION_VARIANT_MACROS_H_

