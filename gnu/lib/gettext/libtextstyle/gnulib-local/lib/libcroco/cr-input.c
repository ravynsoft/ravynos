/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8-*- */

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
#include "stdio.h"
#include <string.h>
#include "cr-input.h"
#include "cr-enc-handler.h"

/**
 *@CRInput:
 *
 *The definition of the #CRInput class.
 */

/*******************
 *Private type defs
 *******************/

/**
 *The private attributes of
 *the #CRInputPriv class.
 */
struct _CRInputPriv {
        /*
         *The input buffer
         */
        guchar *in_buf;
        gulong in_buf_size;

        gulong nb_bytes;

        /*
         *The index of the next byte
         *to be read.
         */
        gulong next_byte_index;

        /*
         *The current line number
         */
        gulong line;

        /*
         *The current col number
         */
        gulong col;

        gboolean end_of_line;
        gboolean end_of_input;

        /*
         *the reference count of this
         *instance.
         */
        guint ref_count;
        gboolean free_in_buf;
};

#define PRIVATE(object) (object)->priv

/***************************
 *private constants
 **************************/
#define CR_INPUT_MEM_CHUNK_SIZE 1024 * 4

static CRInput *cr_input_new_real (void);

static CRInput *
cr_input_new_real (void)
{
        CRInput *result = NULL;

        result = g_try_malloc (sizeof (CRInput));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRInput));

        PRIVATE (result) = g_try_malloc (sizeof (CRInputPriv));
        if (!PRIVATE (result)) {
                cr_utils_trace_info ("Out of memory");
                g_free (result);
                return NULL;
        }
        memset (PRIVATE (result), 0, sizeof (CRInputPriv));
        PRIVATE (result)->free_in_buf = TRUE;
        return result;
}

/****************
 *Public methods
 ***************/

/**
 * cr_input_new_from_buf:
 *@a_buf: the memory buffer to create the input stream from.
 *The #CRInput keeps this pointer so user should not free it !.
 *@a_len: the size of the input buffer.
 *@a_enc: the buffer's encoding.
 *@a_free_buf: if set to TRUE, this a_buf will be freed
 *at the destruction of this instance. If set to false, it is up
 *to the caller to free it.
 *
 *Creates a new input stream from a memory buffer.
 *Returns the newly built instance of #CRInput.
 */
CRInput *
cr_input_new_from_buf (guchar * a_buf,
                       gulong a_len,
                       enum CREncoding a_enc,
                       gboolean a_free_buf)
{
        CRInput *result = NULL;
        enum CRStatus status = CR_OK;
        CREncHandler *enc_handler = NULL;
        gulong len = a_len;

        g_return_val_if_fail (a_buf, NULL);

        result = cr_input_new_real ();
        g_return_val_if_fail (result, NULL);

        /*transform the encoding in utf8 */
        if (a_enc != CR_UTF_8) {
                enc_handler = cr_enc_handler_get_instance (a_enc);
                if (!enc_handler) {
                        goto error;
                }

                status = cr_enc_handler_convert_input
                        (enc_handler, a_buf, &len,
                         &PRIVATE (result)->in_buf,
                         &PRIVATE (result)->in_buf_size);
                if (status != CR_OK)
                        goto error;
                PRIVATE (result)->free_in_buf = TRUE;
                if (a_free_buf == TRUE && a_buf) {
                        g_free (a_buf) ;
                        a_buf = NULL ;
                }                
                PRIVATE (result)->nb_bytes = PRIVATE (result)->in_buf_size;
        } else {
                PRIVATE (result)->in_buf = (guchar *) a_buf;
                PRIVATE (result)->in_buf_size = a_len;
                PRIVATE (result)->nb_bytes = a_len;
                PRIVATE (result)->free_in_buf = a_free_buf;
        }
        PRIVATE (result)->line = 1;
        PRIVATE (result)->col =  0;
        return result;

 error:
        if (result) {
                cr_input_destroy (result);
                result = NULL;
        }

        return NULL;
}

/**
 * cr_input_new_from_uri:
 *@a_file_uri: the file to create *the input stream from.
 *@a_enc: the encoding of the file *to create the input from.
 *
 *Creates a new input stream from
 *a file.
 *
 *Returns the newly created input stream if
 *this method could read the file and create it,
 *NULL otherwise.
 */

CRInput *
cr_input_new_from_uri (const gchar * a_file_uri, enum CREncoding a_enc)
{
        CRInput *result = NULL;
        enum CRStatus status = CR_OK;
        FILE *file_ptr = NULL;
        guchar tmp_buf[CR_INPUT_MEM_CHUNK_SIZE] = { 0 };
        gulong nb_read = 0,
                len = 0,
                buf_size = 0;
        gboolean loop = TRUE;
        guchar *buf = NULL;

        g_return_val_if_fail (a_file_uri, NULL);

        file_ptr = fopen (a_file_uri, "r");

        if (file_ptr == NULL) {

#ifdef CR_DEBUG
                cr_utils_trace_debug ("could not open file");
#endif
                g_warning ("Could not open file %s\n", a_file_uri);

                return NULL;
        }

        /*load the file */
        while (loop) {
                nb_read = fread (tmp_buf, 1 /*read bytes */ ,
                                 CR_INPUT_MEM_CHUNK_SIZE /*nb of bytes */ ,
                                 file_ptr);

                if (nb_read != CR_INPUT_MEM_CHUNK_SIZE) {
                        /*we read less chars than we wanted */
                        if (feof (file_ptr)) {
                                /*we reached eof */
                                loop = FALSE;
                        } else {
                                /*a pb occurred !! */
                                cr_utils_trace_debug ("an io error occurred");
                                status = CR_ERROR;
                                goto cleanup;
                        }
                }

                if (status == CR_OK) {
                        /*read went well */
                        buf = g_realloc (buf, len + CR_INPUT_MEM_CHUNK_SIZE);
                        memcpy (buf + len, tmp_buf, nb_read);
                        len += nb_read;
                        buf_size += CR_INPUT_MEM_CHUNK_SIZE;
                }
        }

        if (status == CR_OK) {
                result = cr_input_new_from_buf (buf, len, a_enc, TRUE);
                if (!result) {
                        goto cleanup;
                }
                /*
                 *we should  free buf here because it's own by CRInput.
                 *(see the last parameter of cr_input_new_from_buf().
                 */
                buf = NULL;
        }

 cleanup:
        if (file_ptr) {
                fclose (file_ptr);
                file_ptr = NULL;
        }

        if (buf) {
                g_free (buf);
                buf = NULL;
        }

        return result;
}

/**
 * cr_input_destroy:
 *@a_this: the current instance of #CRInput.
 *
 *The destructor of the #CRInput class.
 */
void
cr_input_destroy (CRInput * a_this)
{
        if (a_this == NULL)
                return;

        if (PRIVATE (a_this)) {
                if (PRIVATE (a_this)->in_buf && PRIVATE (a_this)->free_in_buf) {
                        g_free (PRIVATE (a_this)->in_buf);
                        PRIVATE (a_this)->in_buf = NULL;
                }

                g_free (PRIVATE (a_this));
                PRIVATE (a_this) = NULL;
        }

        g_free (a_this);
}

/**
 * cr_input_ref:
 *@a_this: the current instance of #CRInput.
 *
 *Increments the reference count of the current
 *instance of #CRInput.
 */
void
cr_input_ref (CRInput * a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this));

        PRIVATE (a_this)->ref_count++;
}

/**
 * cr_input_unref:
 *@a_this: the current instance of #CRInput.
 *
 *Decrements the reference count of this instance
 *of #CRInput. If the reference count goes down to
 *zero, this instance is destroyed.
 *
 * Returns TRUE if the instance of #CRInput got destroyed, false otherwise.
 */
gboolean
cr_input_unref (CRInput * a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), FALSE);

        if (PRIVATE (a_this)->ref_count) {
                PRIVATE (a_this)->ref_count--;
        }

        if (PRIVATE (a_this)->ref_count == 0) {
                cr_input_destroy (a_this);
                return TRUE;
        }
        return FALSE;
}

/**
 * cr_input_end_of_input:
 *@a_this: the current instance of #CRInput.
 *@a_end_of_input: out parameter. Is set to TRUE if
 *the current instance has reached the end of its input buffer,
 *FALSE otherwise.
 *
 *Tests wether the current instance of
 *#CRInput has reached its input buffer.
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 * Note that all the out parameters of this method are valid if
 * and only if this method returns CR_OK.
 */
enum CRStatus
cr_input_end_of_input (CRInput const * a_this, gboolean * a_end_of_input)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_end_of_input, CR_BAD_PARAM_ERROR);

        *a_end_of_input = (PRIVATE (a_this)->next_byte_index
                           >= PRIVATE (a_this)->in_buf_size) ? TRUE : FALSE;

        return CR_OK;
}

/**
 * cr_input_get_nb_bytes_left:
 *@a_this: the current instance of #CRInput.
 *
 *Returns the number of bytes left in the input stream
 *before the end, -1 in case of error.
 */
glong
cr_input_get_nb_bytes_left (CRInput const * a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), -1);
        g_return_val_if_fail (PRIVATE (a_this)->nb_bytes
                              <= PRIVATE (a_this)->in_buf_size, -1);
        g_return_val_if_fail (PRIVATE (a_this)->next_byte_index
                              <= PRIVATE (a_this)->nb_bytes, -1);

        if (PRIVATE (a_this)->end_of_input)
                return 0;

        return PRIVATE (a_this)->nb_bytes - PRIVATE (a_this)->next_byte_index;
}

/**
 * cr_input_read_byte:
 *@a_this: the current instance of #CRInput.
 *@a_byte: out parameter the returned byte.
 *
 *Gets the next byte of the input.
 *Updates the state of the input so that
 *the next invocation of this method  returns
 *the next coming byte.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise. All the out parameters of this method are valid if
 *and only if this method returns CR_OK.
 */
enum CRStatus
cr_input_read_byte (CRInput * a_this, guchar * a_byte)
{
        gulong nb_bytes_left = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_byte, CR_BAD_PARAM_ERROR);

        g_return_val_if_fail (PRIVATE (a_this)->next_byte_index <=
                              PRIVATE (a_this)->nb_bytes, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->end_of_input == TRUE)
                return CR_END_OF_INPUT_ERROR;

        nb_bytes_left = cr_input_get_nb_bytes_left (a_this);

        if (nb_bytes_left < 1) {
                return CR_END_OF_INPUT_ERROR;
        }

        *a_byte = PRIVATE (a_this)->in_buf[PRIVATE (a_this)->next_byte_index];

        if (PRIVATE (a_this)->nb_bytes -
            PRIVATE (a_this)->next_byte_index < 2) {
                PRIVATE (a_this)->end_of_input = TRUE;
        } else {
                PRIVATE (a_this)->next_byte_index++;
        }

        return CR_OK;
}

/**
 * cr_input_read_char:
 *@a_this: the current instance of CRInput.
 *@a_char: out parameter. The read character.
 *
 *Reads an unicode character from the current instance of
 *#CRInput.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_read_char (CRInput * a_this, guint32 * a_char)
{
        enum CRStatus status = CR_OK;
        gulong consumed = 0,
                nb_bytes_left = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_char,
                              CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->end_of_input == TRUE)
                return CR_END_OF_INPUT_ERROR;

        nb_bytes_left = cr_input_get_nb_bytes_left (a_this);

        if (nb_bytes_left < 1) {
                return CR_END_OF_INPUT_ERROR;
        }

        status = cr_utils_read_char_from_utf8_buf
                (PRIVATE (a_this)->in_buf
                 +
                 PRIVATE (a_this)->next_byte_index,
                 nb_bytes_left, a_char, &consumed);

        if (status == CR_OK) {
                /*update next byte index */
                PRIVATE (a_this)->next_byte_index += consumed;

                /*update line and column number */
                if (PRIVATE (a_this)->end_of_line == TRUE) {
                        PRIVATE (a_this)->col = 1;
                        PRIVATE (a_this)->line++;
                        PRIVATE (a_this)->end_of_line = FALSE;
                } else if (*a_char != '\n') {
                        PRIVATE (a_this)->col++;
                }

                if (*a_char == '\n') {
                        PRIVATE (a_this)->end_of_line = TRUE;
                }
        }

        return status;
}

/**
 * cr_input_set_line_num:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_line_num: the new line number.
 *
 *Setter of the current line number.
 *
 *Return CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_line_num (CRInput * a_this, glong a_line_num)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->line = a_line_num;

        return CR_OK;
}

/**
 * cr_input_get_line_num:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_line_num: the returned line number.
 *
 *Getter of the current line number.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_line_num (CRInput const * a_this, glong * a_line_num)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_line_num, CR_BAD_PARAM_ERROR);

        *a_line_num = PRIVATE (a_this)->line;

        return CR_OK;
}

/**
 * cr_input_set_column_num:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_col: the new column number.
 *
 *Setter of the current column number.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_column_num (CRInput * a_this, glong a_col)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->col = a_col;

        return CR_OK;
}

/**
 * cr_input_get_column_num:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_col: out parameter
 *
 *Getter of the current column number.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_column_num (CRInput const * a_this, glong * a_col)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_col,
                              CR_BAD_PARAM_ERROR);

        *a_col = PRIVATE (a_this)->col;

        return CR_OK;
}

/**
 * cr_input_increment_line_num:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_increment: the increment to add to the line number.
 *
 *Increments the current line number.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_increment_line_num (CRInput * a_this, glong a_increment)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->line += a_increment;

        return CR_OK;
}

/**
 * cr_input_increment_col_num:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_increment: the increment to add to the column number.
 *
 *Increments the current column number.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_increment_col_num (CRInput * a_this, glong a_increment)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->col += a_increment;

        return CR_OK;
}

/**
 * cr_input_consume_char:
 *@a_this: the this pointer.
 *@a_char: the character to consume. If set to zero,
 *consumes any character.
 *
 *Consumes the next character of the input stream if
 *and only if that character equals a_char.
 *
 *Returns CR_OK upon successful completion, CR_PARSING_ERROR if
 *next char is different from a_char, an other error code otherwise
 */
enum CRStatus
cr_input_consume_char (CRInput * a_this, guint32 a_char)
{
        guint32 c;
        enum CRStatus status;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        if ((status = cr_input_peek_char (a_this, &c)) != CR_OK) {
                return status;
        }

        if (c == a_char || a_char == 0) {
                status = cr_input_read_char (a_this, &c);
        } else {
                return CR_PARSING_ERROR;
        }

        return status;
}

/**
 * cr_input_consume_chars:
 *@a_this: the this pointer of the current instance of #CRInput.
 *@a_char: the character to consume.
 *@a_nb_char: in/out parameter. The number of characters to consume.
 *If set to a negative value, the function will consume all the occurences
 *of a_char found.
 *After return, if the return value equals CR_OK, this variable contains 
 *the number of characters actually consumed.
 *
 *Consumes up to a_nb_char occurences of the next contiguous characters 
 *which equal a_char. Note that the next character of the input stream
 **MUST* equal a_char to trigger the consumption, or else, the error
 *code CR_PARSING_ERROR is returned.
 *If the number of contiguous characters that equals a_char is less than
 *a_nb_char, then this function consumes all the characters it can consume.
 * 
 *Returns CR_OK if at least one character has been consumed, an error code
 *otherwise.
 */
enum CRStatus
cr_input_consume_chars (CRInput * a_this, guint32 a_char, gulong * a_nb_char)
{
        enum CRStatus status = CR_OK;
        gulong nb_consumed = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_nb_char,
                              CR_BAD_PARAM_ERROR);

        g_return_val_if_fail (a_char != 0 || a_nb_char != NULL,
                              CR_BAD_PARAM_ERROR);

        for (nb_consumed = 0; ((status == CR_OK)
                               && (*a_nb_char > 0
                                   && nb_consumed < *a_nb_char));
             nb_consumed++) {
                status = cr_input_consume_char (a_this, a_char);
        }

        *a_nb_char = nb_consumed;

        if ((nb_consumed > 0)
            && ((status == CR_PARSING_ERROR)
                || (status == CR_END_OF_INPUT_ERROR))) {
                status = CR_OK;
        }

        return status;
}

/**
 * cr_input_consume_white_spaces:
 *@a_this: the "this pointer" of the current instance of #CRInput.
 *@a_nb_chars: in/out parameter. The number of white spaces to
 *consume. After return, holds the number of white spaces actually consumed.
 *
 *Same as cr_input_consume_chars() but this one consumes white
 *spaces.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_consume_white_spaces (CRInput * a_this, gulong * a_nb_chars)
{
        enum CRStatus status = CR_OK;
        guint32 cur_char = 0,
                nb_consumed = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_nb_chars,
                              CR_BAD_PARAM_ERROR);

        for (nb_consumed = 0;
             ((*a_nb_chars > 0) && (nb_consumed < *a_nb_chars));
             nb_consumed++) {
                status = cr_input_peek_char (a_this, &cur_char);
                if (status != CR_OK)
                        break;

                /*if the next char is a white space, consume it ! */
                if (cr_utils_is_white_space (cur_char) == TRUE) {
                        status = cr_input_read_char (a_this, &cur_char);
                        if (status != CR_OK)
                                break;
                        continue;
                }

                break;

        }

	*a_nb_chars = (gulong) nb_consumed;

        if (nb_consumed && status == CR_END_OF_INPUT_ERROR) {
                status = CR_OK;
        }

        return status;
}

/**
 * cr_input_peek_char:
 *@a_this: the current instance of #CRInput.
 *@a_char: out parameter. The returned character.
 *
 *Same as cr_input_read_char() but does not update the
 *internal state of the input stream. The next call
 *to cr_input_peek_char() or cr_input_read_char() will thus
 *return the same character as the current one.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_peek_char (CRInput const * a_this, guint32 * a_char)
{
        enum CRStatus status = CR_OK;
        gulong consumed = 0,
                nb_bytes_left = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_char, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->next_byte_index >=
            PRIVATE (a_this)->in_buf_size) {
                return CR_END_OF_INPUT_ERROR;
        }

        nb_bytes_left = cr_input_get_nb_bytes_left (a_this);

        if (nb_bytes_left < 1) {
                return CR_END_OF_INPUT_ERROR;
        }

        status = cr_utils_read_char_from_utf8_buf
                (PRIVATE (a_this)->in_buf +
                 PRIVATE (a_this)->next_byte_index,
                 nb_bytes_left, a_char, &consumed);

        return status;
}

/**
 * cr_input_peek_byte:
 *@a_this: the current instance of #CRInput.
 *@a_origin: the origin to consider in the calculation
 *of the position of the byte to peek.
 *@a_offset: the offset of the byte to peek, starting from
 *the origin specified by a_origin.
 *@a_byte: out parameter the peeked byte.
 *
 *Gets a byte from the input stream,
 *starting from the current position in the input stream.
 *Unlike cr_input_peek_next_byte() this method
 *does not update the state of the current input stream.
 *Subsequent calls to cr_input_peek_byte with the same arguments
 *will return the same byte.
 *
 *Returns CR_OK upon successful completion or,
 *CR_BAD_PARAM_ERROR if at least one of the parameters is invalid;
 *CR_OUT_OF_BOUNDS_ERROR if the indexed byte is out of bounds.
 */
enum CRStatus
cr_input_peek_byte (CRInput const * a_this, enum CRSeekPos a_origin,
                    gulong a_offset, guchar * a_byte)
{
        gulong abs_offset = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_byte, CR_BAD_PARAM_ERROR);

        switch (a_origin) {

        case CR_SEEK_CUR:
                abs_offset = PRIVATE (a_this)->next_byte_index - 1 + a_offset;
                break;

        case CR_SEEK_BEGIN:
                abs_offset = a_offset;
                break;

        case CR_SEEK_END:
                abs_offset = PRIVATE (a_this)->in_buf_size - 1 - a_offset;
                break;

        default:
                return CR_BAD_PARAM_ERROR;
        }

        if (abs_offset < PRIVATE (a_this)->in_buf_size) {

                *a_byte = PRIVATE (a_this)->in_buf[abs_offset];

                return CR_OK;

        } else {
                return CR_END_OF_INPUT_ERROR;
        }
}

/**
 * cr_input_peek_byte2:
 *@a_this: the current byte input stream.
 *@a_offset: the offset of the byte to peek, starting
 *from the current input position pointer.
 *@a_eof: out parameter. Is set to true is we reach end of
 *stream. If set to NULL by the caller, this parameter is not taken
 *in account.
 *
 *Same as cr_input_peek_byte() but with a simplified
 *interface.
 *
 *Returns the read byte or 0 if something bad happened.
 */
guchar
cr_input_peek_byte2 (CRInput const * a_this, gulong a_offset, gboolean * a_eof)
{
        guchar result = 0;
        enum CRStatus status = CR_ERROR;

        g_return_val_if_fail (a_this && PRIVATE (a_this), 0);

        if (a_eof)
                *a_eof = FALSE;

        status = cr_input_peek_byte (a_this, CR_SEEK_CUR, a_offset, &result);

        if ((status == CR_END_OF_INPUT_ERROR)
            && a_eof)
                *a_eof = TRUE;

        return result;
}

/**
 * cr_input_get_byte_addr:
 *@a_this: the current instance of #CRInput.
 *@a_offset: the offset of the byte in the input stream starting
 *from the beginning of the stream.
 *
 *Gets the memory address of the byte located at a given offset
 *in the input stream.
 *
 *Returns the address, otherwise NULL if an error occurred.
 */
guchar *
cr_input_get_byte_addr (CRInput * a_this, gulong a_offset)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), NULL);

        if (a_offset >= PRIVATE (a_this)->nb_bytes) {
                return NULL;
        }

        return &PRIVATE (a_this)->in_buf[a_offset];
}

/**
 * cr_input_get_cur_byte_addr:
 *@a_this: the current input stream
 *@a_offset: out parameter. The returned address.
 *
 *Gets the address of the current character pointer.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_cur_byte_addr (CRInput * a_this, guchar ** a_offset)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_offset,
                              CR_BAD_PARAM_ERROR);

        if (!PRIVATE (a_this)->next_byte_index) {
                return CR_START_OF_INPUT_ERROR;
        }

        *a_offset = cr_input_get_byte_addr
                (a_this, PRIVATE (a_this)->next_byte_index - 1);

        return CR_OK;
}

/**
 * cr_input_seek_index:
 *@a_this: the current instance of #CRInput.
 *@a_origin: the origin to consider during the calculation
 *of the absolute position of the new "current byte index".
 *@a_pos: the relative offset of the new "current byte index."
 *This offset is relative to the origin a_origin.
 *
 *Sets the "current byte index" of the current instance
 *of #CRInput. Next call to cr_input_get_byte() will return
 *the byte next after the new "current byte index".
 *
 *Returns CR_OK upon successful completion otherwise returns
 *CR_BAD_PARAM_ERROR if at least one of the parameters is not valid
 *or CR_OUT_BOUNDS_ERROR in case of error.
 */
enum CRStatus
cr_input_seek_index (CRInput * a_this, enum CRSeekPos a_origin, gint a_pos)
{

        glong abs_offset = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        switch (a_origin) {

        case CR_SEEK_CUR:
                abs_offset = PRIVATE (a_this)->next_byte_index - 1 + a_pos;
                break;

        case CR_SEEK_BEGIN:
                abs_offset = a_pos;
                break;

        case CR_SEEK_END:
                abs_offset = PRIVATE (a_this)->in_buf_size - 1 - a_pos;
                break;

        default:
                return CR_BAD_PARAM_ERROR;
        }

        if ((abs_offset > 0)
            && (gulong) abs_offset < PRIVATE (a_this)->nb_bytes) {

                /*update the input stream's internal state */
                PRIVATE (a_this)->next_byte_index = abs_offset + 1;

                return CR_OK;
        }

        return CR_OUT_OF_BOUNDS_ERROR;
}

/**
 * cr_input_get_cur_pos:
 *@a_this: the current instance of #CRInput.
 *@a_pos: out parameter. The returned position.
 *
 *Gets the position of the "current byte index" which
 *is basically the position of the last returned byte in the
 *input stream.
 *
 *Returns CR_OK upon successful completion. Otherwise,
 *CR_BAD_PARAMETER_ERROR if at least one of the arguments is invalid.
 *CR_START_OF_INPUT if no call to either cr_input_read_byte()
 *or cr_input_seek_index() have been issued before calling 
 *cr_input_get_cur_pos()
 *Note that the out parameters of this function are valid if and only if this
 *function returns CR_OK.
 */
enum CRStatus
cr_input_get_cur_pos (CRInput const * a_this, CRInputPos * a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pos,
                              CR_BAD_PARAM_ERROR);

        a_pos->next_byte_index = PRIVATE (a_this)->next_byte_index;
        a_pos->line = PRIVATE (a_this)->line;
        a_pos->col = PRIVATE (a_this)->col;
        a_pos->end_of_line = PRIVATE (a_this)->end_of_line;
        a_pos->end_of_file = PRIVATE (a_this)->end_of_input;

        return CR_OK;
}

/**
 * cr_input_get_parsing_location:
 *@a_this: the current instance of #CRInput
 *@a_loc: the set parsing location.
 *
 *Gets the current parsing location.
 *The Parsing location is a public datastructure that
 *represents the current line/column/byte offset/ in the input
 *stream.
 *
 *Returns CR_OK upon successful completion, an error
 *code otherwise.
 */
enum CRStatus
cr_input_get_parsing_location (CRInput const *a_this,
                               CRParsingLocation *a_loc)
{
        g_return_val_if_fail (a_this 
                              && PRIVATE (a_this)
                              && a_loc, 
                              CR_BAD_PARAM_ERROR) ;

        a_loc->line = PRIVATE (a_this)->line ;
        a_loc->column = PRIVATE (a_this)->col ;
        if (PRIVATE (a_this)->next_byte_index) {
                a_loc->byte_offset = PRIVATE (a_this)->next_byte_index - 1 ;
        } else {
                a_loc->byte_offset = PRIVATE (a_this)->next_byte_index  ;
        }
        return CR_OK ;
}

/**
 * cr_input_get_cur_index:
 *@a_this: the "this pointer" of the current instance of
 *#CRInput
 *@a_index: out parameter. The returned index.
 *
 *Getter of the next byte index. 
 *It actually returns the index of the
 *next byte to be read.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_get_cur_index (CRInput const * a_this, glong * a_index)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_index, CR_BAD_PARAM_ERROR);

        *a_index = PRIVATE (a_this)->next_byte_index;

        return CR_OK;
}

/**
 * cr_input_set_cur_index:
 *@a_this: the "this pointer" of the current instance
 *of #CRInput .
 *@a_index: the new index to set.
 *
 *Setter of the next byte index.
 *It sets the index of the next byte to be read.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_cur_index (CRInput * a_this, glong a_index)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->next_byte_index = a_index;

        return CR_OK;
}

/**
 * cr_input_set_end_of_file:
 *@a_this: the current instance of #CRInput.
 *@a_eof: the new end of file flag.
 *
 *Sets the end of file flag.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_end_of_file (CRInput * a_this, gboolean a_eof)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->end_of_input = a_eof;

        return CR_OK;
}

/**
 * cr_input_get_end_of_file:
 *@a_this: the current instance of #CRInput.
 *@a_eof: out parameter the place to put the end of
 *file flag.
 *
 *Gets the end of file flag.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_get_end_of_file (CRInput const * a_this, gboolean * a_eof)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_eof, CR_BAD_PARAM_ERROR);

        *a_eof = PRIVATE (a_this)->end_of_input;

        return CR_OK;
}

/**
 * cr_input_set_end_of_line:
 *@a_this: the current instance of #CRInput.
 *@a_eol: the new end of line flag.
 *
 *Sets the end of line flag.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_set_end_of_line (CRInput * a_this, gboolean a_eol)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->end_of_line = a_eol;

        return CR_OK;
}

/**
 * cr_input_get_end_of_line:
 *@a_this: the current instance of #CRInput
 *@a_eol: out parameter. The place to put
 *the returned flag
 *
 *Gets the end of line flag of the current input.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_input_get_end_of_line (CRInput const * a_this, gboolean * a_eol)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_eol, CR_BAD_PARAM_ERROR);

        *a_eol = PRIVATE (a_this)->end_of_line;

        return CR_OK;
}

/**
 * cr_input_set_cur_pos:
 *@a_this: the "this pointer" of the current instance of
 *#CRInput.
 *@a_pos: the new position.
 *
 *Sets the current position in the input stream.
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_input_set_cur_pos (CRInput * a_this, CRInputPos const * a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_pos,
                              CR_BAD_PARAM_ERROR);

        cr_input_set_column_num (a_this, a_pos->col);
        cr_input_set_line_num (a_this, a_pos->line);
        cr_input_set_cur_index (a_this, a_pos->next_byte_index);
        cr_input_set_end_of_line (a_this, a_pos->end_of_line);
        cr_input_set_end_of_file (a_this, a_pos->end_of_file);

        return CR_OK;
}
