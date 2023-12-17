/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

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

/**
 *@file
 *The definition of the #CRTknzr (tokenizer)
 *class.
 */

#include <config.h>
#include "string.h"
#include "cr-tknzr.h"
#include "cr-doc-handler.h"

struct _CRTknzrPriv {
        /**The parser input stream of bytes*/
        CRInput *input;

        /**
         *A cache where tknzr_unget_token()
         *puts back the token. tknzr_get_next_token()
         *first look in this cache, and if and 
         *only if it's empty, fetches the next token
         *from the input stream.
         */
        CRToken *token_cache;

        /**
         *The position of the end of the previous token
         *or char fetched.
         */
        CRInputPos prev_pos;

        CRDocHandler *sac_handler;

        /**
         *The reference count of the current instance
         *of #CRTknzr. Is manipulated by cr_tknzr_ref()
         *and cr_tknzr_unref().
         */
        glong ref_count;
};

#define PRIVATE(obj) ((obj)->priv)

/**
 *return TRUE if the character is a number ([0-9]), FALSE otherwise
 *@param a_char the char to test.
 */
#define IS_NUM(a_char) (((a_char) >= '0' && (a_char) <= '9')?TRUE:FALSE)

/**
 *Checks if 'status' equals CR_OK. If not, goto the 'error' label.
 *
 *@param status the status (of type enum CRStatus) to test.
 *@param is_exception if set to FALSE, the final status returned the
 *current function will be CR_PARSING_ERROR. If set to TRUE, the
 *current status will be the current value of the 'status' variable.
 *
 */
#define CHECK_PARSING_STATUS(status, is_exception) \
if ((status) != CR_OK) \
{ \
        if (is_exception == FALSE) \
        { \
                status = CR_PARSING_ERROR ; \
        } \
        goto error ; \
}

/**
 *Peeks the next char from the input stream of the current tokenizer.
 *invokes CHECK_PARSING_STATUS on the status returned by
 *cr_tknzr_input_peek_char().
 *
 *@param the current instance of #CRTkzr.
 *@param to_char a pointer to the char where to store the
 *char peeked.
 */
#define PEEK_NEXT_CHAR(a_tknzr, a_to_char) \
{\
status = cr_tknzr_peek_char  (a_tknzr, a_to_char) ; \
CHECK_PARSING_STATUS (status, TRUE) \
}

/**
 *Reads the next char from the input stream of the current parser.
 *In case of error, jumps to the "error:" label located in the
 *function where this macro is called.
 *@param parser the curent instance of #CRTknzr
 *@param to_char a pointer to the guint32 char where to store
 *the character read.
 */
#define READ_NEXT_CHAR(a_tknzr, to_char) \
status = cr_tknzr_read_char (a_tknzr, to_char) ;\
CHECK_PARSING_STATUS (status, TRUE)

/**
 *Gets information about the current position in
 *the input of the parser.
 *In case of failure, this macro returns from the 
 *calling function and
 *returns a status code of type enum #CRStatus.
 *@param parser the current instance of #CRTknzr.
 *@param pos out parameter. A pointer to the position 
 *inside the current parser input. Must
 */
#define RECORD_INITIAL_POS(a_tknzr, a_pos) \
status = cr_input_get_cur_pos (PRIVATE  \
(a_tknzr)->input, a_pos) ; \
g_return_val_if_fail (status == CR_OK, status)

/**
 *Gets the address of the current byte inside the
 *parser input.
 *@param parser the current instance of #CRTknzr.
 *@param addr out parameter a pointer (guchar*)
 *to where the address  must be put.
 */
#define RECORD_CUR_BYTE_ADDR(a_tknzr, a_addr) \
status = cr_input_get_cur_byte_addr \
            (PRIVATE (a_tknzr)->input, a_addr) ; \
CHECK_PARSING_STATUS (status, TRUE)

/**
 *Peeks a byte from the topmost parser input at
 *a given offset from the current position.
 *If it fails, goto the "error:" label.
 *
 *@param a_parser the current instance of #CRTknzr.
 *@param a_offset the offset of the byte to peek, the
 *current byte having the offset '0'.
 *@param a_byte_ptr out parameter a pointer (guchar*) to
 *where the peeked char is to be stored.
 */
#define PEEK_BYTE(a_tknzr, a_offset, a_byte_ptr) \
status = cr_tknzr_peek_byte (a_tknzr, \
                             a_offset, \
                             a_byte_ptr) ; \
CHECK_PARSING_STATUS (status, TRUE) ;

#define BYTE(a_input, a_n, a_eof) \
cr_input_peek_byte2 (a_input, a_n, a_eof)

/**
 *Reads a byte from the topmost parser input
 *steam.
 *If it fails, goto the "error" label.
 *@param a_parser the current instance of #CRTknzr.
 *@param a_byte_ptr the guchar * where to put the read char.
 */
#define READ_NEXT_BYTE(a_tknzr, a_byte_ptr) \
status = \
cr_input_read_byte (PRIVATE (a_tknzr)->input, a_byte_ptr) ;\
CHECK_PARSING_STATUS (status, TRUE) ;

/**
 *Skips a given number of byte in the topmost
 *parser input. Don't update line and column number.
 *In case of error, jumps to the "error:" label
 *of the surrounding function.
 *@param a_parser the current instance of #CRTknzr.
 *@param a_nb_bytes the number of bytes to skip.
 */
#define SKIP_BYTES(a_tknzr, a_nb_bytes) \
status = cr_input_seek_index (PRIVATE (a_tknzr)->input, \
                                     CR_SEEK_CUR, a_nb_bytes) ; \
CHECK_PARSING_STATUS (status, TRUE) ;

/**
 *Skip utf8 encoded characters.
 *Updates line and column numbers.
 *@param a_parser the current instance of #CRTknzr.
 *@param a_nb_chars the number of chars to skip. Must be of
 *type glong.
 */
#define SKIP_CHARS(a_tknzr, a_nb_chars) \
{ \
gulong nb_chars = a_nb_chars ; \
status = cr_input_consume_chars \
     (PRIVATE (a_tknzr)->input,0, &nb_chars) ; \
CHECK_PARSING_STATUS (status, TRUE) ; \
}

/**
 *Tests the condition and if it is false, sets
 *status to "CR_PARSING_ERROR" and goto the 'error'
 *label.
 *@param condition the condition to test.
 */
#define ENSURE_PARSING_COND(condition) \
if (! (condition)) {status = CR_PARSING_ERROR; goto error ;}

static enum CRStatus  cr_tknzr_parse_nl (CRTknzr * a_this, 
                                         guchar ** a_start, 
                                         guchar ** a_end,
                                         CRParsingLocation *a_location);

static enum CRStatus cr_tknzr_parse_w (CRTknzr * a_this, 
                                       guchar ** a_start, 
                                       guchar ** a_end,
                                       CRParsingLocation *a_location) ;

static enum CRStatus cr_tknzr_parse_unicode_escape (CRTknzr * a_this, 
                                                    guint32 * a_unicode,
                                                    CRParsingLocation *a_location) ;

static enum CRStatus cr_tknzr_parse_escape (CRTknzr * a_this, 
                                            guint32 * a_esc_code,
                                            CRParsingLocation *a_location);

static enum CRStatus cr_tknzr_parse_string (CRTknzr * a_this, 
                                            CRString ** a_str);

static enum CRStatus cr_tknzr_parse_comment (CRTknzr * a_this, 
                                             CRString ** a_comment);

static enum CRStatus cr_tknzr_parse_nmstart (CRTknzr * a_this, 
                                             guint32 * a_char, 
                                             CRParsingLocation *a_location);

static enum CRStatus cr_tknzr_parse_num (CRTknzr * a_this,
                                         CRNum ** a_num);

/**********************************
 *PRIVATE methods
 **********************************/

/**
 *Parses a "w" as defined by the css spec at [4.1.1]:
 * w ::= [ \t\r\n\f]*
 *
 *@param a_this the current instance of #CRTknzr.
 *@param a_start out param. Upon successfull completion, points
 *to the beginning of the parsed white space, points to NULL otherwise.
 *Can also point to NULL is there is no white space actually.
 *@param a_end out param. Upon successfull completion, points
 *to the end of the parsed white space, points to NULL otherwise.
 *Can also point to NULL is there is no white space actually.
 */
static enum CRStatus
cr_tknzr_parse_w (CRTknzr * a_this, 
                  guchar ** a_start, 
                  guchar ** a_end, 
                  CRParsingLocation *a_location)
{
        guint32 cur_char = 0;
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_start && a_end, 
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        *a_start = NULL;
        *a_end = NULL;

        READ_NEXT_CHAR (a_this, &cur_char);

        if (cr_utils_is_white_space (cur_char) == FALSE) {
                status = CR_PARSING_ERROR;
                goto error;
        }
        if (a_location) {
                cr_tknzr_get_parsing_location (a_this, 
                                               a_location) ;
        }
        RECORD_CUR_BYTE_ADDR (a_this, a_start);
        *a_end = *a_start;

        for (;;) {
                gboolean is_eof = FALSE;

                cr_input_get_end_of_file (PRIVATE (a_this)->input, &is_eof);
                if (is_eof)
                        break;

                status = cr_tknzr_peek_char (a_this, &cur_char);
                if (status == CR_END_OF_INPUT_ERROR) {
                        break;
                } else if (status != CR_OK) {
                        goto error;
                }

                if (cr_utils_is_white_space (cur_char) == TRUE) {
                        READ_NEXT_CHAR (a_this, &cur_char);
                        RECORD_CUR_BYTE_ADDR (a_this, a_end);
                } else {
                        break;
                }
        }

        return CR_OK;

      error:
        cr_tknzr_set_cur_pos (a_this, &init_pos);

        return status;
}

