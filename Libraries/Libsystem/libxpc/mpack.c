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

#define MPACK_INTERNAL 1

#include "mpack.h"


/* mpack-platform.c */

#define MPACK_INTERNAL 1

/* #include "mpack-platform.h" */

#if MPACK_DEBUG && MPACK_STDIO
#include <stdarg.h>
#endif



#if MPACK_DEBUG && MPACK_STDIO
void mpack_assert_fail_format(const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = 0;
    mpack_assert_fail(buffer);
}

void mpack_break_hit_format(const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = 0;
    mpack_break_hit(buffer);
}
#endif

#if MPACK_CUSTOM_ASSERT
void mpack_break_hit(const char* message) {
    // If we have a custom assert handler, break just wraps it
    // for simplicity.
    mpack_assert_fail(message);
}
#else
void mpack_assert_fail(const char* message) {
    MPACK_UNUSED(message);

    #if MPACK_STDIO
    fprintf(stderr, "%s\n", message);
    #endif

    #if defined(__GCC__) || defined(__clang__)
    __builtin_trap();
    #elif WIN32
    __debugbreak();
    #endif

    #if MPACK_STDLIB
    abort();
    #elif defined(__GCC__) || defined(__clang__)
    __builtin_abort();
    #endif

    MPACK_UNREACHABLE;
}

void mpack_break_hit(const char* message) {
    MPACK_UNUSED(message);

    #if MPACK_STDIO
    fprintf(stderr, "%s\n", message);
    #endif

    #if defined(__GCC__) || defined(__clang__)
    __builtin_trap();
    #elif WIN32
    __debugbreak();
    #elif MPACK_STDLIB
    abort();
    #elif defined(__GCC__) || defined(__clang__)
    __builtin_abort();
    #endif
}
#endif



#if !MPACK_STDLIB

// The below are adapted from the C wikibook:
//     https://en.wikibooks.org/wiki/C_Programming/Strings

void* mpack_memset(void *s, int c, size_t n) {
    unsigned char *us = (unsigned char *)s;
    unsigned char uc = (unsigned char)c;
    while (n-- != 0)
        *us++ = uc;
    return s;
}

void* mpack_memcpy(void *s1, const void *s2, size_t n) {
    char * __restrict dst = (char *)s1;
    const char * __restrict src = (const char *)s2;
    while (n-- != 0)
        *dst++ = *src++;
    return s1;
}

void* mpack_memmove(void *s1, const void *s2, size_t n) {
    char *p1 = (char *)s1;
    const char *p2 = (const char *)s2;
    if (p2 < p1 && p1 < p2 + n) {
        p2 += n;
        p1 += n;
        while (n-- != 0)
            *--p1 = *--p2;
    } else
        while (n-- != 0)
            *p1++ = *p2++;
    return s1;
}

int mpack_memcmp(const void* s1, const void* s2, size_t n) {
     const unsigned char *us1 = (const unsigned char *) s1;
     const unsigned char *us2 = (const unsigned char *) s2;
     while (n-- != 0) {
         if (*us1 != *us2)
             return (*us1 < *us2) ? -1 : +1;
         us1++;
         us2++;
     }
     return 0;
}

size_t mpack_strlen(const char *s) {
    const char *p = s;
    while (*p != '\0')
        p++;
    return (size_t)(p - s);
}

#endif



#if defined(MPACK_MALLOC) && !defined(MPACK_REALLOC)
void* mpack_realloc(void* old_ptr, size_t used_size, size_t new_size) {
    void* new_ptr = malloc(new_size);
    if (new_ptr == NULL)
        return NULL;
    mpack_memcpy(new_ptr, old_ptr, used_size);
    MPACK_FREE(old_ptr);
    return new_ptr;
}
#endif

/* mpack-common.c */

#define MPACK_INTERNAL 1

/* #include "mpack-common.h" */

#if MPACK_DEBUG && MPACK_STDIO
#include <stdarg.h>
#endif

const char* mpack_error_to_string(mpack_error_t error) {
    #if MPACK_DEBUG
    switch (error) {
        #define MPACK_ERROR_STRING_CASE(e) case e: return #e
        MPACK_ERROR_STRING_CASE(mpack_ok);
        MPACK_ERROR_STRING_CASE(mpack_error_io);
        MPACK_ERROR_STRING_CASE(mpack_error_invalid);
        MPACK_ERROR_STRING_CASE(mpack_error_type);
        MPACK_ERROR_STRING_CASE(mpack_error_too_big);
        MPACK_ERROR_STRING_CASE(mpack_error_memory);
        MPACK_ERROR_STRING_CASE(mpack_error_bug);
        MPACK_ERROR_STRING_CASE(mpack_error_data);
        #undef MPACK_ERROR_STRING_CASE
        default: break;
    }
    mpack_assert(0, "unrecognized error %i", (int)error);
    return "(unknown mpack_error_t)";
    #else
    MPACK_UNUSED(error);
    return "";
    #endif
}

const char* mpack_type_to_string(mpack_type_t type) {
    #if MPACK_DEBUG
    switch (type) {
        #define MPACK_TYPE_STRING_CASE(e) case e: return #e
        MPACK_TYPE_STRING_CASE(mpack_type_nil);
        MPACK_TYPE_STRING_CASE(mpack_type_bool);
        MPACK_TYPE_STRING_CASE(mpack_type_float);
        MPACK_TYPE_STRING_CASE(mpack_type_double);
        MPACK_TYPE_STRING_CASE(mpack_type_int);
        MPACK_TYPE_STRING_CASE(mpack_type_uint);
        MPACK_TYPE_STRING_CASE(mpack_type_str);
        MPACK_TYPE_STRING_CASE(mpack_type_bin);
        MPACK_TYPE_STRING_CASE(mpack_type_ext);
        MPACK_TYPE_STRING_CASE(mpack_type_array);
        MPACK_TYPE_STRING_CASE(mpack_type_map);
        #undef MPACK_TYPE_STRING_CASE
        default: break;
    }
    mpack_assert(0, "unrecognized type %i", (int)type);
    return "(unknown mpack_type_t)";
    #else
    MPACK_UNUSED(type);
    return "";
    #endif
}

int mpack_tag_cmp(mpack_tag_t left, mpack_tag_t right) {

    // positive numbers may be stored as int; convert to uint
    if (left.type == mpack_type_int && left.v.i >= 0) {
        left.type = mpack_type_uint;
        left.v.u = left.v.i;
    }
    if (right.type == mpack_type_int && right.v.i >= 0) {
        right.type = mpack_type_uint;
        right.v.u = right.v.i;
    }

    if (left.type != right.type)
        return (int)left.type - (int)right.type;

    switch (left.type) {
        case mpack_type_nil:
            return 0;

        case mpack_type_bool:
            return (int)left.v.b - (int)right.v.b;

        case mpack_type_int:
            if (left.v.i == right.v.i)
                return 0;
            return (left.v.i < right.v.i) ? -1 : 1;

        case mpack_type_uint:
            if (left.v.u == right.v.u)
                return 0;
            return (left.v.u < right.v.u) ? -1 : 1;

        case mpack_type_array:
        case mpack_type_map:
            if (left.v.n == right.v.n)
                return 0;
            return (left.v.n < right.v.n) ? -1 : 1;

        case mpack_type_str:
        case mpack_type_bin:
            if (left.v.l == right.v.l)
                return 0;
            return (left.v.l < right.v.l) ? -1 : 1;

        case mpack_type_ext:
            if (left.exttype == right.exttype) {
                if (left.v.l == right.v.l)
                    return 0;
                return (left.v.l < right.v.l) ? -1 : 1;
            }
            return (int)left.exttype - (int)right.exttype;

        // floats should not normally be compared for equality. we compare
        // with memcmp() to silence compiler warnings, but this will return
        // equal if both are NaNs with the same representation (though we may
        // want this, for instance if you are for some bizarre reason using
        // floats as map keys.) i'm not sure what the right thing to
        // do is here. check for NaN first? always return false if the type
        // is float? use operator== and pragmas to silence compiler warning?
        // please send me your suggestions.
        // note also that we don't convert floats to doubles, so when this is
        // used for ordering purposes, all floats are ordered before all
        // doubles.
        case mpack_type_float:
            return mpack_memcmp(&left.v.f, &right.v.f, sizeof(left.v.f));
        case mpack_type_double:
            return mpack_memcmp(&left.v.d, &right.v.d, sizeof(left.v.d));

        default:
            break;
    }
    
    mpack_assert(0, "unrecognized type %i", (int)left.type);
    return false;
}



#if MPACK_READ_TRACKING || MPACK_WRITE_TRACKING

#ifndef MPACK_TRACKING_INITIAL_CAPACITY
// seems like a reasonable number. we grow by doubling, and it only
// needs to be as long as the maximum depth of the message.
#define MPACK_TRACKING_INITIAL_CAPACITY 8
#endif

MPACK_INTERNAL_STATIC mpack_error_t mpack_track_init(mpack_track_t* track) {
    track->count = 0;
    track->capacity = MPACK_TRACKING_INITIAL_CAPACITY;
    track->elements = (mpack_track_element_t*)MPACK_MALLOC(sizeof(mpack_track_element_t) * track->capacity);
    if (track->elements == NULL)
        return mpack_error_memory;
    return mpack_ok;
}

MPACK_INTERNAL_STATIC mpack_error_t mpack_track_grow(mpack_track_t* track) {
    mpack_assert(track->elements, "null track elements!");
    mpack_assert(track->count == track->capacity, "incorrect growing?");

    size_t new_capacity = track->capacity * 2;

    mpack_track_element_t* new_elements = (mpack_track_element_t*)mpack_realloc(track->elements,
            sizeof(mpack_track_element_t) * track->count, sizeof(mpack_track_element_t) * new_capacity);
    if (new_elements == NULL)
        return mpack_error_memory;

    track->elements = new_elements;
    track->capacity = new_capacity;
    return mpack_ok;
}

#endif


/* mpack-writer.c */

#define MPACK_INTERNAL 1

/* #include "mpack-writer.h" */

#if MPACK_WRITER

#if MPACK_WRITE_TRACKING
#define MPACK_WRITER_TRACK(writer, error) mpack_writer_flag_if_error(writer, error)

static inline void mpack_writer_flag_if_error(mpack_writer_t* writer, mpack_error_t error) {
    if (error != mpack_ok)
        mpack_writer_flag_error(writer, error);
}
#else
#define MPACK_WRITER_TRACK(writer, error) MPACK_UNUSED(writer)
#endif

static inline void mpack_writer_track_element(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_element(&writer->track, true));
}

void mpack_writer_init(mpack_writer_t* writer, char* buffer, size_t size) {
    mpack_memset(writer, 0, sizeof(*writer));
    writer->buffer = buffer;
    writer->size = size;
    MPACK_WRITER_TRACK(writer, mpack_track_init(&writer->track));
}

void mpack_writer_init_error(mpack_writer_t* writer, mpack_error_t error) {
    mpack_memset(writer, 0, sizeof(*writer));
    writer->error = error;
}

#ifdef MPACK_MALLOC
typedef struct mpack_growable_writer_t {
    char** target_data;
    size_t* target_size;
} mpack_growable_writer_t;

static void mpack_growable_writer_flush(mpack_writer_t* writer, const char* data, size_t count) {

    // This is an intrusive flush function which modifies the writer's buffer
    // in response to a flush instead of emptying it in order to add more
    // capacity for data. This removes the need to copy data from a fixed buffer
    // into a growable one, improving performance.
    //
    // There are three ways flush can be called:
    //   - flushing the buffer during writing (used is zero, count is all data, data is buffer)
    //   - flushing extra data during writing (used is all flushed data, count is extra data, data is not buffer)
    //   - flushing during teardown (used and count are both all flushed data, data is buffer)
    //
    // We handle these here, making sure used is the total count in all three cases.
    mpack_log("flush size %i used %i data %p buffer %p\n", (int)writer->size, (int)writer->used, data, writer->buffer);

    // if the given data is not the old buffer, we'll need to actually copy it into the buffer
    bool is_extra_data = (data != writer->buffer);

    // if we're flushing all data (used is zero), we should actually grow
    size_t new_size = writer->size;
    if (writer->used == 0 && count != 0)
        new_size *= 2;
    while (new_size < (is_extra_data ? writer->used + count : count))
        new_size *= 2;

    if (new_size > writer->size) {
        mpack_log("flush growing from %i to %i\n", (int)writer->size, (int)new_size);

        char* new_buffer = (char*)mpack_realloc(writer->buffer, count, new_size);
        if (new_buffer == NULL) {
            mpack_writer_flag_error(writer, mpack_error_memory);
            return;
        }

        writer->buffer = new_buffer;
        writer->size = new_size;
    }

    if (is_extra_data) {
        mpack_memcpy(writer->buffer + writer->used, data, count);
        // add our extra data to count
        writer->used += count;
    } else {
        // used is either zero or count; set it to count
        writer->used = count;
    }
}

