/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFBuiltinConverters.c
	Copyright (c) 1999-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include "CFStringEncodingConverterExt.h"
#include "CFUniChar.h"
#include "CFUnicodeDecomposition.h"
#include "CFUnicodePrecomposition.h"
#include "CFStringEncodingConverterPriv.h"
#include "CFInternal.h"

#define ParagraphSeparator 0x2029
#define ASCIINewLine 0x0a
static int8_t __CFMapsParagraphSeparator = -1;

CF_INLINE bool __CFIsParagraphSeparator(UTF16Char character) {
    if (-1 == __CFMapsParagraphSeparator) __CFMapsParagraphSeparator = (1 ? false : true);

    return ((__CFMapsParagraphSeparator && (ParagraphSeparator == character)) ? true : false);
}

/* Precomposition */
static const uint32_t __CFLatin1CombiningCharBitmap[] = { // 0x300 ~ 0x35FF
    0xFBB94010, 0x01800000, 0x0000000,
};

bool CFStringEncodingIsValidCombiningCharacterForLatin1(UniChar character) {
    return ((character >= 0x300) && (character < 0x360) && (__CFLatin1CombiningCharBitmap[(character - 0x300) / 32] & (1 << (31 - ((character - 0x300) % 32)))) ? true : false);
}

UniChar CFStringEncodingPrecomposeLatinCharacter(const UniChar *character, CFIndex numChars, CFIndex *usedChars) {
    if (numChars > 0) {
        UTF32Char ch = *(character++), nextCh, composedChar;
        CFIndex usedCharLen = 1;

        if (CFUniCharIsSurrogateHighCharacter(ch) || CFUniCharIsSurrogateLowCharacter(ch)) {
            if (usedChars) (*usedChars) = usedCharLen;
            return ch;
        }

        while (usedCharLen < numChars) {
            nextCh = *(character++);

            if (CFUniCharIsSurrogateHighCharacter(nextCh) || CFUniCharIsSurrogateLowCharacter(nextCh)) break;

            if (CFUniCharIsMemberOf(nextCh, kCFUniCharNonBaseCharacterSet) && ((composedChar = CFUniCharPrecomposeCharacter(ch, nextCh)) != 0xFFFD)) {
                if (composedChar > 0xFFFF) { // Non-base
                    break;
                } else {
                    ch = composedChar;
                }
            } else {
                break;
            }
            ++usedCharLen;
        }
        if (usedChars) (*usedChars) = usedCharLen;
        if (usedCharLen > 1) return ch;
    }
    return 0xFFFD;
}

/* ASCII */
static bool __CFToASCII(uint32_t flags, UniChar character, uint8_t *byte) {
    if (character < 0x80) {
        *byte = (uint8_t)character;
    } else if (__CFIsParagraphSeparator(character)) {
        *byte = ASCIINewLine;
    } else {
        return false;
    }
    return true;
}

static bool __CFFromASCII(uint32_t flags, uint8_t byte, UniChar *character) {
    if (byte < 0x80) {
        *character = (UniChar)byte;
        return true;
    } else {
        return false;
    }
}


CF_PRIVATE const CFStringEncodingConverter __CFConverterASCII = {
    __CFToASCII, __CFFromASCII, 1, 1, kCFStringEncodingConverterCheapEightBit,
    NULL, NULL, NULL, NULL, NULL, NULL,
};

/* ISO Latin 1 (8859-1) */
static bool __CFToISOLatin1(uint32_t flags, UniChar character, uint8_t *byte) {
    if (character <= 0xFF) {
        *byte = (uint8_t)character;
    } else if (__CFIsParagraphSeparator(character)) {
        *byte = ASCIINewLine;
    } else {
        return false;
    }

    return true;
}

static bool __CFFromISOLatin1(uint32_t flags, uint8_t byte, UniChar *character) {
    *character = (UniChar)byte;
    return true;
}

static CFIndex __CFToISOLatin1Precompose(uint32_t flags, const UniChar *character, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    uint8_t byte;
    CFIndex usedCharLen;

    if (__CFToISOLatin1(flags, CFStringEncodingPrecomposeLatinCharacter(character, numChars, &usedCharLen), &byte) && byte && (usedCharLen > 1)) {
        if (maxByteLen) *bytes = byte;
        *usedByteLen = 1;
        return usedCharLen;
    } else {
        return 0;
    }
}

CF_PRIVATE const CFStringEncodingConverter __CFConverterISOLatin1 = {
    __CFToISOLatin1, __CFFromISOLatin1, 1, 1, kCFStringEncodingConverterCheapEightBit,
    NULL, NULL, NULL, NULL, __CFToISOLatin1Precompose, CFStringEncodingIsValidCombiningCharacterForLatin1,
};

