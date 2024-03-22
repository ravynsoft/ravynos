/*
 * Copyright (C) 2010 The Android Open Source Project
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

/**
 * @defgroup ANativeWindow Native Window
 *
 * ANativeWindow represents the producer end of an image queue.
 * It is the C counterpart of the android.view.Surface object in Java,
 * and can be converted both ways. Depending on the consumer, images
 * submitted to ANativeWindow can be shown on the display or sent to
 * other consumers, such as video encoders.
 * @{
 */

/**
 * @file native_window.h
 * @brief API for accessing a native window.
 */

#ifndef ANDROID_NATIVE_WINDOW_H
#define ANDROID_NATIVE_WINDOW_H

#include <stdint.h>
#include <sys/cdefs.h>

#include <android/data_space.h>
#include <android/hardware_buffer.h>
#include <android/rect.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Legacy window pixel format names, kept for backwards compatibility.
 * New code and APIs should use AHARDWAREBUFFER_FORMAT_*.
 */
enum ANativeWindow_LegacyFormat {
    // NOTE: these values must match the values from graphics/common/x.x/types.hal

    /** Red: 8 bits, Green: 8 bits, Blue: 8 bits, Alpha: 8 bits. **/
    WINDOW_FORMAT_RGBA_8888          = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    /** Red: 8 bits, Green: 8 bits, Blue: 8 bits, Unused: 8 bits. **/
    WINDOW_FORMAT_RGBX_8888          = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM,
    /** Red: 5 bits, Green: 6 bits, Blue: 5 bits. **/
    WINDOW_FORMAT_RGB_565            = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM,
};

/**
 * Transforms that can be applied to buffers as they are displayed to a window.
 *
 * Supported transforms are any combination of horizontal mirror, vertical
 * mirror, and clockwise 90 degree rotation, in that order. Rotations of 180
 * and 270 degrees are made up of those basic transforms.
 */
enum ANativeWindowTransform {
    ANATIVEWINDOW_TRANSFORM_IDENTITY            = 0x00,
    ANATIVEWINDOW_TRANSFORM_MIRROR_HORIZONTAL   = 0x01,
    ANATIVEWINDOW_TRANSFORM_MIRROR_VERTICAL     = 0x02,
    ANATIVEWINDOW_TRANSFORM_ROTATE_90           = 0x04,

    ANATIVEWINDOW_TRANSFORM_ROTATE_180          = ANATIVEWINDOW_TRANSFORM_MIRROR_HORIZONTAL |
                                                  ANATIVEWINDOW_TRANSFORM_MIRROR_VERTICAL,
    ANATIVEWINDOW_TRANSFORM_ROTATE_270          = ANATIVEWINDOW_TRANSFORM_ROTATE_180 |
                                                  ANATIVEWINDOW_TRANSFORM_ROTATE_90,
};

struct ANativeWindow;
/**
 * Opaque type that provides access to a native window.
 *
 * A pointer can be obtained using {@link ANativeWindow_fromSurface()}.
 */
typedef struct ANativeWindow ANativeWindow;

/**
 * Struct that represents a windows buffer.
 *
 * A pointer can be obtained using {@link ANativeWindow_lock()}.
 */
typedef struct ANativeWindow_Buffer {
    /// The number of pixels that are shown horizontally.
    int32_t width;

    /// The number of pixels that are shown vertically.
    int32_t height;

    /// The number of *pixels* that a line in the buffer takes in
    /// memory. This may be >= width.
    int32_t stride;

    /// The format of the buffer. One of AHardwareBuffer_Format.
    int32_t format;

    /// The actual bits.
    void* bits;

    /// Do not touch.
    uint32_t reserved[6];
} ANativeWindow_Buffer;

/**
 * Acquire a reference on the given {@link ANativeWindow} object. This prevents the object
 * from being deleted until the reference is removed.
 */
void ANativeWindow_acquire(ANativeWindow* window);

/**
 * Remove a reference that was previously acquired with {@link ANativeWindow_acquire()}.
 */
void ANativeWindow_release(ANativeWindow* window);

/**
 * Return the current width in pixels of the window surface.
 *
 * \return negative value on error.
 */
int32_t ANativeWindow_getWidth(ANativeWindow* window);

/**
 * Return the current height in pixels of the window surface.
 *
 * \return a negative value on error.
 */
int32_t ANativeWindow_getHeight(ANativeWindow* window);

/**
 * Return the current pixel format (AHARDWAREBUFFER_FORMAT_*) of the window surface.
 *
 * \return a negative value on error.
 */
int32_t ANativeWindow_getFormat(ANativeWindow* window);

/**
 * Change the format and size of the window buffers.
 *
 * The width and height control the number of pixels in the buffers, not the
 * dimensions of the window on screen. If these are different than the
 * window's physical size, then its buffer will be scaled to match that size
 * when compositing it to the screen. The width and height must be either both zero
 * or both non-zero.
 *
 * For all of these parameters, if 0 is supplied then the window's base
 * value will come back in force.
 *
 * \param width width of the buffers in pixels.
 * \param height height of the buffers in pixels.
 * \param format one of the AHardwareBuffer_Format constants.
 * \return 0 for success, or a negative value on error.
 */
int32_t ANativeWindow_setBuffersGeometry(ANativeWindow* window,
        int32_t width, int32_t height, int32_t format);

