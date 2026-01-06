/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __rfc4648_h
#define __rfc4648_h

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum _rfc4648_type {
    RFC4648_TYPE_BASE16,
    RFC4648_TYPE_BASE16_LCASE,
    RFC4648_TYPE_BASE32,
    RFC4648_TYPE_BASE64,
    RFC4648_TYPE_BASE64_SAFE
} rfc4648_type_t;

#ifdef __cplusplus
extern "C" {
#endif

size_t
rfc4648_get_encoded_size
(
    rfc4648_type_t type,
    size_t         size
);

size_t
rfc4648_get_decoded_size
(
    rfc4648_type_t type,
    size_t         size
);

bool
rfc4648_can_decode
(
    char const     *input,
    size_t          insize,
    rfc4648_type_t *type
);

bool
rfc4648_encode
(
    rfc4648_type_t  type,
    void const     *input,
    size_t          insize,
    char           *output,
    size_t         *outsize
);

bool
rfc4648_decode
(
    rfc4648_type_t  type,
    void const     *input,
    size_t          insize,
    char           *output,
    size_t         *outsize,
    bool            recover
);

#ifdef __cplusplus
}
#endif

#endif  /* !__rfc4648_h */
