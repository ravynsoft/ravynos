/**
 * CoreUtils.h
 * Author: Zoe Knox      Created: 2026-02-17
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

#ifndef __CoreUtils_H
#define __CoreUtils_H

#include <Block.h>
#include <AssertMacros.h>
#include <libkern/OSTypes.h>
#include <sys/socket.h>

#ifdef __clang__
#define CU_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define CU_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#else
#define CU_ASSUME_NONNULL_BEGIN
#define CU_ASSUME_NONNULL_END
#endif

#ifndef sizeof_field
#define sizeof_field(TYPE, FIELD)   sizeof(((TYPE *)0)->FIELD)
#endif

#define STATIC_INLINE static inline

#include <CoreUtils/CommonServices.h>
#include <CoreUtils/DebugServices.h>
// #import <CoreUtils/SoftLinking.h>

// static const char path[] = "/System/Library/PrivateFrameworks/CoreUtils.framework/CoreUtils";

int16_t ReadBig16(int16_t *p);
int32_t ReadBig32(int32_t *p);
void WriteBig16(int16_t *p, int16_t v);
void WriteBig32(int32_t *p, int32_t v);

// These comments were found in mDNSResponder. FIXME: implement all these
// Note: strcpy_literal() appears in CoreUtils code, but isn't currently defined in framework headers.
//      Note: This was copied from CoreUtils because the HTTPHeader_Validate function is currently not exported in the framework.
//      Note: Similar to ParseEscapedString() from CoreUtils except that _ParseEscapedString() takes an optional C string
//      Note: Based on systemf() from CoreUtils framework.
//      Note: This was copied from CoreUtils because the SocketWriteAll function is currently not exported in the framework.
//      Note: This was copied from CoreUtils because the StringToIPv4Address function is currently not exported in the framework.
//      Note: This was copied from CoreUtils because the StringToIPv6Address function is currently not exported in the framework.
//      Note: This was copied from CoreUtils because the StringArray_Free function is currently not exported in the framework.
//      Note: Based on ServerSocketOpenEx() from CoreUtils. Added parameter to not use SO_REUSEPORT.
// These SipHash routines were copied from CoreUtils-500.9. (mDNSMacOSX/BLE.c)

void loadCoreUtils(void);


#endif /* __CoreUtils_H */