/**
 * Lock the window's next drawing surface for writing.
 * inOutDirtyBounds is used as an in/out parameter, upon entering the
 * function, it contains the dirty region, that is, the region the caller
 * intends to redraw. When the function returns, inOutDirtyBounds is updated
 * with the actual area the caller needs to redraw -- this region is often
 * extended by {@link ANativeWindow_lock}.
 *
 * \return 0 for success, or a negative value on error.
 */
int32_t ANativeWindow_lock(ANativeWindow* window, ANativeWindow_Buffer* outBuffer,
        ARect* inOutDirtyBounds);

/**
 * Unlock the window's drawing surface after previously locking it,
 * posting the new buffer to the display.
 *
 * \return 0 for success, or a negative value on error.
 */
int32_t ANativeWindow_unlockAndPost(ANativeWindow* window);

/**
 * Set a transform that will be applied to future buffers posted to the window.
 *
 * Available since API level 26.
 *
 * \param transform combination of {@link ANativeWindowTransform} flags
 * \return 0 for success, or -EINVAL if \p transform is invalid
 */
int32_t ANativeWindow_setBuffersTransform(ANativeWindow* window, int32_t transform) __INTRODUCED_IN(26);

/**
 * All buffers queued after this call will be associated with the dataSpace
 * parameter specified.
 *
 * dataSpace specifies additional information about the buffer.
 * For example, it can be used to convey the color space of the image data in
 * the buffer, or it can be used to indicate that the buffers contain depth
 * measurement data instead of color images. The default dataSpace is 0,
 * ADATASPACE_UNKNOWN, unless it has been overridden by the producer.
 *
 * Available since API level 28.
 *
 * \param dataSpace data space of all buffers queued after this call.
 * \return 0 for success, -EINVAL if window is invalid or the dataspace is not
 * supported.
 */
int32_t ANativeWindow_setBuffersDataSpace(ANativeWindow* window, int32_t dataSpace) __INTRODUCED_IN(28);

/**
 * Get the dataspace of the buffers in window.
 *
 * Available since API level 28.
 *
 * \return the dataspace of buffers in window, ADATASPACE_UNKNOWN is returned if
 * dataspace is unknown, or -EINVAL if window is invalid.
 */
int32_t ANativeWindow_getBuffersDataSpace(ANativeWindow* window) __INTRODUCED_IN(28);

/** Compatibility value for ANativeWindow_setFrameRate. */
enum ANativeWindow_FrameRateCompatibility {
    /**
     * There are no inherent restrictions on the frame rate of this window. When
     * the system selects a frame rate other than what the app requested, the
     * app will be able to run at the system frame rate without requiring pull
     * down. This value should be used when displaying game content, UIs, and
     * anything that isn't video.
     */
    ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_DEFAULT = 0,
    /**
     * This window is being used to display content with an inherently fixed
     * frame rate, e.g.\ a video that has a specific frame rate. When the system
     * selects a frame rate other than what the app requested, the app will need
     * to do pull down or use some other technique to adapt to the system's
     * frame rate. The user experience is likely to be worse (e.g. more frame
     * stuttering) than it would be if the system had chosen the app's requested
     * frame rate. This value should be used for video content.
     */
    ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_FIXED_SOURCE = 1
};

/**
 * Sets the intended frame rate for this window.
 *
 * On devices that are capable of running the display at different refresh
 * rates, the system may choose a display refresh rate to better match this
 * window's frame rate. Usage of this API won't introduce frame rate throttling,
 * or affect other aspects of the application's frame production
 * pipeline. However, because the system may change the display refresh rate,
 * calls to this function may result in changes to Choreographer callback
 * timings, and changes to the time interval at which the system releases
 * buffers back to the application.
 *
 * Note that this only has an effect for windows presented on the display. If
 * this ANativeWindow is consumed by something other than the system compositor,
 * e.g. a media codec, this call has no effect.
 *
 * Available since API level 30.
 *
 * \param frameRate The intended frame rate of this window, in frames per
 * second. 0 is a special value that indicates the app will accept the system's
 * choice for the display frame rate, which is the default behavior if this
 * function isn't called. The frameRate param does <em>not</em> need to be a
 * valid refresh rate for this device's display - e.g., it's fine to pass 30fps
 * to a device that can only run the display at 60fps.
 *
 * \param compatibility The frame rate compatibility of this window. The
 * compatibility value may influence the system's choice of display refresh
 * rate. See the ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_* values for more info.
 *
 * \return 0 for success, -EINVAL if the window, frame rate, or compatibility
 * value are invalid.
 */
int32_t ANativeWindow_setFrameRate(ANativeWindow* window, float frameRate, int8_t compatibility)
        __INTRODUCED_IN(30);

/**
 * Provides a hint to the window that buffers should be preallocated ahead of
 * time. Note that the window implementation is not guaranteed to preallocate
 * any buffers, for instance if an implementation disallows allocation of new
 * buffers, or if there is insufficient memory in the system to preallocate
 * additional buffers
 *
 * Available since API level 30.
 */
void ANativeWindow_tryAllocateBuffers(ANativeWindow* window);

#ifdef __cplusplus
};
#endif

#endif // ANDROID_NATIVE_WINDOW_H

/** @} */
