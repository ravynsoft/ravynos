/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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

enum {
    kDisplayAppleVendorID       = 0x610
};

struct EDIDDetailedTimingDesc {
    UInt16      clock;
    UInt8       horizActive;
    UInt8       horizBlanking;
    UInt8       horizHigh;
    UInt8       verticalActive;
    UInt8       verticalBlanking;
    UInt8       verticalHigh;
    UInt8       horizSyncOffset;
    UInt8       horizSyncWidth;
    UInt8       verticalSyncOffsetWidth;
    UInt8       syncHigh;
    UInt8       horizImageSize;
    UInt8       verticalImageSize;
    UInt8       imageSizeHigh;
    UInt8       horizBorder;
    UInt8       verticalBorder;
    UInt8       flags;
};
typedef struct EDIDDetailedTimingDesc EDIDDetailedTimingDesc;

struct EDIDGeneralDesc {
    UInt16      flag1;
    UInt8       flag2;
    UInt8       type;
    UInt8       flag3;
    UInt8       data[13];
};
typedef struct EDIDGeneralDesc EDIDGeneralDesc;

union EDIDDesc {
    EDIDDetailedTimingDesc      timing;
    EDIDGeneralDesc             general;
};
typedef union EDIDDesc EDIDDesc;

struct EDID {
    UInt8       header[8];
    UInt8       vendorProduct[4];
    UInt8       serialNumber[4];
    UInt8       weekOfManufacture;
    UInt8       yearOfManufacture;
    UInt8       version;
    UInt8       revision;
    UInt8       displayParams[5];
    UInt8       colorCharacteristics[10];
    UInt8       establishedTimings[3];
    UInt16      standardTimings[8];
    EDIDDesc    descriptors[4];
    UInt8       extension;
    UInt8       checksum;
};
typedef struct EDID EDID;

#define featureSupport  displayParams[4]

struct GTFTimingCurve
{
    UInt32 startHFrequency;
    UInt32 c;
    UInt32 m;
    UInt32 k;
    UInt32 j;
};
typedef struct GTFTimingCurve GTFTimingCurve;

enum {
    kExtTagCEA  = 0x02,
    kExtTagVTB  = 0x10,
    kExtTagDI   = 0x40,
    kExtTagDID  = 0x70
};


struct DIEXT {
    UInt8       header;
    UInt8       version;

    // digital only
    UInt8       standardSupported;
    UInt8       standardVersion[4];
    UInt8       dataFormatDesc;
    UInt8       dataFormats;
    UInt8       minPixelClockPerLink;
    UInt8       maxPixelClockPerLink[2];
    UInt8       crossoverFreq[2];

    // analogue/digital
    UInt8       subPixelLayout;
    UInt8       subPixelConfig;
    UInt8       subPixelShape;
    UInt8       horizontalPitch;
    UInt8       verticalPitch;
    UInt8       majorCapabilities;
    UInt8       miscCapabilities;
    UInt8       frameRateConversion;
    UInt8       convertedVerticalFreq[2];
    UInt8       convertedHorizontalFreq[2];
    UInt8       displayScanOrientation;

    UInt8       colorLuminanceDefault;
    UInt8       colorLuminancePreferred;
    UInt8       colorLuminanceCapabilities[2];

    UInt8       colorDepthFlags;
    UInt8       rgbBitsPerColor[3];
    UInt8       yuvBitsPerColor[3];

    UInt8       aspectRatioConversionModes;

    UInt8       packetizedDigitalVideo[16];
    UInt8       reserved[17];
    UInt8       audio[9];
    UInt8       gamma[46];

    UInt8       checksum;
};
typedef struct DIEXT DIEXT;

struct VTBEXT {
    UInt8       header;
    UInt8       version;
    UInt8       numDetailedTimings;
    UInt8       numCVTTimings;
    UInt8       numStandardTimings;
    UInt8       data[122];
    UInt8       checksum;
};
typedef struct VTBEXT VTBEXT;

struct VTBCVTTimingDesc {
    UInt8       verticalSize;
    UInt8       verticalSizeHigh;
    UInt8       refreshRates;
};
typedef struct VTBCVTTimingDesc VTBCVTTimingDesc;

// VTBCVTTimingDesc.verticalSizeHigh
enum {
    kVTBCVTAspectRatioMask      = 0x0c,
    kVTBCVTAspectRatio4By3      = 0x00,
    kVTBCVTAspectRatio16By9     = 0x04,
    kVTBCVTAspectRatio16By10    = 0x08,
};

// VTBCVTTimingDesc.refreshRates
enum {
    kVTBCVTPreferredRefreshMask = 0x60,
    kVTBCVTPreferredRefresh50   = 0x00,
    kVTBCVTPreferredRefresh60   = 0x20,
    kVTBCVTPreferredRefresh75   = 0x40,
    kVTBCVTPreferredRefresh85   = 0x60,

    kVTBCVTRefresh50            = 0x10,
    kVTBCVTRefresh60            = 0x08,
    kVTBCVTRefresh75            = 0x04,
    kVTBCVTRefresh85            = 0x02,
    kVTBCVTRefresh60RBlank      = 0x01
};

struct CEA861EXT {
    UInt8       header;
    UInt8       version;
    UInt8       detailedTimingsOffset;
    UInt8       flags;
    UInt8       data[123];
    UInt8       checksum;
};
typedef struct CEA861EXT CEA861EXT;

// CEA861EXT.flags (v2)
enum {
    kDTVSupportsUnderscan       = 0x80,
    kDTVSupportsBasicAudio      = 0x40,
    kDTVSupportsYUV444          = 0x20,
    kDTVSupportsYUV422          = 0x10,
    kDTVNumNativeTimings        = 0x0f
};

struct DisplayIDEXT {
    UInt8       header;
    UInt8       version;
    UInt8       sectionSize;
    UInt8       productType;
    UInt8       extensionCount;
    UInt8       section[122];
    UInt8       checksum;
};
typedef struct DisplayIDEXT DisplayIDEXT;

struct DisplayIDBlock {
    UInt8       type;
    UInt8       version;
    UInt8       size;
    UInt8       data[0];
};
typedef struct DisplayIDBlock DisplayIDBlock;

enum {
    kDIDBlockTypeProductIdentification  = 0x00,
    kDIDBlockTypeDetailedType1          = 0x03,
    kDIDBlockTypeVendorSpecific         = 0x7f
};

struct DisplayIDDetailedType1 {
    UInt8       pixelClock[3];
    UInt8       flags;
    UInt8       horizActive[2];
    UInt8       horizBlanking[2];
    UInt8       horizSyncOffset[2];
    UInt8       horizSyncWidth[2];
    UInt8       verticalActive[2];
    UInt8       verticalBlanking[2];
    UInt8       verticalSyncOffset[2];
    UInt8       verticalSyncWidth[2];
};
typedef struct DisplayIDDetailedType1 DisplayIDDetailedType1;

struct DisplayIDBlockDetailedType1 {
    DisplayIDBlock         header;
    DisplayIDDetailedType1 detailed[0];
};
typedef struct DisplayIDBlockDetailedType1 DisplayIDBlockDetailedType1;

__private_extern__ IOReturn
readFile(const char *path, vm_address_t * objAddr, vm_size_t * objSize);

__private_extern__ CFMutableDictionaryRef
readPlist( const char * path, UInt32 key );

__private_extern__ Boolean
writePlist( const char * path, CFMutableDictionaryRef dict, UInt32 key );

__private_extern__ Boolean
IODisplayEDIDName( EDID * edid, char * name );


