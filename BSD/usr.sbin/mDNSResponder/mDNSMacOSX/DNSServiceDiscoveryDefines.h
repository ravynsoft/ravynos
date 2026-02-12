/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2003, 2006, 2009, 2011 Apple Inc. All rights reserved.
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

#ifndef __DNS_SERVICE_DISCOVERY_DEFINES_H
#define __DNS_SERVICE_DISCOVERY_DEFINES_H

#include <mach/mach_types.h>

typedef char DNSCString[1024];
typedef char sockaddr_t[128];

typedef const char * record_data_t;
typedef struct { char bytes[4]; } IPPort;

#endif  /* __DNS_SERVICE_DISCOVERY_DEFINES_H */
