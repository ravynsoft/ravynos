/*
 * Copyright (c) 1998-2014 Apple Inc. All rights reserved.
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

#ifndef	_IOCDTYPES_H
#define	_IOCDTYPES_H

#include <IOKit/IOTypes.h>
#include <libkern/OSByteOrder.h>

#pragma pack(push, 1)                        /* (enable 8-bit struct packing) */

/*
 * Minutes, Seconds, Frames (M:S:F)
 *
 * All M:S:F values passed across I/O Kit APIs are guaranteed to be
 * binary-encoded numbers (no BCD-encoded numbers are ever passed).
 */

typedef struct
{
    UInt8 minute;
    UInt8 second;
    UInt8 frame;
} CDMSF;

/*
 * Media Catalogue Numbers (MCN), International Standard Recording Codes (ISRC)
 *
 * All MCN and ISRC values passed across I/O Kit APIs are guaranteed
 * to have a zero-terminating byte, for convenient use as C strings.
 */

#define kCDMCNMaxLength  13
#define kCDISRCMaxLength 12

typedef char CDMCN [kCDMCNMaxLength  + 1];
typedef char CDISRC[kCDISRCMaxLength + 1];

/*
 * Audio Status
 *
 * All CDAudioStatus fields passed across I/O Kit APIs are guaranteed to
 * be binary-encoded numbers (no BCD-encoded numbers are ever passed).
 */

#define kCDAudioStatusUnsupported 0x00
#define kCDAudioStatusActive      0x11
#define kCDAudioStatusPaused      0x12
#define kCDAudioStatusSuccess     0x13
#define kCDAudioStatusFailure     0x14
#define kCDAudioStatusNone        0x15

typedef struct
{
    UInt8 status;
    struct
    {
        CDMSF time;
        struct
        {
            UInt8 index;
            UInt8 number;
            CDMSF time;
        } track;
    } position;
} CDAudioStatus;

/*
 * Table Of Contents
 *
 * All CDTOC fields passed across I/O Kit APIs are guaranteed to be
 * binary-encoded numbers (no BCD-encoded numbers are ever passed).
 */

typedef struct
{
    UInt8 session;
#ifdef __LITTLE_ENDIAN__
    UInt8 control:4, adr:4;
#else /* !__LITTLE_ENDIAN__ */
    UInt8 adr:4, control:4;
#endif /* !__LITTLE_ENDIAN__ */
    UInt8 tno;
    UInt8 point;
    CDMSF address;
    UInt8 zero;
    CDMSF p;
} CDTOCDescriptor;

typedef struct
{
    UInt16          length;
    UInt8           sessionFirst;
    UInt8           sessionLast;
    CDTOCDescriptor descriptors[0];
} CDTOC;

/*
 * Table Of Contents Descriptor Count Convenience Function
 */

static inline UInt32 CDTOCGetDescriptorCount(CDTOC * toc)
{
    UInt32 tocSize = OSSwapBigToHostInt16(toc->length) + (UInt32) sizeof(toc->length);

    return (tocSize < (UInt32) sizeof(CDTOC)) ? 0 : 
           (tocSize - (UInt32) sizeof(CDTOC)) / (UInt32) sizeof(CDTOCDescriptor);
}

/*
 * M:S:F To LBA Convenience Function
 */

static inline UInt32 CDConvertMSFToLBA(CDMSF msf)
{
    return (((msf.minute * 60U) + msf.second) * 75U) + msf.frame - 150U;
}

/*
 * M:S:F To Clipped LBA Convenience Function
 */

static inline UInt32 CDConvertMSFToClippedLBA(CDMSF msf)
{
    return (msf.minute == 0 && msf.second <= 1) ? 0 : CDConvertMSFToLBA(msf);
}

/*
 * LBA To M:S:F Convenience Function
 */

