/*
	File:		DecompMakeData.c

	Contains:	Tool to generate tables for use by FixDecomps (CatalogCheck.c). It takes raw data on 				combining classes and decomposition changes, massages it into the trie form needed by
                the function, and emits it on stdout (which should be directed to a file DecompData.h).

	Copyright:	© 2002 by Apple Computer, Inc., all rights reserved.

	CVS change log:

		$Log: DecompMakeData.c,v $
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
		
		Revision 1.1  2002/10/16 06:33:26  pedberg
		Initial working version of function and related tools and tables
		
		
	Notes:
	
	1. To build:
	cc DecompMakeData.c -o DecompMakeData -g
	
	2. To use:
	./DecompMakeData > DecompData.h

*/

#include <stddef.h>
#include <stdio.h>

// Internal includes
#include "DecompDataEnums.h"	// enums for data tables

struct UniCharClassAndRepl {
	u_int16_t	uChar;
	u_int16_t	combClass;
	u_int16_t	action;
	u_int16_t	matchAndReplacement[3];
};
typedef struct UniCharClassAndRepl UniCharClassAndRepl;

// The following is the raw data on
// 1. Current combining classes, derived from the Unicode 3.2.0 data file
// 2. Changes in decomposition sequences, derived by comparing the canonical decompositions derived from
// the Unicode 2.1.2 data file with the decompositions derived from the Unicode 3.2.0 data file (in both
// cases excluding decompositions in the ranges 2000-2FFF, F900-FAFF, etc.).
// These are folded into a single table so we can do one lookup of the high-order 12 bits of the shifted
// UniChar to determine if there is anything of interest.
//
// Note that these ignore non-BMP characters; the new decompositions and combining classes for those are
// not really relevant for the purpose of fixing the HFS+ filenames.

