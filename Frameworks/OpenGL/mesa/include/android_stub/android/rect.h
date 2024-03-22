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
 * @addtogroup NativeActivity Native Activity
 * @{
 */

/**
 * @file rect.h
 */

#ifndef ANDROID_RECT_H
#define ANDROID_RECT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Rectangular window area.
 *
 * This is the NDK equivalent of the android.graphics.Rect class in Java. It is
 * used with {@link ANativeActivityCallbacks::onContentRectChanged} event
 * callback and the ANativeWindow_lock() function.
 *
 * In a valid ARect, left <= right and top <= bottom. ARect with left=0, top=10,
 * right=1, bottom=11 contains only one pixel at x=0, y=10.
 */
typedef struct ARect {
#ifdef __cplusplus
    typedef int32_t value_type;
#endif
    /// Minimum X coordinate of the rectangle.
    int32_t left;
    /// Minimum Y coordinate of the rectangle.
    int32_t top;
    /// Maximum X coordinate of the rectangle.
    int32_t right;
    /// Maximum Y coordinate of the rectangle.
    int32_t bottom;
} ARect;

#ifdef __cplusplus
};
#endif

#endif // ANDROID_RECT_H

/** @} */
