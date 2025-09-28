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
 *@CRNum:
 *
 *The definition
 *of the #CRNum class.
 */

#include <config.h>
#include "cr-num.h"
#include "string.h"

/**
 * cr_num_new:
 *
 *#CRNum.
 *
 *Returns the newly built instance of
 *#CRNum.
 */
CRNum *
cr_num_new (void)
{
        CRNum *result = NULL;

        result = g_try_malloc (sizeof (CRNum));

        if (result == NULL) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRNum));

        return result;
}

/**
 * cr_num_new_with_val:
 * @a_val: the numerical value of the number.
 * @a_type: the type of number.
 * 
 * A constructor of #CRNum.
 *
 * Returns the newly built instance of #CRNum or
 * NULL if an error arises.
 */
CRNum *
cr_num_new_with_val (gdouble a_val, enum CRNumType a_type)
{
        CRNum *result = NULL;

        result = cr_num_new ();

        g_return_val_if_fail (result, NULL);

        result->val = a_val;
        result->type = a_type;

        return result;
}

/**
 * cr_num_to_string:
 *@a_this: the current instance of #CRNum.
 *
 *Returns the newly built string representation
 *of the current instance of #CRNum. The returned
 *string is NULL terminated. The caller *must*
 *free the returned string.
 */
guchar *
cr_num_to_string (CRNum const * a_this)
{
        gdouble test_val = 0.0;

        guchar *tmp_char1 = NULL,
                *tmp_char2 = NULL,
                *result = NULL;

        g_return_val_if_fail (a_this, NULL);

        test_val = a_this->val - (glong) a_this->val;

        if (!test_val) {
                tmp_char1 = (guchar *) g_strdup_printf ("%ld", (glong) a_this->val);
        } else {
                tmp_char1 = (guchar *) g_new0 (char, G_ASCII_DTOSTR_BUF_SIZE + 1);
                if (tmp_char1 != NULL)
                        g_ascii_dtostr ((gchar *) tmp_char1, G_ASCII_DTOSTR_BUF_SIZE, a_this->val);
        }

        g_return_val_if_fail (tmp_char1, NULL);

        switch (a_this->type) {
        case NUM_LENGTH_EM:
                tmp_char2 = (guchar *) "em";
                break;

        case NUM_LENGTH_EX:
                tmp_char2 = (guchar *) "ex";
                break;

        case NUM_LENGTH_PX:
                tmp_char2 = (guchar *) "px";
                break;

        case NUM_LENGTH_IN:
                tmp_char2 = (guchar *) "in";
                break;

        case NUM_LENGTH_CM:
                tmp_char2 = (guchar *) "cm";
                break;

        case NUM_LENGTH_MM:
                tmp_char2 = (guchar *) "mm";
                break;

        case NUM_LENGTH_PT:
                tmp_char2 = (guchar *) "pt";
                break;

        case NUM_LENGTH_PC:
                tmp_char2 = (guchar *) "pc";
                break;

        case NUM_ANGLE_DEG:
                tmp_char2 = (guchar *) "deg";
                break;

        case NUM_ANGLE_RAD:
                tmp_char2 = (guchar *) "rad";
                break;

        case NUM_ANGLE_GRAD:
                tmp_char2 = (guchar *) "grad";
                break;

        case NUM_TIME_MS:
                tmp_char2 = (guchar *) "ms";
                break;

        case NUM_TIME_S:
                tmp_char2 = (guchar *) "s";
                break;

        case NUM_FREQ_HZ:
                tmp_char2 = (guchar *) "Hz";
                break;

        case NUM_FREQ_KHZ:
                tmp_char2 = (guchar *) "KHz";
                break;

        case NUM_PERCENTAGE:
                tmp_char2 = (guchar *) "%";
                break;
        case NUM_INHERIT:
                tmp_char2 = (guchar *) "inherit";
                break ;
        case NUM_AUTO:
                tmp_char2 = (guchar *) "auto";
                break ;
        case NUM_GENERIC:
                tmp_char2 = NULL ;
                break ;
        default:
                tmp_char2 = (guchar *) "unknown";
                break;
        }

        if (tmp_char2) {
                result = (guchar *)  g_strconcat ((gchar *) tmp_char1, tmp_char2, NULL);
                g_free (tmp_char1);
        } else {
                result = tmp_char1;
        }

        return result;
}

/**
 * cr_num_copy:
 *@a_src: the instance of #CRNum to copy.
 *Must be non NULL.
 *@a_dest: the destination of the copy.
 *Must be non NULL
 *
 *Copies an instance of #CRNum.
 *
 *Returns CR_OK upon successful completion, an
 *error code otherwise.
 */
enum CRStatus
cr_num_copy (CRNum * a_dest, CRNum const * a_src)
{
        g_return_val_if_fail (a_dest && a_src, CR_BAD_PARAM_ERROR);

        memcpy (a_dest, a_src, sizeof (CRNum));

        return CR_OK;
}

/**
 * cr_num_dup:
 *@a_this: the instance of #CRNum to duplicate.
 *
 *Duplicates an instance of #CRNum
 *
 *Returns the newly created (duplicated) instance of #CRNum.
 *Must be freed by cr_num_destroy().
 */
CRNum *
cr_num_dup (CRNum const * a_this)
{
        CRNum *result = NULL;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_this, NULL);

        result = cr_num_new ();
        g_return_val_if_fail (result, NULL);

        status = cr_num_copy (result, a_this);
        g_return_val_if_fail (status == CR_OK, NULL);

        return result;
}

/**
 * cr_num_set:
 *Sets an instance of #CRNum.
 *@a_this: the current instance of #CRNum to be set.
 *@a_val: the new numerical value to be hold by the current
 *instance of #CRNum
 *@a_type: the new type of #CRNum.
 *
 * Returns CR_OK upon succesful completion, an error code otherwise.
 */
enum CRStatus
cr_num_set (CRNum * a_this, gdouble a_val, enum CRNumType a_type)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        a_this->val = a_val;
        a_this->type = a_type;

        return CR_OK;
}

/**
 * cr_num_is_fixed_length:
 * @a_this: the current instance of #CRNum .
 *
 *Tests if the current instance of #CRNum is a fixed
 *length value or not. Typically a fixed length value
 *is anything from NUM_LENGTH_EM to NUM_LENGTH_PC.
 *See the definition of #CRNumType to see what we mean.
 *
 *Returns TRUE if the instance of #CRNum is a fixed length number,
 *FALSE otherwise.
 */
gboolean
cr_num_is_fixed_length (CRNum const * a_this)
{
        gboolean result = FALSE;

        g_return_val_if_fail (a_this, FALSE);

        if (a_this->type >= NUM_LENGTH_EM 
            && a_this->type <= NUM_LENGTH_PC) {
                result = TRUE ;
        }
        return result ;
}

/**
 * cr_num_destroy:
 *@a_this: the this pointer of
 *the current instance of #CRNum.
 *
 *The destructor of #CRNum.
 */
void
cr_num_destroy (CRNum * a_this)
{
        g_return_if_fail (a_this);

        g_free (a_this);
}
