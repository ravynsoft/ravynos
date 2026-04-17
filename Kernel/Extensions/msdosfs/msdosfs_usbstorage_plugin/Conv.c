/*
 
 Copyright © 2017 Apple Inc. All rights reserved.
 
 Conv.c
 usbstorage_plugin
 
 Created by Yakov Ben Zaken on 16/10/2017.
 
 */

#include "Conv.h"
#include <ctype.h>
#include "utfconvdata.h"

extern u_char l2u[256];

/*
 * The UTF-16 code points in a long name directory entry are scattered across
 * three areas within the directory entry.  This array contains the byte offsets
 * of each UTF-16 code point, making it easier to access them in a single loop.
 */
const uint8_t puLongNameOffset[13] = {
    1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30
};

#define MAX_MAC2SFM            (0x80)
#define MAX_SFM2MAC            (0x2a)
#define SFMCODE_PREFIX_MASK    (0xf000)

#define UCS_ALT_NULL    0x2400

/* Surrogate Pair Constants */
#define SP_HALF_SHIFT    10
#define SP_HALF_BASE    0x0010000U
#define SP_HALF_MASK    0x3FFU
#define SP_HIGH_FIRST    0xD800U
#define SP_HIGH_LAST    0xDBFFU
#define SP_LOW_FIRST    0xDC00U
#define SP_LOW_LAST    0xDFFFU

#define HANGUL_SBASE 0xAC00
#define HANGUL_LBASE 0x1100
#define HANGUL_VBASE 0x1161
#define HANGUL_TBASE 0x11A7
#define HANGUL_SCOUNT 11172
#define HANGUL_LCOUNT 19
#define HANGUL_VCOUNT 21
#define HANGUL_TCOUNT 28
#define HANGUL_NCOUNT (HANGUL_VCOUNT * HANGUL_TCOUNT)

#define RECURSIVE_DECOMPOSITION (1 << 15)
#define EXTRACT_COUNT(value)    (((value) >> 12) & 0x0007)

typedef struct {
    u_int16_t _key;
    u_int16_t _value;
} unicode_mappings16;

typedef struct {
    u_int32_t _key;
    u_int32_t _value;
} unicode_mappings32;

/*
 * Macintosh Unicode (LSB) to Microsoft Services for Macintosh (SFM) Unicode
 */
static const uint16_t
mac2sfm[MAX_MAC2SFM] = {
    0x0,    0xf001, 0xf002, 0xf003, 0xf004, 0xf005, 0xf006, 0xf007,     /* 00-07 */
    0xf008, 0xf009, 0xf00a, 0xf00b, 0xf00c, 0xf00d, 0xf00e, 0xf00f,     /* 08-0f */
    0xf010, 0xf011, 0xf012, 0xf013, 0xf014, 0xf015, 0xf016, 0xf017,     /* 10-17 */
    0xf018, 0xf019, 0xf01a, 0xf01b, 0xf01c, 0xf01d, 0xf01e, 0xf01f,     /* 18-1f */
    0x20,   0x21,   0xf020, 0x23,   0x24,   0x25,   0x26,   0x27,       /* 20-27 */
    0x28,   0x29,   0xf021, 0x2b,   0x2c,   0x2d,   0x2e,   0xf022,     /* 28-2f */
    0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,       /* 30-37 */
    0x38,   0x39,   0xf022, 0x3b,   0xf023, 0x3d,   0xf024, 0xf025,     /* 38-3f */
    0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,       /* 40-47 */
    0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,       /* 48-4f */
    0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,       /* 50-57 */
    0x58,   0x59,   0x5a,   0x5b,   0xf026, 0x5d,   0x5e,   0x5f,       /* 58-5f */
    0x60,   0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,       /* 60-67 */
    0x68,   0x69,   0x6a,   0x6b,   0x6c,   0x6d,   0x6e,   0x6f,       /* 68-6f */
    0x70,   0x71,   0x72,   0x73,   0x74,   0x75,   0x76,   0x77,       /* 70-77 */
    0x78,   0x79,   0x7a,   0x7b,   0xf027, 0x7d,   0x7e,   0x7f,       /* 78-7f */
};

