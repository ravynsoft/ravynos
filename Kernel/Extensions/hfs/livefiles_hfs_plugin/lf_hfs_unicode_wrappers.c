//
//  lf_hfs_unicode_wrappers.c
//  livefiles_hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#include "lf_hfs_unicode_wrappers.h"
#include "lf_hfs_ucs_string_cmp_data.h"
#include "lf_hfs_sbunicode.h"



enum {
    kMinFileExtensionChars = 1,    /* does not include dot */
    kMaxFileExtensionChars = 5    /* does not include dot */
};


#define EXTENSIONCHAR(c)    (((c) >= 0x61 && (c) <= 0x7A) || \
                             ((c) >= 0x41 && (c) <= 0x5A) || \
                             ((c) >= 0x30 && (c) <= 0x39))


#define IsHexDigit(c)        (((c) >= (u_int8_t) '0' && (c) <= (u_int8_t) '9') || \
                              ((c) >= (u_int8_t) 'A' && (c) <= (u_int8_t) 'F'))


static void
GetFilenameExtension( ItemCount length, ConstUniCharArrayPtr unicodeStr, char* extStr );


static u_int32_t
HexStringToInteger( u_int32_t length, const u_int8_t *hexStr );


/*
 * Get filename extension (if any) as a C string
 */
static void
GetFilenameExtension(ItemCount length, ConstUniCharArrayPtr unicodeStr, char * extStr)
{
    u_int64_t    i;
    UniChar    c;
    u_int16_t    extChars;    /* number of extension chars (excluding dot) */
    u_int16_t    maxExtChars;
    Boolean    foundExtension;
    
    extStr[0] = '\0';    /* assume there's no extension */
    
    if ( length < 3 )
        return;        /* "x.y" is smallest possible extension */
    
    if ( length < (kMaxFileExtensionChars + 2) )
        maxExtChars = length - 2;    /* save room for prefix + dot */
    else
        maxExtChars = kMaxFileExtensionChars;
    
    i = length;
    extChars = 0;
    foundExtension = false;
    
    while ( extChars <= maxExtChars ) {
        c = unicodeStr[--i];
        
        /* look for leading dot */
        if ( c == (UniChar) '.' ) {
            if ( extChars > 0 )    /* cannot end with a dot */
                foundExtension = true;
            break;
        }
        
        if ( EXTENSIONCHAR(c) )
            ++extChars;
        else
            break;
    }
    
    /* if we found one then copy it */
    if ( foundExtension ) {
        u_int8_t *extStrPtr = (u_int8_t *)extStr;
        const UniChar *unicodeStrPtr = &unicodeStr[i];
        
        for ( i = 0; i <= extChars; ++i )
            *(extStrPtr++) = (u_int8_t) *(unicodeStrPtr++);
        extStr[extChars + 1] = '\0';    /* terminate extension + dot */
    }
}

//
//    FastUnicodeCompare - Compare two Unicode strings; produce a relative ordering
//
//        IF                RESULT
//    --------------------------
//    str1 < str2        =>    -1
//    str1 = str2        =>     0
//    str1 > str2        =>    +1
//
//    The lower case table starts with 256 entries (one for each of the upper bytes
//    of the original Unicode char).  If that entry is zero, then all characters with
//    that upper byte are already case folded.  If the entry is non-zero, then it is
//    the _index_ (not byte offset) of the start of the sub-table for the characters
//    with that upper byte.  All ignorable characters are folded to the value zero.
//
//    In pseudocode:
//
//        Let c = source Unicode character
//        Let table[] = lower case table
//
//        lower = table[highbyte(c)]
//        if (lower == 0)
//            lower = c
//        else
//            lower = table[lower+lowbyte(c)]
//
//        if (lower == 0)
//            ignore this character
//
//    To handle ignorable characters, we now need a loop to find the next valid character.
//    Also, we can't pre-compute the number of characters to compare; the string length might
//    be larger than the number of non-ignorable characters.  Further, we must be able to handle
//    ignorable characters at any point in the string, including as the first or last characters.
//    We use a zero value as a sentinel to detect both end-of-string and ignorable characters.
//    Since the File Manager doesn't prevent the NUL character (value zero) as part of a filename,
//    the case mapping table is assumed to map u+0000 to some non-zero value (like 0xFFFF, which is
//    an invalid Unicode character).
//
//    Pseudocode:
//
//        while (1) {
//            c1 = GetNextValidChar(str1)            //    returns zero if at end of string
//            c2 = GetNextValidChar(str2)
//
//            if (c1 != c2) break                    //    found a difference
//
//            if (c1 == 0)                        //    reached end of string on both strings at once?
//                return 0;                        //    yes, so strings are equal
//        }
//
//        // When we get here, c1 != c2.  So, we just need to determine which one is less.
//        if (c1 < c2)
//            return -1;
//        else
//            return 1;
//

