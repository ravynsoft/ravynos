//
//  lf_hfs_defs.h
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_defs_h
#define lf_hfs_defs_h

#include <MacTypes.h>
#include "lf_hfs_vnode.h"

typedef const unsigned char * ConstUTF8Param;
typedef struct vnode* FileReference;
typedef const UniChar *        ConstUniCharArrayPtr;

enum {
    dskFulErr               = -34,          /*disk full*/
    bdNamErr                = -37,          /*there may be no bad names in the final system!*/
    paramErr                = -50,          /*error in user parameter list*/
    memFullErr              = -108,         /*Not enough room in heap zone*/
    fileBoundsErr           = -1309,        /*file's EOF, offset, mark or size is too big*/
    kTECUsedFallbacksStatus = -8783,

};

enum {
    fsRtParID    = 1,
    fsRtDirID    = 2
};

enum {
    /* Mac OS encodings*/
    kTextEncodingMacRoman        = 0L,
    kTextEncodingMacJapanese    = 1,
    kTextEncodingMacChineseTrad    = 2,
    kTextEncodingMacKorean        = 3,
    kTextEncodingMacArabic        = 4,
    kTextEncodingMacHebrew        = 5,
    kTextEncodingMacGreek        = 6,
    kTextEncodingMacCyrillic    = 7,
    kTextEncodingMacDevanagari    = 9,
    kTextEncodingMacGurmukhi    = 10,
    kTextEncodingMacGujarati    = 11,
    kTextEncodingMacOriya        = 12,
    kTextEncodingMacBengali        = 13,
    kTextEncodingMacTamil        = 14,
    kTextEncodingMacTelugu        = 15,
    kTextEncodingMacKannada        = 16,
    kTextEncodingMacMalayalam    = 17,
    kTextEncodingMacSinhalese    = 18,
    kTextEncodingMacBurmese        = 19,
    kTextEncodingMacKhmer        = 20,
    kTextEncodingMacThai        = 21,
    kTextEncodingMacLaotian        = 22,
    kTextEncodingMacGeorgian    = 23,
    kTextEncodingMacArmenian    = 24,
    kTextEncodingMacChineseSimp    = 25,
    kTextEncodingMacTibetan        = 26,
    kTextEncodingMacMongolian    = 27,
    kTextEncodingMacEthiopic    = 28,
    kTextEncodingMacCentralEurRoman = 29,
    kTextEncodingMacVietnamese    = 30,
    kTextEncodingMacExtArabic    = 31,    /* The following use script code 0, smRoman*/
    kTextEncodingMacSymbol        = 33,
    kTextEncodingMacDingbats    = 34,
    kTextEncodingMacTurkish        = 35,
    kTextEncodingMacCroatian    = 36,
    kTextEncodingMacIcelandic    = 37,
    kTextEncodingMacRomanian    = 38,
    kTextEncodingMacUnicode        = 0x7E,

    kTextEncodingMacFarsi        = 0x8C,    /* Like MacArabic but uses Farsi digits */                                                        /* The following use script code 7, smCyrillic */
    kTextEncodingMacUkrainian    = 0x98,    /* The following use script code 32, smUnimplemented */
};

#if DEBUG
void RequireFileLock(FileReference vp, int shareable);
#define REQUIRE_FILE_LOCK(vp,s) RequireFileLock((vp),(s))
#else
#define REQUIRE_FILE_LOCK(vp,s)
#endif

#define BlockMoveData(src, dest, len)    bcopy((src), (dest), (len))

#define ClearMemory(start, length)    bzero((start), (size_t)(length));

enum {
    /* Finder Flags */
    kHasBeenInited        = 0x0100,
    kHasCustomIcon        = 0x0400,
    kIsStationery        = 0x0800,
    kNameLocked        = 0x1000,
    kHasBundle        = 0x2000,
    kIsInvisible        = 0x4000,
    kIsAlias        = 0x8000
};
#endif /* lf_hfs_defs_h */
