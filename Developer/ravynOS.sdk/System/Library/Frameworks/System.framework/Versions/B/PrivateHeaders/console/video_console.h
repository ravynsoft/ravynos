/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * @APPLE_FREE_COPYRIGHT@
 */

#ifndef _VIDEO_CONSOLE_H_
#define _VIDEO_CONSOLE_H_

#include <device/device_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define kVCSysctlProgressOptions      "kern.progressoptions"
#define kVCSysctlConsoleOptions       "kern.consoleoptions"
#define kVCSysctlProgressMeterEnable  "kern.progressmeterenable"
#define kVCSysctlProgressMeter        "kern.progressmeter"

enum{
	kVCDarkReboot       = 0x00000001,
	kVCAcquireImmediate = 0x00000002,
	kVCUsePosition      = 0x00000004,
	kVCDarkBackground   = 0x00000008,
	kVCLightBackground  = 0x00000010,
};

enum {
	kDataRotate0   = 0,
	kDataRotate90  = 1,
	kDataRotate180 = 2,
	kDataRotate270 = 3
};

struct vc_progress_user_options {
	uint32_t options;
	// fractional position of middle of spinner 0 (0.0) - 0xFFFFFFFF (1.0)
	uint32_t x_pos;
	uint32_t y_pos;
	uint32_t resv[8];
};
typedef struct vc_progress_user_options vc_progress_user_options;



#ifdef __cplusplus
}
#endif

#endif /* _VIDEO_CONSOLE_H_ */
