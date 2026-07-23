/**
 * log.c
 * Author: Zoe Knox      Created: 2026-02-15
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 * SPDX: BSD-2-Clause
 *
 * This is an original implementation of Apple's libsystem_trace.dylib based on
 * open-source code, including ASL, XNU, Swift, and other sources, and the API
 * specs on developer.apple.com. It is not based on decompilation or diassembly
 * of any closed-source object files.
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

/* We really want to use the structs internally */
#undef OS_OBJECT_USE_OBJC
#define OS_OBJECT_USE_OBJC 0

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <dlfcn.h>
#include <errno.h>
#include <mach/mach_time.h>
#include <objc/runtime.h>
#include <CoreFoundation/CFString.h>
#include "log_internal.h"
#include <os/log_private.h>
#include <os/log_encode_types.h>
#include <os/log_encode.h>
#include "init.h"


struct os_log_s _os_log_default;
struct os_log_s _os_log_disabled;
struct os_log_s _os_log_null;

__BEGIN_DECLS

os_log_t
os_log_create(const char* subsystem, const char* category)
{
    struct os_log_s* log = calloc(1, sizeof(struct os_log_s));
    if (!log)
        return (os_log_t)NULL;

    if (_CFStringCreateWithCString) {
        log->subsystem = (void*)_CFStringCreateWithCString(NULL,
            subsystem, kCFStringEncodingUTF8);
        log->category = (void*)_CFStringCreateWithCString(NULL,
            category, kCFStringEncodingUTF8);
    } else {
        log->subsystem = subsystem ? strdup(subsystem) : NULL;
        log->category = category ? strdup(category) : NULL;
    }

    log->sink_type = OS_LOG_SINK_TYPE_FD;
    log->sink_dest = STDOUT_FILENO;

    os_retain(log);
    return log;
}

bool
os_log_shim_enabled(os_log_t log, os_log_type_t type)
{
    switch (type) {
        case OS_LOG_TYPE_DEFAULT:
        case OS_LOG_TYPE_DEBUG:
        case OS_LOG_TYPE_INFO:
            return true;
        default:
            return false;
    }
    return true;
}

bool os_log_type_enabled(os_log_t log, os_log_type_t type)
{
    return true; // FIXME
}

bool
os_log_info_enabled(os_log_t log)
{
    return os_log_shim_enabled(log, OS_LOG_TYPE_INFO)
        && os_log_type_enabled(log, OS_LOG_TYPE_INFO);
}

bool
os_log_debug_enabled(os_log_t log)
{
    return os_log_shim_enabled(log, OS_LOG_TYPE_DEBUG)
        && os_log_type_enabled(log, OS_LOG_TYPE_DEBUG);
}

static const char* _typeToStr(os_log_type_t type)
{
    switch (type) {
        case OS_LOG_TYPE_DEFAULT: return "Normal";
        case OS_LOG_TYPE_INFO: return "Info";
        case OS_LOG_TYPE_DEBUG: return "Debug";
        case OS_LOG_TYPE_ERROR: return "ERROR";
        case OS_LOG_TYPE_FAULT: return "FAULT";
    }
    return "unknown";
}

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
void
os_log_with_args(os_log_t oslog, os_log_type_t type, const char* format, va_list args, void* ret_addr)
{
}

__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW
void
_os_log_internal(void* dso, os_log_t log, os_log_type_t type, const char* message, ...)
{
}

__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW
int
_os_log_internal_driverKit(void* dso, os_log_t log, os_log_type_t type, const char* message, ...)
{
    return 0; // FIXME: should return EPERM, ENOBUFS, EINVAL on error
}

/**
 * @function os_log_encode
 * @abstract Encodes the provided variable arguments using the provided
 *           format string into the memory pointed to by buffer. It will
 *           write at most buffer_size bytes. The encoded buffer can then
 *           be passed to os_log_pack_size() and os_log_pack_fill().
 * @param buffer       Pointer to destination buffer
 * @param buffer_size  Size of the destination buffer
 * @param format       printf-style format string
 * @param args         Varargs list to fill in format string
 * @param flags        Currently not used
 * @return The number of bytes written to the buffer. Zero is returned
 *         if buffer is NULL, buffer_size is too small, or an error
 *         occurred in serialization.
 */