static inline CDMSF CDConvertLBAToMSF(UInt32 lba)
{
    CDMSF msf;

    lba += 150;
    msf.minute = (lba / (75 * 60));
    msf.second = (lba % (75 * 60)) / 75;
    msf.frame  = (lba % (75     ));
    
    return msf;
}

/*
 * Track Number To M:S:F Convenience Function
 *
 * The CDTOC structure is assumed to be complete, that is, none of
 * the descriptors are missing or clipped due to an insufficiently
 * sized buffer holding the CDTOC contents.
 */

static inline CDMSF CDConvertTrackNumberToMSF(UInt8 track, CDTOC * toc)
{
    UInt32 count = CDTOCGetDescriptorCount(toc);
    UInt32 i;
    CDMSF  msf   = { 0xFF, 0xFF, 0xFF };

    for (i = 0; i < count; i++)
    {
        if (toc->descriptors[i].point == track && toc->descriptors[i].adr == 1)
        {
            msf = toc->descriptors[i].p;
            break;
        }
    }

    return msf;
}

/*
 * Sector Areas, Sector Types
 *
 * Bytes Per Type      CDDA       Mode1      Mode2   Mode2Form1 Mode2Form2
 *       Per Area  +----------+----------+----------+----------+----------+
 * Sync            | 0        | 12       | 12       | 12       | 12       |
 * Header          | 0        | 4        | 4        | 4        | 4        |
 * SubHeader       | 0        | 0        | 0        | 8        | 8        |
 * User            | 2352     | 2048     | 2336     | 2048     | 2328     |
 * Auxiliary       | 0        | 288      | 0        | 280      | 0        |
 * ErrorFlags      | 294      | 294      | 294      | 294      | 294      |
 * SubChannel      | 96       | 96       | 96       | 96       | 96       |
 * SubChannelQ     | 16       | 16       | 16       | 16       | 16       |
 *                 +----------+----------+----------+----------+----------+
 */

typedef enum
{
    kCDSectorAreaSync        = 0x80,
    kCDSectorAreaHeader      = 0x20,
    kCDSectorAreaSubHeader   = 0x40,
    kCDSectorAreaUser        = 0x10,
    kCDSectorAreaAuxiliary   = 0x08,
    kCDSectorAreaErrorFlags  = 0x02,
    kCDSectorAreaSubChannel  = 0x01,
    kCDSectorAreaSubChannelQ = 0x04
} CDSectorArea;

typedef enum
{
    kCDSectorTypeUnknown     = 0x00,
    kCDSectorTypeCDDA        = 0x01,
    kCDSectorTypeMode1       = 0x02,
    kCDSectorTypeMode2       = 0x03,
    kCDSectorTypeMode2Form1  = 0x04,
    kCDSectorTypeMode2Form2  = 0x05,
    kCDSectorTypeCount       = 0x06
} CDSectorType;

typedef enum
{
    kCDSectorSizeCDDA        = 2352,
    kCDSectorSizeMode1       = 2048,
    kCDSectorSizeMode2       = 2336,
    kCDSectorSizeMode2Form1  = 2048,
    kCDSectorSizeMode2Form2  = 2328,
    kCDSectorSizeWhole       = 2352
} CDSectorSize;

/*
 * Media Types
 */

enum
{
    kCDMediaTypeUnknown      = 0x0100,
    kCDMediaTypeROM          = 0x0102, /* CD-ROM */
    kCDMediaTypeR            = 0x0104, /* CD-R   */
    kCDMediaTypeRW           = 0x0105, /* CD-RW  */

    kCDMediaTypeMin          = 0x0100,
    kCDMediaTypeMax          = 0x01FF
};

typedef UInt32 CDMediaType;

/*
 * Media Speed (kB/s)
 */

#define kCDSpeedMin 0x00B0
#define kCDSpeedMax 0xFFFF

/*
 * MMC Formats
 */

