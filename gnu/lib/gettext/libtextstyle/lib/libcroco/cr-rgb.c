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

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cr-rgb.h"
#include "cr-term.h"
#include "cr-parser.h"

static const CRRgb gv_standard_colors[] = {
        {(const guchar*)"aliceblue",   240, 248, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"antiquewhite",        250, 235, 215, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"aqua",          0, 255, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"aquamarine",  127, 255, 212, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"azure",       240, 255, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"beige",       245, 245, 220, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"bisque",      255, 228, 196, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"black",         0,   0,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"blanchedalmond",      255, 235, 205, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"blue",          0,   0, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"blueviolet",  138,  43, 226, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"brown",       165,  42,  42, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"burlywood",   222, 184, 135, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"cadetblue",    95, 158, 160, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"chartreuse",  127, 255,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"chocolate",   210, 105,  30, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"coral",       255, 127,  80, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"cornflowerblue",      100, 149, 237, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"cornsilk",    255, 248, 220, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"crimson",     220,  20,  60, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"cyan",          0, 255, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkblue",      0,   0, 139, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkcyan",      0, 139, 139, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkgoldenrod",       184, 134,  11, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkgray",    169, 169, 169, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkgreen",     0, 100,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkgrey",    169, 169, 169, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkkhaki",   189, 183, 107, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkmagenta", 139,   0, 139, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkolivegreen",       85, 107,  47, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkorange",  255, 140,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkorchid",  153,  50, 204, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkred",     139,   0,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darksalmon",  233, 150, 122, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkseagreen",        143, 188, 143, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkslateblue",        72,  61, 139, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkslategray",        47,  79,  79, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkslategrey",        47,  79,  79, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkturquoise",         0, 206, 209, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"darkviolet",  148,   0, 211, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"deeppink",    255,  20, 147, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"deepskyblue",   0, 191, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"dimgray",     105, 105, 105, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"dimgrey",     105, 105, 105, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"dodgerblue",   30, 144, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"firebrick",   178,  34,  34, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"floralwhite", 255, 250, 240, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"forestgreen",  34, 139,  34, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"fuchsia",     255,   0, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"gainsboro",   220, 220, 220, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"ghostwhite",  248, 248, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"gold",        255, 215,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"goldenrod",   218, 165,  32, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"gray",        128, 128, 128, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"green",         0, 128,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"greenyellow", 173, 255,  47, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"grey",        128, 128, 128, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"honeydew",    240, 255, 240, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"hotpink",     255, 105, 180, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"indianred",   205,  92,  92, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"indigo",       75,   0, 130, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"ivory",       255, 255, 240, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"khaki",       240, 230, 140, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lavender",    230, 230, 250, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lavenderblush",       255, 240, 245, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lawngreen",   124, 252,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lemonchiffon",        255, 250, 205, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightblue",   173, 216, 230, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightcoral",  240, 128, 128, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightcyan",   224, 255, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightgoldenrodyellow",        250, 250, 210, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightgray",   211, 211, 211, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightgreen",  144, 238, 144, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightgrey",   211, 211, 211, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightpink",   255, 182, 193, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightsalmon", 255, 160, 122, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightseagreen",        32, 178, 170, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightskyblue",        135, 206, 250, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightslategray",      119, 136, 153, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightslategrey",      119, 136, 153, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightsteelblue",      176, 196, 222, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lightyellow", 255, 255, 224, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"lime",          0, 255,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"limegreen",    50, 205,  50, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"linen",       250, 240, 230, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"magenta",     255,   0, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"maroon",      128,   0,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumaquamarine",    102, 205, 170, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumblue",    0,   0, 205, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumorchid",        186,  85, 211, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumpurple",        147, 112, 219, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumseagreen",       60, 179, 113, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumslateblue",     123, 104, 238, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumspringgreen",     0, 250, 154, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumturquoise",      72, 209, 204, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mediumvioletred",     199,  21, 133, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"midnightblue",         25,  25, 112, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mintcream",   245, 255, 250, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"mistyrose",   255, 228, 225, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"moccasin",    255, 228, 181, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"navajowhite", 255, 222, 173, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"navy",          0,   0, 128, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"oldlace",     253, 245, 230, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"olive",       128, 128,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"olivedrab",   107, 142,  35, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"orange",      255, 165,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"orangered",   255,  69,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"orchid",      218, 112, 214, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"palegoldenrod",       238, 232, 170, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"palegreen",   152, 251, 152, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"paleturquoise",       175, 238, 238, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"palevioletred",       219, 112, 147, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"papayawhip",  255, 239, 213, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"peachpuff",   255, 218, 185, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"peru",        205, 133,  63, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"pink",        255, 192, 203, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"plum",        221, 160, 221, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"powderblue",  176, 224, 230, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"purple",      128,   0, 128, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"red",         255,   0,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"rosybrown",   188, 143, 143, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"royalblue",    65, 105, 225, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"saddlebrown", 139,  69,  19, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"salmon",      250, 128, 114, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"sandybrown",  244, 164,  96, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"seagreen",     46, 139,  87, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"seashell",    255, 245, 238, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"sienna",      160,  82,  45, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"silver",      192, 192, 192, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"skyblue",     135, 206, 235, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"slateblue",   106,  90, 205, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"slategray",   112, 128, 144, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"slategrey",   112, 128, 144, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"snow",        255, 250, 250, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"springgreen",   0, 255, 127, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"steelblue",    70, 130, 180, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"tan",         210, 180, 140, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"teal",          0, 128, 128, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"thistle",     216, 191, 216, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"tomato",      255,  99,  71, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"transparent", 255, 255, 255, FALSE, FALSE, TRUE, {0,0,0}},
        {(const guchar*)"turquoise",    64, 224, 208, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"violet",      238, 130, 238, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"wheat",       245, 222, 179, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"white",       255, 255, 255, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"whitesmoke",  245, 245, 245, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"yellow",      255, 255,   0, FALSE, FALSE, FALSE, {0,0,0}},
        {(const guchar*)"yellowgreen", 154, 205,  50, FALSE, FALSE, FALSE, {0,0,0}}
};

/**
 * cr_rgb_new:
 *
 *The default constructor of #CRRgb.
 *
 *Returns the newly built instance of #CRRgb
 */
CRRgb *
cr_rgb_new (void)
{
        CRRgb *result = NULL;

        result = g_try_malloc (sizeof (CRRgb));

        if (result == NULL) {
                cr_utils_trace_info ("No more memory");
                return NULL;
        }

        memset (result, 0, sizeof (CRRgb));

        return result;
}

/**
 * cr_rgb_new_with_vals:
 *@a_red: the red component of the color.
 *@a_green: the green component of the color.
 *@a_blue: the blue component of the color.
 *@a_unit: the unit of the rgb values.
 *(either percentage or integer values)
 *
 *A constructor of #CRRgb.
 *
 *Returns the newly built instance of #CRRgb.
 */
CRRgb *
cr_rgb_new_with_vals (gulong a_red, gulong a_green,
                      gulong a_blue, gboolean a_is_percentage)
{
        CRRgb *result = NULL;

        result = cr_rgb_new ();

        g_return_val_if_fail (result, NULL);

        result->red = a_red;
        result->green = a_green;
        result->blue = a_blue;
        result->is_percentage = a_is_percentage;

        return result;
}

/**
 * cr_rgb_to_string:
 *@a_this: the instance of #CRRgb to serialize.
 *
 *Serializes the rgb into a zero terminated string.
 *
 *Returns the zero terminated string containing the serialized
 *rgb. MUST BE FREED by the caller using g_free().
 */
guchar *
cr_rgb_to_string (CRRgb const * a_this)
{
        guchar *result = NULL;
        GString *str_buf = NULL;

        str_buf = g_string_new (NULL);
        g_return_val_if_fail (str_buf, NULL);

        if (a_this->is_percentage == 1) {
                g_string_append_printf (str_buf, "%ld", a_this->red);

                g_string_append (str_buf, "%, ");

                g_string_append_printf (str_buf, "%ld", a_this->green);
                g_string_append (str_buf, "%, ");

                g_string_append_printf (str_buf, "%ld", a_this->blue);
                g_string_append_c (str_buf, '%');
        } else {
                g_string_append_printf (str_buf, "%ld", a_this->red);
                g_string_append (str_buf, ", ");

                g_string_append_printf (str_buf, "%ld", a_this->green);
                g_string_append (str_buf, ", ");

                g_string_append_printf (str_buf, "%ld", a_this->blue);
        }

        if (str_buf) {
                result = (guchar *) str_buf->str;
                g_string_free (str_buf, FALSE);
        }

        return result;
}

/**
 * cr_rgb_dump:
 *@a_this: the "this pointer" of
 *the current instance of #CRRgb.
 *@a_fp: the destination file pointer.
 *
 *Dumps the current instance of #CRRgb
 *to a file.
 */
void
cr_rgb_dump (CRRgb const * a_this, FILE * a_fp)
{
        guchar *str = NULL;

        g_return_if_fail (a_this);

        str = cr_rgb_to_string (a_this);

        if (str) {
                fprintf (a_fp, "%s", str);
                g_free (str);
                str = NULL;
        }
}

/**
 * cr_rgb_compute_from_percentage:
 *@a_this: the current instance of #CRRgb
 *
 *If the rgb values are expressed in percentage,
 *compute their real value.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_rgb_compute_from_percentage (CRRgb * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        if (a_this->is_percentage == FALSE)
                return CR_OK;
        a_this->red = a_this->red * 255 / 100;
        a_this->green = a_this->green * 255 / 100;
        a_this->blue = a_this->blue * 255 / 100;
        a_this->is_percentage = FALSE;
        return CR_OK;
}

/**
 * cr_rgb_set:
 *@a_this: the current instance of #CRRgb.
 *@a_red: the red value.
 *@a_green: the green value.
 *@a_blue: the blue value.
 *
 *Sets rgb values to the RGB.
 *
 *Returns CR_OK upon successful completion, an error code
 *otherwise.
 */
enum CRStatus
cr_rgb_set (CRRgb * a_this, gulong a_red,
            gulong a_green, gulong a_blue, gboolean a_is_percentage)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
        if (a_is_percentage != FALSE) {
                g_return_val_if_fail (a_red <= 100
                                      && a_green <= 100
                                      && a_blue <= 100, CR_BAD_PARAM_ERROR);
        }

        a_this->is_percentage = a_is_percentage;

        a_this->red = a_red;
        a_this->green = a_green;
        a_this->blue = a_blue;
        a_this->inherit = FALSE ;
        a_this->is_transparent = FALSE ;
        return CR_OK;
}