static void mpack_growable_writer_teardown(mpack_writer_t* writer) {
    mpack_growable_writer_t* growable_writer = (mpack_growable_writer_t*)writer->context;

    if (mpack_writer_error(writer) == mpack_ok) {

        // shrink the buffer to an appropriate size if the data is
        // much smaller than the buffer
        if (writer->used < writer->size / 2) {
            char* buffer = (char*)mpack_realloc(writer->buffer, writer->used, writer->used);
            if (!buffer) {
                MPACK_FREE(writer->buffer);
                mpack_writer_flag_error(writer, mpack_error_memory);
                return;
            }
            writer->buffer = buffer;
            writer->size = writer->used;
        }

        *growable_writer->target_data = writer->buffer;
        *growable_writer->target_size = writer->used;
        writer->buffer = NULL;

    } else if (writer->buffer) {
        MPACK_FREE(writer->buffer);
        writer->buffer = NULL;
    }

    MPACK_FREE(growable_writer);
    writer->context = NULL;
}

void mpack_writer_init_growable(mpack_writer_t* writer, char** target_data, size_t* target_size) {
    *target_data = NULL;
    *target_size = 0;

    mpack_growable_writer_t* growable_writer = (mpack_growable_writer_t*) MPACK_MALLOC(sizeof(mpack_growable_writer_t));
    if (growable_writer == NULL) {
        mpack_writer_init_error(writer, mpack_error_memory);
        return;
    }
    mpack_memset(growable_writer, 0, sizeof(*growable_writer));

    growable_writer->target_data = target_data;
    growable_writer->target_size = target_size;

    size_t capacity = MPACK_BUFFER_SIZE;
    char* buffer = (char*)MPACK_MALLOC(capacity);

    mpack_writer_init(writer, buffer, capacity);
    mpack_writer_set_context(writer, growable_writer);
    mpack_writer_set_flush(writer, mpack_growable_writer_flush);
    mpack_writer_set_teardown(writer, mpack_growable_writer_teardown);
}
#endif

#if MPACK_STDIO
typedef struct mpack_file_writer_t {
    FILE* file;
    char buffer[MPACK_BUFFER_SIZE];
} mpack_file_writer_t;

static void mpack_file_writer_flush(mpack_writer_t* writer, const char* buffer, size_t count) {
    mpack_file_writer_t* file_writer = (mpack_file_writer_t*)writer->context;
    size_t written = fwrite((const void*)buffer, 1, count, file_writer->file);
    if (written != count)
        mpack_writer_flag_error(writer, mpack_error_io);
}

static void mpack_file_writer_teardown(mpack_writer_t* writer) {
    mpack_file_writer_t* file_writer = (mpack_file_writer_t*)writer->context;

    if (file_writer->file) {
        int ret = fclose(file_writer->file);
        file_writer->file = NULL;
        if (ret != 0)
            mpack_writer_flag_error(writer, mpack_error_io);
    }

    MPACK_FREE(file_writer);
}

void mpack_writer_init_file(mpack_writer_t* writer, const char* filename) {
    mpack_file_writer_t* file_writer = (mpack_file_writer_t*) MPACK_MALLOC(sizeof(mpack_file_writer_t));
    if (file_writer == NULL) {
        mpack_writer_init_error(writer, mpack_error_memory);
        return;
    }

    file_writer->file = fopen(filename, "wb");
    if (file_writer->file == NULL) {
        mpack_writer_init_error(writer, mpack_error_io);
        MPACK_FREE(file_writer);
        return;
    }

    mpack_writer_init(writer, file_writer->buffer, sizeof(file_writer->buffer));
    mpack_writer_set_context(writer, file_writer);
    mpack_writer_set_flush(writer, mpack_file_writer_flush);
    mpack_writer_set_teardown(writer, mpack_file_writer_teardown);
}
#endif

void mpack_writer_flag_error(mpack_writer_t* writer, mpack_error_t error) {
    mpack_log("writer %p setting error %i: %s\n", writer, (int)error, mpack_error_to_string(error));

    if (writer->error == mpack_ok) {
        writer->error = error;
        #if MPACK_SETJMP
        if (writer->jump_env)
            longjmp(*writer->jump_env, 1);
        #endif
    }
}

static void mpack_write_native_big(mpack_writer_t* writer, const char* p, size_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;
    mpack_log("big write for %i bytes from %p, %i space left in buffer\n",
            (int)count, p, (int)(writer->size - writer->used));
    mpack_assert(count > writer->size - writer->used,
            "big write requested for %i bytes, but there is %i available "
            "space in buffer. call mpack_write_native() instead",
            (int)count, (int)(writer->size - writer->used));

    // we'll need a flush function
    if (!writer->flush) {
        mpack_writer_flag_error(writer, mpack_error_io);
        return;
    }

    // we assume that the flush function is orders of magnitude slower
    // than memcpy(), so we fill the buffer up first to try to flush as
    // infrequently as possible.
    
    // fill the remaining space in the buffer
    size_t n = writer->size - writer->used;
    if (count < n)
        n = count;
    mpack_memcpy(writer->buffer + writer->used, p, n);
    writer->used += n;
    p += n;
    count -= n;
    if (count == 0)
        return;

    // flush the buffer
    size_t used = writer->used;
    writer->used = 0;
    writer->flush(writer, writer->buffer, used);
    if (mpack_writer_error(writer) != mpack_ok)
        return;

    // note that an intrusive flush function (such as mpack_growable_writer_flush())
    // may have changed size and/or reset used to a non-zero value. we treat both as
    // though they may have changed, and there may still be data in the buffer.

    // flush the extra data directly if it doesn't fit in the buffer
    if (count > writer->size - writer->used) {
        writer->flush(writer, p, count);
        if (mpack_writer_error(writer) != mpack_ok)
            return;
    } else {
        mpack_memcpy(writer->buffer + writer->used, p, count);
        writer->used += count;
    }
}

static inline void mpack_write_native(mpack_writer_t* writer, const char* p, size_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;
    if (writer->size - writer->used < count) {
        mpack_write_native_big(writer, p, count);
    } else {
        mpack_memcpy(writer->buffer + writer->used, p, count);
        writer->used += count;
    }
}

MPACK_ALWAYS_INLINE void mpack_store_native_u8_at(char* p, uint8_t val) {
    uint8_t* u = (uint8_t*)p;
    u[0] = val;
}

MPACK_ALWAYS_INLINE void mpack_store_native_u16_at(char* p, uint16_t val) {
    uint8_t* u = (uint8_t*)p;
    u[0] = (uint8_t)((val >> 8) & 0xFF);
    u[1] = (uint8_t)( val       & 0xFF);
}

MPACK_ALWAYS_INLINE void mpack_store_native_u32_at(char* p, uint32_t val) {
    uint8_t* u = (uint8_t*)p;
    u[0] = (uint8_t)((val >> 24) & 0xFF);
    u[1] = (uint8_t)((val >> 16) & 0xFF);
    u[2] = (uint8_t)((val >>  8) & 0xFF);
    u[3] = (uint8_t)( val        & 0xFF);
}

MPACK_ALWAYS_INLINE void mpack_store_native_u64_at(char* p, uint64_t val) {
    uint8_t* u = (uint8_t*)p;
    u[0] = (uint8_t)((val >> 56) & 0xFF);
    u[1] = (uint8_t)((val >> 48) & 0xFF);
    u[2] = (uint8_t)((val >> 40) & 0xFF);
    u[3] = (uint8_t)((val >> 32) & 0xFF);
    u[4] = (uint8_t)((val >> 24) & 0xFF);
    u[5] = (uint8_t)((val >> 16) & 0xFF);
    u[6] = (uint8_t)((val >>  8) & 0xFF);
    u[7] = (uint8_t)( val        & 0xFF);
}

static inline void mpack_write_native_u8(mpack_writer_t* writer, uint8_t val) {
    if (writer->size - writer->used >= sizeof(val)) {
        mpack_store_native_u8_at(writer->buffer + writer->used, val);
        writer->used += sizeof(val);
    } else {
        char c[sizeof(val)];
        mpack_store_native_u8_at(c, val);
        mpack_write_native_big(writer, c, sizeof(c));
    }
}

static inline void mpack_write_native_u16(mpack_writer_t* writer, uint16_t val) {
    if (writer->size - writer->used >= sizeof(val)) {
        mpack_store_native_u16_at(writer->buffer + writer->used, val);
        writer->used += sizeof(val);
    } else {
        char c[sizeof(val)];
        mpack_store_native_u16_at(c, val);
        mpack_write_native_big(writer, c, sizeof(c));
    }
}

static inline void mpack_write_native_u32(mpack_writer_t* writer, uint32_t val) {
    if (writer->size - writer->used >= sizeof(val)) {
        mpack_store_native_u32_at(writer->buffer + writer->used, val);
        writer->used += sizeof(val);
    } else {
        char c[sizeof(val)];
        mpack_store_native_u32_at(c, val);
        mpack_write_native_big(writer, c, sizeof(c));
    }
}

static inline void mpack_write_native_u64(mpack_writer_t* writer, uint64_t val) {
    if (writer->size - writer->used >= sizeof(val)) {
        mpack_store_native_u64_at(writer->buffer + writer->used, val);
        writer->used += sizeof(val);
    } else {
        char c[sizeof(val)];
        mpack_store_native_u64_at(c, val);
        mpack_write_native_big(writer, c, sizeof(c));
    }
}

static inline void mpack_write_native_i8  (mpack_writer_t* writer,  int8_t  val) {mpack_write_native_u8  (writer, (uint8_t )val);}
static inline void mpack_write_native_i16 (mpack_writer_t* writer,  int16_t val) {mpack_write_native_u16 (writer, (uint16_t)val);}
static inline void mpack_write_native_i32 (mpack_writer_t* writer,  int32_t val) {mpack_write_native_u32 (writer, (uint32_t)val);}
static inline void mpack_write_native_i64 (mpack_writer_t* writer,  int64_t val) {mpack_write_native_u64 (writer, (uint64_t)val);}


static inline void mpack_write_native_float(mpack_writer_t* writer, float value) {
    union {
        float f;
        uint32_t i;
    } u;
    u.f = value;
    mpack_write_native_u32(writer, u.i);
}

static inline void mpack_write_native_double(mpack_writer_t* writer, double value) {
    union {
        double d;
        uint64_t i;
    } u;
    u.d = value;
    mpack_write_native_u64(writer, u.i);
}

mpack_error_t mpack_writer_destroy(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_destroy(&writer->track, false));

    // flush any outstanding data
    if (mpack_writer_error(writer) == mpack_ok && writer->used != 0 && writer->flush != NULL) {
        writer->flush(writer, writer->buffer, writer->used);
        writer->flush = NULL;
    }

    if (writer->teardown) {
        writer->teardown(writer);
        writer->teardown = NULL;
    }

    #if MPACK_SETJMP
    if (writer->jump_env)
        MPACK_FREE(writer->jump_env);
    writer->jump_env = NULL;
    #endif

    return writer->error;
}

void mpack_writer_destroy_cancel(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_destroy(&writer->track, true));

    if (writer->teardown)
        writer->teardown(writer);
    writer->teardown = NULL;

    #if MPACK_SETJMP
    if (writer->jump_env)
        MPACK_FREE(writer->jump_env);
    writer->jump_env = NULL;
    #endif
}

void mpack_write_tag(mpack_writer_t* writer, mpack_tag_t value) {
    mpack_writer_track_element(writer);

    switch (value.type) {

        case mpack_type_nil:    mpack_write_nil   (writer);          break;

        case mpack_type_bool:   mpack_write_bool  (writer, value.v.b); break;
        case mpack_type_float:  mpack_write_float (writer, value.v.f); break;
        case mpack_type_double: mpack_write_double(writer, value.v.d); break;
        case mpack_type_int:    mpack_write_int   (writer, value.v.i); break;
        case mpack_type_uint:   mpack_write_uint  (writer, value.v.u); break;

        case mpack_type_str: mpack_start_str(writer, value.v.l); break;
        case mpack_type_bin: mpack_start_bin(writer, value.v.l); break;
        case mpack_type_ext: mpack_start_ext(writer, value.exttype, value.v.l); break;

        case mpack_type_array: mpack_start_array(writer, value.v.n); break;
        case mpack_type_map:   mpack_start_map(writer, value.v.n);   break;

        default:
            mpack_assert(0, "unrecognized type %i", (int)value.type);
            break;
    }
}

void mpack_write_u8(mpack_writer_t* writer, uint8_t value) {
    mpack_writer_track_element(writer);
    if (value <= 0x7f) {
        mpack_write_native_u8(writer, (uint8_t)value);
    } else {
        mpack_write_native_u8(writer, 0xcc);
        mpack_write_native_u8(writer, (uint8_t)value);
    }
}

void mpack_write_u16(mpack_writer_t* writer, uint16_t value) {
    mpack_writer_track_element(writer);
    if (value <= 0x7f) {
        mpack_write_native_u8(writer, (uint8_t)value);
    } else if (value <= UINT8_MAX) {
        mpack_write_native_u8(writer, 0xcc);
        mpack_write_native_u8(writer, (uint8_t)value);
    } else {
        mpack_write_native_u8(writer, 0xcd);
        mpack_write_native_u16(writer, value);
    }
}