int32_t FastUnicodeCompare ( register ConstUniCharArrayPtr str1, register ItemCount length1,
                            register ConstUniCharArrayPtr str2, register ItemCount length2)
{
    register u_int16_t     c1,c2;
    register u_int16_t     temp;
    register u_int16_t*    lowerCaseTable;

    lowerCaseTable = (u_int16_t*) gLowerCaseTable;

    while (1) {
        /* Set default values for c1, c2 in case there are no more valid chars */
        c1 = 0;
        c2 = 0;

        /* Find next non-ignorable char from str1, or zero if no more */
        while (length1 && c1 == 0) {
            c1 = *(str1++);
            --length1;
            /* check for basic latin first */
            if (c1 < 0x0100) {
                c1 = gLatinCaseFold[c1];
                break;
            }
            /* case fold if neccessary */
            if ((temp = lowerCaseTable[c1>>8]) != 0)
                c1 = lowerCaseTable[temp + (c1 & 0x00FF)];
        }


        /* Find next non-ignorable char from str2, or zero if no more */
        while (length2 && c2 == 0) {
            c2 = *(str2++);
            --length2;
            /* check for basic latin first */
            if (c2 < 0x0100) {
                c2 = gLatinCaseFold[c2];
                break;
            }
            /* case fold if neccessary */
            if ((temp = lowerCaseTable[c2>>8]) != 0)
                c2 = lowerCaseTable[temp + (c2 & 0x00FF)];
        }

        if (c1 != c2)        //    found a difference, so stop looping
            break;

        if (c1 == 0)        //    did we reach the end of both strings at the same time?
            return 0;        //    yes, so strings are equal
    }

    if (c1 < c2)
        return -1;
    else
        return 1;
}


/*
 * UnicodeBinaryCompare
 * Compare two UTF-16 strings and perform case-sensitive (binary) matching against them.
 *
 * Results are emitted like FastUnicodeCompare:
 *
 *
 *        IF                RESULT
 *    --------------------------
 *    str1 < str2        =>    -1
 *    str1 = str2        =>     0
 *    str1 > str2        =>    +1
 *
 * The case matching source code is greatly simplified due to the lack of case-folding
 * in this comparison routine. We compare, in order: the lengths, then do character-by-
 * character comparisons.
 *
 */
int32_t UnicodeBinaryCompare (register ConstUniCharArrayPtr str1, register ItemCount len1,
                              register ConstUniCharArrayPtr str2, register ItemCount len2) {
    uint16_t c1 =0;
    uint16_t c2 =0;
    ItemCount string_length;
    int32_t result = 0;

    /* First generate the string length (for comparison purposes) */
    if (len1 < len2) {
        string_length = len1;
        --result;
    }
    else if (len1 > len2) {
        string_length = len2;
        ++result;
    }
    else {
        string_length = len1;
    }

    /* now compare the two string pointers */
    while (string_length--) {
        c1 = *(str1++);
        c2 = *(str2++);

        if (c1 > c2) {
            result = 1;
            break;
        }

        if (c1 < c2) {
            result = -1;
            break;
        }
        /* If equal, iterate to the next two respective chars */
    }

    return result;
}

/*
 * extract the file id from a mangled name
 */