/* Mac Roman */
#define NUM_MACROMAN_FROM_UNI 129
static const CFStringEncodingUnicodeTo8BitCharMap macRoman_from_uni[NUM_MACROMAN_FROM_UNI] = {
    { 0x00A0, 0xCA }, /* NO-BREAK SPACE */
    { 0x00A1, 0xC1 }, /* INVERTED EXCLAMATION MARK */
    { 0x00A2, 0xA2 }, /* CENT SIGN */
    { 0x00A3, 0xA3 }, /* POUND SIGN */
    { 0x00A5, 0xB4 }, /* YEN SIGN */
    { 0x00A7, 0xA4 }, /* SECTION SIGN */
    { 0x00A8, 0xAC }, /* DIAERESIS */
    { 0x00A9, 0xA9 }, /* COPYRIGHT SIGN */
    { 0x00AA, 0xBB }, /* FEMININE ORDINAL INDICATOR */
    { 0x00AB, 0xC7 }, /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    { 0x00AC, 0xC2 }, /* NOT SIGN */
    { 0x00AE, 0xA8 }, /* REGISTERED SIGN */
    { 0x00AF, 0xF8 }, /* MACRON */
    { 0x00B0, 0xA1 }, /* DEGREE SIGN */
    { 0x00B1, 0xB1 }, /* PLUS-MINUS SIGN */
    { 0x00B4, 0xAB }, /* ACUTE ACCENT */
    { 0x00B5, 0xB5 }, /* MICRO SIGN */
    { 0x00B6, 0xA6 }, /* PILCROW SIGN */
    { 0x00B7, 0xE1 }, /* MIDDLE DOT */
    { 0x00B8, 0xFC }, /* CEDILLA */
    { 0x00BA, 0xBC }, /* MASCULINE ORDINAL INDICATOR */
    { 0x00BB, 0xC8 }, /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    { 0x00BF, 0xC0 }, /* INVERTED QUESTION MARK */
    { 0x00C0, 0xCB }, /* LATIN CAPITAL LETTER A WITH GRAVE */
    { 0x00C1, 0xE7 }, /* LATIN CAPITAL LETTER A WITH ACUTE */
    { 0x00C2, 0xE5 }, /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    { 0x00C3, 0xCC }, /* LATIN CAPITAL LETTER A WITH TILDE */
    { 0x00C4, 0x80 }, /* LATIN CAPITAL LETTER A WITH DIAERESIS */
    { 0x00C5, 0x81 }, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
    { 0x00C6, 0xAE }, /* LATIN CAPITAL LIGATURE AE */
    { 0x00C7, 0x82 }, /* LATIN CAPITAL LETTER C WITH CEDILLA */
    { 0x00C8, 0xE9 }, /* LATIN CAPITAL LETTER E WITH GRAVE */
    { 0x00C9, 0x83 }, /* LATIN CAPITAL LETTER E WITH ACUTE */
    { 0x00CA, 0xE6 }, /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    { 0x00CB, 0xE8 }, /* LATIN CAPITAL LETTER E WITH DIAERESIS */
    { 0x00CC, 0xED }, /* LATIN CAPITAL LETTER I WITH GRAVE */
    { 0x00CD, 0xEA }, /* LATIN CAPITAL LETTER I WITH ACUTE */
    { 0x00CE, 0xEB }, /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
    { 0x00CF, 0xEC }, /* LATIN CAPITAL LETTER I WITH DIAERESIS */
    { 0x00D1, 0x84 }, /* LATIN CAPITAL LETTER N WITH TILDE */
    { 0x00D2, 0xF1 }, /* LATIN CAPITAL LETTER O WITH GRAVE */
    { 0x00D3, 0xEE }, /* LATIN CAPITAL LETTER O WITH ACUTE */
    { 0x00D4, 0xEF }, /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    { 0x00D5, 0xCD }, /* LATIN CAPITAL LETTER O WITH TILDE */
    { 0x00D6, 0x85 }, /* LATIN CAPITAL LETTER O WITH DIAERESIS */
    { 0x00D8, 0xAF }, /* LATIN CAPITAL LETTER O WITH STROKE */
    { 0x00D9, 0xF4 }, /* LATIN CAPITAL LETTER U WITH GRAVE */
    { 0x00DA, 0xF2 }, /* LATIN CAPITAL LETTER U WITH ACUTE */
    { 0x00DB, 0xF3 }, /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
    { 0x00DC, 0x86 }, /* LATIN CAPITAL LETTER U WITH DIAERESIS */
    { 0x00DF, 0xA7 }, /* LATIN SMALL LETTER SHARP S */
    { 0x00E0, 0x88 }, /* LATIN SMALL LETTER A WITH GRAVE */
    { 0x00E1, 0x87 }, /* LATIN SMALL LETTER A WITH ACUTE */
    { 0x00E2, 0x89 }, /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
    { 0x00E3, 0x8B }, /* LATIN SMALL LETTER A WITH TILDE */
    { 0x00E4, 0x8A }, /* LATIN SMALL LETTER A WITH DIAERESIS */
    { 0x00E5, 0x8C }, /* LATIN SMALL LETTER A WITH RING ABOVE */
    { 0x00E6, 0xBE }, /* LATIN SMALL LIGATURE AE */
    { 0x00E7, 0x8D }, /* LATIN SMALL LETTER C WITH CEDILLA */
    { 0x00E8, 0x8F }, /* LATIN SMALL LETTER E WITH GRAVE */
    { 0x00E9, 0x8E }, /* LATIN SMALL LETTER E WITH ACUTE */
    { 0x00EA, 0x90 }, /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
    { 0x00EB, 0x91 }, /* LATIN SMALL LETTER E WITH DIAERESIS */
    { 0x00EC, 0x93 }, /* LATIN SMALL LETTER I WITH GRAVE */
    { 0x00ED, 0x92 }, /* LATIN SMALL LETTER I WITH ACUTE */
    { 0x00EE, 0x94 }, /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
    { 0x00EF, 0x95 }, /* LATIN SMALL LETTER I WITH DIAERESIS */
    { 0x00F1, 0x96 }, /* LATIN SMALL LETTER N WITH TILDE */
    { 0x00F2, 0x98 }, /* LATIN SMALL LETTER O WITH GRAVE */
    { 0x00F3, 0x97 }, /* LATIN SMALL LETTER O WITH ACUTE */
    { 0x00F4, 0x99 }, /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
    { 0x00F5, 0x9B }, /* LATIN SMALL LETTER O WITH TILDE */
    { 0x00F6, 0x9A }, /* LATIN SMALL LETTER O WITH DIAERESIS */
    { 0x00F7, 0xD6 }, /* DIVISION SIGN */
    { 0x00F8, 0xBF }, /* LATIN SMALL LETTER O WITH STROKE */
    { 0x00F9, 0x9D }, /* LATIN SMALL LETTER U WITH GRAVE */
    { 0x00FA, 0x9C }, /* LATIN SMALL LETTER U WITH ACUTE */
    { 0x00FB, 0x9E }, /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
    { 0x00FC, 0x9F }, /* LATIN SMALL LETTER U WITH DIAERESIS */
    { 0x00FF, 0xD8 }, /* LATIN SMALL LETTER Y WITH DIAERESIS */
    { 0x0131, 0xF5 }, /* LATIN SMALL LETTER DOTLESS I */
    { 0x0152, 0xCE }, /* LATIN CAPITAL LIGATURE OE */
    { 0x0153, 0xCF }, /* LATIN SMALL LIGATURE OE */
    { 0x0178, 0xD9 }, /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
    { 0x0192, 0xC4 }, /* LATIN SMALL LETTER F WITH HOOK */
    { 0x02C6, 0xF6 }, /* MODIFIER LETTER CIRCUMFLEX ACCENT */
    { 0x02C7, 0xFF }, /* CARON */
    { 0x02D8, 0xF9 }, /* BREVE */
    { 0x02D9, 0xFA }, /* DOT ABOVE */
    { 0x02DA, 0xFB }, /* RING ABOVE */
    { 0x02DB, 0xFE }, /* OGONEK */
    { 0x02DC, 0xF7 }, /* SMALL TILDE */
    { 0x02DD, 0xFD }, /* DOUBLE ACUTE ACCENT */
    { 0x03A9, 0xBD }, /* OHM SIGN (Canonical ?) */
    { 0x03C0, 0xB9 }, /* GREEK SMALL LETTER PI */
    { 0x2013, 0xD0 }, /* EN DASH */
    { 0x2014, 0xD1 }, /* EM DASH */
    { 0x2018, 0xD4 }, /* LEFT SINGLE QUOTATION MARK */
    { 0x2019, 0xD5 }, /* RIGHT SINGLE QUOTATION MARK */
    { 0x201A, 0xE2 }, /* SINGLE LOW-9 QUOTATION MARK */
    { 0x201C, 0xD2 }, /* LEFT DOUBLE QUOTATION MARK */
    { 0x201D, 0xD3 }, /* RIGHT DOUBLE QUOTATION MARK */
    { 0x201E, 0xE3 }, /* DOUBLE LOW-9 QUOTATION MARK */
    { 0x2020, 0xA0 }, /* DAGGER */
    { 0x2021, 0xE0 }, /* DOUBLE DAGGER */
    { 0x2022, 0xA5 }, /* BULLET */
    { 0x2026, 0xC9 }, /* HORIZONTAL ELLIPSIS */
    { 0x2030, 0xE4 }, /* PER MILLE SIGN */
    { 0x2039, 0xDC }, /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
    { 0x203A, 0xDD }, /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
    { 0x2044, 0xDA }, /* FRACTION SLASH */
    { 0x20AC, 0xDB }, /* EURO SIGN */
    { 0x2122, 0xAA }, /* TRADE MARK SIGN */
    { 0x2126, 0xBD }, /* OHM SIGN */
    { 0x2202, 0xB6 }, /* PARTIAL DIFFERENTIAL */
    { 0x2206, 0xC6 }, /* INCREMENT */
    { 0x220F, 0xB8 }, /* N-ARY PRODUCT */
    { 0x2211, 0xB7 }, /* N-ARY SUMMATION */
    { 0x221A, 0xC3 }, /* SQUARE ROOT */
    { 0x221E, 0xB0 }, /* INFINITY */
    { 0x222B, 0xBA }, /* INTEGRAL */
    { 0x2248, 0xC5 }, /* ALMOST EQUAL TO */
    { 0x2260, 0xAD }, /* NOT EQUAL TO */
    { 0x2264, 0xB2 }, /* LESS-THAN OR EQUAL TO */
    { 0x2265, 0xB3 }, /* GREATER-THAN OR EQUAL TO */
    { 0x25CA, 0xD7 }, /* LOZENGE */
    { 0xF8FF, 0xF0 }, /* Apple logo */
    { 0xFB01, 0xDE }, /* LATIN SMALL LIGATURE FI */
    { 0xFB02, 0xDF }, /* LATIN SMALL LIGATURE FL */
};