void mpack_write_u32(mpack_writer_t* writer, uint32_t value) {
    mpack_writer_track_element(writer);
    if (value <= 0x7f) {
        mpack_write_native_u8(writer, (uint8_t)value);
    } else if (value <= UINT8_MAX) {
        mpack_write_native_u8(writer, 0xcc);
        mpack_write_native_u8(writer, (uint8_t)value);
    } else if (value <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xcd);
        mpack_write_native_u16(writer, (uint16_t)value);
    } else {
        mpack_write_native_u8(writer, 0xce);
        mpack_write_native_u32(writer, value);
    }
}

void mpack_write_u64(mpack_writer_t* writer, uint64_t value) {
    mpack_writer_track_element(writer);
    if (value <= 0x7f) {
        mpack_write_native_u8(writer, (uint8_t)value);
    } else if (value <= UINT8_MAX) {
        mpack_write_native_u8(writer, 0xcc);
        mpack_write_native_u8(writer, (uint8_t)value);
    } else if (value <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xcd);
        mpack_write_native_u16(writer, (uint16_t)value);
    } else if (value <= UINT32_MAX) {
        mpack_write_native_u8(writer, 0xce);
        mpack_write_native_u32(writer, (uint32_t)value);
    } else {
        mpack_write_native_u8(writer, 0xcf);
        mpack_write_native_u64(writer, value);
    }
}

void mpack_write_i8(mpack_writer_t* writer, int8_t value) {

    // write any non-negative number as a uint
    if (value >= 0) {
        mpack_write_u8(writer, (uint8_t)value);
        return;
    }

    mpack_writer_track_element(writer);
    if (value >= -32) {
        mpack_write_native_i8(writer, (int8_t)0xe0 | (int8_t)value); // TODO: remove this (compatibility/1.1 difference?)
    } else {
        mpack_write_native_u8(writer, 0xd0);
        mpack_write_native_i8(writer, value);
    }

}

void mpack_write_i16(mpack_writer_t* writer, int16_t value) {

    // write any non-negative number as a uint
    if (value >= 0) {
        mpack_write_u16(writer, (uint16_t)value);
        return;
    }

    mpack_writer_track_element(writer);
    if (value >= -32) {
        mpack_write_native_i8(writer, (int8_t)0xe0 | (int8_t)value); // TODO: remove this (compatibility/1.1 difference?)
    } else if (value >= INT8_MIN) {
        mpack_write_native_u8(writer, 0xd0);
        mpack_write_native_i8(writer, (int8_t)value);
    } else {
        mpack_write_native_u8(writer, 0xd1);
        mpack_write_native_i16(writer, value);
    }

}

void mpack_write_i32(mpack_writer_t* writer, int32_t value) {

    // write any non-negative number as a uint
    if (value >= 0) {
        mpack_write_u32(writer, (uint32_t)value);
        return;
    }

    mpack_writer_track_element(writer);
    if (value >= -32) {
        mpack_write_native_i8(writer, (int8_t)0xe0 | (int8_t)value); // TODO: remove this (compatibility/1.1 difference?)
    } else if (value >= INT8_MIN) {
        mpack_write_native_u8(writer, 0xd0);
        mpack_write_native_i8(writer, (int8_t)value);
    } else if (value >= INT16_MIN) {
        mpack_write_native_u8(writer, 0xd1);
        mpack_write_native_i16(writer, (int16_t)value);
    } else {
        mpack_write_native_u8(writer, 0xd2);
        mpack_write_native_i32(writer, value);
    }

}

void mpack_write_i64(mpack_writer_t* writer, int64_t value) {

    // write any non-negative number as a uint
    if (value >= 0) {
        mpack_write_u64(writer, (uint64_t)value);
        return;
    }

    mpack_writer_track_element(writer);
    if (value >= -32) {
        mpack_write_native_i8(writer, (int8_t)0xe0 | (int8_t)value); // TODO: remove this (compatibility/1.1 difference?)
    } else if (value >= INT8_MIN) {
        mpack_write_native_u8(writer, 0xd0);
        mpack_write_native_i8(writer, (int8_t)value);
    } else if (value >= INT16_MIN) {
        mpack_write_native_u8(writer, 0xd1);
        mpack_write_native_i16(writer, (int16_t)value);
    } else if (value >= INT32_MIN) {
        mpack_write_native_u8(writer, 0xd2);
        mpack_write_native_i32(writer, (int32_t)value);
    } else {
        mpack_write_native_u8(writer, 0xd3);
        mpack_write_native_i64(writer, value);
    }

}

void mpack_write_bool(mpack_writer_t* writer, bool value) {
    mpack_writer_track_element(writer);
    mpack_write_native_u8(writer, (uint8_t)(0xc2 | (value ? 1 : 0)));
}

void mpack_write_true(mpack_writer_t* writer) {
    mpack_writer_track_element(writer);
    mpack_write_native_u8(writer, (uint8_t)0xc3);
}

void mpack_write_false(mpack_writer_t* writer) {
    mpack_writer_track_element(writer);
    mpack_write_native_u8(writer, (uint8_t)0xc2);
}

void mpack_write_nil(mpack_writer_t* writer) {
    mpack_writer_track_element(writer);
    mpack_write_native_u8(writer, 0xc0);
}

void mpack_write_float(mpack_writer_t* writer, float value) {
    mpack_writer_track_element(writer);
    mpack_write_native_u8(writer, 0xca);
    mpack_write_native_float(writer, value);
}

void mpack_write_double(mpack_writer_t* writer, double value) {
    mpack_writer_track_element(writer);
    mpack_write_native_u8(writer, 0xcb);
    mpack_write_native_double(writer, value);
}

#if MPACK_WRITE_TRACKING
void mpack_finish_array(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_pop(&writer->track, mpack_type_array));
}

void mpack_finish_map(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_pop(&writer->track, mpack_type_map));
}

void mpack_finish_str(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_pop(&writer->track, mpack_type_str));
}

void mpack_finish_bin(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_pop(&writer->track, mpack_type_bin));
}

void mpack_finish_ext(mpack_writer_t* writer) {
    MPACK_WRITER_TRACK(writer, mpack_track_pop(&writer->track, mpack_type_ext));
}

void mpack_finish_type(mpack_writer_t* writer, mpack_type_t type) {
    MPACK_WRITER_TRACK(writer, mpack_track_pop(&writer->track, type));
}
#endif

void mpack_start_array(mpack_writer_t* writer, uint32_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;

    mpack_writer_track_element(writer);
    if (count <= 15) {
        mpack_write_native_u8(writer, (uint8_t)(0x90 | count));
    } else if (count <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xdc);
        mpack_write_native_u16(writer, (uint16_t)count);
    } else {
        mpack_write_native_u8(writer, 0xdd);
        mpack_write_native_u32(writer, count);
    }

    MPACK_WRITER_TRACK(writer, mpack_track_push(&writer->track, mpack_type_array, count));
}

void mpack_start_map(mpack_writer_t* writer, uint32_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;

    mpack_writer_track_element(writer);
    if (count <= 15) {
        mpack_write_native_u8(writer, (uint8_t)(0x80 | count));
    } else if (count <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xde);
        mpack_write_native_u16(writer, (uint16_t)count);
    } else {
        mpack_write_native_u8(writer, 0xdf);
        mpack_write_native_u32(writer, count);
    }

    MPACK_WRITER_TRACK(writer, mpack_track_push(&writer->track, mpack_type_map, count));
}

void mpack_start_str(mpack_writer_t* writer, uint32_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;

    mpack_writer_track_element(writer);
    if (count <= 31) {
        mpack_write_native_u8(writer, (uint8_t)(0xa0 | count));
    } else if (count <= UINT8_MAX) {
        // TODO: THIS NOT AVAILABLE IN COMPATIBILITY MODE?? was not in 1.0?
        mpack_write_native_u8(writer, 0xd9);
        mpack_write_native_u8(writer, (uint8_t)count);
    } else if (count <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xda);
        mpack_write_native_u16(writer, (uint16_t)count);
    } else {
        mpack_write_native_u8(writer, 0xdb);
        mpack_write_native_u32(writer, count);
    }

    MPACK_WRITER_TRACK(writer, mpack_track_push(&writer->track, mpack_type_str, count));
}

void mpack_start_bin(mpack_writer_t* writer, uint32_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;

    mpack_writer_track_element(writer);
    if (count <= UINT8_MAX) {
        mpack_write_native_u8(writer, 0xc4);
        mpack_write_native_u8(writer, (uint8_t)count);
    } else if (count <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xc5);
        mpack_write_native_u16(writer, (uint16_t)count);
    } else {
        mpack_write_native_u8(writer, 0xc6);
        mpack_write_native_u32(writer, count);
    }

    MPACK_WRITER_TRACK(writer, mpack_track_push(&writer->track, mpack_type_bin, count));
}

void mpack_start_ext(mpack_writer_t* writer, int8_t exttype, uint32_t count) {
    if (mpack_writer_error(writer) != mpack_ok)
        return;

    // TODO: fail if compatibility mode

    mpack_writer_track_element(writer);
    if (count == 1) {
        mpack_write_native_u8(writer, 0xd4);
        mpack_write_native_i8(writer, exttype);
    } else if (count == 2) {
        mpack_write_native_u8(writer, 0xd5);
        mpack_write_native_i8(writer, exttype);
    } else if (count == 4) {
        mpack_write_native_u8(writer, 0xd6);
        mpack_write_native_i8(writer, exttype);
    } else if (count == 8) {
        mpack_write_native_u8(writer, 0xd7);
        mpack_write_native_i8(writer, exttype);
    } else if (count == 16) {
        mpack_write_native_u8(writer, 0xd8);
        mpack_write_native_i8(writer, exttype);
    } else if (count <= UINT8_MAX) {
        mpack_write_native_u8(writer, 0xc7);
        mpack_write_native_u8(writer, (uint8_t)count);
        mpack_write_native_i8(writer, exttype);
    } else if (count <= UINT16_MAX) {
        mpack_write_native_u8(writer, 0xc8);
        mpack_write_native_u16(writer, (uint16_t)count);
        mpack_write_native_i8(writer, exttype);
    } else {
        mpack_write_native_u8(writer, 0xc9);
        mpack_write_native_u32(writer, count);
        mpack_write_native_i8(writer, exttype);
    }

    MPACK_WRITER_TRACK(writer, mpack_track_push(&writer->track, mpack_type_ext, count));
}

void mpack_write_str(mpack_writer_t* writer, const char* data, uint32_t count) {
    mpack_start_str(writer, count);
    mpack_write_bytes(writer, data, count);
    mpack_finish_str(writer);
}

void mpack_write_bin(mpack_writer_t* writer, const char* data, uint32_t count) {
    mpack_start_bin(writer, count);
    mpack_write_bytes(writer, data, count);
    mpack_finish_bin(writer);
}

void mpack_write_ext(mpack_writer_t* writer, int8_t exttype, const char* data, uint32_t count) {
    mpack_start_ext(writer, exttype, count);
    mpack_write_bytes(writer, data, count);
    mpack_finish_ext(writer);
}

void mpack_write_bytes(mpack_writer_t* writer, const char* data, size_t count) {
    MPACK_WRITER_TRACK(writer, mpack_track_bytes(&writer->track, false, count));
    mpack_write_native(writer, data, count);
}

void mpack_write_cstr(mpack_writer_t* writer, const char* str) {
    size_t len = mpack_strlen(str);
    if (len > UINT32_MAX)
        mpack_writer_flag_error(writer, mpack_error_invalid);
    mpack_write_str(writer, str, (uint32_t)len);
}

#endif


/* mpack-reader.c */

#define MPACK_INTERNAL 1

/* #include "mpack-reader.h" */

#if MPACK_READER

void mpack_reader_init(mpack_reader_t* reader, char* buffer, size_t size, size_t count) {
    mpack_memset(reader, 0, sizeof(*reader));
    reader->buffer = buffer;
    reader->size = size;
    reader->left = count;
    MPACK_READER_TRACK(reader, mpack_track_init(&reader->track));
}

void mpack_reader_init_error(mpack_reader_t* reader, mpack_error_t error) {
    mpack_memset(reader, 0, sizeof(*reader));
    reader->error = error;
}

void mpack_reader_init_data(mpack_reader_t* reader, const char* data, size_t count) {
    mpack_memset(reader, 0, sizeof(*reader));
    reader->left = count;

    // unfortunately we have to cast away the const to store the buffer,
    // but we won't be modifying it because there's no fill function.
    // the buffer size is left at 0 to ensure no fill function can be
    // set or used (see mpack_reader_set_fill().)
    #ifdef __cplusplus
    reader->buffer = const_cast<char*>(data);
    #else
    reader->buffer = (char*)data;
    #endif

    MPACK_READER_TRACK(reader, mpack_track_init(&reader->track));
}

#if MPACK_STDIO
typedef struct mpack_file_reader_t {
    FILE* file;
    char buffer[MPACK_BUFFER_SIZE];
} mpack_file_reader_t;