HFSCatalogNodeID
GetEmbeddedFileID(const unsigned char * filename, u_int32_t length, u_int32_t *prefixLength)
{
    short    extChars;
    short    i;
    u_int8_t    c;
    
    *prefixLength = 0;
    
    if ( filename == NULL )
        return 0;
    
    if ( length < 28 )
        return 0;    /* too small to have been mangled */
    
    /* big enough for a file ID (#10) and an extension (.x) ? */
    if ( length > 5 )
        extChars = CountFilenameExtensionChars(filename, length);
    else
        extChars = 0;
    
    /* skip over dot plus extension characters */
    if ( extChars > 0 )
        length -= (extChars + 1);
    
    /* scan for file id digits */
    for ( i = length - 1; i >= 0; --i) {
        c = filename[i];
        
        /* look for file ID marker */
        if ( c == '#' ) {
            if ( (length - i) < 3 )
                break;    /* too small to be a file ID */
            
            *prefixLength = i;
            return HexStringToInteger(length - i - 1, &filename[i+1]);
        }
        
        if ( !IsHexDigit(c) )
            break;    /* file ID string must have hex digits */
    }
    
    return 0;
}

/*
 * Count filename extension characters (if any)
 */
u_int32_t
CountFilenameExtensionChars( const unsigned char * filename, u_int32_t length )
{
    UniChar    c;
    u_int16_t  maxExtChars;

    if ( length < 3 )
        return 0;    /* "x.y" is smallest possible extension    */
    
    if ( length < (kMaxFileExtensionChars + 2) )
        maxExtChars = length - 2;    /* save room for prefix + dot */
    else
        maxExtChars = kMaxFileExtensionChars;
    
    u_int32_t extChars = 0;        /* number of extension chars (excluding dot) - assume there's no extension */
    u_int32_t i = length - 1;      /* index to last ascii character */

    while ( extChars <= maxExtChars ) {
        c = filename[i--];
        
        /* look for leading dot */
        if ( c == (u_int8_t) '.' )    {
            if ( extChars > 0 )    /* cannot end with a dot */
                return (extChars);
            
            break;
        }
        
        if ( EXTENSIONCHAR(c) )
            ++extChars;
        else
            break;
    }
    
    return 0;
}

static u_int32_t
HexStringToInteger(u_int32_t length, const u_int8_t *hexStr)
{
    u_int32_t        value;
    u_int32_t        i;
    u_int8_t        c;
    const u_int8_t    *p;
    
    value = 0;
    p = hexStr;
    
    for ( i = 0; i < length; ++i ) {
        c = *p++;
        
        if (c >= '0' && c <= '9') {
            value = value << 4;
            value += (u_int32_t) c - (u_int32_t) '0';
        } else if (c >= 'A' && c <= 'F') {
            value = value << 4;
            value += 10 + ((unsigned int) c - (unsigned int) 'A');
        } else {
            return 0;    /* bad character */
        }
    }
    
    return value;
}

OSErr
ConvertUnicodeToUTF8Mangled(ByteCount srcLen, ConstUniCharArrayPtr srcStr, ByteCount maxDstLen,
                            ByteCount *actualDstLen, unsigned char* dstStr, HFSCatalogNodeID cnid)
{
    ByteCount subMaxLen;
    size_t utf8len;
    char fileIDStr[15];
    char extStr[15];
    
    snprintf(fileIDStr, sizeof(fileIDStr), "#%X", cnid);
    GetFilenameExtension(srcLen/sizeof(UniChar), srcStr, extStr);
    
    /* remove extension chars from source */
    srcLen -= strlen(extStr) * sizeof(UniChar);
    subMaxLen = maxDstLen - (strlen(extStr) + strlen(fileIDStr));
    
    (void) utf8_encodestr(srcStr, srcLen, dstStr, &utf8len, subMaxLen, ':', UTF_ADD_NULL_TERM);
    
    strlcat((char *)dstStr, fileIDStr, maxDstLen);
    strlcat((char *)dstStr, extStr, maxDstLen);
    *actualDstLen = utf8len + (strlen(extStr) + strlen(fileIDStr));
    
    return noErr;
}