/*
 * SFM Unicode (LSB) to Macintosh Unicode (LSB)
 */
static const uint8_t
sfm2mac[MAX_SFM2MAC] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,    /* 00-07 */
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,    /* 08-0F */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,    /* 10-17 */
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,    /* 18-1F */
    0x22, 0x2a, 0x3a, 0x3c, 0x3e, 0x3f, 0x5c, 0x7c,    /* 20-27 */
    0x20, 0x2e,                                        /* 28-29 */
};

signed char utf_extrabytes[32] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    -1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1,  1,  2,  2,  3, -1
};

const char hexdigits[16] = {
    '0',  '1',  '2',  '3',  '4',  '5',  '6', '7',
    '8',  '9',  'A',  'B',  'C',  'D',  'E', 'F'
};

// ---------------------------------------------------------------------------------------------
static uint16_t ucs_to_sfm(uint16_t uUnichodeCh, int lastchar);
static u_int32_t unicode_recursive_decompose(u_int16_t character, u_int16_t *convertedChars);
static u_int32_t getmappedvalue32(const unicode_mappings32 *theTable, u_int32_t numElem, u_int16_t character);
static u_int16_t getmappedvalue16(const unicode_mappings16 *theTable, u_int32_t numElem, u_int16_t character);
static int unicode_decomposeable(u_int16_t character);
static uint16_t sfm_to_ucs(uint16_t uUnichodeCh);
static void priortysort(uint16_t* characters, int count);
static uint8_t get_combining_class(uint16_t character);
static int unicode_combinable(uint16_t character);
static u_int16_t unicode_combine(u_int16_t base, u_int16_t combining);
// ---------------------------------------------------------------------------------------------

/*
 * Encode illegal NTFS filename characters into SFM Private Unicode characters
 *
 * Assumes non-zero ASCII input.
 */
static uint16_t ucs_to_sfm(uint16_t uUnichodeCh, int lastchar)
{
    /* The last character of filename cannot be a space or period. */
    if (lastchar) {
        if (uUnichodeCh == 0x20)
            return (0xf028);
        else if (uUnichodeCh == 0x2e)
            return (0xf029);
    }
    /* 0x01 - 0x1f is simple transformation. */
    if (uUnichodeCh <= 0x1f) {
        return ((uint16_t)(uUnichodeCh | 0xf000));
    } else /* 0x20 - 0x7f */ {
        uint16_t lsb = mac2sfm[uUnichodeCh];
        return lsb;
    }
    return (uUnichodeCh);
}

/*
 * Test for a combining character.
 *
 * Similar to __CFUniCharIsNonBaseCharacter except that
 * unicode_combinable also includes Hangul Jamo characters.
 */
static int unicode_combinable(uint16_t character)
{
    const uint8_t *bitmap = __CFUniCharCombiningBitmap;
    uint8_t value;
    if (character < 0x0300)
        return (0);
    value = bitmap[(character >> 8) & 0xFF];
    if (value == 0xFF) {
        return (1);
    } else if (value) {
        bitmap = bitmap + ((value - 1) * 32) + 256;
        return (bitmap[(character & 0xFF) / 8] & (1 << (character % 8)) ? 1 : 0);
    }
    return (0);
}

static uint8_t get_combining_class(uint16_t character) {
    const uint8_t *bitmap = __CFUniCharCombiningPropertyBitmap;
    uint8_t value = bitmap[(character >> 8)];
    if (value) {
        bitmap = bitmap + (value * 256);
        return bitmap[character % 256];
    }
    return (0);
}

/*
 * priortysort - order combining chars into canonical order
 *
 * Similar to CFUniCharPrioritySort
 */
static void priortysort(uint16_t* characters, int count)
{
    uint32_t p1, p2;
    uint16_t *ch1, *ch2;
    uint16_t *end;
    int changes = 1;
    uint16_t tmp, tmp2;
    end = characters + count;
    do {
        changes = 0;
        ch1 = characters;
        ch2 = characters + 1;
        tmp = *ch1;
        p2 = get_combining_class(tmp);
        while (ch2 < end) {
            p1 = p2;
            tmp = *ch2;
            p2 = get_combining_class(tmp);
            if (p1 > p2) {
                tmp  = *ch1;
                tmp2 = *ch2;
                *ch1 = tmp2;
                *ch2 = tmp;
                changes = 1;
            }
            ++ch1;
            ++ch2;
        }
    } while (changes);
}