static bool __CFToMacRoman(uint32_t flags, UniChar character, uint8_t *byte) {
    if (character < 0x80) {
        *byte = (uint8_t)character;
        return true;
    } else {
        return CFStringEncodingUnicodeTo8BitEncoding(macRoman_from_uni, NUM_MACROMAN_FROM_UNI, character, byte);
    }
}

static const UniChar macRoman_to_uni[128] = {
    0x00C4, /* LATIN CAPITAL LETTER A WITH DIAERESIS */
    0x00C5, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
    0x00C7, /* LATIN CAPITAL LETTER C WITH CEDILLA */
    0x00C9, /* LATIN CAPITAL LETTER E WITH ACUTE */
    0x00D1, /* LATIN CAPITAL LETTER N WITH TILDE */
    0x00D6, /* LATIN CAPITAL LETTER O WITH DIAERESIS */
    0x00DC, /* LATIN CAPITAL LETTER U WITH DIAERESIS */
    0x00E1, /* LATIN SMALL LETTER A WITH ACUTE */
    0x00E0, /* LATIN SMALL LETTER A WITH GRAVE */
    0x00E2, /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
    0x00E4, /* LATIN SMALL LETTER A WITH DIAERESIS */
    0x00E3, /* LATIN SMALL LETTER A WITH TILDE */
    0x00E5, /* LATIN SMALL LETTER A WITH RING ABOVE */
    0x00E7, /* LATIN SMALL LETTER C WITH CEDILLA */
    0x00E9, /* LATIN SMALL LETTER E WITH ACUTE */
    0x00E8, /* LATIN SMALL LETTER E WITH GRAVE */
    0x00EA, /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
    0x00EB, /* LATIN SMALL LETTER E WITH DIAERESIS */
    0x00ED, /* LATIN SMALL LETTER I WITH ACUTE */
    0x00EC, /* LATIN SMALL LETTER I WITH GRAVE */
    0x00EE, /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
    0x00EF, /* LATIN SMALL LETTER I WITH DIAERESIS */
    0x00F1, /* LATIN SMALL LETTER N WITH TILDE */
    0x00F3, /* LATIN SMALL LETTER O WITH ACUTE */
    0x00F2, /* LATIN SMALL LETTER O WITH GRAVE */
    0x00F4, /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
    0x00F6, /* LATIN SMALL LETTER O WITH DIAERESIS */
    0x00F5, /* LATIN SMALL LETTER O WITH TILDE */
    0x00FA, /* LATIN SMALL LETTER U WITH ACUTE */
    0x00F9, /* LATIN SMALL LETTER U WITH GRAVE */
    0x00FB, /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
    0x00FC, /* LATIN SMALL LETTER U WITH DIAERESIS */
    0x2020, /* DAGGER */
    0x00B0, /* DEGREE SIGN */
    0x00A2, /* CENT SIGN */
    0x00A3, /* POUND SIGN */
    0x00A7, /* SECTION SIGN */
    0x2022, /* BULLET */
    0x00B6, /* PILCROW SIGN */
    0x00DF, /* LATIN SMALL LETTER SHARP S */
    0x00AE, /* REGISTERED SIGN */
    0x00A9, /* COPYRIGHT SIGN */
    0x2122, /* TRADE MARK SIGN */
    0x00B4, /* ACUTE ACCENT */
    0x00A8, /* DIAERESIS */
    0x2260, /* NOT EQUAL TO */
    0x00C6, /* LATIN CAPITAL LIGATURE AE */
    0x00D8, /* LATIN CAPITAL LETTER O WITH STROKE */
    0x221E, /* INFINITY */
    0x00B1, /* PLUS-MINUS SIGN */
    0x2264, /* LESS-THAN OR EQUAL TO */
    0x2265, /* GREATER-THAN OR EQUAL TO */
    0x00A5, /* YEN SIGN */
    0x00B5, /* MICRO SIGN */
    0x2202, /* PARTIAL DIFFERENTIAL */
    0x2211, /* N-ARY SUMMATION */
    0x220F, /* N-ARY PRODUCT */
    0x03C0, /* GREEK SMALL LETTER PI */
    0x222B, /* INTEGRAL */
    0x00AA, /* FEMININE ORDINAL INDICATOR */
    0x00BA, /* MASCULINE ORDINAL INDICATOR */
    0x03A9, /* OHM SIGN (Canonical mapping) */
    0x00E6, /* LATIN SMALL LIGATURE AE */
    0x00F8, /* LATIN SMALL LETTER O WITH STROKE */
    0x00BF, /* INVERTED QUESTION MARK */
    0x00A1, /* INVERTED EXCLAMATION MARK */
    0x00AC, /* NOT SIGN */
    0x221A, /* SQUARE ROOT */
    0x0192, /* LATIN SMALL LETTER F WITH HOOK */
    0x2248, /* ALMOST EQUAL TO */
    0x2206, /* INCREMENT */
    0x00AB, /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    0x00BB, /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    0x2026, /* HORIZONTAL ELLIPSIS */
    0x00A0, /* NO-BREAK SPACE */
    0x00C0, /* LATIN CAPITAL LETTER A WITH GRAVE */
    0x00C3, /* LATIN CAPITAL LETTER A WITH TILDE */
    0x00D5, /* LATIN CAPITAL LETTER O WITH TILDE */
    0x0152, /* LATIN CAPITAL LIGATURE OE */
    0x0153, /* LATIN SMALL LIGATURE OE */
    0x2013, /* EN DASH */
    0x2014, /* EM DASH */
    0x201C, /* LEFT DOUBLE QUOTATION MARK */
    0x201D, /* RIGHT DOUBLE QUOTATION MARK */
    0x2018, /* LEFT SINGLE QUOTATION MARK */
    0x2019, /* RIGHT SINGLE QUOTATION MARK */
    0x00F7, /* DIVISION SIGN */
    0x25CA, /* LOZENGE */
    0x00FF, /* LATIN SMALL LETTER Y WITH DIAERESIS */
    0x0178, /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
    0x2044, /* FRACTION SLASH */
    0x20AC, /* EURO SIGN */
    0x2039, /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
    0x203A, /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
    0xFB01, /* LATIN SMALL LIGATURE FI */
    0xFB02, /* LATIN SMALL LIGATURE FL */
    0x2021, /* DOUBLE DAGGER */
    0x00B7, /* MIDDLE DOT */
    0x201A, /* SINGLE LOW-9 QUOTATION MARK */
    0x201E, /* DOUBLE LOW-9 QUOTATION MARK */
    0x2030, /* PER MILLE SIGN */
    0x00C2, /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    0x00CA, /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    0x00C1, /* LATIN CAPITAL LETTER A WITH ACUTE */
    0x00CB, /* LATIN CAPITAL LETTER E WITH DIAERESIS */
    0x00C8, /* LATIN CAPITAL LETTER E WITH GRAVE */
    0x00CD, /* LATIN CAPITAL LETTER I WITH ACUTE */
    0x00CE, /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
    0x00CF, /* LATIN CAPITAL LETTER I WITH DIAERESIS */
    0x00CC, /* LATIN CAPITAL LETTER I WITH GRAVE */
    0x00D3, /* LATIN CAPITAL LETTER O WITH ACUTE */
    0x00D4, /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    0xF8FF, /* Apple logo */
    0x00D2, /* LATIN CAPITAL LETTER O WITH GRAVE */
    0x00DA, /* LATIN CAPITAL LETTER U WITH ACUTE */
    0x00DB, /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
    0x00D9, /* LATIN CAPITAL LETTER U WITH GRAVE */
    0x0131, /* LATIN SMALL LETTER DOTLESS I */
    0x02C6, /* MODIFIER LETTER CIRCUMFLEX ACCENT */
    0x02DC, /* SMALL TILDE */
    0x00AF, /* MACRON */
    0x02D8, /* BREVE */
    0x02D9, /* DOT ABOVE */
    0x02DA, /* RING ABOVE */
    0x00B8, /* CEDILLA */
    0x02DD, /* DOUBLE ACUTE ACCENT */
    0x02DB, /* OGONEK */
    0x02C7, /* CARON */
};

