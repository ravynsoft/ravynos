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
 * modify it under the terms of version 2.1 of 
 * the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include "cr-fonts.h"
#include <string.h>

static enum CRStatus
cr_font_family_to_string_real (CRFontFamily const * a_this,
                               gboolean a_walk_list, GString ** a_string)
{
        guchar const *name = NULL;
        enum CRStatus result = CR_OK;

        if (!*a_string) {
                *a_string = g_string_new (NULL);
                g_return_val_if_fail (*a_string,
                                      CR_INSTANCIATION_FAILED_ERROR);
        }

        if (!a_this) {
                g_string_append (*a_string, "NULL");
                return CR_OK;
        }

        switch (a_this->type) {
        case FONT_FAMILY_SANS_SERIF:
                name = (guchar const *) "sans-serif";
                break;

        case FONT_FAMILY_SERIF:
                name = (guchar const *) "sans-serif";
                break;

        case FONT_FAMILY_CURSIVE:
                name = (guchar const *) "cursive";
                break;

        case FONT_FAMILY_FANTASY:
                name = (guchar const *) "fantasy";
                break;

        case FONT_FAMILY_MONOSPACE:
                name = (guchar const *) "monospace";
                break;

        case FONT_FAMILY_NON_GENERIC:
                name = (guchar const *) a_this->name;
                break;

        default:
                name = NULL;
                break;
        }

        if (name) {
                if (a_this->prev) {
                        g_string_append_printf (*a_string, ", %s", name);
                } else {
                        g_string_append (*a_string, (const gchar *) name);
                }
        }
        if (a_walk_list == TRUE && a_this->next) {
                result = cr_font_family_to_string_real (a_this->next,
                                                        TRUE, a_string);
        }
        return result;
}

static const gchar *
cr_predefined_absolute_font_size_to_string (enum CRPredefinedAbsoluteFontSize
                                            a_code)
{
        gchar const *str = NULL;

        switch (a_code) {
        case FONT_SIZE_XX_SMALL:
                str = "xx-small";
                break;
        case FONT_SIZE_X_SMALL:
                str = "x-small";
                break;
        case FONT_SIZE_SMALL:
                str = "small";
                break;
        case FONT_SIZE_MEDIUM:
                str = "medium";
                break;
        case FONT_SIZE_LARGE:
                str = "large";
                break;
        case FONT_SIZE_X_LARGE:
                str = "x-large";
                break;
        case FONT_SIZE_XX_LARGE:
                str = "xx-large";
                break;
        default:
                str = "unknown absolute font size value";
        }
        return str;
}

static const gchar *
cr_relative_font_size_to_string (enum CRRelativeFontSize a_code)
{
        gchar const *str = NULL;

        switch (a_code) {
        case FONT_SIZE_LARGER:
                str = "larger";
                break;
        case FONT_SIZE_SMALLER:
                str = "smaller";
                break;
        default:
                str = "unknown relative font size value";
                break;
        }
        return str;
}

/**
 * cr_font_family_new:
 * @a_type: the type of font family to create.
 * @a_name: the name of the font family.
 *
 * create a font family.
 *
 * Returns the newly built font family.
 */
CRFontFamily *
cr_font_family_new (enum CRFontFamilyType a_type, guchar * a_name)
{
        CRFontFamily *result = NULL;

        result = g_try_malloc (sizeof (CRFontFamily));

        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRFontFamily));
        result->type = a_type;

        cr_font_family_set_name (result, a_name);

        return result;
}

/**
 * cr_font_family_to_string:
 * @a_this: the current instance of #CRFontFamily.
 * @a_walk_font_family_list: wether the serialize the entire list.
 *
 * Returns the seriliazed font family. The caller has to free it using
 * g_free().
 */
guchar *
cr_font_family_to_string (CRFontFamily const * a_this,
                          gboolean a_walk_font_family_list)
{
        enum CRStatus status = CR_OK;
        guchar *result = NULL;
        GString *stringue = NULL;

        if (!a_this) {
                result = (guchar *) g_strdup ("NULL");
                g_return_val_if_fail (result, NULL);
                return result;
        }
        status = cr_font_family_to_string_real (a_this,
                                                a_walk_font_family_list,
                                                &stringue);

        if (status == CR_OK && stringue) {
                result = (guchar *) stringue->str;
                g_string_free (stringue, FALSE);
                stringue = NULL;

        } else {
                if (stringue) {
                        g_string_free (stringue, TRUE);
                        stringue = NULL;
                }
        }

        return result;
}

