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
 *The definition of the #CRToken class.
 *Abstracts a css2 token.
 */

#include <config.h>
#include <string.h>
#include "cr-token.h"

/*
 *TODO: write a CRToken::to_string() method.
 */

/**
 *Frees the attributes of the current instance
 *of #CRtoken.
 *@param a_this the current instance of #CRToken.
 */
static void
cr_token_clear (CRToken * a_this)
{
        g_return_if_fail (a_this);

        switch (a_this->type) {
        case S_TK:
        case CDO_TK:
        case CDC_TK:
        case INCLUDES_TK:
        case DASHMATCH_TK:
        case PAGE_SYM_TK:
        case MEDIA_SYM_TK:
        case FONT_FACE_SYM_TK:
        case CHARSET_SYM_TK:
        case IMPORT_SYM_TK:
        case IMPORTANT_SYM_TK:
        case SEMICOLON_TK:
        case NO_TK:
        case DELIM_TK:
        case CBO_TK:
        case CBC_TK:
        case BO_TK:
        case BC_TK:
                break;

        case STRING_TK:
        case IDENT_TK:
        case HASH_TK:
        case URI_TK:
        case FUNCTION_TK:
        case COMMENT_TK:
        case ATKEYWORD_TK:
                if (a_this->u.str) {
                        cr_string_destroy (a_this->u.str);
                        a_this->u.str = NULL;
                }
                break;

        case EMS_TK:
        case EXS_TK:
        case LENGTH_TK:
        case ANGLE_TK:
        case TIME_TK:
        case FREQ_TK:
        case PERCENTAGE_TK:
        case NUMBER_TK:
        case PO_TK:
        case PC_TK:
                if (a_this->u.num) {
                        cr_num_destroy (a_this->u.num);
                        a_this->u.num = NULL;
                }
                break;

        case DIMEN_TK:
                if (a_this->u.num) {
                        cr_num_destroy (a_this->u.num);
                        a_this->u.num = NULL;
                }

                if (a_this->dimen) {
                        cr_string_destroy (a_this->dimen);
                        a_this->dimen = NULL;
                }

                break;

        case RGB_TK:
                if (a_this->u.rgb) {
                        cr_rgb_destroy (a_this->u.rgb) ;
                        a_this->u.rgb = NULL ;
                }
                break ;

        case UNICODERANGE_TK:
                /*not supported yet. */
                break;

        default:
                cr_utils_trace_info ("I don't know how to clear this token\n") ;
                break;
        }

        a_this->type = NO_TK;
}

/**
 *Default constructor of
 *the #CRToken class.
 *@return the newly built instance of #CRToken.
 */
CRToken *
cr_token_new (void)
{
        CRToken *result = NULL;

        result = g_try_malloc (sizeof (CRToken));

        if (result == NULL) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRToken));

        return result;
}

/**
 *Sets the type of curren instance of
 *#CRToken to 'S_TK' (S in the css2 spec)
 *@param a_this the current instance of #CRToken.
 *@return CR_OK upon successfull completion, an error
 *code otherwise.
 */
enum CRStatus
cr_token_set_s (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = S_TK;

        return CR_OK;
}

/**
 *Sets the type of the current instance of
 *#CRToken to 'CDO_TK' (CDO as said by the css2 spec)
 *@param a_this the current instance of #CRToken.
 *@return CR_OK upon successfull completion, an error
 *code otherwise.
 */
enum CRStatus
cr_token_set_cdo (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = CDO_TK;

        return CR_OK;
}

/**
 *Sets the type of the current token to
 *CDC_TK (CDC as said by the css2 spec).
 *@param a_this the current instance of #CRToken.
 *@return CR_OK upon successfull completion, an error
 *code otherwise.
 */
enum CRStatus
cr_token_set_cdc (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = CDC_TK;

        return CR_OK;
}

/**
 *Sets the type of the current instance of
 *#CRToken to INCLUDES_TK (INCLUDES as said by the css2 spec).
 *@param a_this the current instance of #CRToken.
 *@return CR_OK upon successfull completion, an error
 *code otherwise.
 */
enum CRStatus
cr_token_set_includes (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = INCLUDES_TK;

        return CR_OK;
}

/**
 *Sets the type of the current instance of
 *#CRToken to DASHMATCH_TK (DASHMATCH as said by the css2 spec).
 *@param a_this the current instance of #CRToken.
 *@return CR_OK upon successfull completion, an error
 *code otherwise.
 */
enum CRStatus
cr_token_set_dashmatch (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = DASHMATCH_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_comment (CRToken * a_this, CRString * a_str)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = COMMENT_TK;
        a_this->u.str = a_str ;
        return CR_OK;
}

enum CRStatus
cr_token_set_string (CRToken * a_this, CRString * a_str)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = STRING_TK;

        a_this->u.str = a_str ;

        return CR_OK;
}

