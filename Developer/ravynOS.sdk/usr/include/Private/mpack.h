/**
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Nicholas Fraser
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

/*
 * This is the MPack 0.5.1 amalgamation package.
 *
 * http://github.com/ludocode/mpack
 */

#ifndef MPACK_H
#define MPACK_H 1

#define MPACK_AMALGAMATED 1

#include "mpack-config.h"


/* mpack-platform.h */

/**
 * @file
 *
 * Abstracts all platform-specific code from MPack. This contains
 * implementations of standard C functions when libc is not available,
 * as well as wrappers to library functions.
 */

#ifndef MPACK_PLATFORM_H
#define MPACK_PLATFORM_H 1

#if defined(WIN32) && MPACK_INTERNAL
#define _CRT_SECURE_NO_WARNINGS 1
#endif

/* #include "mpack-config.h" */

/* For now, nothing in here should be seen by Doxygen. */
/** @cond */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS 1
#endif
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS 1
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>

#if MPACK_STDLIB
#include <string.h>
#include <stdlib.h>
#endif
#if MPACK_STDIO
#include <stdio.h>
#endif
#if MPACK_SETJMP
#include <setjmp.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif



#define MPACK_UNUSED(var) ((void)(var))

#if MPACK_AMALGAMATED
#define MPACK_INTERNAL_STATIC static
#else
#define MPACK_INTERNAL_STATIC
#endif

#define MPACK_STRINGIFY_IMPL(arg) #arg
#define MPACK_STRINGIFY(arg) MPACK_STRINGIFY_IMPL(arg)



/* Some compiler-specific keywords and builtins */
#if defined(__GNUC__) || defined(__clang__)
    #define MPACK_UNREACHABLE __builtin_unreachable()
    #define MPACK_NORETURN(fn) fn __attribute__((noreturn))
    #define MPACK_ALWAYS_INLINE __attribute__((always_inline)) static inline
#elif _MSC_VER
    #define MPACK_UNREACHABLE __assume(0)
    #define MPACK_NORETURN(fn) __declspec(noreturn) fn
    #define MPACK_ALWAYS_INLINE __forceinline static
#else
    #define MPACK_UNREACHABLE ((void)0)
    #define MPACK_NORETURN(fn) fn
    #define MPACK_ALWAYS_INLINE static inline
#endif




/*
 * Here we define mpack_assert() and mpack_break(). They both work like a normal
 * assertion function in debug mode, causing a trap or abort. However, on some platforms
 * you can safely resume execution from mpack_break(), whereas mpack_assert() is
 * always fatal.
 *
 * In release mode, mpack_assert() is converted to an assurance to the compiler
 * that the expression cannot be false (via e.g. __assume() or __builtin_unreachable())
 * to improve optimization where supported. There is thus no point in "safely" handling
 * the case of this being false. Writing mpack_assert(0) rarely makes sense;
 * the compiler will throw away any code after it. If at any time an mpack_assert()
 * is not true, the behaviour is undefined. This also means the expression is
 * evaluated even in release.
 *
 * mpack_break() on the other hand is compiled to nothing in release. It is
 * used in situations where we want to highlight a programming error as early as
 * possible (in the debugger), but we still handle the situation safely if it
 * happens in release to avoid producing incorrect results (such as in
 * MPACK_WRITE_TRACKING.) It does not take an expression to test because it
 * belongs in a safe-handling block after its failing condition has been tested.
 *
 * If stdio is available, we can add a format string describing the error, and
 * on some compilers we can declare it noreturn to get correct results from static
 * analysis tools. Note that the format string and arguments are not evaluated unless
 * the assertion is hit.
 *
 * Note that any arguments to mpack_assert() beyond the first are only evaluated
 * if the expression is false (and are never evaluated in release.)
 *
 * mpack_assert_fail() and mpack_break_hit() are defined separately
 * because assert is noreturn and break isn't. This distinction is very
 * important for static analysis tools to give correct results.
 */