/**
 * cr_font_family_set_name:
 * @a_this: the current instance of #CRFontFamily.
 * @a_name: the new name
 *
 * Returns CR_OK upon sucessful completion, an error code otherwise.
 */
enum CRStatus
cr_font_family_set_name (CRFontFamily * a_this, guchar * a_name)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        /*
         *only non generic font families can have a name
         */

        if (a_this->type != FONT_FAMILY_NON_GENERIC) {
                return CR_BAD_PARAM_ERROR;
        }

        if (a_this->name) {
                g_free (a_this->name);
                a_this->name = NULL;
        }

        a_this->name = a_name;
        return CR_OK;
}

/**
 * cr_font_family_append:
 * @a_this: the current instance of #CRFontFamily.
 * @a_family_to_append: the font family to append to the list
 *
 * Returns the new font family list.
 */
CRFontFamily *
cr_font_family_append (CRFontFamily * a_this,
                       CRFontFamily * a_family_to_append)
{
        CRFontFamily *cur_ff = NULL;

        g_return_val_if_fail (a_family_to_append, NULL);

        if (!a_this)
                return a_family_to_append;

        for (cur_ff = a_this; cur_ff && cur_ff->next; cur_ff = cur_ff->next) ;

        cur_ff->next = a_family_to_append;
        a_family_to_append->prev = cur_ff;

        return a_this;

}

/**
 * cr_font_family_prepend:
 * @a_this: the current instance #CRFontFamily.
 * @a_family_to_prepend: the font family to prepend to the list.
 *
 * Returns the font family list.
 */
CRFontFamily *
cr_font_family_prepend (CRFontFamily * a_this,
                        CRFontFamily * a_family_to_prepend)
{
        g_return_val_if_fail (a_this && a_family_to_prepend, NULL);

        if (!a_this)
                return a_family_to_prepend;

        a_family_to_prepend->next = a_this;
        a_this->prev = a_family_to_prepend;

        return a_family_to_prepend;
}

/**
 * cr_font_family_destroy:
 * @a_this: the current instance of #CRFontFamily.
 *
 * Returns CR_OK upon sucessful completion, an error code otherwise.
 */
enum CRStatus
cr_font_family_destroy (CRFontFamily * a_this)
{
        CRFontFamily *cur_ff = NULL;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        for (cur_ff = a_this; cur_ff && cur_ff->next; cur_ff = cur_ff->next) ;

        for (; cur_ff; cur_ff = cur_ff->prev) {
                if (a_this->name) {
                        g_free (a_this->name);
                        a_this->name = NULL;
                }

                if (cur_ff->next) {
                        g_free (cur_ff->next);

                }

                if (cur_ff->prev == NULL) {
                        g_free (a_this);
                }
        }

        return CR_OK;
}

/***************************************************
 *'font-size' manipulation functions definitions
 ***************************************************/

/**
 * cr_font_size_new:
 *
 * Returns the newly created font size.
 */
CRFontSize *
cr_font_size_new (void)
{
        CRFontSize *result = NULL;

        result = g_try_malloc (sizeof (CRFontSize));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRFontSize));

        return result;
}

/**
 * cr_font_size_clear:
 * @a_this: the current instance of #CRFontSize
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_font_size_clear (CRFontSize * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        switch (a_this->type) {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
        case RELATIVE_FONT_SIZE:
        case INHERITED_FONT_SIZE:
                memset (a_this, 0, sizeof (CRFontSize));
                break;

        case ABSOLUTE_FONT_SIZE:
                memset (a_this, 0, sizeof (CRFontSize));
                break;

        default:
                return CR_UNKNOWN_TYPE_ERROR;
        }

        return CR_OK;
}

/**
 * cr_font_size_copy:
 * @a_dst: the destination #CRFontSize (where to copy to).
 * @a_src: the source #CRFontSize (where to copy from).
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_font_size_copy (CRFontSize * a_dst, CRFontSize const * a_src)
{
        g_return_val_if_fail (a_dst && a_src, CR_BAD_PARAM_ERROR);

        switch (a_src->type) {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
        case RELATIVE_FONT_SIZE:
        case INHERITED_FONT_SIZE:
                cr_font_size_clear (a_dst);
                memcpy (a_dst, a_src, sizeof (CRFontSize));
                break;

        case ABSOLUTE_FONT_SIZE:
                cr_font_size_clear (a_dst);
                cr_num_copy (&a_dst->value.absolute,
                             &a_src->value.absolute);
                a_dst->type = a_src->type;
                break;

        default:
                return CR_UNKNOWN_TYPE_ERROR;
        }
        return CR_OK;
}

/**
 * cr_font_size_set_predefined_absolute_font_size:
 * @a_this: the current instance of #CRFontSize.
 * @a_predefined: what to set.
 *
 * Returns CR_OK upon sucessful completion, an error code otherwise.
 */