static size_t mpack_file_reader_fill(mpack_reader_t* reader, char* buffer, size_t count) {
    mpack_file_reader_t* file_reader = (mpack_file_reader_t*)reader->context;
    return fread((void*)buffer, 1, count, file_reader->file);
}

static void mpack_file_reader_teardown(mpack_reader_t* reader) {
    mpack_file_reader_t* file_reader = (mpack_file_reader_t*)reader->context;

    if (file_reader->file) {
        int ret = fclose(file_reader->file);
        file_reader->file = NULL;
        if (ret != 0)
            mpack_reader_flag_error(reader, mpack_error_io);
    }

    MPACK_FREE(file_reader);
}

void mpack_reader_init_file(mpack_reader_t* reader, const char* filename) {
    mpack_file_reader_t* file_reader = (mpack_file_reader_t*) MPACK_MALLOC(sizeof(mpack_file_reader_t));
    if (file_reader == NULL) {
        mpack_reader_init_error(reader, mpack_error_memory);
        return;
    }

    file_reader->file = fopen(filename, "rb");
    if (file_reader->file == NULL) {
        mpack_reader_init_error(reader, mpack_error_io);
        MPACK_FREE(file_reader);
        return;
    }

    mpack_reader_init(reader, file_reader->buffer, sizeof(file_reader->buffer), 0);
    mpack_reader_set_context(reader, file_reader);
    mpack_reader_set_fill(reader, mpack_file_reader_fill);
    mpack_reader_set_teardown(reader, mpack_file_reader_teardown);
}
#endif

mpack_error_t mpack_reader_destroy_impl(mpack_reader_t* reader, bool cancel) {
    MPACK_UNUSED(cancel);
    MPACK_READER_TRACK(reader, mpack_track_destroy(&reader->track, cancel));

    if (reader->teardown)
        reader->teardown(reader);
    reader->teardown = NULL;

    #if MPACK_SETJMP
    if (reader->jump_env)
        MPACK_FREE(reader->jump_env);
    reader->jump_env = NULL;
    #endif

    return reader->error;
}

void mpack_reader_destroy_cancel(mpack_reader_t* reader) {
    mpack_reader_destroy_impl(reader, true);
}

mpack_error_t mpack_reader_destroy(mpack_reader_t* reader) {
    return mpack_reader_destroy_impl(reader, false);
}

size_t mpack_reader_remaining(mpack_reader_t* reader, const char** data) {
    MPACK_READER_TRACK(reader, mpack_track_check_empty(&reader->track));
    if (data)
        *data = reader->buffer + reader->pos;
    return reader->left;
}

void mpack_reader_flag_error(mpack_reader_t* reader, mpack_error_t error) {
    mpack_log("reader %p setting error %i: %s\n", reader, (int)error, mpack_error_to_string(error));

    if (reader->error == mpack_ok) {
        reader->error = error;
        #if MPACK_SETJMP
        if (reader->jump_env)
            longjmp(*reader->jump_env, 1);
        #endif
    }
}

// A helper to call the reader fill function. This makes sure it's
// implemented and guards against overflow in case it returns -1.
static inline size_t mpack_fill(mpack_reader_t* reader, char* p, size_t count) {
    if (!reader->fill)
        return 0;
    size_t ret = reader->fill(reader, p, count);
    if (ret == ((size_t)(-1)))
        return 0;
    return ret;
}

// Reads count bytes into p. Used when there are not enough bytes
// left in the buffer to satisfy a read.
void mpack_read_native_big(mpack_reader_t* reader, char* p, size_t count) {
    if (mpack_reader_error(reader) != mpack_ok) {
        mpack_memset(p, 0, count);
        return;
    }

    mpack_log("big read for %i bytes into %p, %i left in buffer, buffer size %i\n",
            (int)count, p, (int)reader->left, (int)reader->size);

    if (count <= reader->left) {
        mpack_assert(0,
                "big read requested for %i bytes, but there are %i bytes "
                "left in buffer. call mpack_read_native() instead",
                (int)count, (int)reader->left);
        mpack_reader_flag_error(reader, mpack_error_bug);
        mpack_memset(p, 0, count);
        return;
    }

    if (reader->size == 0) {
        // somewhat debatable what error should be returned here. when
        // initializing a reader with an in-memory buffer it's not
        // necessarily a bug if the data is blank; it might just have
        // been truncated to zero. for this reason we return the same
        // error as if the data was truncated.
        mpack_reader_flag_error(reader, mpack_error_io);
        mpack_memset(p, 0, count);
        return;
    }

    // flush what's left of the buffer
    if (reader->left > 0) {
        mpack_log("flushing %i bytes remaining in buffer\n", (int)reader->left);
        mpack_memcpy(p, reader->buffer + reader->pos, reader->left);
        count -= reader->left;
        p += reader->left;
        reader->pos += reader->left;
        reader->left = 0;
    }

    // we read only in multiples of the buffer size. read the middle portion, if any
    size_t middle = count - (count % reader->size);
    if (middle > 0) {
        mpack_log("reading %i bytes in middle\n", (int)middle);
        if (mpack_fill(reader, p, middle) < middle) {
            mpack_reader_flag_error(reader, mpack_error_io);
            mpack_memset(p, 0, count);
            return;
        }
        count -= middle;
        p += middle;
        if (count == 0)
            return;
    }

    // fill the buffer
    reader->pos = 0;
    reader->left = mpack_fill(reader, reader->buffer, reader->size);
    mpack_log("filled %i bytes into buffer\n", (int)reader->left);
    if (reader->left < count) {
        mpack_reader_flag_error(reader, mpack_error_io);
        mpack_memset(p, 0, count);
        return;
    }

    // serve the remainder
    mpack_log("serving %i remaining bytes from %p to %p\n", (int)count, reader->buffer+reader->pos,p);
    mpack_memcpy(p, reader->buffer + reader->pos, count);
    reader->pos += count;
    reader->left -= count;
}

void mpack_skip_bytes(mpack_reader_t* reader, size_t count) {
    // TODO: This is currently very slow, potentially even slower than just
    // reading the data. Skip needs to be implemented properly.
    char c[128];
    size_t i = 0;
    while (i < count && mpack_reader_error(reader) == mpack_ok) {
        size_t amount = ((count - i) > sizeof(c)) ? sizeof(c) : (count - i);
        mpack_read_bytes(reader, c, amount);
        i += amount;
    }
}

void mpack_read_bytes(mpack_reader_t* reader, char* p, size_t count) {
    mpack_reader_track_bytes(reader, count);
    mpack_read_native(reader, p, count);
}

// internal inplace reader for when it straddles the end of the buffer
// this is split out to inline the common case, although this isn't done
// right now because we can't inline tracking yet
static const char* mpack_read_bytes_inplace_big(mpack_reader_t* reader, size_t count) {

    // we should only arrive here from inplace straddle; this should already be checked
    mpack_assert(mpack_reader_error(reader) == mpack_ok, "already in error state? %s",
            mpack_error_to_string(mpack_reader_error(reader)));
    mpack_assert(reader->left < count, "already enough bytes in buffer: %i left, %i count", (int)reader->left, (int)count);

    // we'll need a fill function to get more data
    if (!reader->fill) {
        mpack_reader_flag_error(reader, mpack_error_io);
        return NULL;
    }

    // make sure the buffer is big enough to actually fit the data
    if (count > reader->size) {
        mpack_reader_flag_error(reader, mpack_error_too_big);
        return NULL;
    }

    // shift the remaining data back to the start and fill the buffer back up
    mpack_memmove(reader->buffer, reader->buffer + reader->pos, reader->left);
    reader->pos = 0;
    reader->left += mpack_fill(reader, reader->buffer + reader->left, reader->size - reader->left);
    if (reader->left < count) {
        mpack_reader_flag_error(reader, mpack_error_io);
        return NULL;
    }
    reader->pos += count;
    reader->left -= count;
    return reader->buffer;
}

const char* mpack_read_bytes_inplace(mpack_reader_t* reader, size_t count) {
    if (mpack_reader_error(reader) != mpack_ok)
        return NULL;

    mpack_reader_track_bytes(reader, count);

    // if we have enough bytes already in the buffer, we can return it directly.
    if (reader->left >= count) {
        reader->pos += count;
        reader->left -= count;
        return reader->buffer + reader->pos - count;
    }

    return mpack_read_bytes_inplace_big(reader, count);
}

mpack_tag_t mpack_read_tag(mpack_reader_t* reader) {
    mpack_tag_t var;
    mpack_memset(&var, 0, sizeof(var));
    var.type = mpack_type_nil;

    // get the type
    uint8_t type = mpack_read_native_u8(reader);
    if (mpack_reader_error(reader))
        return var;
    mpack_reader_track_element(reader);

    // unfortunately, by far the fastest way to parse a tag is to switch
    // on the first byte, and to explicitly list every possible byte. so for
    // infix types, the list of cases is quite large. the compiler optimizes
    // this nicely (and it takes very little space.)
    switch (type) {

        // positive fixnum
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
        case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
        case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
        case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
            var.type = mpack_type_uint;
            var.v.u = type;
            return var;

        // negative fixnum
        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
            var.type = mpack_type_int;
            var.v.i = (int8_t)type;
            return var;

        // fixmap
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
            var.type = mpack_type_map;
            var.v.n = type & ~0xf0;
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_map, var.v.n));
            return var;

        // fixarray
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
            var.type = mpack_type_array;
            var.v.n = type & ~0xf0;
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_array, var.v.n));
            return var;

        // fixstr
        case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
            var.type = mpack_type_str;
            var.v.l = type & ~0xe0;
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_str, var.v.l));
            return var;

        // nil
        case 0xc0:
            var.type = mpack_type_nil;
            return var;

        // bool
        case 0xc2: case 0xc3:
            var.type = mpack_type_bool;
            var.v.b = type & 1;
            return var;

        // bin8
        case 0xc4:
            var.type = mpack_type_bin;
            var.v.l = mpack_read_native_u8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_bin, var.v.l));
            return var;

        // bin16
        case 0xc5:
            var.type = mpack_type_bin;
            var.v.l = mpack_read_native_u16(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_bin, var.v.l));
            return var;

        // bin32
        case 0xc6:
            var.type = mpack_type_bin;
            var.v.l = mpack_read_native_u32(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_bin, var.v.l));
            return var;

        // ext8
        case 0xc7:
            var.type = mpack_type_ext;
            var.v.l = mpack_read_native_u8(reader);
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // ext16
        case 0xc8:
            var.type = mpack_type_ext;
            var.v.l = mpack_read_native_u16(reader);
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // ext32
        case 0xc9:
            var.type = mpack_type_ext;
            var.v.l = mpack_read_native_u32(reader);
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // float
        case 0xca:
            var.type = mpack_type_float;
            var.v.f = mpack_read_native_float(reader);
            return var;

        // double
        case 0xcb:
            var.type = mpack_type_double;
            var.v.d = mpack_read_native_double(reader);
            return var;

        // uint8
        case 0xcc:
            var.type = mpack_type_uint;
            var.v.u = mpack_read_native_u8(reader);
            return var;

        // uint16
        case 0xcd:
            var.type = mpack_type_uint;
            var.v.u = mpack_read_native_u16(reader);
            return var;

        // uint32
        case 0xce:
            var.type = mpack_type_uint;
            var.v.u = mpack_read_native_u32(reader);
            return var;

        // uint64
        case 0xcf:
            var.type = mpack_type_uint;
            var.v.u = mpack_read_native_u64(reader);
            return var;

        // int8
        case 0xd0:
            var.type = mpack_type_int;
            var.v.i = mpack_read_native_i8(reader);
            return var;

        // int16
        case 0xd1:
            var.type = mpack_type_int;
            var.v.i = mpack_read_native_i16(reader);
            return var;

        // int32
        case 0xd2:
            var.type = mpack_type_int;
            var.v.i = mpack_read_native_i32(reader);
            return var;

        // int64
        case 0xd3:
            var.type = mpack_type_int;
            var.v.i = mpack_read_native_i64(reader);
            return var;

        // fixext1
        case 0xd4:
            var.type = mpack_type_ext;
            var.v.l = 1;
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // fixext2
        case 0xd5:
            var.type = mpack_type_ext;
            var.v.l = 2;
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // fixext4
        case 0xd6:
            var.type = mpack_type_ext;
            var.v.l = 4;
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // fixext8
        case 0xd7:
            var.type = mpack_type_ext;
            var.v.l = 8;
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // fixext16
        case 0xd8:
            var.type = mpack_type_ext;
            var.v.l = 16;
            var.exttype = mpack_read_native_i8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_ext, var.v.l));
            return var;

        // str8
        case 0xd9:
            var.type = mpack_type_str;
            var.v.l = mpack_read_native_u8(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_str, var.v.l));
            return var;

        // str16
        case 0xda:
            var.type = mpack_type_str;
            var.v.l = mpack_read_native_u16(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_str, var.v.l));
            return var;

        // str32
        case 0xdb:
            var.type = mpack_type_str;
            var.v.l = mpack_read_native_u32(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_str, var.v.l));
            return var;

        // array16
        case 0xdc:
            var.type = mpack_type_array;
            var.v.n = mpack_read_native_u16(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_array, var.v.n));
            return var;

        // array32
        case 0xdd:
            var.type = mpack_type_array;
            var.v.n = mpack_read_native_u32(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_array, var.v.n));
            return var;

        // map16
        case 0xde:
            var.type = mpack_type_map;
            var.v.n = mpack_read_native_u16(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_map, var.v.n));
            return var;

        // map32
        case 0xdf:
            var.type = mpack_type_map;
            var.v.n = mpack_read_native_u32(reader);
            MPACK_READER_TRACK(reader, mpack_track_push(&reader->track, mpack_type_map, var.v.n));
            return var;

        // reserved
        case 0xc1:
            break;
    }

    // unrecognized type
    mpack_reader_flag_error(reader, mpack_error_invalid);
    return var;
}

