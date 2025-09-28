/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* libcroco - Library for parsing and applying CSS
 * Copyright (C) 2006-2019 Free Software Foundation, Inc.
 *
 * This file is not part of the GNU gettext program, but is used with
 * GNU gettext.
 *
 * The original copyright notice is as follows:
 */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2003-2004 Dodji Seketeli.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * Author: Dodji Seketeli
 */

#include <config.h>
#include "cr-utils.h"
#include "cr-string.h"

/**
 *@file:
 *Some misc utility functions used
 *in the libcroco.
 *Note that troughout this file I will
 *refer to the CSS SPECIFICATIONS DOCUMENTATION
 *written by the w3c guys. You can find that document
 *at http://www.w3.org/TR/REC-CSS2/ .
 */

/****************************
 *Encoding transformations and
 *encoding helpers
 ****************************/

/*
 *Here is the correspondance between the ucs-4 charactere codes
 *and there matching utf-8 encoding pattern as dscribed by RFC 2279:
 *
 *UCS-4 range (hex.)    UTF-8 octet sequence (binary)
 *------------------    -----------------------------
 *0000 0000-0000 007F   0xxxxxxx
 *0000 0080-0000 07FF   110xxxxx 10xxxxxx
 *0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
 *0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
 *0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx
 */

/**
 *Given an utf8 string buffer, calculates
 *the length of this string if it was encoded
 *in ucs4.
 *@param a_in_start a pointer to the begining of
 *the input utf8 string.
 *@param a_in_end a pointre to the end of the input
 *utf8 string (points to the last byte of the buffer)
 *@param a_len out parameter the calculated length.
 *@return CR_OK upon succesfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_utils_utf8_str_len_as_ucs4 (const guchar * a_in_start,
                               const guchar * a_in_end, gulong * a_len)
{
        guchar *byte_ptr = NULL;
        gint len = 0;

        /*
         *to store the final decoded 
         *unicode char
         */
        guint c = 0;

        g_return_val_if_fail (a_in_start && a_in_end && a_len,
                              CR_BAD_PARAM_ERROR);
        *a_len = 0;

        for (byte_ptr = (guchar *) a_in_start;
             byte_ptr <= a_in_end; byte_ptr++) {
                gint nb_bytes_2_decode = 0;

                if (*byte_ptr <= 0x7F) {
                        /*
                         *7 bits long char
                         *encoded over 1 byte:
                         * 0xxx xxxx
                         */
                        c = *byte_ptr;
                        nb_bytes_2_decode = 1;

                } else if ((*byte_ptr & 0xE0) == 0xC0) {
                        /*
                         *up to 11 bits long char.
                         *encoded over 2 bytes:
                         *110x xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 0x1F;
                        nb_bytes_2_decode = 2;

                } else if ((*byte_ptr & 0xF0) == 0xE0) {
                        /*
                         *up to 16 bit long char
                         *encoded over 3 bytes:
                         *1110 xxxx  10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 0x0F;
                        nb_bytes_2_decode = 3;

                } else if ((*byte_ptr & 0xF8) == 0xF0) {
                        /*
                         *up to 21 bits long char
                         *encoded over 4 bytes:
                         *1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 0x7;
                        nb_bytes_2_decode = 4;

                } else if ((*byte_ptr & 0xFC) == 0xF8) {
                        /*
                         *up to 26 bits long char
                         *encoded over 5 bytes.
                         *1111 10xx  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 3;
                        nb_bytes_2_decode = 5;

                } else if ((*byte_ptr & 0xFE) == 0xFC) {
                        /*
                         *up to 31 bits long char
                         *encoded over 6 bytes:
                         *1111 110x  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 1;
                        nb_bytes_2_decode = 6;

                } else {
                        /*
                         *BAD ENCODING
                         */
                        return CR_ENCODING_ERROR;
                }

                /*
                 *Go and decode the remaining byte(s)
                 *(if any) to get the current character.
                 */
                for (; nb_bytes_2_decode > 1; nb_bytes_2_decode--) {
                        /*decode the next byte */
                        byte_ptr++;

                        /*byte pattern must be: 10xx xxxx */
                        if ((*byte_ptr & 0xC0) != 0x80) {
                                return CR_ENCODING_ERROR;
                        }

                        c = (c << 6) | (*byte_ptr & 0x3F);
                }

                len++;
        }

        *a_len = len;

        return CR_OK;
}