enum CRStatus 
cr_font_size_set_predefined_absolute_font_size (CRFontSize *a_this, 
                                                enum CRPredefinedAbsoluteFontSize a_predefined)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;
        g_return_val_if_fail (a_predefined >= FONT_SIZE_XX_SMALL
                              && a_predefined < NB_PREDEFINED_ABSOLUTE_FONT_SIZES,
                              CR_BAD_PARAM_ERROR) ;

        a_this->type = PREDEFINED_ABSOLUTE_FONT_SIZE ;
        a_this->value.predefined = a_predefined ;

        return CR_OK ;
}

/**
 * cr_font_size_set_relative_font_size:
 * @a_this: the current instance of #CRFontSize
 * @a_relative: the new relative font size
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus 
cr_font_size_set_relative_font_size (CRFontSize *a_this,
                                     enum CRRelativeFontSize a_relative)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;
        g_return_val_if_fail (a_relative >= FONT_SIZE_LARGER
                              && a_relative < NB_RELATIVE_FONT_SIZE,
                              CR_BAD_PARAM_ERROR) ;
        
        a_this->type = RELATIVE_FONT_SIZE ;
        a_this->value.relative = a_relative ;
        return CR_OK ;
}

/**
 * cr_font_size_set_absolute_font_size:
 * @a_this: the current instance of #CRFontSize
 * @a_num_type: the type of number to set.
 * @a_value: the actual value to set.
 *
 * Returns CR_OK upon succesful completion, an error code otherwise.
 */
enum CRStatus 
cr_font_size_set_absolute_font_size (CRFontSize *a_this,
                                     enum CRNumType a_num_type,
                                     gdouble a_value)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;
        g_return_val_if_fail (a_num_type >= NUM_AUTO
                              && a_num_type < NB_NUM_TYPE,
                              CR_BAD_PARAM_ERROR) ;

        a_this->type = ABSOLUTE_FONT_SIZE ;
        cr_num_set (&a_this->value.absolute,
                    a_value, a_num_type) ;        
        return CR_OK ;
}

/**
 * cr_font_size_set_to_inherit:
 * @a_this: the current instance of #CRFontSize 
 *
 * Returns CR_OK upon succesful completion, an error code otherwise.
 */
enum CRStatus
cr_font_size_set_to_inherit (CRFontSize *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        cr_font_size_clear (a_this) ;
        a_this->type = INHERITED_FONT_SIZE ;

        return CR_OK ;
}

/**
 * cr_font_size_is_set_to_inherit:
 * @a_this: the current instance of #CRFontSize.
 *
 * Returns TRUE if the current instance is set to 'inherit'. 
 */
gboolean
cr_font_size_is_set_to_inherit (CRFontSize const *a_this)
{
        g_return_val_if_fail (a_this, FALSE) ;

        return a_this->type == INHERITED_FONT_SIZE ;
}

/**
 * cr_font_size_to_string:
 * @a_this: the current instance of #CRFontSize
 *
 * Returns the serialized form of #CRFontSize. The returned string
 * has to bee freed using g_free().
 */
gchar *
cr_font_size_to_string (CRFontSize const * a_this)
{
        gchar *str = NULL;

        if (!a_this) {
                str = g_strdup ("NULL");
                g_return_val_if_fail (str, NULL);
                return str;
        }
        switch (a_this->type) {
        case PREDEFINED_ABSOLUTE_FONT_SIZE:
                str = g_strdup (cr_predefined_absolute_font_size_to_string
                                (a_this->value.predefined));
                break;
        case ABSOLUTE_FONT_SIZE:
                str = (gchar *) cr_num_to_string (&a_this->value.absolute);
                break;
        case RELATIVE_FONT_SIZE:
                str = g_strdup (cr_relative_font_size_to_string
                                (a_this->value.relative));
                break;
        case INHERITED_FONT_SIZE:
                str = g_strdup ("inherit");
                break;
        default:
                break;
        }
        return str;
}

