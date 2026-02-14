/*
 * Copyright (c) 2005-2007 Apple Inc. All Rights Reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 * testcpp.h
 */

#ifndef _TESTCPP_H_
#define _TESTCPP_H_  1

#include "testmore.h"

#ifdef __cplusplus

#define no_throw(THIS, TESTNAME) \
({ \
    bool _this; \
    try { THIS; _this = true; } catch (...) { _this = false; } \
    test_ok(_this, TESTNAME, test_directive, test_reason, \
		__FILE__, __LINE__, \
		"#          got: <unknown excepetion>\n" \
		"#     expected: <no throw>\n"); \
})
#define does_throw(THIS, TESTNAME) \
({ \
    bool _this; \
    try { THIS; _this = false; } catch (...) { _this = true; } \
    test_ok(_this, TESTNAME, test_directive, test_reason, \
		__FILE__, __LINE__, \
		"#          got: <no throw>\n" \
		"#     expected: <any excepetion>\n"); \
})
#define is_throw(THIS, CLASS, METHOD, VALUE, TESTNAME) \
({ \
    bool _this; \
    try \
	{ \
		THIS; \
		_this = test_ok(false, TESTNAME, test_directive, test_reason, \
			__FILE__, __LINE__, \
			"#          got: <no throw>\n" \
			"#     expected: %s.%s == %s\n", \
			#CLASS, #METHOD, #VALUE); \
	} \
    catch (const CLASS &_exception) \
    { \
		_this = test_ok(_exception.METHOD == (VALUE), TESTNAME, \
			test_directive, test_reason, __FILE__, __LINE__, \
			"#          got: %d\n" \
			"#     expected: %s.%s == %s\n", \
			_exception.METHOD, #CLASS, #METHOD, #VALUE); \
	} \
    catch (...) \
    { \
    	_this = test_ok(false, TESTNAME, test_directive, test_reason, \
			__FILE__, __LINE__, \
			"#          got: <unknown excepetion>\n" \
			"#     expected: %s.%s == %s\n", \
			#CLASS, #METHOD, #VALUE); \
	} \
	_this; \
})
#endif /* __cplusplus */

#endif /* !_TESTCPP_H_ */