/**
 *Given an ucs4 string, this function
 *returns the size (in bytes) this string
 *would have occupied if it was encoded in utf-8.
 *@param a_in_start a pointer to the beginning of the input
 *buffer.
 *@param a_in_end a pointer to the end of the input buffer.
 *@param a_len out parameter. The computed length.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_ucs4_str_len_as_utf8 (const guint32 * a_in_start,
                               const guint32 * a_in_end, gulong * a_len)
{
        gint len = 0;
        guint32 *char_ptr = NULL;

        g_return_val_if_fail (a_in_start && a_in_end && a_len,
                              CR_BAD_PARAM_ERROR);

        for (char_ptr = (guint32 *) a_in_start;
             char_ptr <= a_in_end; char_ptr++) {
                if (*char_ptr <= 0x7F) {
                        /*the utf-8 char would take 1 byte */
                        len += 1;
                } else if (*char_ptr <= 0x7FF) {
                        /*the utf-8 char would take 2 bytes */
                        len += 2;
                } else if (*char_ptr <= 0xFFFF) {
                        len += 3;
                } else if (*char_ptr <= 0x1FFFFF) {
                        len += 4;
                } else if (*char_ptr <= 0x3FFFFFF) {
                        len += 5;
                } else if (*char_ptr <= 0x7FFFFFFF) {
                        len += 6;
                }
        }

        *a_len = len;
        return CR_OK;
}

/**
 *Given an ucsA string, this function
 *returns the size (in bytes) this string
 *would have occupied if it was encoded in utf-8.
 *@param a_in_start a pointer to the beginning of the input
 *buffer.
 *@param a_in_end a pointer to the end of the input buffer.
 *@param a_len out parameter. The computed length.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_ucs1_str_len_as_utf8 (const guchar * a_in_start,
                               const guchar * a_in_end, gulong * a_len)
{
        gint len = 0;
        guchar *char_ptr = NULL;

        g_return_val_if_fail (a_in_start && a_in_end && a_len,
                              CR_BAD_PARAM_ERROR);

        for (char_ptr = (guchar *) a_in_start;
             char_ptr <= a_in_end; char_ptr++) {
                if (*char_ptr <= 0x7F) {
                        /*the utf-8 char would take 1 byte */
                        len += 1;
                } else {
                        /*the utf-8 char would take 2 bytes */
                        len += 2;
                }
        }

        *a_len = len;
        return CR_OK;
}

/**
 *Converts an utf8 buffer into an ucs4 buffer.
 *
 *@param a_in the input utf8 buffer to convert.
 *@param a_in_len in/out parameter. The size of the
 *input buffer to convert. After return, this parameter contains
 *the actual number of bytes consumed.
 *@param a_out the output converted ucs4 buffer. Must be allocated by
 *the caller.
 *@param a_out_len in/out parameter. The size of the output buffer.
 *If this size is actually smaller than the real needed size, the function
 *just converts what it can and returns a success status. After return,
 *this param points to the actual number of characters decoded.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_utf8_to_ucs4 (const guchar * a_in,
                       gulong * a_in_len, guint32 * a_out, gulong * a_out_len)
{
        gulong in_len = 0,
                out_len = 0,
                in_index = 0,
                out_index = 0;
        enum CRStatus status = CR_OK;

        /*
         *to store the final decoded 
         *unicode char
         */
        guint c = 0;

        g_return_val_if_fail (a_in && a_in_len
                              && a_out && a_out_len, CR_BAD_PARAM_ERROR);

        if (*a_in_len < 1) {
                status = CR_OK;
                goto end;
        }

        in_len = *a_in_len;
        out_len = *a_out_len;

        for (in_index = 0, out_index = 0;
             (in_index < in_len) && (out_index < out_len);
             in_index++, out_index++) {
                gint nb_bytes_2_decode = 0;

                if (a_in[in_index] <= 0x7F) {
                        /*
                         *7 bits long char
                         *encoded over 1 byte:
                         * 0xxx xxxx
                         */
                        c = a_in[in_index];
                        nb_bytes_2_decode = 1;

                } else if ((a_in[in_index] & 0xE0) == 0xC0) {
                        /*
                         *up to 11 bits long char.
                         *encoded over 2 bytes:
                         *110x xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 0x1F;
                        nb_bytes_2_decode = 2;

                } else if ((a_in[in_index] & 0xF0) == 0xE0) {
                        /*
                         *up to 16 bit long char
                         *encoded over 3 bytes:
                         *1110 xxxx  10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 0x0F;
                        nb_bytes_2_decode = 3;

                } else if ((a_in[in_index] & 0xF8) == 0xF0) {
                        /*
                         *up to 21 bits long char
                         *encoded over 4 bytes:
                         *1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 0x7;
                        nb_bytes_2_decode = 4;

                } else if ((a_in[in_index] & 0xFC) == 0xF8) {
                        /*
                         *up to 26 bits long char
                         *encoded over 5 bytes.
                         *1111 10xx  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 3;
                        nb_bytes_2_decode = 5;

                } else if ((a_in[in_index] & 0xFE) == 0xFC) {
                        /*
                         *up to 31 bits long char
                         *encoded over 6 bytes:
                         *1111 110x  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 1;
                        nb_bytes_2_decode = 6;

                } else {
                        /*BAD ENCODING */
                        goto end;
                }

                /*
                 *Go and decode the remaining byte(s)
                 *(if any) to get the current character.
                 */
                for (; nb_bytes_2_decode > 1; nb_bytes_2_decode--) {
                        /*decode the next byte */
                        in_index++;

                        /*byte pattern must be: 10xx xxxx */
                        if ((a_in[in_index] & 0xC0) != 0x80) {
                                goto end;
                        }

                        c = (c << 6) | (a_in[in_index] & 0x3F);
                }

                /*
                 *The decoded ucs4 char is now
                 *in c.
                 */

                /************************
                 *Some security tests
                 ***********************/

                /*be sure c is a char */
                if (c == 0xFFFF || c == 0xFFFE)
                        goto end;

                /*be sure c is inferior to the max ucs4 char value */
                if (c > 0x10FFFF)
                        goto end;

                /*
                 *c must be less than UTF16 "lower surrogate begin"
                 *or higher than UTF16 "High surrogate end"
                 */
                if (c >= 0xD800 && c <= 0xDFFF)
                        goto end;

                /*Avoid characters that equals zero */
                if (c == 0)
                        goto end;

                a_out[out_index] = c;
        }

      end:
        *a_out_len = out_index + 1;
        *a_in_len = in_index + 1;

        return status;
}

