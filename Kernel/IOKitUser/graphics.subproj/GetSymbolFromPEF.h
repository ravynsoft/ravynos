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
/*
    File:        GetSymbolFromPEF.h

    Contains:    xxx put contents here xxx

    Written by:    Jeffrey Robbin

    Copyright:    © 1994 by Apple Computer, Inc., all rights reserved.

    Change History (most recent first):

         <2>     9/20/94    TS        Dump the globals!
         <1>      9/6/94    TS        first checked in

*/

#ifndef __GETSYMBOLFROMPEF__
#define __GETSYMBOLFROMPEF__

//#include <Types.h>

// #pragma options align=mac68k

#define NewPtrSys(a)    malloc(a)
#define DisposePtr(a)    free(a)

/*
    Container information
*/

struct ContainerHeader {               // File/container header layout:
    uint32_t    magicCookie;      // PEF container magic cookie
    uint32_t    containerID;      // 'peff'
    uint32_t    architectureID;   // 'pwpc' | 'm68k'
    uint32_t    versionNumber;    // format version number
    uint32_t    dateTimeStamp;    // date/time stamp (Mac format)
    uint32_t    oldDefVersion;    // old definition version number
    uint32_t    oldImpVersion;    // old implementation version number
    uint32_t    currentVersion;   // current version number
    uint16_t    nbrOfSections;    // nbr of sections (rel. 1)
    uint16_t    loadableSections; // nbr of loadable sectionsfor execution
                                       //   (= nbr of 1st non-loadable section)
    LogicalAddress    memoryAddress;   // location this container was last loaded
};
typedef struct ContainerHeader ContainerHeader, *ContainerHeaderPtr;

#define  kMagic1  'joy!'
#define  kMagic2  'peff'


/*
    Section information
*/

struct SectionHeader {               // Section header layout:
    int32_t   sectionName;     // section name (str tbl container offset)
    uint32_t  sectionAddress;  // preferred base address for the section
    int32_t   execSize;        // execution (byte) size including 0 init)
    int32_t   initSize;        // init (byte) size before 0 init
    int32_t   rawSize;         // raw data size (bytes)
    int32_t   containerOffset; // container offest to section's raw data
    uint8_t   regionKind;      // section/region classification
    uint8_t   shareKind;       // sharing classification
    uint8_t   alignment;       // log 2 alignment
    uint8_t   reserved;        // <reserved>
};
typedef struct SectionHeader SectionHeader, *SectionHeaderPtr;

/* regionKind section classification: */
/* loadable sections */
#define kCodeSection      0U  /* code section */
#define kDataSection      1U  /* data section */
#define kPIDataSection    2U  /* "pattern" initialized data */
#define kConstantSection  3U  /* read-only data */
#define kExecDataSection  6U  /* "self modifying" code (!?) */
/* non-loadable sections */
#define kLoaderSection    4U  /* loader */
#define kDebugSection     5U  /* debugging info */
#define kExceptionSection 7U  /* exception data */
#define kTracebackSection 8U  /* traceback data */


/*
    Loader Information
*/

struct LoaderHeader {     // Loader raw data section header layout:
    int32_t entrySection;    // entry point section number
    int32_t entryOffset;     // entry point descr. ldr section offset
    int32_t initSection;     // init routine section number
    int32_t initOffset;      // init routine descr. ldr section offset
    int32_t termSection;     // term routine section number
    int32_t termOffset;      // term routine descr. ldr section offset
    int32_t nbrImportIDs;    // nbr of import container id entries
    int32_t nbrImportSyms;   // nbr of import symbol table entries
    int32_t nbrRelocSects;   // nbr of relocation sections (headers)
    int32_t relocsOffset;    // reloc. instructions ldr section offset
    int32_t strTblOffset;    // string table ldr section offset
    int32_t slotTblOffset; // hash slot table ldr section offset
    int32_t hashSlotTblSz;   // log 2 of nbr of hash slot entries
    int32_t nbrExportSyms;   // nbr of export symbol table entries
};
typedef struct LoaderHeader LoaderHeader, *LoaderHeaderPtr;

struct LoaderHashSlotEntry { // Loader export hash slot table entry layout:
    uint32_t  slotEntry; // chain count (0:13), chain index (14:31)
};
typedef struct LoaderHashSlotEntry LoaderHashSlotEntry, *LoaderHashSlotEntryPtr;

struct LoaderExportChainEntry { // Loader export hash chain tbl entry layout:
    union {
        uint32_t  _hashWord; // name length and hash value
        struct {
            unsigned short _nameLength; // name length is top half of hash word
            unsigned short doNotUseThis; // this field should never be accessed!
        } _h_h;
    } _h;
};
typedef struct LoaderExportChainEntry LoaderExportChainEntry, *LoaderExportChainEntryPtr;

struct ExportSymbolEntry {        // Loader export symbol table entry layout:
    uint32_t  class_and_name; // symClass (0:7), nameOffset (8:31)
    int32_t address;                 // ldr section offset to exported symbol
    short sectionNumber;          // section nbr that this export belongs to
};
typedef struct ExportSymbolEntry ExportSymbolEntry, *ExportSymbolEntryPtr;



/*
    Unpacking Information
*/

 /* "pattern" initialized data opcodes: */
#define kZero              0U     /* zero (clear) bytes */
#define kBlock             1U     /* block transfer bytes */
#define kRepeat            2U     /* repeat block xfer bytes */
#define kRepeatBlock       3U     /* repeat block xfer with contant prefix */
#define kRepeatZero        4U     /* repeat block xfer with contant prefix 0 */
#define kOpcodeShift       0x05U  /* shift to access opcode */
#define kFirstOperandMask  0x1FU  /* mask to access 1st operand (count) */

#define PIOP(x)  (unsigned char)((x) >> kOpcodeShift)  /* extract opcode */
#define PICNT(x)  (int)((x) & kFirstOperandMask) /* extract 1st operand (count) */

/* The following macros are used for extracting count value operands from pidata...            */

#define kCountShift  0x07UL  /* value shift to concat count bytes */
#define kCountMask   0x7FUL  /* mask to extract count bits from a byte */
#define IS_LAST_PICNT_BYTE(x) (((x) & 0x80U) == 0) /* true if last count byte */
#define CONCAT_PICNT(value, x) (((value)<<kCountShift) | ((x) & kCountMask))



/*
    Function Prototypes
*/

static OSStatus GetSymbolFromPEF(
    char *theSymbolName,
    LogicalAddress thePEFPtr,
    LogicalAddress theSymbolPtr,
    ByteCount      theSymbolSize);

static Boolean SymbolCompare(
    StringPtr     theLookedForSymbol,
    StringPtr     theExportSymbol,
    uint32_t  theExportSymbolLength);
                        
static OSErr UnpackPiData(
    LogicalAddress   thePEFPtr,
    SectionHeaderPtr sectionHeaderPtr,
    LogicalAddress * theData);

static unsigned char PEFGetNextByte(
    unsigned char ** rawBuffer,
    int32_t * rawBufferRemaining);

static uint32_t  PEFGetCount(
    unsigned char ** rawBuffer,
    int * rawBufferRemaining);


// #pragma options align=reset

#endif
