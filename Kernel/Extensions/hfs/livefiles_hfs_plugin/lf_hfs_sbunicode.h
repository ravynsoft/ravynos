/* Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_sbunicode.h
 *  livefiles_hfs
 *
 *  Created by Oded Shoshani on 31/1/18.
 */

#ifndef lf_hfs_sbunicode_h
#define lf_hfs_sbunicode_h

/*
 Includes Unicode 3.2 decomposition code derived from Core Foundation
 */

/*
 * UTF-8 (Unicode Transformation Format)
 *
 * UTF-8 is the Unicode Transformation Format that serializes a Unicode
 * character as a sequence of one to four bytes. Only the shortest form
 * required to represent the significant Unicode bits is legal.
 *
 * UTF-8 Multibyte Codes
 *
 * Bytes   Bits   Unicode Min  Unicode Max   UTF-8 Byte Sequence (binary)
 * -----------------------------------------------------------------------------
 *   1       7       0x0000        0x007F    0xxxxxxx
 *   2      11       0x0080        0x07FF    110xxxxx 10xxxxxx
 *   3      16       0x0800        0xFFFF    1110xxxx 10xxxxxx 10xxxxxx
 *   4      21      0x10000      0x10FFFF    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 * -----------------------------------------------------------------------------
 */

/*
 * UTF-8 encode/decode flags
 */
#define UTF_REVERSE_ENDIAN      0x0001   /* reverse UCS-2 byte order */
#define UTF_ADD_NULL_TERM       0x0002   /* add null termination */
#define UTF_DECOMPOSED          0x0004   /* generate fully decomposed UCS-2 */
#define UTF_PRECOMPOSED         0x0008   /* generate precomposed UCS-2 */
#define UTF_ESCAPE_ILLEGAL      0x0010   /* escape illegal UTF-8 */
#define UTF_SFM_CONVERSIONS     0x0020   /* Use SFM mappings for illegal NTFS chars */

#define UTF_BIG_ENDIAN       \
((BYTE_ORDER == BIG_ENDIAN) ? 0 : UTF_REVERSE_ENDIAN)
#define UTF_LITTLE_ENDIAN    \
((BYTE_ORDER == LITTLE_ENDIAN) ? 0 : UTF_REVERSE_ENDIAN)



/*
 * utf8_encodelen - Calculate the UTF-8 encoding length
 *
 * This function takes a Unicode input string, ucsp, of ucslen bytes
 * and calculates the size of the UTF-8 output in bytes (not including
 * a NULL termination byte). The string must reside in kernel memory.
 *
 * If '/' chars are possible in the Unicode input then an alternate
 * (replacement) char should be provided in altslash.
 *
 * FLAGS
 *    UTF_REVERSE_ENDIAN:  Unicode byte order is opposite current runtime
 *
 *    UTF_BIG_ENDIAN:  Unicode byte order is always big endian
 *
 *    UTF_LITTLE_ENDIAN:  Unicode byte order is always little endian
 *
 *    UTF_DECOMPOSED:  generate fully decomposed output
 *
 *    UTF_PRECOMPOSED is ignored since utf8_encodestr doesn't support it
 *
 * ERRORS
 *    None
 */
size_t utf8_encodelen(const u_int16_t * ucsp, size_t ucslen, u_int16_t altslash, int flags);

/*
 * utf8_encodestr - Encodes a Unicode string to UTF-8
 *
 * NOTES:
 *    The resulting UTF-8 string is NULL terminated.
 *
 *    If '/' chars are allowed on disk then an alternate
 *    (replacement) char must be provided in altslash.
 *
 * input flags:
 *    UTF_REVERSE_ENDIAN: Unicode byteorder is opposite current runtime
 *
 *    UTF_BIG_ENDIAN:  Unicode byte order is always big endian
 *
 *    UTF_LITTLE_ENDIAN:  Unicode byte order is always little endian
 *
 *    UTF_DECOMPOSED:  generate fully decomposed output
 *
 *    UTF_ADD_NULL_TERM:  add NULL termination to UTF-8 output
 *
 * result:
 *    ENAMETOOLONG: Name didn't fit; only buflen bytes were encoded
 *
 *    EINVAL: Illegal char found; char was replaced by an '_'.
 */
extern int utf8_encodestr(const u_int16_t * ucsp, size_t ucslen, u_int8_t * utf8p, size_t * utf8len, size_t buflen, u_int16_t altslash, int flags);

/*
 * utf8_decodestr - Decodes a UTF-8 string back to Unicode
 *
 * NOTES:
 *    The input UTF-8 string does not need to be null terminated
 *    if utf8len is set.
 *
 *    If '/' chars are allowed on disk then an alternate
 *    (replacement) char must be provided in altslash.
 *
 * input flags:
 *    UTF_REV_ENDIAN:  Unicode byte order is opposite current runtime
 *
 *    UTF_BIG_ENDIAN:  Unicode byte order is always big endian
 *
 *    UTF_LITTLE_ENDIAN:  Unicode byte order is always little endian
 *
 *    UTF_DECOMPOSED:  generate fully decomposed output (NFD)
 *
 *    UTF_PRECOMPOSED:  generate precomposed output (NFC)
 *
 *    UTF_ESCAPE_ILLEGAL:  percent escape any illegal UTF-8 input
 *
 * result:
 *    ENAMETOOLONG: Name didn't fit; only ucslen chars were decoded.
 *
 *    EINVAL: Illegal UTF-8 sequence found.
 */
int utf8_decodestr(const u_int8_t* utf8p, size_t utf8len, u_int16_t* ucsp, size_t *ucslen, size_t buflen, u_int16_t altslash, int flags);

#endif /* lf_hfs_sbunicode_h */