/**
 *Reads a character from an utf8 buffer.
 *Actually decode the next character code (unicode character code)
 *and returns it.
 *@param a_in the starting address of the utf8 buffer.
 *@param a_in_len the length of the utf8 buffer.
 *@param a_out output parameter. The resulting read char.
 *@param a_consumed the number of the bytes consumed to
 *decode the returned character code.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_read_char_from_utf8_buf (const guchar * a_in,
                                  gulong a_in_len,
                                  guint32 * a_out, gulong * a_consumed)
{
        gulong in_index = 0,
               nb_bytes_2_decode = 0;
        enum CRStatus status = CR_OK;

        /*
         *to store the final decoded 
         *unicode char
         */
        guint32 c = 0;

        g_return_val_if_fail (a_in && a_out && a_out
                              && a_consumed, CR_BAD_PARAM_ERROR);

        if (a_in_len < 1) {
                status = CR_OK;
                goto end;
        }

        if (*a_in <= 0x7F) {
                /*
                 *7 bits long char
                 *encoded over 1 byte:
                 * 0xxx xxxx
                 */
                c = *a_in;
                nb_bytes_2_decode = 1;

        } else if ((*a_in & 0xE0) == 0xC0) {
                /*
                 *up to 11 bits long char.
                 *encoded over 2 bytes:
                 *110x xxxx  10xx xxxx
                 */
                c = *a_in & 0x1F;
                nb_bytes_2_decode = 2;

        } else if ((*a_in & 0xF0) == 0xE0) {
                /*
                 *up to 16 bit long char
                 *encoded over 3 bytes:
                 *1110 xxxx  10xx xxxx  10xx xxxx
                 */
                c = *a_in & 0x0F;
                nb_bytes_2_decode = 3;

        } else if ((*a_in & 0xF8) == 0xF0) {
                /*
                 *up to 21 bits long char
                 *encoded over 4 bytes:
                 *1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                 */
                c = *a_in & 0x7;
                nb_bytes_2_decode = 4;

        } else if ((*a_in & 0xFC) == 0xF8) {
                /*
                 *up to 26 bits long char
                 *encoded over 5 bytes.
                 *1111 10xx  10xx xxxx  10xx xxxx  
                 *10xx xxxx  10xx xxxx
                 */
                c = *a_in & 3;
                nb_bytes_2_decode = 5;

        } else if ((*a_in & 0xFE) == 0xFC) {
                /*
                 *up to 31 bits long char
                 *encoded over 6 bytes:
                 *1111 110x  10xx xxxx  10xx xxxx  
                 *10xx xxxx  10xx xxxx  10xx xxxx
                 */
                c = *a_in & 1;
                nb_bytes_2_decode = 6;

        } else {
                /*BAD ENCODING */
                goto end;
        }

        if (nb_bytes_2_decode > a_in_len) {
                status = CR_END_OF_INPUT_ERROR;
                goto end;
        }

        /*
         *Go and decode the remaining byte(s)
         *(if any) to get the current character.
         */
        for (in_index = 1; in_index < nb_bytes_2_decode; in_index++) {
                /*byte pattern must be: 10xx xxxx */
                if ((a_in[in_index] & 0xC0) != 0x80) {
                        goto end;
                }

                c = (c << 6) | (a_in[in_index] & 0x3F);
        }

        /*
         *The decoded ucs4 char is now
         *in c.
         */

    /************************
     *Some security tests
     ***********************/

        /*be sure c is a char */
        if (c == 0xFFFF || c == 0xFFFE)
                goto end;

        /*be sure c is inferior to the max ucs4 char value */
        if (c > 0x10FFFF)
                goto end;

        /*
         *c must be less than UTF16 "lower surrogate begin"
         *or higher than UTF16 "High surrogate end"
         */
        if (c >= 0xD800 && c <= 0xDFFF)
                goto end;

        /*Avoid characters that equals zero */
        if (c == 0)
                goto end;

        *a_out = c;

      end:
        *a_consumed = nb_bytes_2_decode;

        return status;
}