static const UniCharClassAndRepl uCharClassAndRepl[] = {
//	cur char	comb	replacement							next chars that		replacement string
//	to match	class	action								must also match		for cur or all
//	--------	-----	----------------------------		---------------		---------------------
	{ 0x00A8,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x00A8, 0x0301			}	},
	{ 0x01F8,	  0,	kReplaceCurWithTwo,					{					0x004E, 0x0300			}	},
	{ 0x01F9,	  0,	kReplaceCurWithTwo,					{					0x006E, 0x0300			}	},
	{ 0x0218,	  0,	kReplaceCurWithTwo,					{					0x0053, 0x0326			}	},
	{ 0x0219,	  0,	kReplaceCurWithTwo,					{					0x0073, 0x0326			}	},
	{ 0x021A,	  0,	kReplaceCurWithTwo,					{					0x0054, 0x0326			}	},
	{ 0x021B,	  0,	kReplaceCurWithTwo,					{					0x0074, 0x0326			}	},
	{ 0x021E,	  0,	kReplaceCurWithTwo,					{					0x0048, 0x030C			}	},
	{ 0x021F,	  0,	kReplaceCurWithTwo,					{					0x0068, 0x030C			}	},
	{ 0x0226,	  0,	kReplaceCurWithTwo,					{					0x0041, 0x0307			}	},
	{ 0x0227,	  0,	kReplaceCurWithTwo,					{					0x0061, 0x0307			}	},
	{ 0x0228,	  0,	kReplaceCurWithTwo,					{					0x0045, 0x0327			}	},
	{ 0x0229,	  0,	kReplaceCurWithTwo,					{					0x0065, 0x0327			}	},
	{ 0x022A,	  0,	kReplaceCurWithThree,				{					0x004F, 0x0308, 0x0304	}	},
	{ 0x022B,	  0,	kReplaceCurWithThree,				{					0x006F, 0x0308, 0x0304	}	},
	{ 0x022C,	  0,	kReplaceCurWithThree,				{					0x004F, 0x0303, 0x0304	}	},
	{ 0x022D,	  0,	kReplaceCurWithThree,				{					0x006F, 0x0303, 0x0304	}	},
	{ 0x022E,	  0,	kReplaceCurWithTwo,					{					0x004F, 0x0307			}	},
	{ 0x022F,	  0,	kReplaceCurWithTwo,					{					0x006F, 0x0307			}	},
	{ 0x0230,	  0,	kReplaceCurWithThree,				{					0x004F, 0x0307, 0x0304	}	},
	{ 0x0231,	  0,	kReplaceCurWithThree,				{					0x006F, 0x0307, 0x0304	}	},
	{ 0x0232,	  0,	kReplaceCurWithTwo,					{					0x0059, 0x0304			}	},
	{ 0x0233,	  0,	kReplaceCurWithTwo,					{					0x0079, 0x0304			}	},
	{ 0x0300,	230,	0,	{ 0	}	},
	{ 0x0301,	230,	0,	{ 0	}	},
	{ 0x0302,	230,	0,	{ 0	}	},
	{ 0x0303,	230,	0,	{ 0	}	},
	{ 0x0304,	230,	0,	{ 0	}	},
	{ 0x0305,	230,	0,	{ 0	}	},
	{ 0x0306,	230,	kIfNextOneMatchesReplaceAllWithOne,	{ 0x0307,			0x0310					}	},
	{ 0x0307,	230,	0,	{ 0	}	},
	{ 0x0308,	230,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x0308, 0x0301			}	},
	{ 0x0309,	230,	0,	{ 0	}	},
	{ 0x030A,	230,	0,	{ 0	}	},
	{ 0x030B,	230,	0,	{ 0	}	},
	{ 0x030C,	230,	0,	{ 0	}	},
	{ 0x030D,	230,	0,	{ 0	}	},
	{ 0x030E,	230,	0,	{ 0	}	},
	{ 0x030F,	230,	0,	{ 0	}	},
	{ 0x0310,	230,	0,	{ 0	}	},
	{ 0x0311,	230,	0,	{ 0	}	},
	{ 0x0312,	230,	0,	{ 0	}	},
	{ 0x0313,	230,	0,	{ 0	}	},
	{ 0x0314,	230,	0,	{ 0	}	},
	{ 0x0315,	232,	0,	{ 0	}	},
	{ 0x0316,	220,	0,	{ 0	}	},
	{ 0x0317,	220,	0,	{ 0	}	},
	{ 0x0318,	220,	0,	{ 0	}	},
	{ 0x0319,	220,	0,	{ 0	}	},
	{ 0x031A,	232,	0,	{ 0	}	},
	{ 0x031B,	216,	0,	{ 0	}	},
	{ 0x031C,	220,	0,	{ 0	}	},
	{ 0x031D,	220,	0,	{ 0	}	},
	{ 0x031E,	220,	0,	{ 0	}	},
	{ 0x031F,	220,	0,	{ 0	}	},
	{ 0x0320,	220,	0,	{ 0	}	},
	{ 0x0321,	202,	0,	{ 0	}	},
	{ 0x0322,	202,	0,	{ 0	}	},
	{ 0x0323,	220,	0,	{ 0	}	},
	{ 0x0324,	220,	0,	{ 0	}	},
	{ 0x0325,	220,	0,	{ 0	}	},
	{ 0x0326,	220,	0,	{ 0	}	},
	{ 0x0327,	202,	0,	{ 0	}	},
	{ 0x0328,	202,	0,	{ 0	}	},
	{ 0x0329,	220,	0,	{ 0	}	},
	{ 0x032A,	220,	0,	{ 0	}	},
	{ 0x032B,	220,	0,	{ 0	}	},
	{ 0x032C,	220,	0,	{ 0	}	},
	{ 0x032D,	220,	0,	{ 0	}	},
	{ 0x032E,	220,	0,	{ 0	}	},
	{ 0x032F,	220,	0,	{ 0	}	},
	{ 0x0330,	220,	0,	{ 0	}	},
	{ 0x0331,	220,	0,	{ 0	}	},
	{ 0x0332,	220,	0,	{ 0	}	},
	{ 0x0333,	220,	0,	{ 0	}	},
	{ 0x0334,	  1,	0,	{ 0	}	},
	{ 0x0335,	  1,	0,	{ 0	}	},
	{ 0x0336,	  1,	0,	{ 0	}	},
	{ 0x0337,	  1,	0,	{ 0	}	},
	{ 0x0338,	  1,	0,	{ 0	}	},
	{ 0x0339,	220,	0,	{ 0	}	},
	{ 0x033A,	220,	0,	{ 0	}	},
	{ 0x033B,	220,	0,	{ 0	}	},
	{ 0x033C,	220,	0,	{ 0	}	},
	{ 0x033D,	230,	0,	{ 0	}	},
	{ 0x033E,	230,	0,	{ 0	}	},
	{ 0x033F,	230,	0,	{ 0	}	},
	{ 0x0340,	230,	0,	{ 0	}	},
	{ 0x0341,	230,	0,	{ 0	}	},
	{ 0x0342,	230,	0,	{ 0	}	},
	{ 0x0343,	230,	0,	{ 0	}	},
	{ 0x0344,	230,	0,	{ 0	}	},
	{ 0x0345,	240,	0,	{ 0	}	},
	{ 0x0346,	230,	0,	{ 0	}	},
	{ 0x0347,	220,	0,	{ 0	}	},
	{ 0x0348,	220,	0,	{ 0	}	},
	{ 0x0349,	220,	0,	{ 0	}	},
	{ 0x034A,	230,	0,	{ 0	}	},
	{ 0x034B,	230,	0,	{ 0	}	},
	{ 0x034C,	230,	0,	{ 0	}	},
	{ 0x034D,	220,	0,	{ 0	}	},
	{ 0x034E,	220,	0,	{ 0	}	},
	{ 0x0360,	234,	0,	{ 0	}	},
	{ 0x0361,	234,	0,	{ 0	}	},
	{ 0x0362,	233,	0,	{ 0	}	},
	{ 0x0363,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0364,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0365,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0366,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0367,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0368,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0369,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x036A,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x036B,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x036C,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x036D,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x036E,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x036F,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x0391,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x0391, 0x0301			}	},
	{ 0x0395,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x0395, 0x0301			}	},
	{ 0x0397,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x0397, 0x0301			}	},
	{ 0x0399,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x0399, 0x0301			}	},
	{ 0x039F,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x039F, 0x0301			}	},
	{ 0x03A5,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03A5, 0x0301			}	},
	{ 0x03A9,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03A9, 0x0301			}	},
	{ 0x03B1,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03B1, 0x0301			}	},
	{ 0x03B5,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03B5, 0x0301			}	},
	{ 0x03B7,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03B7, 0x0301			}	},
	{ 0x03B9,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03B9, 0x0301			}	},
	{ 0x03BF,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03BF, 0x0301			}	},
	{ 0x03C5,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03C5, 0x0301			}	},
	{ 0x03C9,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03C9, 0x0301			}	},
	{ 0x03D2,	  0,	kIfNextOneMatchesReplaceAllWithTwo,	{ 0x030D,			0x03D2, 0x0301			}	},
	{ 0x0400,	  0,	kReplaceCurWithTwo,					{					0x0415, 0x0300			}	},
	{ 0x040D,	  0,	kReplaceCurWithTwo,					{					0x0418, 0x0300			}	},
	{ 0x0450,	  0,	kReplaceCurWithTwo,					{					0x0435, 0x0300			}	},
	{ 0x045D,	  0,	kReplaceCurWithTwo,					{					0x0438, 0x0300			}	},
	{ 0x0483,	230,	0,	{ 0	}	},
	{ 0x0484,	230,	0,	{ 0	}	},
	{ 0x0485,	230,	0,	{ 0	}	},
	{ 0x0486,	230,	0,	{ 0	}	},
	{ 0x04EC,	  0,	kReplaceCurWithTwo,					{					0x042D, 0x0308			}	},
	{ 0x04ED,	  0,	kReplaceCurWithTwo,					{					0x044D, 0x0308			}	},
	{ 0x0591,	220,	0,	{ 0	}	},
	{ 0x0592,	230,	0,	{ 0	}	},
	{ 0x0593,	230,	0,	{ 0	}	},
	{ 0x0594,	230,	0,	{ 0	}	},
	{ 0x0595,	230,	0,	{ 0	}	},
	{ 0x0596,	220,	0,	{ 0	}	},
	{ 0x0597,	230,	0,	{ 0	}	},
	{ 0x0598,	230,	0,	{ 0	}	},
	{ 0x0599,	230,	0,	{ 0	}	},
	{ 0x059A,	222,	0,	{ 0	}	},
	{ 0x059B,	220,	0,	{ 0	}	},
	{ 0x059C,	230,	0,	{ 0	}	},
	{ 0x059D,	230,	0,	{ 0	}	},
	{ 0x059E,	230,	0,	{ 0	}	},
	{ 0x059F,	230,	0,	{ 0	}	},
	{ 0x05A0,	230,	0,	{ 0	}	},
	{ 0x05A1,	230,	0,	{ 0	}	},
	{ 0x05A3,	220,	0,	{ 0	}	},
	{ 0x05A4,	220,	0,	{ 0	}	},
	{ 0x05A5,	220,	0,	{ 0	}	},
	{ 0x05A6,	220,	0,	{ 0	}	},
	{ 0x05A7,	220,	0,	{ 0	}	},
	{ 0x05A8,	230,	0,	{ 0	}	},
	{ 0x05A9,	230,	0,	{ 0	}	},
	{ 0x05AA,	220,	0,	{ 0	}	},
	{ 0x05AB,	230,	0,	{ 0	}	},
	{ 0x05AC,	230,	0,	{ 0	}	},
	{ 0x05AD,	222,	0,	{ 0	}	},
	{ 0x05AE,	228,	0,	{ 0	}	},
	{ 0x05AF,	230,	0,	{ 0	}	},
	{ 0x05B0,	 10,	0,	{ 0	}	},
	{ 0x05B1,	 11,	0,	{ 0	}	},
	{ 0x05B2,	 12,	0,	{ 0	}	},
	{ 0x05B3,	 13,	0,	{ 0	}	},
	{ 0x05B4,	 14,	0,	{ 0	}	},
	{ 0x05B5,	 15,	0,	{ 0	}	},
	{ 0x05B6,	 16,	0,	{ 0	}	},
	{ 0x05B7,	 17,	0,	{ 0	}	},
	{ 0x05B8,	 18,	0,	{ 0	}	},
	{ 0x05B9,	 19,	0,	{ 0	}	},
	{ 0x05BB,	 20,	0,	{ 0	}	},
	{ 0x05BC,	 21,	0,	{ 0	}	},
	{ 0x05BD,	 22,	0,	{ 0	}	},
	{ 0x05BF,	 23,	0,	{ 0	}	},
	{ 0x05C1,	 24,	0,	{ 0	}	},
	{ 0x05C2,	 25,	0,	{ 0	}	},
	{ 0x05C4,	230,	0,	{ 0	}	},
	{ 0x0622,	  0,	kReplaceCurWithTwo,					{					0x0627, 0x0653			}	},
	{ 0x0623,	  0,	kReplaceCurWithTwo,					{					0x0627, 0x0654			}	},
	{ 0x0624,	  0,	kReplaceCurWithTwo,					{					0x0648, 0x0654			}	},
	{ 0x0625,	  0,	kReplaceCurWithTwo,					{					0x0627, 0x0655			}	},
	{ 0x0626,	  0,	kReplaceCurWithTwo,					{					0x064A, 0x0654			}	},
	{ 0x064B,	 27,	0,	{ 0	}	},
	{ 0x064C,	 28,	0,	{ 0	}	},
	{ 0x064D,	 29,	0,	{ 0	}	},
	{ 0x064E,	 30,	0,	{ 0	}	},
	{ 0x064F,	 31,	0,	{ 0	}	},
	{ 0x0650,	 32,	0,	{ 0	}	},
	{ 0x0651,	 33,	0,	{ 0	}	},
	{ 0x0652,	 34,	0,	{ 0	}	},
	{ 0x0653,	230,	0,	{ 0	}	},
	{ 0x0654,	230,	0,	{ 0	}	},
	{ 0x0655,	220,	0,	{ 0	}	},
	{ 0x0670,	 35,	0,	{ 0	}	},
	{ 0x06C0,	  0,	kReplaceCurWithTwo,					{					0x06D5, 0x0654			}	},
	{ 0x06C2,	  0,	kReplaceCurWithTwo,					{					0x06C1, 0x0654			}	},
	{ 0x06D3,	  0,	kReplaceCurWithTwo,					{					0x06D2, 0x0654			}	},
	{ 0x06D6,	230,	0,	{ 0	}	},
	{ 0x06D7,	230,	0,	{ 0	}	},
	{ 0x06D8,	230,	0,	{ 0	}	},
	{ 0x06D9,	230,	0,	{ 0	}	},
	{ 0x06DA,	230,	0,	{ 0	}	},
	{ 0x06DB,	230,	0,	{ 0	}	},
	{ 0x06DC,	230,	0,	{ 0	}	},
	{ 0x06DF,	230,	0,	{ 0	}	},
	{ 0x06E0,	230,	0,	{ 0	}	},
	{ 0x06E1,	230,	0,	{ 0	}	},
	{ 0x06E2,	230,	0,	{ 0	}	},
	{ 0x06E3,	220,	0,	{ 0	}	},
	{ 0x06E4,	230,	0,	{ 0	}	},
	{ 0x06E7,	230,	0,	{ 0	}	},
	{ 0x06E8,	230,	0,	{ 0	}	},
	{ 0x06EA,	220,	0,	{ 0	}	},
	{ 0x06EB,	230,	0,	{ 0	}	},
	{ 0x06EC,	230,	0,	{ 0	}	},
	{ 0x06ED,	220,	0,	{ 0	}	},
	{ 0x0711,	 36,	0,	{ 0	}	},
	{ 0x0730,	230,	0,	{ 0	}	},
	{ 0x0731,	220,	0,	{ 0	}	},
	{ 0x0732,	230,	0,	{ 0	}	},
	{ 0x0733,	230,	0,	{ 0	}	},
	{ 0x0734,	220,	0,	{ 0	}	},
	{ 0x0735,	230,	0,	{ 0	}	},
	{ 0x0736,	230,	0,	{ 0	}	},
	{ 0x0737,	220,	0,	{ 0	}	},
	{ 0x0738,	220,	0,	{ 0	}	},
	{ 0x0739,	220,	0,	{ 0	}	},
	{ 0x073A,	230,	0,	{ 0	}	},
	{ 0x073B,	220,	0,	{ 0	}	},
	{ 0x073C,	220,	0,	{ 0	}	},
	{ 0x073D,	230,	0,	{ 0	}	},
	{ 0x073E,	220,	0,	{ 0	}	},
	{ 0x073F,	230,	0,	{ 0	}	},
	{ 0x0740,	230,	0,	{ 0	}	},
	{ 0x0741,	230,	0,	{ 0	}	},
	{ 0x0742,	220,	0,	{ 0	}	},
	{ 0x0743,	230,	0,	{ 0	}	},
	{ 0x0744,	220,	0,	{ 0	}	},
	{ 0x0745,	230,	0,	{ 0	}	},
	{ 0x0746,	220,	0,	{ 0	}	},
	{ 0x0747,	230,	0,	{ 0	}	},
	{ 0x0748,	220,	0,	{ 0	}	},
	{ 0x0749,	230,	0,	{ 0	}	},
	{ 0x074A,	230,	0,	{ 0	}	},
	{ 0x093C,	  7,	0,	{ 0	}	},
	{ 0x094D,	  9,	0,	{ 0	}	},
	{ 0x0951,	230,	0,	{ 0	}	},
	{ 0x0952,	220,	0,	{ 0	}	},
	{ 0x0953,	230,	0,	{ 0	}	},
	{ 0x0954,	230,	0,	{ 0	}	},
	{ 0x09AC,	  0,	kIfNextOneMatchesReplaceAllWithOne,	{ 0x09BC,			0x09B0					}	},
	{ 0x09BC,	  7,	0,	{ 0	}	},
	{ 0x09CD,	  9,	0,	{ 0	}	},
	{ 0x0A21,	  0,	kIfNextOneMatchesReplaceAllWithOne,	{ 0x0A3C,			0x0A5C					}	},
	{ 0x0A33,	  0,	kReplaceCurWithTwo,					{					0x0A32, 0x0A3C			}	},
	{ 0x0A36,	  0,	kReplaceCurWithTwo,					{					0x0A38, 0x0A3C			}	},
	{ 0x0A3C,	  7,	0,	{ 0	}	},
	{ 0x0A4D,	  9,	0,	{ 0	}	},
	{ 0x0ABC,	  7,	0,	{ 0	}	},
	{ 0x0ACD,	  9,	0,	{ 0	}	},
	{ 0x0B2F,	  0,	kIfNextOneMatchesReplaceAllWithOne,	{ 0x0B3C,			0x0B5F					}	},
	{ 0x0B3C,	  7,	0,	{ 0	}	},
	{ 0x0B4D,	  9,	0,	{ 0	}	},
	{ 0x0BCD,	  9,	0,	{ 0	}	},
	{ 0x0C4D,	  9,	0,	{ 0	}	},
	{ 0x0C55,	 84,	0,	{ 0	}	},
	{ 0x0C56,	 91,	0,	{ 0	}	},
	{ 0x0CCD,	  9,	0,	{ 0	}	},
	{ 0x0D4D,	  9,	0,	{ 0	}	},
	{ 0x0DCA,	  9,	0,	{ 0	}	},
	{ 0x0DDA,	  0,	kReplaceCurWithTwo,					{					0x0DD9, 0x0DCA			}	},
	{ 0x0DDC,	  0,	kReplaceCurWithTwo,					{					0x0DD9, 0x0DCF			}	},
	{ 0x0DDD,	  0,	kReplaceCurWithThree,				{					0x0DD9, 0x0DCF, 0x0DCA	}	},
	{ 0x0DDE,	  0,	kReplaceCurWithTwo,					{					0x0DD9, 0x0DDF			}	},
	{ 0x0E38,	103,	0,	{ 0	}	},
	{ 0x0E39,	103,	0,	{ 0	}	},
	{ 0x0E3A,	  9,	0,	{ 0	}	},
	{ 0x0E48,	107,	0,	{ 0	}	},
	{ 0x0E49,	107,	0,	{ 0	}	},
	{ 0x0E4A,	107,	0,	{ 0	}	},
	{ 0x0E4B,	107,	0,	{ 0	}	},
	{ 0x0E4D,	  0,	kIfNextOneMatchesReplaceAllWithOne,	{ 0x0E32,			0x0E33					}	},
	{ 0x0EB8,	118,	0,	{ 0	}	},
	{ 0x0EB9,	118,	0,	{ 0	}	},
	{ 0x0EC8,	122,	0,	{ 0	}	},
	{ 0x0EC9,	122,	0,	{ 0	}	},
	{ 0x0ECA,	122,	0,	{ 0	}	},
	{ 0x0ECB,	122,	0,	{ 0	}	},
	{ 0x0ECD,	  0,	kIfNextOneMatchesReplaceAllWithOne,	{ 0x0EB2,			0x0EB3					}	},
	{ 0x0F18,	220,	0,	{ 0	}	},
	{ 0x0F19,	220,	0,	{ 0	}	},
	{ 0x0F35,	220,	0,	{ 0	}	},
	{ 0x0F37,	220,	0,	{ 0	}	},
	{ 0x0F39,	216,	0,	{ 0	}	},
	{ 0x0F71,	129,	0,	{ 0	}	},
	{ 0x0F72,	130,	0,	{ 0	}	},
	{ 0x0F74,	132,	0,	{ 0	}	},
	{ 0x0F7A,	130,	0,	{ 0	}	},
	{ 0x0F7B,	130,	0,	{ 0	}	},
	{ 0x0F7C,	130,	0,	{ 0	}	},
	{ 0x0F7D,	130,	0,	{ 0	}	},
	{ 0x0F80,	130,	0,	{ 0	}	},
	{ 0x0F82,	230,	0,	{ 0	}	},
	{ 0x0F83,	230,	0,	{ 0	}	},
	{ 0x0F84,	  9,	0,	{ 0	}	},
	{ 0x0F86,	230,	0,	{ 0	}	},
	{ 0x0F87,	230,	0,	{ 0	}	},
	{ 0x0FB2,	  0,	kIfNextTwoMatchReplaceAllWithOne,	{ 0x0F80, 0x0F71,	0x0F77					}	},
	{ 0x0FB3,	  0,	kIfNextTwoMatchReplaceAllWithOne,	{ 0x0F80, 0x0F71,	0x0F79					}	},
	{ 0x0FC6,	220,	0,	{ 0	}	},
	{ 0x1026,	  0,	kReplaceCurWithTwo,					{					0x1025, 0x102E			}	},
	{ 0x1037,	  7,	0,	{ 0	}	},
	{ 0x1039,	  9,	0,	{ 0	}	},
	{ 0x1714,	  9,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x1734,	  9,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x17D2,	  9,	0,	{ 0	}	},
	{ 0x18A9,	228,	0,	{ 0	}	},
	{ 0x20D0,	230,	0,	{ 0	}	},
	{ 0x20D1,	230,	0,	{ 0	}	},
	{ 0x20D2,	  1,	0,	{ 0	}	},
	{ 0x20D3,	  1,	0,	{ 0	}	},
	{ 0x20D4,	230,	0,	{ 0	}	},
	{ 0x20D5,	230,	0,	{ 0	}	},
	{ 0x20D6,	230,	0,	{ 0	}	},
	{ 0x20D7,	230,	0,	{ 0	}	},
	{ 0x20D8,	  1,	0,	{ 0	}	},
	{ 0x20D9,	  1,	0,	{ 0	}	},
	{ 0x20DA,	  1,	0,	{ 0	}	},
	{ 0x20DB,	230,	0,	{ 0	}	},
	{ 0x20DC,	230,	0,	{ 0	}	},
	{ 0x20E1,	230,	0,	{ 0	}	},
	{ 0x20E5,	  1,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x20E6,	  1,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x20E7,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x20E8,	220,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x20E9,	230,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x20EA,	  1,	0,	{ 0	}	},	// new char in Uncode 3.2
	{ 0x302A,	218,	0,	{ 0	}	},
	{ 0x302B,	228,	0,	{ 0	}	},
	{ 0x302C,	232,	0,	{ 0	}	},
	{ 0x302D,	222,	0,	{ 0	}	},
	{ 0x302E,	224,	0,	{ 0	}	},
	{ 0x302F,	224,	0,	{ 0	}	},
	{ 0x3099,	  8,	0,	{ 0	}	},
	{ 0x309A,	  8,	0,	{ 0	}	},
	{ 0xFB1D,	  0,	kReplaceCurWithTwo,					{					0x05D9, 0x05B4			}	},
	{ 0xFB1E,	 26,	0,	{ 0	}	},
	{ 0xFE20,	230,	0,	{ 0	}	},
	{ 0xFE21,	230,	0,	{ 0	}	},
	{ 0xFE22,	230,	0,	{ 0	}	},
	{ 0xFE23,	230,	0,	{ 0	}	},
	{ 0,		  0,	0,	{ 0	}	}
};