/**
 * cr_rgb_set_to_inherit:
 *@a_this: the current instance of #CRRgb
 *
 *sets the value of the rgb to inherit.
 *Look at the css spec from chapter 6.1 to 6.2 to understand
 *the meaning of "inherit".
 *
 * Returns CR_OK upon succesful completion, an error code otherwise.
 */
enum CRStatus 
cr_rgb_set_to_inherit (CRRgb *a_this, gboolean a_inherit)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        a_this->inherit = a_inherit ;

        return CR_OK ;
}

/**
 * cr_rgb_is_set_to_inherit:
 *
 * @a_this: the current instance of #CRRgb.
 *
 * Returns TRUE if the rgb is set to the value "inherit", FALSE otherwise.
 */
gboolean
cr_rgb_is_set_to_inherit (CRRgb const *a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

        return a_this->inherit ;
}

/**
 * cr_rgb_is_set_to_transparent:
 *@a_this: the current instance of
 *#CRRgb
 *
 *Tests if the the rgb is set to the
 *value "transparent" or not.
 *
 *Returns TRUE if the rgb has been set to
 *transparent, FALSE otherwise.
 */
gboolean 
cr_rgb_is_set_to_transparent (CRRgb const *a_this)
{
        g_return_val_if_fail (a_this, FALSE) ;
        return a_this->is_transparent ;
}


