/*
 * Copyright © 2022 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef FREEDRENO_COMMON_H_
#define FREEDRENO_COMMON_H_

#include "util/u_atomic.h"

#ifdef __cplusplus
template<typename E>
struct BitmaskEnum {
   E value;

   using underlying = typename std::underlying_type_t<E>;

#define FOREACH_TYPE(M, ...) \
   M(E,          ##__VA_ARGS__) \
   M(bool,       ##__VA_ARGS__) \
   M(uint8_t,    ##__VA_ARGS__) \
   M(int8_t,     ##__VA_ARGS__) \
   M(uint16_t,   ##__VA_ARGS__) \
   M(int16_t,    ##__VA_ARGS__) \
   M(uint32_t,   ##__VA_ARGS__) \
   M(int32_t,    ##__VA_ARGS__)

#define CONSTRUCTOR(T) BitmaskEnum(T value) :  value(static_cast<E>(value)) {}
   FOREACH_TYPE(CONSTRUCTOR)
#undef CONSTRUCTOR

#define CAST(T) inline operator T() const { return static_cast<T>(value); }
   FOREACH_TYPE(CAST)
#undef CAST

#define BOP(T, OP)                          \
   inline E operator OP(T rhs) const {      \
      return static_cast<E> (               \
         static_cast<underlying>(value) OP  \
         static_cast<underlying>(rhs)       \
      );                                    \
   }
   FOREACH_TYPE(BOP, |)
   FOREACH_TYPE(BOP, &)
#undef BOP

#define BOP(OP)                                                    \
   inline BitmaskEnum<E> operator OP(BitmaskEnum<E> rhs) const {   \
      return static_cast<E> (                                      \
         static_cast<underlying>(value) OP                         \
         static_cast<underlying>(rhs.value)                        \
      );                                                           \
   }
   BOP(|)
   BOP(&)
#undef BOP

#if defined(__GNUC__) && !defined(__clang)
/*
 * Silence:
 *
 *   ../src/freedreno/common/freedreno_common.h: In instantiation of 'E& BitmaskEnum<E>::operator|=(BitmaskEnum<E>::underlying) [with E = fd_dirty_3d_state; BitmaskEnum<E>::underlying = unsigned int]':
 *   ../src/gallium/drivers/freedreno/freedreno_context.h:620:16:   required from here
 *   ../src/freedreno/common/freedreno_common.h:68:39: error: dereferencing type-punned pointer will break strict-aliasing rules [-Werror=strict-aliasing]
 *      68 |         reinterpret_cast<underlying&>(value) OP static_cast<underlying>(rhs) ); \
 *         |                                       ^~~~~
 *
 * I cannot reproduce on gcc 12.2.1 or with clang 14.0.5 so I'm going to assume
 * this is a bug with gcc 10.x
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#define UOP(T, OP)                          \
   inline E& operator OP(T rhs) {           \
      return reinterpret_cast<E&>(          \
        reinterpret_cast<underlying&>(value) OP static_cast<underlying>(rhs) ); \
   }
   UOP(underlying, |=)
   UOP(underlying, &=)
#undef UOP

#if defined(__GNUC__) && !defined(__clang) && (__GNUC__ < 7)
#pragma GCC diagnostic pop
#endif

   inline E operator ~() const {
      static_assert(sizeof(E) == sizeof(BitmaskEnum<E>));
      return static_cast<E> (
            ~static_cast<underlying>(value)
      );
   }
#undef FOREACH_TYPE
};
#define BITMASK_ENUM(E) BitmaskEnum<E>
#else
#define BITMASK_ENUM(E) enum E
#endif

#ifdef __cplusplus
#  define EXTERNC extern "C"
#  define BEGINC EXTERNC {
#  define ENDC }
#else
#  define EXTERNC
#  define BEGINC
#  define ENDC
#endif

/*
 * SWAP - swap value of @a and @b
 */
#define SWAP(a, b)                                                             \
   do {                                                                        \
      __typeof(a) __tmp = (a);                                                 \
      (a) = (b);                                                               \
      (b) = __tmp;                                                             \
   } while (0)

/* for conditionally setting boolean flag(s): */
#define COND(bool, val) ((bool) ? (val) : 0)

#define BIT(bit) BITFIELD64_BIT(bit)

/**
 * Helper for allocating sequence #s where zero is a non-valid seqno
 */
typedef struct {
   uint32_t counter;
} seqno_t;

static inline uint32_t
seqno_next(seqno_t *seq)
{
   uint32_t n;
   do {
      n = p_atomic_inc_return(&seq->counter);
   } while (n == 0);
   return n;
}

static inline uint16_t
seqno_next_u16(seqno_t *seq)
{
   uint16_t n;
   do {
      n = p_atomic_inc_return(&seq->counter);
   } while (n == 0);
   return n;
}

#endif /* FREEDRENO_COMMON_H_ */
