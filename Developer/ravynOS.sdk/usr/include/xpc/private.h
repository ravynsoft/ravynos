/**
 * xpc/private.h
 * Author: Zoe Knox      Created: 2026-02-14
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 * SPDX: BSD-2-Clause
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

#ifndef __XPC_PRIVATE_H__
#define __XPC_PRIVATE_H__

#include <xpc/launchd.h>
#include <Private/xpc_internal.h>

/**
 * @function _xpc_runtime_is_app_sandboxed
 *
 * @abstract
 * Returns `true` if the runtime is currently sandboxed. In ravynOS, this
 * function currently always returns `false`.
 *
 * @discussion
 * This is a private API. It is used in libcopyfile and ...?
 */
XPC_EXPORT
bool
_xpc_runtime_is_app_sandboxed(void);


/**
 * @function xpc_create_from_plist
 *
 * @abstract
 * Accepts the serialized plist blob given by `buf` and `length`, and returns
 * an equivalent XPC_DICTIONARY object.
 *
 * @param buf Pointer to start of plist data
 * @param length Length of plist data in bytes
 *
 * @return xpc_object_t The converted plist
 */
XPC_EXPORT
xpc_object_t
xpc_create_from_plist(uint8_t *buf, size_t length);

#endif /* __XPC_PRIVATE_H__ */