enum {
	kMaxRangeCount			= 108,
	kMaxReplaceDataCount	= 256,
	kIndexValuesPerLine		= 16,
	kReplDataValuesPerLine	= 8
};

static int8_t	rangesIndex[kHiFieldEntryCount];					// if >= 0, then index into xxxRanges[]
static u_int8_t	classRanges[kMaxRangeCount][kLoFieldEntryCount];
static u_int8_t	replRanges[kMaxRangeCount][kLoFieldEntryCount];
static u_int16_t rangesKey[kMaxRangeCount];							// remembers starting Unicode for range
static u_int16_t replacementData[kMaxReplaceDataCount];

int main(int argc, char *argv[]) {
	u_int32_t						entryIndex, rangeIndex;
	const UniCharClassAndRepl *		classAndReplPtr;
	int32_t							rangeCount;
	u_int32_t						replDataCount;

	// print header stuff
	plog("/*\n");
	plog("\tFile:\t\tDecompData.h\n");
	plog("\tContains:\tData tables for use in FixDecomps (CatalogCheck.c)\n");
	plog("\tNote:\t\tThis file is generated automatically by running DecompMakeData\n");
	plog("*/\n");
	plog("#include \"DecompDataEnums.h\"\n\n");

	// initialize arrays
	for (entryIndex = 0; entryIndex < kHiFieldEntryCount; entryIndex++) {
		rangesIndex[entryIndex] = -1;
	}
	for (rangeIndex = 0; rangeIndex < kMaxRangeCount; rangeIndex++) {
		for (entryIndex = 0; entryIndex < kLoFieldEntryCount; entryIndex++) {
			classRanges[rangeIndex][entryIndex] = 0;
			replRanges[rangeIndex][entryIndex] = 0;
		}
	}
	rangeCount = 0;
	replDataCount = 0;
	replacementData[replDataCount++] = 0;	// need to start real data at index 1

	// process data
	for (classAndReplPtr = uCharClassAndRepl; classAndReplPtr->uChar != 0; classAndReplPtr++) {
		u_int32_t	matchAndReplacementCount, matchAndReplacementIndex;
		u_int16_t	shiftUChar = classAndReplPtr->uChar + kShiftUniCharOffset;
		if (shiftUChar >= kShiftUniCharLimit) {
			plog("Exceeded uChar range for 0x%04X\n", classAndReplPtr->uChar);
			return 1;
		}
		entryIndex = shiftUChar >> kLoFieldBitSize;
		if (rangesIndex[entryIndex] == -1) {
			if (rangeCount >= kMaxRangeCount) {
				plog("Exceeded max range count with 0x%04X\n", classAndReplPtr->uChar);
				return 1;
			}
			rangesKey[rangeCount] = classAndReplPtr->uChar & ~kLoFieldMask;
			rangesIndex[entryIndex] = rangeCount++;
		}
		entryIndex = shiftUChar & kLoFieldMask;

		if (classAndReplPtr->combClass != 0)
			classRanges[rangeCount - 1][entryIndex] = classAndReplPtr->combClass;
		
		if (classAndReplPtr->action != 0) {
			switch (classAndReplPtr->action) {
				case kReplaceCurWithTwo:
				case kIfNextOneMatchesReplaceAllWithOne:
					matchAndReplacementCount = 2;
					break;
				case kReplaceCurWithThree:
				case kIfNextOneMatchesReplaceAllWithTwo:
				case kIfNextTwoMatchReplaceAllWithOne:
					matchAndReplacementCount = 3;
					break;
				default:
					matchAndReplacementCount = 0;
					break;
			}
			if (replDataCount + matchAndReplacementCount >= kMaxReplaceDataCount) {
				plog("Exceeded max replacement data count with 0x%04X\n", classAndReplPtr->uChar);
				return 1;
			}
			replRanges[rangeCount - 1][entryIndex] = replDataCount;
			replacementData[replDataCount++] = classAndReplPtr->action;
			for (matchAndReplacementIndex = 0; matchAndReplacementIndex < matchAndReplacementCount; matchAndReplacementIndex++) {
				replacementData[replDataCount++] = classAndReplPtr->matchAndReplacement[matchAndReplacementIndex];
			}
		}
	}
	
	// print filled-in index
	plog("static const int8_t classAndReplIndex[kHiFieldEntryCount] = {\n");
	for (entryIndex = 0; entryIndex < kHiFieldEntryCount; entryIndex++) {
		char *	formatPtr = (entryIndex + 1 < kHiFieldEntryCount)? "%2d,\t": "%2d\t";
		if (entryIndex % kIndexValuesPerLine == 0)			// beginning of line,
			plog("\t");								//  print tab
		plog(formatPtr, rangesIndex[entryIndex]);		// print values
		if ((entryIndex + 1) % kIndexValuesPerLine == 0)		// end of line, print starting UniChar value
			plog("// uChar 0x%04X-\n", (u_int16_t)(((entryIndex + 1 - kIndexValuesPerLine) << kLoFieldBitSize) - kShiftUniCharOffset) );
	}
	plog("};\n\n");
	
	// print filled in class ranges
	plog("static const u_int8_t combClassRanges[][kLoFieldEntryCount] = {\n", kLoFieldEntryCount);
	for (rangeIndex = 0; rangeIndex < rangeCount; rangeIndex++) {
		plog("\t{\t");
		for (entryIndex = 0; entryIndex < kLoFieldEntryCount; entryIndex++) {
			char *	formatPtr = (entryIndex + 1 < kLoFieldEntryCount)? "%3d,": "%3d";
			plog(formatPtr, classRanges[rangeIndex][entryIndex]);	// print values
		}
		plog("\t},\t// uChar 0x%04X-\n", rangesKey[rangeIndex]);
	}
	plog("};\n\n");

	// print filled in repl ranges
	plog("static const u_int8_t replaceRanges[][kLoFieldEntryCount] = {\n", kLoFieldEntryCount);
	for (rangeIndex = 0; rangeIndex < rangeCount; rangeIndex++) {
		plog("\t{\t");
		for (entryIndex = 0; entryIndex < kLoFieldEntryCount; entryIndex++) {
			char *	formatPtr = (entryIndex + 1 < kLoFieldEntryCount)? "%3d,": "%3d";
			plog(formatPtr, replRanges[rangeIndex][entryIndex]);	// print values
		}
		plog("\t},\t// uChar 0x%04X-\n", rangesKey[rangeIndex]);
	}
	plog("};\n\n");
	
	// print filled in replacement data
	plog("static const u_int16_t replaceData[] = {\n");
	for (entryIndex = 0; entryIndex < replDataCount; entryIndex++) {
		char *	formatPtr = (entryIndex + 1 < replDataCount)? "0x%04X,\t": "0x%04X\t";
		if (entryIndex % kReplDataValuesPerLine == 0)			// beginning of line,
			plog("\t");										//  print tab
		plog(formatPtr, replacementData[entryIndex]);			// print values
		if ((entryIndex + 1) % kReplDataValuesPerLine == 0 || entryIndex + 1 == replDataCount)	// end of line,
			plog("// index %d-\n", entryIndex & ~(kReplDataValuesPerLine-1) );	// print starting index value
	}
	plog("};\n\n");

	// print summary info
	plog("// combClassData:\n");
	plog("// trimmed index: kHiFieldEntryCount(= %d) bytes\n", kHiFieldEntryCount);
	plog("// ranges: 2 * %d ranges * kLoFieldEntryCount(= %d) bytes = %d\n", rangeCount, kLoFieldEntryCount, 2*rangeCount*kLoFieldEntryCount);
	plog("// replData: %d entries * 2 = %d\n", replDataCount, 2*replDataCount);
	plog("// total: %d\n\n", kHiFieldEntryCount + 2*rangeCount*kLoFieldEntryCount + 2*replDataCount);
	
	return 0;
}