#if MPACK_DEBUG
    MPACK_NORETURN(void mpack_assert_fail(const char* message));
    #if MPACK_STDIO
        MPACK_NORETURN(void mpack_assert_fail_format(const char* format, ...));
        #define mpack_assert_fail_at(line, file, expr, ...) \
                mpack_assert_fail_format("mpack assertion failed at " file ":" #line "\n" expr "\n" __VA_ARGS__)
    #else
        #define mpack_assert_fail_at(line, file, ...) \
                mpack_assert_fail("mpack assertion failed at " file ":" #line )
    #endif

    #define mpack_assert_fail_pos(line, file, expr, ...) mpack_assert_fail_at(line, file, expr, __VA_ARGS__)
    #define mpack_assert(expr, ...) ((!(expr)) ? mpack_assert_fail_pos(__LINE__, __FILE__, #expr, __VA_ARGS__) : (void)0)

    void mpack_break_hit(const char* message);
    #if MPACK_STDIO
        void mpack_break_hit_format(const char* format, ...);
        #define mpack_break_hit_at(line, file, ...) \
                mpack_break_hit_format("mpack breakpoint hit at " file ":" #line "\n" __VA_ARGS__)
    #else
        #define mpack_break_hit_at(line, file, ...) \
                mpack_break_hit("mpack breakpoint hit at " file ":" #line )
    #endif
    #define mpack_break_hit_pos(line, file, ...) mpack_break_hit_at(line, file, __VA_ARGS__)
    #define mpack_break(...) mpack_break_hit_pos(__LINE__, __FILE__, __VA_ARGS__)
#else
    #define mpack_assert(expr, ...) ((!(expr)) ? MPACK_UNREACHABLE, (void)0 : (void)0)
    #define mpack_break(...) ((void)0)
#endif



#if MPACK_STDLIB
#define mpack_memset memset
#define mpack_memcpy memcpy
#define mpack_memmove memmove
#define mpack_memcmp memcmp
#define mpack_strlen strlen
#else
void* mpack_memset(void *s, int c, size_t n);
void* mpack_memcpy(void *s1, const void *s2, size_t n);
void* mpack_memmove(void *s1, const void *s2, size_t n);
int mpack_memcmp(const void* s1, const void* s2, size_t n);
size_t mpack_strlen(const char *s);
#endif



/* Debug logging */
#if 0
#define mpack_log(...) printf(__VA_ARGS__);
#else
#define mpack_log(...) ((void)0)
#endif



/* Make sure our configuration makes sense */
#if defined(MPACK_MALLOC) && !defined(MPACK_FREE)
    #error "MPACK_MALLOC requires MPACK_FREE."
#endif
#if !defined(MPACK_MALLOC) && defined(MPACK_FREE)
    #error "MPACK_FREE requires MPACK_MALLOC."
#endif
#if MPACK_READ_TRACKING && (!defined(MPACK_READER) || !MPACK_READER)
    #error "MPACK_READ_TRACKING requires MPACK_READER."
#endif
#if MPACK_WRITE_TRACKING && (!defined(MPACK_WRITER) || !MPACK_WRITER)
    #error "MPACK_WRITE_TRACKING requires MPACK_WRITER."
#endif
#ifndef MPACK_MALLOC
    #if MPACK_STDIO
    #error "MPACK_STDIO requires preprocessor definitions for MPACK_MALLOC and MPACK_FREE."
    #endif
    #if MPACK_READ_TRACKING
    #error "MPACK_READ_TRACKING requires preprocessor definitions for MPACK_MALLOC and MPACK_FREE."
    #endif
    #if MPACK_WRITE_TRACKING
    #error "MPACK_WRITE_TRACKING requires preprocessor definitions for MPACK_MALLOC and MPACK_FREE."
    #endif
#endif



/* Implement realloc if unavailable */
#ifdef MPACK_MALLOC
    #ifdef MPACK_REALLOC
    static inline void* mpack_realloc(void* old_ptr, size_t used_size, size_t new_size) {
        MPACK_UNUSED(used_size);
        return MPACK_REALLOC(old_ptr, new_size);
    }
    #else
    void* mpack_realloc(void* old_ptr, size_t used_size, size_t new_size);
    #endif
#endif



/**
 * @}
 */

#ifdef __cplusplus
}
#endif

/** @endcond */

#endif


/* mpack-common.h */

/**
 * @file
 *
 * Defines types and functions shared by the MPack reader and writer.
 */

#ifndef MPACK_COMMON_H
#define MPACK_COMMON_H 1

/* #include "mpack-platform.h" */

/** @cond */
#ifndef MPACK_STACK_SIZE
#define MPACK_STACK_SIZE 4096
#endif
/** @endcond */



/* Version information */

#define MPACK_VERSION_MAJOR 0  /**< The major version number of MPack. */
#define MPACK_VERSION_MINOR 5  /**< The minor version number of MPack. */
#define MPACK_VERSION_PATCH 1  /**< The patch version number of MPack. */

/** A number containing the version number of MPack for comparison purposes. */
#define MPACK_VERSION ((MPACK_VERSION_MAJOR * 10000) + \
        (MPACK_VERSION_MINOR * 100) + MPACK_VERSION_PATCH)

/** A macro to test for a minimum version of MPack. */
#define MPACK_VERSION_AT_LEAST(major, minor, patch) \
        (MPACK_VERSION >= (((major) * 10000) + ((minor) * 100) + (patch)))

/** @cond */
#if (MPACK_VERSION_PATCH > 0)
#define MPACK_VERSION_STRING_BASE \
        MPACK_STRINGIFY(MPACK_VERSION_MAJOR) "." \
        MPACK_STRINGIFY(MPACK_VERSION_MINOR) "." \
        MPACK_STRINGIFY(MPACK_VERSION_PATCH)
#else
#define MPACK_VERSION_STRING_BASE \
        MPACK_STRINGIFY(MPACK_VERSION_MAJOR) "." \
        MPACK_STRINGIFY(MPACK_VERSION_MINOR)
#endif
/** @endcond */

/**
 * @def MPACK_VERSION_STRING
 * @hideinitializer
 *
 * A string containing the MPack version.
 */
#if MPACK_AMALGAMATED
#define MPACK_VERSION_STRING MPACK_VERSION_STRING_BASE
#else
#define MPACK_VERSION_STRING MPACK_VERSION_STRING_BASE "dev"
#endif

/**
 * @def MPACK_LIBRARY_STRING
 * @hideinitializer
 *
 * A string describing MPack, containing the library name, version and debug mode.
 */
#if MPACK_DEBUG
#define MPACK_LIBRARY_STRING "MPack " MPACK_VERSION_STRING "-debug"
#else
#define MPACK_LIBRARY_STRING "MPack " MPACK_VERSION_STRING
#endif



#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup common Common Elements
 *
 * Contains types and functions shared by both the encoding and decoding
 * portions of MPack.
 *
 * @{
 */

/**
 * Error states for MPack objects.
 *
 * When a reader, writer, or tree is in an error state, all subsequent calls
 * are ignored and their return values are nil/zero. You should check whether
 * the source is in an error state before using such values.
 */
typedef enum mpack_error_t {
    mpack_ok = 0,        /**< No error. */
    mpack_error_io = 2,  /**< The reader or writer failed to fill or flush, or some other file or socket error occurred. */
    mpack_error_invalid, /**< The data read is not valid MessagePack. */
    mpack_error_type,    /**< The type or value range did not match what was expected by the caller. */
    mpack_error_too_big, /**< A read or write was bigger than the maximum size allowed for that operation. */
    mpack_error_memory,  /**< An allocation failure occurred. */
    mpack_error_bug,     /**< The MPack API was used incorrectly. (This will always assert in debug mode.) */
    mpack_error_data,    /**< The contained data is not valid. */
} mpack_error_t;

/**
 * Converts an mpack error to a string. This function returns an empty
 * string when MPACK_DEBUG is not set.
 */
const char* mpack_error_to_string(mpack_error_t error);

/**
 * Defines the type of a MessagePack tag.
 */
typedef enum mpack_type_t {
    mpack_type_nil = 1, /**< A null value. */
    mpack_type_bool,    /**< A boolean (true or false.) */
    mpack_type_float,   /**< A 32-bit IEEE 754 floating point number. */
    mpack_type_double,  /**< A 64-bit IEEE 754 floating point number. */
    mpack_type_int,     /**< A 64-bit signed integer. */
    mpack_type_uint,    /**< A 64-bit unsigned integer. */
    mpack_type_str,     /**< A string. */
    mpack_type_bin,     /**< A chunk of binary data. */
    mpack_type_ext,     /**< A typed MessagePack extension object containing a chunk of binary data. */
    mpack_type_array,   /**< An array of MessagePack objects. */
    mpack_type_map,     /**< An ordered map of key/value pairs of MessagePack objects. */
} mpack_type_t;

/**
 * Converts an mpack type to a string. This function returns an empty
 * string when MPACK_DEBUG is not set.
 */
const char* mpack_type_to_string(mpack_type_t type);

/**
 * An MPack tag is a MessagePack object header. It is a variant type representing
 * any kind of object, and includes the value of that object when it is not a
 * compound type (i.e. boolean, integer, float.)
 *
 * If the type is compound (str, bin, ext, array or map), the embedded data is
 * stored separately.
 */
typedef struct mpack_tag_t {
    mpack_type_t type; /**< The type of value. */

    int8_t exttype; /**< The extension type if the type is @ref mpack_type_ext. */

    /** The value for non-compound types. */
    union
    {
        bool     b; /**< The value if the type is bool. */
        float    f; /**< The value if the type is float. */
        double   d; /**< The value if the type is double. */
        int64_t  i; /**< The value if the type is signed int. */
        uint64_t u; /**< The value if the type is unsigned int. */
        uint32_t l; /**< The number of bytes if the type is str, bin or ext. */

        /** The element count if the type is an array, or the number of
            key/value pairs if the type is map. */
        uint32_t n;
    } v;
} mpack_tag_t;

/** Generates a nil tag. */
static inline mpack_tag_t mpack_tag_nil(void) {
    mpack_tag_t ret;
    mpack_memset(&ret, 0, sizeof(ret));
    ret.type = mpack_type_nil;
    return ret;
}

/** Generates a signed int tag. */
static inline mpack_tag_t mpack_tag_int(int64_t value) {
    mpack_tag_t ret;
    mpack_memset(&ret, 0, sizeof(ret));
    ret.type = mpack_type_int;
    ret.v.i = value;
    return ret;
}

/** Generates an unsigned int tag. */
static inline mpack_tag_t mpack_tag_uint(uint64_t value) {
    mpack_tag_t ret;
    mpack_memset(&ret, 0, sizeof(ret));
    ret.type = mpack_type_uint;
    ret.v.u = value;
    return ret;
}

/** Generates a bool tag. */
static inline mpack_tag_t mpack_tag_bool(bool value) {
    mpack_tag_t ret;
    mpack_memset(&ret, 0, sizeof(ret));
    ret.type = mpack_type_bool;
    ret.v.b = value;
    return ret;
}

/** Generates a float tag. */
static inline mpack_tag_t mpack_tag_float(float value) {
    mpack_tag_t ret;
    mpack_memset(&ret, 0, sizeof(ret));
    ret.type = mpack_type_float;
    ret.v.f = value;
    return ret;
}

/** Generates a double tag. */
static inline mpack_tag_t mpack_tag_double(double value) {
    mpack_tag_t ret;
    mpack_memset(&ret, 0, sizeof(ret));
    ret.type = mpack_type_double;
    ret.v.d = value;
    return ret;
}

/**
 * Compares two tags with an arbitrary fixed ordering. Returns 0 if the tags are
 * equal, a negative integer if left comes before right, or a positive integer
 * otherwise.
 *
 * See mpack_tag_equal() for information on when tags are considered
 * to be equal.
 *
 * The ordering is not guaranteed to be preserved across mpack versions; do not
 * rely on it in serialized data.
 */
int mpack_tag_cmp(mpack_tag_t left, mpack_tag_t right);

/**
 * Compares two tags for equality. Tags are considered equal if the types are compatible
 * and the values (for non-compound types) are equal.
 *
 * The field width of variable-width fields is ignored (and in fact is not stored
 * in a tag), and positive numbers in signed integers are considered equal to their
 * unsigned counterparts. So for example the value 1 stored as a positive fixint
 * is equal to the value 1 stored in a 64-bit unsigned integer field.
 *
 * The "extension type" of an extension object is considered part of the value
 * and much match exactly.
 *
 * Floating point numbers are compared bit-for-bit, not using the language's operator==.
 */
static inline bool mpack_tag_equal(mpack_tag_t left, mpack_tag_t right) {
    return mpack_tag_cmp(left, right) == 0;
}

/**
 * @}
 */



/* Helpers for fetching an arbitrarily sized int from a memory
 * location, regardless of endianness or alignment. */
/** @cond */

MPACK_ALWAYS_INLINE uint8_t mpack_load_native_u8(const char* p) {
    return (uint8_t)p[0];
}

MPACK_ALWAYS_INLINE uint16_t mpack_load_native_u16(const char* p) {
    return (uint16_t)((((uint16_t)(uint8_t)p[0]) << 8) |
           ((uint16_t)(uint8_t)p[1]));
}

MPACK_ALWAYS_INLINE uint32_t mpack_load_native_u32(const char* p) {
    return (((uint32_t)(uint8_t)p[0]) << 24) |
           (((uint32_t)(uint8_t)p[1]) << 16) |
           (((uint32_t)(uint8_t)p[2]) <<  8) |
            ((uint32_t)(uint8_t)p[3]);
}

MPACK_ALWAYS_INLINE uint64_t mpack_load_native_u64(const char* p) {
    return (((uint64_t)(uint8_t)p[0]) << 56) |
           (((uint64_t)(uint8_t)p[1]) << 48) |
           (((uint64_t)(uint8_t)p[2]) << 40) |
           (((uint64_t)(uint8_t)p[3]) << 32) |
           (((uint64_t)(uint8_t)p[4]) << 24) |
           (((uint64_t)(uint8_t)p[5]) << 16) |
           (((uint64_t)(uint8_t)p[6]) <<  8) |
            ((uint64_t)(uint8_t)p[7]);
}

/** @endcond */



#if MPACK_READ_TRACKING || MPACK_WRITE_TRACKING

/* Tracks the write state of compound elements (maps, arrays, */
/* strings, binary blobs and extension types) */
/** @cond */

typedef struct mpack_track_element_t {
    mpack_type_t type;
    uint64_t left; // we need 64-bit because (2 * INT32_MAX) elements can be stored in a map
} mpack_track_element_t;

typedef struct mpack_track_t {
    size_t count;
    size_t capacity;
    mpack_track_element_t* elements;
} mpack_track_t;

#if MPACK_INTERNAL
MPACK_INTERNAL_STATIC mpack_error_t mpack_track_init(mpack_track_t* track);
MPACK_INTERNAL_STATIC mpack_error_t mpack_track_grow(mpack_track_t* track);

// These look like some overly large inline functions, but really
// they are mostly asserts. They boil down to just a few checks
// and assignments.

static inline mpack_error_t mpack_track_push(mpack_track_t* track, mpack_type_t type, uint64_t count) {
    mpack_assert(track->elements, "null track elements!");

    // maps have twice the number of elements (key/value pairs)
    if (type == mpack_type_map)
        count *= 2;

    // grow if needed
    if (track->count == track->capacity) {
        mpack_error_t error = mpack_track_grow(track);
        if (error != mpack_ok)
            return error;
    }

    // insert new track
    track->elements[track->count].type = type;
    track->elements[track->count].left = count;
    ++track->count;
    return mpack_ok;
}

static inline mpack_error_t mpack_track_pop(mpack_track_t* track, mpack_type_t type) {
    mpack_assert(track->elements, "null track elements!");

    if (track->count == 0) {
        mpack_break("attempting to close a %s but nothing was opened!", mpack_type_to_string(type));
        return mpack_error_bug;
    }

    mpack_track_element_t* element = &track->elements[track->count - 1];

    if (element->type != type) {
        mpack_break("attempting to close a %s but the open element is a %s!",
                mpack_type_to_string(type), mpack_type_to_string(element->type));
        return mpack_error_bug;
    }

    if (element->left != 0) {
        mpack_break("attempting to close a %s but there are %" PRIu64 " %s left",
                mpack_type_to_string(type), element->left,
                (type == mpack_type_map || type == mpack_type_array) ? "elements" : "bytes");
        return mpack_error_bug;
    }

    --track->count;
    return mpack_ok;
}

static inline mpack_error_t mpack_track_element(mpack_track_t* track, bool read) {
    MPACK_UNUSED(read);
    mpack_assert(track->elements, "null track elements!");

    // if there are no open elements, that's fine, we can read elements at will
    if (track->count == 0)
        return mpack_ok;

    mpack_track_element_t* element = &track->elements[track->count - 1];

    if (element->type != mpack_type_map && element->type != mpack_type_array) {
        mpack_break("elements cannot be %s within an %s", read ? "read" : "written",
                mpack_type_to_string(element->type));
        return mpack_error_bug;
    }

    if (element->left == 0) {
        mpack_break("too many elements %s for %s", read ? "read" : "written",
                mpack_type_to_string(element->type));
        return mpack_error_bug;
    }

    --element->left;
    return mpack_ok;
}

static inline mpack_error_t mpack_track_bytes(mpack_track_t* track, bool read, uint64_t count) {
    MPACK_UNUSED(read);
    mpack_assert(track->elements, "null track elements!");

    if (track->count == 0) {
        mpack_break("bytes cannot be %s with no open bin, str or ext", read ? "read" : "written");
        return mpack_error_bug;
    }

    mpack_track_element_t* element = &track->elements[track->count - 1];

    if (element->type == mpack_type_map || element->type == mpack_type_array) {
        mpack_break("bytes cannot be %s within an %s", read ? "read" : "written",
                mpack_type_to_string(element->type));
        return mpack_error_bug;
    }

    if (element->left < count) {
        mpack_break("too many bytes %s for %s", read ? "read" : "written",
                mpack_type_to_string(element->type));
        return mpack_error_bug;
    }

    element->left -= count;
    return mpack_ok;
}

static inline mpack_error_t mpack_track_check_empty(mpack_track_t* track) {
    if (track->count != 0) {
        mpack_assert(0, "unclosed %s", mpack_type_to_string(track->elements[0].type));
        return mpack_error_bug;
    }
    return mpack_ok;
}

static inline mpack_error_t mpack_track_destroy(mpack_track_t* track, bool cancel) {
    mpack_error_t error = cancel ? mpack_ok : mpack_track_check_empty(track);
    MPACK_FREE(track->elements);
    track->elements = NULL;
    return error;
}

#endif
/** @endcond */
#endif



#if MPACK_INTERNAL

/* The below code is from Bjoern Hoehrmann's Flexible and Economical */
/* UTF-8 decoder, modified to make it static and add the mpack prefix. */

/* Copyright (c) 2008-2010 Bjoern Hoehrmann <bjoern@hoehrmann.de> */
/* See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details. */

#define MPACK_UTF8_ACCEPT 0
#define MPACK_UTF8_REJECT 12

static const uint8_t mpack_utf8d[] = {
  /* The first part of the table maps bytes to character classes that */
  /* to reduce the size of the transition table and create bitmasks. */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

  /* The second part is a transition table that maps a combination */
  /* of a state of the automaton and a character class to a state. */
   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12,
};

static inline
uint32_t mpack_utf8_decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = mpack_utf8d[byte];

  *codep = (*state != MPACK_UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = mpack_utf8d[256 + *state + type];
  return *state;
}

#endif



#ifdef __cplusplus
}
#endif

#endif


/* mpack-writer.h */

/**
 * @file
 *
 * Declares the MPack Writer.
 */

#ifndef MPACK_WRITER_H
#define MPACK_WRITER_H 1

/* #include "mpack-common.h" */

#if MPACK_WRITER

#ifdef __cplusplus
extern "C" {
#endif

#if MPACK_WRITE_TRACKING
struct mpack_track_t;
#endif

/**
 * @defgroup writer Write API
 *
 * The MPack Write API encodes structured data of a fixed (hardcoded) schema to MessagePack.
 *
 * @{
 */

/**
 * A buffered MessagePack encoder.
 *
 * The encoder wraps an existing buffer and, optionally, a flush function.
 * This allows efficiently encoding to an in-memory buffer or to a stream.
 *
 * All write operations are synchronous; they will block until the
 * data is fully written, or an error occurs.
 */
typedef struct mpack_writer_t mpack_writer_t;

/**
 * The mpack writer's flush function to flush the buffer to the output stream.
 * It should flag an appropriate error on the writer if flushing fails.
 * Keep in mind that flagging an error may longjmp.
 *
 * The specified context for callbacks is at writer->context.
 */
typedef void (*mpack_flush_t)(mpack_writer_t* writer, const char* buffer, size_t count);

/**
 * A teardown function to be called when the writer is destroyed.
 */
typedef void (*mpack_writer_teardown_t)(mpack_writer_t* writer);

struct mpack_writer_t {
    mpack_flush_t flush;              /* Function to write bytes to the output stream */
    mpack_writer_teardown_t teardown; /* Function to teardown the context on destroy */
    void* context;                    /* Context for writer callbacks */

    char* buffer;         /* Byte buffer */
    size_t size;          /* Size of the buffer */
    size_t used;          /* How many bytes have been written into the buffer */
    mpack_error_t error;  /* Error state */

    #if MPACK_SETJMP
    /* Optional jump target in case of error (pointer because it's
     * very large and may be unused) */
    jmp_buf* jump_env;
    #endif

    #if MPACK_WRITE_TRACKING
    mpack_track_t track; /* Stack of map/array/str/bin/ext writes */
    #endif
};

/**
 * @name Core Writer Functions
 * @{
 */

/**
 * Initializes an mpack writer with the given buffer. The writer
 * does not assume ownership of the buffer.
 *
 * Trying to write past the end of the buffer will result in mpack_error_io unless
 * a flush function is set with mpack_writer_set_flush(). To use the data without
 * flushing, call mpack_writer_buffer_used() to determine the number of bytes
 * written.
 *
 * @param writer The MPack writer.
 * @param buffer The buffer into which to write mpack data.
 * @param size The size of the buffer.
 */
void mpack_writer_init(mpack_writer_t* writer, char* buffer, size_t size);

#ifdef MPACK_MALLOC
/**
 * Initializes an mpack writer using a growable buffer.
 *
 * The data is placed in the given data pointer if and when the writer
 * is destroyed without error. The data should be freed with MPACK_FREE().
 * The data pointer is NULL during writing, and will remain NULL
 * if an error occurs.
 *
 * mpack_error_memory is raised if the buffer fails to grow.
 *
 * @param writer The MPack writer.
 * @param data Where to place the allocated data.
 * @param size Where to write the size of the data.
 */
void mpack_writer_init_growable(mpack_writer_t* writer, char** data, size_t* size);
#endif

/**
 * Initializes an mpack writer directly into an error state. Use this if you
 * are writing a wrapper to mpack_writer_init() which can fail its setup.
 */
void mpack_writer_init_error(mpack_writer_t* writer, mpack_error_t error);

#if MPACK_STDIO
/**
 * Initializes an mpack writer that writes to a file.
 */
void mpack_writer_init_file(mpack_writer_t* writer, const char* filename);
#endif

/**
 * @def mpack_writer_init_stack(writer, flush, context)
 * @hideinitializer
 *
 * Initializes an mpack writer using stack space.
 */

#define mpack_writer_init_stack_line_ex(line, writer) \
    char mpack_buf_##line[MPACK_STACK_SIZE]; \
    mpack_writer_init(writer, mpack_buf_##line, sizeof(mpack_buf_##line))

#define mpack_writer_init_stack_line(line, writer) \
    mpack_writer_init_stack_line_ex(line, writer)

#define mpack_writer_init_stack(writer) \
    mpack_writer_init_stack_line(__LINE__, (writer))

#if MPACK_SETJMP

/**
 * @hideinitializer
 *
 * Registers a jump target in case of error.
 *
 * If the writer is in an error state, 1 is returned when this is called. Otherwise
 * 0 is returned when this is called, and when the first error occurs, control flow
 * will jump to the point where this was called, resuming as though it returned 1.
 * This ensures an error handling block runs exactly once in case of error.
 *
 * A writer that jumps still needs to be destroyed. You must call
 * mpack_writer_destroy() in your jump handler after getting the final error state.
 *
 * The argument may be evaluated multiple times.
 *
 * @returns 0 if the writer is not in an error state; 1 if and when an error occurs.
 * @see mpack_writer_destroy()
 */
#define MPACK_WRITER_SETJMP(writer)                                        \
    (mpack_assert((writer)->jump_env == NULL, "already have a jump set!"), \
	((writer)->error != mpack_ok) ? 1 :                                    \
		!((writer)->jump_env = (jmp_buf*)MPACK_MALLOC(sizeof(jmp_buf))) ?  \
			((writer)->error = mpack_error_memory, 1) :                    \
			(setjmp(*(writer)->jump_env)))

/**
 * Clears a jump target. Subsequent write errors will not cause the writer to
 * jump.
 */
static inline void mpack_writer_clearjmp(mpack_writer_t* writer) {
    if (writer->jump_env)
        MPACK_FREE(writer->jump_env);
    writer->jump_env = NULL;
}
#endif

/**
 * Cleans up the mpack writer, flushing any buffered bytes to the
 * underlying stream, if any. Returns the final error state of the
 * writer in case an error occurred flushing. Causes an assert if
 * there are any unclosed compound types in tracking mode.
 *
 * Note that if a jump handler is set, a writer may jump during destroy if it
 * fails to flush any remaining data. In this case the writer will not be fully
 * destroyed; you can still get the error state, and you must call destroy as
 * usual in the jump handler.
 */
mpack_error_t mpack_writer_destroy(mpack_writer_t* writer);

/**
 * Cleans up the mpack writer, discarding any open writes and unflushed data.
 *
 * Use this to cancel writing in the middle of writing a document (for example
 * in case an error occurred.) This should be used instead of mpack_writer_destroy()
 * because the former will assert in tracking mode if there are any unclosed
 * compound types.
 */
void mpack_writer_destroy_cancel(mpack_writer_t* writer);

/**
 * Sets the custom pointer to pass to the writer callbacks, such as flush
 * or teardown.
 *
 * @param writer The MPack writer.
 * @param context User data to pass to the writer callbacks.
 */
static inline void mpack_writer_set_context(mpack_writer_t* writer, void* context) {
    writer->context = context;
}

/**
 * Sets the flush function to write out the data when the buffer is full.
 *
 * If no flush function is used, trying to write past the end of the
 * buffer will result in mpack_error_io.
 *
 * This should normally be used with mpack_writer_set_context() to register
 * a custom pointer to pass to the flush function.
 *
 * @param writer The MPack writer.
 * @param flush The function to write out data from the buffer.
 */
static inline void mpack_writer_set_flush(mpack_writer_t* writer, mpack_flush_t flush) {
    mpack_assert(writer->size != 0, "cannot use flush function without a writeable buffer!");
    writer->flush = flush;
}

/**
 * Sets the teardown function to call when the writer is destroyed.
 *
 * This should normally be used with mpack_writer_set_context() to register
 * a custom pointer to pass to the teardown function.
 *
 * @param writer The MPack writer.
 * @param teardown The function to call when the writer is destroyed.
 */
static inline void mpack_writer_set_teardown(mpack_writer_t* writer, mpack_writer_teardown_t teardown) {
    writer->teardown = teardown;
}

/**
 * Returns the number of bytes currently stored in the buffer. This
 * may be less than the total number of bytes written if bytes have
 * been flushed to an underlying stream.
 */
static inline size_t mpack_writer_buffer_used(mpack_writer_t* writer) {
    return writer->used;
}

/**
 * Places the writer in the given error state, jumping if a jump target is set.
 *
 * This allows you to externally flag errors, for example if you are validating
 * data as you read it.
 *
 * If the writer is already in an error state, this call is ignored and no jump
 * is performed.
 */
void mpack_writer_flag_error(mpack_writer_t* writer, mpack_error_t error);


/**
 * Queries the error state of the mpack writer.
 *
 * If a writer is in an error state, you should discard all data since the
 * last time the error flag was checked. The error flag cannot be cleared.
 */
static inline mpack_error_t mpack_writer_error(mpack_writer_t* writer) {
    return writer->error;
}

/**
 * Writes a MessagePack object header (an MPack Tag.)
 *
 * If the value is a map, array, string, binary or extension type, the
 * containing elements or bytes must be written separately and the
 * appropriate finish function must be called (as though one of the
 * mpack_start_*() functions was called.)
 */
void mpack_write_tag(mpack_writer_t* writer, mpack_tag_t tag);

/**
 * @}
 */

/**
 * @name Typed Write Functions
 * @{
 */

/*! Writes an 8-bit integer in the most efficient packing available. */
void mpack_write_i8(mpack_writer_t* writer, int8_t value);

/*! Writes a 16-bit integer in the most efficient packing available. */
void mpack_write_i16(mpack_writer_t* writer, int16_t value);

/*! Writes a 32-bit integer in the most efficient packing available. */
void mpack_write_i32(mpack_writer_t* writer, int32_t value);

/*! Writes a 64-bit integer in the most efficient packing available. */
void mpack_write_i64(mpack_writer_t* writer, int64_t value);

/*! Writes an integer in the most efficient packing available. */
static inline void mpack_write_int(mpack_writer_t* writer, int64_t value) {
    mpack_write_i64(writer, value);
}

/*! Writes an 8-bit unsigned integer in the most efficient packing available. */
void mpack_write_u8(mpack_writer_t* writer, uint8_t value);

/*! Writes an 16-bit unsigned integer in the most efficient packing available. */
void mpack_write_u16(mpack_writer_t* writer, uint16_t value);

/*! Writes an 32-bit unsigned integer in the most efficient packing available. */
void mpack_write_u32(mpack_writer_t* writer, uint32_t value);

/*! Writes an 64-bit unsigned integer in the most efficient packing available. */
void mpack_write_u64(mpack_writer_t* writer, uint64_t value);

/*! Writes an unsigned integer in the most efficient packing available. */
static inline void mpack_write_uint(mpack_writer_t* writer, uint64_t value) {
    mpack_write_u64(writer, value);
}

/*! Writes a float. */
void mpack_write_float(mpack_writer_t* writer, float value);

/*! Writes a double. */
void mpack_write_double(mpack_writer_t* writer, double value);

/*! Writes a boolean. */
void mpack_write_bool(mpack_writer_t* writer, bool value);

/*! Writes a boolean with value true. */
void mpack_write_true(mpack_writer_t* writer);

/*! Writes a boolean with value false. */
void mpack_write_false(mpack_writer_t* writer);

/*! Writes a nil. */
void mpack_write_nil(mpack_writer_t* writer);

/**
 * Writes a string.
 *
 * To stream a string in chunks, use mpack_start_str() instead.
 *
 * MPack does not care about the underlying encoding, but UTF-8 is highly
 * recommended, especially for compatibility with JSON.
 */
void mpack_write_str(mpack_writer_t* writer, const char* str, uint32_t length);

/**
 * Writes a binary blob.
 *
 * To stream a binary blob in chunks, use mpack_start_bin() instead.
 */
void mpack_write_bin(mpack_writer_t* writer, const char* data, uint32_t count);

/**
 * Writes an extension type.
 *
 * To stream an extension blob in chunks, use mpack_start_ext() instead.
 *
 * Extension types [0, 127] are available for application-specific types. Extension
 * types [-128, -1] are reserved for future extensions of MessagePack.
 */
void mpack_write_ext(mpack_writer_t* writer, int8_t exttype, const char* data, uint32_t count);

/**
 * Opens an array. count elements should follow, and mpack_finish_array()
 * should be called when done.
 */
void mpack_start_array(mpack_writer_t* writer, uint32_t count);

/**
 * Opens a map. count*2 elements should follow, and mpack_finish_map()
 * should be called when done.
 *
 * Remember that while map elements in MessagePack are implicitly ordered,
 * they are not ordered in JSON. If you need elements to be read back
 * in the order they are written, consider use an array instead.
 */
void mpack_start_map(mpack_writer_t* writer, uint32_t count);

/**
 * Opens a string. count bytes should be written with calls to 
 * mpack_write_bytes(), and mpack_finish_str() should be called
 * when done.
 *
 * To write an entire string at once, use mpack_write_str() or
 * mpack_write_cstr() instead.
 *
 * MPack does not care about the underlying encoding, but UTF-8 is highly
 * recommended, especially for compatibility with JSON.
 */
void mpack_start_str(mpack_writer_t* writer, uint32_t count);

/**
 * Opens a binary blob. count bytes should be written with calls to 
 * mpack_write_bytes(), and mpack_finish_bin() should be called
 * when done.
 */
void mpack_start_bin(mpack_writer_t* writer, uint32_t count);

/**
 * Opens an extension type. count bytes should be written with calls
 * to mpack_write_bytes(), and mpack_finish_ext() should be called
 * when done.
 *
 * Extension types [0, 127] are available for application-specific types. Extension
 * types [-128, -1] are reserved for future extensions of MessagePack.
 */
void mpack_start_ext(mpack_writer_t* writer, int8_t exttype, uint32_t count);

/**
 * Writes a portion of bytes for a string, binary blob or extension type which
 * was opened by one of the mpack_start_*() functions. The corresponding
 * mpack_finish_*() function should be called when done.
 *
 * To write an entire string, binary blob or extension type at
 * once, use one of the mpack_write_*() functions instead.
 *
 * @see mpack_start_str()
 * @see mpack_start_bin()
 * @see mpack_start_ext()
 * @see mpack_finish_str()
 * @see mpack_finish_bin()
 * @see mpack_finish_ext()
 * @see mpack_write_str()
 * @see mpack_write_bin()
 * @see mpack_write_ext()
 */
void mpack_write_bytes(mpack_writer_t* writer, const char* data, size_t count);

#if MPACK_WRITE_TRACKING
/**
 * Finishes writing an array.
 *
 * This will track writes to ensure that the correct number of elements are written.
 */
void mpack_finish_array(mpack_writer_t* writer);

/**
 * Finishes writing a map.
 *
 * This will track writes to ensure that the correct number of elements are written.
 */
void mpack_finish_map(mpack_writer_t* writer);

/**
 * Finishes writing a string.
 *
 * This will track writes to ensure that the correct number of bytes are written.
 */
void mpack_finish_str(mpack_writer_t* writer);

/**
 * Finishes writing a binary blob.
 *
 * This will track writes to ensure that the correct number of bytes are written.
 */
void mpack_finish_bin(mpack_writer_t* writer);

/**
 * Finishes writing an extended type binary data blob.
 *
 * This will track writes to ensure that the correct number of bytes are written.
 */
void mpack_finish_ext(mpack_writer_t* writer);

/**
 * Finishes writing the given compound type.
 *
 * This will track writes to ensure that the correct number of elements
 * or bytes are written.
 */
void mpack_finish_type(mpack_writer_t* writer, mpack_type_t type);
#else
static inline void mpack_finish_array(mpack_writer_t* writer) {MPACK_UNUSED(writer);}
static inline void mpack_finish_map(mpack_writer_t* writer) {MPACK_UNUSED(writer);}
static inline void mpack_finish_str(mpack_writer_t* writer) {MPACK_UNUSED(writer);}
static inline void mpack_finish_bin(mpack_writer_t* writer) {MPACK_UNUSED(writer);}
static inline void mpack_finish_ext(mpack_writer_t* writer) {MPACK_UNUSED(writer);}
static inline void mpack_finish_type(mpack_writer_t* writer, mpack_type_t type) {MPACK_UNUSED(writer); MPACK_UNUSED(type);}
#endif

/**
 * Writes a null-terminated string. (The null-terminator is not written.)
 *
 * MPack does not care about the underlying encoding, but UTF-8 is highly
 * recommended, especially for compatibility with JSON.
 */
void mpack_write_cstr(mpack_writer_t* writer, const char* str);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
#endif


/* mpack-reader.h */

/**
 * @file
 *
 * Declares the core MPack Tag Reader.
 */

#ifndef MPACK_READER_H
#define MPACK_READER_H 1

/* #include "mpack-common.h" */

#if MPACK_READER

#ifdef __cplusplus
extern "C" {
#endif

#if MPACK_READ_TRACKING
struct mpack_track_t;
#endif

/**
 * @defgroup reader Core Reader API
 *
 * The MPack Core Reader API contains functions for imperatively reading
 * dynamically typed data from a MessagePack stream. This forms the basis
 * of the Expect and Node APIs.
 *
 * @{
 */

/**
 * A buffered MessagePack decoder.
 *
 * The decoder wraps an existing buffer and, optionally, a fill function.
 * This allows efficiently decoding data from existing memory buffers, files,
 * streams, etc.
 *
 * All read operations are synchronous; they will block until the
 * requested data is fully read, or an error occurs.
 *
 * This structure is opaque; its fields should not be accessed outside
 * of MPack.
 */
typedef struct mpack_reader_t mpack_reader_t;

/**
 * The mpack reader's fill function. It should fill the buffer as
 * much as possible, returning the number of bytes put into the buffer.
 *
 * In case of error, it should flag an appropriate error on the reader.
 */
typedef size_t (*mpack_fill_t)(mpack_reader_t* reader, char* buffer, size_t count);

/**
 * A teardown function to be called when the reader is destroyed.
 */
typedef void (*mpack_reader_teardown_t)(mpack_reader_t* reader);

struct mpack_reader_t {
    mpack_fill_t fill;                /* Function to read bytes into the buffer */
    mpack_reader_teardown_t teardown; /* Function to teardown the context on destroy */
    void* context;                    /* Context for reader callbacks */

    char* buffer;       /* Byte buffer */
    size_t size;        /* Size of the buffer, or zero if it's const */
    size_t left;        /* How many bytes are left in the buffer */
    size_t pos;         /* Position within the buffer */
    mpack_error_t error;  /* Error state */

    #if MPACK_SETJMP
    /* Optional jump target in case of error (pointer because it's
     * very large and may be unused) */
    jmp_buf* jump_env;
    #endif

    #if MPACK_READ_TRACKING
    mpack_track_t track; /* Stack of map/array/str/bin/ext reads */
    #endif
};

#if MPACK_SETJMP

/**
 * @hideinitializer
 *
 * Registers a jump target in case of error.
 *
 * If the reader is in an error state, 1 is returned when this is called. Otherwise
 * 0 is returned when this is called, and when the first error occurs, control flow
 * will jump to the point where this was called, resuming as though it returned 1.
 * This ensures an error handling block runs exactly once in case of error.
 *
 * A reader that jumps still needs to be destroyed. You must call
 * mpack_reader_destroy() in your jump handler after getting the final error state.
 *
 * The argument may be evaluated multiple times.
 *
 * @returns 0 if the reader is not in an error state; 1 if and when an error occurs.
 * @see mpack_reader_destroy()
 */
#define MPACK_READER_SETJMP(reader)                                        \
    (mpack_assert((reader)->jump_env == NULL, "already have a jump set!"), \
    ((reader)->error != mpack_ok) ? 1 :                                    \
        !((reader)->jump_env = (jmp_buf*)MPACK_MALLOC(sizeof(jmp_buf))) ?  \
            ((reader)->error = mpack_error_memory, 1) :                    \
            (setjmp(*(reader)->jump_env)))

/**
 * Clears a jump target. Subsequent read errors will not cause the reader to
 * jump.
 */
static inline void mpack_reader_clearjmp(mpack_reader_t* reader) {
    if (reader->jump_env)
        MPACK_FREE(reader->jump_env);
    reader->jump_env = NULL;
}
#endif

/**
 * Initializes an mpack reader with the given buffer. The reader does
 * not assume ownership of the buffer, but the buffer must be writeable
 * if a fill function will be used to refill it.
 *
 * @param reader The MPack reader.
 * @param buffer The buffer with which to read mpack data.
 * @param size The size of the buffer.
 * @param count The number of bytes already in the buffer.
 */
void mpack_reader_init(mpack_reader_t* reader, char* buffer, size_t size, size_t count);

/**
 * Initializes an mpack reader directly into an error state. Use this if you
 * are writing a wrapper to mpack_reader_init() which can fail its setup.
 */
void mpack_reader_init_error(mpack_reader_t* reader, mpack_error_t error);

/**
 * Initializes an mpack reader to parse a pre-loaded contiguous chunk of data. The
 * reader does not assume ownership of the data.
 *
 * @param reader The MPack reader.
 * @param data The data to parse.
 * @param count The number of bytes pointed to by data.
 */
void mpack_reader_init_data(mpack_reader_t* reader, const char* data, size_t count);

#if MPACK_STDIO
/**
 * Initializes an mpack reader that reads from a file.
 */
void mpack_reader_init_file(mpack_reader_t* reader, const char* filename);
#endif

/**
 * @def mpack_reader_init_stack(reader)
 * @hideinitializer
 *
 * Initializes an mpack reader using stack space as a buffer. A fill function
 * should be added to the reader to fill the buffer.
 *
 * @see mpack_reader_set_fill
 */

/** @cond */
#define mpack_reader_init_stack_line_ex(line, reader) \
    char mpack_buf_##line[MPACK_STACK_SIZE]; \
    mpack_reader_init((reader), mpack_buf_##line, sizeof(mpack_buf_##line), 0)

#define mpack_reader_init_stack_line(line, reader) \
    mpack_reader_init_stack_line_ex(line, reader)
/** @endcond */

#define mpack_reader_init_stack(reader) \
    mpack_reader_init_stack_line(__LINE__, (reader))

/**
 * Cleans up the mpack reader, ensuring that all compound elements
 * have been completely read. Returns the final error state of the
 * reader.
 *
 * This will assert in tracking mode if the reader has any incomplete
 * reads. If you want to cancel reading in the middle of a compound
 * element and don't care about the rest of the document, call
 * mpack_reader_destroy_cancel() instead.
 *
 * @see mpack_reader_destroy_cancel()
 */
mpack_error_t mpack_reader_destroy(mpack_reader_t* reader);

/**
 * Cleans up the mpack reader, discarding any open reads.
 *
 * This should be used if you decide to cancel reading in the middle
 * of the document.
 */
void mpack_reader_destroy_cancel(mpack_reader_t* reader);

/**
 * Sets the custom pointer to pass to the reader callbacks, such as fill
 * or teardown.
 *
 * @param reader The MPack reader.
 * @param context User data to pass to the reader callbacks.
 */
static inline void mpack_reader_set_context(mpack_reader_t* reader, void* context) {
    reader->context = context;
}

/**
 * Sets the fill function to refill the data buffer when it runs out of data.
 *
 * If no fill function is used, trying to read past the end of the
 * buffer will result in mpack_error_io.
 *
 * This should normally be used with mpack_reader_set_context() to register
 * a custom pointer to pass to the fill function.
 *
 * @param reader The MPack reader.
 * @param fill The function to fetch additional data into the buffer.
 */
static inline void mpack_reader_set_fill(mpack_reader_t* reader, mpack_fill_t fill) {
    mpack_assert(reader->size != 0, "cannot use fill function without a writeable buffer!");
    reader->fill = fill;
}

/**
 * Sets the teardown function to call when the reader is destroyed.
 *
 * This should normally be used with mpack_reader_set_context() to register
 * a custom pointer to pass to the teardown function.
 *
 * @param reader The MPack reader.
 * @param teardown The function to call when the reader is destroyed.
 */
static inline void mpack_reader_set_teardown(mpack_reader_t* reader, mpack_reader_teardown_t teardown) {
    reader->teardown = teardown;
}

/**
 * Places the reader in the given error state, jumping if a jump target is set.
 *
 * This allows you to externally flag errors, for example if you are validating
 * data as you read it.
 *
 * If the reader is already in an error state, this call is ignored and no jump
 * is performed.
 */
void mpack_reader_flag_error(mpack_reader_t* reader, mpack_error_t error);

/**
 * Places the reader in the given error state if the given error is not mpack_ok.
 *
 * This allows you to externally flag errors, for example if you are validating
 * data as you read it.
 *
 * If the error is mpack_ok, or if the reader is already in an error state, this
 * call is ignored and no jump is performed.
 */
static inline void mpack_reader_flag_if_error(mpack_reader_t* reader, mpack_error_t error) {
    if (error != mpack_ok)
        mpack_reader_flag_error(reader, error);
}

/**
 * Returns bytes left in the reader's buffer.
 *
 * If you are done reading MessagePack data but there is other interesting data
 * following it, the reader may have buffered too much data. The number of bytes
 * remaining in the buffer and a pointer to the position of those bytes can be
 * queried here.
 *
 * If you know the length of the mpack chunk beforehand, it's better to instead
 * have your fill function limit the data it reads so that the reader does not
 * have extra data. In this case you can simply check that this returns zero.
 *
 * @param reader The MPack reader from which to query remaining data.
 * @param data [out] A pointer to the remaining data, or NULL.
 * @return The number of bytes remaining in the buffer.
 */
size_t mpack_reader_remaining(mpack_reader_t* reader, const char** data);

/**
 * Queries the error state of the MPack reader.
 *
 * If a reader is in an error state, you should discard all data since the
 * last time the error flag was checked. The error flag cannot be cleared.
 */
static inline mpack_error_t mpack_reader_error(mpack_reader_t* reader) {
    return reader->error;
}

/**
 * Reads a MessagePack object header (an MPack tag.)
 *
 * If an error occurs, the mpack_reader_t is placed in an error state, a
 * longjmp is performed (if set), and the return value is undefined.
 * If the reader is already in an error state, the return value
 * is undefined.
 *
 * If the type is compound (i.e. is a map, array, string, binary or
 * extension type), additional reads are required to get the actual data,
 * and the corresponding done function (or cancel) should be called when
 * done.
 *
 * Note that maps in JSON are unordered, so it is recommended not to expect
 * a specific ordering for your map values in case your data is converted
 * to/from JSON.
 * 
 * @see mpack_read_bytes()
 * @see mpack_done_array()
 * @see mpack_done_map()
 * @see mpack_done_str()
 * @see mpack_done_bin()
 * @see mpack_done_ext()
 * @see mpack_cancel()
 */
mpack_tag_t mpack_read_tag(mpack_reader_t* reader);

/**
 * Skips bytes from the underlying stream. This is used only to
 * skip the contents of a string, binary blob or extension object.
 */
void mpack_skip_bytes(mpack_reader_t* reader, size_t count);

/**
 * Reads bytes from a string, binary blob or extension object.
 */
void mpack_read_bytes(mpack_reader_t* reader, char* p, size_t count);

/**
 * Reads bytes from a string, binary blob or extension object in-place in
 * the buffer. This can be used to avoid copying the data.
 *
 * The returned pointer is invalidated the next time the reader's fill
 * function is called, or when the buffer is destroyed.
 *
 * The size requested must be at most the buffer size. If the requested size is
 * larger than the buffer size, mpack_error_too_big is raised and the
 * return value is undefined.
 *
 * The reader will move data around in the buffer if needed to ensure that
 * the pointer can always be returned, so it is unlikely to be faster unless
 * count is very small compared to the buffer size. If you need to check
 * whether a small size is reasonable (for example you intend to handle small and
 * large sizes differently), you can call mpack_should_read_bytes_inplace().
 *
 * As with all read functions, the return value is undefined if the reader
 * is in an error state.
 *
 * @see mpack_should_read_bytes_inplace()
 */
const char* mpack_read_bytes_inplace(mpack_reader_t* reader, size_t count);

/**
 * Returns true if it's a good idea to read the given number of bytes
 * in-place.
 *
 * If the read will be larger than some small fraction of the buffer size,
 * this will return false to avoid shuffling too much data back and forth
 * in the buffer.
 *
 * Use this if you're expecting arbitrary size data, and you want to read
 * in-place where possible but will fall back to a normal read if the data
 * is too large.
 *
 * @see mpack_read_bytes_inplace()
 */
static inline bool mpack_should_read_bytes_inplace(mpack_reader_t* reader, size_t count) {
    return (reader->size == 0 || count > reader->size / 8);
}

#if MPACK_READ_TRACKING
/**
 * Finishes reading an array.
 *
 * This will track reads to ensure that the correct number of elements are read.
 */
void mpack_done_array(mpack_reader_t* reader);

/**
 * @fn mpack_done_map(mpack_reader_t* reader)
 *
 * Finishes reading a map.
 *
 * This will track reads to ensure that the correct number of elements are read.
 */
void mpack_done_map(mpack_reader_t* reader);

/**
 * @fn mpack_done_str(mpack_reader_t* reader)
 *
 * Finishes reading a string.
 *
 * This will track reads to ensure that the correct number of bytes are read.
 */
void mpack_done_str(mpack_reader_t* reader);

/**
 * @fn mpack_done_bin(mpack_reader_t* reader)
 *
 * Finishes reading a binary data blob.
 *
 * This will track reads to ensure that the correct number of bytes are read.
 */
void mpack_done_bin(mpack_reader_t* reader);

/**
 * @fn mpack_done_ext(mpack_reader_t* reader)
 *
 * Finishes reading an extended type binary data blob.
 *
 * This will track reads to ensure that the correct number of bytes are read.
 */
void mpack_done_ext(mpack_reader_t* reader);

/**
 * Finishes reading the given type.
 *
 * This will track reads to ensure that the correct number of elements
 * or bytes are read.
 */
void mpack_done_type(mpack_reader_t* reader, mpack_type_t type);
#else
static inline void mpack_done_array(mpack_reader_t* reader) {MPACK_UNUSED(reader);}
static inline void mpack_done_map(mpack_reader_t* reader) {MPACK_UNUSED(reader);}
static inline void mpack_done_str(mpack_reader_t* reader) {MPACK_UNUSED(reader);}
static inline void mpack_done_bin(mpack_reader_t* reader) {MPACK_UNUSED(reader);}
static inline void mpack_done_ext(mpack_reader_t* reader) {MPACK_UNUSED(reader);}
static inline void mpack_done_type(mpack_reader_t* reader, mpack_type_t type) {MPACK_UNUSED(reader); MPACK_UNUSED(type);}
#endif

/**
 * Reads and discards the next object. This will read and discard all
 * contained data as well if it is a compound type.
 */
void mpack_discard(mpack_reader_t* reader);

#if MPACK_DEBUG && MPACK_STDIO && MPACK_SETJMP && !MPACK_NO_PRINT
/*! Converts a chunk of messagepack to JSON and pretty-prints it to stdout. */
void mpack_debug_print(const char* data, int len);
#endif

/**
 * @}
 */



#if MPACK_INTERNAL

void mpack_read_native_big(mpack_reader_t* reader, char* p, size_t count);

// Reads count bytes into p, deferring to mpack_read_native_big() if more
// bytes are needed than are available in the buffer.
static inline void mpack_read_native(mpack_reader_t* reader, char* p, size_t count) {
    if (count > reader->left) {
        mpack_read_native_big(reader, p, count);
    } else {
        mpack_memcpy(p, reader->buffer + reader->pos, count);
        reader->pos += count;
        reader->left -= count;
    }
}

// Reads native bytes with jump disabled. This allows mpack reader functions
// to hold an allocated buffer and read native data into it without leaking it.
static inline void mpack_read_native_nojump(mpack_reader_t* reader, char* p, size_t count) {
    #if MPACK_SETJMP
    jmp_buf* jump_env = reader->jump_env;
    reader->jump_env = NULL;
    #endif
    mpack_read_native(reader, p, count);
    #if MPACK_SETJMP
    reader->jump_env = jump_env;
    #endif
}

MPACK_ALWAYS_INLINE uint8_t mpack_read_native_u8(mpack_reader_t* reader) {
    if (reader->left >= sizeof(uint8_t)) {
        uint8_t ret = mpack_load_native_u8(reader->buffer + reader->pos);
        reader->pos += sizeof(uint8_t);
        reader->left -= sizeof(uint8_t);
        return ret;
    }

    char c[sizeof(uint8_t)];
    mpack_read_native_big(reader, c, sizeof(c));
    return mpack_load_native_u8(c);
}

MPACK_ALWAYS_INLINE uint16_t mpack_read_native_u16(mpack_reader_t* reader) {
    if (reader->left >= sizeof(uint16_t)) {
        uint16_t ret = mpack_load_native_u16(reader->buffer + reader->pos);
        reader->pos += sizeof(uint16_t);
        reader->left -= sizeof(uint16_t);
        return ret;
    }

    char c[sizeof(uint16_t)];
    mpack_read_native_big(reader, c, sizeof(c));
    return mpack_load_native_u16(c);
}

MPACK_ALWAYS_INLINE uint32_t mpack_read_native_u32(mpack_reader_t* reader) {
    if (reader->left >= sizeof(uint32_t)) {
        uint32_t ret = mpack_load_native_u32(reader->buffer + reader->pos);
        reader->pos += sizeof(uint32_t);
        reader->left -= sizeof(uint32_t);
        return ret;
    }

    char c[sizeof(uint32_t)];
    mpack_read_native_big(reader, c, sizeof(c));
    return mpack_load_native_u32(c);
}

MPACK_ALWAYS_INLINE uint64_t mpack_read_native_u64(mpack_reader_t* reader) {
    if (reader->left >= sizeof(uint64_t)) {
        uint64_t ret = mpack_load_native_u64(reader->buffer + reader->pos);
        reader->pos += sizeof(uint64_t);
        reader->left -= sizeof(uint64_t);
        return ret;
    }

    char c[sizeof(uint64_t)];
    mpack_read_native_big(reader, c, sizeof(c));
    return mpack_load_native_u64(c);
}

MPACK_ALWAYS_INLINE int8_t  mpack_read_native_i8  (mpack_reader_t* reader) {return (int8_t) mpack_read_native_u8  (reader);}
MPACK_ALWAYS_INLINE int16_t mpack_read_native_i16 (mpack_reader_t* reader) {return (int16_t)mpack_read_native_u16 (reader);}
MPACK_ALWAYS_INLINE int32_t mpack_read_native_i32 (mpack_reader_t* reader) {return (int32_t)mpack_read_native_u32 (reader);}
MPACK_ALWAYS_INLINE int64_t mpack_read_native_i64 (mpack_reader_t* reader) {return (int64_t)mpack_read_native_u64 (reader);}

MPACK_ALWAYS_INLINE float mpack_read_native_float(mpack_reader_t* reader) {
    union {
        float f;
        uint32_t i;
    } u;
    u.i = mpack_read_native_u32(reader);
    return u.f;
}

MPACK_ALWAYS_INLINE double mpack_read_native_double(mpack_reader_t* reader) {
    union {
        double d;
        uint64_t i;
    } u;
    u.i = mpack_read_native_u64(reader);
    return u.d;
}

#if MPACK_READ_TRACKING
#define MPACK_READER_TRACK(reader, error) mpack_reader_flag_if_error(reader, error)
#else
#define MPACK_READER_TRACK(reader, error) MPACK_UNUSED(reader)
#endif

static inline void mpack_reader_track_element(mpack_reader_t* reader) {
    MPACK_READER_TRACK(reader, mpack_track_element(&reader->track, true));
}

static inline void mpack_reader_track_bytes(mpack_reader_t* reader, uint64_t count) {
    MPACK_READER_TRACK(reader, mpack_track_bytes(&reader->track, true, count));
    MPACK_UNUSED(count);
}

#endif



#ifdef __cplusplus
}
#endif

#endif
#endif


/* mpack-expect.h */

/**
 * @file
 *
 * Declares the MPack static Expect API.
 */

#ifndef MPACK_EXPECT_H
#define MPACK_EXPECT_H 1

/* #include "mpack-reader.h" */

#if MPACK_EXPECT

#if !MPACK_READER
#error "MPACK_EXPECT requires MPACK_READER."
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup expect Expect API
 *
 * The MPack Expect API allows you to easily read MessagePack data when you
 * expect it to follow a predefined schema.
 *
 * The main purpose of the Expect API is convenience, so the API is lax. It
 * allows overlong / inefficiently encoded sequences, and it automatically
 * converts between similar types where there is no loss of precision (unless
 * otherwise noted.) It will convert from unsigned to signed or from float to
 * double for example.
 *
 * When using any of the expect functions, if the type or value of what was
 * read does not match what is expected, @ref mpack_error_type is raised.
 *
 * @{
 */

/**
 * @name Basic Number Functions
 * @{
 */

/**
 * Reads an 8-bit unsigned integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in an 8-bit unsigned int.
 *
 * Returns zero if an error occurs.
 */
uint8_t mpack_expect_u8(mpack_reader_t* reader);

/**
 * Reads a 16-bit unsigned integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 16-bit unsigned int.
 *
 * Returns zero if an error occurs.
 */
uint16_t mpack_expect_u16(mpack_reader_t* reader);

/**
 * Reads a 32-bit unsigned integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 32-bit unsigned int.
 *
 * Returns zero if an error occurs.
 */
uint32_t mpack_expect_u32(mpack_reader_t* reader);

/**
 * Reads a 64-bit unsigned integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 64-bit unsigned int.
 *
 * Returns zero if an error occurs.
 */
uint64_t mpack_expect_u64(mpack_reader_t* reader);

/**
 * Reads an 8-bit signed integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in an 8-bit signed int.
 *
 * Returns zero if an error occurs.
 */
int8_t mpack_expect_i8(mpack_reader_t* reader);

/**
 * Reads a 16-bit signed integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 16-bit signed int.
 *
 * Returns zero if an error occurs.
 */
int16_t mpack_expect_i16(mpack_reader_t* reader);

/**
 * Reads a 32-bit signed integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 32-bit signed int.
 *
 * Returns zero if an error occurs.
 */
int32_t mpack_expect_i32(mpack_reader_t* reader);

/**
 * Reads a 64-bit signed integer.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 64-bit signed int.
 *
 * Returns zero if an error occurs.
 */
int64_t mpack_expect_i64(mpack_reader_t* reader);

/**
 * Reads a number, returning the value as a float. The underlying value can be an
 * integer, float or double; the value is converted to a float.
 *
 * Note that reading a double or a large integer with this function can incur a
 * loss of precision.
 *
 * @throws mpack_error_type if the underlying value is not a float, double or integer.
 */
float mpack_expect_float(mpack_reader_t* reader);

/**
 * Reads a number, returning the value as a double. The underlying value can be an
 * integer, float or double; the value is converted to a double.
 *
 * Note that reading a very large integer with this function can incur a
 * loss of precision.
 *
 * @throws mpack_error_type if the underlying value is not a float, double or integer.
 */
double mpack_expect_double(mpack_reader_t* reader);

/**
 * Reads a float. The underlying value must be a float, not a double or an integer.
 * This ensures no loss of precision can occur.
 *
 * @throws mpack_error_type if the underlying value is not a float.
 */
float mpack_expect_float_strict(mpack_reader_t* reader);

/**
 * Reads a double. The underlying value must be a float or double, not an integer.
 * This ensures no loss of precision can occur.
 *
 * @throws mpack_error_type if the underlying value is not a float or double.
 */
double mpack_expect_double_strict(mpack_reader_t* reader);

/**
 * @}
 */

/**
 * @name Ranged Number Functions
 * @{
 */

/**
 * Reads an 8-bit unsigned integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in an 8-bit unsigned int.
 *
 * Returns min_value if an error occurs.
 */
uint8_t mpack_expect_u8_range(mpack_reader_t* reader, uint8_t min_value, uint8_t max_value);

/**
 * Reads a 16-bit unsigned integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 16-bit unsigned int.
 *
 * Returns min_value if an error occurs.
 */
uint16_t mpack_expect_u16_range(mpack_reader_t* reader, uint16_t min_value, uint16_t max_value);

/**
 * Reads a 32-bit unsigned integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 32-bit unsigned int.
 *
 * Returns min_value if an error occurs.
 */
uint32_t mpack_expect_u32_range(mpack_reader_t* reader, uint32_t min_value, uint32_t max_value);

/**
 * Reads a 64-bit unsigned integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 64-bit unsigned int.
 *
 * Returns min_value if an error occurs.
 */
uint64_t mpack_expect_u64_range(mpack_reader_t* reader, uint64_t min_value, uint64_t max_value);

static inline uint8_t mpack_expect_u8_max(mpack_reader_t* reader, uint8_t max_value) {
    return mpack_expect_u8_range(reader, 0, max_value);
}

static inline uint16_t mpack_expect_u16_max(mpack_reader_t* reader, uint16_t max_value) {
    return mpack_expect_u16_range(reader, 0, max_value);
}

static inline uint32_t mpack_expect_u32_max(mpack_reader_t* reader, uint32_t max_value) {
    return mpack_expect_u32_range(reader, 0, max_value);
}

static inline uint64_t mpack_expect_u64_max(mpack_reader_t* reader, uint64_t max_value) {
    return mpack_expect_u64_range(reader, 0, max_value);
}

/**
 * Reads an 8-bit signed integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in an 8-bit signed int.
 *
 * Returns min_value if an error occurs.
 */
int8_t mpack_expect_i8_range(mpack_reader_t* reader, int8_t min_value, int8_t max_value);

/**
 * Reads a 16-bit signed integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 16-bit signed int.
 *
 * Returns min_value if an error occurs.
 */
int16_t mpack_expect_i16_range(mpack_reader_t* reader, int16_t min_value, int16_t max_value);

/**
 * Reads a 32-bit signed integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 32-bit signed int.
 *
 * Returns min_value if an error occurs.
 */
int32_t mpack_expect_i32_range(mpack_reader_t* reader, int32_t min_value, int32_t max_value);

/**
 * Reads a 64-bit signed integer, ensuring that it falls within the given range.
 *
 * The underlying type may be an integer type of any size and signedness,
 * as long as the value can be represented in a 64-bit signed int.
 *
 * Returns min_value if an error occurs.
 */
int64_t mpack_expect_i64_range(mpack_reader_t* reader, int64_t min_value, int64_t max_value);

/**
 * Reads a number, ensuring that it falls within the given range and returning
 * the value as a float. The underlying value can be an integer, float or
 * double; the value is converted to a float.
 *
 * Note that reading a double or a large integer with this function can incur a
 * loss of precision.
 *
 * @throws mpack_error_type if the underlying value is not a float, double or integer.
 */
float mpack_expect_float_range(mpack_reader_t* reader, float min_value, float max_value);

/**
 * Reads a number, ensuring that it falls within the given range and returning
 * the value as a double. The underlying value can be an integer, float or
 * double; the value is converted to a double.
 *
 * Note that reading a very large integer with this function can incur a
 * loss of precision.
 *
 * @throws mpack_error_type if the underlying value is not a float, double or integer.
 */
double mpack_expect_double_range(mpack_reader_t* reader, double min_value, double max_value);

/**
 * @}
 */

/**
 * @name Matching Number Functions
 * @{
 */

/**
 * Reads an unsigned integer, ensuring that it exactly matches the given value.
 *
 * mpack_error_type is raised if the value is not representable as an unsigned
 * integer or if it does not exactly match the given value.
 */
void mpack_expect_uint_match(mpack_reader_t* reader, uint64_t value);

/**
 * Reads a signed integer, ensuring that it exactly matches the given value.
 *
 * mpack_error_type is raised if the value is not representable as a signed
 * integer or if it does not exactly match the given value.
 */
void mpack_expect_int_match(mpack_reader_t* reader, int64_t value);

/**
 * @name Other Basic Types
 * @{
 */

/**
 * Reads a nil.
 */
void mpack_expect_nil(mpack_reader_t* reader);

/**
 * Reads a bool. Note that integers will raise mpack_error_type; the value
 * must be strictly a bool.
 */
bool mpack_expect_bool(mpack_reader_t* reader);

/**
 * Reads a bool, raising a mpack_error_type if it is not true.
 */
void mpack_expect_true(mpack_reader_t* reader);

/**
 * Reads a bool, raising a mpack_error_type if it is not false.
 */
void mpack_expect_false(mpack_reader_t* reader);


/**
 * @}
 */

/**
 * @name Compound Types
 * @{
 */

/**
 * Reads the start of a map, returning its element count.
 *
 * A number of values follow equal to twice the element count of the map,
 * alternating between keys and values. @ref mpack_done_map() must be called
 * once all elements have been read.
 *
 * mpack_error_type is raised if the value is not a map.
 *
 * Note that maps in JSON are unordered, so it is recommended not to expect
 * a specific ordering for your map values in case your data is converted
 * to/from JSON.
 */
uint32_t mpack_expect_map(mpack_reader_t* reader);

/**
 * Reads the start of a map with a number of elements in the given range, returning
 * its element count.
 *
 * A number of values follow equal to twice the element count of the map,
 * alternating between keys and values. @ref mpack_done_map() must be called
 * once all elements have been read.
 *
 * mpack_error_type is raised if the value is not a map or if its size does
 * not fall within the given range.
 *
 * Note that maps in JSON are unordered, so it is recommended not to expect
 * a specific ordering for your map values in case your data is converted
 * to/from JSON.
 */
uint32_t mpack_expect_map_range(mpack_reader_t* reader, uint32_t min_count, uint32_t max_count);

/**
 * Reads the start of a map of the exact size given.
 *
 * A number of values follow equal to twice the element count of the map,
 * alternating between keys and values. @ref mpack_done_map() must be called
 * once all elements have been read.
 *
 * @ref mpack_error_type is raised if the value is not a map or if its size
 * does not match the given count.
 *
 * Note that maps in JSON are unordered, so it is recommended not to expect
 * a specific ordering for your map values in case your data is converted
 * to/from JSON.
 */
void mpack_expect_map_match(mpack_reader_t* reader, uint32_t count);

bool mpack_expect_map_or_nil(mpack_reader_t* reader, uint32_t* count);

/**
 * Reads the start of an array, returning its element count.
 *
 * A number of values follow equal to the element count of the array.
 * @ref mpack_done_array() must be called once all elements have been read.
 */
uint32_t mpack_expect_array(mpack_reader_t* reader);

/**
 * Reads the start of an array with a number of elements in the given range,
 * returning its element count.
 *
 * A number of values follow equal to the element count of the array.
 * @ref mpack_done_array() must be called once all elements have been read.
 *
 * @throws mpack_error_type if the value is not an array or if its size does
 * not fall within the given range.
 */
uint32_t mpack_expect_array_range(mpack_reader_t* reader, uint32_t min_count, uint32_t max_count);

static inline uint32_t mpack_expect_array_max(mpack_reader_t* reader, uint32_t max_count) {
    return mpack_expect_array_range(reader, 0, max_count);
}

/**
 * Reads the start of an array of the exact size given.
 *
 * A number of values follow equal to the element count of the array.
 * @ref mpack_done_array() must be called once all elements have been read.
 *
 * @throws mpack_error_type if the value is not an array or if its size does
 * not fall within the given range.
 */
void mpack_expect_array_match(mpack_reader_t* reader, uint32_t count);

#ifdef MPACK_MALLOC
/**
 * @hideinitializer
 *
 * Reads the start of an array and allocates storage for it, placing its
 * size in count. A number of objects follow equal to the element count
 * of the array.
 */
#define mpack_expect_array_alloc(reader, Type, max_count, count) \
    ((Type*)mpack_expect_array_alloc_impl(reader, sizeof(Type), max_count, count))
#endif

/**
 * @}
 */

/** @cond */
#ifdef MPACK_MALLOC
void* mpack_expect_array_alloc_impl(mpack_reader_t* reader,
        size_t element_size, uint32_t max_count, size_t* count);
#endif
/** @endcond */


/**
 * @name String Functions
 * @{
 */

/**
 * Reads the start of a string, returning its size in bytes.
 *
 * The bytes follow and must be read separately with mpack_read_bytes()
 * or mpack_read_bytes_inplace(). @ref mpack_done_str() must be called
 * once all bytes have been read.
 *
 * mpack_error_type is raised if the value is not a string.
 */
uint32_t mpack_expect_str(mpack_reader_t* reader);

/**
 * Reads a string of at most the given size, writing it into the
 * given buffer and returning its size in bytes.
 *
 * Note that this does not add a null-terminator! No null-terminator
 * is written, even if the string fits. Use mpack_expect_cstr() to
 * get a null-terminator.
 */
size_t mpack_expect_str_buf(mpack_reader_t* reader, char* buf, size_t bufsize);

/**
 * Reads the start of a string, raising an error if its length is not
 * at most the given number of bytes (not including any null-terminator.)
 *
 * The bytes follow and must be read separately with mpack_read_bytes()
 * or mpack_read_bytes_inplace(). @ref mpack_done_str() must be called
 * once all bytes have been read.
 *
 * mpack_error_type is raised if the value is not a string or if its
 * length does not match.
 */
static inline void mpack_expect_str_max(mpack_reader_t* reader, uint32_t maxsize) {
    if (mpack_expect_str(reader) > maxsize)
        mpack_reader_flag_error(reader, mpack_error_type);
}

/**
 * Reads the start of a string, raising an error if its length is not
 * exactly the given number of bytes (not including any null-terminator.)
 *
 * The bytes follow and must be read separately with mpack_read_bytes()
 * or mpack_read_bytes_inplace(). @ref mpack_done_str() must be called
 * once all bytes have been read.
 *
 * mpack_error_type is raised if the value is not a string or if its
 * length does not match.
 */
static inline void mpack_expect_str_length(mpack_reader_t* reader, uint32_t count) {
    if (mpack_expect_str(reader) != count)
        mpack_reader_flag_error(reader, mpack_error_type);
}


/**
 * Reads a string with the given total maximum size, allocating storage for it.
 * A null-terminator will be added to the string. The length in bytes of the string,
 * not including the null-terminator, will be written to size.
 */
char* mpack_expect_str_alloc(mpack_reader_t* reader, size_t maxsize, size_t* size);

/**
 * Reads a string with the given total maximum size, allocating storage for it
 * and ensuring it is valid UTF-8. A null-terminator will be added to the string.
 * The length in bytes of the string, not including the null-terminator,
 * will be written to size.
 */
char* mpack_expect_utf8_alloc(mpack_reader_t* reader, size_t maxsize, size_t* size);

/**
 * Reads a string into the given buffer, ensures it has no null-bytes,
 * and adds null-terminator at the end.
 *
 * Raises mpack_error_too_big if there is not enough room for the string and null-terminator.
 * Raises mpack_error_type if the value is not a string or contains a null byte.
 */
void mpack_expect_cstr(mpack_reader_t* reader, char* buf, size_t size);

/**
 * Reads a string into the given buffer, ensures it is a valid UTF-8 string,
 * and adds null-terminator at the end.
 *
 * Raises mpack_error_too_big if there is not enough room for the string and null-terminator.
 * Raises mpack_error_type if the value is not a string or is not a valid UTF-8 string.
 */
void mpack_expect_utf8_cstr(mpack_reader_t* reader, char* buf, size_t size);

#ifdef MPACK_MALLOC
/**
 * Reads a string, allocates storage for it, ensures it has no null-bytes,
 * and adds null-terminator at the end. You assume ownership of the
 * returned pointer if reading succeeds.
 *
 * Raises mpack_error_too_big if the string plus null-terminator is larger than the given maxsize.
 * Raises mpack_error_invalid if the value is not a string or contains a null byte.
 */
char* mpack_expect_cstr_alloc(mpack_reader_t* reader, size_t maxsize);
#endif

/**
 * Reads a string, ensuring it exactly matches the given null-terminated
 * string.
 *
 * Remember that maps are unordered in JSON. Don't use this for map keys
 * unless the map has only a single key!
 */
void mpack_expect_cstr_match(mpack_reader_t* reader, const char* str);

/**
 * @}
 */

/**
 * @name Binary Data / Extension Functions
 * @{
 */

/**
 * Reads the start of a binary blob, returning its size in bytes.
 *
 * The bytes follow and must be read separately with mpack_read_bytes()
 * or mpack_read_bytes_inplace(). @ref mpack_done_bin() must be called
 * once all bytes have been read.
 *
 * mpack_error_type is raised if the value is not a binary blob.
 */
uint32_t mpack_expect_bin(mpack_reader_t* reader);

/**
 * Reads the start of a binary blob, raising an error if its length is not
 * at most the given number of bytes.
 *
 * The bytes follow and must be read separately with mpack_read_bytes()
 * or mpack_read_bytes_inplace(). @ref mpack_done_bin() must be called
 * once all bytes have been read.
 *
 * mpack_error_type is raised if the value is not a binary blob or if its
 * length does not match.
 */
static inline void mpack_expect_bin_max(mpack_reader_t* reader, uint32_t maxsize) {
    if (mpack_expect_str(reader) > maxsize)
        mpack_reader_flag_error(reader, mpack_error_type);
}

/**
 * Reads the start of a binary blob, raising an error if its length is not
 * exactly the given number of bytes.
 *
 * The bytes follow and must be read separately with mpack_read_bytes()
 * or mpack_read_bytes_inplace(). @ref mpack_done_bin() must be called
 * once all bytes have been read.
 *
 * mpack_error_type is raised if the value is not a binary blob or if its
 * length does not match.
 */
static inline void mpack_expect_bin_size(mpack_reader_t* reader, uint32_t count) {
    if (mpack_expect_str(reader) != count)
        mpack_reader_flag_error(reader, mpack_error_type);
}

/**
 * Reads a binary blob into the given buffer, returning its size in bytes.
 *
 * For compatibility, this will accept if the underlying type is string or
 * binary (since in MessagePack 1.0, strings and binary data were combined
 * under the "raw" type which became string in 1.1.)
 */
size_t mpack_expect_bin_buf(mpack_reader_t* reader, char* buf, size_t size);

const char* mpack_expect_bin_inplace(mpack_reader_t* reader, size_t maxsize, size_t* size);

/**
 * Reads a binary blob with the given total maximum size, allocating storage for it.
 */
char* mpack_expect_bin_alloc(mpack_reader_t* reader, size_t maxsize, size_t* size);

/**
 * Reads an extension object with the given total maximum size, allocating storage
 * for it. The extension type will be written to exttype, and the size will be
 * written to size.
 */
char* mpack_expect_ext_alloc(mpack_reader_t* reader, size_t maxsize, uint8_t* exttype, size_t* size);

/**
 * Reads an extension object of the given type with the given total maximum size,
 * allocating storage for it. The size will be written to size.
 */
char* mpack_expect_ext_type_alloc(mpack_reader_t* reader, uint8_t exttype, size_t maxsize, size_t* size);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
#endif



/* mpack-node.h */

/**
 * @file
 *
 * Declares the MPack dynamic Node API.
 */

#ifndef MPACK_NODE_H
#define MPACK_NODE_H 1

/* #include "mpack-reader.h" */

#if MPACK_NODE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup node Node API
 *
 * The MPack Node API allows you to parse a chunk of MessagePack data
 * in-place into a dynamically typed data structure.
 *
 * @{
 */

/**
 * A handle to node in a parsed MPack tree. Note that mpack_node_t is passed by value.
 *
 * Nodes represent either primitive values or compound types. If a
 * node is a compound type, it contains a link to its child nodes, or
 * a pointer to its underlying data.
 *
 * Nodes are immutable.
 */
typedef struct mpack_node_t mpack_node_t;

/**
 * The storage for nodes in an MPack tree.
 *
 * You only need to use this if you intend to provide your own storage
 * for nodes instead of letting the tree allocate it.
 */
typedef struct mpack_node_data_t mpack_node_data_t;

/**
 * An MPack tree parsed from a blob of MessagePack.
 *
 * The tree contains a single root node which contains all parsed data.
 * The tree and its nodes are immutable.
 */
typedef struct mpack_tree_t mpack_tree_t;

/**
 * A teardown function to be called when the tree is destroyed.
 */
typedef void (*mpack_tree_teardown_t)(mpack_tree_t* tree);



/* Hide internals from documentation */
/** @cond */

/*
 * mpack_tree_link_t forms a linked list of node pages. It is allocated
 * separately from the page so that we can store the first link internally
 * without a malloc (the only link in a pooled tree), and we don't
 * affect the size of page pools or violate strict aliasing.
 */
typedef struct mpack_tree_link_t {
    struct mpack_tree_link_t* next;
    mpack_node_data_t* nodes;
    size_t pos;
    size_t left;
} mpack_tree_link_t;

struct mpack_node_t {
    mpack_node_data_t* data;
    mpack_tree_t* tree;
};

struct mpack_node_data_t {
    mpack_type_t type;

    int8_t exttype; /**< \internal The extension type if the type is mpack_type_ext. */

    /* The value for non-compound types. */
    union
    {
        bool     b; /* The value if the type is bool. */
        float    f; /* The value if the type is float. */
        double   d; /* The value if the type is double. */
        int64_t  i; /* The value if the type is signed int. */
        uint64_t u; /* The value if the type is unsigned int. */

        struct {
            uint32_t l; /* The number of bytes if the type is str, bin or ext. */
            const char* bytes;
        } data;

        struct {
            /* The element count if the type is an array, or the number of
               key/value pairs if the type is map. */
            uint32_t n;
            mpack_node_data_t* children;
        } content;
    } value;
};

struct mpack_tree_t {
    mpack_tree_teardown_t teardown; /* Function to teardown the context on destroy */
    void* context;                  /* Context for tree callbacks */

    mpack_node_data_t nil_node; /* a nil node to be returned in case of error */
    mpack_error_t error;

    size_t node_count;
    size_t size;
    mpack_node_data_t* root;

    mpack_tree_link_t page;
    #ifdef MPACK_MALLOC
    bool owned;
    #endif

    #if MPACK_SETJMP
    /* Optional jump target in case of error (pointer because it's
     * very large and may be unused) */
    jmp_buf* jump_env;
    #endif
};

// internal functions

static inline mpack_node_t mpack_node(mpack_tree_t* tree, mpack_node_data_t* data) {
    mpack_node_t node;
    node.data = data;
    node.tree = tree;
    return node;
}

static inline mpack_node_data_t* mpack_node_child(mpack_node_t node, size_t child) {
    return node.data->value.content.children + child;
}

static inline mpack_node_t mpack_tree_nil_node(mpack_tree_t* tree) {
    return mpack_node(tree, &tree->nil_node);
}

/** @endcond */



/**
 * @name Tree Functions
 * @{
 */

#ifdef MPACK_MALLOC
/**
 * Initializes a tree by parsing the given data buffer. The tree must be destroyed
 * with mpack_tree_destroy(), even if parsing fails.
 *
 * The tree will allocate pages of nodes as needed, and free them when destroyed.
 *
 * Any string or blob data types reference the original data, so the data
 * pointer must remain valid until after the tree is destroyed.
 */
void mpack_tree_init(mpack_tree_t* tree, const char* data, size_t length);
#endif

/**
 * Initializes a tree by parsing the given data buffer, using the given
 * node data pool to store the results.
 *
 * If the data does not fit in the pool, mpack_error_too_big will be flagged
 * on the tree.
 *
 * The tree must be destroyed with mpack_tree_destroy(), even if parsing fails.
 */
void mpack_tree_init_pool(mpack_tree_t* tree, const char* data, size_t length, mpack_node_data_t* node_pool, size_t node_pool_count);

/**
 * Initializes an mpack tree directly into an error state. Use this if you
 * are writing a wrapper to mpack_tree_init() which can fail its setup.
 */
void mpack_tree_init_error(mpack_tree_t* tree, mpack_error_t error);

#if MPACK_STDIO
/**
 * Initializes a tree by reading and parsing the given file. The tree must be
 * destroyed with mpack_tree_destroy(), even if parsing fails.
 *
 * The file is opened, loaded fully into memory, and closed before this call
 * returns.
 *
 * @param tree The tree to initialize
 * @param filename The filename passed to fopen() to read the file
 * @param max_bytes The maximum size of file to load, or 0 for unlimited size.
 */
void mpack_tree_init_file(mpack_tree_t* tree, const char* filename, size_t max_bytes);
#endif

/**
 * Returns the root node of the tree, if the tree is not in an error state.
 * Returns a nil node otherwise.
 */
mpack_node_t mpack_tree_root(mpack_tree_t* tree);

/**
 * Returns the error state of the tree.
 */
static inline mpack_error_t mpack_tree_error(mpack_tree_t* tree) {
    return tree->error;
}

/**
 * Returns the number of bytes used in the buffer when the tree was
 * parsed. If there is something in the buffer after the MessagePack
 * object (such as another object), this can be used to find it.
 */
static inline size_t mpack_tree_size(mpack_tree_t* tree) {
    return tree->size;
}

/**
 * Destroys the tree.
 */
mpack_error_t mpack_tree_destroy(mpack_tree_t* tree);

/**
 * Sets the custom pointer to pass to the tree callbacks, such as teardown.
 *
 * @param tree The MPack tree.
 * @param context User data to pass to the tree callbacks.
 */
static inline void mpack_tree_set_context(mpack_tree_t* tree, void* context) {
    tree->context = context;
}

/**
 * Sets the teardown function to call when the tree is destroyed.
 *
 * This should normally be used with mpack_tree_set_context() to register
 * a custom pointer to pass to the teardown function.
 *
 * @param tree The MPack tree.
 * @param teardown The function to call when the tree is destroyed.
 */
static inline void mpack_tree_set_teardown(mpack_tree_t* tree, mpack_tree_teardown_t teardown) {
    tree->teardown = teardown;
}

#if MPACK_SETJMP

/**
 * @hideinitializer
 *
 * Registers a jump target in case of error.
 *
 * If the tree is in an error state, 1 is returned when this is called. Otherwise
 * 0 is returned when this is called, and when the first error occurs, control flow
 * will jump to the point where this was called, resuming as though it returned 1.
 * This ensures an error handling block runs exactly once in case of error.
 *
 * A tree that jumps still needs to be destroyed. You must call
 * mpack_tree_destroy() in your jump handler after getting the final error state.
 *
 * The argument may be evaluated multiple times.
 *
 * @returns 0 if the tree is not in an error state; 1 if and when an error occurs.
 * @see mpack_tree_destroy()
 */
#define MPACK_TREE_SETJMP(tree)                                          \
    (mpack_assert((tree)->jump_env == NULL, "already have a jump set!"), \
    ((tree)->error != mpack_ok) ? 1 :                                    \
        !((tree)->jump_env = (jmp_buf*)MPACK_MALLOC(sizeof(jmp_buf))) ?  \
            ((tree)->error = mpack_error_memory, 1) :                    \
            (setjmp(*(tree)->jump_env)))

/**
 * Clears a jump target. Subsequent tree reading errors will not cause a jump.
 */
static inline void mpack_tree_clearjmp(mpack_tree_t* tree) {
    if (tree->jump_env)
        MPACK_FREE(tree->jump_env);
    tree->jump_env = NULL;
}
#endif

/**
 * Places the tree in the given error state, jumping if a jump target is set.
 *
 * This allows you to externally flag errors, for example if you are validating
 * data as you read it.
 *
 * If the tree is already in an error state, this call is ignored and no jump
 * is performed.
 */
void mpack_tree_flag_error(mpack_tree_t* tree, mpack_error_t error);

/**
 * Places the node's tree in the given error state, jumping if a jump target is set.
 *
 * This allows you to externally flag errors, for example if you are validating
 * data as you read it.
 *
 * If the tree is already in an error state, this call is ignored and no jump
 * is performed.
 */
void mpack_node_flag_error(mpack_node_t node, mpack_error_t error);

/**
 * @}
 */

/**
 * @name Node Core Functions
 * @{
 */

/**
 * Returns the error state of the node's tree.
 */
static inline mpack_error_t mpack_node_error(mpack_node_t node) {
    return mpack_tree_error(node.tree);
}

/**
 * Returns a tag describing the given node.
 */
static inline mpack_tag_t mpack_node_tag(mpack_node_t node) {
    mpack_tag_t tag;
    mpack_memset(&tag, 0, sizeof(tag));
    tag.type = node.data->type;
    switch (node.data->type) {
        case mpack_type_nil:                                            break;
        case mpack_type_bool:    tag.v.b = node.data->value.b;          break;
        case mpack_type_float:   tag.v.f = node.data->value.f;          break;
        case mpack_type_double:  tag.v.d = node.data->value.d;          break;
        case mpack_type_int:     tag.v.i = node.data->value.i;          break;
        case mpack_type_uint:    tag.v.u = node.data->value.u;          break;
        case mpack_type_str:     tag.v.l = node.data->value.data.l;     break;
        case mpack_type_bin:     tag.v.l = node.data->value.data.l;     break;
        case mpack_type_ext:     tag.v.l = node.data->value.data.l;     break;
        case mpack_type_array:   tag.v.n = node.data->value.content.n;  break;
        case mpack_type_map:     tag.v.n = node.data->value.content.n;  break;
    }
    return tag;
}

#if MPACK_DEBUG && MPACK_STDIO && MPACK_SETJMP && !MPACK_NO_PRINT
/**
 * Converts a node to JSON and pretty-prints it to stdout.
 *
 * This function is only available in debugging mode.
 */
void mpack_node_print(mpack_node_t node);
#endif

/**
 * @}
 */

/**
 * @name Node Primitive Value Functions
 * @{
 */

/**
 * Returns the type of the node.
 */
static inline mpack_type_t mpack_node_type(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return mpack_type_nil;
    return node.data->type;
}

/**
 * Checks if the given node is of nil type, raising mpack_error_type otherwise.
 */
static inline void mpack_node_nil(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return;
    if (node.data->type != mpack_type_nil)
        mpack_node_flag_error(node, mpack_error_type);
}

/**
 * Returns the bool value of the node. If this node is not of the correct
 * type, mpack_error_type is raised, and the return value should be discarded.
 */
static inline bool mpack_node_bool(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return false;

    if (node.data->type == mpack_type_bool)
        return node.data->value.b;

    mpack_node_flag_error(node, mpack_error_type);
    return false;
}

/**
 * Checks if the given node is of bool type with value true, raising
 * mpack_error_type otherwise.
 */
static inline void mpack_node_true(mpack_node_t node) {
    if (mpack_node_bool(node) != true)
        mpack_node_flag_error(node, mpack_error_type);
}

/**
 * Checks if the given node is of bool type with value false, raising
 * mpack_error_type otherwise.
 */
static inline void mpack_node_false(mpack_node_t node) {
    if (mpack_node_bool(node) != false)
        mpack_node_flag_error(node, mpack_error_type);
}

/**
 * Returns the 8-bit unsigned value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline uint8_t mpack_node_u8(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= UINT8_MAX)
            return (uint8_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= 0 && node.data->value.i <= UINT8_MAX)
            return (uint8_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 8-bit signed value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline int8_t mpack_node_i8(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= INT8_MAX)
            return (int8_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= INT8_MIN && node.data->value.i <= INT8_MAX)
            return (int8_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 16-bit unsigned value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline uint16_t mpack_node_u16(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= UINT16_MAX)
            return (uint16_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= 0 && node.data->value.i <= UINT16_MAX)
            return (uint16_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 16-bit signed value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline int16_t mpack_node_i16(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= INT16_MAX)
            return (int16_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= INT16_MIN && node.data->value.i <= INT16_MAX)
            return (int16_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 32-bit unsigned value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline uint32_t mpack_node_u32(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= UINT32_MAX)
            return (uint32_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= 0 && node.data->value.i <= UINT32_MAX)
            return (uint32_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 32-bit signed value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline int32_t mpack_node_i32(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= INT32_MAX)
            return (int32_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= INT32_MIN && node.data->value.i <= INT32_MAX)
            return (int32_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 64-bit unsigned value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline uint64_t mpack_node_u64(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        return node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        if (node.data->value.i >= 0)
            return (uint64_t)node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the 64-bit signed value of the node. If this node is not
 * of a compatible type, mpack_error_type is raised, and the
 * return value should be discarded.
 */
static inline int64_t mpack_node_i64(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_uint) {
        if (node.data->value.u <= (uint64_t)INT64_MAX)
            return (int64_t)node.data->value.u;
    } else if (node.data->type == mpack_type_int) {
        return node.data->value.i;
    }

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the float value of the node. The underlying value can be an
 * integer, float or double; the value is converted to a float.
 *
 * Note that reading a double or a large integer with this function can incur a
 * loss of precision.
 *
 * @throws mpack_error_type if the underlying value is not a float, double or integer.
 */
static inline float mpack_node_float(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0.0f;

    if (node.data->type == mpack_type_uint)
        return (float)node.data->value.u;
    else if (node.data->type == mpack_type_int)
        return (float)node.data->value.i;
    else if (node.data->type == mpack_type_float)
        return node.data->value.f;
    else if (node.data->type == mpack_type_double)
        return (float)node.data->value.d;

    mpack_node_flag_error(node, mpack_error_type);
    return 0.0f;
}

/**
 * Returns the double value of the node. The underlying value can be an
 * integer, float or double; the value is converted to a double.
 *
 * Note that reading a very large integer with this function can incur a
 * loss of precision.
 *
 * @throws mpack_error_type if the underlying value is not a float, double or integer.
 */
static inline double mpack_node_double(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0.0;

    if (node.data->type == mpack_type_uint)
        return (double)node.data->value.u;
    else if (node.data->type == mpack_type_int)
        return (double)node.data->value.i;
    else if (node.data->type == mpack_type_float)
        return (double)node.data->value.f;
    else if (node.data->type == mpack_type_double)
        return node.data->value.d;

    mpack_node_flag_error(node, mpack_error_type);
    return 0.0;
}

/**
 * Returns the float value of the node. The underlying value must be a float,
 * not a double or an integer. This ensures no loss of precision can occur.
 *
 * @throws mpack_error_type if the underlying value is not a float.
 */
static inline float mpack_node_float_strict(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0.0f;

    if (node.data->type == mpack_type_float)
        return node.data->value.f;

    mpack_node_flag_error(node, mpack_error_type);
    return 0.0f;
}

/**
 * Returns the double value of the node. The underlying value must be a float
 * or double, not an integer. This ensures no loss of precision can occur.
 *
 * @throws mpack_error_type if the underlying value is not a float or double.
 */
static inline double mpack_node_double_strict(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0.0;

    if (node.data->type == mpack_type_float)
        return (double)node.data->value.f;
    else if (node.data->type == mpack_type_double)
        return node.data->value.d;

    mpack_node_flag_error(node, mpack_error_type);
    return 0.0;
}

/**
 * @}
 */

/**
 * @name Node Data Functions
 * @{
 */

/**
 * Returns the extension type of the given ext node.
 */
static inline int8_t mpack_node_exttype(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_ext)
        return node.data->exttype;

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the length of the given str, bin or ext node.
 */
static inline size_t mpack_node_data_len(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    mpack_type_t type = node.data->type;
    if (type == mpack_type_str || type == mpack_type_bin || type == mpack_type_ext)
        return (size_t)node.data->value.data.l;

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns the length in bytes of the given string node. This does not
 * include any null-terminator.
 */
static inline size_t mpack_node_strlen(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type == mpack_type_str)
        return (size_t)node.data->value.data.l;

    mpack_node_flag_error(node, mpack_error_type);
    return 0;
}

/**
 * Returns a pointer to the data contained by this node.
 *
 * Note that strings are not null-terminated! Use mpack_node_copy_cstr() or
 * mpack_node_cstr_alloc() to get a null-terminated string.
 *
 * The pointer is valid as long as the data backing the tree is valid.
 *
 * If this node is not of a str, bin or map, mpack_error_type is raised, and
 * NULL is returned.
 */
static inline const char* mpack_node_data(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return NULL;

    mpack_type_t type = node.data->type;
    if (type == mpack_type_str || type == mpack_type_bin || type == mpack_type_ext)
        return node.data->value.data.bytes;

    mpack_node_flag_error(node, mpack_error_type);
    return NULL;
}

/**
 * Copies the bytes contained by this node into the given buffer, returning the
 * number of bytes in the node.
 *
 * If this node is not of a str, bin or map, mpack_error_type is raised, and the
 * buffer and return value should be discarded. If the node's data does not fit
 * in the given buffer, mpack_error_data is raised, and the buffer and return value
 * should be discarded.
 *
 * @param node The string node from which to copy data
 * @param buffer A buffer in which to copy the node's bytes
 * @param size The size of the given buffer
 */
size_t mpack_node_copy_data(mpack_node_t node, char* buffer, size_t size);

/**
 * Copies the bytes contained by this string node into the given buffer and adds
 * a null terminator. If this node is not of a string type, mpack_error_type is raised,
 * and the buffer should be discarded. If the string does not fit, mpack_error_data is
 * raised, and the buffer should be discarded.
 *
 * If this node is not of a string type, mpack_error_type is raised, and the
 * buffer and return value should be discarded. If the string and null-terminator
 * do not fit in the given buffer, mpack_error_data is raised, and the buffer and
 * return value should be discarded.
 *
 * @param node The string node from which to copy data
 * @param buffer A buffer in which to copy the node's string
 * @param size The size of the given buffer
 */
void mpack_node_copy_cstr(mpack_node_t node, char* buffer, size_t size);

#ifdef MPACK_MALLOC
/**
 * Allocates a new chunk of data using MPACK_MALLOC with the bytes
 * contained by this node. The returned string should be freed with MPACK_FREE.
 *
 * If this node is not a str, bin or ext type, mpack_error_type is raised
 * and the return value should be discarded. If the string and null-terminator
 * are longer than the given maximum length, mpack_error_too_big is raised, and
 * the return value should be discarded. If an allocation failure occurs,
 * mpack_error_memory is raised and the return value should be discarded.
 */
char* mpack_node_data_alloc(mpack_node_t node, size_t maxlen);

/**
 * Allocates a new null-terminated string using MPACK_MALLOC with the string
 * contained by this node. The returned string should be freed with MPACK_FREE.
 *
 * If this node is not a string type, mpack_error_type is raised, and the return
 * value should be discarded.
 */
char* mpack_node_cstr_alloc(mpack_node_t node, size_t maxlen);
#endif

/**
 * @}
 */

/**
 * @name Compound Node Functions
 * @{
 */

// internal implementation of map key lookup functions to support optional flag
mpack_node_t mpack_node_map_str_impl(mpack_node_t node, const char* str, size_t length, bool optional);
mpack_node_t mpack_node_map_int_impl(mpack_node_t node, int64_t num, bool optional);
mpack_node_t mpack_node_map_uint_impl(mpack_node_t node, uint64_t num, bool optional);

/**
 * Returns the length of the given array node. Raises mpack_error_type
 * and returns 0 if the given node is not an array.
 */
static inline size_t mpack_node_array_length(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type != mpack_type_array) {
        mpack_node_flag_error(node, mpack_error_type);
        return 0;
    }

    return (size_t)node.data->value.content.n;
}

/**
 * Returns the node in the given array at the given index. If the node
 * is not an array, mpack_error_type is raised and a nil node is returned.
 * If the given index is out of bounds, mpack_error_data is raised and
 * a nil node is returned.
 */
static inline mpack_node_t mpack_node_array_at(mpack_node_t node, size_t index) {
    if (mpack_node_error(node) != mpack_ok)
        return mpack_tree_nil_node(node.tree);

    if (node.data->type != mpack_type_array) {
        mpack_node_flag_error(node, mpack_error_type);
        return mpack_tree_nil_node(node.tree);
    }

    if (index >= node.data->value.content.n) {
        mpack_node_flag_error(node, mpack_error_data);
        return mpack_tree_nil_node(node.tree);
    }

    return mpack_node(node.tree, mpack_node_child(node, index));
}

/**
 * Returns the number of key/value pairs in the given map node. Raises
 * mpack_error_type and returns 0 if the given node is not a map.
 */
static inline size_t mpack_node_map_count(mpack_node_t node) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    if (node.data->type != mpack_type_map) {
        mpack_node_flag_error(node, mpack_error_type);
        return 0;
    }

    return node.data->value.content.n;
}

// internal node map lookup
static inline mpack_node_t mpack_node_map_at(mpack_node_t node, size_t index, size_t offset) {
    if (mpack_node_error(node) != mpack_ok)
        return mpack_tree_nil_node(node.tree);

    if (node.data->type != mpack_type_map) {
        mpack_node_flag_error(node, mpack_error_type);
        return mpack_tree_nil_node(node.tree);
    }

    if (index >= node.data->value.content.n) {
        mpack_node_flag_error(node, mpack_error_data);
        return mpack_tree_nil_node(node.tree);
    }

    return mpack_node(node.tree, mpack_node_child(node, index * 2 + offset));
}

/**
 * Returns the key node in the given map at the given index.
 *
 * A nil node is returned in case of error.
 *
 * @throws mpack_error_type if the node is not a map
 * @throws mpack_error_data if the given index is out of bounds
 */
static inline mpack_node_t mpack_node_map_key_at(mpack_node_t node, size_t index) {
    return mpack_node_map_at(node, index, 0);
}

/**
 * Returns the value node in the given map at the given index.
 *
 * A nil node is returned in case of error.
 *
 * @throws mpack_error_type if the node is not a map
 * @throws mpack_error_data if the given index is out of bounds
 */
static inline mpack_node_t mpack_node_map_value_at(mpack_node_t node, size_t index) {
    return mpack_node_map_at(node, index, 1);
}

/**
 * Returns the value node in the given map for the given integer key. If the given
 * node is not a map, mpack_error_type is raised and a nil node is
 * returned. If the given key does not exist in the map, mpack_error_data
 * is raised and a nil node is returned.
 */
static inline mpack_node_t mpack_node_map_int(mpack_node_t node, int64_t num) {
    return mpack_node_map_int_impl(node, num, false);
}

/**
 * Returns the value node in the given map for the given integer key, or NULL
 * if the map does not contain the given key.
 *
 * @throws mpack_error_type if the node is not a map
 */
static inline mpack_node_t mpack_node_map_int_optional(mpack_node_t node, int64_t num) {
    return mpack_node_map_int_impl(node, num, true);
}

/**
 * Returns the value node in the given map for the given unsigned integer key. If
 * the given node is not a map, mpack_error_type is raised and a nil node is
 * returned. If the given key does not exist in the map, mpack_error_data
 * is raised and a nil node is returned.
 */
static inline mpack_node_t mpack_node_map_uint(mpack_node_t node, uint64_t num) {
    return mpack_node_map_uint_impl(node, num, false);
}

/**
 * Returns the value node in the given map for the given unsigned integer
 * key, or NULL if the map does not contain the given key.
 *
 * @throws mpack_error_type if the node is not a map
 */
static inline mpack_node_t mpack_node_map_uint_optional(mpack_node_t node, uint64_t num) {
    return mpack_node_map_uint_impl(node, num, true);
}

/**
 * Returns the value node in the given map for the given string key. If the given
 * node is not a map, mpack_error_type is raised and a nil node is
 * returned. If the given key does not exist in the map, mpack_error_data
 * is raised and a nil node is returned.
 */
static inline mpack_node_t mpack_node_map_str(mpack_node_t node, const char* str, size_t length) {
    return mpack_node_map_str_impl(node, str, length, false);
}

/**
 * Returns the value node in the given map for the given string key, or NULL
 * if the map does not contain the given key.
 *
 * @throws mpack_error_type if the node is not a map
 */
static inline mpack_node_t mpack_node_map_str_optional(mpack_node_t node, const char* str, size_t length) {
    return mpack_node_map_str_impl(node, str, length, true);
}

/**
 * Returns the value node in the given map for the given null-terminated string key.
 * If the given node is not a map, mpack_error_type is raised and a nil node is
 * returned. If the given key does not exist in the map, mpack_error_data
 * is raised and a nil node is returned.
 */
static inline mpack_node_t mpack_node_map_cstr(mpack_node_t node, const char* cstr) {
    return mpack_node_map_str(node, cstr, mpack_strlen(cstr));
}

/**
 * Returns the value node in the given map for the given null-terminated
 * string key, or NULL if the map does not contain the given key.
 *
 * @throws mpack_error_type if the node is not a map
 */
static inline mpack_node_t mpack_node_map_cstr_optional(mpack_node_t node, const char* cstr) {
    return mpack_node_map_str_optional(node, cstr, mpack_strlen(cstr));
}

/**
 * Returns true if the given node map contains a value for the given string key.
 * If the given node is not a map, mpack_error_type is raised and null is
 * returned.
 */
bool mpack_node_map_contains_str(mpack_node_t node, const char* str, size_t length);

/**
 * Returns true if the given node map contains a value for the given
 * null-terminated string key. If the given node is not a map, mpack_error_type
 * is raised and null is returned.
 */
static inline bool mpack_node_map_contains_cstr(mpack_node_t node, const char* cstr) {
    return mpack_node_map_contains_str(node, cstr, mpack_strlen(cstr));
}

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
#endif


#endif