/*
 * Decode any SFM Private Unicode characters
 */
static uint16_t sfm_to_ucs(uint16_t uUnichodeCh)
{
    if (((uUnichodeCh & 0xffC0) == SFMCODE_PREFIX_MASK) &&
        ((uUnichodeCh & 0x003f) <= MAX_SFM2MAC)) {
        uUnichodeCh = sfm2mac[uUnichodeCh & 0x003f];
    }
    return (uUnichodeCh);
}

/*
 * Test for a precomposed character.
 *
 * Similar to __CFUniCharIsDecomposableCharacter.
 */
static int unicode_decomposeable(u_int16_t character)
{
    const u_int8_t *bitmap = __CFUniCharDecomposableBitmap;
    u_int8_t value;
    if (character < 0x00C0) {
        return 0;
    }
    value = bitmap[(character >> 8) & 0xFF];
    if (value == 0xFF) {
        return 1;
    } else if (value) {
        bitmap = bitmap + ((value - 1) * 32) + 256;
        return bitmap[(character & 0xFF) / 8] & (1 << (character % 8)) ? 1 : 0;
    }
    return 0;
}

static u_int32_t getmappedvalue32(const unicode_mappings32 *theTable, u_int32_t numElem, u_int16_t character)
{
    const unicode_mappings32 *p, *q, *divider;
    if ((character < theTable[0]._key) || (character > theTable[numElem - 1]._key)) {
        return 0;
    }
    p = theTable;
    q = p + (numElem - 1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);   /* divide by 2 */
        if (character < divider->_key) {
            q = divider - 1;
        } else if (character > divider->_key) {
            p = divider + 1;
        } else {
            return divider->_value;
        }
    }
    return 0;
}

static u_int16_t getmappedvalue16(const unicode_mappings16 *theTable, u_int32_t numElem, u_int16_t character)
{
    const unicode_mappings16 *p, *q, *divider;
    if ((character < theTable[0]._key) || (character > theTable[numElem - 1]._key)) {
        return 0;
    }
    p = theTable;
    q = p + (numElem - 1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);   /* divide by 2 */
        if (character < divider->_key) {
            q = divider - 1;
        } else if (character > divider->_key) {
            p = divider + 1;
        } else {
            return divider->_value;
        }
    }
    return 0;
}

static u_int32_t unicode_recursive_decompose(u_int16_t character, u_int16_t *convertedChars)
{
    u_int16_t value;
    u_int32_t length;
    u_int16_t firstChar;
    u_int16_t theChar;
    const u_int16_t *bmpMappings;
    u_int32_t usedLength;
    value = getmappedvalue16((const unicode_mappings16 *)__CFUniCharDecompositionTable, __UniCharDecompositionTableLength, character);
    length = EXTRACT_COUNT(value);
    firstChar = value & 0x0FFF;
    theChar = firstChar;
    bmpMappings = (length == 1 ? &theChar : __CFUniCharMultipleDecompositionTable + firstChar);
    usedLength = 0;
    if (value & RECURSIVE_DECOMPOSITION) {
        usedLength = unicode_recursive_decompose((u_int16_t)*bmpMappings, convertedChars);
        --length; /* Decrement for the first char */
        if (!usedLength) {
            return 0;
        }
        ++bmpMappings;
        convertedChars += usedLength;
    }
    usedLength += length;
    while (length--) {
        *(convertedChars++) = *(bmpMappings++);
    }
    return usedLength;
}

/*
 * unicode_decompose - decompose a composed Unicode char
 *
 * Composed Unicode characters are forbidden on
 * HFS Plus volumes. ucs_decompose will convert a
 * composed character into its correct decomposed
 * sequence.
 *
 * Similar to CFUniCharDecomposeCharacter
 */