static bool __CFFromMacRoman(uint32_t flags, uint8_t byte, UniChar *character) {
    *character = (byte < 0x80 ? (UniChar)byte : macRoman_to_uni[byte - 0x80]);
    return true;
}

static CFIndex __CFToMacRomanPrecompose(uint32_t flags, const UniChar *character, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    uint8_t byte;
    CFIndex usedCharLen;

    if (__CFToMacRoman(flags, CFStringEncodingPrecomposeLatinCharacter(character, numChars, &usedCharLen), &byte) && byte && (usedCharLen > 1)) {
        if (maxByteLen) *bytes = byte;
        *usedByteLen = 1;
        return usedCharLen;
    } else {
        return 0;
    }
}

CF_PRIVATE const CFStringEncodingConverter __CFConverterMacRoman = {
    __CFToMacRoman, __CFFromMacRoman, 1, 1, kCFStringEncodingConverterCheapEightBit,
    NULL, NULL, NULL, NULL, __CFToMacRomanPrecompose, CFStringEncodingIsValidCombiningCharacterForLatin1,
};

/* Win Latin1 (ANSI CodePage 1252) */
#define NUM_1252_FROM_UNI 27
static const CFStringEncodingUnicodeTo8BitCharMap cp1252_from_uni[NUM_1252_FROM_UNI] = {
    {0x0152, 0x8C}, // LATIN CAPITAL LIGATURE OE
    {0x0153, 0x9C}, // LATIN SMALL LIGATURE OE
    {0x0160, 0x8A}, // LATIN CAPITAL LETTER S WITH CARON
    {0x0161, 0x9A}, // LATIN SMALL LETTER S WITH CARON
    {0x0178, 0x9F}, // LATIN CAPITAL LETTER Y WITH DIAERESIS
    {0x017D, 0x8E}, // LATIN CAPITAL LETTER Z WITH CARON
    {0x017E, 0x9E}, // LATIN SMALL LETTER Z WITH CARON
    {0x0192, 0x83}, // LATIN SMALL LETTER F WITH HOOK
    {0x02C6, 0x88}, // MODIFIER LETTER CIRCUMFLEX ACCENT
    {0x02DC, 0x98}, // SMALL TILDE
    {0x2013, 0x96}, // EN DASH
    {0x2014, 0x97}, // EM DASH
    {0x2018, 0x91}, // LEFT SINGLE QUOTATION MARK
    {0x2019, 0x92}, // RIGHT SINGLE QUOTATION MARK
    {0x201A, 0x82}, // SINGLE LOW-9 QUOTATION MARK
    {0x201C, 0x93}, // LEFT DOUBLE QUOTATION MARK
    {0x201D, 0x94}, // RIGHT DOUBLE QUOTATION MARK
    {0x201E, 0x84}, // DOUBLE LOW-9 QUOTATION MARK
    {0x2020, 0x86}, // DAGGER
    {0x2021, 0x87}, // DOUBLE DAGGER
    {0x2022, 0x95}, // BULLET
    {0x2026, 0x85}, // HORIZONTAL ELLIPSIS
    {0x2030, 0x89}, // PER MILLE SIGN
    {0x2039, 0x8B}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    {0x203A, 0x9B}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
    {0x20AC, 0x80}, // EURO SIGN
    {0x2122, 0x99}, // TRADE MARK SIGN
};

static bool __CFToWinLatin1(uint32_t flags, UniChar character, uint8_t *byte) {
    if ((character < 0x80) || ((character > 0x9F) && (character <= 0x00FF))) {
        *byte = (uint8_t)character;
        return true;
    }
    return CFStringEncodingUnicodeTo8BitEncoding(cp1252_from_uni, NUM_1252_FROM_UNI, character, byte);
}