/**
 *Parses a newline as defined in the css2 spec:
 * nl   ::=    \n|\r\n|\r|\f
 *
 *@param a_this the "this pointer" of the current instance of #CRTknzr.
 *@param a_start a pointer to the first character of the successfully 
 *parsed string.
 *@param a_end a pointer to the last character of the successfully parsed
 *string.
 *@result CR_OK uppon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_nl (CRTknzr * a_this, 
                   guchar ** a_start, 
                   guchar ** a_end, 
                   CRParsingLocation *a_location)
{
        CRInputPos init_pos;
        guchar next_chars[2] = { 0 };
        enum CRStatus status = CR_PARSING_ERROR;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_start && a_end, CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        PEEK_BYTE (a_this, 1, &next_chars[0]);
        PEEK_BYTE (a_this, 2, &next_chars[1]);

        if ((next_chars[0] == '\r' && next_chars[1] == '\n')) {
                SKIP_BYTES (a_this, 1);
                if (a_location) {
                        cr_tknzr_get_parsing_location 
                                (a_this, a_location) ;
                }
                SKIP_CHARS (a_this, 1);

                RECORD_CUR_BYTE_ADDR (a_this, a_end);

                status = CR_OK;
        } else if (next_chars[0] == '\n'
                   || next_chars[0] == '\r' || next_chars[0] == '\f') {
                SKIP_CHARS (a_this, 1);
                if (a_location) {
                        cr_tknzr_get_parsing_location 
                                (a_this, a_location) ;
                }
                RECORD_CUR_BYTE_ADDR (a_this, a_start);
                *a_end = *a_start;
                status = CR_OK;
        } else {
                status = CR_PARSING_ERROR;
                goto error;
        }
        return CR_OK ;

 error:
        cr_tknzr_set_cur_pos (a_this, &init_pos) ;
        return status;
}

/**
 *Go ahead in the parser input, skipping all the spaces.
 *If the next char if not a white space, this function does nothing.
 *In any cases, it stops when it encounters a non white space character.
 *
 *@param a_this the current instance of #CRTknzr.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_try_to_skip_spaces (CRTknzr * a_this)
{
        enum CRStatus status = CR_ERROR;
        guint32 cur_char = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, CR_BAD_PARAM_ERROR);

        status = cr_input_peek_char (PRIVATE (a_this)->input, &cur_char);

        if (status != CR_OK) {
                if (status == CR_END_OF_INPUT_ERROR)
                        return CR_OK;
                return status;
        }

        if (cr_utils_is_white_space (cur_char) == TRUE) {
                gulong nb_chars = -1; /*consume all spaces */

                status = cr_input_consume_white_spaces
                        (PRIVATE (a_this)->input, &nb_chars);
        }

        return status;
}

/**
 *Parses a "comment" as defined in the css spec at [4.1.1]:
 *COMMENT ::= \/\*[^*]*\*+([^/][^*]*\*+)*\/ .
 *This complex regexp is just to say that comments start
 *with the two chars '/''*' and ends with the two chars '*''/'.
 *It also means that comments cannot be nested.
 *So based on that, I've just tried to implement the parsing function
 *simply and in a straight forward manner.
 */
static enum CRStatus
cr_tknzr_parse_comment (CRTknzr * a_this, 
                        CRString ** a_comment)
{
        enum CRStatus status = CR_OK;
        CRInputPos init_pos;
        guint32 cur_char = 0, next_char= 0;
        CRString *comment = NULL;
        CRParsingLocation loc = {0} ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, 
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);        
        READ_NEXT_CHAR (a_this, &cur_char) ;        
        ENSURE_PARSING_COND (cur_char == '/');
        cr_tknzr_get_parsing_location (a_this, &loc) ;

        READ_NEXT_CHAR (a_this, &cur_char);
        ENSURE_PARSING_COND (cur_char == '*');
        comment = cr_string_new ();
        for (;;) { /* [^*]* */
                PEEK_NEXT_CHAR (a_this, &next_char);
                if (next_char == '*')
                        break;
                READ_NEXT_CHAR (a_this, &cur_char);
                g_string_append_unichar (comment->stryng, cur_char);
        }
        /* Stop condition: next_char == '*' */
        for (;;) { /* \*+ */
                READ_NEXT_CHAR(a_this, &cur_char);
                ENSURE_PARSING_COND (cur_char == '*');
                g_string_append_unichar (comment->stryng, cur_char);
                PEEK_NEXT_CHAR (a_this, &next_char);
                if (next_char != '*')
                        break;
        }
        /* Stop condition: next_char != '*' */
        for (;;) { /* ([^/][^*]*\*+)* */
                if (next_char == '/')
                        break;
                READ_NEXT_CHAR(a_this, &cur_char);
                g_string_append_unichar (comment->stryng, cur_char);
                for (;;) { /* [^*]* */
                        PEEK_NEXT_CHAR (a_this, &next_char);
                        if (next_char == '*')
                                break;
                        READ_NEXT_CHAR (a_this, &cur_char);
                        g_string_append_unichar (comment->stryng, cur_char);
                }
                /* Stop condition: next_char = '*', no need to verify, because peek and read exit to error anyway */
                for (;;) { /* \*+ */
                        READ_NEXT_CHAR(a_this, &cur_char);
                        ENSURE_PARSING_COND (cur_char == '*');
                        g_string_append_unichar (comment->stryng, cur_char);
                        PEEK_NEXT_CHAR (a_this, &next_char);
                        if (next_char != '*')
                                break;
                }
                /* Continue condition: next_char != '*' */
        }
        /* Stop condition: next_char == '\/' */
        READ_NEXT_CHAR(a_this, &cur_char);
        g_string_append_unichar (comment->stryng, cur_char);

        if (status == CR_OK) {
                cr_parsing_location_copy (&comment->location, 
                                          &loc) ;
                *a_comment = comment;                
                return CR_OK;
        }
 error:

        if (comment) {
                cr_string_destroy (comment);
                comment = NULL;
        }

        cr_tknzr_set_cur_pos (a_this, &init_pos);

        return status;
}

/**
 *Parses an 'unicode' escape sequence defined
 *in css spec at chap 4.1.1:
 *unicode ::= \\[0-9a-f]{1,6}[ \n\r\t\f]?
 *@param a_this the current instance of #CRTknzr.
 *@param a_start out parameter. A pointer to the start
 *of the unicode escape sequence. Must *NOT* be deleted by
 *the caller.
 *@param a_end out parameter. A pointer to the last character
 *of the unicode escape sequence. Must *NOT* be deleted by the caller.
 *@return CR_OK if parsing succeded, an error code otherwise.
 *Error code can be either CR_PARSING_ERROR if the string 
 *parsed just doesn't
 *respect the production or another error if a 
 *lower level error occurred.
 */
static enum CRStatus
cr_tknzr_parse_unicode_escape (CRTknzr * a_this, 
                               guint32 * a_unicode,
                               CRParsingLocation *a_location)
{
        guint32 cur_char;
        CRInputPos init_pos;
        glong occur = 0;
        guint32 unicode = 0;
        guchar *tmp_char_ptr1 = NULL,
                *tmp_char_ptr2 = NULL;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_unicode, CR_BAD_PARAM_ERROR);

        /*first, let's backup the current position pointer */
        RECORD_INITIAL_POS (a_this, &init_pos);

        READ_NEXT_CHAR (a_this, &cur_char);

        if (cur_char != '\\') {
                status = CR_PARSING_ERROR;
                goto error;
        }
        if (a_location) {
                cr_tknzr_get_parsing_location 
                        (a_this, a_location) ;
        }
        PEEK_NEXT_CHAR (a_this, &cur_char);

        for (occur = 0, unicode = 0; ((cur_char >= '0' && cur_char <= '9')
                                      || (cur_char >= 'a' && cur_char <= 'f')
                                      || (cur_char >= 'A' && cur_char <= 'F'))
             && occur < 6; occur++) {
                gint cur_char_val = 0;

                READ_NEXT_CHAR (a_this, &cur_char);

                if ((cur_char >= '0' && cur_char <= '9')) {
                        cur_char_val = (cur_char - '0');
                } else if ((cur_char >= 'a' && cur_char <= 'f')) {
                        cur_char_val = 10 + (cur_char - 'a');
                } else if ((cur_char >= 'A' && cur_char <= 'F')) {
                        cur_char_val = 10 + (cur_char - 'A');
                }

                unicode = unicode * 16 + cur_char_val;

                PEEK_NEXT_CHAR (a_this, &cur_char);
        }

        /* Eat a whitespace if possible. */
        cr_tknzr_parse_w (a_this, &tmp_char_ptr1, 
                          &tmp_char_ptr2, NULL);
        *a_unicode = unicode;
        return CR_OK;

      error:
        /*
         *restore the initial position pointer backuped at
         *the beginning of this function.
         */
        cr_tknzr_set_cur_pos (a_this, &init_pos);

        return status;
}

/**
 *parses an escape sequence as defined by the css spec:
 *escape ::= {unicode}|\\[ -~\200-\4177777]
 *@param a_this the current instance of #CRTknzr .
 */
static enum CRStatus
cr_tknzr_parse_escape (CRTknzr * a_this, guint32 * a_esc_code,
                       CRParsingLocation *a_location)
{
        enum CRStatus status = CR_OK;
        guint32 cur_char = 0;
        CRInputPos init_pos;
        guchar next_chars[2];

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_esc_code, CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        PEEK_BYTE (a_this, 1, &next_chars[0]);
        PEEK_BYTE (a_this, 2, &next_chars[1]);

        if (next_chars[0] != '\\') {
                status = CR_PARSING_ERROR;
                goto error;
        }

        if ((next_chars[1] >= '0' && next_chars[1] <= '9')
            || (next_chars[1] >= 'a' && next_chars[1] <= 'f')
            || (next_chars[1] >= 'A' && next_chars[1] <= 'F')) {
                status = cr_tknzr_parse_unicode_escape (a_this, a_esc_code, 
                                                        a_location);
        } else {
                /*consume the '\' char */
                READ_NEXT_CHAR (a_this, &cur_char);
                if (a_location) {
                        cr_tknzr_get_parsing_location (a_this, 
                                                       a_location) ;
                }
                /*then read the char after the '\' */
                READ_NEXT_CHAR (a_this, &cur_char);

                if (cur_char != ' ' && (cur_char < 200 || cur_char > 4177777)) {
                        status = CR_PARSING_ERROR;
                        goto error;
                }
                *a_esc_code = cur_char;

        }
        if (status == CR_OK) {
                return CR_OK;
        }
 error:
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;
}

/**
 *Parses a string type as defined in css spec [4.1.1]:
 *
 *string ::= {string1}|{string2}
 *string1 ::= \"([\t !#$%&(-~]|\\{nl}|\'|{nonascii}|{escape})*\"
 *string2 ::= \'([\t !#$%&(-~]|\\{nl}|\"|{nonascii}|{escape})*\'
 *
 *@param a_this the current instance of #CRTknzr.
 *@param a_start out parameter. Upon successfull completion, 
 *points to the beginning of the string, points to an undefined value
 *otherwise.
 *@param a_end out parameter. Upon successfull completion, points to
 *the beginning of the string, points to an undefined value otherwise.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_string (CRTknzr * a_this, CRString ** a_str)
{
        guint32 cur_char = 0,
                delim = 0;
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;
        CRString *str = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_str, CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);
        READ_NEXT_CHAR (a_this, &cur_char);

        if (cur_char == '"')
                delim = '"';
        else if (cur_char == '\'')
                delim = '\'';
        else {
                status = CR_PARSING_ERROR;
                goto error;
        }
        str = cr_string_new ();
        if (str) {
                cr_tknzr_get_parsing_location 
                        (a_this, &str->location) ;
        }
        for (;;) {
                guchar next_chars[2] = { 0 };

                PEEK_BYTE (a_this, 1, &next_chars[0]);
                PEEK_BYTE (a_this, 2, &next_chars[1]);

                if (next_chars[0] == '\\') {
                        guchar *tmp_char_ptr1 = NULL,
                                *tmp_char_ptr2 = NULL;
                        guint32 esc_code = 0;

                        if (next_chars[1] == '\'' || next_chars[1] == '"') {
                                g_string_append_unichar (str->stryng, 
                                                         next_chars[1]);
                                SKIP_BYTES (a_this, 2);
                                status = CR_OK;
                        } else {
                                status = cr_tknzr_parse_escape
                                        (a_this, &esc_code, NULL);

                                if (status == CR_OK) {
                                        g_string_append_unichar
                                                (str->stryng, 
                                                 esc_code);
                                }
                        }

                        if (status != CR_OK) {
                                /*
                                 *consume the '\' char, and try to parse
                                 *a newline.
                                 */
                                READ_NEXT_CHAR (a_this, &cur_char);

                                status = cr_tknzr_parse_nl
                                        (a_this, &tmp_char_ptr1,
                                         &tmp_char_ptr2, NULL);
                        }

                        CHECK_PARSING_STATUS (status, FALSE);
                } else if (strchr ("\t !#$%&", next_chars[0])
                           || (next_chars[0] >= '(' && next_chars[0] <= '~')) {
                        READ_NEXT_CHAR (a_this, &cur_char);
                        g_string_append_unichar (str->stryng, 
                                                 cur_char);
                        status = CR_OK;
                }

                else if (cr_utils_is_nonascii (next_chars[0])) {
                        READ_NEXT_CHAR (a_this, &cur_char);
                        g_string_append_unichar (str->stryng, cur_char);
                } else if (next_chars[0] == delim) {
                        READ_NEXT_CHAR (a_this, &cur_char);
                        break;
                } else {
                        status = CR_PARSING_ERROR;
                        goto error;
                }
        }

        if (status == CR_OK) {
                if (*a_str == NULL) {
                        *a_str = str;
                        str = NULL;
                } else {
                        (*a_str)->stryng = g_string_append_len
                                ((*a_str)->stryng,
                                 str->stryng->str, 
                                 str->stryng->len);
                        cr_string_destroy (str);
                }
                return CR_OK;
        }

 error:

        if (str) {
                cr_string_destroy (str) ;
                str = NULL;
        }
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;
}

/**
 *Parses the an nmstart as defined by the css2 spec [4.1.1]:
 * nmstart [a-zA-Z]|{nonascii}|{escape}
 *
 *@param a_this the current instance of #CRTknzr.
 *@param a_start out param. A pointer to the starting point of
 *the token.
 *@param a_end out param. A pointer to the ending point of the
 *token.
 *@param a_char out param. The actual parsed nmchar.
 *@return CR_OK upon successfull completion, 
 *an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_nmstart (CRTknzr * a_this, 
                        guint32 * a_char,
                        CRParsingLocation *a_location)
{
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;
        guint32 cur_char = 0,
                next_char = 0;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_char, CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        PEEK_NEXT_CHAR (a_this, &next_char);

        if (next_char == '\\') {
                status = cr_tknzr_parse_escape (a_this, a_char,
                                                a_location);

                if (status != CR_OK)
                        goto error;

        } else if (cr_utils_is_nonascii (next_char) == TRUE
                   || ((next_char >= 'a') && (next_char <= 'z'))
                   || ((next_char >= 'A') && (next_char <= 'Z'))
                ) {
                READ_NEXT_CHAR (a_this, &cur_char);
                if (a_location) {
                        cr_tknzr_get_parsing_location (a_this, 
                                                       a_location) ;
                }
                *a_char = cur_char;
                status = CR_OK;
        } else {
                status = CR_PARSING_ERROR;
                goto error;
        }

        return CR_OK;

 error:        
        cr_tknzr_set_cur_pos (a_this, &init_pos);

        return status;

}

/**
 *Parses an nmchar as described in the css spec at
 *chap 4.1.1:
 *nmchar ::= [a-z0-9-]|{nonascii}|{escape}
 *
 *Humm, I have added the possibility for nmchar to
 *contain upper case letters.
 *
 *@param a_this the current instance of #CRTknzr.
 *@param a_start out param. A pointer to the starting point of
 *the token.
 *@param a_end out param. A pointer to the ending point of the
 *token.
 *@param a_char out param. The actual parsed nmchar.
 *@return CR_OK upon successfull completion, 
 *an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_nmchar (CRTknzr * a_this, guint32 * a_char,
                       CRParsingLocation *a_location)
{
        guint32 cur_char = 0,
                next_char = 0;
        enum CRStatus status = CR_OK;
        CRInputPos init_pos;

        g_return_val_if_fail (a_this && PRIVATE (a_this) && a_char,
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        status = cr_input_peek_char (PRIVATE (a_this)->input, 
                                     &next_char) ;
        if (status != CR_OK)
                goto error;

        if (next_char == '\\') {
                status = cr_tknzr_parse_escape (a_this, a_char, 
                                                a_location);

                if (status != CR_OK)
                        goto error;

        } else if (cr_utils_is_nonascii (next_char) == TRUE
                   || ((next_char >= 'a') && (next_char <= 'z'))
                   || ((next_char >= 'A') && (next_char <= 'Z'))
                   || ((next_char >= '0') && (next_char <= '9'))
                   || (next_char == '-')
                   || (next_char == '_') /*'_' not allowed by the spec. */
                ) {
                READ_NEXT_CHAR (a_this, &cur_char);
                *a_char = cur_char;
                status = CR_OK;
                if (a_location) {
                        cr_tknzr_get_parsing_location
                                (a_this, a_location) ;
                }
        } else {
                status = CR_PARSING_ERROR;
                goto error;
        }
        return CR_OK;

 error:
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;
}

/**
 *Parses an "ident" as defined in css spec [4.1.1]:
 *ident ::= {nmstart}{nmchar}*
 *
 *Actually parses it using the css3 grammar:
 *ident ::= -?{nmstart}{nmchar}*
 *@param a_this the currens instance of #CRTknzr.
 *
 *@param a_str a pointer to parsed ident. If *a_str is NULL,
 *this function allocates a new instance of CRString. If not, 
 *the function just appends the parsed string to the one passed.
 *In both cases it is up to the caller to free *a_str.
 *
 *@return CR_OK upon successfull completion, an error code 
 *otherwise.
 */
static enum CRStatus
cr_tknzr_parse_ident (CRTknzr * a_this, CRString ** a_str)
{
        guint32 tmp_char = 0;
        CRString *stringue = NULL ;
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;
        gboolean location_is_set = FALSE ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_str, CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);
        PEEK_NEXT_CHAR (a_this, &tmp_char) ;
        stringue = cr_string_new () ;
        g_return_val_if_fail (stringue, 
                              CR_OUT_OF_MEMORY_ERROR) ;

        if (tmp_char == '-') {
                READ_NEXT_CHAR (a_this, &tmp_char) ;
                cr_tknzr_get_parsing_location
                        (a_this, &stringue->location) ;
                location_is_set = TRUE ;
                g_string_append_unichar (stringue->stryng, 
                                         tmp_char) ;
        }
        status = cr_tknzr_parse_nmstart (a_this, &tmp_char, NULL);
        if (status != CR_OK) {
                status = CR_PARSING_ERROR;
                goto end ;
        }
        if (location_is_set == FALSE) {
                cr_tknzr_get_parsing_location 
                        (a_this, &stringue->location) ;
                location_is_set = TRUE ;
        }
        g_string_append_unichar (stringue->stryng, tmp_char);
        for (;;) {
                status = cr_tknzr_parse_nmchar (a_this, 
                                                &tmp_char, 
                                                NULL);
                if (status != CR_OK) {
                        status = CR_OK ;
                        break;
                }
                g_string_append_unichar (stringue->stryng, tmp_char);
        }
        if (status == CR_OK) {
                if (!*a_str) {
                        *a_str = stringue ;
                
                } else {
                        g_string_append_len ((*a_str)->stryng, 
                                             stringue->stryng->str, 
                                             stringue->stryng->len) ;
                        cr_string_destroy (stringue) ;
                }
                stringue = NULL ;
        }

 error:
 end:
        if (stringue) {
                cr_string_destroy (stringue) ;
                stringue = NULL ;
        }
        if (status != CR_OK ) {
                cr_tknzr_set_cur_pos (a_this, &init_pos) ;
        }
        return status ;
}


/**
 *Parses a "name" as defined by css spec [4.1.1]:
 *name ::= {nmchar}+
 *
 *@param a_this the current instance of #CRTknzr.
 *
 *@param a_str out parameter. A pointer to the successfully parsed
 *name. If *a_str is set to NULL, this function allocates a new instance
 *of CRString. If not, it just appends the parsed name to the passed *a_str.
 *In both cases, it is up to the caller to free *a_str.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_name (CRTknzr * a_this, 
                     CRString ** a_str)
{
        guint32 tmp_char = 0;
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;
        gboolean str_needs_free = FALSE,
                is_first_nmchar=TRUE ;
        glong i = 0;
        CRParsingLocation loc = {0} ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_str,
                              CR_BAD_PARAM_ERROR) ;

        RECORD_INITIAL_POS (a_this, &init_pos);

        if (*a_str == NULL) {
                *a_str = cr_string_new ();
                str_needs_free = TRUE;
        }
        for (i = 0;; i++) {
                if (is_first_nmchar == TRUE) {
                        status = cr_tknzr_parse_nmchar 
                                (a_this, &tmp_char,
                                 &loc) ;
                        is_first_nmchar = FALSE ;
                } else {
                        status = cr_tknzr_parse_nmchar 
                                (a_this, &tmp_char, NULL) ;
                }
                if (status != CR_OK)
                        break;                
                g_string_append_unichar ((*a_str)->stryng, 
                                         tmp_char);
        }
        if (i > 0) {
                cr_parsing_location_copy 
                        (&(*a_str)->location, &loc) ;
                return CR_OK;
        }
        if (str_needs_free == TRUE && *a_str) {
                cr_string_destroy (*a_str);
                *a_str = NULL;
        }
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return CR_PARSING_ERROR;
}

/**
 *Parses a "hash" as defined by the css spec in [4.1.1]:
 *HASH ::= #{name}
 */
static enum CRStatus
cr_tknzr_parse_hash (CRTknzr * a_this, CRString ** a_str)
{
        guint32 cur_char = 0;
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;
        gboolean str_needs_free = FALSE;
        CRParsingLocation loc = {0} ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input,
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);
        READ_NEXT_CHAR (a_this, &cur_char);
        if (cur_char != '#') {
                status = CR_PARSING_ERROR;
                goto error;
        }
        if (*a_str == NULL) {
                *a_str = cr_string_new ();
                str_needs_free = TRUE;
        }
        cr_tknzr_get_parsing_location (a_this,
                                       &loc) ;
        status = cr_tknzr_parse_name (a_this, a_str);
        cr_parsing_location_copy (&(*a_str)->location, &loc) ;
        if (status != CR_OK) {
                goto error;
        }
        return CR_OK;

 error:
        if (str_needs_free == TRUE && *a_str) {
                cr_string_destroy (*a_str);
                *a_str = NULL;
        }

        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;
}

/**
 *Parses an uri as defined by the css spec [4.1.1]:
 * URI ::= url\({w}{string}{w}\)
 *         |url\({w}([!#$%&*-~]|{nonascii}|{escape})*{w}\)
 *
 *@param a_this the current instance of #CRTknzr.
 *@param a_str the successfully parsed url.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_uri (CRTknzr * a_this, 
                    CRString ** a_str)
{
        guint32 cur_char = 0;
        CRInputPos init_pos;
        enum CRStatus status = CR_PARSING_ERROR;
        guchar tab[4] = { 0 }, *tmp_ptr1 = NULL, *tmp_ptr2 = NULL;
        CRString *str = NULL;
        CRParsingLocation location = {0} ;

        g_return_val_if_fail (a_this 
                              && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_str, 
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        PEEK_BYTE (a_this, 1, &tab[0]);
        PEEK_BYTE (a_this, 2, &tab[1]);
        PEEK_BYTE (a_this, 3, &tab[2]);
        PEEK_BYTE (a_this, 4, &tab[3]);

        if (tab[0] != 'u' || tab[1] != 'r' || tab[2] != 'l' || tab[3] != '(') {
                status = CR_PARSING_ERROR;
                goto error;
        }
        /*
         *Here, we want to skip 4 bytes ('u''r''l''(').
         *But we also need to keep track of the parsing location
         *of the 'u'. So, we skip 1 byte, we record the parsing
         *location, then we skip the 3 remaining bytes.
         */
        SKIP_CHARS (a_this, 1);
        cr_tknzr_get_parsing_location (a_this, &location) ;
        SKIP_CHARS (a_this, 3);
        cr_tknzr_try_to_skip_spaces (a_this);
        status = cr_tknzr_parse_string (a_this, a_str);

        if (status == CR_OK) {
                guint32 next_char = 0;
                status = cr_tknzr_parse_w (a_this, &tmp_ptr1, 
                                           &tmp_ptr2, NULL);
                cr_tknzr_try_to_skip_spaces (a_this);
                PEEK_NEXT_CHAR (a_this, &next_char);
                if (next_char == ')') {
                        READ_NEXT_CHAR (a_this, &cur_char);
                        status = CR_OK;
                } else {
                        status = CR_PARSING_ERROR;
                }
        }
        if (status != CR_OK) {
                str = cr_string_new ();
                for (;;) {
                        guint32 next_char = 0;
                        PEEK_NEXT_CHAR (a_this, &next_char);
                        if (strchr ("!#$%&", next_char)
                            || (next_char >= '*' && next_char <= '~')
                            || (cr_utils_is_nonascii (next_char) == TRUE)) {
                                READ_NEXT_CHAR (a_this, &cur_char);
                                g_string_append_unichar 
                                        (str->stryng, cur_char);
                                status = CR_OK;
                        } else {
                                guint32 esc_code = 0;
                                status = cr_tknzr_parse_escape
                                        (a_this, &esc_code, NULL);
                                if (status == CR_OK) {
                                        g_string_append_unichar
                                                (str->stryng, 
                                                 esc_code);
                                } else {
                                        status = CR_OK;
                                        break;
                                }
                        }
                }
                cr_tknzr_try_to_skip_spaces (a_this);
                READ_NEXT_CHAR (a_this, &cur_char);
                if (cur_char == ')') {
                        status = CR_OK;
                } else {
                        status = CR_PARSING_ERROR;
                        goto error;
                }
                if (str) {                        
                        if (*a_str == NULL) {
                                *a_str = str;
                                str = NULL;
                        } else {
                                g_string_append_len
                                        ((*a_str)->stryng,
                                         str->stryng->str,
                                         str->stryng->len);
                                cr_string_destroy (str);
                        }                        
                }
        }

        cr_parsing_location_copy
                (&(*a_str)->location,
                 &location) ;
        return CR_OK ;
 error:
        if (str) {
                cr_string_destroy (str);
                str = NULL;
        }
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;
}

/**
 *parses an RGB as defined in the css2 spec.
 *rgb: rgb '('S*{num}%?S* ',' {num}#?S*,S*{num}#?S*')'
 *
 *@param a_this the "this pointer" of the current instance of
 *@param a_rgb out parameter the parsed rgb.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_rgb (CRTknzr * a_this, CRRgb ** a_rgb)
{
        enum CRStatus status = CR_OK;
        CRInputPos init_pos;
        CRNum *num = NULL;
        guchar next_bytes[3] = { 0 }, cur_byte = 0;
        glong red = 0,
                green = 0,
                blue = 0,
                i = 0;
        gboolean is_percentage = FALSE;
        CRParsingLocation location = {0} ;

        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        PEEK_BYTE (a_this, 1, &next_bytes[0]);
        PEEK_BYTE (a_this, 2, &next_bytes[1]);
        PEEK_BYTE (a_this, 3, &next_bytes[2]);

        if (((next_bytes[0] == 'r') || (next_bytes[0] == 'R'))
            && ((next_bytes[1] == 'g') || (next_bytes[1] == 'G'))
            && ((next_bytes[2] == 'b') || (next_bytes[2] == 'B'))) {
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, &location) ;
                SKIP_CHARS (a_this, 2);
        } else {
                status = CR_PARSING_ERROR;
                goto error;
        }
        READ_NEXT_BYTE (a_this, &cur_byte);
        ENSURE_PARSING_COND (cur_byte == '(');

        cr_tknzr_try_to_skip_spaces (a_this);
        status = cr_tknzr_parse_num (a_this, &num);
        ENSURE_PARSING_COND ((status == CR_OK) && (num != NULL));

        if (num->val > G_MAXLONG) {
                status = CR_PARSING_ERROR;
                goto error;
        }

        red = num->val;
        cr_num_destroy (num);
        num = NULL;

        PEEK_BYTE (a_this, 1, &next_bytes[0]);
        if (next_bytes[0] == '%') {
                SKIP_CHARS (a_this, 1);
                is_percentage = TRUE;
        }
        cr_tknzr_try_to_skip_spaces (a_this);

        for (i = 0; i < 2; i++) {
                READ_NEXT_BYTE (a_this, &cur_byte);
                ENSURE_PARSING_COND (cur_byte == ',');

                cr_tknzr_try_to_skip_spaces (a_this);
                status = cr_tknzr_parse_num (a_this, &num);
                ENSURE_PARSING_COND ((status == CR_OK) && (num != NULL));

                if (num->val > G_MAXLONG) {
                        status = CR_PARSING_ERROR;
                        goto error;
                }

                PEEK_BYTE (a_this, 1, &next_bytes[0]);
                if (next_bytes[0] == '%') {
                        SKIP_CHARS (a_this, 1);
                        is_percentage = 1;
                }

                if (i == 0) {
                        green = num->val;
                } else if (i == 1) {
                        blue = num->val;
                }

                if (num) {
                        cr_num_destroy (num);
                        num = NULL;
                }
                cr_tknzr_try_to_skip_spaces (a_this);
        }

        READ_NEXT_BYTE (a_this, &cur_byte);
        if (*a_rgb == NULL) {
                *a_rgb = cr_rgb_new_with_vals (red, green, blue,
                                               is_percentage);

                if (*a_rgb == NULL) {
                        status = CR_ERROR;
                        goto error;
                }
                status = CR_OK;
        } else {
                (*a_rgb)->red = red;
                (*a_rgb)->green = green;
                (*a_rgb)->blue = blue;
                (*a_rgb)->is_percentage = is_percentage;

                status = CR_OK;
        }

        if (status == CR_OK) {
                if (a_rgb && *a_rgb) {
                        cr_parsing_location_copy 
                                (&(*a_rgb)->location, 
                                 &location) ;
                }
                return CR_OK;
        }

 error:
        if (num) {
                cr_num_destroy (num);
                num = NULL;
        }

        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return CR_OK;
}

/**
 *Parses a atkeyword as defined by the css spec in [4.1.1]:
 *ATKEYWORD ::= @{ident}
 *
 *@param a_this the "this pointer" of the current instance of
 *#CRTknzr.
 *
 *@param a_str out parameter. The parsed atkeyword. If *a_str is
 *set to NULL this function allocates a new instance of CRString and
 *sets it to the parsed atkeyword. If not, this function just appends
 *the parsed atkeyword to the end of *a_str. In both cases it is up to
 *the caller to free *a_str.
 *
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
cr_tknzr_parse_atkeyword (CRTknzr * a_this, 
                          CRString ** a_str)
{
        guint32 cur_char = 0;
        CRInputPos init_pos;
        gboolean str_needs_free = FALSE;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_str, CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);

        READ_NEXT_CHAR (a_this, &cur_char);

        if (cur_char != '@') {
                status = CR_PARSING_ERROR;
                goto error;
        }

        if (*a_str == NULL) {
                *a_str = cr_string_new ();
                str_needs_free = TRUE;
        }
        status = cr_tknzr_parse_ident (a_this, a_str);
        if (status != CR_OK) {
                goto error;
        }
        return CR_OK;
 error:

        if (str_needs_free == TRUE && *a_str) {
                cr_string_destroy (*a_str);
                *a_str = NULL;
        }
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;
}

static enum CRStatus
cr_tknzr_parse_important (CRTknzr * a_this,
                          CRParsingLocation *a_location)
{
        guint32 cur_char = 0;
        CRInputPos init_pos;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input,
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);
        READ_NEXT_CHAR (a_this, &cur_char);
        ENSURE_PARSING_COND (cur_char == '!');
        if (a_location) {
                cr_tknzr_get_parsing_location (a_this, 
                                               a_location) ;
        }
        cr_tknzr_try_to_skip_spaces (a_this);

        if (BYTE (PRIVATE (a_this)->input, 1, NULL) == 'i'
            && BYTE (PRIVATE (a_this)->input, 2, NULL) == 'm'
            && BYTE (PRIVATE (a_this)->input, 3, NULL) == 'p'
            && BYTE (PRIVATE (a_this)->input, 4, NULL) == 'o'
            && BYTE (PRIVATE (a_this)->input, 5, NULL) == 'r'
            && BYTE (PRIVATE (a_this)->input, 6, NULL) == 't'
            && BYTE (PRIVATE (a_this)->input, 7, NULL) == 'a'
            && BYTE (PRIVATE (a_this)->input, 8, NULL) == 'n'
            && BYTE (PRIVATE (a_this)->input, 9, NULL) == 't') {
                SKIP_BYTES (a_this, 9);
                if (a_location) {
                        cr_tknzr_get_parsing_location (a_this,
                                                       a_location) ;
                }
                return CR_OK;
        } else {
                status = CR_PARSING_ERROR;
        }

 error:
        cr_tknzr_set_cur_pos (a_this, &init_pos);

        return status;
}

/**
 *Parses a num as defined in the css spec [4.1.1]:
 *[0-9]+|[0-9]*\.[0-9]+
 *@param a_this the current instance of #CRTknzr.
 *@param a_num out parameter. The parsed number.
 *@return CR_OK upon successfull completion, 
 *an error code otherwise.
 *
 *The CSS specification says that numbers may be
 *preceeded by '+' or '-' to indicate the sign.
 *Technically, the "num" construction as defined
 *by the tokenizer doesn't allow this, but we parse
 *it here for simplicity.
 */
static enum CRStatus
cr_tknzr_parse_num (CRTknzr * a_this, 
                    CRNum ** a_num)
{
        enum CRStatus status = CR_PARSING_ERROR;
        enum CRNumType val_type = NUM_GENERIC;
        gboolean parsing_dec,  /* true iff seen decimal point. */
                parsed; /* true iff the substring seen so far is a valid CSS
                           number, i.e. `[0-9]+|[0-9]*\.[0-9]+'. */
        guint32 cur_char = 0,
                next_char = 0;
        gdouble numerator, denominator = 1;
        CRInputPos init_pos;
        CRParsingLocation location = {0} ;
        int sign = 1;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, 
                              CR_BAD_PARAM_ERROR);

        RECORD_INITIAL_POS (a_this, &init_pos);
        READ_NEXT_CHAR (a_this, &cur_char);

        if (cur_char == '+' || cur_char == '-') {
                if (cur_char == '-') {
                        sign = -1;
                }
                READ_NEXT_CHAR (a_this, &cur_char);
        }

        if (IS_NUM (cur_char)) {
                numerator = (cur_char - '0');
                parsing_dec = FALSE;
                parsed = TRUE;
        } else if (cur_char == '.') {
                numerator = 0;
                parsing_dec = TRUE;
                parsed = FALSE;
        } else {
                status = CR_PARSING_ERROR;
                goto error;
        }
        cr_tknzr_get_parsing_location (a_this, &location) ;

        for (;;) {
                status = cr_tknzr_peek_char (a_this, &next_char);
                if (status != CR_OK) {
                        if (status == CR_END_OF_INPUT_ERROR)
                                status = CR_OK;
                        break;
                }
                if (next_char == '.') {
                        if (parsing_dec) {
                                status = CR_PARSING_ERROR;
                                goto error;
                        }

                        READ_NEXT_CHAR (a_this, &cur_char);
                        parsing_dec = TRUE;
                        parsed = FALSE;  /* In CSS, there must be at least
                                            one digit after `.'. */
                } else if (IS_NUM (next_char)) {
                        READ_NEXT_CHAR (a_this, &cur_char);
                        parsed = TRUE;

                        numerator = numerator * 10 + (cur_char - '0');
                        if (parsing_dec) {
                                denominator *= 10;
                        }
                } else {
                        break;
                }
        }

        if (!parsed) {
                status = CR_PARSING_ERROR;
        }

        /*
         *Now, set the output param values.
         */
        if (status == CR_OK) {
                gdouble val = (numerator / denominator) * sign;
                if (*a_num == NULL) {
                        *a_num = cr_num_new_with_val (val, val_type);

                        if (*a_num == NULL) {
                                status = CR_ERROR;
                                goto error;
                        }
                } else {
                        (*a_num)->val = val;
                        (*a_num)->type = val_type;
                }
                cr_parsing_location_copy (&(*a_num)->location,
                                          &location) ;
                return CR_OK;
        }

 error:

        cr_tknzr_set_cur_pos (a_this, &init_pos);

        return status;
}

/*********************************************
 *PUBLIC methods
 ********************************************/

CRTknzr *
cr_tknzr_new (CRInput * a_input)
{
        CRTknzr *result = NULL;

        result = g_try_malloc (sizeof (CRTknzr));

        if (result == NULL) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRTknzr));

        result->priv = g_try_malloc (sizeof (CRTknzrPriv));

        if (result->priv == NULL) {
                cr_utils_trace_info ("Out of memory");

                if (result) {
                        g_free (result);
                        result = NULL;
                }

                return NULL;
        }
        memset (result->priv, 0, sizeof (CRTknzrPriv));
        if (a_input)
                cr_tknzr_set_input (result, a_input);
        return result;
}

CRTknzr *
cr_tknzr_new_from_buf (guchar * a_buf, gulong a_len,
                       enum CREncoding a_enc, 
                       gboolean a_free_at_destroy)
{
        CRTknzr *result = NULL;
        CRInput *input = NULL;

        input = cr_input_new_from_buf (a_buf, a_len, a_enc,
                                       a_free_at_destroy);

        g_return_val_if_fail (input != NULL, NULL);

        result = cr_tknzr_new (input);

        return result;
}

CRTknzr *
cr_tknzr_new_from_uri (const guchar * a_file_uri, 
                       enum CREncoding a_enc)
{
        CRTknzr *result = NULL;
        CRInput *input = NULL;

        input = cr_input_new_from_uri ((const gchar *) a_file_uri, a_enc);
        g_return_val_if_fail (input != NULL, NULL);

        result = cr_tknzr_new (input);

        return result;
}

void
cr_tknzr_ref (CRTknzr * a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this));

        PRIVATE (a_this)->ref_count++;
}

gboolean
cr_tknzr_unref (CRTknzr * a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), FALSE);

        if (PRIVATE (a_this)->ref_count > 0) {
                PRIVATE (a_this)->ref_count--;
        }

        if (PRIVATE (a_this)->ref_count == 0) {
                cr_tknzr_destroy (a_this);
                return TRUE;
        }

        return FALSE;
}

enum CRStatus
cr_tknzr_set_input (CRTknzr * a_this, CRInput * a_input)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->input) {
                cr_input_unref (PRIVATE (a_this)->input);
        }

        PRIVATE (a_this)->input = a_input;

        cr_input_ref (PRIVATE (a_this)->input);

        return CR_OK;
}

enum CRStatus
cr_tknzr_get_input (CRTknzr * a_this, CRInput ** a_input)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        *a_input = PRIVATE (a_this)->input;

        return CR_OK;
}

/*********************************
 *Tokenizer input handling routines
 *********************************/

/**
 *Reads the next byte from the parser input stream.
 *@param a_this the "this pointer" of the current instance of
 *#CRParser.
 *@param a_byte out parameter the place where to store the byte
 *read.
 *@return CR_OK upon successfull completion, an error 
 *code otherwise.
 */
enum CRStatus
cr_tknzr_read_byte (CRTknzr * a_this, guchar * a_byte)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this), CR_BAD_PARAM_ERROR);

        return cr_input_read_byte (PRIVATE (a_this)->input, a_byte);

}

/**
 *Reads the next char from the parser input stream.
 *@param a_this the current instance of #CRTknzr.
 *@param a_char out parameter. The read char.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_tknzr_read_char (CRTknzr * a_this, guint32 * a_char)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_char, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_read_char (PRIVATE (a_this)->input, a_char);
}

/**
 *Peeks a char from the parser input stream.
 *To "peek a char" means reads the next char without consuming it.
 *Subsequent calls to this function return the same char.
 *@param a_this the current instance of #CRTknzr.
 *@param a_char out parameter. The peeked char uppon successfull completion.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_tknzr_peek_char (CRTknzr * a_this, guint32 * a_char)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_char, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_peek_char (PRIVATE (a_this)->input, a_char);
}

/**
 *Peeks a byte ahead at a given postion in the parser input stream.
 *@param a_this the current instance of #CRTknzr.
 *@param a_offset the offset of the peeked byte starting from the current
 *byte in the parser input stream.
 *@param a_byte out parameter. The peeked byte upon 
 *successfull completion.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_tknzr_peek_byte (CRTknzr * a_this, gulong a_offset, guchar * a_byte)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input && a_byte,
                              CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_peek_byte (PRIVATE (a_this)->input,
                                   CR_SEEK_CUR, a_offset, a_byte);
}

/**
 *Same as cr_tknzr_peek_byte() but this api returns the byte peeked.
 *@param a_this the current instance of #CRTknzr.
 *@param a_offset the offset of the peeked byte starting from the current
 *byte in the parser input stream.
 *@param a_eof out parameter. If not NULL, is set to TRUE if we reached end of
 *file, FALE otherwise. If the caller sets it to NULL, this parameter 
 *is just ignored.
 *@return the peeked byte.
 */
guchar
cr_tknzr_peek_byte2 (CRTknzr * a_this, gulong a_offset, gboolean * a_eof)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, 0);

        return cr_input_peek_byte2 (PRIVATE (a_this)->input, a_offset, a_eof);
}

/**
 *Gets the number of bytes left in the topmost input stream
 *associated to this parser.
 *@param a_this the current instance of #CRTknzr
 *@return the number of bytes left or -1 in case of error.
 */
glong
cr_tknzr_get_nb_bytes_left (CRTknzr * a_this)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_get_nb_bytes_left (PRIVATE (a_this)->input);
}

enum CRStatus
cr_tknzr_get_cur_pos (CRTknzr * a_this, CRInputPos * a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_pos, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_get_cur_pos (PRIVATE (a_this)->input, a_pos);
}

enum CRStatus 
cr_tknzr_get_parsing_location (CRTknzr *a_this,
                               CRParsingLocation *a_loc)
{
        g_return_val_if_fail (a_this 
                              && PRIVATE (a_this)
                              && a_loc,
                              CR_BAD_PARAM_ERROR) ;

        return cr_input_get_parsing_location 
                (PRIVATE (a_this)->input, a_loc) ;
}

enum CRStatus
cr_tknzr_get_cur_byte_addr (CRTknzr * a_this, guchar ** a_addr)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, CR_BAD_PARAM_ERROR);
        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_get_cur_byte_addr (PRIVATE (a_this)->input, a_addr);
}

enum CRStatus
cr_tknzr_seek_index (CRTknzr * a_this, enum CRSeekPos a_origin, gint a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_seek_index (PRIVATE (a_this)->input, a_origin, a_pos);
}

enum CRStatus
cr_tknzr_consume_chars (CRTknzr * a_this, guint32 a_char, glong * a_nb_char)
{
	gulong consumed = *(gulong *) a_nb_char;
	enum CRStatus status;
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_input_set_cur_pos (PRIVATE (a_this)->input,
                                      &PRIVATE (a_this)->prev_pos);
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        status = cr_input_consume_chars (PRIVATE (a_this)->input,
                                         a_char, &consumed);
	*a_nb_char = (glong) consumed;
	return status;
}

enum CRStatus
cr_tknzr_set_cur_pos (CRTknzr * a_this, CRInputPos * a_pos)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        return cr_input_set_cur_pos (PRIVATE (a_this)->input, a_pos);
}

enum CRStatus
cr_tknzr_unget_token (CRTknzr * a_this, CRToken * a_token)
{
        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->token_cache == NULL,
                              CR_BAD_PARAM_ERROR);

        PRIVATE (a_this)->token_cache = a_token;

        return CR_OK;
}

/**
 *Returns the next token of the input stream.
 *This method is really central. Each parsing
 *method calls it.
 *@param a_this the current tokenizer.
 *@param a_tk out parameter. The returned token.
 *for the sake of mem leak avoidance, *a_tk must
 *be NULL.
 *@param CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_tknzr_get_next_token (CRTknzr * a_this, CRToken ** a_tk)
{
        enum CRStatus status = CR_OK;
        CRToken *token = NULL;
        CRInputPos init_pos;
        guint32 next_char = 0;
        guchar next_bytes[4] = { 0 };
        gboolean reached_eof = FALSE;
        CRInput *input = NULL;
        CRString *str = NULL;
        CRRgb *rgb = NULL;
        CRParsingLocation location = {0} ;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && a_tk && *a_tk == NULL
                              && PRIVATE (a_this)->input, 
                              CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->token_cache) {
                *a_tk = PRIVATE (a_this)->token_cache;
                PRIVATE (a_this)->token_cache = NULL;
                return CR_OK;
        }

        RECORD_INITIAL_POS (a_this, &init_pos);

        status = cr_input_get_end_of_file
                (PRIVATE (a_this)->input, &reached_eof);
        ENSURE_PARSING_COND (status == CR_OK);

        if (reached_eof == TRUE) {
                status = CR_END_OF_INPUT_ERROR;
                goto error;
        }

        input = PRIVATE (a_this)->input;

        PEEK_NEXT_CHAR (a_this, &next_char);
        token = cr_token_new ();
        ENSURE_PARSING_COND (token);

        switch (next_char) {
        case '@':
                {
                        if (BYTE (input, 2, NULL) == 'f'
                            && BYTE (input, 3, NULL) == 'o'
                            && BYTE (input, 4, NULL) == 'n'
                            && BYTE (input, 5, NULL) == 't'
                            && BYTE (input, 6, NULL) == '-'
                            && BYTE (input, 7, NULL) == 'f'
                            && BYTE (input, 8, NULL) == 'a'
                            && BYTE (input, 9, NULL) == 'c'
                            && BYTE (input, 10, NULL) == 'e') {
                                SKIP_CHARS (a_this, 1);
                                cr_tknzr_get_parsing_location 
                                        (a_this, &location) ;
                                SKIP_CHARS (a_this, 9);
                                status = cr_token_set_font_face_sym (token);
                                CHECK_PARSING_STATUS (status, TRUE);
                                cr_parsing_location_copy (&token->location,
                                                          &location) ;
                                goto done;
                        }

                        if (BYTE (input, 2, NULL) == 'c'
                            && BYTE (input, 3, NULL) == 'h'
                            && BYTE (input, 4, NULL) == 'a'
                            && BYTE (input, 5, NULL) == 'r'
                            && BYTE (input, 6, NULL) == 's'
                            && BYTE (input, 7, NULL) == 'e'
                            && BYTE (input, 8, NULL) == 't') {
                                SKIP_CHARS (a_this, 1);
                                cr_tknzr_get_parsing_location
                                        (a_this, &location) ;
                                SKIP_CHARS (a_this, 7);
                                status = cr_token_set_charset_sym (token);
                                CHECK_PARSING_STATUS (status, TRUE);
                                cr_parsing_location_copy (&token->location,
                                                          &location) ;
                                goto done;
                        }

                        if (BYTE (input, 2, NULL) == 'i'
                            && BYTE (input, 3, NULL) == 'm'
                            && BYTE (input, 4, NULL) == 'p'
                            && BYTE (input, 5, NULL) == 'o'
                            && BYTE (input, 6, NULL) == 'r'
                            && BYTE (input, 7, NULL) == 't') {
                                SKIP_CHARS (a_this, 1);
                                cr_tknzr_get_parsing_location 
                                        (a_this, &location) ;
                                SKIP_CHARS (a_this, 6);
                                status = cr_token_set_import_sym (token);
                                CHECK_PARSING_STATUS (status, TRUE);
                                cr_parsing_location_copy (&token->location,
                                                          &location) ;
                                goto done;
                        }

                        if (BYTE (input, 2, NULL) == 'm'
                            && BYTE (input, 3, NULL) == 'e'
                            && BYTE (input, 4, NULL) == 'd'
                            && BYTE (input, 5, NULL) == 'i'
                            && BYTE (input, 6, NULL) == 'a') {
                                SKIP_CHARS (a_this, 1);
                                cr_tknzr_get_parsing_location (a_this, 
                                                               &location) ;
                                SKIP_CHARS (a_this, 5);
                                status = cr_token_set_media_sym (token);
                                CHECK_PARSING_STATUS (status, TRUE);
                                cr_parsing_location_copy (&token->location, 
                                                          &location) ;
                                goto done;
                        }

                        if (BYTE (input, 2, NULL) == 'p'
                            && BYTE (input, 3, NULL) == 'a'
                            && BYTE (input, 4, NULL) == 'g'
                            && BYTE (input, 5, NULL) == 'e') {
                                SKIP_CHARS (a_this, 1);
                                cr_tknzr_get_parsing_location (a_this, 
                                                               &location) ;
                                SKIP_CHARS (a_this, 4);
                                status = cr_token_set_page_sym (token);
                                CHECK_PARSING_STATUS (status, TRUE);
                                cr_parsing_location_copy (&token->location, 
                                                          &location) ;
                                goto done;
                        }
                        status = cr_tknzr_parse_atkeyword (a_this, &str);
                        if (status == CR_OK) {
                                status = cr_token_set_atkeyword (token, str);
                                CHECK_PARSING_STATUS (status, TRUE);
                                if (str) {
                                        cr_parsing_location_copy (&token->location, 
                                                                  &str->location) ;
                                }
                                goto done;
                        }
                }
                break;

        case 'u':

                if (BYTE (input, 2, NULL) == 'r'
                    && BYTE (input, 3, NULL) == 'l'
                    && BYTE (input, 4, NULL) == '(') {
                        CRString *str2 = NULL;

                        status = cr_tknzr_parse_uri (a_this, &str2);
                        if (status == CR_OK) {
                                status = cr_token_set_uri (token, str2);
                                CHECK_PARSING_STATUS (status, TRUE);
                                if (str2) {
                                        cr_parsing_location_copy (&token->location,
                                                                  &str2->location) ;
                                }
                                goto done;
                        }
                } 
                goto fallback;
                break;

        case 'r':
                if (BYTE (input, 2, NULL) == 'g'
                    && BYTE (input, 3, NULL) == 'b'
                    && BYTE (input, 4, NULL) == '(') {
                        status = cr_tknzr_parse_rgb (a_this, &rgb);
                        if (status == CR_OK && rgb) {
                                status = cr_token_set_rgb (token, rgb);
                                CHECK_PARSING_STATUS (status, TRUE);
                                if (rgb) {
                                        cr_parsing_location_copy (&token->location, 
                                                                  &rgb->location) ;
                                }
                                rgb = NULL;
                                goto done;
                        }

                }
                goto fallback;
                break;

        case '<':
                if (BYTE (input, 2, NULL) == '!'
                    && BYTE (input, 3, NULL) == '-'
                    && BYTE (input, 4, NULL) == '-') {
                        SKIP_CHARS (a_this, 1);
                        cr_tknzr_get_parsing_location (a_this, 
                                                       &location) ;
                        SKIP_CHARS (a_this, 3);
                        status = cr_token_set_cdo (token);
                        CHECK_PARSING_STATUS (status, TRUE);
                        cr_parsing_location_copy (&token->location, 
                                                  &location) ;
                        goto done;
                }
                break;

        case '-':
                if (BYTE (input, 2, NULL) == '-'
                    && BYTE (input, 3, NULL) == '>') {
                        SKIP_CHARS (a_this, 1);
                        cr_tknzr_get_parsing_location (a_this, 
                                                       &location) ;
                        SKIP_CHARS (a_this, 2);
                        status = cr_token_set_cdc (token);
                        CHECK_PARSING_STATUS (status, TRUE);
                        cr_parsing_location_copy (&token->location, 
                                                  &location) ;
                        goto done;
                } else {
                        status = cr_tknzr_parse_ident
                                (a_this, &str);
                        if (status == CR_OK) {
                                cr_token_set_ident
                                        (token, str);
                                if (str) {
                                        cr_parsing_location_copy (&token->location, 
                                                                  &str->location) ;
                                }
                                goto done;
                        } else {
                                goto parse_number;
                        }
                }
                break;

        case '~':
                if (BYTE (input, 2, NULL) == '=') {
                        SKIP_CHARS (a_this, 1);
                        cr_tknzr_get_parsing_location (a_this, 
                                                       &location) ;
                        SKIP_CHARS (a_this, 1);
                        status = cr_token_set_includes (token);
                        CHECK_PARSING_STATUS (status, TRUE);
                        cr_parsing_location_copy (&token->location, 
                                                  &location) ;
                        goto done;
                }
                break;

        case '|':
                if (BYTE (input, 2, NULL) == '=') {
                        SKIP_CHARS (a_this, 1);
                        cr_tknzr_get_parsing_location (a_this, 
                                                       &location) ;
                        SKIP_CHARS (a_this, 1);
                        status = cr_token_set_dashmatch (token);
                        CHECK_PARSING_STATUS (status, TRUE);
                        cr_parsing_location_copy (&token->location,
                                                  &location) ;
                        goto done;
                }
                break;

        case '/':
                if (BYTE (input, 2, NULL) == '*') {
                        status = cr_tknzr_parse_comment (a_this, &str);

                        if (status == CR_OK) {
                                status = cr_token_set_comment (token, str);
                                str = NULL;
                                CHECK_PARSING_STATUS (status, TRUE);
                                if (str) {
                                        cr_parsing_location_copy (&token->location, 
                                                                  &str->location) ;
                                }
                                goto done;
                        }
                }
                break ;

        case ';':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_semicolon (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_parsing_location_copy (&token->location, 
                                          &location) ;
                goto done;

        case '{':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_cbo (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                goto done;

        case '}':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_cbc (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_parsing_location_copy (&token->location, 
                                          &location) ;
                goto done;

        case '(':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_po (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_parsing_location_copy (&token->location, 
                                          &location) ;
                goto done;

        case ')':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_pc (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_parsing_location_copy (&token->location, 
                                          &location) ;
                goto done;

        case '[':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_bo (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_parsing_location_copy (&token->location, 
                                          &location) ;
                goto done;

        case ']':
                SKIP_CHARS (a_this, 1);
                cr_tknzr_get_parsing_location (a_this, 
                                               &location) ;
                status = cr_token_set_bc (token);
                CHECK_PARSING_STATUS (status, TRUE);
                cr_parsing_location_copy (&token->location, 
                                          &location) ;
                goto done;

        case ' ':
        case '\t':
        case '\n':
        case '\f':
        case '\r':
                {
                        guchar *start = NULL,
                                *end = NULL;

                        status = cr_tknzr_parse_w (a_this, &start, 
                                                   &end, &location);
                        if (status == CR_OK) {
                                status = cr_token_set_s (token);
                                CHECK_PARSING_STATUS (status, TRUE);
                                cr_tknzr_get_parsing_location (a_this, 
                                                               &location) ;
                                goto done;
                        }
                }
                break;

        case '#':
                {
                        status = cr_tknzr_parse_hash (a_this, &str);
                        if (status == CR_OK && str) {
                                status = cr_token_set_hash (token, str);
                                CHECK_PARSING_STATUS (status, TRUE);
                                if (str) {
                                        cr_parsing_location_copy (&token->location,
                                                                  &str->location) ;
                                }
                                str = NULL;
                                goto done;
                        }
                }
                break;

        case '\'':
        case '"':
                status = cr_tknzr_parse_string (a_this, &str);
                if (status == CR_OK && str) {
                        status = cr_token_set_string (token, str);
                        CHECK_PARSING_STATUS (status, TRUE);
                        if (str) {
                                cr_parsing_location_copy (&token->location, 
                                                          &str->location) ;
                        }
                        str = NULL;
                        goto done;
                }
                break;

        case '!':
                status = cr_tknzr_parse_important (a_this, &location);
                if (status == CR_OK) {
                        status = cr_token_set_important_sym (token);
                        CHECK_PARSING_STATUS (status, TRUE);
                        cr_parsing_location_copy (&token->location, 
                                                  &location) ;
                        goto done;
                }
                break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
        case '+':
        /* '-' case is handled separately above for --> comments */
        parse_number:
                {
                        CRNum *num = NULL;

                        status = cr_tknzr_parse_num (a_this, &num);
                        if (status == CR_OK && num) {
                                next_bytes[0] = BYTE (input, 1, NULL);
                                next_bytes[1] = BYTE (input, 2, NULL);
                                next_bytes[2] = BYTE (input, 3, NULL);
                                next_bytes[3] = BYTE (input, 4, NULL);

                                if (next_bytes[0] == 'e'
                                    && next_bytes[1] == 'm') {
                                        num->type = NUM_LENGTH_EM;
                                        status = cr_token_set_ems (token,
                                                                   num);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'e'
                                           && next_bytes[1] == 'x') {
                                        num->type = NUM_LENGTH_EX;
                                        status = cr_token_set_exs (token,
                                                                   num);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'p'
                                           && next_bytes[1] == 'x') {
                                        num->type = NUM_LENGTH_PX;
                                        status = cr_token_set_length
                                                (token, num, LENGTH_PX_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'c'
                                           && next_bytes[1] == 'm') {
                                        num->type = NUM_LENGTH_CM;
                                        status = cr_token_set_length
                                                (token, num, LENGTH_CM_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'm'
                                           && next_bytes[1] == 'm') {
                                        num->type = NUM_LENGTH_MM;
                                        status = cr_token_set_length
                                                (token, num, LENGTH_MM_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'i'
                                           && next_bytes[1] == 'n') {
                                        num->type = NUM_LENGTH_IN;
                                        status = cr_token_set_length
                                                (token, num, LENGTH_IN_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'p'
                                           && next_bytes[1] == 't') {
                                        num->type = NUM_LENGTH_PT;
                                        status = cr_token_set_length
                                                (token, num, LENGTH_PT_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'p'
                                           && next_bytes[1] == 'c') {
                                        num->type = NUM_LENGTH_PC;
                                        status = cr_token_set_length
                                                (token, num, LENGTH_PC_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'd'
                                           && next_bytes[1] == 'e'
                                           && next_bytes[2] == 'g') {
                                        num->type = NUM_ANGLE_DEG;
                                        status = cr_token_set_angle
                                                (token, num, ANGLE_DEG_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 3);
                                } else if (next_bytes[0] == 'r'
                                           && next_bytes[1] == 'a'
                                           && next_bytes[2] == 'd') {
                                        num->type = NUM_ANGLE_RAD;
                                        status = cr_token_set_angle
                                                (token, num, ANGLE_RAD_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 3);
                                } else if (next_bytes[0] == 'g'
                                           && next_bytes[1] == 'r'
                                           && next_bytes[2] == 'a'
                                           && next_bytes[3] == 'd') {
                                        num->type = NUM_ANGLE_GRAD;
                                        status = cr_token_set_angle
                                                (token, num, ANGLE_GRAD_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 4);
                                } else if (next_bytes[0] == 'm'
                                           && next_bytes[1] == 's') {
                                        num->type = NUM_TIME_MS;
                                        status = cr_token_set_time
                                                (token, num, TIME_MS_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 's') {
                                        num->type = NUM_TIME_S;
                                        status = cr_token_set_time
                                                (token, num, TIME_S_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 1);
                                } else if (next_bytes[0] == 'H'
                                           && next_bytes[1] == 'z') {
                                        num->type = NUM_FREQ_HZ;
                                        status = cr_token_set_freq
                                                (token, num, FREQ_HZ_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 2);
                                } else if (next_bytes[0] == 'k'
                                           && next_bytes[1] == 'H'
                                           && next_bytes[2] == 'z') {
                                        num->type = NUM_FREQ_KHZ;
                                        status = cr_token_set_freq
                                                (token, num, FREQ_KHZ_ET);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 3);
                                } else if (next_bytes[0] == '%') {
                                        num->type = NUM_PERCENTAGE;
                                        status = cr_token_set_percentage
                                                (token, num);
                                        num = NULL;
                                        SKIP_CHARS (a_this, 1);
                                } else {
                                        status = cr_tknzr_parse_ident (a_this,
                                                                       &str);
                                        if (status == CR_OK && str) {
                                                num->type = NUM_UNKNOWN_TYPE;
                                                status = cr_token_set_dimen
                                                        (token, num, str);
                                                num = NULL;
                                                CHECK_PARSING_STATUS (status,
                                                                      TRUE);
                                                str = NULL;
                                        } else {
                                                status = cr_token_set_number
                                                        (token, num);
                                                num = NULL;
                                                CHECK_PARSING_STATUS (status, CR_OK);
                                                str = NULL;
                                        }
                                }
                                if (token && token->u.num) {
                                        cr_parsing_location_copy (&token->location,
                                                                  &token->u.num->location) ;
                                } else {
                                        status = CR_ERROR ;
                                }
                                goto done ;
                        }
                }
                break;

        default:
        fallback:
                /*process the fallback cases here */

                if (next_char == '\\'
                    || (cr_utils_is_nonascii (next_bytes[0]) == TRUE)
                    || ((next_char >= 'a') && (next_char <= 'z'))
                    || ((next_char >= 'A') && (next_char <= 'Z'))) {
                        status = cr_tknzr_parse_ident (a_this, &str);
                        if (status == CR_OK && str) {
                                guint32 next_c = 0;

                                status = cr_input_peek_char
                                        (PRIVATE (a_this)->input, &next_c);

                                if (status == CR_OK && next_c == '(') {

                                        SKIP_CHARS (a_this, 1);
                                        status = cr_token_set_function
                                                (token, str);
                                        CHECK_PARSING_STATUS (status, TRUE);
                                        /*ownership is transfered
                                         *to token by cr_token_set_function.
                                         */
                                        if (str) {
                                                cr_parsing_location_copy (&token->location, 
                                                                          &str->location) ;
                                        }
                                        str = NULL;
                                } else {
                                        status = cr_token_set_ident (token,
                                                                     str);
                                        CHECK_PARSING_STATUS (status, TRUE);
                                        if (str) {
                                                cr_parsing_location_copy (&token->location, 
                                                                          &str->location) ;
                                        }
                                        str = NULL;
                                }
                                goto done;
                        } else {
                                if (str) {
                                        cr_string_destroy (str);
                                        str = NULL;
                                }
                        }
                }
                break;
        }

        READ_NEXT_CHAR (a_this, &next_char);
        cr_tknzr_get_parsing_location (a_this, 
                                       &location) ;
        status = cr_token_set_delim (token, next_char);
        CHECK_PARSING_STATUS (status, TRUE);
        cr_parsing_location_copy (&token->location, 
                                  &location) ;
 done:

        if (status == CR_OK && token) {
                *a_tk = token;
                /*
                 *store the previous position input stream pos.
                 */
                memmove (&PRIVATE (a_this)->prev_pos,
                         &init_pos, sizeof (CRInputPos));
                return CR_OK;
        }

 error:
        if (token) {
                cr_token_destroy (token);
                token = NULL;
        }

        if (str) {
                cr_string_destroy (str);
                str = NULL;
        }
        cr_tknzr_set_cur_pos (a_this, &init_pos);
        return status;

}

enum CRStatus
cr_tknzr_parse_token (CRTknzr * a_this, enum CRTokenType a_type,
                      enum CRTokenExtraType a_et, gpointer a_res,
                      gpointer a_extra_res)
{
        enum CRStatus status = CR_OK;
        CRToken *token = NULL;

        g_return_val_if_fail (a_this && PRIVATE (a_this)
                              && PRIVATE (a_this)->input
                              && a_res, CR_BAD_PARAM_ERROR);

        status = cr_tknzr_get_next_token (a_this, &token);
        if (status != CR_OK)
                return status;
        if (token == NULL)
                return CR_PARSING_ERROR;

        if (token->type == a_type) {
                switch (a_type) {
                case NO_TK:
                case S_TK:
                case CDO_TK:
                case CDC_TK:
                case INCLUDES_TK:
                case DASHMATCH_TK:
                case IMPORT_SYM_TK:
                case PAGE_SYM_TK:
                case MEDIA_SYM_TK:
                case FONT_FACE_SYM_TK:
                case CHARSET_SYM_TK:
                case IMPORTANT_SYM_TK:
                        status = CR_OK;
                        break;

                case STRING_TK:
                case IDENT_TK:
                case HASH_TK:
                case ATKEYWORD_TK:
                case FUNCTION_TK:
                case COMMENT_TK:
                case URI_TK:
                        *((CRString **) a_res) = token->u.str;
                        token->u.str = NULL;
                        status = CR_OK;
                        break;

                case EMS_TK:
                case EXS_TK:
                case PERCENTAGE_TK:
                case NUMBER_TK:
                        *((CRNum **) a_res) = token->u.num;
                        token->u.num = NULL;
                        status = CR_OK;
                        break;

                case LENGTH_TK:
                case ANGLE_TK:
                case TIME_TK:
                case FREQ_TK:
                        if (token->extra_type == a_et) {
                                *((CRNum **) a_res) = token->u.num;
                                token->u.num = NULL;
                                status = CR_OK;
                        }
                        break;

                case DIMEN_TK:
                        *((CRNum **) a_res) = token->u.num;
                        if (a_extra_res == NULL) {
                                status = CR_BAD_PARAM_ERROR;
                                goto error;
                        }

                        *((CRString **) a_extra_res) = token->dimen;
                        token->u.num = NULL;
                        token->dimen = NULL;
                        status = CR_OK;
                        break;

                case DELIM_TK:
                        *((guint32 *) a_res) = token->u.unichar;
                        status = CR_OK;
                        break;

                case UNICODERANGE_TK:
                default:
                        status = CR_PARSING_ERROR;
                        break;
                }

                cr_token_destroy (token);
                token = NULL;
        } else {
                cr_tknzr_unget_token (a_this, token);
                token = NULL;
                status = CR_PARSING_ERROR;
        }

        return status;

      error:

        if (token) {
                cr_tknzr_unget_token (a_this, token);
                token = NULL;
        }

        return status;
}

void
cr_tknzr_destroy (CRTknzr * a_this)
{
        g_return_if_fail (a_this);

        if (PRIVATE (a_this) && PRIVATE (a_this)->input) {
                if (cr_input_unref (PRIVATE (a_this)->input)
                    == TRUE) {
                        PRIVATE (a_this)->input = NULL;
                }
        }

        if (PRIVATE (a_this)->token_cache) {
                cr_token_destroy (PRIVATE (a_this)->token_cache);
                PRIVATE (a_this)->token_cache = NULL;
        }

        if (PRIVATE (a_this)) {
                g_free (PRIVATE (a_this));
                PRIVATE (a_this) = NULL;
        }

        g_free (a_this);
}
