//
//  posix_utilities.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#ifndef posix_utilities_h
#define posix_utilities_h

#include "mDNSEmbeddedAPI.h"

// timestamp format: "2008-08-08 20:00:00.000000+0800", a 64-byte buffer is enough to store the result
extern void getLocalTimestamp(char * const buffer, mDNSu32 buffer_len);

#endif /* posix_utilities_h */