static int unicode_decompose(u_int16_t character, u_int16_t *convertedChars)
{
    if ((character >= HANGUL_SBASE) &&
        (character <= (HANGUL_SBASE + HANGUL_SCOUNT))) {
        u_int32_t length;
        character -= HANGUL_SBASE;
        length = (character % HANGUL_TCOUNT ? 3 : 2);
        *(convertedChars++) =
        character / HANGUL_NCOUNT + HANGUL_LBASE;
        *(convertedChars++) =
        (character % HANGUL_NCOUNT) / HANGUL_TCOUNT + HANGUL_VBASE;
        if (length > 2) {
            *convertedChars = (character % HANGUL_TCOUNT) + HANGUL_TBASE;
        }
        return length;
    } else {
        return unicode_recursive_decompose(character, convertedChars);
    }
}

/*
 * unicode_combine - generate a precomposed Unicode char
 *
 * Precomposed Unicode characters are required for some volume
 * formats and network protocols.  unicode_combine will combine
 * a decomposed character sequence into a single precomposed
 * (composite) character.
 *
 * Similar toCFUniCharPrecomposeCharacter but unicode_combine
 * also handles Hangul Jamo characters.
 */
static u_int16_t unicode_combine(u_int16_t base, u_int16_t combining)
{
    u_int32_t value;
    /* Check HANGUL */
    if ((combining >= HANGUL_VBASE) && (combining < (HANGUL_TBASE + HANGUL_TCOUNT))) {
        /* 2 char Hangul sequences */
        if ((combining < (HANGUL_VBASE + HANGUL_VCOUNT)) &&
            (base >= HANGUL_LBASE && base < (HANGUL_LBASE + HANGUL_LCOUNT))) {
            return HANGUL_SBASE +
            ((base - HANGUL_LBASE) * (HANGUL_VCOUNT * HANGUL_TCOUNT)) +
            ((combining  - HANGUL_VBASE) * HANGUL_TCOUNT);
        }
        /* 3 char Hangul sequences */
        if ((combining > HANGUL_TBASE) &&
            (base >= HANGUL_SBASE && base < (HANGUL_SBASE + HANGUL_SCOUNT))) {
            if ((base - HANGUL_SBASE) % HANGUL_TCOUNT) {
                return 0;
            } else {
                return base + (combining - HANGUL_TBASE);
            }
        }
    }
    value = getmappedvalue32( (const unicode_mappings32 *)__CFUniCharPrecompSourceTable, __CFUniCharPrecompositionTableLength, combining);
    if (value) {
        value = getmappedvalue16( (const unicode_mappings16 *) ((const u_int32_t *)__CFUniCharBMPPrecompDestinationTable + (value & 0xFFFF)), (value >> 16), base);
    }
    return value;
}

// ---------------------------------------------------------------------------------------------