/**
 * cr_font_size_get_smaller_predefined:
 * @a_font_size: the font size to consider.
 * @a_smaller_size: out parameter. The a smaller value than @a_font_size. 
 */
void 
cr_font_size_get_smaller_predefined_font_size 
				(enum CRPredefinedAbsoluteFontSize a_font_size,
			         enum CRPredefinedAbsoluteFontSize *a_smaller_size)
{
        enum CRPredefinedAbsoluteFontSize result = FONT_SIZE_MEDIUM ;

        g_return_if_fail (a_smaller_size) ;
        g_return_if_fail (a_font_size < NB_PREDEFINED_ABSOLUTE_FONT_SIZES
                          && a_font_size >= FONT_SIZE_XX_SMALL) ;

        switch (a_font_size) {
        case FONT_SIZE_XX_SMALL:
                result =  FONT_SIZE_XX_SMALL ;
                break ;
        case FONT_SIZE_X_SMALL:
                result =  FONT_SIZE_XX_SMALL ;
                break ;
        case FONT_SIZE_SMALL:
                result =  FONT_SIZE_X_SMALL;
                break ;
        case FONT_SIZE_MEDIUM:
                result =  FONT_SIZE_SMALL;
                break ;
        case FONT_SIZE_LARGE:
                result =  FONT_SIZE_MEDIUM;
                break ;
        case FONT_SIZE_X_LARGE:
                result =  FONT_SIZE_LARGE;
                break ;
        case FONT_SIZE_XX_LARGE:
                result =  FONT_SIZE_XX_LARGE;
                break ;
	case FONT_SIZE_INHERIT:
                cr_utils_trace_info ("can't return a smaller size for FONT_SIZE_INHERIT") ;                
                result =  FONT_SIZE_MEDIUM ;
                break ;
        default:
                cr_utils_trace_info ("Unknown FONT_SIZE") ;
                result = FONT_SIZE_MEDIUM ;
                break ;
        }
        *a_smaller_size = result ;
}


/**
 * cr_font_size_get_larger_predefined_font_size:
 * @a_font_size: the font size to consider.
 * @a_larger_size: out parameter. the font size considered larger than
 * @a_font_size.
 *
 */
void 
cr_font_size_get_larger_predefined_font_size 
			(enum CRPredefinedAbsoluteFontSize a_font_size,
		         enum CRPredefinedAbsoluteFontSize *a_larger_size)
{
        enum CRPredefinedAbsoluteFontSize result = FONT_SIZE_MEDIUM ;
        
        g_return_if_fail (a_larger_size) ;
        g_return_if_fail (a_font_size >= FONT_SIZE_XX_SMALL 
                          && a_font_size < NB_PREDEFINED_ABSOLUTE_FONT_SIZES) ;

        switch (a_font_size) {
        case FONT_SIZE_XX_SMALL:
                result =  FONT_SIZE_X_SMALL ;
                break ;
        case FONT_SIZE_X_SMALL:
                result =  FONT_SIZE_SMALL ;
                break ;
        case FONT_SIZE_SMALL:
                result =  FONT_SIZE_MEDIUM;
                break ;
        case FONT_SIZE_MEDIUM:
                result =  FONT_SIZE_LARGE;
                break ;
        case FONT_SIZE_LARGE:
                result =  FONT_SIZE_X_LARGE;
                break ;
        case FONT_SIZE_X_LARGE:
                result =  FONT_SIZE_XX_LARGE ;
                break ;
        case FONT_SIZE_XX_LARGE:
                result =  FONT_SIZE_XX_LARGE;
                break ;
	case FONT_SIZE_INHERIT:
                cr_utils_trace_info ("can't return a bigger size for FONT_SIZE_INHERIT") ;                
                result =  FONT_SIZE_MEDIUM ;
                break ;
        default:
                cr_utils_trace_info ("Unknown FONT_SIZE") ;
                result = FONT_SIZE_MEDIUM ;
                break ;
        }
        *a_larger_size = result ;
}