// Read Table Of Contents Format Types
typedef UInt8 CDTOCFormat;
enum
{
    kCDTOCFormatTOC  = 0x02, // CDTOC
    kCDTOCFormatPMA  = 0x03, // CDPMA
    kCDTOCFormatATIP = 0x04, // CDATIP
    kCDTOCFormatTEXT = 0x05  // CDTEXT
};

// Read Table Of Contents Format 0x03
struct CDPMADescriptor
{
    UInt8 reserved;
#ifdef __LITTLE_ENDIAN__
    UInt8 control:4, adr:4;
#else /* !__LITTLE_ENDIAN__ */
    UInt8 adr:4, control:4;
#endif /* !__LITTLE_ENDIAN__ */
    UInt8 tno;
    UInt8 point;
    CDMSF address;
    UInt8 zero;
    CDMSF p;
};
typedef struct CDPMADescriptor CDPMADescriptor;

struct CDPMA
{
    UInt16          dataLength;
    UInt8           reserved;
    UInt8           reserved2;
    CDPMADescriptor descriptors[0];
};
typedef struct CDPMA CDPMA;

// Read Table Of Contents Format 0x04
struct CDATIP
{
    UInt16 dataLength;
    UInt8  reserved[2];
#ifdef __LITTLE_ENDIAN__
    UInt8  referenceSpeed:3;
    UInt8  reserved3:1;
    UInt8  indicativeTargetWritingPower:3;
    UInt8  reserved2:1;

    UInt8  reserved5:6;
    UInt8  unrestrictedUse:1;
    UInt8  reserved4:1;

    UInt8  a3Valid:1;
    UInt8  a2Valid:1;
    UInt8  a1Valid:1;
    UInt8  discSubType:3;
    UInt8  discType:1;
    UInt8  reserved6:1;
#else /* !__LITTLE_ENDIAN__ */
    UInt8  reserved2:1;
    UInt8  indicativeTargetWritingPower:3;
    UInt8  reserved3:1;
    UInt8  referenceSpeed:3;

    UInt8  reserved4:1;
    UInt8  unrestrictedUse:1;
    UInt8  reserved5:6;

    UInt8  reserved6:1;
    UInt8  discType:1;
    UInt8  discSubType:3;
    UInt8  a1Valid:1;
    UInt8  a2Valid:1;
    UInt8  a3Valid:1;
#endif /* !__LITTLE_ENDIAN__ */
    UInt8  reserved7;
    CDMSF  startTimeOfLeadIn;
    UInt8  reserved8;
    CDMSF  lastPossibleStartTimeOfLeadOut;
    UInt8  reserved9;
    UInt8  a1[3];
    UInt8  reserved10;
    UInt8  a2[3];
    UInt8  reserved11;
    UInt8  a3[3];
    UInt8  reserved12;
};
typedef struct CDATIP CDATIP;

// Read Table Of Contents Format 0x05
struct CDTEXTDescriptor
{
    UInt8 packType;
    UInt8 trackNumber;
    UInt8 sequenceNumber;
#ifdef __LITTLE_ENDIAN__
    UInt8 characterPosition:4;
    UInt8 blockNumber:3;
    UInt8 doubleByteCharacterCode:1;
#else /* !__LITTLE_ENDIAN__ */
    UInt8 doubleByteCharacterCode:1;
    UInt8 blockNumber:3;
    UInt8 characterPosition:4;
#endif /* !__LITTLE_ENDIAN__ */
    UInt8 textData[12];
    UInt8 reserved[2];
};
typedef struct CDTEXTDescriptor CDTEXTDescriptor;

struct CDTEXT
{
    UInt16           dataLength;
    UInt8            reserved;
    UInt8            reserved2;
    CDTEXTDescriptor descriptors[0];
};
typedef struct CDTEXT CDTEXT;

