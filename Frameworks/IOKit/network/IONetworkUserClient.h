/*
 * Copyright (c) 1998-2008 Apple Inc. All rights reserved.
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

#ifndef _IONETWORKUSERCLIENT_H
#define _IONETWORKUSERCLIENT_H

// IONetworkUserClient type ID.
//
#define kIONetworkUserClientTypeID   0xff000001
#define kIONUCType                   0xff000001  // FIXME

// IONetworkUserClient call structure definitions.
//
enum {
        kIONUCResetNetworkDataIndex          = 0,
        kIONUCWriteNetworkDataIndex          = 1,
        kIONUCReadNetworkDataIndex           = 2,
        kIONUCGetNetworkDataCapacityIndex    = 3,
        kIONUCGetNetworkDataHandleIndex      = 4,
        kIONUCLastIndex
};


#endif /* !_IONETWORKUSERCLIENT_H */