/**
 *
 */
enum CRStatus
cr_utils_utf8_str_len_as_ucs1 (const guchar * a_in_start,
                               const guchar * a_in_end, gulong * a_len)
{
        /*
         *Note: this function can be made shorter
         *but it considers all the cases of the utf8 encoding
         *to ease further extensions ...
         */

        guchar *byte_ptr = NULL;
        gint len = 0;

        /*
         *to store the final decoded 
         *unicode char
         */
        guint c = 0;

        g_return_val_if_fail (a_in_start && a_in_end && a_len,
                              CR_BAD_PARAM_ERROR);
        *a_len = 0;

        for (byte_ptr = (guchar *) a_in_start;
             byte_ptr <= a_in_end; byte_ptr++) {
                gint nb_bytes_2_decode = 0;

                if (*byte_ptr <= 0x7F) {
                        /*
                         *7 bits long char
                         *encoded over 1 byte:
                         * 0xxx xxxx
                         */
                        c = *byte_ptr;
                        nb_bytes_2_decode = 1;

                } else if ((*byte_ptr & 0xE0) == 0xC0) {
                        /*
                         *up to 11 bits long char.
                         *encoded over 2 bytes:
                         *110x xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 0x1F;
                        nb_bytes_2_decode = 2;

                } else if ((*byte_ptr & 0xF0) == 0xE0) {
                        /*
                         *up to 16 bit long char
                         *encoded over 3 bytes:
                         *1110 xxxx  10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 0x0F;
                        nb_bytes_2_decode = 3;

                } else if ((*byte_ptr & 0xF8) == 0xF0) {
                        /*
                         *up to 21 bits long char
                         *encoded over 4 bytes:
                         *1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 0x7;
                        nb_bytes_2_decode = 4;

                } else if ((*byte_ptr & 0xFC) == 0xF8) {
                        /*
                         *up to 26 bits long char
                         *encoded over 5 bytes.
                         *1111 10xx  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 3;
                        nb_bytes_2_decode = 5;

                } else if ((*byte_ptr & 0xFE) == 0xFC) {
                        /*
                         *up to 31 bits long char
                         *encoded over 6 bytes:
                         *1111 110x  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = *byte_ptr & 1;
                        nb_bytes_2_decode = 6;

                } else {
                        /*
                         *BAD ENCODING
                         */
                        return CR_ENCODING_ERROR;
                }

                /*
                 *Go and decode the remaining byte(s)
                 *(if any) to get the current character.
                 */
                for (; nb_bytes_2_decode > 1; nb_bytes_2_decode--) {
                        /*decode the next byte */
                        byte_ptr++;

                        /*byte pattern must be: 10xx xxxx */
                        if ((*byte_ptr & 0xC0) != 0x80) {
                                return CR_ENCODING_ERROR;
                        }

                        c = (c << 6) | (*byte_ptr & 0x3F);
                }

                /*
                 *The decoded ucs4 char is now
                 *in c.
                 */

                if (c <= 0xFF) { /*Add other conditions to support
                                  *other char sets (ucs2, ucs3, ucs4).
                                  */
                        len++;
                } else {
                        /*the char is too long to fit
                         *into the supposed charset len.
                         */
                        return CR_ENCODING_ERROR;
                }
        }

        *a_len = len;

        return CR_OK;
}

