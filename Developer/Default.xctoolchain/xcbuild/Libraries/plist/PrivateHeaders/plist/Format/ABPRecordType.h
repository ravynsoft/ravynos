/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#ifndef __plist_Format_ABPRecordType_h
#define __plist_Format_ABPRecordType_h

#include <cstdio>
#include <cstdint>

typedef enum _ABPRecordType {
    kABPRecordTypeInvalid = -1,
    kABPRecordTypeNull = 0,
    kABPRecordTypeBoolTrue,
    kABPRecordTypeBoolFalse,
    kABPRecordTypeFill,
    kABPRecordTypeDate,
    kABPRecordTypeInteger,
    kABPRecordTypeReal,
    kABPRecordTypeData,
    kABPRecordTypeStringASCII,
    kABPRecordTypeStringUnicode,
    kABPRecordTypeUid,
    kABPRecordTypeArray,
    kABPRecordTypeDictionary
} ABPRecordType;

static inline ABPRecordType
__ABPByteToRecordType(uint8_t byte)
{
    if ((byte & 0xf0) == 0 || byte == 0x33) {
        switch (byte) {
            case 0x00: /* 0000 0000: null */
                return kABPRecordTypeNull;
            case 0x08: /* 0000 1000: false */
                return kABPRecordTypeBoolFalse;
            case 0x09: /* 0000 1001: true */
                return kABPRecordTypeBoolTrue;
            case 0x0f: /* 0000 1111: fill byte */
                return kABPRecordTypeFill;
            case 0x33: /* 0011 0011: date (double be) */
                return kABPRecordTypeDate;
            default:
                return kABPRecordTypeInvalid;
        }
    } else {
        switch (byte >> 4) {
            case 0x1: /* 0001 nnnn: int */
                return kABPRecordTypeInteger;
            case 0x2: /* 0010 nnnn: real */
                return kABPRecordTypeReal;
            case 0x4: /* 0100 nnnn: data */
                return kABPRecordTypeData;
            case 0x5: /* 0101 nnnn: ascii string */
                return kABPRecordTypeStringASCII;
            case 0x6: /* 0110 nnnn: unicode string */
                return kABPRecordTypeStringUnicode;
            case 0x8: /* 1000 nnnn: uid */
                return kABPRecordTypeUid;
            case 0xa: /* 1010 nnnn: array */
                return kABPRecordTypeArray;
            case 0xd: /* 1101 nnnn: dictionary */
                return kABPRecordTypeDictionary;
            default:
                return kABPRecordTypeInvalid;
        }
    }
}

static inline int
__ABPRecordTypeToByte(ABPRecordType type, size_t size)
{
    switch (type) {
        case kABPRecordTypeNull:
            return 0x00; /* 0000 0000: null */
        case kABPRecordTypeBoolFalse:
            return 0x08; /* 0000 1000: false */
        case kABPRecordTypeBoolTrue:
            return 0x09; /* 0000 1001: true */
        case kABPRecordTypeFill:
            return 0x0f; /* 0000 1111: fill byte */
        case kABPRecordTypeDate:
            return 0x33; /* 0011 0011: date (double be) */
        case kABPRecordTypeInteger:
            return 0x10 | (size & 0xf); /* 0001 nnnn: int */
        case kABPRecordTypeReal:
            return 0x20 | (size & 0xf); /* 0010 nnnn: real */
        case kABPRecordTypeData:
            if (size > 15) size = 0xf;
            return 0x40 | (size & 0xf); /* 0100 nnnn: data */
        case kABPRecordTypeStringASCII:
            if (size > 15) size = 0xf;
            return 0x50 | (size & 0xf); /* 0101 nnnn: ascii string */
        case kABPRecordTypeStringUnicode:
            if (size > 15) size = 0xf;
            return 0x60 | (size & 0xf); /* 0110 nnnn: unicode string */
        case kABPRecordTypeUid:
            if (size > 16) size = 0xf; else size--;
            return 0x80 | (size & 0xf); /* 1000 nnnn: uid */
        case kABPRecordTypeArray:
            if (size > 15) size = 0xf;
            return 0xa0 | (size & 0xf); /* 1010 nnnn: array */
        case kABPRecordTypeDictionary:
            if (size > 15) size = 0xf;
            return 0xd0 | (size & 0xf); /* 1101 nnnn: dictionary */
        default:
            return EOF;
    }
}

#endif  /* !__plist_Format_ABPRecordType_h */