/**
 * cr_rgb_set_to_transparent:
 *@a_this: the current instance of #CRRgb
 *@a_is_transparent: set to transparent or not.
 *
 *Sets the rgb to the "transparent" value (or not)
 *Returns CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus 
cr_rgb_set_to_transparent (CRRgb *a_this, 
                           gboolean a_is_transparent)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;        
        a_this->is_transparent = a_is_transparent ;
        return CR_OK ;
}

/**
 * cr_rgb_set_from_rgb:
 *@a_this: the current instance of #CRRgb.
 *@a_rgb: the rgb to "copy"
 *
 *Sets the rgb from an other one.
 *
 *Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_rgb_set_from_rgb (CRRgb * a_this, CRRgb const * a_rgb)
{
        g_return_val_if_fail (a_this && a_rgb, CR_BAD_PARAM_ERROR);

        cr_rgb_copy (a_this, a_rgb) ;

        return CR_OK;
}

static int
cr_rgb_color_name_compare (const void *a,
                           const void *b)
{
        const char *a_color_name = a;
        const CRRgb *rgb = b;

        return g_ascii_strcasecmp (a_color_name, (const char *) rgb->name);
}

/**
 * cr_rgb_set_from_name:
 * @a_this: the current instance of #CRRgb
 * @a_color_name: the color name
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_rgb_set_from_name (CRRgb * a_this, const guchar * a_color_name)
{
        enum CRStatus status = CR_OK;
        CRRgb *result;

        g_return_val_if_fail (a_this && a_color_name, CR_BAD_PARAM_ERROR);

        result = bsearch (a_color_name,
                          gv_standard_colors,
                          G_N_ELEMENTS (gv_standard_colors),
                          sizeof (gv_standard_colors[0]),
                          cr_rgb_color_name_compare);
        if (result != NULL)
                cr_rgb_set_from_rgb (a_this, result);
        else
               status = CR_UNKNOWN_TYPE_ERROR;

        return status;
}

/**
 * cr_rgb_set_from_hex_str:
 * @a_this: the current instance of #CRRgb
 * @a_hex: the hexadecimal value to set.
 *
 * Returns CR_OK upon successful completion.
 */
