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

#ifndef _IONETWORKMEDIUM_H
#define _IONETWORKMEDIUM_H

__BEGIN_DECLS

#include <net/if_media.h>

/*! @typedef IOMediumType
    @discussion A 32-bit value divided into fields which describes
    a single medium type. */

typedef UInt32 IOMediumType;

/*! @defined kIOMediumType
    @abstract A property of IONetworkMedium objects.
    @discussion The kIOMediumType property is an OSNumber object that describes the type of
        medium that this object represents. 
*/

#define kIOMediumType           "Type"

/*! @defined kIOMediumFlags
    @abstract A property of IONetworkMedium objects.
    @discussion The kIOMediumFlags property is an OSNumber object that describes a set of
        attributes assigned to the medium. 
*/

#define kIOMediumFlags          "Flags"

/*! @defined kIOMediumSpeed
    @abstract A property of IONetworkMedium objects.
    @discussion The kIOMediumSpeed property is an OSNumber object that describes the maximum link
        speed supported by the medium in bits per second. 
*/

#define kIOMediumSpeed          "Speed"

/*! @defined kIOMediumIndex
    @abstract A property of IONetworkMedium objects.
    @discussion The kIOMediumIndex property is an OSNumber object that describes an index assigned
        by the owner of the medium object. Its interpretation is driver
        specific.
*/

#define kIOMediumIndex          "Index"

//===========================================================================
// Medium Type (IOMediumType).
//
// The medium type is encoded by a 32-bit value. The definitions of
// the fields and the encoding for each field is adapted from FreeBSD.
//
// Bits     Definition
// -------------------
//  4-0     medium subtype
//  7-5     network type
// 15-8     network specific options
// 19-16    reserved
// 27-20    common options
// 31-28    instance number

// Ethernet.
//
enum {
    kIOMediumEthernet             =  IFM_ETHER,
    kIOMediumEthernetAuto         =  ( IFM_AUTO    | IFM_ETHER ),
    kIOMediumEthernetManual       =  ( IFM_MANUAL  | IFM_ETHER ),
    kIOMediumEthernetNone         =  ( IFM_NONE    | IFM_ETHER ),
    kIOMediumEthernet10BaseT      =  ( IFM_10_T    | IFM_ETHER ),
    kIOMediumEthernet10Base2      =  ( IFM_10_2    | IFM_ETHER ),
    kIOMediumEthernet10Base5      =  ( IFM_10_5    | IFM_ETHER ),
    kIOMediumEthernet100BaseTX    =  ( IFM_100_TX  | IFM_ETHER ),
    kIOMediumEthernet100BaseFX    =  ( IFM_100_FX  | IFM_ETHER ),
    kIOMediumEthernet100BaseT4    =  ( IFM_100_T4  | IFM_ETHER ),
    kIOMediumEthernet100BaseVG    =  ( IFM_100_VG  | IFM_ETHER ),
    kIOMediumEthernet100BaseT2    =  ( IFM_100_T2  | IFM_ETHER ),
    kIOMediumEthernet1000BaseSX   =  ( IFM_1000_SX | IFM_ETHER ),
    kIOMediumEthernet10BaseSTP    =  ( IFM_10_STP  | IFM_ETHER ),
    kIOMediumEthernet10BaseFL     =  ( IFM_10_FL   | IFM_ETHER ),
    kIOMediumEthernet1000BaseLX   =  ( IFM_1000_LX | IFM_ETHER ),
    kIOMediumEthernet1000BaseCX   =  ( IFM_1000_CX | IFM_ETHER ),
    kIOMediumEthernet1000BaseTX   =  ( IFM_1000_T  | IFM_ETHER ), //deprecated- use kIOMediumEthernet1000BaseT instead
    kIOMediumEthernet1000BaseT    =  ( IFM_1000_T  | IFM_ETHER ),
    kIOMediumEthernetHomePNA1     =  ( IFM_HPNA_1  | IFM_ETHER ),
	kIOMediumEthernet10GBaseSR    =  ( IFM_10G_SR  | IFM_ETHER ),
	kIOMediumEthernet10GBaseLR    =  ( IFM_10G_LR  | IFM_ETHER ),
    kIOMediumEthernet10GBaseCX4   =  ( IFM_10G_CX4 | IFM_ETHER ),
    kIOMediumEthernet10GBaseT     =  ( IFM_10G_T   | IFM_ETHER ),
    kIOMediumEthernet2500BaseT    =  ( IFM_2500_T  | IFM_ETHER ),
    kIOMediumEthernet5000BaseT    =  ( IFM_5000_T  | IFM_ETHER )
};

// IEEE 802.11 Wireless.
//
enum {
    kIOMediumIEEE80211            =  IFM_IEEE80211,
    kIOMediumIEEE80211Auto        =  ( IFM_AUTO           | IFM_IEEE80211 ),
    kIOMediumIEEE80211Manual      =  ( IFM_MANUAL         | IFM_IEEE80211 ),
    kIOMediumIEEE80211None        =  ( IFM_NONE           | IFM_IEEE80211 ),
    kIOMediumIEEE80211FH1         =  ( IFM_IEEE80211_FH1  | IFM_IEEE80211 ),
    kIOMediumIEEE80211FH2         =  ( IFM_IEEE80211_FH2  | IFM_IEEE80211 ),
    kIOMediumIEEE80211DS2         =  ( IFM_IEEE80211_DS2  | IFM_IEEE80211 ),
    kIOMediumIEEE80211DS5         =  ( IFM_IEEE80211_DS5  | IFM_IEEE80211 ),
    kIOMediumIEEE80211DS11        =  ( IFM_IEEE80211_DS11 | IFM_IEEE80211 ),
    kIOMediumIEEE80211DS1         =  ( IFM_IEEE80211_DS1  | IFM_IEEE80211 ),
    kIOMediumIEEE80211OptionAdhoc =  IFM_IEEE80211_ADHOC
};

// Common options.
//
enum {
    kIOMediumOptionFullDuplex     = IFM_FDX,
    kIOMediumOptionHalfDuplex     = IFM_HDX,
    kIOMediumOptionFlowControl    = IFM_FLOW,
    kIOMediumOptionEEE            = IFM_EEE,
    kIOMediumOptionFlag0          = IFM_FLAG0,
    kIOMediumOptionFlag1          = IFM_FLAG1,
    kIOMediumOptionFlag2          = IFM_FLAG2,
    kIOMediumOptionLoopback       = IFM_LOOP
};

// Medium type masks.
//
#define kIOMediumSubTypeMask        IFM_TMASK
#define kIOMediumNetworkTypeMask    IFM_NMASK
#define kIOMediumOptionsMask        IFM_OMASK
#define kIOMediumCommonOptionsMask  IFM_GMASK
#define kIOMediumInstanceShift      IFM_ISHIFT
#define kIOMediumInstanceMask       IFM_IMASK

// Medium type field accessors.
//
#define IOMediumGetSubType(x)       ((x)  & kIOMediumSubTypeMask)
#define IOMediumGetNetworkType(x)   ((x)  & kIOMediumNetworkTypeMask)
#define IOMediumGetInstance(x)      (((x) & kIOMediumInstanceMask) >> \
                                            kIOMediumInstanceShift)

//===========================================================================
// Medium flags.


//===========================================================================
// Link status bits.
//
enum {
    kIONetworkLinkValid           = IFM_AVALID,     // link status is valid
    kIONetworkLinkActive          = IFM_ACTIVE,     // link is up/active.
    kIONetworkLinkNoNetworkChange = IFM_WAKESAMENET
};

__END_DECLS


#endif /* !_IONETWORKMEDIUM_H */
