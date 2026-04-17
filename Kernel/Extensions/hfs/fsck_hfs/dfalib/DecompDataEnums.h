/*
	File:		DecompDataEnums.h

	Contains:	Constants for data tables used with FixDecomps (CatalogCheck.c)

	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.

	CVS change log:

		$Log: DecompDataEnums.h,v $
		Revision 1.2  2002/12/20 01:20:36  lindak
		Merged PR-2937515-2 into ZZ100
		Old HFS+ decompositions need to be repaired
		
		Revision 1.1.4.1  2002/12/16 18:55:22  jcotting
		integrated code from text group (Peter Edberg) that will correct some
		illegal names created with obsolete Unicode 2.1.2 decomposition rules
		Bug #: 2937515
		Submitted by: jerry cottingham
		Reviewed by: don brady
		
		Revision 1.1.2.1  2002/10/25 17:15:22  jcotting
		added code from Peter Edberg that will detect and offer replacement
		names for file system object names with pre-Jaguar decomp errors
		Bug #: 2937515
		Submitted by: jerry cottingham
		Reviewed by: don brady
		
		Revision 1.1  2002/10/16 06:33:25  pedberg
		Initial working version of function and related tools and tables
		
		
*/

#ifndef __DECOMPDATAENUMS__
#define __DECOMPDATAENUMS__

// Basic table parameters for 2-stage trie:
// The high 12 bits of a UniChar provide an index into a first-level table;
// if the entry there is >= 0, it is an index into a table of 16-element
// ranges indexed by the low 4 bits of the UniChar. Since the UniChars of interest
// for combining classes and sequence updates are either in the range 0000-30FF
// or in the range FB00-FFFF, we eliminate the large middle section of the first-
// level table by first adding 0500 to the UniChar to wrap the UniChars of interest
// into the range 0000-35FF.
enum {
	kLoFieldBitSize		= 4,
	kShiftUniCharOffset	= 0x0500,	// add to UniChar so FB00 & up wraps to 0000
	kShiftUniCharLimit	= 0x3600	// if UniChar + offset >= limit, no need to check
};

// The following are all derived from kLoFieldBitSize
enum {
	kLoFieldEntryCount	= 1 << kLoFieldBitSize,
	kHiFieldEntryCount	= kShiftUniCharLimit >> kLoFieldBitSize,
	kLoFieldMask		= (1 << kLoFieldBitSize) - 1
};

// Action codes for sequence replacement/updating
enum {											// next + repl = total chars
	// a value of 0 means no action
	kReplaceCurWithTwo					= 0x02,	//    0 + 2 = 2
	kReplaceCurWithThree				= 0x03,	//    0 + 3 = 3
	kIfNextOneMatchesReplaceAllWithOne	= 0x12,	//    1 + 1 = 2
	kIfNextOneMatchesReplaceAllWithTwo	= 0x13,	//    1 + 2 = 3
	kIfNextTwoMatchReplaceAllWithOne	= 0x23	//    2 + 1 = 3
};

#endif // __FSCKFIXDECOMPS__