void mpack_discard(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (mpack_reader_error(reader))
        return;
    switch (var.type) {
        case mpack_type_str:
            mpack_skip_bytes(reader, var.v.l);
            mpack_done_str(reader);
            break;
        case mpack_type_bin:
            mpack_skip_bytes(reader, var.v.l);
            mpack_done_bin(reader);
            break;
        case mpack_type_ext:
            mpack_skip_bytes(reader, var.v.l);
            mpack_done_ext(reader);
            break;
        case mpack_type_array: {
            for (; var.v.n > 0; --var.v.n) {
                mpack_discard(reader);
                if (mpack_reader_error(reader))
                    break;
            }
            break;
        }
        case mpack_type_map: {
            for (; var.v.n > 0; --var.v.n) {
                mpack_discard(reader);
                mpack_discard(reader);
                if (mpack_reader_error(reader))
                    break;
            }
            break;
        }
        default:
            break;
    }
}

#if MPACK_READ_TRACKING
void mpack_done_array(mpack_reader_t* reader) {
    MPACK_READER_TRACK(reader, mpack_track_pop(&reader->track, mpack_type_array));
}

void mpack_done_map(mpack_reader_t* reader) {
    MPACK_READER_TRACK(reader, mpack_track_pop(&reader->track, mpack_type_map));
}

void mpack_done_str(mpack_reader_t* reader) {
    MPACK_READER_TRACK(reader, mpack_track_pop(&reader->track, mpack_type_str));
}

void mpack_done_bin(mpack_reader_t* reader) {
    MPACK_READER_TRACK(reader, mpack_track_pop(&reader->track, mpack_type_bin));
}

void mpack_done_ext(mpack_reader_t* reader) {
    MPACK_READER_TRACK(reader, mpack_track_pop(&reader->track, mpack_type_ext));
}

void mpack_done_type(mpack_reader_t* reader, mpack_type_t type) {
    MPACK_READER_TRACK(reader, mpack_track_pop(&reader->track, type));
}
#endif

#if MPACK_DEBUG && MPACK_STDIO && MPACK_SETJMP && !MPACK_NO_PRINT
static void mpack_debug_print_element(mpack_reader_t* reader, size_t depth) {
    mpack_tag_t val = mpack_read_tag(reader);
    switch (val.type) {

        case mpack_type_nil:
            printf("null");
            break;
        case mpack_type_bool:
            printf(val.v.b ? "true" : "false");
            break;

        case mpack_type_float:
            printf("%f", val.v.f);
            break;
        case mpack_type_double:
            printf("%f", val.v.d);
            break;

        case mpack_type_int:
            printf("%" PRIi64, val.v.i);
            break;
        case mpack_type_uint:
            printf("%" PRIu64, val.v.u);
            break;

        case mpack_type_bin:
            // skip data
            for (size_t i = 0; i < val.v.l; ++i)
                mpack_read_native_u8(reader);
            printf("<binary data>");
            mpack_done_bin(reader);
            break;

        case mpack_type_ext:
            // skip data
            for (size_t i = 0; i < val.v.l; ++i)
                mpack_read_native_u8(reader);
            printf("<ext data of type %i>", val.exttype);
            mpack_done_ext(reader);
            break;

        case mpack_type_str:
            putchar('"');
            for (size_t i = 0; i < val.v.l; ++i) {
                char c;
                mpack_read_bytes(reader, &c, 1);
                switch (c) {
                    case '\n': printf("\\n"); break;
                    case '\\': printf("\\\\"); break;
                    case '"': printf("\\\""); break;
                    default: putchar(c); break;
                }
            }
            putchar('"');
            mpack_done_str(reader);
            break;

        case mpack_type_array:
            printf("[\n");
            for (size_t i = 0; i < val.v.n; ++i) {
                for (size_t j = 0; j < depth + 1; ++j)
                    printf("    ");
                mpack_debug_print_element(reader, depth + 1);
                if (i != val.v.n - 1)
                    putchar(',');
                putchar('\n');
            }
            for (size_t i = 0; i < depth; ++i)
                printf("    ");
            putchar(']');
            mpack_done_array(reader);
            break;

        case mpack_type_map:
            printf("{\n");
            for (size_t i = 0; i < val.v.n; ++i) {
                for (size_t j = 0; j < depth + 1; ++j)
                    printf("    ");
                mpack_debug_print_element(reader, depth + 1);
                printf(": ");
                mpack_debug_print_element(reader, depth + 1);
                if (i != val.v.n - 1)
                    putchar(',');
                putchar('\n');
            }
            for (size_t i = 0; i < depth; ++i)
                printf("    ");
            putchar('}');
            mpack_done_map(reader);
            break;
    }
}

void mpack_debug_print(const char* data, int len) {
    mpack_reader_t reader;
    mpack_reader_init_data(&reader, data, len);
    if (MPACK_READER_SETJMP(&reader)) {
        printf("<mpack parsing error %s>\n", mpack_error_to_string(mpack_reader_error(&reader)));
        return;
    }

    int depth = 2;
    for (int i = 0; i < depth; ++i)
        printf("    ");
    mpack_debug_print_element(&reader, depth);
    putchar('\n');

    if (mpack_reader_remaining(&reader, NULL) > 0)
        printf("<%i extra bytes at end of mpack>\n", (int)mpack_reader_remaining(&reader, NULL));
}
#endif

#endif


/* mpack-expect.c */

#define MPACK_INTERNAL 1

/* #include "mpack-expect.h" */

#if MPACK_EXPECT


// Basic Number Functions

uint8_t mpack_expect_u8(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= UINT8_MAX)
            return (uint8_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= 0 && var.v.i <= UINT8_MAX)
            return (uint8_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

uint16_t mpack_expect_u16(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= UINT16_MAX)
            return (uint16_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= 0 && var.v.i <= UINT16_MAX)
            return (uint16_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

uint32_t mpack_expect_u32(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= UINT32_MAX)
            return (uint32_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= 0 && var.v.i <= UINT32_MAX)
            return (uint32_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

uint64_t mpack_expect_u64(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        return var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= 0)
            return (uint64_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

int8_t mpack_expect_i8(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= INT8_MAX)
            return (int8_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= INT8_MIN && var.v.i <= INT8_MAX)
            return (int8_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

int16_t mpack_expect_i16(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= INT16_MAX)
            return (int16_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= INT16_MIN && var.v.i <= INT16_MAX)
            return (int16_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

int32_t mpack_expect_i32(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= INT32_MAX)
            return (int32_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        if (var.v.i >= INT32_MIN && var.v.i <= INT32_MAX)
            return (int32_t)var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

int64_t mpack_expect_i64(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint) {
        if (var.v.u <= INT64_MAX)
            return (int64_t)var.v.u;
    } else if (var.type == mpack_type_int) {
        return var.v.i;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

float mpack_expect_float(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint)
        return (float)var.v.u;
    else if (var.type == mpack_type_int)
        return (float)var.v.i;
    else if (var.type == mpack_type_float)
        return var.v.f;
    else if (var.type == mpack_type_double)
        return (float)var.v.d;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0.0f;
}

double mpack_expect_double(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_uint)
        return (double)var.v.u;
    else if (var.type == mpack_type_int)
        return (double)var.v.i;
    else if (var.type == mpack_type_float)
        return (double)var.v.f;
    else if (var.type == mpack_type_double)
        return var.v.d;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0.0;
}

float mpack_expect_float_strict(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_float)
        return var.v.f;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0.0f;
}

double mpack_expect_double_strict(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_float)
        return (double)var.v.f;
    else if (var.type == mpack_type_double)
        return var.v.d;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0.0;
}


// Ranged Number Functions

int8_t mpack_expect_i8_range(mpack_reader_t* reader, int8_t min_value, int8_t max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value, "min_value %i must be less than or equal to max_value %i",
            min_value, max_value);

    // read the value
    int8_t val = mpack_expect_i8(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}

// TODO: missing i16_range, i32_range, i64_range?

uint8_t mpack_expect_u8_range(mpack_reader_t* reader, uint8_t min_value, uint8_t max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value, "min_value %u must be less than or equal to max_value %u",
            min_value, max_value);

    // read the value
    uint8_t val = mpack_expect_u8(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}

uint16_t mpack_expect_u16_range(mpack_reader_t* reader, uint16_t min_value, uint16_t max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value, "min_value %u must be less than or equal to max_value %u",
            min_value, max_value);

    // read the value
    uint16_t val = mpack_expect_u16(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}

uint32_t mpack_expect_u32_range(mpack_reader_t* reader, uint32_t min_value, uint32_t max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value, "min_value %u must be less than or equal to max_value %u",
            min_value, max_value);

    // read the value
    uint32_t val = mpack_expect_u32(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}

uint64_t mpack_expect_u64_range(mpack_reader_t* reader, uint64_t min_value, uint64_t max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value,
            "min_value %" PRIu64 " must be less than or equal to max_value %" PRIu64, min_value, max_value);


    // read the value
    uint64_t val = mpack_expect_u64(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}

float mpack_expect_float_range(mpack_reader_t* reader, float min_value, float max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value, "min_value %f must be less than or equal to max_value %f",
            min_value, max_value);

    // read the value
    float val = mpack_expect_float(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}

double mpack_expect_double_range(mpack_reader_t* reader, double min_value, double max_value) {

    // make sure the range is sensible
    mpack_assert(min_value <= max_value, "min_value %f must be less than or equal to max_value %f",
            min_value, max_value);

    // read the value
    double val = mpack_expect_double(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_value;

    // make sure it fits
    if (val < min_value || val > max_value) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_value;
    }

    return val;
}


// Matching Number Functions

void mpack_expect_uint_match(mpack_reader_t* reader, uint64_t value) {
    if (mpack_expect_u64(reader) != value)
        mpack_reader_flag_error(reader, mpack_error_type);
}

void mpack_expect_int_match(mpack_reader_t* reader, int64_t value) {
    if (mpack_expect_i64(reader) != value)
        mpack_reader_flag_error(reader, mpack_error_type);
}


// Other Basic Types

void mpack_expect_nil(mpack_reader_t* reader) {
    mpack_reader_track_element(reader);
    uint8_t type = mpack_read_native_u8(reader);
    if (reader->error != mpack_ok)
        return;
    if (type != 0xc0)
        mpack_reader_flag_error(reader, mpack_error_type);
}

bool mpack_expect_bool(mpack_reader_t* reader) {
    mpack_reader_track_element(reader);
    uint8_t type = mpack_read_native_u8(reader);
    if (reader->error != mpack_ok)
        return false;
    if ((type & ~1) != 0xc2)
        mpack_reader_flag_error(reader, mpack_error_type);
    return (bool)(type & 1);
}

void mpack_expect_true(mpack_reader_t* reader) {
    if (mpack_expect_bool(reader) != true)
        mpack_reader_flag_error(reader, mpack_error_type);
}

void mpack_expect_false(mpack_reader_t* reader) {
    if (mpack_expect_bool(reader) != false)
        mpack_reader_flag_error(reader, mpack_error_type);
}


// Compound Types

uint32_t mpack_expect_map(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_map)
        return var.v.n;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

void mpack_expect_map_match(mpack_reader_t* reader, uint32_t count) {
    if (mpack_expect_map(reader) != count)
        mpack_reader_flag_error(reader, mpack_error_type);
}

bool mpack_expect_map_or_nil(mpack_reader_t* reader, uint32_t* count) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_nil) {
        *count = 0;
        return false;
    }
    if (var.type == mpack_type_map) {
        *count = var.v.n;
        return true;
    }
    mpack_reader_flag_error(reader, mpack_error_type);
    *count = 0;
    return false;
}

uint32_t mpack_expect_array(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_array)
        return var.v.n;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

void mpack_expect_array_match(mpack_reader_t* reader, uint32_t count) {
    if (mpack_expect_array(reader) != count)
        mpack_reader_flag_error(reader, mpack_error_type);
}

uint32_t mpack_expect_array_range(mpack_reader_t* reader, uint32_t min_count, uint32_t max_count) {

    // make sure the range is sensible
    mpack_assert(min_count <= max_count, "min_count %u must be less than or equal to max_count %u",
            min_count, max_count);

    // read the count
    uint32_t count = mpack_expect_array(reader);
    if (mpack_reader_error(reader) != mpack_ok)
        return min_count;

    // make sure it fits
    if (count < min_count || count > max_count) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return min_count;
    }

    return count;
}

#ifdef MPACK_MALLOC
void* mpack_expect_array_alloc_impl(mpack_reader_t* reader, size_t element_size, uint32_t max_count, size_t* out_count) {
    size_t count = *out_count = mpack_expect_array(reader);
    if (mpack_reader_error(reader))
        return NULL;
    if (count > max_count) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return NULL;
    }
    void* p = MPACK_MALLOC(element_size * count);
    if (p == NULL)
        mpack_reader_flag_error(reader, mpack_error_memory);
    return p;
}
#endif


