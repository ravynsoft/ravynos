/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_VNDK_NATIVEWINDOW_AHARDWAREBUFFER_H
#define ANDROID_VNDK_NATIVEWINDOW_AHARDWAREBUFFER_H

// vndk is a superset of the NDK
#include <android/hardware_buffer.h>

#include <cutils/native_handle.h>

__BEGIN_DECLS

const native_handle_t* AHardwareBuffer_getNativeHandle(const AHardwareBuffer* buffer);

enum CreateFromHandleMethod {
    // enum values chosen to match internal GraphicBuffer::HandleWrapMethod
    AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_REGISTER = 2,
    AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_CLONE = 3,
};

/**
 * Create a AHardwareBuffer from a native handle.
 *
 * This function wraps a native handle in a AHardwareBuffer suitable for use by applications or
 * other parts of the system. The contents of desc will be returned by AHardwareBuffer_describe().
 *
 * If method is AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_REGISTER, the handle is assumed to be
 * unregistered, and it will be registered/imported before being wrapped in the AHardwareBuffer.
 * If successful, the AHardwareBuffer will own the handle.
 *
 * If method is AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_CLONE, the handle will be cloned and the
 * clone registered. The AHardwareBuffer will own the cloned handle but not the original.
 */
int AHardwareBuffer_createFromHandle(const AHardwareBuffer_Desc* desc,
                                     const native_handle_t* handle, int32_t method,
                                     AHardwareBuffer** outBuffer);

/**
 * Buffer pixel formats.
 */
enum {
    /* for future proofing, keep these in sync with system/graphics-base.h */

    /* same as HAL_PIXEL_FORMAT_BGRA_8888 */
    AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM           = 5,
    /* same as HAL_PIXEL_FORMAT_YV12 */
    AHARDWAREBUFFER_FORMAT_YV12                     = 0x32315659,
    /* same as HAL_PIXEL_FORMAT_Y8 */
    AHARDWAREBUFFER_FORMAT_Y8                       = 0x20203859,
    /* same as HAL_PIXEL_FORMAT_Y16 */
    AHARDWAREBUFFER_FORMAT_Y16                      = 0x20363159,
    /* same as HAL_PIXEL_FORMAT_RAW16 */
    AHARDWAREBUFFER_FORMAT_RAW16                    = 0x20,
    /* same as HAL_PIXEL_FORMAT_RAW10 */
    AHARDWAREBUFFER_FORMAT_RAW10                    = 0x25,
    /* same as HAL_PIXEL_FORMAT_RAW12 */
    AHARDWAREBUFFER_FORMAT_RAW12                    = 0x26,
    /* same as HAL_PIXEL_FORMAT_RAW_OPAQUE */
    AHARDWAREBUFFER_FORMAT_RAW_OPAQUE               = 0x24,
    /* same as HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED */
    AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED   = 0x22,
    /* same as HAL_PIXEL_FORMAT_YCBCR_422_SP */
    AHARDWAREBUFFER_FORMAT_YCbCr_422_SP             = 0x10,
    /* same as HAL_PIXEL_FORMAT_YCRCB_420_SP */
    AHARDWAREBUFFER_FORMAT_YCrCb_420_SP             = 0x11,
    /* same as HAL_PIXEL_FORMAT_YCBCR_422_I */
    AHARDWAREBUFFER_FORMAT_YCbCr_422_I              = 0x14,
};

/**
 * Buffer usage flags.
 */
enum {
    /* for future proofing, keep these in sync with hardware/gralloc.h */

    /* The buffer will be written by the HW camera pipeline. */
    AHARDWAREBUFFER_USAGE_CAMERA_WRITE              = 2UL << 16,
    /* The buffer will be read by the HW camera pipeline. */
    AHARDWAREBUFFER_USAGE_CAMERA_READ               = 4UL << 16,
    /* Mask for the camera access values. */
    AHARDWAREBUFFER_USAGE_CAMERA_MASK               = 6UL << 16,
};

__END_DECLS

#endif /* ANDROID_VNDK_NATIVEWINDOW_AHARDWAREBUFFER_H */