static const uint16_t cp1252_to_uni[32] = {
    0x20AC, //  EURO SIGN
    0xFFFD, //  NOT USED
    0x201A, //  SINGLE LOW-9 QUOTATION MARK
    0x0192, //  LATIN SMALL LETTER F WITH HOOK
    0x201E, //  DOUBLE LOW-9 QUOTATION MARK
    0x2026, //  HORIZONTAL ELLIPSIS
    0x2020, //  DAGGER
    0x2021, //  DOUBLE DAGGER
    0x02C6, //  MODIFIER LETTER CIRCUMFLEX ACCENT
    0x2030, //  PER MILLE SIGN
    0x0160, //  LATIN CAPITAL LETTER S WITH CARON
    0x2039, //  SINGLE LEFT-POINTING ANGLE QUOTATION MARK
    0x0152, //  LATIN CAPITAL LIGATURE OE
    0xFFFD, //  NOT USED
    0x017D, //  LATIN CAPITAL LETTER Z WITH CARON
    0xFFFD, //  NOT USED
    0xFFFD, //  NOT USED
    0x2018, //  LEFT SINGLE QUOTATION MARK
    0x2019, //  RIGHT SINGLE QUOTATION MARK
    0x201C, //  LEFT DOUBLE QUOTATION MARK
    0x201D, //  RIGHT DOUBLE QUOTATION MARK
    0x2022, //  BULLET
    0x2013, //  EN DASH
    0x2014, //  EM DASH
    0x02DC, //  SMALL TILDE
    0x2122, //  TRADE MARK SIGN
    0x0161, //  LATIN SMALL LETTER S WITH CARON
    0x203A, //  SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
    0x0153, //  LATIN SMALL LIGATURE OE
    0xFFFD, //  NOT USED
    0x017E, //  LATIN SMALL LETTER Z WITH CARON
    0x0178, //  LATIN CAPITAL LETTER Y WITH DIAERESIS
};

static bool __CFFromWinLatin1(uint32_t flags, uint8_t byte, UniChar *character) {
    *character = (byte < 0x80 || byte > 0x9F ? (UniChar)byte : cp1252_to_uni[byte - 0x80]);
    return (*character != 0xFFFD);
}

static CFIndex __CFToWinLatin1Precompose(uint32_t flags, const UniChar *character, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    uint8_t byte;
    CFIndex usedCharLen;

    if (__CFToWinLatin1(flags, CFStringEncodingPrecomposeLatinCharacter(character, numChars, &usedCharLen), &byte) && byte && (usedCharLen > 1)) {
        if (maxByteLen) *bytes = byte;
        *usedByteLen = 1;
        return usedCharLen;
    } else {
        return 0;
    }
}

CF_PRIVATE const CFStringEncodingConverter __CFConverterWinLatin1 = {
    __CFToWinLatin1, __CFFromWinLatin1, 1, 1, kCFStringEncodingConverterCheapEightBit,
    NULL, NULL, NULL, NULL, __CFToWinLatin1Precompose, CFStringEncodingIsValidCombiningCharacterForLatin1,
};

/* NEXTSTEP Encoding */
#define NUM_NEXTSTEP_FROM_UNI	127

static const CFStringEncodingUnicodeTo8BitCharMap nextstep_from_tab[NUM_NEXTSTEP_FROM_UNI] = {
        { 0x00a0, 0x80 },
        { 0x00a1, 0xa1 },
        { 0x00a2, 0xa2 },
        { 0x00a3, 0xa3 },
        { 0x00a4, 0xa8 },
        { 0x00a5, 0xa5 },
        { 0x00a6, 0xb5 },
        { 0x00a7, 0xa7 },
        { 0x00a8, 0xc8 },
        { 0x00a9, 0xa0 },
        { 0x00aa, 0xe3 },
        { 0x00ab, 0xab },
        { 0x00ac, 0xbe },
/*	{ 0x00ad, 0x2d }, <= 96/10/25 rick removed; converts soft-hyphen to hyphen! */
        { 0x00ae, 0xb0 },
        { 0x00af, 0xc5 },
        { 0x00b1, 0xd1 },
        { 0x00b2, 0xc9 },
        { 0x00b3, 0xcc },
        { 0x00b4, 0xc2 },
        { 0x00b5, 0x9d },
        { 0x00b6, 0xb6 },
        { 0x00b7, 0xb4 },
        { 0x00b8, 0xcb },
        { 0x00b9, 0xc0 },
        { 0x00ba, 0xeb },
        { 0x00bb, 0xbb },
        { 0x00bc, 0xd2 },
        { 0x00bd, 0xd3 },
        { 0x00be, 0xd4 },
        { 0x00bf, 0xbf },
        { 0x00c0, 0x81 },
        { 0x00c1, 0x82 },
        { 0x00c2, 0x83 },
        { 0x00c3, 0x84 },
        { 0x00c4, 0x85 },
        { 0x00c5, 0x86 },
        { 0x00c6, 0xe1 },
        { 0x00c7, 0x87 },
        { 0x00c8, 0x88 },
        { 0x00c9, 0x89 },
        { 0x00ca, 0x8a },
        { 0x00cb, 0x8b },
        { 0x00cc, 0x8c },
        { 0x00cd, 0x8d },
        { 0x00ce, 0x8e },
        { 0x00cf, 0x8f },
        { 0x00d0, 0x90 },
        { 0x00d1, 0x91 },
        { 0x00d2, 0x92 },
        { 0x00d3, 0x93 },
        { 0x00d4, 0x94 },
        { 0x00d5, 0x95 },
        { 0x00d6, 0x96 },
        { 0x00d7, 0x9e },
        { 0x00d8, 0xe9 },
        { 0x00d9, 0x97 },
        { 0x00da, 0x98 },
        { 0x00db, 0x99 },
        { 0x00dc, 0x9a },
        { 0x00dd, 0x9b },
        { 0x00de, 0x9c },
        { 0x00df, 0xfb },
        { 0x00e0, 0xd5 },
        { 0x00e1, 0xd6 },
        { 0x00e2, 0xd7 },
        { 0x00e3, 0xd8 },
        { 0x00e4, 0xd9 },
        { 0x00e5, 0xda },
        { 0x00e6, 0xf1 },
        { 0x00e7, 0xdb },
        { 0x00e8, 0xdc },
        { 0x00e9, 0xdd },
        { 0x00ea, 0xde },
        { 0x00eb, 0xdf },
        { 0x00ec, 0xe0 },
        { 0x00ed, 0xe2 },
        { 0x00ee, 0xe4 },
        { 0x00ef, 0xe5 },
        { 0x00f0, 0xe6 },
        { 0x00f1, 0xe7 },
        { 0x00f2, 0xec },
        { 0x00f3, 0xed },
        { 0x00f4, 0xee },
        { 0x00f5, 0xef },
        { 0x00f6, 0xf0 },
        { 0x00f7, 0x9f },
        { 0x00f8, 0xf9 },
        { 0x00f9, 0xf2 },
        { 0x00fa, 0xf3 },
        { 0x00fb, 0xf4 },
        { 0x00fc, 0xf6 },
        { 0x00fd, 0xf7 },
        { 0x00fe, 0xfc },
        { 0x00ff, 0xfd },
        { 0x0131, 0xf5 },
        { 0x0141, 0xe8 },
        { 0x0142, 0xf8 },
        { 0x0152, 0xea },
        { 0x0153, 0xfa },
        { 0x0192, 0xa6 },
        { 0x02c6, 0xc3 },
        { 0x02c7, 0xcf },
        { 0x02cb, 0xc1 },
        { 0x02d8, 0xc6 },
        { 0x02d9, 0xc7 },
        { 0x02da, 0xca },
        { 0x02db, 0xce },
        { 0x02dc, 0xc4 },
        { 0x02dd, 0xcd },
        { 0x2013, 0xb1 },
        { 0x2014, 0xd0 },
        { 0x2019, 0xa9 },
        { 0x201a, 0xb8 },
        { 0x201c, 0xaa },
        { 0x201d, 0xba },
        { 0x201e, 0xb9 },
        { 0x2020, 0xb2 },
        { 0x2021, 0xb3 },
        { 0x2022, 0xb7 },
        { 0x2026, 0xbc },
        { 0x2030, 0xbd },
        { 0x2039, 0xac },
        { 0x203a, 0xad },
        { 0x2044, 0xa4 },
        { 0xfb01, 0xae },
        { 0xfb02, 0xaf },
        { 0xfffd, 0xff },
};