enum CRStatus
cr_token_set_ident (CRToken * a_this, CRString * a_ident)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = IDENT_TK;
        a_this->u.str = a_ident;
        return CR_OK;
}


enum CRStatus
cr_token_set_function (CRToken * a_this, CRString * a_fun_name)
{
        g_return_val_if_fail (a_this,
                              CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = FUNCTION_TK;
        a_this->u.str  = a_fun_name;
        return CR_OK;
}

enum CRStatus
cr_token_set_hash (CRToken * a_this, CRString * a_hash)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = HASH_TK;
        a_this->u.str = a_hash;

        return CR_OK;
}

enum CRStatus
cr_token_set_rgb (CRToken * a_this, CRRgb * a_rgb)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = RGB_TK;
        a_this->u.rgb = a_rgb;

        return CR_OK;
}

enum CRStatus
cr_token_set_import_sym (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = IMPORT_SYM_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_page_sym (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = PAGE_SYM_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_media_sym (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = MEDIA_SYM_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_font_face_sym (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = FONT_FACE_SYM_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_charset_sym (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = CHARSET_SYM_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_atkeyword (CRToken * a_this, CRString * a_atname)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);
        a_this->type = ATKEYWORD_TK;
        a_this->u.str = a_atname;
        return CR_OK;
}

enum CRStatus
cr_token_set_important_sym (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
        cr_token_clear (a_this);
        a_this->type = IMPORTANT_SYM_TK;
        return CR_OK;
}

enum CRStatus
cr_token_set_ems (CRToken * a_this, CRNum * a_num)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
        cr_token_clear (a_this);
        a_this->type = EMS_TK;
        a_this->u.num = a_num;
        return CR_OK;
}

enum CRStatus
cr_token_set_exs (CRToken * a_this, CRNum * a_num)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
        cr_token_clear (a_this);
        a_this->type = EXS_TK;
        a_this->u.num = a_num;
        return CR_OK;
}

enum CRStatus
cr_token_set_length (CRToken * a_this, CRNum * a_num,
                     enum CRTokenExtraType a_et)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = LENGTH_TK;
        a_this->extra_type = a_et;
        a_this->u.num = a_num;

        return CR_OK;
}

enum CRStatus
cr_token_set_angle (CRToken * a_this, CRNum * a_num,
                    enum CRTokenExtraType a_et)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = ANGLE_TK;
        a_this->extra_type = a_et;
        a_this->u.num = a_num;

        return CR_OK;
}

enum CRStatus
cr_token_set_time (CRToken * a_this, CRNum * a_num,
                   enum CRTokenExtraType a_et)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = TIME_TK;
        a_this->extra_type = a_et;
        a_this->u.num = a_num;

        return CR_OK;
}

enum CRStatus
cr_token_set_freq (CRToken * a_this, CRNum * a_num,
                   enum CRTokenExtraType a_et)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = FREQ_TK;
        a_this->extra_type = a_et;
        a_this->u.num = a_num;

        return CR_OK;
}

enum CRStatus
cr_token_set_dimen (CRToken * a_this, CRNum * a_num, 
                    CRString * a_dim)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
        cr_token_clear (a_this);
        a_this->type = DIMEN_TK;
        a_this->u.num = a_num;
        a_this->dimen = a_dim;
        return CR_OK;

}

enum CRStatus
cr_token_set_percentage (CRToken * a_this, CRNum * a_num)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = PERCENTAGE_TK;
        a_this->u.num = a_num;

        return CR_OK;
}

enum CRStatus
cr_token_set_number (CRToken * a_this, CRNum * a_num)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = NUMBER_TK;
        a_this->u.num = a_num;
        return CR_OK;
}

enum CRStatus
cr_token_set_uri (CRToken * a_this, CRString * a_uri)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = URI_TK;
        a_this->u.str = a_uri;

        return CR_OK;
}

enum CRStatus
cr_token_set_delim (CRToken * a_this, guint32 a_char)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = DELIM_TK;
        a_this->u.unichar = a_char;

        return CR_OK;
}

enum CRStatus
cr_token_set_semicolon (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = SEMICOLON_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_cbo (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = CBO_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_cbc (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = CBC_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_po (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = PO_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_pc (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = PC_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_bo (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = BO_TK;

        return CR_OK;
}

enum CRStatus
cr_token_set_bc (CRToken * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        cr_token_clear (a_this);

        a_this->type = BC_TK;

        return CR_OK;
}

/**
 *The destructor of the #CRToken class.
 *@param a_this the current instance of #CRToken.
 */
void
cr_token_destroy (CRToken * a_this)
{
        g_return_if_fail (a_this);

        cr_token_clear (a_this);

        g_free (a_this);
}