// Read Disc Information Format
struct CDDiscInfo
{
    UInt16 dataLength;
#ifdef __LITTLE_ENDIAN__
    UInt8  discStatus:2;
    UInt8  stateOfLastSession:2;
    UInt8  erasable:1;
    UInt8  reserved:3;
#else /* !__LITTLE_ENDIAN__ */
    UInt8  reserved:3;
    UInt8  erasable:1;
    UInt8  stateOfLastSession:2;
    UInt8  discStatus:2;
#endif /* !__LITTLE_ENDIAN__ */
    UInt8  numberOfFirstTrack;
    UInt8  numberOfSessionsLSB;
    UInt8  firstTrackNumberInLastSessionLSB;
    UInt8  lastTrackNumberInLastSessionLSB;
#ifdef __LITTLE_ENDIAN__
    UInt8  reserved3:5;
    UInt8  unrestrictedUse:1;
    UInt8  discBarCodeValid:1;
    UInt8  discIdentificationValid:1;
#else /* !__LITTLE_ENDIAN__ */
    UInt8  discIdentificationValid:1;
    UInt8  discBarCodeValid:1;
    UInt8  unrestrictedUse:1;
    UInt8  reserved3:5;
#endif /* !__LITTLE_ENDIAN__ */
    UInt8  discType;
    UInt8  numberOfSessionsMSB;
    UInt8  firstTrackNumberInLastSessionMSB;
    UInt8  lastTrackNumberInLastSessionMSB;
    UInt32 discIdentification;
    UInt8  reserved7;
    CDMSF  lastSessionLeadInStartTime;
    UInt8  reserved8;
    CDMSF  lastPossibleStartTimeOfLeadOut;
    UInt8  discBarCode[8];
    UInt8  reserved9;
    UInt8  numberOfOPCTableEntries;
    UInt8  opcTableEntries[0];
};
typedef struct CDDiscInfo CDDiscInfo;

// Read Track Information Address Types
typedef UInt8 CDTrackInfoAddressType;
enum
{
    kCDTrackInfoAddressTypeLBA           = 0x00,
    kCDTrackInfoAddressTypeTrackNumber   = 0x01,
    kCDTrackInfoAddressTypeSessionNumber = 0x02,
};

// Read Track Information Format
struct CDTrackInfo
{
    UInt16 dataLength;
    UInt8  trackNumberLSB;
    UInt8  sessionNumberLSB;
    UInt8  reserved;
#ifdef __LITTLE_ENDIAN__
    UInt8  trackMode:4;
    UInt8  copy:1;
    UInt8  damage:1;
    UInt8  reserved3:2;

    UInt8  dataMode:4;
    UInt8  fixedPacket:1;
    UInt8  packet:1;
    UInt8  blank:1;
    UInt8  reservedTrack:1;

    UInt8  nextWritableAddressValid:1;
    UInt8  lastRecordedAddressValid:1;
    UInt8  reserved5:6;
#else /* !__LITTLE_ENDIAN__ */
    UInt8  reserved3:2;
    UInt8  damage:1;
    UInt8  copy:1;
    UInt8  trackMode:4;

    UInt8  reservedTrack:1;
    UInt8  blank:1;
    UInt8  packet:1;
    UInt8  fixedPacket:1;
    UInt8  dataMode:4;

    UInt8  reserved5:6;
    UInt8  lastRecordedAddressValid:1;
    UInt8  nextWritableAddressValid:1;
#endif /* !__LITTLE_ENDIAN__ */
    UInt32 trackStartAddress;
    UInt32 nextWritableAddress;
    UInt32 freeBlocks;
    UInt32 fixedPacketSize;
    UInt32 trackSize;
    UInt32 lastRecordedAddress;
    UInt8  trackNumberMSB;
    UInt8  sessionNumberMSB;
    UInt8  reserved6;
    UInt8  reserved7;
};
typedef struct CDTrackInfo CDTrackInfo;

#pragma pack(pop)                        /* (reset to default struct packing) */

#endif /* _IOCDTYPES_H */
