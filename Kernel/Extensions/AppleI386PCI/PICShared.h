/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOKIT_PICSHARED_H
#define _IOKIT_PICSHARED_H

#if     APIC_DEBUG
#define APIC_LOG(args...)  kprintf(args)
#else
#define APIC_LOG(args...)
#endif

/*
 * First 32-bit value in the IOInterruptSpecifier data is
 * the vector number, followed by the interrupt flags.
 */
#define DATA_TO_VECTOR(data) \
        (((UInt32 *)(data)->getBytesNoCopy())[0])

#define DATA_TO_FLAGS(data)  \
        (((UInt32 *)(data)->getBytesNoCopy())[1])

/*
 * Interrupt flags returned by DATA_TO_FLAGS().
 */
enum {
    kInterruptTriggerModeMask  = 0x01,
    kInterruptTriggerModeEdge  = 0x00,
    kInterruptTriggerModeLevel = kInterruptTriggerModeMask,
    kInterruptPolarityMask     = 0x02,
    kInterruptPolarityHigh     = 0x00,
    kInterruptPolarityLow      = kInterruptPolarityMask,
    kInterruptShareableMask    = 0x04,
    kInterruptNotShareable     = 0x00,
    kInterruptIsShareable      = kInterruptShareableMask,
};

/*
 * Keys for properties in the interrupt controller device/nub.
 */
#define kInterruptControllerNameKey   "InterruptControllerName"
#define kDestinationAPICIDKey         "Destination APIC ID"
#define kBaseVectorNumberKey          "Base Vector Number"
#define kVectorCountKey               "Vector Count"
#define kPhysicalAddressKey           "Physical Address"
#define kTimerVectorNumberKey         "Timer Vector Number"

/*
 * callPlatformFunction function names.
 */
#define kHandleSleepWakeFunction      "HandleSleepWake"
#define kSetVectorPhysicalDestination "SetVectorPhysicalDestination"

#endif /* !_IOKIT_PICSHARED_H */