errno_t
CONV_UTF8ToUnistr255(const uint8_t *puUtf8Ch, size_t utf8len, struct unistr255 *unicode, uint32_t uFlags)
{
    int iErr = 0;

    uint16_t* puUnichodeCh = unicode->chars;
    uint16_t* puBuffStart = puUnichodeCh;
    uint16_t* puBuffEnd = puBuffStart + 255;
    bool bUseFSM = uFlags & UTF_SFM_CONVERSIONS;
    
    unsigned int uUnichodeCh, uUnichodeCh2, uUnichodeCh3;
    unsigned int byte;
    int iCombCharCount = 0;
    int iExtrabytes;

    while (utf8len-- > 0 && (byte = *puUtf8Ch++) != '\0')
    {
        if (puUnichodeCh >= puBuffEnd)
            goto toolong;
        /* check for ascii */
        if (byte < 0x80)
        {
            //handle altslash
            uUnichodeCh = (bUseFSM || byte == '/') ? ucs_to_sfm((uint16_t)byte, utf8len == 0) : byte;
        }
        else
        {
            uint32_t ch;
            iExtrabytes = utf_extrabytes[byte >> 3];
            if ((iExtrabytes < 0) || ((int)utf8len < iExtrabytes))
            {
                goto escape;
            }
            utf8len -= iExtrabytes;
            switch (iExtrabytes) {
                case 1:
                    ch = byte; ch <<= 6;   /* 1st byte */
                    byte = *puUtf8Ch++;       /* 2nd byte */
                    if ((byte >> 6) != 2)
                        goto escape2;
                    ch += byte;
                    ch -= 0x00003080U;
                    if (ch < 0x0080)
                        goto escape2;
                    uUnichodeCh = ch;
                    break;
                case 2:
                    ch = byte; ch <<= 6;   /* 1st byte */
                    byte = *puUtf8Ch++;       /* 2nd byte */
                    if ((byte >> 6) != 2)
                        goto escape2;
                    ch += byte; ch <<= 6;
                    byte = *puUtf8Ch++;       /* 3rd byte */
                    if ((byte >> 6) != 2)
                        goto escape3;
                    ch += byte;
                    ch -= 0x000E2080U;
                    if (ch < 0x0800)
                        goto escape3;
                    if (ch >= 0xD800) {
                        if (ch <= 0xDFFF)
                            goto escape3;
                        if (ch == 0xFFFE || ch == 0xFFFF)
                            goto escape3;
                    }
                    uUnichodeCh = ch;
                    break;
                case 3:
                    ch = byte; ch <<= 6;   /* 1st byte */
                    byte = *puUtf8Ch++;       /* 2nd byte */
                    if ((byte >> 6) != 2)
                        goto escape2;
                    ch += byte; ch <<= 6;
                    byte = *puUtf8Ch++;       /* 3rd byte */
                    if ((byte >> 6) != 2)
                        goto escape3;
                    ch += byte; ch <<= 6;
                    byte = *puUtf8Ch++;       /* 4th byte */
                    if ((byte >> 6) != 2)
                        goto escape4;
                    ch += byte;
                    ch -= 0x03C82080U + SP_HALF_BASE;
                    uUnichodeCh = (ch >> SP_HALF_SHIFT) + SP_HIGH_FIRST;
                    if (uUnichodeCh < SP_HIGH_FIRST || uUnichodeCh > SP_HIGH_LAST)
                        goto escape4;
                    *puUnichodeCh = uUnichodeCh;
                    ++puUnichodeCh;
                    if (puUnichodeCh >= puBuffEnd)
                        goto toolong;
                    uUnichodeCh = (ch & SP_HALF_MASK) + SP_LOW_FIRST;
                    if (uUnichodeCh < SP_LOW_FIRST || uUnichodeCh > SP_LOW_LAST) {
                        --puUnichodeCh;
                        goto escape4;
                    }
                    *puUnichodeCh = uUnichodeCh;
                    ++puUnichodeCh;
                    continue;
                default:
                    iErr = EINVAL;
                    goto exit;
            }
            if (puUnichodeCh != puBuffStart)
            {
                u_int16_t composite, base;
                if (unicode_combinable(uUnichodeCh)) {
                    base = puUnichodeCh[-1];
                    composite = unicode_combine(base, uUnichodeCh);
                    if (composite) {
                        --puUnichodeCh;
                        uUnichodeCh = composite;
                    }
                }
            }

            if (uUnichodeCh == UCS_ALT_NULL)
                uUnichodeCh = '\0';
        }

        /*
         * Make multiple combining character sequences canonical
         */
        if (unicode_combinable((uint16_t)uUnichodeCh)) {
            ++iCombCharCount;   /* start tracking a run */
        } else if (iCombCharCount) {
            if (iCombCharCount > 1) {
                priortysort(puUnichodeCh - iCombCharCount, iCombCharCount);
            }
            iCombCharCount = 0;  /* start over */
        }
        *puUnichodeCh = uUnichodeCh;
        ++puUnichodeCh;
        continue;
        /*
         * Escape illegal UTF-8 into something legal.
         */
    escape4:
        puUtf8Ch -= 3;
        goto escape;
    escape3:
        puUtf8Ch -= 2;
        goto escape;
    escape2:
        puUtf8Ch -= 1;
    escape:
        if (iExtrabytes > 0)
            utf8len += iExtrabytes;
        byte = *(puUtf8Ch - 1);
        if ((puUnichodeCh + 2) >= puBuffEnd)
            goto toolong;
        uUnichodeCh  = '%';
        uUnichodeCh2 = hexdigits[byte >> 4];
        uUnichodeCh3 = hexdigits[byte & 0x0F];
        *puUnichodeCh = uUnichodeCh;  ++puUnichodeCh;
        *puUnichodeCh = uUnichodeCh2; ++puUnichodeCh;
        *puUnichodeCh = uUnichodeCh3; ++puUnichodeCh;
    }
    /*
     * Make a previous combining sequence canonical
     */
    if (iCombCharCount > 1) {
        priortysort(puUnichodeCh - iCombCharCount, iCombCharCount);
    }

exit:
    unicode->length = ((uint8_t*)puUnichodeCh - (uint8_t*)puBuffStart)/sizeof(uint16_t);
    return (iErr);
toolong:
    iErr = ENAMETOOLONG;
    goto exit;
}

