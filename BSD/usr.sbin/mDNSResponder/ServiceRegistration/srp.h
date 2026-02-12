/* srp.h
 *
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
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
 *
 * Service Registration Protocol common definitions
 */

#ifndef __SRP_H
#define __SRP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __clang__
#define NULLABLE _Nullable
#define NONNULL _Nonnull
#else
#define NULLABLE
#define NONNULL
#endif

#define ERROR(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define INFO(fmt, ...)  fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

typedef struct srp_key srp_key_t;
#endif

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