enum CRStatus
cr_rgb_set_from_hex_str (CRRgb * a_this, const guchar * a_hex)
{
        enum CRStatus status = CR_OK;
        gulong i = 0;
        guchar colors[3] = { 0 };

        g_return_val_if_fail (a_this && a_hex, CR_BAD_PARAM_ERROR);

        if (strlen ((const char *) a_hex) == 3) {
                for (i = 0; i < 3; i++) {
                        if (a_hex[i] >= '0' && a_hex[i] <= '9') {
                                colors[i] = a_hex[i] - '0';
                                colors[i] = (colors[i] << 4) | colors[i];
                        } else if (a_hex[i] >= 'a' && a_hex[i] <= 'z') {
                                colors[i] = 10 + a_hex[i] - 'a';
                                colors[i] = (colors[i] << 4) | colors[i];
                        } else if (a_hex[i] >= 'A' && a_hex[i] <= 'Z') {
                                colors[i] = 10 + a_hex[i] - 'A';
                                colors[i] = (colors[i] << 4) | colors[i];
                        } else {
                                status = CR_UNKNOWN_TYPE_ERROR;
                        }
                }
        } else if (strlen ((const char *) a_hex) == 6) {
                for (i = 0; i < 6; i++) {
                        if (a_hex[i] >= '0' && a_hex[i] <= '9') {
                                colors[i / 2] <<= 4;
                                colors[i / 2] |= a_hex[i] - '0';
                                status = CR_OK;
                        } else if (a_hex[i] >= 'a' && a_hex[i] <= 'z') {
                                colors[i / 2] <<= 4;
                                colors[i / 2] |= 10 + a_hex[i] - 'a';
                                status = CR_OK;
                        } else if (a_hex[i] >= 'A' && a_hex[i] <= 'Z') {
                                colors[i / 2] <<= 4;
                                colors[i / 2] |= 10 + a_hex[i] - 'A';
                                status = CR_OK;
                        } else {
                                status = CR_UNKNOWN_TYPE_ERROR;
                        }
                }
        } else {
                status = CR_UNKNOWN_TYPE_ERROR;
        }

        if (status == CR_OK) {
                status = cr_rgb_set (a_this, colors[0],
                                     colors[1], colors[2], FALSE);
                cr_rgb_set_to_transparent (a_this, FALSE) ;
        }
        return status;
}