size_t os_log_encode(void* buffer,
    size_t buffer_size,
    const char* format,
    va_list args,
    uint32_t flags)
{
    struct os_log_arginfo_s header;
    os_log_buffer_context_t ctx;

    if (buffer == NULL || format == NULL || buffer_size <= 0)
        return 0;

    if (_os_log_encode(format, args, errno, ctx))
        return ctx->arg_content_sz;

    return 0;
}

/* Wrapper used by os/assumes.h */
size_t __os_log_encode(void* buffer,
    size_t buffer_size,
    const char* format,
    ...)
{
    va_list args;
    va_start(args, format);
    size_t size = os_log_encode(buffer, buffer_size, format, args, 0);
    va_end(args);
    return size;
}

size_t os_log_pack_size(const char* format, void* args, uint32_t flags)
{
    size_t psize = sizeof(struct os_log_pack_s);
    _os_log_arg_header* ah = args;
    const char* fmtp = format;

    if (!format)
        return 0;

    psize += strlen(format) + 1;

    /* Use format specifiers to enumerate the args chunk */
    while (*fmtp) {
        if (*fmtp == '%') {
            fmtp++;
            if (*fmtp != '%')
                psize += ah->payload_size;
        }
        fmtp++;
    }

    return psize;
}

uint8_t* os_log_pack_fill(void* pack,
    size_t pack_size,
    int saved_errno,
    const void* dso,
    const char* format)
{
    Dl_info info;

    if (!pack || pack_size == 0 || !format)
        return NULL;

    if (pack_size < strlen(format) + sizeof(struct os_log_pack_s))
        return NULL;

    uint8_t* payload = (uint8_t*)pack + sizeof(struct os_log_pack_s);
    struct os_log_pack_s *s = (struct os_log_pack_s *)pack;

    s->olp_continuous_time = mach_continuous_time();
    clock_gettime(CLOCK_REALTIME, &s->olp_wall_time);

    /* Find the mach_header */
    if (dladdr(dso, &info))
        s->olp_mh = info.dli_fbase;

    s->olp_pc = dso;
    s->olp_format = (const char *)payload;
    memcpy(s->olp_format, format, strlen(format) + 1);

    payload += strlen(format) + 1;
    return payload;
}

void os_log_pack_send(os_log_pack_t pack,
    os_log_t log,
    os_log_type_t type)
{
    /* FIXME: Send completed pack to ring buffer for logging */
}

void __os_log_impl(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* format,
    uint8_t* buffer,
    uint32_t buffer_size)
{
    if (!format)
        return;

    /* This _should_ be identical to the NSObject layout */
    struct os_log_s* s = (struct os_log_s*)log;
    fprintf(stderr, "[%s] %s.%s(%p): ", _typeToStr(type), s->subsystem, s->category, dso);
    fputs(format, stderr);
    //vfprintf(stderr, format, args); // FIXME
    fputc('\n', stderr);

}

void __os_log_error_impl(void* dso,
    os_log_t log,
    const char* format,
    uint8_t* buffer,
    uint32_t buffer_size)
{
    __os_log_impl(dso, log, OS_LOG_TYPE_ERROR, format, buffer, buffer_size);
}

void __os_log_fault_impl(void* dso,
    os_log_t log,
    const char* format,
    uint8_t* buffer,
    uint32_t buffer_size)
{
    __os_log_impl(dso, log, OS_LOG_TYPE_FAULT, format, buffer, buffer_size);
}

size_t _os_log_pack_init(void* pack,
    size_t pack_size,
    int saved_errno,
    const void* dso,
    const char* format,
    va_list args)
{

}

size_t _os_trace_encode(void* buffer,
    size_t buffer_size,
    const char* format,
    va_list args,
    uint32_t flags)
{
    return os_log_encode(buffer, buffer_size, format, args, flags);
}

void _os_trace_encode_and_send(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* format,
    va_list args,
    uint32_t flags)
{
    size_t buffer_size = 4096;
    char buffer[buffer_size];
    os_log_pack_t pack;

    size_t len = os_log_encode(buffer, buffer_size, format, args, flags);
    if (!len)
        return;

    //os_log_pack_size()
}

void _os_trace_debug(const char* message)
{
    _os_trace_encode_and_send(__builtin_return_address(0),
                              OS_LOG_DEFAULT,
                              OS_LOG_TYPE_DEBUG,
                              message,
                              NULL,
                              0);
}

void _os_trace_error(const char* message)
{
    _os_trace_encode_and_send(__builtin_return_address(0),
                              OS_LOG_DEFAULT,
                              OS_LOG_TYPE_ERROR,
                              message,
                              NULL,
                              0);
}

