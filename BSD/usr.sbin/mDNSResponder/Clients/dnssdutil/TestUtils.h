//
//  TestUtils.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#ifndef    __TestUtils_h
#define    __TestUtils_h

#include <TargetConditionals.h>
#include <MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DNSSDUTIL_XCTEST "DNSSDUTIL_XCTEST"

Boolean TestUtilsRunXCTestNamed(const char * classname);

#ifdef __cplusplus
}
#endif

#endif // __TestUtils_h