/**
 * cr_font_size_is_predefined_absolute_font_size:
 * @a_font_size: the font size to consider.
 *
 * Returns TRUE if the instance is an predefined absolute font size, FALSE
 * otherwise.
 */
gboolean
cr_font_size_is_predefined_absolute_font_size 
				(enum CRPredefinedAbsoluteFontSize a_font_size)
{
        if (a_font_size >= FONT_SIZE_XX_SMALL
            && a_font_size < NB_PREDEFINED_ABSOLUTE_FONT_SIZES) {
                return TRUE ;
        } else {
                return FALSE ;
        }
}

/**
 * cr_font_size_adjust_to_string:
 * @a_this: the instance of #CRFontSizeAdjust.
 *
 * Returns the serialized form of #CRFontSizeAdjust
 */
gchar *
cr_font_size_adjust_to_string (CRFontSizeAdjust const * a_this)
{
        gchar *str = NULL;

        if (!a_this) {
                str = g_strdup ("NULL");
                g_return_val_if_fail (str, NULL);
                return str;
        }

        switch (a_this->type) {
        case FONT_SIZE_ADJUST_NONE:
                str = g_strdup ("none");
                break;
        case FONT_SIZE_ADJUST_NUMBER:
                if (a_this->num)
                        str = (gchar *) cr_num_to_string (a_this->num);
                else
                        str = g_strdup ("unknown font-size-adjust property value"); /* Should raise an error no?*/
                break;
        case FONT_SIZE_ADJUST_INHERIT:
                str = g_strdup ("inherit");
        }
        return str;
}

/**
 * cr_font_style_to_string:
 * @a_code: the current instance of #CRFontStyle .
 *
 * Returns the serialized #CRFontStyle. The caller must free the returned
 * string using g_free().
 */
const gchar *
cr_font_style_to_string (enum CRFontStyle a_code)
{
        gchar *str = NULL;

        switch (a_code) {
        case FONT_STYLE_NORMAL:
                str = (gchar *) "normal";
                break;
        case FONT_STYLE_ITALIC:
                str = (gchar *) "italic";
                break;
        case FONT_STYLE_OBLIQUE:
                str = (gchar *) "oblique";
                break;
        case FONT_STYLE_INHERIT:
                str = (gchar *) "inherit";
                break;
        default:
                str = (gchar *) "unknown font style value";
                break;
        }
        return str;
}

/**
 * cr_font_variant_to_string:
 * @a_code: the current instance of #CRFontVariant.
 *
 * Returns the serialized form of #CRFontVariant. The caller has
 * to free the returned string using g_free().
 */
const gchar *
cr_font_variant_to_string (enum CRFontVariant a_code)
{
        gchar *str = NULL;

        switch (a_code) {
        case FONT_VARIANT_NORMAL:
                str = (gchar *) "normal";
                break;
        case FONT_VARIANT_SMALL_CAPS:
                str = (gchar *) "small-caps";
                break;
        case FONT_VARIANT_INHERIT:
                str = (gchar *) "inherit";
                break;
        }
        return str;
}

/**
 * cr_font_weight_get_bolder:
 * @a_weight: the #CRFontWeight to consider.
 *
 * Returns a font weight bolder than @a_weight
 */
enum CRFontWeight
cr_font_weight_get_bolder (enum CRFontWeight a_weight)
{
        if (a_weight == FONT_WEIGHT_INHERIT) {
                cr_utils_trace_info ("can't return a bolder weight for FONT_WEIGHT_INHERIT") ;
                return a_weight;
        } else if (a_weight >= FONT_WEIGHT_900) {
                return FONT_WEIGHT_900 ;
        } else if (a_weight < FONT_WEIGHT_NORMAL) {
                return FONT_WEIGHT_NORMAL ;
        } else if (a_weight == FONT_WEIGHT_BOLDER
                   || a_weight == FONT_WEIGHT_LIGHTER) {
                cr_utils_trace_info ("FONT_WEIGHT_BOLDER or FONT_WEIGHT_LIGHTER should not appear here") ;
                return FONT_WEIGHT_NORMAL ;
        } else {
                return a_weight << 1 ;
        }
}

/**
 * cr_font_weight_to_string:
 * @a_code: the font weight to consider.
 *
 * Returns the serialized form of #CRFontWeight.
 */
