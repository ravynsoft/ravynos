// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
***************************************************************************
*   Copyright (C) 1999-2014 International Business Machines Corporation   *
*   and others. All rights reserved.                                      *
***************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "unicode/utypes.h"
#include "rbbidata57.h"
#include "rbbirb57.h"
#include "utrie.h"
#include "udatamem.h"
#include "cmemory.h"
#include "cstring.h"
#include "umutex.h"

#include "uassert.h"


//-----------------------------------------------------------------------------------
//
//   Trie access folding function.  Copied as-is from properties code in uchar.c
//
//-----------------------------------------------------------------------------------
U_CDECL_BEGIN
static int32_t U_CALLCONV
getFoldingOffset(uint32_t data) {
    /* if bit 15 is set, then the folding offset is in bits 14..0 of the 16-bit trie result */
    if(data&0x8000) {
        return (int32_t)(data&0x7fff);
    } else {
        return 0;
    }
}
U_CDECL_END

U_NAMESPACE_BEGIN

//-----------------------------------------------------------------------------
//
//    Constructors.
//
//-----------------------------------------------------------------------------
RBBIDataWrapper57::RBBIDataWrapper57(const RBBIDataHeader57 *data, UErrorCode &status) {
    init0();
    init(data, status);
}

RBBIDataWrapper57::RBBIDataWrapper57(const RBBIDataHeader57 *data, enum EDontAdopt, UErrorCode &status) {
    init0();
    init(data, status);
    fDontFreeData = TRUE;
}

RBBIDataWrapper57::RBBIDataWrapper57(UDataMemory* udm, UErrorCode &status) {
    init0();
    if (U_FAILURE(status)) {
        return;
    }
    const DataHeader *dh = udm->pHeader;
    int32_t headerSize = dh->dataHeader.headerSize;
    if (  !(headerSize >= 20 &&
            dh->info.isBigEndian == U_IS_BIG_ENDIAN &&
            dh->info.charsetFamily == U_CHARSET_FAMILY &&
            dh->info.dataFormat[0] == 0x42 &&  // dataFormat="Brk "
            dh->info.dataFormat[1] == 0x72 &&
            dh->info.dataFormat[2] == 0x6b &&
            dh->info.dataFormat[3] == 0x20)
            // Note: info.fFormatVersion is duplicated in the RBBIDataHeader57, and is
            //       validated when checking that.
        ) {
        status = U_INVALID_FORMAT_ERROR;
        return;
    }
    const char *dataAsBytes = reinterpret_cast<const char *>(dh);
    const RBBIDataHeader57 *rbbidh = reinterpret_cast<const RBBIDataHeader57 *>(dataAsBytes + headerSize);
    init(rbbidh, status);
    fUDataMem = udm;
}

//-----------------------------------------------------------------------------
//
//    init().   Does most of the work of construction, shared between the
//              constructors.
//
//-----------------------------------------------------------------------------
void RBBIDataWrapper57::init0() {
    fHeader = NULL;
    fForwardTable = NULL;
    fReverseTable = NULL;
    fSafeFwdTable = NULL;
    fSafeRevTable = NULL;
    fRuleSource = NULL;
    fRuleStatusTable = NULL;
    fUDataMem = NULL;
    fRefCount = 0;
    fDontFreeData = TRUE;
}

void RBBIDataWrapper57::init(const RBBIDataHeader57 *data, UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }
    fHeader = data;
    if (fHeader->fMagic != 0xb1a0 || fHeader->fFormatVersion[0] != 3) 
    {
        status = U_INVALID_FORMAT_ERROR;
        return;
    }
    // Note: in ICU version 3.2 and earlier, there was a formatVersion 1
    //       that is no longer supported.  At that time fFormatVersion was
    //       an int32_t field, rather than an array of 4 bytes.

    fDontFreeData = FALSE;
    if (data->fFTableLen != 0) {
        fForwardTable = (RBBIStateTable *)((char *)data + fHeader->fFTable);
    }
    if (data->fRTableLen != 0) {
        fReverseTable = (RBBIStateTable *)((char *)data + fHeader->fRTable);
    }
    if (data->fSFTableLen != 0) {
        fSafeFwdTable = (RBBIStateTable *)((char *)data + fHeader->fSFTable);
    }
    if (data->fSRTableLen != 0) {
        fSafeRevTable = (RBBIStateTable *)((char *)data + fHeader->fSRTable);
    }


    utrie_unserialize(&fTrie,
                       (uint8_t *)data + fHeader->fTrie,
                       fHeader->fTrieLen,
                       &status);
    if (U_FAILURE(status)) {
        return;
    }
    fTrie.getFoldingOffset=getFoldingOffset;


    fRuleSource   = (UChar *)((char *)data + fHeader->fRuleSource);
    fRuleString.setTo(TRUE, fRuleSource, -1);
    U_ASSERT(data->fRuleSourceLen > 0);

    fRuleStatusTable = (int32_t *)((char *)data + fHeader->fStatusTable);
    fStatusMaxIdx    = data->fStatusTableLen / sizeof(int32_t);

    fRefCount = 1;

