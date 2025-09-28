/* defines types for tags */
#ifndef _TAGTYPES_H
#define _TAGTYPES_H

#define TT_APTR    1
#define TT_WORD    2
#define TT_UWORD   3
#define TT_LONG    4
#define TT_ULONG   5
#define TT_STRPTR  6
#define TT_UBYTE   7

typedef union TagReturn
{
    WORD    tr_word;
    UWORD  tr_uword;
    LONG    tr_long;
    ULONG   tr_ulong;
    STRPTR  tr_strptr;
    APTR     tr_aptr;
    UBYTE * tr_ubyte;
}
TAGRET;

#endif
