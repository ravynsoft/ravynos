/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2007-2011 Apple Inc. All rights reserved.
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

#ifndef H_HELPERMSG_TYPES_H
#define H_HELPERMSG_TYPES_H

#include <stdint.h>
typedef uint8_t v4addr_t [ 4];
typedef uint8_t ethaddr_t[ 6];
typedef uint8_t v6addr_t [16];
typedef const char *string_t;

#define PFPortArraySize 4
typedef uint16_t pfArray_t [PFPortArraySize];

#endif /* H_HELPERMSG_TYPES_H */