// String Functions

uint32_t mpack_expect_str(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_str)
        return var.v.l;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

size_t mpack_expect_str_buf(mpack_reader_t* reader, char* buf, size_t bufsize) {
    size_t strsize = mpack_expect_str(reader);
    if (mpack_reader_error(reader))
        return 0;
    if (strsize > bufsize) {
        mpack_reader_flag_error(reader, mpack_error_too_big);
        return 0;
    }
    mpack_read_bytes(reader, buf, strsize);
    if (mpack_reader_error(reader))
        return 0;
    mpack_done_str(reader);
    return strsize;
}


// Binary Blob Functions

uint32_t mpack_expect_bin(mpack_reader_t* reader) {
    mpack_tag_t var = mpack_read_tag(reader);
    if (var.type == mpack_type_bin)
        return var.v.l;
    mpack_reader_flag_error(reader, mpack_error_type);
    return 0;
}

size_t mpack_expect_bin_buf(mpack_reader_t* reader, char* buf, size_t bufsize) {
    size_t binsize = mpack_expect_bin(reader);
    if (mpack_reader_error(reader))
        return 0;
    if (binsize > bufsize) {
        mpack_reader_flag_error(reader, mpack_error_too_big);
        return 0;
    }
    mpack_read_bytes(reader, buf, binsize);
    if (mpack_reader_error(reader))
        return 0;
    mpack_done_bin(reader);
    return binsize;
}

void mpack_expect_cstr(mpack_reader_t* reader, char* buf, size_t bufsize) {

    // make sure buffer makes sense
    mpack_assert(bufsize >= 1, "buffer size is zero; you must have room for at least a null-terminator");

    // expect a str
    size_t rawsize = mpack_expect_str_buf(reader, buf, bufsize - 1);
    if (mpack_reader_error(reader)) {
        buf[0] = 0;
        return;
    }
    buf[rawsize] = 0;

    // check it for null bytes
    for (size_t i = 0; i < rawsize; ++i) {
        if (buf[i] == 0) {
            buf[0] = 0;
            mpack_reader_flag_error(reader, mpack_error_type);
            return;
        }
    }
}

void mpack_expect_utf8_cstr(mpack_reader_t* reader, char* buf, size_t bufsize) {

    // make sure buffer makes sense
    mpack_assert(bufsize >= 1, "buffer size is zero; you must have room for at least a null-terminator");

    // expect a raw
    size_t rawsize = mpack_expect_str_buf(reader, buf, bufsize - 1);
    if (mpack_reader_error(reader)) {
        buf[0] = 0;
        return;
    }
    buf[rawsize] = 0;

    // check encoding
    uint32_t state = 0;
    uint32_t codepoint = 0;
    for (size_t i = 0; i < rawsize; ++i) {
        if (mpack_utf8_decode(&state, &codepoint, buf[i]) == MPACK_UTF8_REJECT) {
            buf[0] = 0;
            mpack_reader_flag_error(reader, mpack_error_type);
            return;
        }
    }

}

#ifdef MPACK_MALLOC
char* mpack_expect_cstr_alloc(mpack_reader_t* reader, size_t maxsize) {

    // make sure argument makes sense
    if (maxsize < 1) {
        mpack_break("maxsize is zero; you must have room for at least a null-terminator");
        mpack_reader_flag_error(reader, mpack_error_bug);
        return NULL;
    }

    // read size
    size_t length = mpack_expect_str(reader); // TODO: use expect str max? create expect str max...
    if (mpack_reader_error(reader))
        return NULL;
    if (length > (maxsize - 1)) {
        mpack_reader_flag_error(reader, mpack_error_type);
        return NULL;
    }

    // allocate
    char* str = (char*)MPACK_MALLOC(length + 1);
    if (str == NULL) {
        mpack_reader_flag_error(reader, mpack_error_memory);
        return NULL;
    }

    // read with jump disabled so we don't leak our buffer
    mpack_reader_track_bytes(reader, length);
    mpack_read_native_nojump(reader, str, length);

    if (mpack_reader_error(reader)) {
        MPACK_FREE(str);
        return NULL;
    }
    str[length] = 0;
    mpack_done_str(reader);
    return str;
}
#endif

void mpack_expect_cstr_match(mpack_reader_t* reader, const char* str) {
    if (reader->error != mpack_ok)
        return;

    // expect a str the correct length
    size_t len = mpack_strlen(str);
    if (len > UINT32_MAX)
        mpack_reader_flag_error(reader, mpack_error_invalid);
    mpack_expect_str_length(reader, (uint32_t)len);
    if (mpack_reader_error(reader))
        return;

    // check each byte
    for (size_t i = 0; i < len; ++i) {
        mpack_reader_track_bytes(reader, 1);
        if (mpack_read_native_u8(reader) != *str++) {
            mpack_reader_flag_error(reader, mpack_error_type);
            return;
        }
    }

    mpack_done_str(reader);
}


#endif


/* mpack-node.c */

#define MPACK_INTERNAL 1

/* #include "mpack-node.h" */

#if MPACK_NODE



/*
 * Tree Parsing
 */

typedef struct mpack_level_t {
    mpack_node_data_t* child;
    size_t left; // children left in level
} mpack_level_t;

typedef struct mpack_tree_parser_t {
    mpack_tree_t* tree;
    const char* data;
    size_t left; // bytes left in data
    size_t possible_nodes_left;

    size_t level;
    size_t depth;
    mpack_level_t* stack;
    bool stack_allocated;
} mpack_tree_parser_t;

static inline uint8_t mpack_tree_u8(mpack_tree_parser_t* parser) {
    if (parser->possible_nodes_left < sizeof(uint8_t)) {
        mpack_tree_flag_error(parser->tree, mpack_error_io);
        return 0;
    }
    uint8_t val = mpack_load_native_u8(parser->data);
    parser->data += sizeof(uint8_t);
    parser->left -= sizeof(uint8_t);
    parser->possible_nodes_left -= sizeof(uint8_t);
    return val;
}

static inline uint16_t mpack_tree_u16(mpack_tree_parser_t* parser) {
    if (parser->possible_nodes_left < sizeof(uint16_t)) {
        mpack_tree_flag_error(parser->tree, mpack_error_io);
        return 0;
    }
    uint16_t val = mpack_load_native_u16(parser->data);
    parser->data += sizeof(uint16_t);
    parser->left -= sizeof(uint16_t);
    parser->possible_nodes_left -= sizeof(uint16_t);
    return val;
}

static inline uint32_t mpack_tree_u32(mpack_tree_parser_t* parser) {
    if (parser->possible_nodes_left < sizeof(uint32_t)) {
        mpack_tree_flag_error(parser->tree, mpack_error_io);
        return 0;
    }
    uint32_t val = mpack_load_native_u32(parser->data);
    parser->data += sizeof(uint32_t);
    parser->left -= sizeof(uint32_t);
    parser->possible_nodes_left -= sizeof(uint32_t);
    return val;
}

static inline uint64_t mpack_tree_u64(mpack_tree_parser_t* parser) {
    if (parser->possible_nodes_left < sizeof(uint64_t)) {
        mpack_tree_flag_error(parser->tree, mpack_error_io);
        return 0;
    }
    uint64_t val = mpack_load_native_u64(parser->data);
    parser->data += sizeof(uint64_t);
    parser->left -= sizeof(uint64_t);
    parser->possible_nodes_left -= sizeof(uint64_t);
    return val;
}

static inline int8_t  mpack_tree_i8 (mpack_tree_parser_t* parser) {return (int8_t) mpack_tree_u8(parser); }
static inline int16_t mpack_tree_i16(mpack_tree_parser_t* parser) {return (int16_t)mpack_tree_u16(parser);}
static inline int32_t mpack_tree_i32(mpack_tree_parser_t* parser) {return (int32_t)mpack_tree_u32(parser);}
static inline int64_t mpack_tree_i64(mpack_tree_parser_t* parser) {return (int64_t)mpack_tree_u64(parser);}

static inline float mpack_tree_float(mpack_tree_parser_t* parser) {
    union {
        float f;
        uint32_t i;
    } u;
    u.i = mpack_tree_u32(parser);
    return u.f;
}

static inline double mpack_tree_double(mpack_tree_parser_t* parser) {
    union {
        double d;
        uint64_t i;
    } u;
    u.i = mpack_tree_u64(parser);
    return u.d;
}

void mpack_tree_parse_children(mpack_tree_parser_t* parser, mpack_node_data_t* node) {
    mpack_type_t type = node->type;
    size_t total = node->value.content.n;

    // Make sure we have enough room in the stack
    if (parser->level + 1 == parser->depth) {
        #ifdef MPACK_MALLOC
        size_t new_depth = parser->depth * 2;
        mpack_log("growing stack to depth %i\n", (int)new_depth);

        // Replace the stack-allocated parsing stack
        if (parser->stack_allocated) {
            mpack_level_t* new_stack = (mpack_level_t*)MPACK_MALLOC(sizeof(mpack_level_t) * new_depth);
            if (!new_stack) {
                mpack_tree_flag_error(parser->tree, mpack_error_memory);
                parser->level = 0;
                return;
            }
            memcpy(new_stack, parser->stack, sizeof(mpack_level_t) * parser->depth);
            parser->stack = new_stack;
            parser->stack_allocated = false;

        // Realloc the allocated parsing stack
        } else {
            parser->stack = (mpack_level_t*)mpack_realloc(parser->stack, sizeof(mpack_level_t) * parser->depth, sizeof(mpack_level_t) * new_depth);
            if (!parser->stack) {
                mpack_tree_flag_error(parser->tree, mpack_error_memory);
                parser->level = 0;
                return;
            }
        }
        parser->depth = new_depth;
        #else
        mpack_tree_flag_error(parser->tree, mpack_error_too_big);
        parser->level = 0;
        return;
        #endif
    }

    // Calculate total elements to read
    if (type == mpack_type_map) {
        if ((uint64_t)total * 2 > (uint64_t)SIZE_MAX) {
            mpack_tree_flag_error(parser->tree, mpack_error_too_big);
            parser->level = 0;
            return;
        }
        total *= 2;
    }

    // Each node is at least one byte. Count these bytes now to make
    // sure there is enough data left.
    if (total > parser->possible_nodes_left) {
        mpack_tree_flag_error(parser->tree, mpack_error_invalid);
        parser->level = 0;
        return;
    }
    parser->possible_nodes_left -= total;

    // If there are enough nodes left in the current page, no need to grow
    if (total <= parser->tree->page.left) {
        node->value.content.children = parser->tree->page.nodes + parser->tree->page.pos;
        parser->tree->page.pos += total;
        parser->tree->page.left -= total;

    } else {

        #ifdef MPACK_MALLOC

        // We can't grow if we're using a fixed pool
        if (!parser->tree->owned) {
            mpack_tree_flag_error(parser->tree, mpack_error_too_big);
            parser->level = 0;
            return;
        }

        // Otherwise we need to grow, and the node's children need to be contiguous.
        // This is a heuristic to decide whether we should waste the remaining space
        // in the current page and start a new one, or give the children their
        // own page. With a fraction of 1/8, this causes at most 12% additional
        // waste. Note that reducing this too much causes less cache coherence and
        // more malloc() overhead due to smaller allocations, so there's a tradeoff
        // here. This heuristic could use some improvement, especially with custom
        // page sizes.

        // Allocate the new link first. The two cases below put it into the list before trying
        // to allocate its nodes so it gets freed later in case of allocation failure.
        mpack_tree_link_t* link = (mpack_tree_link_t*)MPACK_MALLOC(sizeof(mpack_tree_link_t));
        if (link == NULL) {
            mpack_tree_flag_error(parser->tree, mpack_error_invalid);
            parser->level = 0;
            return;
        }

        if (total > MPACK_NODE_PAGE_SIZE || parser->tree->page.left > MPACK_NODE_PAGE_SIZE / 8) {
            mpack_log("allocating seperate page for %i children, %i left in page of size %i\n",
                    (int)total, (int)parser->tree->page.left, (int)MPACK_NODE_PAGE_SIZE);

            // Allocate only this node's children and insert it after the current page
            link->next = parser->tree->page.next;
            parser->tree->page.next = link;
            link->nodes = (mpack_node_data_t*)MPACK_MALLOC(sizeof(mpack_node_data_t) * total);
            if (link->nodes == NULL) {
                mpack_tree_flag_error(parser->tree, mpack_error_invalid);
                parser->level = 0;
                return;
            }

            // Use the new page for the node's children. pos and left are not used.
            node->value.content.children = link->nodes;

        } else {
            mpack_log("allocating new page for %i children, wasting %i in page of size %i\n",
                    (int)total, (int)parser->tree->page.left, (int)MPACK_NODE_PAGE_SIZE);

            // Move the current page into the new link, and allocate a new page
            *link = parser->tree->page;
            parser->tree->page.next = link;
            parser->tree->page.nodes = (mpack_node_data_t*)MPACK_MALLOC(sizeof(mpack_node_data_t) * MPACK_NODE_PAGE_SIZE);
            if (parser->tree->page.nodes == NULL) {
                mpack_tree_flag_error(parser->tree, mpack_error_invalid);
                parser->level = 0;
                return;
            }

            // Take this node's children from the page
            node->value.content.children = parser->tree->page.nodes;
            parser->tree->page.pos = total;
            parser->tree->page.left = MPACK_NODE_PAGE_SIZE - total;
        }

        #else
        // We can't grow if we don't have an allocator
        mpack_tree_flag_error(parser->tree, mpack_error_too_big);
        parser->level = 0;
        return;
        #endif
    }

    // Push this node onto the stack to read its children
    ++parser->level;
    parser->stack[parser->level].child = node->value.content.children;
    parser->stack[parser->level].left = total;
}