/**
 *Converts an utf8 string into an ucs4 string.
 *@param a_in the input string to convert.
 *@param a_in_len in/out parameter. The length of the input
 *string. After return, points to the actual number of bytes
 *consumed. This can be usefull to debug the input stream in case
 *of encoding error.
 *@param a_out out parameter. Points to the output string. It is allocated 
 *by this function and must be freed by the caller.
 *@param a_out_len out parameter. The length of the output string.
 *@return CR_OK upon successfull completion, an error code otherwise.
 *
 */
enum CRStatus
cr_utils_utf8_str_to_ucs4 (const guchar * a_in,
                           gulong * a_in_len,
                           guint32 ** a_out, gulong * a_out_len)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_in && a_in_len
                              && a_out && a_out_len, CR_BAD_PARAM_ERROR);

        status = cr_utils_utf8_str_len_as_ucs4 (a_in,
                                                &a_in[*a_in_len - 1],
                                                a_out_len);

        g_return_val_if_fail (status == CR_OK, status);

        *a_out = g_malloc0 (*a_out_len * sizeof (guint32));

        status = cr_utils_utf8_to_ucs4 (a_in, a_in_len, *a_out, a_out_len);

        return status;
}

/**
 *Converts an ucs4 buffer into an utf8 buffer.
 *
 *@param a_in the input ucs4 buffer to convert.
 *@param a_in_len in/out parameter. The size of the
 *input buffer to convert. After return, this parameter contains
 *the actual number of characters consumed.
 *@param a_out the output converted utf8 buffer. Must be allocated by
 *the caller.
 *@param a_out_len in/out parameter. The size of the output buffer.
 *If this size is actually smaller than the real needed size, the function
 *just converts what it can and returns a success status. After return,
 *this param points to the actual number of bytes in the buffer.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_ucs4_to_utf8 (const guint32 * a_in,
                       gulong * a_in_len, guchar * a_out, gulong * a_out_len)
{
        gulong in_len = 0,
                in_index = 0,
                out_index = 0;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_in && a_in_len && a_out && a_out_len,
                              CR_BAD_PARAM_ERROR);

        if (*a_in_len < 1) {
                status = CR_OK;
                goto end;
        }

        in_len = *a_in_len;

        for (in_index = 0; in_index < in_len; in_index++) {
                /*
                 *FIXME: return whenever we encounter forbidden char values.
                 */

                if (a_in[in_index] <= 0x7F) {
                        a_out[out_index] = a_in[in_index];
                        out_index++;
                } else if (a_in[in_index] <= 0x7FF) {
                        a_out[out_index] = (0xC0 | (a_in[in_index] >> 6));
                        a_out[out_index + 1] =
                                (0x80 | (a_in[in_index] & 0x3F));
                        out_index += 2;
                } else if (a_in[in_index] <= 0xFFFF) {
                        a_out[out_index] = (0xE0 | (a_in[in_index] >> 12));
                        a_out[out_index + 1] =
                                (0x80 | ((a_in[in_index] >> 6) & 0x3F));
                        a_out[out_index + 2] =
                                (0x80 | (a_in[in_index] & 0x3F));
                        out_index += 3;
                } else if (a_in[in_index] <= 0x1FFFFF) {
                        a_out[out_index] = (0xF0 | (a_in[in_index] >> 18));
                        a_out[out_index + 1]
                                = (0x80 | ((a_in[in_index] >> 12) & 0x3F));
                        a_out[out_index + 2]
                                = (0x80 | ((a_in[in_index] >> 6) & 0x3F));
                        a_out[out_index + 3]
                                = (0x80 | (a_in[in_index] & 0x3F));
                        out_index += 4;
                } else if (a_in[in_index] <= 0x3FFFFFF) {
                        a_out[out_index] = (0xF8 | (a_in[in_index] >> 24));
                        a_out[out_index + 1] =
                                (0x80 | (a_in[in_index] >> 18));
                        a_out[out_index + 2]
                                = (0x80 | ((a_in[in_index] >> 12) & 0x3F));
                        a_out[out_index + 3]
                                = (0x80 | ((a_in[in_index] >> 6) & 0x3F));
                        a_out[out_index + 4]
                                = (0x80 | (a_in[in_index] & 0x3F));
                        out_index += 5;
                } else if (a_in[in_index] <= 0x7FFFFFFF) {
                        a_out[out_index] = (0xFC | (a_in[in_index] >> 30));
                        a_out[out_index + 1] =
                                (0x80 | (a_in[in_index] >> 24));
                        a_out[out_index + 2]
                                = (0x80 | ((a_in[in_index] >> 18) & 0x3F));
                        a_out[out_index + 3]
                                = (0x80 | ((a_in[in_index] >> 12) & 0x3F));
                        a_out[out_index + 4]
                                = (0x80 | ((a_in[in_index] >> 6) & 0x3F));
                        a_out[out_index + 4]
                                = (0x80 | (a_in[in_index] & 0x3F));
                        out_index += 6;
                } else {
                        status = CR_ENCODING_ERROR;
                        goto end;
                }
        }                       /*end for */

      end:
        *a_in_len = in_index + 1;
        *a_out_len = out_index + 1;

        return status;
}