#ifdef RBBI_DEBUG
    char *debugEnv = getenv("U_RBBIDEBUG");
    if (debugEnv && uprv_strstr(debugEnv, "data")) {this->printData();}
#endif
}


//-----------------------------------------------------------------------------
//
//    Destructor.     Don't call this - use removeReference() instead.
//
//-----------------------------------------------------------------------------
RBBIDataWrapper57::~RBBIDataWrapper57() {
    U_ASSERT(fRefCount == 0);
    if (fUDataMem) {
        udata_close(fUDataMem);
    } else if (!fDontFreeData) {
        uprv_free((void *)fHeader);
    }
}



//-----------------------------------------------------------------------------
//
//   Operator ==    Consider two RBBIDataWrappers to be equal if they
//                  refer to the same underlying data.  Although
//                  the data wrappers are normally shared between
//                  iterator instances, it's possible to independently
//                  open the same data twice, and get two instances, which
//                  should still be ==.
//
//-----------------------------------------------------------------------------
UBool RBBIDataWrapper57::operator ==(const RBBIDataWrapper57 &other) const {
    if (fHeader == other.fHeader) {
        return TRUE;
    }
    if (fHeader->fLength != other.fHeader->fLength) {
        return FALSE;
    }
    if (uprv_memcmp(fHeader, other.fHeader, fHeader->fLength) == 0) {
        return TRUE;
    }
    return FALSE;
}

int32_t  RBBIDataWrapper57::hashCode() {
    return fHeader->fFTableLen;
}



//-----------------------------------------------------------------------------
//
//    Reference Counting.   A single RBBIDataWrapper57 object is shared among
//                          however many RulesBasedBreakIterator instances are
//                          referencing the same data.
//
//-----------------------------------------------------------------------------
void RBBIDataWrapper57::removeReference() {
    if (umtx_atomic_dec(&fRefCount) == 0) {
        delete this;
    }
}


RBBIDataWrapper57 *RBBIDataWrapper57::addReference() {
   umtx_atomic_inc(&fRefCount);
   return this;
}



//-----------------------------------------------------------------------------
//
//  getRuleSourceString
//
//-----------------------------------------------------------------------------
const UnicodeString &RBBIDataWrapper57::getRuleSourceString() const {
    return fRuleString;
}


//-----------------------------------------------------------------------------
//
//  print   -  debugging function to dump the runtime data tables.
//
//-----------------------------------------------------------------------------
#ifdef RBBI_DEBUG
void  RBBIDataWrapper57::printTable(const char *heading, const RBBIStateTable *table) {
    uint32_t   c;
    uint32_t   s;

    RBBIDebugPrintf("   %s\n", heading);

    RBBIDebugPrintf("State |  Acc  LA TagIx");
    for (c=0; c<fHeader->fCatCount; c++) {RBBIDebugPrintf("%3d ", c);}
    RBBIDebugPrintf("\n------|---------------"); for (c=0;c<fHeader->fCatCount; c++) {
        RBBIDebugPrintf("----");
    }
    RBBIDebugPrintf("\n");

    if (table == NULL) {
        RBBIDebugPrintf("         N U L L   T A B L E\n\n");
        return;
    }
    for (s=0; s<table->fNumStates; s++) {
        RBBIStateTableRow *row = (RBBIStateTableRow *)
                                  (table->fTableData + (table->fRowLen * s));
        RBBIDebugPrintf("%4d  |  %3d %3d %3d ", s, row->fAccepting, row->fLookAhead, row->fTagIdx);
        for (c=0; c<fHeader->fCatCount; c++)  {
            RBBIDebugPrintf("%3d ", row->fNextState[c]);
        }
        RBBIDebugPrintf("\n");
    }
    RBBIDebugPrintf("\n");
}
#endif


#ifdef RBBI_DEBUG
void  RBBIDataWrapper57::printData() {
    RBBIDebugPrintf("RBBI Data at %p\n", (void *)fHeader);
    RBBIDebugPrintf("   Version = {%d %d %d %d}\n", fHeader->fFormatVersion[0], fHeader->fFormatVersion[1],
                                                    fHeader->fFormatVersion[2], fHeader->fFormatVersion[3]);
    RBBIDebugPrintf("   total length of data  = %d\n", fHeader->fLength);
    RBBIDebugPrintf("   number of character categories = %d\n\n", fHeader->fCatCount);

    printTable("Forward State Transition Table", fForwardTable);
    printTable("Reverse State Transition Table", fReverseTable);
    printTable("Safe Forward State Transition Table", fSafeFwdTable);
    printTable("Safe Reverse State Transition Table", fSafeRevTable);

    RBBIDebugPrintf("\nOrignal Rules source:\n");
    for (int32_t c=0; fRuleSource[c] != 0; c++) {
        RBBIDebugPrintf("%c", fRuleSource[c]);
    }
    RBBIDebugPrintf("\n\n");
}
#endif


U_NAMESPACE_END
U_NAMESPACE_USE

//-----------------------------------------------------------------------------
//
//  ubrk_swap   - in standard rbbidata.cpp
//
//-----------------------------------------------------------------------------

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */
