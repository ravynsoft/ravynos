/*
 * Copyright © 2021 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "vk_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

/* __VK_ARG_N(...) returns the number of arguments provided to it  */
#define __VK_ARG_SEQ(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define __VK_ARG_N(...) __VK_ARG_SEQ(__VA_ARGS__,8,7,6,5,4,3,2,1,0)

#define VK_LOG_OBJS(...)                                        \
   __VK_ARG_N(__VA_ARGS__), (const void*[]){__VA_ARGS__}

#define VK_LOG_NO_OBJS(instance) 0, (const void**)instance

#define vk_logd(objects_macro, format, ...)                             \
   __vk_log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,            \
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,                \
            objects_macro, __FILE__, __LINE__, format, ## __VA_ARGS__)

#define vk_logi(objects_macro, format, ...)                             \
   __vk_log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,               \
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,                \
            objects_macro, __FILE__, __LINE__, format, ## __VA_ARGS__)

#define vk_logw(objects_macro, format, ...)                             \
   __vk_log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,            \
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,                \
            objects_macro, __FILE__, __LINE__, format, ## __VA_ARGS__)

#define vk_loge(objects_macro, format, ...)                             \
   __vk_log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,              \
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,                \
            objects_macro, __FILE__, __LINE__, format, ## __VA_ARGS__)

#define vk_perf(objects_macro, format, ...)                             \
   __vk_log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,            \
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,            \
            objects_macro, __FILE__, __LINE__, format, ## __VA_ARGS__)

#define __vk_log(severity, type, object_count,                          \
                 objects_or_instance, file, line, format, ...)          \
   __vk_log_impl(severity, type, object_count, objects_or_instance,     \
                 file, line, format, ## __VA_ARGS__)

void PRINTFLIKE(7, 8)
__vk_log_impl(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
              VkDebugUtilsMessageTypeFlagsEXT types,
              int object_count,
              const void **objects_or_instance,
              const char *file,
              int line,
              const char *format,
              ...);

#define vk_error(obj, error) \
   __vk_errorf(obj, error, __FILE__, __LINE__, NULL)

#define vk_errorf(obj, error, ...) \
   __vk_errorf(obj, error, __FILE__, __LINE__, __VA_ARGS__)

VkResult
__vk_errorv(const void *_obj, VkResult error,
            const char *file, int line,
            const char *format, va_list va);

VkResult PRINTFLIKE(5, 6)
__vk_errorf(const void *_obj, VkResult error,
            const char *file, int line,
            const char *format, ...);

#ifdef __cplusplus
}
#endif