/**
 *Converts an ucs4 string into an utf8 string.
 *@param a_in the input string to convert.
 *@param a_in_len in/out parameter. The length of the input
 *string. After return, points to the actual number of characters
 *consumed. This can be usefull to debug the input string in case
 *of encoding error.
 *@param a_out out parameter. Points to the output string. It is allocated 
 *by this function and must be freed by the caller.
 *@param a_out_len out parameter. The length (in bytes) of the output string.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_ucs4_str_to_utf8 (const guint32 * a_in,
                           gulong * a_in_len,
                           guchar ** a_out, gulong * a_out_len)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_in && a_in_len && a_out
                              && a_out_len, CR_BAD_PARAM_ERROR);

        status = cr_utils_ucs4_str_len_as_utf8 (a_in,
                                                &a_in[*a_out_len - 1],
                                                a_out_len);

        g_return_val_if_fail (status == CR_OK, status);

        status = cr_utils_ucs4_to_utf8 (a_in, a_in_len, *a_out, a_out_len);

        return status;
}

/**
 *Converts an ucs1 buffer into an utf8 buffer.
 *The caller must know the size of the resulting buffer and
 *allocate it prior to calling this function.
 *
 *@param a_in the input ucs1 buffer.
 *
 *@param a_in_len in/out parameter. The length of the input buffer.
 *After return, points to the number of bytes actually consumed even
 *in case of encoding error.
 *
 *@param a_out out parameter. The output utf8 converted buffer.
 *
 *@param a_out_len in/out parameter. The size of the output buffer.
 *If the output buffer size is shorter than the actual needed size, 
 *this function just convert what it can.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 *
 */
enum CRStatus
cr_utils_ucs1_to_utf8 (const guchar * a_in,
                       gulong * a_in_len, guchar * a_out, gulong * a_out_len)
{
        gulong out_index = 0,
                in_index = 0,
                in_len = 0,
                out_len = 0;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_in && a_in_len
                              && a_out_len, 
                              CR_BAD_PARAM_ERROR);

        if (*a_in_len == 0) {
                *a_out_len = 0 ;
                return status;
        }
        g_return_val_if_fail (a_out, CR_BAD_PARAM_ERROR) ;

        in_len = *a_in_len;
        out_len = *a_out_len;

        for (in_index = 0, out_index = 0;
             (in_index < in_len) && (out_index < out_len); in_index++) {
                /*
                 *FIXME: return whenever we encounter forbidden char values.
                 */

                if (a_in[in_index] <= 0x7F) {
                        a_out[out_index] = a_in[in_index];
                        out_index++;
                } else {
                        a_out[out_index] = (0xC0 | (a_in[in_index] >> 6));
                        a_out[out_index + 1] =
                                (0x80 | (a_in[in_index] & 0x3F));
                        out_index += 2;
                }
        }                       /*end for */

        *a_in_len = in_index;
        *a_out_len = out_index;

        return status;
}

/**
 *Converts an ucs1 string into an utf8 string.
 *@param a_in_start the beginning of the input string to convert.
 *@param a_in_end the end of the input string to convert.
 *@param a_out out parameter. The converted string.
 *@param a_out out parameter. The length of the converted string.
 *@return CR_OK upon successfull completion, an error code otherwise.
 *
 */
enum CRStatus
cr_utils_ucs1_str_to_utf8 (const guchar * a_in,
                           gulong * a_in_len,
                           guchar ** a_out, gulong * a_out_len)
{
        gulong out_len = 0;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_in && a_in_len && a_out
                              && a_out_len, CR_BAD_PARAM_ERROR);

        if (*a_in_len < 1) {
                *a_out_len = 0;
                *a_out = NULL;
                return CR_OK;
        }

        status = cr_utils_ucs1_str_len_as_utf8 (a_in, &a_in[*a_in_len - 1],
                                                &out_len);

        g_return_val_if_fail (status == CR_OK, status);

        *a_out = g_malloc0 (out_len);

        status = cr_utils_ucs1_to_utf8 (a_in, a_in_len, *a_out, &out_len);

        *a_out_len = out_len;

        return status;
}