static bool __CFToNextStepLatin(uint32_t flags, UniChar character, uint8_t *byte) {
    if (character < 0x80) {
        *byte = (uint8_t)character;
        return true;
    } else if (__CFIsParagraphSeparator(character)) {
        *byte = ASCIINewLine;
        return true;
    } else {
        return CFStringEncodingUnicodeTo8BitEncoding(nextstep_from_tab, NUM_NEXTSTEP_FROM_UNI, character, byte);
    }
};

static const UniChar NSToPrecompUnicodeTable[128] = {
        /* NextStep Encoding	Unicode */
        /*  128	figspace */	0x00a0,		/* 0x2007 is fig space */
        /*  129	Agrave */	0x00c0,
        /*  130	Aacute */	0x00c1,
        /*  131	Acircumflex */	0x00c2,
        /*  132	Atilde */	0x00c3,
        /*  133	Adieresis */	0x00c4,
        /*  134	Aring */	0x00c5,
        /*  135	Ccedilla */	0x00c7,
        /*  136	Egrave */	0x00c8,
        /*  137	Eacute */	0x00c9,
        /*  138	Ecircumflex */	0x00ca,
        /*  139	Edieresis */	0x00cb,
        /*  140	Igrave */	0x00cc,
        /*  141	Iacute */	0x00cd,
        /*  142	Icircumflex */	0x00ce,
        /*  143	Idieresis */	0x00cf,
        /*  144	Eth */		0x00d0,
        /*  145	Ntilde */	0x00d1,
        /*  146	Ograve */	0x00d2,
        /*  147	Oacute */	0x00d3,
        /*  148	Ocircumflex */	0x00d4,
        /*  149	Otilde */	0x00d5,
        /*  150	Odieresis */	0x00d6,
        /*  151	Ugrave */	0x00d9,
        /*  152	Uacute */	0x00da,
        /*  153	Ucircumflex */	0x00db,
        /*  154	Udieresis */	0x00dc,
        /*  155	Yacute */	0x00dd,
        /*  156	Thorn */	0x00de,
        /*  157	mu */		0x00b5,
        /*  158	multiply */	0x00d7,
        /*  159	divide */	0x00f7,
        /*  160	copyright */	0x00a9,
        /*  161	exclamdown */	0x00a1,
        /*  162	cent */		0x00a2,
        /*  163	sterling */	0x00a3,
        /*  164	fraction */	0x2044,
        /*  165	yen */		0x00a5,
        /*  166	florin */	0x0192,
        /*  167	section */	0x00a7,
        /*  168	currency */	0x00a4,
        /*  169	quotesingle */	0x2019,
        /*  170	quotedblleft */	0x201c,
        /*  171	guillemotleft */ 0x00ab,
        /*  172	guilsinglleft */ 0x2039,
        /*  173	guilsinglright */ 0x203a,
        /*  174	fi */		0xFB01,
        /*  175	fl */		0xFB02,
        /*  176	registered */	0x00ae,
        /*  177	endash */	0x2013,
        /*  178	dagger */	0x2020,
        /*  179	daggerdbl */	0x2021,
        /*  180	periodcentered */ 0x00b7,
        /*  181	brokenbar */	0x00a6,
        /*  182	paragraph */	0x00b6,
        /*  183	bullet */	0x2022,
        /*  184	quotesinglbase */ 0x201a,
        /*  185	quotedblbase */	0x201e,
        /*  186	quotedblright */ 0x201d,
        /*  187	guillemotright */ 0x00bb,
        /*  188	ellipsis */	0x2026,
        /*  189	perthousand */	0x2030,
        /*  190	logicalnot */	0x00ac,
        /*  191	questiondown */	0x00bf,
        /*  192	onesuperior */	0x00b9,
        /*  193	grave */	0x02cb,
        /*  194	acute */	0x00b4,
        /*  195	circumflex */	0x02c6,
        /*  196	tilde */	0x02dc,
        /*  197	macron */	0x00af,
        /*  198	breve */	0x02d8,
        /*  199	dotaccent */	0x02d9,
        /*  200	dieresis */	0x00a8,
        /*  201	twosuperior */	0x00b2,
        /*  202	ring */		0x02da,
        /*  203	cedilla */	0x00b8,
        /*  204	threesuperior */ 0x00b3,
        /*  205	hungarumlaut */	0x02dd,
        /*  206	ogonek */	0x02db,
        /*  207	caron */	0x02c7,
        /*  208	emdash */	0x2014,
        /*  209	plusminus */	0x00b1,
        /*  210	onequarter */	0x00bc,
        /*  211	onehalf */	0x00bd,
        /*  212	threequarters */ 0x00be,
        /*  213	agrave */	0x00e0,
        /*  214	aacute */	0x00e1,
        /*  215	acircumflex */	0x00e2,
        /*  216	atilde */	0x00e3,
        /*  217	adieresis */	0x00e4,
        /*  218	aring */	0x00e5,
        /*  219	ccedilla */	0x00e7,
        /*  220	egrave */	0x00e8,
        /*  221	eacute */	0x00e9,
        /*  222	ecircumflex */	0x00ea,
        /*  223	edieresis */	0x00eb,
        /*  224	igrave */	0x00ec,
        /*  225	AE */		0x00c6,
        /*  226	iacute */	0x00ed,
        /*  227	ordfeminine */	0x00aa,
        /*  228	icircumflex */	0x00ee,
        /*  229	idieresis */	0x00ef,
        /*  230	eth */		0x00f0,
        /*  231	ntilde */	0x00f1,
        /*  232	Lslash */	0x0141,
        /*  233	Oslash */	0x00d8,
        /*  234	OE */		0x0152,
        /*  235	ordmasculine */	0x00ba,
        /*  236	ograve */	0x00f2,
        /*  237	oacute */	0x00f3,
        /*  238	ocircumflex */	0x00f4,
        /*  239	otilde */	0x00f5,
        /*  240	odieresis */	0x00f6,
        /*  241	ae */		0x00e6,
        /*  242	ugrave */	0x00f9,
        /*  243	uacute */	0x00fa,
        /*  244	ucircumflex */	0x00fb,
        /*  245	dotlessi */	0x0131,
        /*  246	udieresis */	0x00fc,
        /*  247	yacute */	0x00fd,
        /*  248	lslash */	0x0142,
        /*  249	oslash */	0x00f8,
        /*  250	oe */		0x0153,
        /*  251	germandbls */	0x00df,
        /*  252	thorn */	0x00fe,
        /*  253	ydieresis */	0x00ff,
        /*  254	.notdef */	0xFFFD,
        /*  255	.notdef */	0xFFFD
};