void mpack_tree_parse_bytes(mpack_tree_parser_t* parser, mpack_node_data_t* node) {
    size_t length = node->value.data.l;
    if (length > parser->possible_nodes_left) {
        mpack_tree_flag_error(parser->tree, mpack_error_invalid);
        parser->level = 0;
        return;
    }
    node->value.data.bytes = parser->data;
    parser->data += length;
    parser->left -= length;
    parser->possible_nodes_left -= length;
}

void mpack_tree_parse(mpack_tree_t* tree, const char* data, size_t length) {
    mpack_log("starting parse\n");

    // This function is unfortunately huge and ugly, but there isn't
    // a good way to break it apart without losing performance. It's
    // well-commented to try to make up for it.

    if (length == 0) {
        mpack_tree_init_error(tree, mpack_error_io);
        return;
    }
    if (tree->page.left == 0) {
        mpack_break("initial page has no nodes!");
        mpack_tree_init_error(tree, mpack_error_bug);
        return;
    }
    tree->root = tree->page.nodes + tree->page.pos;
    ++tree->page.pos;
    --tree->page.left;

    // Setup parser
    mpack_tree_parser_t parser;
    mpack_memset(&parser, 0, sizeof(parser));
    parser.tree = tree;
    parser.data = data;
    parser.left = length;

    // We read nodes in a loop instead of recursively for maximum
    // performance. The stack holds the amount of children left to
    // read in each level of the tree.

    // Even when we have a malloc() function, it's much faster to
    // allocate the initial parsing stack on the call stack. We
    // replace it with a heap allocation if we need to grow it.
    #ifdef MPACK_MALLOC
    static const size_t initial_depth = MPACK_NODE_INITIAL_DEPTH;
    parser.stack_allocated = true;
    #else
    static const size_t initial_depth = MPACK_NODE_MAX_DEPTH_WITHOUT_MALLOC;
    #endif

    mpack_level_t stack_[initial_depth];
    parser.depth = initial_depth;
    parser.stack = stack_;

    // We keep track of the number of possible nodes left in the data. This
    // is to ensure that malicious nested data is not trying to make us
    // run out of memory by allocating too many nodes. (For example malicious
    // data that repeats 0xDE 0xFF 0xFF would otherwise cause us to run out
    // of memory. With this, the parser can only allocate as many nodes as
    // there are bytes in the data (plus the paging overhead, 12%.) An error
    // will be flagged immediately if and when there isn't enough data left
    // to fully read all children of all open compound types on the stack.)
    parser.possible_nodes_left = length;

    // configure the root node
    --parser.possible_nodes_left;
    tree->node_count = 1;
    parser.level = 0;
    parser.stack[0].child = tree->root;
    parser.stack[0].left = 1;

    do {
        mpack_node_data_t* node = parser.stack[parser.level].child;
        --parser.stack[parser.level].left;
        ++parser.stack[parser.level].child;

        // read the type (we've already counted this byte in possible_nodes_left)
        ++parser.possible_nodes_left;
        uint8_t type = mpack_tree_u8(&parser);

        // as with mpack_read_tag(), the fastest way to parse a node is to switch
        // on the first byte, and to explicitly list every possible byte.
        switch (type) {

            // positive fixnum
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
            case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
            case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
            case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
            case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
            case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
            case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
            case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
            case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
            case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
            case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
                node->type = mpack_type_uint;
                node->value.u = type;
                break;

            // negative fixnum
            case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
            case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
            case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
            case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                node->type = mpack_type_int;
                node->value.i = (int8_t)type;
                break;

            // fixmap
            case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
            case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                node->type = mpack_type_map;
                node->value.content.n = type & ~0xf0;
                mpack_tree_parse_children(&parser, node);
                break;

            // fixarray
            case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
            case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
                node->type = mpack_type_array;
                node->value.content.n = type & ~0xf0;
                mpack_tree_parse_children(&parser, node);
                break;

            // fixstr
            case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
            case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
            case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
            case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
                node->type = mpack_type_str;
                node->value.data.l = type & ~0xe0;
                mpack_tree_parse_bytes(&parser, node);
                break;

            // nil
            case 0xc0:
                node->type = mpack_type_nil;
                break;

            // bool
            case 0xc2: case 0xc3:
                node->type = mpack_type_bool;
                node->value.b = type & 1;
                break;

            // bin8
            case 0xc4:
                node->type = mpack_type_bin;
                node->value.data.l = mpack_tree_u8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // bin16
            case 0xc5:
                node->type = mpack_type_bin;
                node->value.data.l = mpack_tree_u16(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // bin32
            case 0xc6:
                node->type = mpack_type_bin;
                node->value.data.l = mpack_tree_u32(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // ext8
            case 0xc7:
                node->type = mpack_type_ext;
                node->value.data.l = mpack_tree_u8(&parser);
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // ext16
            case 0xc8:
                node->type = mpack_type_ext;
                node->value.data.l = mpack_tree_u16(&parser);
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // ext32
            case 0xc9:
                node->type = mpack_type_ext;
                node->value.data.l = mpack_tree_u32(&parser);
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // float
            case 0xca:
                node->type = mpack_type_float;
                node->value.f = mpack_tree_float(&parser);
                break;

            // double
            case 0xcb:
                node->type = mpack_type_double;
                node->value.d = mpack_tree_double(&parser);
                break;

            // uint8
            case 0xcc:
                node->type = mpack_type_uint;
                node->value.u = mpack_tree_u8(&parser);
                break;

            // uint16
            case 0xcd:
                node->type = mpack_type_uint;
                node->value.u = mpack_tree_u16(&parser);
                break;

            // uint32
            case 0xce:
                node->type = mpack_type_uint;
                node->value.u = mpack_tree_u32(&parser);
                break;

            // uint64
            case 0xcf:
                node->type = mpack_type_uint;
                node->value.u = mpack_tree_u64(&parser);
                break;

            // int8
            case 0xd0:
                node->type = mpack_type_int;
                node->value.i = mpack_tree_i8(&parser);
                break;

            // int16
            case 0xd1:
                node->type = mpack_type_int;
                node->value.i = mpack_tree_i16(&parser);
                break;

            // int32
            case 0xd2:
                node->type = mpack_type_int;
                node->value.i = mpack_tree_i32(&parser);
                break;

            // int64
            case 0xd3:
                node->type = mpack_type_int;
                node->value.i = mpack_tree_i64(&parser);
                break;

            // fixext1
            case 0xd4:
                node->type = mpack_type_ext;
                node->value.data.l = 1;
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // fixext2
            case 0xd5:
                node->type = mpack_type_ext;
                node->value.data.l = 2;
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // fixext4
            case 0xd6:
                node->type = mpack_type_ext;
                node->value.data.l = 4;
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // fixext8
            case 0xd7:
                node->type = mpack_type_ext;
                node->value.data.l = 8;
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // fixext16
            case 0xd8:
                node->type = mpack_type_ext;
                node->value.data.l = 16;
                node->exttype = mpack_tree_i8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // str8
            case 0xd9:
                node->type = mpack_type_str;
                node->value.data.l = mpack_tree_u8(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // str16
            case 0xda:
                node->type = mpack_type_str;
                node->value.data.l = mpack_tree_u16(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // str32
            case 0xdb:
                node->type = mpack_type_str;
                node->value.data.l = mpack_tree_u32(&parser);
                mpack_tree_parse_bytes(&parser, node);
                break;

            // array16
            case 0xdc:
                node->type = mpack_type_array;
                node->value.content.n = mpack_tree_u16(&parser);
                mpack_tree_parse_children(&parser, node);
                break;

            // array32
            case 0xdd:
                node->type = mpack_type_array;
                node->value.content.n = mpack_tree_u32(&parser);
                mpack_tree_parse_children(&parser, node);
                break;

            // map16
            case 0xde:
                node->type = mpack_type_map;
                node->value.content.n = mpack_tree_u16(&parser);
                mpack_tree_parse_children(&parser, node);
                break;

            // map32
            case 0xdf:
                node->type = mpack_type_map;
                node->value.content.n = mpack_tree_u32(&parser);
                mpack_tree_parse_children(&parser, node);
                break;

            // reserved
            case 0xc1:
                mpack_tree_flag_error(tree, mpack_error_invalid);
                break;
        }

        // Pop any empty compound types from the stack
        while (parser.level != 0 && parser.stack[parser.level].left == 0)
            --parser.level;
    } while (parser.level != 0 && mpack_tree_error(parser.tree) == mpack_ok);

    #ifdef MPACK_MALLOC
    if (!parser.stack_allocated)
        MPACK_FREE(parser.stack);
    #endif

    tree->size = length - parser.left;
    mpack_log("parsed tree of %i bytes, %i bytes left\n", (int)tree->size, (int)parser.left);
    mpack_log("%i nodes in final page\n", (int)tree->page.pos);

    // This seems like a bug / performance flaw in GCC. In release the
    // below assert would compile to:
    //
    //     (!(possible_nodes_left == remaining) ? __builtin_unreachable() : ((void)0))
    //
    // This produces identical assembly with GCC 5.1 on ARM64 under -O3, but
    // with -O3 -flto, node parsing is over 4% slower. This should be a no-op
    // even in -flto since the function ends here and possible_nodes_left
    // does not escape this function.
    //
    // Leaving a TODO: here to explore this further. In the meantime we preproc it
    // under MPACK_DEBUG.
    #if MPACK_DEBUG
    mpack_assert(parser.possible_nodes_left == parser.left,
            "incorrect calculation of possible nodes! %i possible nodes, but %i bytes remaining",
            (int)parser.possible_nodes_left, (int)parser.left);
    #endif
}



/*
 * Tree functions
 */

mpack_node_t mpack_tree_root(mpack_tree_t* tree) {
    return mpack_node(tree, (mpack_tree_error(tree) != mpack_ok) ? &tree->nil_node : tree->root);
}

void mpack_tree_init_clear(mpack_tree_t* tree) {
    mpack_memset(tree, 0, sizeof(*tree));
    tree->nil_node.type = mpack_type_nil;
}

#ifdef MPACK_MALLOC
void mpack_tree_init(mpack_tree_t* tree, const char* data, size_t length) {
    mpack_tree_init_clear(tree);
    tree->owned = true;

    // allocate first page
    mpack_log("allocating initial page of size %i\n", (int)MPACK_NODE_PAGE_SIZE);
    tree->page.nodes = (mpack_node_data_t*)MPACK_MALLOC(sizeof(mpack_node_data_t) * MPACK_NODE_PAGE_SIZE);
    if (tree->page.nodes == NULL) {
        tree->error = mpack_error_memory;
        return;
    }
    tree->page.next = NULL;
    tree->page.pos = 0;
    tree->page.left = MPACK_NODE_PAGE_SIZE;

    mpack_tree_parse(tree, data, length);
}
#endif

void mpack_tree_init_pool(mpack_tree_t* tree, const char* data, size_t length, mpack_node_data_t* node_pool, size_t node_pool_count) {
    mpack_tree_init_clear(tree);

    tree->page.next = NULL;
    tree->page.nodes = node_pool;
    tree->page.pos = 0;
    tree->page.left = node_pool_count;

    mpack_tree_parse(tree, data, length);
}

void mpack_tree_init_error(mpack_tree_t* tree, mpack_error_t error) {
    mpack_tree_init_clear(tree);
    tree->error = error;
}

#if MPACK_STDIO
typedef struct mpack_file_tree_t {
    char* data;
    size_t size;
    char buffer[MPACK_BUFFER_SIZE];
} mpack_file_tree_t;

static void mpack_file_tree_teardown(mpack_tree_t* tree) {
    mpack_file_tree_t* file_tree = (mpack_file_tree_t*)tree->context;
    MPACK_FREE(file_tree->data);
    MPACK_FREE(file_tree);
}

static bool mpack_file_tree_read(mpack_tree_t* tree, mpack_file_tree_t* file_tree, const char* filename, size_t max_size) {

    // open the file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        mpack_tree_init_error(tree, mpack_error_io);
        return false;
    }

    // get the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size < 0) {
        fclose(file);
        mpack_tree_init_error(tree, mpack_error_io);
        return false;
    }
    if (size == 0) {
        fclose(file);
        mpack_tree_init_error(tree, mpack_error_invalid);
        return false;
    }

    // make sure the size is less than max_size
    // (this mess exists to safely convert between long and size_t regardless of their widths)
    if (max_size != 0 && (((uint64_t)LONG_MAX > (uint64_t)SIZE_MAX && size > (long)SIZE_MAX) || (size_t)size > max_size)) {
        fclose(file);
        mpack_tree_init_error(tree, mpack_error_too_big);
        return false;
    }

    // allocate data
    file_tree->data = (char*)MPACK_MALLOC(size);
    if (file_tree->data == NULL) {
        fclose(file);
        mpack_tree_init_error(tree, mpack_error_memory);
        return false;
    }

    // read the file
    long total = 0;
    while (total < size) {
        size_t read = fread(file_tree->data + total, 1, (size_t)(size - total), file);
        if (read <= 0) {
            fclose(file);
            mpack_tree_init_error(tree, mpack_error_io);
            MPACK_FREE(file_tree->data);
            return false;
        }
        total += read;
    }

    fclose(file);
    file_tree->size = (size_t)size;
    return true;
}

void mpack_tree_init_file(mpack_tree_t* tree, const char* filename, size_t max_size) {

    // the C STDIO family of file functions use long (e.g. ftell)
    if (max_size > LONG_MAX) {
        mpack_break("max_size of %" PRIu64 " is invalid, maximum is LONG_MAX", (uint64_t)max_size);
        mpack_tree_init_error(tree, mpack_error_too_big);
        return;
    }

    // allocate file tree
    mpack_file_tree_t* file_tree = (mpack_file_tree_t*) MPACK_MALLOC(sizeof(mpack_file_tree_t));
    if (file_tree == NULL) {
        mpack_tree_init_error(tree, mpack_error_memory);
        return;
    }

    // read all data
    if (!mpack_file_tree_read(tree, file_tree, filename, max_size)) {
        MPACK_FREE(file_tree);
        return;
    }

    mpack_tree_init(tree, file_tree->data, file_tree->size);
    mpack_tree_set_context(tree, file_tree);
    mpack_tree_set_teardown(tree, mpack_file_tree_teardown);
}
#endif

mpack_error_t mpack_tree_destroy(mpack_tree_t* tree) {
    #ifdef MPACK_MALLOC
    if (tree->owned) {
        if (tree->page.nodes)
            MPACK_FREE(tree->page.nodes);
        mpack_tree_link_t* link = tree->page.next;
        while (link) {
            mpack_tree_link_t* next = link->next;
            if (link->nodes)
                MPACK_FREE(link->nodes);
            MPACK_FREE(link);
            link = next;
        }
    }
    #endif

    if (tree->teardown)
        tree->teardown(tree);
    tree->teardown = NULL;

    #if MPACK_SETJMP
    if (tree->jump_env)
        MPACK_FREE(tree->jump_env);
    tree->jump_env = NULL;
    #endif

    return tree->error;
}

void mpack_tree_flag_error(mpack_tree_t* tree, mpack_error_t error) {
    mpack_log("tree %p setting error %i: %s\n", tree, (int)error, mpack_error_to_string(error));

    if (tree->error == mpack_ok) {
        tree->error = error;
        #if MPACK_SETJMP
        if (tree->jump_env)
            longjmp(*tree->jump_env, 1);
        #endif
    }

}



/*
 * Node misc functions
 */

void mpack_node_flag_error(mpack_node_t node, mpack_error_t error) {
    mpack_tree_flag_error(node.tree, error);
}

#if MPACK_DEBUG && MPACK_STDIO && MPACK_SETJMP && !MPACK_NO_PRINT
static void mpack_node_print_element(mpack_node_t node, size_t depth) {
    mpack_node_data_t* data = node.data;
    switch (data->type) {

        case mpack_type_nil:
            printf("null");
            break;
        case mpack_type_bool:
            printf(data->value.b ? "true" : "false");
            break;

        case mpack_type_float:
            printf("%f", data->value.f);
            break;
        case mpack_type_double:
            printf("%f", data->value.d);
            break;

        case mpack_type_int:
            printf("%" PRIi64, data->value.i);
            break;
        case mpack_type_uint:
            printf("%" PRIu64, data->value.u);
            break;

        case mpack_type_bin:
            printf("<binary data of length %u>", data->value.data.l);
            break;

        case mpack_type_ext:
            printf("<ext data of type %i and length %u>", data->exttype, data->value.data.l);
            break;

        case mpack_type_str:
            {
                putchar('"');
                const char* bytes = mpack_node_data(node);
                for (size_t i = 0; i < data->value.data.l; ++i) {
                    char c = bytes[i];
                    switch (c) {
                        case '\n': printf("\\n"); break;
                        case '\\': printf("\\\\"); break;
                        case '"': printf("\\\""); break;
                        default: putchar(c); break;
                    }
                }
                putchar('"');
            }
            break;

        case mpack_type_array:
            printf("[\n");
            for (size_t i = 0; i < data->value.content.n; ++i) {
                for (size_t j = 0; j < depth + 1; ++j)
                    printf("    ");
                mpack_node_print_element(mpack_node_array_at(node, i), depth + 1);
                if (i != data->value.content.n - 1)
                    putchar(',');
                putchar('\n');
            }
            for (size_t i = 0; i < depth; ++i)
                printf("    ");
            putchar(']');
            break;

        case mpack_type_map:
            printf("{\n");
            for (size_t i = 0; i < data->value.content.n; ++i) {
                for (size_t j = 0; j < depth + 1; ++j)
                    printf("    ");
                mpack_node_print_element(mpack_node_map_key_at(node, i), depth + 1);
                printf(": ");
                mpack_node_print_element(mpack_node_map_value_at(node, i), depth + 1);
                if (i != data->value.content.n - 1)
                    putchar(',');
                putchar('\n');
            }
            for (size_t i = 0; i < depth; ++i)
                printf("    ");
            putchar('}');
            break;
    }
}

void mpack_node_print(mpack_node_t node) {
    int depth = 2;
    for (int i = 0; i < depth; ++i)
        printf("    ");
    mpack_node_print_element(node, depth);
    putchar('\n');
}
#endif



/*
 * Node Data Functions
 */

size_t mpack_node_copy_data(mpack_node_t node, char* buffer, size_t size) {
    if (mpack_node_error(node) != mpack_ok)
        return 0;

    mpack_type_t type = node.data->type;
    if (type != mpack_type_str && type != mpack_type_bin && type != mpack_type_ext) {
        mpack_node_flag_error(node, mpack_error_type);
        return 0;
    }

    if (node.data->value.data.l > size) {
        mpack_node_flag_error(node, mpack_error_too_big);
        return 0;
    }

    mpack_memcpy(buffer, node.data->value.data.bytes, node.data->value.data.l);
    return (size_t)node.data->value.data.l;
}

void mpack_node_copy_cstr(mpack_node_t node, char* buffer, size_t size) {
    if (mpack_node_error(node) != mpack_ok)
        return;

    mpack_assert(size >= 1, "buffer size is zero; you must have room for at least a null-terminator");

    if (node.data->type != mpack_type_str) {
        buffer[0] = '\0';
        mpack_node_flag_error(node, mpack_error_type);
        return;
    }

    if (node.data->value.data.l > size - 1) {
        buffer[0] = '\0';
        mpack_node_flag_error(node, mpack_error_too_big);
        return;
    }

    mpack_memcpy(buffer, node.data->value.data.bytes, node.data->value.data.l);
    buffer[node.data->value.data.l] = '\0';
}

#ifdef MPACK_MALLOC
char* mpack_node_data_alloc(mpack_node_t node, size_t maxlen) {
    if (mpack_node_error(node) != mpack_ok)
        return NULL;

    // make sure this is a valid data type
    mpack_type_t type = node.data->type;
    if (type != mpack_type_str && type != mpack_type_bin && type != mpack_type_ext) {
        mpack_node_flag_error(node, mpack_error_type);
        return NULL;
    }

    if (node.data->value.data.l > maxlen) {
        mpack_node_flag_error(node, mpack_error_too_big);
        return NULL;
    }

    char* ret = (char*) MPACK_MALLOC((size_t)node.data->value.data.l);
    if (ret == NULL) {
        mpack_node_flag_error(node, mpack_error_memory);
        return NULL;
    }

    mpack_memcpy(ret, node.data->value.data.bytes, node.data->value.data.l);
    return ret;
}

char* mpack_node_cstr_alloc(mpack_node_t node, size_t maxlen) {
    if (mpack_node_error(node) != mpack_ok)
        return NULL;

    // make sure maxlen makes sense
    if (maxlen < 1) {
        mpack_break("maxlen is zero; you must have room for at least a null-terminator");
        mpack_node_flag_error(node, mpack_error_bug);
        return NULL;
    }

    if (node.data->type != mpack_type_str) {
        mpack_node_flag_error(node, mpack_error_type);
        return NULL;
    }

    if (node.data->value.data.l > maxlen - 1) {
        mpack_node_flag_error(node, mpack_error_too_big);
        return NULL;
    }

    char* ret = (char*) MPACK_MALLOC((size_t)(node.data->value.data.l + 1));
    if (ret == NULL) {
        mpack_node_flag_error(node, mpack_error_memory);
        return NULL;
    }

    mpack_memcpy(ret, node.data->value.data.bytes, node.data->value.data.l);
    ret[node.data->value.data.l] = '\0';
    return ret;
}
#endif


/*
 * Compound Node Functions
 */

mpack_node_t mpack_node_map_int_impl(mpack_node_t node, int64_t num, bool optional) {
    if (mpack_node_error(node) != mpack_ok)
        return mpack_tree_nil_node(node.tree);

    if (node.data->type != mpack_type_map) {
        mpack_node_flag_error(node, mpack_error_type);
        return mpack_tree_nil_node(node.tree);
    }

    for (size_t i = 0; i < node.data->value.content.n; ++i) {
        mpack_node_data_t* key = mpack_node_child(node, i * 2);
        mpack_node_data_t* value = mpack_node_child(node, i * 2 + 1);

        if (key->type == mpack_type_int && key->value.i == num)
            return mpack_node(node.tree, value);
        if (key->type == mpack_type_uint && num >= 0 && key->value.u == (uint64_t)num)
            return mpack_node(node.tree, value);
    }

    if (!optional)
        mpack_node_flag_error(node, mpack_error_data);
    return mpack_tree_nil_node(node.tree);
}

mpack_node_t mpack_node_map_uint_impl(mpack_node_t node, uint64_t num, bool optional) {
    if (mpack_node_error(node) != mpack_ok)
        return mpack_tree_nil_node(node.tree);

    if (node.data->type != mpack_type_map) {
        mpack_node_flag_error(node, mpack_error_type);
        return mpack_tree_nil_node(node.tree);
    }

    for (size_t i = 0; i < node.data->value.content.n; ++i) {
        mpack_node_data_t* key = mpack_node_child(node, i * 2);
        mpack_node_data_t* value = mpack_node_child(node, i * 2 + 1);

        if (key->type == mpack_type_uint && key->value.u == num)
            return mpack_node(node.tree, value);
        if (key->type == mpack_type_int && key->value.i >= 0 && (uint64_t)key->value.i == num)
            return mpack_node(node.tree, value);
    }

    if (!optional)
        mpack_node_flag_error(node, mpack_error_data);
    return mpack_tree_nil_node(node.tree);
}

mpack_node_t mpack_node_map_str_impl(mpack_node_t node, const char* str, size_t length, bool optional) {
    if (mpack_node_error(node) != mpack_ok)
        return mpack_tree_nil_node(node.tree);

    if (node.data->type != mpack_type_map) {
        mpack_node_flag_error(node, mpack_error_type);
        return mpack_tree_nil_node(node.tree);
    }

    for (size_t i = 0; i < node.data->value.content.n; ++i) {
        mpack_node_data_t* key = mpack_node_child(node, i * 2);
        mpack_node_data_t* value = mpack_node_child(node, i * 2 + 1);

        if (key->type == mpack_type_str && key->value.data.l == length && mpack_memcmp(str, key->value.data.bytes, length) == 0)
            return mpack_node(node.tree, value);
    }

    if (!optional)
        mpack_node_flag_error(node, mpack_error_data);
    return mpack_tree_nil_node(node.tree);
}

bool mpack_node_map_contains_str(mpack_node_t node, const char* str, size_t length) {
    if (mpack_node_error(node) != mpack_ok)
        return false;

    if (node.data->type != mpack_type_map) {
        mpack_node_flag_error(node, mpack_error_type);
        return false;
    }

    for (size_t i = 0; i < node.data->value.content.n; ++i) {
        mpack_node_data_t* key = mpack_node_child(node, i * 2);
        if (key->type == mpack_type_str && key->value.data.l == length && mpack_memcmp(str, key->value.data.bytes, length) == 0)
            return true;
    }

    return false;
}


#endif