/**
 *Converts an utf8 buffer into an ucs1 buffer.
 *The caller must know the size of the resulting
 *converted buffer, and allocated it prior to calling this
 *function.
 *
 *@param a_in the input utf8 buffer to convert.
 *
 *@param a_in_len in/out parameter. The size of the input utf8 buffer.
 *After return, points to the number of bytes consumed
 *by the function even in case of encoding error.
 *
 *@param a_out out parameter. Points to the resulting buffer.
 *Must be allocated by the caller. If the size of a_out is shorter
 *than its required size, this function converts what it can and return
 *a successfull status.
 *
 *@param a_out_len in/out parameter. The size of the output buffer.
 *After return, points to the number of bytes consumed even in case of
 *encoding error.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_utils_utf8_to_ucs1 (const guchar * a_in,
                       gulong * a_in_len, guchar * a_out, gulong * a_out_len)
{
        gulong in_index = 0,
                out_index = 0,
                in_len = 0,
                out_len = 0;
        enum CRStatus status = CR_OK;

        /*
         *to store the final decoded 
         *unicode char
         */
        guint32 c = 0;

        g_return_val_if_fail (a_in && a_in_len
                              && a_out && a_out_len, CR_BAD_PARAM_ERROR);

        if (*a_in_len < 1) {
                goto end;
        }

        in_len = *a_in_len;
        out_len = *a_out_len;

        for (in_index = 0, out_index = 0;
             (in_index < in_len) && (out_index < out_len);
             in_index++, out_index++) {
                gint nb_bytes_2_decode = 0;

                if (a_in[in_index] <= 0x7F) {
                        /*
                         *7 bits long char
                         *encoded over 1 byte:
                         * 0xxx xxxx
                         */
                        c = a_in[in_index];
                        nb_bytes_2_decode = 1;

                } else if ((a_in[in_index] & 0xE0) == 0xC0) {
                        /*
                         *up to 11 bits long char.
                         *encoded over 2 bytes:
                         *110x xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 0x1F;
                        nb_bytes_2_decode = 2;

                } else if ((a_in[in_index] & 0xF0) == 0xE0) {
                        /*
                         *up to 16 bit long char
                         *encoded over 3 bytes:
                         *1110 xxxx  10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 0x0F;
                        nb_bytes_2_decode = 3;

                } else if ((a_in[in_index] & 0xF8) == 0xF0) {
                        /*
                         *up to 21 bits long char
                         *encoded over 4 bytes:
                         *1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 0x7;
                        nb_bytes_2_decode = 4;

                } else if ((a_in[in_index] & 0xFC) == 0xF8) {
                        /*
                         *up to 26 bits long char
                         *encoded over 5 bytes.
                         *1111 10xx  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 3;
                        nb_bytes_2_decode = 5;

                } else if ((a_in[in_index] & 0xFE) == 0xFC) {
                        /*
                         *up to 31 bits long char
                         *encoded over 6 bytes:
                         *1111 110x  10xx xxxx  10xx xxxx  
                         *10xx xxxx  10xx xxxx  10xx xxxx
                         */
                        c = a_in[in_index] & 1;
                        nb_bytes_2_decode = 6;

                } else {
                        /*BAD ENCODING */
                        status = CR_ENCODING_ERROR;
                        goto end;
                }

                /*
                 *Go and decode the remaining byte(s)
                 *(if any) to get the current character.
                 */
                if (in_index + nb_bytes_2_decode - 1 >= in_len) {
                        goto end;
                }

                for (; nb_bytes_2_decode > 1; nb_bytes_2_decode--) {
                        /*decode the next byte */
                        in_index++;

                        /*byte pattern must be: 10xx xxxx */
                        if ((a_in[in_index] & 0xC0) != 0x80) {
                                status = CR_ENCODING_ERROR;
                                goto end;
                        }

                        c = (c << 6) | (a_in[in_index] & 0x3F);
                }

                /*
                 *The decoded ucs4 char is now
                 *in c.
                 */

                if (c > 0xFF) {
                        status = CR_ENCODING_ERROR;
                        goto end;
                }

                a_out[out_index] = c;
        }

      end:
        *a_out_len = out_index;
        *a_in_len = in_index;

        return status;
}

/**
 *Converts an utf8 buffer into an
 *ucs1 buffer.
 *@param a_in_start the start of the input buffer.
 *@param a_in_end the end of the input buffer.
 *@param a_out out parameter. The resulting converted ucs4 buffer.
 *Must be freed by the caller.
 *@param a_out_len out parameter. The length of the converted buffer.
 *@return CR_OK upon successfull completion, an error code otherwise.
 *Note that out parameters are valid if and only if this function
 *returns CR_OK.
 */