static bool __CFFromNextStepLatin(uint32_t flags, uint8_t byte, UniChar *character) {
    return ((*character = (byte < 0x80 ? (UniChar)byte : NSToPrecompUnicodeTable[byte - 0x80])) != 0xFFFD);
}

static CFIndex __CFToNextStepLatinPrecompose(uint32_t flags, const UniChar *character, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    uint8_t byte;
    CFIndex usedCharLen;

    if (__CFToNextStepLatin(flags, CFStringEncodingPrecomposeLatinCharacter(character, numChars, &usedCharLen), &byte) && byte && (usedCharLen > 1)) {
        if (maxByteLen) *bytes = byte;
        *usedByteLen = 1;
        return usedCharLen;
    } else {
        return 0;
    }
}

CF_PRIVATE const CFStringEncodingConverter __CFConverterNextStepLatin = {
    __CFToNextStepLatin, __CFFromNextStepLatin, 1, 1, kCFStringEncodingConverterCheapEightBit,
    NULL, NULL, NULL, NULL, __CFToNextStepLatinPrecompose, CFStringEncodingIsValidCombiningCharacterForLatin1,
};

/* UTF8 */
/*
 * Copyright 2001 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

static const uint32_t kReplacementCharacter =   0x0000FFFDUL;
static const uint32_t kMaximumUCS2 =		0x0000FFFFUL;
static const uint32_t kMaximumUTF16 =		0x0010FFFFUL;
static const uint32_t kMaximumUCS4 =		0x7FFFFFFFUL;

static const int halfShift			= 10;
static const uint32_t halfBase		= 0x0010000UL;
static const uint32_t halfMask		= 0x3FFUL;
static const uint32_t kSurrogateHighStart	= 0xD800UL;
static const uint32_t kSurrogateHighEnd	= 0xDBFFUL;
static const uint32_t kSurrogateLowStart	= 0xDC00UL;
static const uint32_t kSurrogateLowEnd	= 0xDFFFUL;

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 */
static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32Char offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
					 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

static const uint8_t firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* This code is similar in effect to making successive calls on the mbtowc and wctomb routines in FSS-UTF. However, it is considerably different in code:
        * it is adapted to be consistent with UTF16,
        * constants have been gathered.
        * loops & conditionals have been removed as much as possible for
        * efficiency, in favor of drop-through switch statements.
*/

CF_INLINE uint16_t __CFUTF8BytesToWriteForCharacter(uint32_t ch) {
    if (ch < 0x80) return  1;
    else if (ch < 0x800) return 2;
    else if (ch < 0x10000) return 3;
    else if (ch < 0x200000) return 4;
    else if (ch < 0x4000000) return 5;
    else if (ch <= kMaximumUCS4) return 6;
    else return 0;
}

CF_INLINE uint16_t __CFToUTF8Core(uint32_t ch, uint8_t *bytes, uint32_t maxByteLen) {
    uint16_t bytesToWrite = __CFUTF8BytesToWriteForCharacter(ch);
    const uint32_t byteMask = 0xBF;
    const uint32_t byteMark = 0x80;

    if (!bytesToWrite) {
        bytesToWrite = 2;
        ch = kReplacementCharacter;
    }

    if (maxByteLen < bytesToWrite) return 0;

    switch (bytesToWrite) {	/* note: code falls through cases! */
        case 6: bytes[5] = (ch | byteMark) & byteMask; ch >>= 6;
        case 5: bytes[4] = (ch | byteMark) & byteMask; ch >>= 6;
        case 4: bytes[3] = (ch | byteMark) & byteMask; ch >>= 6;
        case 3: bytes[2] = (ch | byteMark) & byteMask; ch >>= 6;
        case 2: bytes[1] = (ch | byteMark) & byteMask; ch >>= 6;
        case 1: bytes[0] =  ch | firstByteMark[bytesToWrite];
    }
    return bytesToWrite;
}

static CFIndex __CFToUTF8(uint32_t flags, const UniChar *characters, CFIndex numChars, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    uint16_t bytesWritten;
    uint32_t ch;
    const UniChar *beginCharacter = characters;
    const UniChar *endCharacter = characters + numChars;
    const uint8_t *beginBytes = bytes;
    const uint8_t *endBytes = bytes + maxByteLen;
    bool isStrict = (flags & kCFStringEncodingUseHFSPlusCanonical ? false : true);

    while ((characters < endCharacter) && (!maxByteLen || (bytes < endBytes))) {
        ch = *(characters++);

        if (ch < 0x80) { // ASCII
            if (maxByteLen) *bytes = ch;
            ++bytes;
        } else {
            if (ch >= kSurrogateHighStart) {
                if (ch <= kSurrogateHighEnd) {
                    if ((characters < endCharacter) && ((*characters >= kSurrogateLowStart) && (*characters <= kSurrogateLowEnd))) {
                        ch = ((ch - kSurrogateHighStart) << halfShift) + (*(characters++) - kSurrogateLowStart) + halfBase;
                    } else if (isStrict) {
                        --characters;
                        break;
                    }
                } else if (isStrict && (ch <= kSurrogateLowEnd)) {
                    --characters;
                    break;
                }
            }
    
            if (!(bytesWritten = (maxByteLen ? __CFToUTF8Core(ch, bytes, endBytes - bytes) : __CFUTF8BytesToWriteForCharacter(ch)))) {
                characters -= (ch < 0x10000 ? 1 : 2);
                break;
            }
            bytes += bytesWritten;
        }
    }

    if (usedByteLen) *usedByteLen = bytes - beginBytes;
    return characters - beginCharacter;
}

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *	length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

CF_INLINE bool __CFIsLegalUTF8(const uint8_t *source, CFIndex length) {
    if (length > 4) return false;

    const uint8_t *srcptr = source+length;
    uint8_t head = *source;

    while (--srcptr > source) if ((*srcptr & 0xC0) != 0x80) return false;

    if (((head >= 0x80) && (head < 0xC2)) || (head > 0xF4)) return false;

    if (((head == 0xE0) && (*(source + 1) < 0xA0)) || ((head == 0xED) && (*(source + 1) > 0x9F)) || ((head == 0xF0) && (*(source + 1) < 0x90)) || ((head == 0xF4) && (*(source + 1) > 0x8F))) return false;
    return true;
}

