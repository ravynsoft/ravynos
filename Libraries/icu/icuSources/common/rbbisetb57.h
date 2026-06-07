// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
/*
**********************************************************************
*   Copyright (c) 2001-2005, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*
**********************************************************************
*   Legacy version of RangeDescriptor and RBBISetBuilder from ICU 57,
*   only for use by Apple RuleBasedTokenizer
**********************************************************************
*/

#ifndef RBBISETB57_H
#define RBBISETB57_H

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "rbbirb57.h"
#include "rbbinode.h"
#include "uvector.h"

struct  UNewTrie;

U_NAMESPACE_BEGIN

class   RBBIRuleBuilder57;

//
//  RBBISetBuilder57   Derives the character categories used by the runtime RBBI engine
//                   from the Unicode Sets appearing in the source  RBBI rules, and
//                   creates the TRIE table used to map from Unicode to the
//                   character categories.
//


//
//  RangeDescriptor57
//
//     Each of the non-overlapping character ranges gets one of these descriptors.
//     All of them are strung together in a linked list, which is kept in order
//     (by character)
//
class RangeDescriptor57 : public UMemory {
public:
    UChar32            fStartChar;      // Start of range, unicode 32 bit value.
    UChar32            fEndChar;        // End of range, unicode 32 bit value.
    int32_t            fNum;            // runtime-mapped input value for this range.
    UVector           *fIncludesSets;   // vector of the the original
                                        //   Unicode sets that include this range.
                                        //    (Contains ptrs to uset nodes)
    RangeDescriptor57 *fNext;           // Next RangeDescriptor57 in the linked list.

    RangeDescriptor57(UErrorCode &status);
    RangeDescriptor57(const RangeDescriptor57 &other, UErrorCode &status);
    ~RangeDescriptor57();
    void split(UChar32 where, UErrorCode &status);   // Spit this range in two at "where", with
                                        //   where appearing in the second (higher) part.
    void setDictionaryFlag();           // Check whether this range appears as part of
                                        //   the Unicode set named "dictionary"

private:
    RangeDescriptor57(const RangeDescriptor57 &other); // forbid copying of this class
    RangeDescriptor57 &operator=(const RangeDescriptor57 &other); // forbid copying of this class
};


//
//  RBBISetBuilder57   Handles processing of Unicode Sets from RBBI rules.
//
//      Starting with the rules parse tree from the scanner,
//
//                   -  Enumerate the set of UnicodeSets that are referenced
//                      by the RBBI rules.
//                   -  compute a derived set of non-overlapping UnicodeSets
//                      that will correspond to columns in the state table for
//                      the RBBI execution engine.
//                   -  construct the trie table that maps input characters
//                      to set numbers in the non-overlapping set of sets.
//


class RBBISetBuilder57 : public UMemory {
public:
    RBBISetBuilder57(RBBIRuleBuilder57 *rb);
    ~RBBISetBuilder57();

    void     build();
    void     addValToSets(UVector *sets,      uint32_t val);
    void     addValToSet (RBBINode *usetNode, uint32_t val);
    int32_t  getNumCharCategories() const;   // CharCategories are the same as input symbol set to the
                                             //    runtime state machine, which are the same as
                                             //    columns in the DFA state table
    int32_t  getTrieSize() /*const*/;        // Size in bytes of the serialized Trie.
    void     serializeTrie(uint8_t *where);  // write out the serialized Trie.
    UChar32  getFirstChar(int32_t  val) const;
    UBool    sawBOF() const;                 // Indicate whether any references to the {bof} pseudo
                                             //   character were encountered.
#ifdef RBBI_DEBUG
    void     printSets();
    void     printRanges();
    void     printRangeGroups();
#else
    #define printSets()
    #define printRanges()
    #define printRangeGroups()
#endif

private:
    void           numberSets();

    RBBIRuleBuilder57     *fRB;             // The RBBI Rule Compiler that owns us.
    UErrorCode            *fStatus;

    RangeDescriptor57     *fRangeList;      // Head of the linked list of RangeDescriptors

    UNewTrie              *fTrie;           // The mapping TRIE that is the end result of processing
    uint32_t               fTrieSize;       //  the Unicode Sets.

    // Groups correspond to character categories -
    //       groups of ranges that are in the same original UnicodeSets.
    //       fGroupCount is the index of the last used group.
    //       fGroupCount+1 is also the number of columns in the RBBI state table being compiled.
    //       State table column 0 is not used.  Column 1 is for end-of-input.
    //       column 2 is for group 0.  Funny counting.
    int32_t               fGroupCount;

    UBool                 fSawBOF;

    RBBISetBuilder57(const RBBISetBuilder57 &other); // forbid copying of this class
    RBBISetBuilder57 &operator=(const RBBISetBuilder57 &other); // forbid copying of this class
};



U_NAMESPACE_END
#endif
