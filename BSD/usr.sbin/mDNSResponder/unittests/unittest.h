/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include "unittest_common.h"

#include <MacTypes.h>

#ifndef _UNITTEST_H_
#define _UNITTEST_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UNITTEST_SETSOCKOPT void mDNSPlatformSetSocktOpt(void *sockCxt, mDNSTransport_Type transType, \
    mDNSAddr_Type addrType, const DNSQuestion *q) \
    { \
        (void)(sockCxt); \
        (void)(transType); \
        (void)(addrType); \
        (void)(q); \
        return; \
    }

#define UNITTEST_UDPCLOSE void mDNSPlatformUDPClose(UDPSocket *sock) \
    { \
        (void)(sock); \
    }

#ifdef __cplusplus
}
#endif

#endif // ndef _UNITTEST_H_