size_t
CONV_Unistr255ToUTF8(const struct unistr255 *utf16, char utf8[FAT_MAX_FILENAME_UTF8])
{
    int iErr  = 0;

    const uint16_t* puUnichodeCh = utf16->chars;
    uint8_t*  puUtf8Ch = (uint8_t *) &utf8[0];
    uint8_t*  puBuffEnd = puUtf8Ch + FAT_MAX_FILENAME_UTF8 -1; //since we have null terminate at the end
    uint16_t  uUnichodeCh;
    uint16_t* puCh = NULL;

    uint32_t uChCount = utf16->length;
    int iExtra = 0;
    uint16_t uSequence[8];

    while (uChCount-- > 0)
    {
        if (iExtra > 0)
        {
            --iExtra;
            uUnichodeCh = *puCh++; // Note: puCh is from the local, aligned "uSequence" array.
        }
        else
        {
            uUnichodeCh = *puUnichodeCh; ++puUnichodeCh;

            if (unicode_decomposeable(uUnichodeCh)) {
                iExtra = unicode_decompose(uUnichodeCh, uSequence) - 1;
                uChCount += iExtra;
                uUnichodeCh = uSequence[0];
                puCh = &uSequence[1];
            }
        }

        /* NULL is not permitted */
        /* Slash and NULL are not permitted */
        if (uUnichodeCh == '/') {
            uUnichodeCh = '_';
            iErr = EINVAL;
        } else if (uUnichodeCh == '\0') {
            uUnichodeCh = UCS_ALT_NULL;
        }

        if (uUnichodeCh < 0x0080) {
            if (puUtf8Ch >= puBuffEnd) {
                iErr = ENAMETOOLONG;
                break;
            }
            *puUtf8Ch++ = (uint8_t)uUnichodeCh;
        } else if (uUnichodeCh < 0x800) {
            if ((puUtf8Ch + 1) >= puBuffEnd) {
                iErr = ENAMETOOLONG;
                break;
            }
            *puUtf8Ch++ = (uint8_t)(0xc0 | (uUnichodeCh >> 6));
            *puUtf8Ch++ = (uint8_t)(0x80 | (0x3f & uUnichodeCh));
        } else {
            /* These chars never valid Unicode. */
            if (uUnichodeCh == 0xFFFE || uUnichodeCh == 0xFFFF) {
                iErr = EINVAL;
                break;
            }
            /* Combine valid surrogate pairs */
            if (uUnichodeCh >= SP_HIGH_FIRST && uUnichodeCh <= SP_HIGH_LAST && uChCount > 0)
            {
                uint16_t ch2;
                uint32_t pair;
                ch2 = *puUnichodeCh;
                if (ch2 >= SP_LOW_FIRST && ch2 <= SP_LOW_LAST)
                {
                    pair = ((uUnichodeCh - SP_HIGH_FIRST) << SP_HALF_SHIFT) + (ch2 - SP_LOW_FIRST) + SP_HALF_BASE;
                    if ((puUtf8Ch + 3) >= puBuffEnd)
                    {
                        iErr = ENAMETOOLONG;
                        break;
                    }
                    --uChCount;
                    ++puUnichodeCh;
                    *puUtf8Ch++ = (uint8_t)(0xf0 | (pair >> 18));
                    *puUtf8Ch++ = (uint8_t)(0x80 | (0x3f & (pair >> 12)));
                    *puUtf8Ch++ = (uint8_t)(0x80 | (0x3f & (pair >> 6)));
                    *puUtf8Ch++ = (uint8_t)(0x80 | (0x3f & pair));
                    continue;
                }
            } else {
                uUnichodeCh = sfm_to_ucs(uUnichodeCh);
                if (uUnichodeCh < 0x0080) {
                    if (puUtf8Ch >= puBuffEnd) {
                        iErr = ENAMETOOLONG;
                        break;
                    }
                    *puUtf8Ch++ = (uint8_t)uUnichodeCh;
                    continue;
                }
            }
            if ((puUtf8Ch + 2) >= puBuffEnd) {
                iErr = ENAMETOOLONG;
                break;
            }
            *puUtf8Ch++ = (uint8_t)(0xe0 | (uUnichodeCh >> 12));
            *puUtf8Ch++ = (uint8_t)(0x80 | (0x3f & (uUnichodeCh >> 6)));
            *puUtf8Ch++ = (uint8_t)(0x80 | (0x3f & uUnichodeCh));
        }
    }

    *puUtf8Ch = '\0';
    return (iErr);
}