enum CRStatus
cr_utils_utf8_str_to_ucs1 (const guchar * a_in,
                           gulong * a_in_len,
                           guchar ** a_out, gulong * a_out_len)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_in && a_in_len
                              && a_out && a_out_len, CR_BAD_PARAM_ERROR);

        if (*a_in_len < 1) {
                *a_out_len = 0;
                *a_out = NULL;
                return CR_OK;
        }

        status = cr_utils_utf8_str_len_as_ucs4 (a_in, &a_in[*a_in_len - 1],
                                                a_out_len);

        g_return_val_if_fail (status == CR_OK, status);

        *a_out = g_malloc0 (*a_out_len * sizeof (guint32));

        status = cr_utils_utf8_to_ucs1 (a_in, a_in_len, *a_out, a_out_len);
        return status;
}

/*****************************************
 *CSS basic types identification utilities
 *****************************************/

/**
 *Returns TRUE if a_char is a white space as
 *defined in the css spec in chap 4.1.1.
 *
 *white-space ::= ' '| \t|\r|\n|\f
 *
 *@param a_char the character to test.
 *return TRUE if is a white space, false otherwise.
 */
gboolean
cr_utils_is_white_space (guint32 a_char)
{
        switch (a_char) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\f':
                return TRUE;
                break;
        default:
                return FALSE;
        }
}

/**
 *Returns true if the character is a newline
 *as defined in the css spec in the chap 4.1.1.
 *
 *nl ::= \n|\r\n|\r|\f
 *
 *@param a_char the character to test.
 *@return TRUE if the character is a newline, FALSE otherwise.
 */
gboolean
cr_utils_is_newline (guint32 a_char)
{
        switch (a_char) {
        case '\n':
        case '\r':
        case '\f':
                return TRUE;
                break;
        default:
                return FALSE;
        }
}

/**
 *returns TRUE if the char is part of an hexa num char:
 *i.e hexa_char ::= [0-9A-F]
 */
gboolean
cr_utils_is_hexa_char (guint32 a_char)
{
        if ((a_char >= '0' && a_char <= '9')
            || (a_char >= 'A' && a_char <= 'F')) {
                return TRUE;
        }
        return FALSE;
}

/**
 *Returns true if the character is a nonascii
 *character (as defined in the css spec chap 4.1.1):
 *
 *nonascii ::= [^\0-\177]
 *
 *@param a_char the character to test.
 *@return TRUE if the character is a nonascii char,
 *FALSE otherwise.
 */
gboolean
cr_utils_is_nonascii (guint32 a_char)
{
        if (a_char <= 177) {
                return FALSE;
        }

        return TRUE;
}

/**
 *Dumps a character a_nb times on a file.
 *@param a_char the char to dump
 *@param a_fp the destination file pointer
 *@param a_nb the number of times a_char is to be dumped.
 */
void
cr_utils_dump_n_chars (guchar a_char, FILE * a_fp, glong a_nb)
{
        glong i = 0;

        for (i = 0; i < a_nb; i++) {
                fprintf (a_fp, "%c", a_char);
        }
}

void
cr_utils_dump_n_chars2 (guchar a_char, GString * a_string, glong a_nb)
{
        glong i = 0;

        g_return_if_fail (a_string);

        for (i = 0; i < a_nb; i++) {
                g_string_append_printf (a_string, "%c", a_char);
        }
}

/**
 *Duplicates a list of GString instances.
 *@return the duplicated list of GString instances or NULL if
 *something bad happened.
 *@param a_list_of_strings the list of strings to be duplicated.
 */
GList *
cr_utils_dup_glist_of_string (GList const * a_list_of_strings)
{
        GList const *cur = NULL;
        GList *result = NULL;

        g_return_val_if_fail (a_list_of_strings, NULL);

        for (cur = a_list_of_strings; cur; cur = cur->next) {
                GString *str = NULL;

                str = g_string_new_len (((GString *) cur->data)->str,
                                        ((GString *) cur->data)->len);
                if (str)
                        result = g_list_append (result, str);
        }

        return result;
}

/**
 *Duplicate a GList where the GList::data is a CRString.
 *@param a_list_of_strings the list to duplicate
 *@return the duplicated list, or NULL if something bad
 *happened.
 */
GList *
cr_utils_dup_glist_of_cr_string (GList const * a_list_of_strings)
{
        GList const *cur = NULL;
        GList *result = NULL;

        g_return_val_if_fail (a_list_of_strings, NULL);

        for (cur = a_list_of_strings; cur; cur = cur->next) {
                CRString *str = NULL;

                str = cr_string_dup ((CRString const *) cur->data) ;
                if (str)
                        result = g_list_append (result, str);
        }

        return result;
}
