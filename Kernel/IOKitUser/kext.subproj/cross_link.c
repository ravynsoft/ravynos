/*
 * Copyright (c) 2014 Apple Computer, Inc. All rights reserved.
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
 */
#include "cross_link.h"


/*********************************************************************
* Module Internal Variables
*********************************************************************/

static boolean_t __sCrossLinkEnabled  = FALSE;
static vm_size_t __sCrossLinkPageSize = 0;


/*********************************************************************
*********************************************************************/
boolean_t isCrossLinking(void)
{
    return __sCrossLinkEnabled;
}

/*********************************************************************
*********************************************************************/
boolean_t setCrossLinkPageSize(vm_size_t crossLinkPageSize)
{
    // verify radix 2
    if ((crossLinkPageSize != 0) && 
        ((crossLinkPageSize & (crossLinkPageSize - 1)) == 0)) {

        __sCrossLinkPageSize = crossLinkPageSize;
        __sCrossLinkEnabled = TRUE;

        return TRUE;   
    } else {
        return FALSE;
    }
}

/*********************************************************************
*********************************************************************/
vm_size_t getEffectivePageSize(void)
{
    if (__sCrossLinkEnabled) {
        return __sCrossLinkPageSize;
    } else {
        return PAGE_SIZE;
    }
}

/*********************************************************************
*********************************************************************/
vm_offset_t roundPageCrossSafe(vm_offset_t offset)
{
    // __sCrossLinkPageSize is checked for power of 2 above
    if (__sCrossLinkEnabled) {
        return (offset + (__sCrossLinkPageSize - 1)) & 
               (~(__sCrossLinkPageSize - 1));
    } else {
        return round_page(offset);
    }
}

/*********************************************************************
*********************************************************************/
mach_vm_offset_t roundPageCrossSafeFixedWidth(mach_vm_offset_t offset)
{
    // __sCrossLinkPageSize is checked for power of 2 above
    if (__sCrossLinkEnabled) {
        return (offset + (__sCrossLinkPageSize - 1)) & 
               (~(__sCrossLinkPageSize - 1));
    } else {
        return mach_vm_round_page(offset);
    }
}