void
CONV_Unistr255ToLowerCase( struct unistr255* psUnistr255 )
{
    for ( uint16_t uIdx=0; uIdx<psUnistr255->length; uIdx++ )
    {
        if ( psUnistr255->chars[uIdx] < 0x100 )
        {
            psUnistr255->chars[uIdx] = l2u[psUnistr255->chars[uIdx]];
        }
    }
}

void
CONV_GetCurrentTime( struct timespec* psTS )
{
    clock_gettime( CLOCK_REALTIME, psTS );
}

errno_t
CONV_UTF8ToLowerCase( char* pcFileNameUTF8, char pcFileNameUTF8LowerCase[FAT_MAX_FILENAME_UTF8])
{
    if (pcFileNameUTF8 == NULL || pcFileNameUTF8LowerCase == NULL)
        return EINVAL;
    
    struct unistr255* psName = (struct unistr255*) malloc(sizeof(struct unistr255));
    if (psName == NULL)
        return ENOMEM;

    CONV_UTF8ToUnistr255((const uint8_t *) pcFileNameUTF8, strlen(pcFileNameUTF8), psName, UTF_SFM_CONVERSIONS);
    CONV_Unistr255ToLowerCase( psName );
    CONV_Unistr255ToUTF8(psName, pcFileNameUTF8LowerCase);

    free(psName);
    return 0;
}

void CONV_convert_to_fsm(struct unistr255* unicode)
{
    for ( uint16_t uIdx=0; uIdx<unicode->length; uIdx++ )
    {
        //If the last char is "." or " " we need to change it.
        //We won't use the mac2sfm table, we will do it manually
        if ( uIdx == unicode->length-1 )
        {
            if ( unicode->chars[uIdx] == ' ' )
            {
                unicode->chars[uIdx] = 0xf028;
                continue;
            }

            if ( unicode->chars[uIdx] == '.' )
            {
                unicode->chars[uIdx] = 0xf029;
                continue;
            }
        }

        if ( unicode->chars[uIdx] < MAX_MAC2SFM )
        {
            unicode->chars[uIdx] = mac2sfm[unicode->chars[uIdx]];
        }
    }
}

void CONV_DuplicateName(char** ppcNewUTF8Name, const char *pcUTF8Name)
{
    *ppcNewUTF8Name = NULL;

    if (pcUTF8Name == NULL)
        return;

    uint32_t uNameLength = (uint32_t) strlen(pcUTF8Name) + 1;
    *ppcNewUTF8Name = malloc(uNameLength);
    if (*ppcNewUTF8Name == NULL)
        return;

    strcpy(*ppcNewUTF8Name, pcUTF8Name);
}