/**
 * cr_rgb_set_from_term:
 *@a_this: the instance of #CRRgb to set
 *@a_value: the terminal from which to set
 *
 *Set the rgb from a terminal symbol
 *
 * Returns CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_rgb_set_from_term (CRRgb *a_this, const struct _CRTerm *a_value)
{
        enum CRStatus status = CR_OK ;
        g_return_val_if_fail (a_this && a_value,
                              CR_BAD_PARAM_ERROR) ;

	switch(a_value->type) {
	case TERM_RGB:
                if (a_value->content.rgb) {
                        cr_rgb_set_from_rgb
                                (a_this, a_value->content.rgb) ;
                }
		break ;
	case TERM_IDENT:
                if (a_value->content.str
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
			if (!strncmp ("inherit",
                                      a_value->content.str->stryng->str,
                                      sizeof ("inherit")-1)) {
				a_this->inherit = TRUE;
                                a_this->is_transparent = FALSE ;
			} else  {
                        	status = cr_rgb_set_from_name
                                        (a_this,
                                         (const guchar *) a_value->content.str->stryng->str) ;
			}
                } else {
                        cr_utils_trace_info 
                                ("a_value has NULL string value") ;
                }
		break ;
	case TERM_HASH:
                if (a_value->content.str
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
                        status = cr_rgb_set_from_hex_str
                                (a_this, 
                                 (const guchar *) a_value->content.str->stryng->str) ;
                } else {
                        cr_utils_trace_info
                                ("a_value has NULL string value") ;
                }
                break ;
	default:
                status =  CR_UNKNOWN_TYPE_ERROR ;
	}
        return status ;
}

enum CRStatus 
cr_rgb_copy (CRRgb *a_dest, CRRgb const *a_src)
{
        g_return_val_if_fail (a_dest && a_src,
                              CR_BAD_PARAM_ERROR) ;

        memcpy (a_dest, a_src, sizeof (CRRgb)) ;
        return CR_OK ;
}

/**
 * cr_rgb_destroy:
 *@a_this: the "this pointer" of the
 *current instance of #CRRgb.
 *
 *Destructor of #CRRgb.
 */
void
cr_rgb_destroy (CRRgb * a_this)
{
        g_return_if_fail (a_this);
        g_free (a_this);
}

/**
 * cr_rgb_parse_from_buf:
 *@a_str: a string that contains a color description
 *@a_enc: the encoding of a_str
 *
 *Parses a text buffer that contains a rgb color
 *
 *Returns the parsed color, or NULL in case of error
 */
CRRgb *
cr_rgb_parse_from_buf (const guchar *a_str,
                              enum CREncoding a_enc)
{
	enum CRStatus status = CR_OK ;
	CRTerm *value = NULL ;
	CRParser * parser = NULL;
	CRRgb *result = NULL;
	
	g_return_val_if_fail (a_str, NULL);
	
	parser = cr_parser_new_from_buf ((guchar *) a_str, strlen ((const char *) a_str), a_enc, FALSE);

	g_return_val_if_fail (parser, NULL);

	status = cr_parser_try_to_skip_spaces_and_comments (parser) ;
	if (status != CR_OK)
	    	goto cleanup;

	status = cr_parser_parse_term (parser, &value);
	if (status != CR_OK)
	    	goto cleanup;

	result = cr_rgb_new ();
	if (!result)
	    	goto cleanup;

	status = cr_rgb_set_from_term (result, value);

cleanup:
	if (parser) {
	    	cr_parser_destroy (parser);
		parser = NULL;
	}
	if (value) {
	    	cr_term_destroy(value);
		value = NULL;
	}
	return result ;
}