const gchar *
cr_font_weight_to_string (enum CRFontWeight a_code)
{
        gchar *str = NULL;

        switch (a_code) {
        case FONT_WEIGHT_NORMAL:
                str = (gchar *) "normal";
                break;
        case FONT_WEIGHT_BOLD:
                str = (gchar *) "bold";
                break;
        case FONT_WEIGHT_BOLDER:
                str = (gchar *) "bolder";
                break;
        case FONT_WEIGHT_LIGHTER:
                str = (gchar *) "lighter";
                break;
        case FONT_WEIGHT_100:
                str = (gchar *) "100";
                break;
        case FONT_WEIGHT_200:
                str = (gchar *) "200";
                break;
        case FONT_WEIGHT_300:
                str = (gchar *) "300";
                break;
        case FONT_WEIGHT_400:
                str = (gchar *) "400";
                break;
        case FONT_WEIGHT_500:
                str = (gchar *) "500";
                break;
        case FONT_WEIGHT_600:
                str = (gchar *) "600";
                break;
        case FONT_WEIGHT_700:
                str = (gchar *) "700";
                break;
        case FONT_WEIGHT_800:
                str = (gchar *) "800";
                break;
        case FONT_WEIGHT_900:
                str = (gchar *) "900";
                break;
        case FONT_WEIGHT_INHERIT:
                str = (gchar *) "inherit";
                break;
        default:
                str = (gchar *) "unknown font-weight property value";
                break;
        }
        return str;
}

/**
 * cr_font_stretch_to_string:
 * @a_code: the instance of #CRFontStretch to consider.
 *
 * Returns the serialized form of #CRFontStretch.
 */
const gchar *
cr_font_stretch_to_string (enum CRFontStretch a_code)
{
        gchar *str = NULL;

        switch (a_code) {
        case FONT_STRETCH_NORMAL:
                str = (gchar *) "normal";
                break;
        case FONT_STRETCH_WIDER:
                str = (gchar *) "wider";
                break;
        case FONT_STRETCH_NARROWER:
                str = (gchar *) "narrower";
                break;
        case FONT_STRETCH_ULTRA_CONDENSED:
                str = (gchar *) "ultra-condensed";
                break;
        case FONT_STRETCH_EXTRA_CONDENSED:
                str = (gchar *) "extra-condensed";
                break;
        case FONT_STRETCH_CONDENSED:
                str = (gchar *) "condensed";
                break;
        case FONT_STRETCH_SEMI_CONDENSED:
                str = (gchar *) "semi-condensed";
                break;
        case FONT_STRETCH_SEMI_EXPANDED:
                str = (gchar *) "semi-expanded";
                break;
        case FONT_STRETCH_EXPANDED:
                str = (gchar *) "expanded";
                break;
        case FONT_STRETCH_EXTRA_EXPANDED:
                str = (gchar *) "extra-expaned";
                break;
        case FONT_STRETCH_ULTRA_EXPANDED:
                str = (gchar *) "ultra-expanded";
                break;
        case FONT_STRETCH_INHERIT:
                str = (gchar *) "inherit";
                break;
        }
        return str;
}

/**
 * cr_font_size_destroy:
 * @a_font_size: the font size to destroy
 *
 */
void
cr_font_size_destroy (CRFontSize * a_font_size)
{
        g_return_if_fail (a_font_size);

        g_free (a_font_size) ;
}

/*******************************************************
 *'font-size-adjust' manipulation function definition
 *******************************************************/

/**
 * cr_font_size_adjust_new:
 *
 * Returns a newly built instance of #CRFontSizeAdjust
 */
CRFontSizeAdjust *
cr_font_size_adjust_new (void)
{
        CRFontSizeAdjust *result = NULL;

        result = g_try_malloc (sizeof (CRFontSizeAdjust));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRFontSizeAdjust));

        return result;
}

/**
 * cr_font_size_adjust_destroy:
 * @a_this: the current instance of #CRFontSizeAdjust.
 *
 */
void
cr_font_size_adjust_destroy (CRFontSizeAdjust * a_this)
{
        g_return_if_fail (a_this);

        if (a_this->type == FONT_SIZE_ADJUST_NUMBER && a_this->num) {
                cr_num_destroy (a_this->num);
                a_this->num = NULL;
        }
}