void _os_log_simple(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* message)
{
}

void _os_log_simple_impl(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* message)
{
}

void _os_log_error(void* dso, os_log_t log, const char* format, ...)
{
    char buffer[4096] = {0};
    size_t buffer_size = sizeof(buffer);
    va_list args;

    va_start(args, format);
    size_t len = os_log_encode(buffer, buffer_size, format, args, 0);
    va_end(args);
    __os_log_impl(dso, log, OS_LOG_TYPE_ERROR, format, buffer, buffer_size);
}

void _os_log_fault(void* dso, os_log_t log, const char* format, ...)
{
    char buffer[4096] = {0};
    size_t buffer_size = sizeof(buffer);
    va_list args;

    va_start(args, format);
    size_t len = os_log_encode(buffer, buffer_size, format, args, 0);
    va_end(args);
    __os_log_impl(dso, log, OS_LOG_TYPE_FAULT, format, buffer, buffer_size);
}

void _os_log_info(void* dso, os_log_t log, const char* format, ...)
{
    char buffer[4096] = {0};
    size_t buffer_size = sizeof(buffer);
    va_list args;

    va_start(args, format);
    size_t len = os_log_encode(buffer, buffer_size, format, args, 0);
    va_end(args);
    __os_log_impl(dso, log, OS_LOG_TYPE_INFO, format, buffer, buffer_size);
}

void _os_log_debug(void* dso, os_log_t log, const char* format, ...)
{
    char buffer[4096] = {0};
    size_t buffer_size = sizeof(buffer);
    va_list args;

    va_start(args, format);
    size_t len = os_log_encode(buffer, buffer_size, format, args, 0);
    va_end(args);
    __os_log_impl(dso, log, OS_LOG_TYPE_DEBUG, format, buffer, buffer_size);
}

void _os_log_set_mode(uint32_t mode)
{
}

uint32_t _os_log_get_mode(void)
{
    return 0; // FIXME: what should this return?
}

void _os_log_preferences_refresh(void)
{
}

// Heuristic: treat small or obviously sensitive data as private
static bool
isLikelyPrivateData(const UInt8 *bytes, CFIndex len)
{
    if (len == 0 || len > 4096) return false;

    // Simple heuristic: check for ASCII ranges typical of passwords/tokens
    int printable = 0;
    for (CFIndex i = 0; i < len; i++) {
        if (bytes[i] >= 32 && bytes[i] <= 126)
            printable++;
    }

    // If mostly printable and short → likely a secret
    if (len <= 64 && printable > (len * 0.8))
        return true;

    return false;
}

bool
_NSCF2data(const void *obj, char *string_value, size_t string_sz, bool *is_private)
{
    if (!string_value || string_sz == 0)
        return false;

    string_value[0] = '\0';
    if (is_private) *is_private = false;

    // Check if this is toll‑free bridged CFData/NSData
    if (!obj || !CFGetTypeID(obj) || CFGetTypeID(obj) != CFDataGetTypeID()) {
        snprintf(string_value, string_sz, "<not CFData>");
        return false;
    }

    CFDataRef data = (CFDataRef)obj;
    CFIndex len = CFDataGetLength(data);
    const UInt8 *bytes = CFDataGetBytePtr(data);

    if (!bytes) {
        snprintf(string_value, string_sz, "<null bytes>");
        return false;
    }

    // Determine privacy
    bool privateFlag = isLikelyPrivateData(bytes, len);
    if (is_private) *is_private = privateFlag;

    if (privateFlag) {
        // Redacted output similar to Apple logging
        snprintf(string_value, string_sz,
                 "<NSData %ld bytes: private>", (long)len);
        return true;
    }

    // Non‑private: produce a hex dump (truncated to fit buffer)
    size_t maxBytes = (string_sz - 1) / 2; // 2 chars per byte
    if (maxBytes > (size_t)len) maxBytes = len;

    char *p = string_value;
    for (size_t i = 0; i < maxBytes; i++) {
        snprintf(p, 3, "%02X", bytes[i]);
        p += 2;
    }

    // If truncated, append ellipsis
    if (maxBytes < (size_t)len && (p - string_value + 4) < string_sz) {
        strcpy(p, "...");
    }

    return true;
}

bool _os_log_string_is_public(const char* str)
{
    return true; // FIXME
}