static CFIndex __CFFromUTF8(uint32_t flags, const uint8_t *bytes, CFIndex numBytes, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    const uint8_t *source = bytes;
    uint16_t extraBytesToRead;
    CFIndex theUsedCharLen = 0;
    uint32_t ch;
    bool isHFSPlus = (flags & kCFStringEncodingUseHFSPlusCanonical ? true : false);
    bool needsToDecompose = (flags & kCFStringEncodingUseCanonical || isHFSPlus ? true : false);
    bool strictUTF8 = (flags & kCFStringEncodingLenientUTF8Conversion ? false : true);
    UTF32Char decomposed[MAX_DECOMPOSED_LENGTH];
    CFIndex decompLength;
    bool isStrict = !isHFSPlus;

    while (numBytes && (!maxCharLen || (theUsedCharLen < maxCharLen))) {
        extraBytesToRead = trailingBytesForUTF8[*source];

        if (extraBytesToRead > --numBytes) break;
        numBytes -= extraBytesToRead;

        /* Do this check whether lenient or strict */
        // We need to allow 0xA9 (copyright in MacRoman and Unicode) not to break existing apps
        // Will use a flag passed in from upper layers to switch restriction mode for this case in the next release
        if ((extraBytesToRead > 3) || (strictUTF8 && !__CFIsLegalUTF8(source, extraBytesToRead + 1))) {
            if ((*source == 0xA9) || (flags & kCFStringEncodingAllowLossyConversion)) {
                numBytes += extraBytesToRead;
                ++source;
                if (maxCharLen) *(characters++) = (UTF16Char)kReplacementCharacter;
                ++theUsedCharLen;
                continue;
            } else {
                break;
            }
        }

        ch = 0;
        /*
         * The cases all fall through. See "Note A" below.
         */
        switch (extraBytesToRead) {
            case 3:	ch += *source++; ch <<= 6;
            case 2:	ch += *source++; ch <<= 6;
            case 1:	ch += *source++; ch <<= 6;
            case 0:	ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        if (ch <= kMaximumUCS2) {
            if (isStrict && (ch >= kSurrogateHighStart && ch <= kSurrogateLowEnd)) {
                source -= (extraBytesToRead + 1);
                break;
            }
            if (needsToDecompose && CFUniCharIsDecomposableCharacter(ch, isHFSPlus)) {
                decompLength = CFUniCharDecomposeCharacter(ch, decomposed, MAX_DECOMPOSED_LENGTH);

                if (maxCharLen) {
                    if (!CFUniCharFillDestinationBuffer(decomposed, decompLength, (void **)&characters, maxCharLen, &theUsedCharLen, kCFUniCharUTF16Format)) break;
                } else {
                    theUsedCharLen += decompLength;
                }
            } else {
                if (maxCharLen) *(characters++) = (UTF16Char)ch;
                ++theUsedCharLen;
            }
        } else if (ch > kMaximumUTF16) {
            if (isStrict) {
                source -= (extraBytesToRead + 1);
                break;
            }
            if (maxCharLen) *(characters++) = (UTF16Char)kReplacementCharacter;
            ++theUsedCharLen;
        } else {
            if (needsToDecompose && CFUniCharIsDecomposableCharacter(ch, isHFSPlus)) {
                decompLength = CFUniCharDecomposeCharacter(ch, decomposed, MAX_DECOMPOSED_LENGTH);

                if (maxCharLen) {
                    if (!CFUniCharFillDestinationBuffer(decomposed, decompLength, (void **)&characters, maxCharLen, &theUsedCharLen, kCFUniCharUTF16Format)) break;
                } else {
                    while (--decompLength >= 0) theUsedCharLen += (decomposed[decompLength] < 0x10000 ? 1 : 2);
                }
            } else {
                if (maxCharLen) {
                    if ((theUsedCharLen + 2) > maxCharLen) break;
                    ch -= halfBase;
                    *(characters++) = (ch >> halfShift) + kSurrogateHighStart;
                    *(characters++) = (ch & halfMask) + kSurrogateLowStart;
                }
                theUsedCharLen += 2;
            }
        }
    }

    if (usedCharLen) *usedCharLen = theUsedCharLen;

    return source - bytes;
}

static CFIndex __CFToUTF8Len(uint32_t flags, const UniChar *characters, CFIndex numChars) {
    uint32_t bytesToWrite = 0;
    uint32_t ch;

    while (numChars) {
        ch = *characters++;
        numChars--;
        if ((ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd) && numChars && (*characters >= kSurrogateLowStart && *characters <= kSurrogateLowEnd)) {
            ch = ((ch - kSurrogateHighStart) << halfShift) + (*characters++ - kSurrogateLowStart) + halfBase;
            numChars--;
        }
        bytesToWrite += __CFUTF8BytesToWriteForCharacter(ch);
    }

    return bytesToWrite;
}

static CFIndex __CFFromUTF8Len(uint32_t flags, const uint8_t *source, CFIndex numBytes) {
    uint16_t extraBytesToRead;
    CFIndex theUsedCharLen = 0;
    uint32_t ch;
    bool isHFSPlus = (flags & kCFStringEncodingUseHFSPlusCanonical ? true : false);
    bool needsToDecompose = (flags & kCFStringEncodingUseCanonical || isHFSPlus ? true : false);
    bool strictUTF8 = (flags & kCFStringEncodingLenientUTF8Conversion ? false : true);
    UTF32Char decomposed[MAX_DECOMPOSED_LENGTH];
    CFIndex decompLength;
    bool isStrict = !isHFSPlus;

    while (numBytes) {
        extraBytesToRead = trailingBytesForUTF8[*source];

        if (extraBytesToRead > --numBytes) break;
        numBytes -= extraBytesToRead;

        /* Do this check whether lenient or strict */
        // We need to allow 0xA9 (copyright in MacRoman and Unicode) not to break existing apps
        // Will use a flag passed in from upper layers to switch restriction mode for this case in the next release
        if ((extraBytesToRead > 3) || (strictUTF8 && !__CFIsLegalUTF8(source, extraBytesToRead + 1))) {
            if ((*source == 0xA9) || (flags & kCFStringEncodingAllowLossyConversion)) {
                numBytes += extraBytesToRead;
                ++source;
                ++theUsedCharLen;
                continue;
            } else {
                break;
            }
        }


        ch = 0;
        /*
         * The cases all fall through. See "Note A" below.
         */
        switch (extraBytesToRead) {
            case 3:	ch += *source++; ch <<= 6;
            case 2:	ch += *source++; ch <<= 6;
            case 1:	ch += *source++; ch <<= 6;
            case 0:	ch += *source++;
        }
        ch -= offsetsFromUTF8[extraBytesToRead];

        if (ch <= kMaximumUCS2) {
            if (isStrict && (ch >= kSurrogateHighStart && ch <= kSurrogateLowEnd)) {
                break;
            }
            if (needsToDecompose && CFUniCharIsDecomposableCharacter(ch, isHFSPlus)) {
                decompLength = CFUniCharDecomposeCharacter(ch, decomposed, MAX_DECOMPOSED_LENGTH);
                theUsedCharLen += decompLength;
            } else {
                ++theUsedCharLen;
            }
        } else if (ch > kMaximumUTF16) {
            ++theUsedCharLen;
        } else {
            if (needsToDecompose && CFUniCharIsDecomposableCharacter(ch, isHFSPlus)) {
                decompLength = CFUniCharDecomposeCharacter(ch, decomposed, MAX_DECOMPOSED_LENGTH);
                while (--decompLength >= 0) theUsedCharLen += (decomposed[decompLength] < 0x10000 ? 1 : 2);
            } else {
                theUsedCharLen += 2;
            }
        }
    }

    return theUsedCharLen;
}

CF_PRIVATE const CFStringEncodingConverter __CFConverterUTF8 = {
    __CFToUTF8, __CFFromUTF8, 3, 2, kCFStringEncodingConverterStandard,
    __CFToUTF8Len, __CFFromUTF8Len, NULL, NULL, NULL, NULL,
};
