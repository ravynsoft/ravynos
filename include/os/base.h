/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_BASE__
#define __OS_BASE__

#include <sys/cdefs.h>

#if __GNUC__
#define os_fastpath(x) ((long)(uintptr_t)__builtin_expect((uintptr_t)(x), ~0l))
#define os_slowpath(x) ((long)(uintptr_t)__builtin_expect((uintptr_t)(x), 0l))
#define os_constant(x) __builtin_constant_p((x))
#define os_hardware_trap() __asm__ __volatile__ (""); __builtin_trap()

#define __OS_COMPILETIME_ASSERT__(e) __extension__({ \
	char __compile_time_assert__[(e) ? 1 : -1];	\
	(void)__compile_time_assert__; \
})

#define __OS_CONST __attribute__((__const__))
#define __OS_PRINTFLIKE(x,y) __attribute__((__format__(printf,x,y)))
#else /* __GNUC__ */
#define os_fastpath(x) (x)
#define os_slowpath(x) (x)
#define os_constant(x) ((long)0)
#define os_hardware_trap() abort()

#define __OS_COMPILETIME_ASSERT__(e) (e)

#define __OS_CONST
#define __OS_PRINTFLIKE(x,y)
#endif /* __GNUC__ */


#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#ifndef __has_include
#define __has_include(x) 0
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __GNUC__
#define OS_NORETURN __attribute__((__noreturn__))
#define OS_NOTHROW __attribute__((__nothrow__))
#define OS_NONNULL1 __attribute__((__nonnull__(1)))
#define OS_NONNULL2 __attribute__((__nonnull__(2)))
#define OS_NONNULL3 __attribute__((__nonnull__(3)))
#define OS_NONNULL4 __attribute__((__nonnull__(4)))
#define OS_NONNULL5 __attribute__((__nonnull__(5)))
#define OS_NONNULL6 __attribute__((__nonnull__(6)))
#define OS_NONNULL7 __attribute__((__nonnull__(7)))
#define OS_NONNULL8 __attribute__((__nonnull__(8)))
#define OS_NONNULL9 __attribute__((__nonnull__(9)))
#define OS_NONNULL10 __attribute__((__nonnull__(10)))
#define OS_NONNULL11 __attribute__((__nonnull__(11)))
#define OS_NONNULL12 __attribute__((__nonnull__(12)))
#define OS_NONNULL13 __attribute__((__nonnull__(13)))
#define OS_NONNULL14 __attribute__((__nonnull__(14)))
#define OS_NONNULL15 __attribute__((__nonnull__(15)))
#define OS_NONNULL_ALL __attribute__((__nonnull__))
#define OS_SENTINEL __attribute__((__sentinel__))
#define OS_PURE __attribute__((__pure__))
#define OS_CONST __attribute__((__const__))
#define OS_WARN_RESULT __attribute__((__warn_unused_result__))
#define OS_MALLOC __attribute__((__malloc__))
#define OS_USED __attribute__((__used__))
#define OS_UNUSED __attribute__((__unused__))
#define OS_WEAK __attribute__((__weak__))
#define OS_WEAK_IMPORT __attribute__((__weak_import__))
#define OS_NOINLINE __attribute__((__noinline__))
#define OS_ALWAYS_INLINE __attribute__((__always_inline__))
#define OS_TRANSPARENT_UNION __attribute__((__transparent_union__))
#define OS_ALIGNED(n) __attribute__((__aligned__((n))))
#define OS_FORMAT_PRINTF(x,y) __attribute__((__format__(printf,x,y)))
#define OS_EXPORT extern __attribute__((visibility("default")))
#ifndef OS_INLINE
#define OS_INLINE static inline
#endif
#define OS_EXPECT(x, v) __builtin_expect((x), (v))
#else
#define OS_NORETURN
#define OS_NOTHROW
#define OS_NONNULL1
#define OS_NONNULL2
#define OS_NONNULL3
#define OS_NONNULL4
#define OS_NONNULL5
#define OS_NONNULL6
#define OS_NONNULL7
#define OS_NONNULL8
#define OS_NONNULL9
#define OS_NONNULL10
#define OS_NONNULL11
#define OS_NONNULL12
#define OS_NONNULL13
#define OS_NONNULL14
#define OS_NONNULL15
#define OS_NONNULL_ALL
#define OS_SENTINEL
#define OS_PURE
#define OS_CONST
#define OS_WARN_RESULT
#define OS_MALLOC
#define OS_USED
#define OS_UNUSED
#define OS_WEAK
#define OS_WEAK_IMPORT
#define OS_NOINLINE
#define OS_ALWAYS_INLINE
#define OS_TRANSPARENT_UNION
#define OS_ALIGNED(n)
#define OS_FORMAT_PRINTF(x,y)
#define OS_EXPORT extern
#ifndef OS_INLINE
#define OS_INLINE static inline
#endif
#define OS_EXPECT(x, v) (x)
#endif

#if __has_extension(attribute_overloadable)
#define OS_OVERLOADABLE __attribute__((__overloadable__))
#else
#define OS_OVERLOADABLE
#endif

#if __has_feature(objc_fixed_enum) || __has_extension(cxx_strong_enums)
#define OS_ENUM(_name, _type, ...) \
		typedef enum : _type { __VA_ARGS__ } _name##_t
#else
#define OS_ENUM(_name, _type, ...) \
		enum { __VA_ARGS__ }; typedef _type _name##_t
#endif

#define __OS_STRINGIFY(s) #s
#define OS_STRINGIFY(s) __OS_STRINGIFY(s)
#define __OS_CONCAT(x, y) x ## y
#define OS_CONCAT(x, y) __OS_CONCAT(x, y)

#endif // __OS_BASE__
