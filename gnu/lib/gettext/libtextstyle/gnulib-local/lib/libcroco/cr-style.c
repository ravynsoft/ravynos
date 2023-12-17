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
 *
 * Author: Dodji Seketeli.
 */

#include <config.h>
#include <string.h>
#include "cr-style.h"

/**
 *@file
 *The definition of the #CRStyle class.
 */

/**
 *A property ID.
 *Each supported css property has an ID which is
 *an entry into a property "population" jump table.
 *each entry of the property population jump table
 *contains code to tranform the literal form of
 *a property value into a strongly typed value.
 */
enum CRPropertyID {
        PROP_ID_NOT_KNOWN = 0,
        PROP_ID_PADDING_TOP,
        PROP_ID_PADDING_RIGHT,
        PROP_ID_PADDING_BOTTOM,
        PROP_ID_PADDING_LEFT,
        PROP_ID_PADDING,
        PROP_ID_BORDER_TOP_WIDTH,
        PROP_ID_BORDER_RIGHT_WIDTH,
        PROP_ID_BORDER_BOTTOM_WIDTH,
        PROP_ID_BORDER_LEFT_WIDTH,
        PROP_ID_BORDER_WIDTH,
        PROP_ID_BORDER_TOP_STYLE,
        PROP_ID_BORDER_RIGHT_STYLE,
        PROP_ID_BORDER_BOTTOM_STYLE,
        PROP_ID_BORDER_LEFT_STYLE,
        PROP_ID_BORDER_STYLE,
        PROP_ID_BORDER_TOP_COLOR,
        PROP_ID_BORDER_RIGHT_COLOR,
        PROP_ID_BORDER_BOTTOM_COLOR,
        PROP_ID_BORDER_LEFT_COLOR,
        PROP_ID_BORDER_TOP,
        PROP_ID_BORDER_RIGHT,
        PROP_ID_BORDER_BOTTOM,
        PROP_ID_BORDER_LEFT,
        PROP_ID_BORDER,
        PROP_ID_MARGIN_TOP,
        PROP_ID_MARGIN_RIGHT,
        PROP_ID_MARGIN_BOTTOM,
        PROP_ID_MARGIN_LEFT,
        PROP_ID_MARGIN,
        PROP_ID_DISPLAY,
        PROP_ID_POSITION,
        PROP_ID_TOP,
        PROP_ID_RIGHT,
        PROP_ID_BOTTOM,
        PROP_ID_LEFT,
        PROP_ID_FLOAT,
        PROP_ID_WIDTH,
        PROP_ID_COLOR,
        PROP_ID_BACKGROUND_COLOR,
        PROP_ID_FONT_FAMILY,
        PROP_ID_FONT_SIZE,
        PROP_ID_FONT_STYLE,
        PROP_ID_FONT_WEIGHT,
	PROP_ID_WHITE_SPACE,
        /*should be the last one. */
        NB_PROP_IDS
};

typedef struct _CRPropertyDesc CRPropertyDesc;

struct _CRPropertyDesc {
        const gchar *name;
        enum CRPropertyID prop_id;
};

static CRPropertyDesc gv_prop_table[] = {
        {"padding-top", PROP_ID_PADDING_TOP},
        {"padding-right", PROP_ID_PADDING_RIGHT},
        {"padding-bottom", PROP_ID_PADDING_BOTTOM},
        {"padding-left", PROP_ID_PADDING_LEFT},
        {"padding", PROP_ID_PADDING},
        {"border-top-width", PROP_ID_BORDER_TOP_WIDTH},
        {"border-right-width", PROP_ID_BORDER_RIGHT_WIDTH},
        {"border-bottom-width", PROP_ID_BORDER_BOTTOM_WIDTH},
        {"border-left-width", PROP_ID_BORDER_LEFT_WIDTH},
        {"border-width", PROP_ID_BORDER_WIDTH},
        {"border-top-style", PROP_ID_BORDER_TOP_STYLE},
        {"border-right-style", PROP_ID_BORDER_RIGHT_STYLE},
        {"border-bottom-style", PROP_ID_BORDER_BOTTOM_STYLE},
        {"border-left-style", PROP_ID_BORDER_LEFT_STYLE},
        {"border-style", PROP_ID_BORDER_STYLE},
        {"border-top", PROP_ID_BORDER_TOP},
        {"border-right", PROP_ID_BORDER_RIGHT},
        {"border-bottom", PROP_ID_BORDER_BOTTOM},
        {"border-left", PROP_ID_BORDER_LEFT},
        {"border", PROP_ID_BORDER},
        {"margin-top", PROP_ID_MARGIN_TOP},
        {"margin-right", PROP_ID_MARGIN_RIGHT},
        {"margin-bottom", PROP_ID_MARGIN_BOTTOM},
        {"margin-left", PROP_ID_MARGIN_LEFT},
        {"margin", PROP_ID_MARGIN},
        {"display", PROP_ID_DISPLAY},
        {"position", PROP_ID_POSITION},
        {"top", PROP_ID_TOP},
        {"right", PROP_ID_RIGHT},
        {"bottom", PROP_ID_BOTTOM},
        {"left", PROP_ID_LEFT},
        {"float", PROP_ID_FLOAT},
        {"width", PROP_ID_WIDTH},
        {"color", PROP_ID_COLOR},
        {"border-top-color", PROP_ID_BORDER_TOP_COLOR},
        {"border-right-color", PROP_ID_BORDER_RIGHT_COLOR},
        {"border-bottom-color", PROP_ID_BORDER_BOTTOM_COLOR},
        {"border-left-color", PROP_ID_BORDER_LEFT_COLOR},
        {"background-color", PROP_ID_BACKGROUND_COLOR},
        {"font-family", PROP_ID_FONT_FAMILY},
        {"font-size", PROP_ID_FONT_SIZE},
        {"font-style", PROP_ID_FONT_STYLE},
        {"font-weight", PROP_ID_FONT_WEIGHT},
	{"white-space", PROP_ID_WHITE_SPACE},
        /*must be the last one */
        {NULL, 0}
};

/**
 *A the key/value pair of this hash table
 *are:
 *key => name of the the css propertie found in gv_prop_table
 *value => matching property id found in gv_prop_table.
 *So this hash table is here just to retrieval of a property id
 *from a property name.
 */
static GHashTable *gv_prop_hash = NULL;

/**
 *incremented by each new instance of #CRStyle
 *and decremented at the it destroy time.
 *When this reaches zero, gv_prop_hash is destroyed.
 */
static gulong gv_prop_hash_ref_count = 0;

struct CRNumPropEnumDumpInfo {
        enum CRNumProp code;
        const gchar *str;
};

static struct CRNumPropEnumDumpInfo gv_num_props_dump_infos[] = {
        {NUM_PROP_TOP, "top"},
        {NUM_PROP_RIGHT, "right"},
        {NUM_PROP_BOTTOM, "bottom"},
        {NUM_PROP_LEFT, "left"},
        {NUM_PROP_PADDING_TOP, "padding-top"},
        {NUM_PROP_PADDING_RIGHT, "padding-right"},
        {NUM_PROP_PADDING_BOTTOM, "padding-bottom"},
        {NUM_PROP_PADDING_LEFT, "padding-left"},
        {NUM_PROP_BORDER_TOP, "border-top"},
        {NUM_PROP_BORDER_RIGHT, "border-right"},
        {NUM_PROP_BORDER_BOTTOM, "border-bottom"},
        {NUM_PROP_BORDER_LEFT, "border-left"},
        {NUM_PROP_MARGIN_TOP, "margin-top"},
        {NUM_PROP_MARGIN_RIGHT, "margin-right"},
        {NUM_PROP_MARGIN_BOTTOM, "margin-bottom"},
        {NUM_PROP_MARGIN_LEFT, "margin-left"},
        {NUM_PROP_WIDTH, "width"},
        {0, NULL}
};

struct CRRgbPropEnumDumpInfo {
        enum CRRgbProp code;
        const gchar *str;
};

static struct CRRgbPropEnumDumpInfo gv_rgb_props_dump_infos[] = {
        {RGB_PROP_BORDER_TOP_COLOR, "border-top-color"},
        {RGB_PROP_BORDER_RIGHT_COLOR, "border-right-color"},
        {RGB_PROP_BORDER_BOTTOM_COLOR, "bottom-color"},
        {RGB_PROP_BORDER_LEFT_COLOR, "left-color"},
        {RGB_PROP_COLOR, "color"},
        {RGB_PROP_BACKGROUND_COLOR, "background-color"},
        {0, NULL}
};

struct CRBorderStylePropEnumDumpInfo {
        enum CRBorderStyleProp code;
        const gchar *str;

};

static struct CRBorderStylePropEnumDumpInfo gv_border_style_props_dump_infos[]
        = {
        {BORDER_STYLE_PROP_TOP, "border-style-top"},
        {BORDER_STYLE_PROP_RIGHT, "border-style-right"},
        {BORDER_STYLE_PROP_BOTTOM, "boder-style-bottom"},
        {BORDER_STYLE_PROP_LEFT, "border-style-left"},
        {0, NULL}
};

static enum CRStatus
  cr_style_init_properties (void);

enum CRDirection {
        DIR_TOP = 0,
        DIR_RIGHT,
        DIR_BOTTOM,
        DIR_LEFT,

        /*must be the last one */
        NB_DIRS
};

static const gchar *num_prop_code_to_string (enum CRNumProp a_code);

static const gchar *rgb_prop_code_to_string (enum CRRgbProp a_code);

static const gchar *border_style_prop_code_to_string (enum CRBorderStyleProp
                                                      a_code);

static enum CRStatus
set_prop_padding_x_from_value (CRStyle * a_style,
                                 CRTerm * a_value, enum CRDirection a_dir);

static enum CRStatus
set_prop_border_x_width_from_value (CRStyle * a_style,
                                    CRTerm * a_value,
                                    enum CRDirection a_dir);
static enum CRStatus
set_prop_border_width_from_value (CRStyle *a_style,
                                  CRTerm *a_value) ;

static enum CRStatus
set_prop_border_x_style_from_value (CRStyle * a_style,
                                    CRTerm * a_value,
                                    enum CRDirection a_dir);
static enum CRStatus
set_prop_border_style_from_value (CRStyle *a_style,
                                  CRTerm *a_value) ;

static enum CRStatus
set_prop_margin_x_from_value (CRStyle * a_style, CRTerm * a_value,
                                enum CRDirection a_dir);

static enum CRStatus
set_prop_display_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_position_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_x_from_value (CRStyle * a_style, CRTerm * a_value,
                         enum CRDirection a_dir);

static enum CRStatus
set_prop_float (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_width (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_color (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_background_color (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_border_x_color_from_value (CRStyle * a_style, CRTerm * a_value,
                                      enum CRDirection a_dir);

static enum CRStatus
set_prop_border_x_from_value (CRStyle * a_style, CRTerm * a_value,
                                enum CRDirection a_dir);

static enum CRStatus
set_prop_border_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_padding_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_margin_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_font_family_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
init_style_font_size_field (CRStyle * a_style);

static enum CRStatus
set_prop_font_size_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_font_style_from_value (CRStyle * a_style, CRTerm * a_value);

static enum CRStatus
set_prop_font_weight_from_value (CRStyle * a_style, CRTerm * a_value);

static const gchar *
num_prop_code_to_string (enum CRNumProp a_code)
{
        guint len = sizeof (gv_num_props_dump_infos) /
                sizeof (struct CRNumPropEnumDumpInfo);
        if (a_code >= len) {
                cr_utils_trace_info ("A field has been added "
                                     "to 'enum CRNumProp' and no matching"
                                     " entry has been "
                                     "added to gv_num_prop_dump_infos table.\n"
                                     "Please add the missing matching entry");
                return NULL;
        }
        if (gv_num_props_dump_infos[a_code].code != a_code) {
                cr_utils_trace_info ("mismatch between the order of fields in"
                                     " 'enum CRNumProp' and "
                                     "the order of entries in "
                                     "the gv_num_prop_dump_infos table");
                return NULL;
        }
        return gv_num_props_dump_infos[a_code].str;
}

static const gchar *
rgb_prop_code_to_string (enum CRRgbProp a_code)
{
        guint len = sizeof (gv_rgb_props_dump_infos) /
                sizeof (struct CRRgbPropEnumDumpInfo);

        if (a_code >= len) {
                cr_utils_trace_info ("A field has been added "
                                     "to 'enum CRRgbProp' and no matching"
                                     " entry has been "
                                     "added to gv_rgb_prop_dump_infos table.\n"
                                     "Please add the missing matching entry");
                return NULL;
        }
        if (gv_rgb_props_dump_infos[a_code].code != a_code) {
                cr_utils_trace_info ("mismatch between the order of fields in"
                                     " 'enum CRRgbProp' and "
                                     "the order of entries in "
                                     "the gv_rgb_props_dump_infos table");
                return NULL;
        }
        return gv_rgb_props_dump_infos[a_code].str;
}

static const gchar *
border_style_prop_code_to_string (enum CRBorderStyleProp a_code)
{
        guint len = sizeof (gv_border_style_props_dump_infos) /
                sizeof (struct CRBorderStylePropEnumDumpInfo);

        if (a_code >= len) {
                cr_utils_trace_info ("A field has been added "
                                     "to 'enum CRBorderStyleProp' and no matching"
                                     " entry has been "
                                     "added to gv_border_style_prop_dump_infos table.\n"
                                     "Please add the missing matching entry");
                return NULL;
        }
        if (gv_border_style_props_dump_infos[a_code].code != a_code) {
                cr_utils_trace_info ("mismatch between the order of fields in"
                                     " 'enum CRBorderStyleProp' and "
                                     "the order of entries in "
                                     "the gv_border_style_props_dump_infos table");
                return NULL;
        }
        return gv_border_style_props_dump_infos[a_code].str;
}

static enum CRStatus
cr_style_init_properties (void)
{

        if (!gv_prop_hash) {
                gulong i = 0;

                gv_prop_hash = g_hash_table_new (g_str_hash, g_str_equal);
                if (!gv_prop_hash) {
                        cr_utils_trace_info ("Out of memory");
                        return CR_ERROR;
                }

                /*load gv_prop_hash from gv_prop_table */
                for (i = 0; gv_prop_table[i].name; i++) {
                        g_hash_table_insert
                                (gv_prop_hash,
                                 (gpointer) gv_prop_table[i].name,
                                 GINT_TO_POINTER (gv_prop_table[i].prop_id));
                }
        }

        return CR_OK;
}

static enum CRPropertyID
cr_style_get_prop_id (const guchar * a_prop)
{
        gpointer *raw_id = NULL;

        if (!gv_prop_hash) {
                cr_style_init_properties ();
        }

        raw_id = g_hash_table_lookup (gv_prop_hash, a_prop);
        if (!raw_id) {
                return PROP_ID_NOT_KNOWN;
        }
        return GPOINTER_TO_INT (raw_id);
}

static enum CRStatus
set_prop_padding_x_from_value (CRStyle * a_style,
                               CRTerm * a_value, enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK;
        CRNum *num_val = NULL;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        if (a_value->type != TERM_NUMBER && a_value->type != TERM_IDENT)
                return CR_BAD_PARAM_ERROR;

        switch (a_dir) {
        case DIR_TOP:
                num_val = &a_style->num_props[NUM_PROP_PADDING_TOP].sv;
                break;

        case DIR_RIGHT:
                num_val = &a_style->num_props[NUM_PROP_PADDING_RIGHT].sv;
                break;

        case DIR_BOTTOM:
                num_val = &a_style->num_props[NUM_PROP_PADDING_BOTTOM].sv;
                break;

        case DIR_LEFT:
                num_val = &a_style->num_props[NUM_PROP_PADDING_LEFT].sv;
                break;

        default:
                return CR_BAD_PARAM_ERROR;
        }

        if (a_value->type == TERM_IDENT) {
                if (a_value->content.str
                    && a_value->content.str->stryng
		    && a_value->content.str->stryng->str
                    && !strncmp ((const char *) "inherit",
                                 a_value->content.str->stryng->str,
                                 sizeof ("inherit")-1)) {
			status = cr_num_set (num_val, 0.0, NUM_INHERIT);
                        return CR_OK;
                } else
                        return CR_UNKNOWN_TYPE_ERROR;
        }

        g_return_val_if_fail (a_value->type == TERM_NUMBER
                              && a_value->content.num, CR_UNKNOWN_TYPE_ERROR);

        switch (a_value->content.num->type) {
        case NUM_LENGTH_EM:
        case NUM_LENGTH_EX:
        case NUM_LENGTH_PX:
        case NUM_LENGTH_IN:
        case NUM_LENGTH_CM:
        case NUM_LENGTH_MM:
        case NUM_LENGTH_PT:
        case NUM_LENGTH_PC:
        case NUM_PERCENTAGE:
                status = cr_num_copy (num_val, a_value->content.num);
                break;
        default:
                status = CR_UNKNOWN_TYPE_ERROR;
                break;
        }

        return status;
}

static enum CRStatus
set_prop_border_x_width_from_value (CRStyle * a_style,
                                    CRTerm * a_value, 
                                    enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK;
        CRNum *num_val = NULL;

        g_return_val_if_fail (a_value && a_style, CR_BAD_PARAM_ERROR);

        switch (a_dir) {
        case DIR_TOP:
                num_val = &a_style->num_props[NUM_PROP_BORDER_TOP].sv;
                break;

        case DIR_RIGHT:
                num_val = &a_style->num_props[NUM_PROP_BORDER_RIGHT].sv;
                break;

        case DIR_BOTTOM:
                num_val = &a_style->num_props[NUM_PROP_BORDER_BOTTOM].sv;
                break;

        case DIR_LEFT:
                num_val = &a_style->num_props[NUM_PROP_BORDER_LEFT].sv;
                break;

        default:
                return CR_BAD_PARAM_ERROR;
                break;
        }

        if (a_value->type == TERM_IDENT) {
                if (a_value->content.str 
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
                        if (!strncmp ("thin",
                                      a_value->content.str->stryng->str,
                                      sizeof ("thin")-1)) {
                                cr_num_set (num_val, BORDER_THIN,
                                            NUM_LENGTH_PX);
                        } else if (!strncmp 
                                   ("medium",
                                    a_value->content.str->stryng->str,
                                             sizeof ("medium")-1)) {
                                cr_num_set (num_val, BORDER_MEDIUM,
                                            NUM_LENGTH_PX);
                        } else if (!strncmp ("thick",
                                             a_value->content.str->stryng->str,
                                             sizeof ("thick")-1)) {
                                cr_num_set (num_val, BORDER_THICK,
                                            NUM_LENGTH_PX);
                        } else {
                                return CR_UNKNOWN_TYPE_ERROR;
                        }
                }
        } else if (a_value->type == TERM_NUMBER) {
                if (a_value->content.num) {
                        cr_num_copy (num_val, a_value->content.num);
                }
        } else if (a_value->type != TERM_NUMBER
                   || a_value->content.num == NULL) {
                return CR_UNKNOWN_TYPE_ERROR;
        }

        return status;
}

static enum CRStatus
set_prop_border_width_from_value (CRStyle *a_style,
                                  CRTerm *a_value)
{
        CRTerm *cur_term = NULL ;
        enum CRDirection direction = DIR_TOP ;

        g_return_val_if_fail (a_style && a_value,
                              CR_BAD_PARAM_ERROR) ;
        cur_term = a_value ;

        if (!cur_term)
                return CR_ERROR ;

        for (direction = DIR_TOP ; 
             direction < NB_DIRS ; direction ++) {
                set_prop_border_x_width_from_value (a_style, 
                                                    cur_term,
                                                    direction) ;
        }

        cur_term = cur_term->next ;
        if (!cur_term)
                return CR_OK ;
        set_prop_border_x_width_from_value (a_style, cur_term,
                                            DIR_RIGHT) ;
        set_prop_border_x_width_from_value (a_style, cur_term,
                                            DIR_LEFT) ;

        cur_term = cur_term->next ;
        if (!cur_term)
                return CR_OK ;
        set_prop_border_x_width_from_value (a_style, cur_term,
                                            DIR_BOTTOM) ;

        cur_term = cur_term->next ;
        if (!cur_term)
                return CR_OK ;
        set_prop_border_x_width_from_value (a_style, cur_term,
                                            DIR_LEFT) ;

        return CR_OK ;
}

static enum CRStatus
set_prop_border_x_style_from_value (CRStyle * a_style,
                                    CRTerm * a_value, enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK;
        enum CRBorderStyle *border_style_ptr = NULL;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_dir) {
        case DIR_TOP:
                border_style_ptr = &a_style->
                        border_style_props[BORDER_STYLE_PROP_TOP];
                break;

        case DIR_RIGHT:
                border_style_ptr =
                        &a_style->border_style_props[BORDER_STYLE_PROP_RIGHT];
                break;

        case DIR_BOTTOM:
                border_style_ptr = &a_style->
                        border_style_props[BORDER_STYLE_PROP_BOTTOM];
                break;

        case DIR_LEFT:
                border_style_ptr = &a_style->
                        border_style_props[BORDER_STYLE_PROP_LEFT];
                break;

        default:
                break;
        }

        if (a_value->type != TERM_IDENT || !a_value->content.str) {
                return CR_UNKNOWN_TYPE_ERROR;
        }

        if (!strncmp ("none", 
                      a_value->content.str->stryng->str, 
                      sizeof ("none")-1)) {
                *border_style_ptr = BORDER_STYLE_NONE;
        } else if (!strncmp ("hidden",
                             a_value->content.str->stryng->str, 
                             sizeof ("hidden")-1)) {
                *border_style_ptr = BORDER_STYLE_HIDDEN;
        } else if (!strncmp ("dotted",
                             a_value->content.str->stryng->str, 
                             sizeof ("dotted")-1)) {
                *border_style_ptr = BORDER_STYLE_DOTTED;
        } else if (!strncmp ("dashed",
                             a_value->content.str->stryng->str, sizeof ("dashed")-1)) {
                *border_style_ptr = BORDER_STYLE_DASHED;
        } else if (!strncmp ("solid",
                             a_value->content.str->stryng->str, sizeof ("solid")-1)) {
                *border_style_ptr = BORDER_STYLE_SOLID;
        } else if (!strncmp ("double",
                             a_value->content.str->stryng->str, sizeof ("double")-1)) {
                *border_style_ptr = BORDER_STYLE_DOUBLE;
        } else if (!strncmp ("groove",
                             a_value->content.str->stryng->str, sizeof ("groove")-1)) {
                *border_style_ptr = BORDER_STYLE_GROOVE;
        } else if (!strncmp ("ridge",
                             a_value->content.str->stryng->str, 
                             sizeof ("ridge")-1)) {
                *border_style_ptr = BORDER_STYLE_RIDGE;
        } else if (!strncmp ("inset",
                             a_value->content.str->stryng->str, 
                             sizeof ("inset")-1)) {
                *border_style_ptr = BORDER_STYLE_INSET;
        } else if (!strncmp ("outset",
                             a_value->content.str->stryng->str, 
                             sizeof ("outset")-1)) {
                *border_style_ptr = BORDER_STYLE_OUTSET;
        } else if (!strncmp ("inherit",
                             a_value->content.str->stryng->str, 
                             sizeof ("inherit")-1)) {
		*border_style_ptr = BORDER_STYLE_INHERIT;
        } else {
                status = CR_UNKNOWN_TYPE_ERROR;
        }

        return status;
}

static enum CRStatus
set_prop_border_style_from_value (CRStyle *a_style,
                                  CRTerm *a_value)
{
        CRTerm *cur_term = NULL ;
        enum CRDirection direction = DIR_TOP ;

        g_return_val_if_fail (a_style && a_value, 
                              CR_BAD_PARAM_ERROR) ;

        cur_term = a_value ;
        if (!cur_term || cur_term->type != TERM_IDENT) {
                return CR_ERROR ;
        }
        
        for (direction = DIR_TOP ; 
             direction < NB_DIRS ;
             direction ++) {
                set_prop_border_x_style_from_value (a_style, 
                                                    cur_term,
                                                    direction) ;
        }
        
        cur_term = cur_term->next ;
        if (!cur_term || cur_term->type != TERM_IDENT) {
                return CR_OK ;
        }
        
        set_prop_border_x_style_from_value (a_style, cur_term, 
                                            DIR_RIGHT) ;
        set_prop_border_x_style_from_value (a_style, cur_term, 
                                            DIR_LEFT) ;

        cur_term = cur_term->next ;
        if (!cur_term || cur_term->type != TERM_IDENT) {
                return CR_OK ;
        }        
        set_prop_border_x_style_from_value (a_style, cur_term,
                                           DIR_BOTTOM) ;
        
        cur_term = cur_term->next ;
        if (!cur_term || cur_term->type != TERM_IDENT) {
                return CR_OK ;
        }
        set_prop_border_x_style_from_value (a_style, cur_term,
                                            DIR_LEFT) ;
        return CR_OK ;
}

static enum CRStatus
set_prop_margin_x_from_value (CRStyle * a_style, CRTerm * a_value,
                              enum CRDirection a_dir)
{
        enum CRStatus status = CR_OK;
        CRNum *num_val = NULL;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_dir) {
        case DIR_TOP:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_TOP].sv;
                break;

        case DIR_RIGHT:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_RIGHT].sv;
                break;

        case DIR_BOTTOM:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_BOTTOM].sv;
                break;

        case DIR_LEFT:
                num_val = &a_style->num_props[NUM_PROP_MARGIN_LEFT].sv;
                break;

        default:
                break;
        }

        switch (a_value->type) {
        case TERM_IDENT:
                if (a_value->content.str
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str
                    && !strcmp (a_value->content.str->stryng->str,
                                 "inherit")) {
			status = cr_num_set (num_val, 0.0, NUM_INHERIT);
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && !strcmp (a_value->content.str->stryng->str,
                                        "auto")) {
                        status = cr_num_set (num_val, 0.0, NUM_AUTO);
                } else {
                        status = CR_UNKNOWN_TYPE_ERROR;
                }
                break ;

        case TERM_NUMBER:
                status = cr_num_copy (num_val, a_value->content.num);
                break;

        default:
                status = CR_UNKNOWN_TYPE_ERROR;
                break;
        }

        return status;
}

struct CRPropDisplayValPair {
        const gchar *prop_name;
        enum CRDisplayType type;
};

static enum CRStatus
set_prop_display_from_value (CRStyle * a_style, CRTerm * a_value)
{
        static const struct CRPropDisplayValPair disp_vals_map[] = {
                {"none", DISPLAY_NONE},
                {"inline", DISPLAY_INLINE},
                {"block", DISPLAY_BLOCK},
                {"run-in", DISPLAY_RUN_IN},
                {"compact", DISPLAY_COMPACT},
                {"marker", DISPLAY_MARKER},
                {"table", DISPLAY_TABLE},
                {"inline-table", DISPLAY_INLINE_TABLE},
                {"table-row-group", DISPLAY_TABLE_ROW_GROUP},
                {"table-header-group", DISPLAY_TABLE_HEADER_GROUP},
                {"table-footer-group", DISPLAY_TABLE_FOOTER_GROUP},
                {"table-row", DISPLAY_TABLE_ROW},
                {"table-column-group", DISPLAY_TABLE_COLUMN_GROUP},
                {"table-column", DISPLAY_TABLE_COLUMN},
                {"table-cell", DISPLAY_TABLE_CELL},
                {"table-caption", DISPLAY_TABLE_CAPTION},
                {"inherit", DISPLAY_INHERIT},
                {NULL, DISPLAY_NONE}
        };

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_value->type) {
        case TERM_IDENT:
                {
                        int i = 0;

                        if (!a_value->content.str
                            || !a_value->content.str->stryng
                            || !a_value->content.str->stryng->str)
                                break;

                        for (i = 0; disp_vals_map[i].prop_name; i++) {
                                if (!strncmp 
                                    (disp_vals_map[i].prop_name,
                                     a_value->content.str->stryng->str,
                                     strlen (disp_vals_map[i].prop_name))) {
                                        a_style->display =
                                                disp_vals_map[i].type;
                                        break;
                                }
                        }
                }
                break;

        default:
                break;
        }

        return CR_OK;
}

struct CRPropPositionValPair {
        const gchar *name;
        enum CRPositionType type;
};

static enum CRStatus
set_prop_position_from_value (CRStyle * a_style, CRTerm * a_value)
{
        enum CRStatus status = CR_UNKNOWN_PROP_VAL_ERROR;
        static const struct CRPropPositionValPair position_vals_map[] = {
                {"static", POSITION_STATIC},
                {"relative", POSITION_RELATIVE},
                {"absolute", POSITION_ABSOLUTE},
                {"fixed", POSITION_FIXED},
                {"inherit", POSITION_INHERIT},
                {NULL, POSITION_STATIC}
                /*must alwas be the last one */
        };

        g_return_val_if_fail (a_value, CR_BAD_PARAM_ERROR);

        switch (a_value->type) {
        case TERM_IDENT:
                {
                        int i = 0;

                        if (!a_value->content.str
                            || !a_value->content.str->stryng
                            || !a_value->content.str->stryng->str)
                                break;

                        for (i = 0; position_vals_map[i].name; i++) {
                                if (!strncmp (position_vals_map[i].name,
                                              a_value->content.str->stryng->str,
                                              strlen (position_vals_map[i].
                                                      name))) {
                                        a_style->position =
                                                position_vals_map[i].type;
                                        status = CR_OK;
                                        break;
                                }
                        }
                }
                break;

        default:
                break;
        }

        return status;
}

static enum CRStatus
set_prop_x_from_value (CRStyle * a_style, CRTerm * a_value,
                       enum CRDirection a_dir)
{
        CRNum *box_offset = NULL;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        if (!(a_value->type == TERM_NUMBER)
            && !(a_value->type == TERM_IDENT)) {
                return CR_UNKNOWN_PROP_VAL_ERROR;
        }

        switch (a_dir) {
        case DIR_TOP:
                box_offset = &a_style->num_props[NUM_PROP_TOP].sv;
                break;

        case DIR_RIGHT:
                box_offset = &a_style->num_props[NUM_PROP_RIGHT].sv;
                break;

        case DIR_BOTTOM:
                box_offset = &a_style->num_props[NUM_PROP_BOTTOM].sv;
                break;
        case DIR_LEFT:
                box_offset = &a_style->num_props[NUM_PROP_LEFT].sv;
                break;

        default:
                break;
        }

        box_offset->type = NUM_AUTO;

        if (a_value->type == TERM_NUMBER && a_value->content.num) {
                cr_num_copy (box_offset, a_value->content.num);
        } else if (a_value->type == TERM_IDENT
                   && a_value->content.str
                   && a_value->content.str->stryng
                   && a_value->content.str->stryng->str) {
                if (!strncmp ("inherit",
                              a_value->content.str->stryng->str,
                              sizeof ("inherit")-1)) {
                        cr_num_set (box_offset, 0.0, NUM_INHERIT);
                } else if (!strncmp ("auto",
                                     a_value->content.str->stryng->str,
                                     sizeof ("auto")-1)) {
                        box_offset->type = NUM_AUTO;
                }
        }

        return CR_OK;
}

static enum CRStatus
set_prop_float (CRStyle * a_style, CRTerm * a_value)
{
        g_return_val_if_fail (a_style && a_value, 
                              CR_BAD_PARAM_ERROR);

        /*the default float type as specified by the css2 spec */
        a_style->float_type = FLOAT_NONE;

        if (a_value->type != TERM_IDENT 
            || !a_value->content.str
            || !a_value->content.str->stryng
            || !a_value->content.str->stryng->str) { 
                /*unknown type, the float type is set to it's default value */
                return CR_OK;
        }

        if (!strncmp ("none", 
                      a_value->content.str->stryng->str, 
                      sizeof ("none")-1)) {
                a_style->float_type = FLOAT_NONE;
        } else if (!strncmp ("left",
                             a_value->content.str->stryng->str, 
                             sizeof ("left")-1)) {
                a_style->float_type = FLOAT_LEFT;
        } else if (!strncmp ("right",
                             a_value->content.str->stryng->str, 
                             sizeof ("right")-1)) {
                a_style->float_type = FLOAT_RIGHT;
        } else if (!strncmp ("inherit",
                             a_value->content.str->stryng->str, 
                             sizeof ("inherit")-1)) {
		a_style->float_type = FLOAT_INHERIT;
        }
        return CR_OK;
}

static enum CRStatus
set_prop_width (CRStyle * a_style, CRTerm * a_value)
{
	CRNum *width = NULL;
        g_return_val_if_fail (a_style 
                              && a_value, 
                              CR_BAD_PARAM_ERROR);

	width = &a_style->num_props[NUM_PROP_WIDTH].sv;
	cr_num_set (width, 0.0, NUM_AUTO);

        if (a_value->type == TERM_IDENT) {
                if (a_value->content.str 
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
                        if (!strncmp ("auto",
                                      a_value->content.str->stryng->str,
                                      sizeof ("auto")-1)) {
				cr_num_set (width, 0.0, NUM_AUTO);
                        } else if (!strncmp ("inherit",
                                             a_value->content.str->stryng->str,
                                             sizeof ("inherit")-1)) {
				cr_num_set (width, 0.0, NUM_INHERIT);
                        }
                }
        } else if (a_value->type == TERM_NUMBER) {
                if (a_value->content.num) {
                        cr_num_copy (&a_style->num_props[NUM_PROP_WIDTH].sv,
                                     a_value->content.num);
                }
        }
        return CR_OK;
}

static enum CRStatus 
set_prop_color (CRStyle * a_style, CRTerm * a_value)
{
	enum CRStatus status = CR_OK;
	CRRgb *a_rgb = &a_style->rgb_props[RGB_PROP_COLOR].sv;

	g_return_val_if_fail (a_style 
                              && a_value, CR_BAD_PARAM_ERROR);

	status = cr_rgb_set_from_term (a_rgb, a_value);

	return status;
}

static enum CRStatus
set_prop_background_color (CRStyle * a_style, CRTerm * a_value)
{
	enum CRStatus status = CR_OK;
	CRRgb *rgb = &a_style->rgb_props[RGB_PROP_BACKGROUND_COLOR].sv;

	g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

	status = cr_rgb_set_from_term (rgb, a_value);
	return status;
}

/**
 *Sets border-top-color, border-right-color,
 *border-bottom-color or border-left-color properties
 *in the style structure. The value is taken from a
 *css2 term of type IDENT or RGB.
 *@param a_style the style structure to set.
 *@param a_value the css2 term to take the color information from.
 *@param a_dir the direction (TOP, LEFT, RIGHT, or BOTTOM).
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
static enum CRStatus
set_prop_border_x_color_from_value (CRStyle * a_style, CRTerm * a_value,
                                    enum CRDirection a_dir)
{
        CRRgb *rgb_color = NULL;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_dir) {
        case DIR_TOP:
                rgb_color = &a_style->rgb_props[RGB_PROP_BORDER_TOP_COLOR].sv;
                break;

        case DIR_RIGHT:
                rgb_color =
                        &a_style->rgb_props[RGB_PROP_BORDER_RIGHT_COLOR].sv;
                break;

        case DIR_BOTTOM:
                rgb_color =
                        &a_style->rgb_props[RGB_PROP_BORDER_BOTTOM_COLOR].sv;
                break;

        case DIR_LEFT:
                rgb_color =
                        &a_style->rgb_props[RGB_PROP_BORDER_LEFT_COLOR].sv;
                break;

        default:
                cr_utils_trace_info ("unknown DIR type");
                return CR_BAD_PARAM_ERROR;
        }

        status = CR_UNKNOWN_PROP_VAL_ERROR;

        if (a_value->type == TERM_IDENT) {
                if (a_value->content.str 
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
                        status = cr_rgb_set_from_name
                                (rgb_color, 
                                 (const guchar *) a_value->content.str->stryng->str);

                }
                if (status != CR_OK) {
                        cr_rgb_set_from_name (rgb_color, (const guchar *) "black");
                }
        } else if (a_value->type == TERM_RGB) {
                if (a_value->content.rgb) {
                        status = cr_rgb_set_from_rgb
                                (rgb_color, a_value->content.rgb);
                }
        }
        return status;
}

static enum CRStatus
set_prop_border_x_from_value (CRStyle * a_style, CRTerm * a_value,
                              enum CRDirection a_dir)
{
        CRTerm *cur_term = NULL;

        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        for (cur_term = a_value; 
             cur_term; 
             cur_term = cur_term->next) {
                status = set_prop_border_x_width_from_value (a_style,
                                                             cur_term, a_dir);

                if (status != CR_OK) {
                        status = set_prop_border_x_style_from_value
                                (a_style, cur_term, a_dir);
                }
                if (status != CR_OK) {
                        status = set_prop_border_x_color_from_value
                                (a_style, cur_term, a_dir);
                }
        }
        return CR_OK;
}

static enum CRStatus
set_prop_border_from_value (CRStyle * a_style, CRTerm * a_value)
{
        enum CRDirection direction = 0;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        for (direction = 0; direction < NB_DIRS; direction++) {
                set_prop_border_x_from_value (a_style, 
                                              a_value, 
                                              direction);
        }

        return CR_OK;
}

static enum CRStatus
set_prop_padding_from_value (CRStyle * a_style, CRTerm * a_value)
{
        CRTerm *cur_term = NULL;
        enum CRDirection direction = 0;
        enum CRStatus status = CR_OK;
        
        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        cur_term = a_value;

        /*filter the eventual non NUMBER terms some user can have written here*/
        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        if (!cur_term)
                return CR_ERROR ;

        for (direction = 0; direction < NB_DIRS; direction++) {
                set_prop_padding_x_from_value (a_style, cur_term, direction);
        }
        cur_term = cur_term->next;

        /*filter non NUMBER terms that some users can have written here...*/
        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        /*the user can have just written padding: 1px*/
        if (!cur_term)
                return CR_OK;

        set_prop_padding_x_from_value (a_style, cur_term, DIR_RIGHT);
        set_prop_padding_x_from_value (a_style, cur_term, DIR_LEFT);

        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        if (!cur_term)
                return CR_OK;

        set_prop_padding_x_from_value (a_style, cur_term, DIR_BOTTOM);

        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        if (!cur_term)
                return CR_OK;
        status = set_prop_padding_x_from_value (a_style, cur_term, DIR_LEFT);
        return status;
}

static enum CRStatus
set_prop_margin_from_value (CRStyle * a_style, CRTerm * a_value)
{
        CRTerm *cur_term = NULL;
        enum CRDirection direction = 0;
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        cur_term = a_value;

        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }

        if (!cur_term)
                return CR_OK;

        for (direction = 0; direction < NB_DIRS; direction++) {
                set_prop_margin_x_from_value (a_style, cur_term, direction);
        }
        cur_term = cur_term->next;

        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        if (!cur_term)
                return CR_OK;

        set_prop_margin_x_from_value (a_style, cur_term, DIR_RIGHT);
        set_prop_margin_x_from_value (a_style, cur_term, DIR_LEFT);

        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        if (!cur_term)
                return CR_OK;

        set_prop_margin_x_from_value (a_style, cur_term, DIR_BOTTOM);

        while (cur_term && cur_term->type != TERM_NUMBER) {
                cur_term = cur_term->next;
        }
        if (!cur_term)
                return CR_OK;

        status = set_prop_margin_x_from_value (a_style, cur_term, DIR_LEFT);        

        return status;
}

static enum CRStatus
set_prop_font_family_from_value (CRStyle * a_style, CRTerm * a_value)
{
        CRTerm *cur_term = NULL;
        CRFontFamily *font_family = NULL,
                *cur_ff = NULL,
                *cur_ff2 = NULL;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

	if (a_value->type == TERM_IDENT &&
	    a_value->content.str &&
	    a_value->content.str->stryng &&
	    a_value->content.str->stryng->str &&
	    !strcmp ("inherit", a_value->content.str->stryng->str))
	{
		font_family = cr_font_family_new (FONT_FAMILY_INHERIT, NULL);
		goto out;
	}

        for (cur_term = a_value; cur_term; cur_term = cur_term->next) {
                switch (cur_term->type) {
                case TERM_IDENT:
                        {
                                enum CRFontFamilyType font_type;

                                if (cur_term->content.str
                                    && cur_term->content.str->stryng
                                    && cur_term->content.str->stryng->str
                                    && !strcmp 
                                    (cur_term->content.str->stryng->str,
                                     "sans-serif")) {
                                        font_type = FONT_FAMILY_SANS_SERIF;
                                } else if (cur_term->content.str
                                           && cur_term->content.str->stryng
                                           && cur_term->content.str->stryng->str
                                           && !strcmp 
                                           (cur_term->content.str->stryng->str, 
                                            "serif")) {
                                        font_type = FONT_FAMILY_SERIF;
                                } else if (cur_term->content.str
                                           && cur_term->content.str->stryng
                                           && cur_term->content.str->stryng->str
                                           && !strcmp (cur_term->content.str->stryng->str, 
                                                       "cursive")) {
                                        font_type = FONT_FAMILY_CURSIVE;
                                } else if (cur_term->content.str
                                           && cur_term->content.str->stryng
                                           && cur_term->content.str->stryng->str
                                           && !strcmp (cur_term->content.str->stryng->str,
                                                       "fantasy")) {
                                        font_type = FONT_FAMILY_FANTASY;
                                } else if (cur_term->content.str
                                           && cur_term->content.str->stryng
                                           && cur_term->content.str->stryng->str
                                           && !strcmp (cur_term->content.str->stryng->str, 
                                                       "monospace")) {
                                        font_type = FONT_FAMILY_MONOSPACE;
                                } else {
                                        /*
                                         *unknown property value.
                                         *ignore it.
                                         */
                                        continue;
                                }

                                cur_ff = cr_font_family_new (font_type, NULL);
                        }
                        break;

                case TERM_STRING:
                        {
                                if (cur_term->content.str
                                    && cur_term->content.str->stryng
                                    && cur_term->content.str->stryng->str) {
                                        cur_ff = cr_font_family_new
                                                (FONT_FAMILY_NON_GENERIC,
                                                 (guchar *) cur_term->content.str->stryng->str);
                                }
                        }
                        break;

                default:
                        break;
                }

                cur_ff2 = cr_font_family_append (font_family, cur_ff);
                if (cur_ff2) {
                        font_family = cur_ff2;
                }
        }

 out:
        if (font_family) {
                if (a_style->font_family) {
                        cr_font_family_destroy (a_style->font_family);
                        a_style->font_family = NULL ;
                }
                a_style->font_family = font_family;
                font_family = NULL ;
        }

        return CR_OK;
}

static enum CRStatus
init_style_font_size_field (CRStyle * a_style)
{
        g_return_val_if_fail (a_style, CR_BAD_PARAM_ERROR);

        memset (&a_style->font_size, 0, 
               sizeof (CRFontSizeVal)) ;
        /*
        if (!a_style->font_size) {
                a_style->font_size = cr_font_size_new ();
                if (!a_style->font_size) {
                        return CR_INSTANCIATION_FAILED_ERROR;
                }
        } else {
                cr_font_size_clear (a_style->font_size);
        }
        */
        return CR_OK;
}

static enum CRStatus
set_prop_font_size_from_value (CRStyle * a_style, CRTerm * a_value)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_value->type) {
        case TERM_IDENT:
                if (a_value->content.str
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str
                    && !strcmp (a_value->content.str->stryng->str,
                                "xx-small")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_XX_SMALL;

                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "x-small")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_X_SMALL;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "small")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_SMALL;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, "medium")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_MEDIUM;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "large")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_LARGE;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "x-large")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_X_LARGE;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "xx-large")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type =
                                PREDEFINED_ABSOLUTE_FONT_SIZE;
                        a_style->font_size.sv.value.predefined =
                                FONT_SIZE_XX_LARGE;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "larger")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type = RELATIVE_FONT_SIZE;
                        a_style->font_size.sv.value.relative = FONT_SIZE_LARGER;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, 
                                       "smaller")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type = RELATIVE_FONT_SIZE;
                        a_style->font_size.sv.value.relative =
                                FONT_SIZE_SMALLER;
                } else if (a_value->content.str
                           && a_value->content.str->stryng
                           && a_value->content.str->stryng->str
                           && !strcmp (a_value->content.str->stryng->str, "inherit")) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);
			a_style->font_size.sv.type = INHERITED_FONT_SIZE;

                } else {
                        cr_utils_trace_info ("Unknown value of font-size") ;
                        status = init_style_font_size_field (a_style);
                        return CR_UNKNOWN_PROP_VAL_ERROR;
                }
                break;

        case TERM_NUMBER:
                if (a_value->content.num) {
                        status = init_style_font_size_field (a_style);
                        g_return_val_if_fail (status == CR_OK, status);

                        a_style->font_size.sv.type = ABSOLUTE_FONT_SIZE;
                        cr_num_copy (&a_style->font_size.sv.value.absolute,
                                     a_value->content.num) ;
                }
                break;

        default:
                status = init_style_font_size_field (a_style);
                return CR_UNKNOWN_PROP_VAL_ERROR;
        }
        return CR_OK;
}

static enum CRStatus
set_prop_font_style_from_value (CRStyle * a_style, CRTerm * a_value)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_value->type) {
        case TERM_IDENT:
                if (a_value->content.str 
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
                        if (!strcmp (a_value->content.str->stryng->str, "normal")) {
                                a_style->font_style = FONT_STYLE_NORMAL;
                        } else if (!strcmp
                                   (a_value->content.str->stryng->str,
				    "italic")) {
                                a_style->font_style = FONT_STYLE_ITALIC;
                        } else if (!strcmp
                                   (a_value->content.str->stryng->str,
				    "oblique")) {
                                a_style->font_style = FONT_STYLE_OBLIQUE;
                        } else if (!strcmp
                                   (a_value->content.str->stryng->str,
				    "inherit")) {
				a_style->font_style = FONT_STYLE_INHERIT;
                        } else {
                                status = CR_UNKNOWN_PROP_VAL_ERROR;
                        }
                }
                break;

        default:
                status = CR_UNKNOWN_PROP_VAL_ERROR;
                break;
        }

        return status;
}

static enum CRStatus
set_prop_font_weight_from_value (CRStyle * a_style, CRTerm * a_value)
{
        enum CRStatus status = CR_OK;

        g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

        switch (a_value->type) {
        case TERM_IDENT:
                if (a_value->content.str 
                    && a_value->content.str->stryng
                    && a_value->content.str->stryng->str) {
                        if (!strcmp (a_value->content.str->stryng->str, 
                                     "normal")) {
                                a_style->font_weight = FONT_WEIGHT_NORMAL;
                        } else if (!strcmp (a_value->content.str->stryng->str,
                                            "bold")) {
                                a_style->font_weight = FONT_WEIGHT_BOLD;
                        } else if (!strcmp (a_value->content.str->stryng->str,
                                            "bolder")) {
                                a_style->font_weight = FONT_WEIGHT_BOLDER;
                        } else if (!strcmp (a_value->content.str->stryng->str,
                                            "lighter")) {
                                a_style->font_weight = FONT_WEIGHT_LIGHTER;
			} else if (!strcmp (a_value->content.str->stryng->str,
                                            "inherit")) {
                                a_style->font_weight = FONT_WEIGHT_INHERIT;

                        } else {
                                status = CR_UNKNOWN_PROP_VAL_ERROR;
                        }

                }
                break;

        case TERM_NUMBER:
                if (a_value->content.num
                    && (a_value->content.num->type == NUM_GENERIC
                        || a_value->content.num->type == NUM_AUTO)) {
                        if (a_value->content.num->val <= 150) {
                                a_style->font_weight = FONT_WEIGHT_100;
                        } else if (a_value->content.num->val <= 250) {
                                a_style->font_weight = FONT_WEIGHT_200;
                        } else if (a_value->content.num->val <= 350) {
                                a_style->font_weight = FONT_WEIGHT_300;
                        } else if (a_value->content.num->val <= 450) {
                                a_style->font_weight = FONT_WEIGHT_400;
                        } else if (a_value->content.num->val <= 550) {
                                a_style->font_weight = FONT_WEIGHT_500;
                        } else if (a_value->content.num->val <= 650) {
                                a_style->font_weight = FONT_WEIGHT_600;
                        } else if (a_value->content.num->val <= 750) {
                                a_style->font_weight = FONT_WEIGHT_700;
                        } else if (a_value->content.num->val <= 850) {
                                a_style->font_weight = FONT_WEIGHT_800;
                        } else {
                                a_style->font_weight = FONT_WEIGHT_900;
                        }
                }
                break;

        default:
                status = CR_UNKNOWN_PROP_VAL_ERROR;
                break;
        }

        return status;
}

static enum CRStatus
set_prop_white_space_from_value (CRStyle * a_style, CRTerm * a_value)
{
	enum CRStatus status = CR_OK;

	g_return_val_if_fail (a_style && a_value, CR_BAD_PARAM_ERROR);

	switch (a_value->type) {
	case TERM_IDENT:
		if (a_value->content.str && a_value->content.str->stryng) {
			if (!strcmp (a_value->content.str->stryng->str, "normal")) {
				a_style->white_space = WHITE_SPACE_NORMAL;
			} else if (!strcmp (a_value->content.str->stryng->str, 
                                            "pre")) {
				a_style->white_space = WHITE_SPACE_PRE;
			} else if (!strcmp (a_value->content.str->stryng->str,
                                            "nowrap")) {
				a_style->white_space = WHITE_SPACE_NOWRAP;
			} else if (!strcmp (a_value->content.str->stryng->str,
                                            "inherit")) {
				a_style->white_space = WHITE_SPACE_INHERIT;
			} else {
				status = CR_UNKNOWN_PROP_VAL_ERROR;
			}
		}
		break;
	default:
		status = CR_UNKNOWN_PROP_VAL_ERROR;
		break;
	}

	return status;
}

/******************
 *Public methods
 ******************/

/**
 *Default constructor of #CRStyle.
 *@param a_set_props_to_initial_values if TRUE, the style properties
 *will be set to the default values. Only the style properties of the
 *root box should be set to their initial values.
 *Otherwise, the style values are set to their default value.
 *Read the CSS2 spec, chapters 6.1.1 to 6.2.
 */
CRStyle *
cr_style_new (gboolean a_set_props_to_initial_values)
{
        CRStyle *result = NULL;

        result = g_try_malloc (sizeof (CRStyle));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRStyle));
        gv_prop_hash_ref_count++;

        if (a_set_props_to_initial_values == TRUE) {
                cr_style_set_props_to_initial_values (result);
        } else {
                cr_style_set_props_to_default_values (result);
        }

        return result;
}

/**
 *Sets the style properties to their default values according to the css2 spec
 * i.e inherit if the property is inherited, its initial value otherwise.
 *@param a_this the current instance of #CRStyle.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus 
cr_style_set_props_to_default_values (CRStyle * a_this)
{
	glong i = 0;

	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
	
	for (i = 0; i < NB_NUM_PROPS; i++)
	{
		switch (i)
		{
		case NUM_PROP_WIDTH:
		case NUM_PROP_TOP:
		case NUM_PROP_RIGHT:
		case NUM_PROP_BOTTOM:
		case NUM_PROP_LEFT:
			cr_num_set (&a_this->num_props[i].sv, 0, NUM_AUTO);
			break;

		case NUM_PROP_PADDING_TOP:
		case NUM_PROP_PADDING_RIGHT:
		case NUM_PROP_PADDING_BOTTOM:
		case NUM_PROP_PADDING_LEFT:
		case NUM_PROP_BORDER_TOP:
		case NUM_PROP_BORDER_RIGHT:
		case NUM_PROP_BORDER_BOTTOM:
		case NUM_PROP_BORDER_LEFT:
		case NUM_PROP_MARGIN_TOP:
		case NUM_PROP_MARGIN_RIGHT:
		case NUM_PROP_MARGIN_BOTTOM:
		case NUM_PROP_MARGIN_LEFT:
			cr_num_set (&a_this->num_props[i].sv,
				    0, NUM_LENGTH_PX);
			break;

		default:
			cr_utils_trace_info ("Unknown property");
			break;
		}
	}

	for (i = 0; i < NB_RGB_PROPS; i++) {
                
		switch (i) {
			/*default foreground color is black */
		case RGB_PROP_COLOR:
			/*
                         *REVIEW: color is inherited and the default value is
			 *ua dependant.
                         */
			cr_rgb_set_to_inherit (&a_this->rgb_props[i].sv,
                                               TRUE) ;
			break;

			/*default background color is white */
		case RGB_PROP_BACKGROUND_COLOR:
			/* TODO: the default value should be transparent */
			cr_rgb_set (&a_this->rgb_props[i].sv,
				    255, 255, 255, FALSE);
                        cr_rgb_set_to_transparent (&a_this->rgb_props[i].sv,
                                                   TRUE) ;
			break;

		default:
			/* 
                         *TODO: for BORDER_COLOR the initial value should
			 * be the same as COLOR 
                         */
			cr_rgb_set (&a_this->rgb_props[i].sv, 0, 0, 0,
				    FALSE);
			break;
		}
	}

	for (i = 0; i < NB_BORDER_STYLE_PROPS; i++) {
		a_this->border_style_props[i] = BORDER_STYLE_NONE;
	}

	a_this->display = DISPLAY_INLINE;
	a_this->position = POSITION_STATIC;
	a_this->float_type = FLOAT_NONE;
	a_this->parent_style = NULL;
	a_this->font_style = FONT_STYLE_INHERIT;
	a_this->font_variant = FONT_VARIANT_INHERIT;
	a_this->font_weight = FONT_WEIGHT_INHERIT;
	a_this->font_family = NULL;
        
        cr_font_size_set_to_inherit (&a_this->font_size.sv) ;
        cr_font_size_clear (&a_this->font_size.cv) ;
        cr_font_size_clear (&a_this->font_size.av) ;

        /* To make the inheritance resolution possible and efficient */
        a_this->inherited_props_resolved = FALSE ;
	return CR_OK;
}

/**
 *Sets the style properties to their initial value according to the css2 spec.
 *This function should be used to initialize the style of the root element
 *of an xml tree.
 *Some properties are user agent dependant like font-family, and
 *are not initialized, read the spec to make you renderer compliant.
 *@param a_this the current instance of #CRStyle.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus 
cr_style_set_props_to_initial_values (CRStyle *a_this)
{
        glong i = 0;

        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        for (i = 0; i < NB_NUM_PROPS; i++) {
                switch (i) {
                case NUM_PROP_WIDTH:
                        cr_num_set (&a_this->num_props[i].sv, 800,
                                    NUM_LENGTH_PX) ;
                        break ;
                case NUM_PROP_TOP:
                case NUM_PROP_RIGHT:
                case NUM_PROP_BOTTOM:
                case NUM_PROP_LEFT:
                        cr_num_set (&a_this->num_props[i].sv, 0, NUM_AUTO);
                        break;

                case NUM_PROP_PADDING_TOP:
                case NUM_PROP_PADDING_RIGHT:
                case NUM_PROP_PADDING_BOTTOM:
                case NUM_PROP_PADDING_LEFT:
                case NUM_PROP_BORDER_TOP:
                case NUM_PROP_BORDER_RIGHT:
                case NUM_PROP_BORDER_BOTTOM:
                case NUM_PROP_BORDER_LEFT:
                case NUM_PROP_MARGIN_TOP:
                case NUM_PROP_MARGIN_RIGHT:
                case NUM_PROP_MARGIN_BOTTOM:
                case NUM_PROP_MARGIN_LEFT:
                        cr_num_set (&a_this->num_props[i].sv,
                                    0, NUM_LENGTH_PX);
                        break;

                default:
                        cr_utils_trace_info ("Unknown property");
                        break;
                }
        }

        for (i = 0; i < NB_RGB_PROPS; i++) {

                switch (i) {
                        /*default foreground color is black */
                case RGB_PROP_COLOR:
                        cr_rgb_set (&a_this->rgb_props[i].sv, 0, 0, 0, FALSE);
                        break;

                        /*default background color is white */
                case RGB_PROP_BACKGROUND_COLOR:
                        cr_rgb_set (&a_this->rgb_props[i].sv,
                                    255, 255, 255, FALSE);
                        cr_rgb_set_to_transparent (&a_this->rgb_props[i].sv,
                                                   TRUE) ;                        
                        break;
                default:
                        cr_rgb_set (&a_this->rgb_props[i].sv, 0, 0, 0, FALSE);
                        break;
                }
        }

        for (i = 0; i < NB_BORDER_STYLE_PROPS; i++) {
                a_this->border_style_props[i] = BORDER_STYLE_NONE;
        }

        a_this->display = DISPLAY_BLOCK;
        a_this->position = POSITION_STATIC;
        a_this->float_type = FLOAT_NONE;
        a_this->font_style = FONT_STYLE_NORMAL;
        a_this->font_variant = FONT_VARIANT_NORMAL;
        a_this->font_weight = FONT_WEIGHT_NORMAL;
        a_this->font_stretch = FONT_STRETCH_NORMAL;
	a_this->white_space = WHITE_SPACE_NORMAL;
        cr_font_size_set_predefined_absolute_font_size
                (&a_this->font_size.sv, FONT_SIZE_MEDIUM) ;
        a_this->inherited_props_resolved = FALSE ;

        return CR_OK;
}

/**
 *Resolves the inherited properties.
 *The function sets the "inherited" properties to either the value of
 *their parent properties.
 *This function is *NOT* recursive. So the inherited properties of
 *the parent style must have been resolved prior to calling this function.
 *@param a_this the instance where 
 *@return CR_OK if a root node is found and the propagation is successful,
 *an error code otherwise
 */
enum CRStatus 
cr_style_resolve_inherited_properties (CRStyle *a_this)
{
	enum CRStatus ret = CR_OK;
	glong i = 0;

	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);
	g_return_val_if_fail (a_this->parent_style, CR_BAD_PARAM_ERROR) ;

        if (a_this->inherited_props_resolved == TRUE)
                return CR_OK ;

        for (i=0 ; i < NB_NUM_PROPS ;i++) {
                if (a_this->num_props[i].sv.type == NUM_INHERIT) {
                        cr_num_copy (&a_this->num_props[i].cv,
                                     &a_this->parent_style->num_props[i].cv);
                }
        }
	for (i=0; i < NB_RGB_PROPS; i++) {
		if (cr_rgb_is_set_to_inherit (&a_this->rgb_props[i].sv) == TRUE) {
			cr_rgb_copy (
				&a_this->rgb_props[i].cv,
				&a_this->parent_style->rgb_props[i].cv);
		}
	}
	for (i = 0; i < NB_BORDER_STYLE_PROPS; i++) {
		if (a_this->border_style_props[i] == BORDER_STYLE_INHERIT) {
			a_this->border_style_props[i] =
			  a_this->parent_style->border_style_props[i];
		}
	}

	if (a_this->display == DISPLAY_INHERIT) {
		a_this->display = a_this->parent_style->display;
	}
	if (a_this->position == POSITION_INHERIT) {
		a_this->position = a_this->parent_style->position;
	}
	if (a_this->float_type == FLOAT_INHERIT) {
		a_this->float_type = a_this->parent_style->float_type;
	}
	if (a_this->font_style == FONT_STYLE_INHERIT) {
		a_this->font_style = a_this->parent_style->font_style;
	}
	if (a_this->font_variant == FONT_VARIANT_INHERIT) {
		a_this->font_variant = a_this->parent_style->font_variant;
	}
	if (a_this->font_weight == FONT_WEIGHT_INHERIT) {
		a_this->font_weight = a_this->parent_style->font_weight;
	}
	if (a_this->font_stretch == FONT_STRETCH_INHERIT) {
		a_this->font_stretch = a_this->parent_style->font_stretch;
	}
	/*NULL is inherit marker for font_famiy*/
	if (a_this->font_family == NULL)  {
		a_this->font_family = a_this->parent_style->font_family;
	}
        if (a_this->font_size.sv.type == INHERITED_FONT_SIZE) {
                cr_font_size_copy (&a_this->font_size.cv,
                                   &a_this->parent_style->font_size.cv) ;
        }
        a_this->inherited_props_resolved = TRUE ;
	return ret;
}

/**
 *Walks through a css2 property declaration, and populated the
 *according field(s) in the #CRStyle structure.
 *If the properties or their value(s) are/is not known, 
 *sets the corresponding field(s) of #CRStyle to its/their default 
 *value(s)
 *@param a_this the instance of #CRStyle to set.
 *@param a_decl the declaration from which the #CRStyle fields are set.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_style_set_style_from_decl (CRStyle * a_this, CRDeclaration * a_decl)
{
        CRTerm *value = NULL;
        enum CRStatus status = CR_OK;

        enum CRPropertyID prop_id = PROP_ID_NOT_KNOWN;

        g_return_val_if_fail (a_this && a_decl
                              && a_decl
                              && a_decl->property
                              && a_decl->property->stryng
                              && a_decl->property->stryng->str,
                              CR_BAD_PARAM_ERROR);

        prop_id = cr_style_get_prop_id
                ((const guchar *) a_decl->property->stryng->str);

        value = a_decl->value;
        switch (prop_id) {
        case PROP_ID_PADDING_TOP:
                status = set_prop_padding_x_from_value
                        (a_this, value, DIR_TOP);
                break;

        case PROP_ID_PADDING_RIGHT:
                status = set_prop_padding_x_from_value
                        (a_this, value, DIR_RIGHT);
                break;
        case PROP_ID_PADDING_BOTTOM:
                status = set_prop_padding_x_from_value
                        (a_this, value, DIR_BOTTOM);
                break;

        case PROP_ID_PADDING_LEFT:
                status = set_prop_padding_x_from_value
                        (a_this, value, DIR_LEFT);
                break;

        case PROP_ID_PADDING:
                status = set_prop_padding_from_value (a_this, value) ;
                break;

        case PROP_ID_BORDER_TOP_WIDTH:
                status = set_prop_border_x_width_from_value (a_this, value,
                                                             DIR_TOP);
                break;

        case PROP_ID_BORDER_RIGHT_WIDTH:
                status = set_prop_border_x_width_from_value (a_this, value,
                                                             DIR_RIGHT);
                break;

        case PROP_ID_BORDER_BOTTOM_WIDTH:
                status = set_prop_border_x_width_from_value (a_this, value,
                                                             DIR_BOTTOM);
                break;

        case PROP_ID_BORDER_LEFT_WIDTH:
                status = set_prop_border_x_width_from_value (a_this, value,
                                                             DIR_LEFT);
                break;

        case PROP_ID_BORDER_WIDTH:
                status = set_prop_border_width_from_value (a_this, value) ;
                break ;

        case PROP_ID_BORDER_TOP_STYLE:
                status = set_prop_border_x_style_from_value (a_this, value,
                                                             DIR_TOP);
                break;

        case PROP_ID_BORDER_RIGHT_STYLE:
                status = set_prop_border_x_style_from_value (a_this, value,
                                                             DIR_RIGHT);
                break;

        case PROP_ID_BORDER_BOTTOM_STYLE:
                status = set_prop_border_x_style_from_value (a_this, value,
                                                             DIR_BOTTOM);
                break;

        case PROP_ID_BORDER_LEFT_STYLE:
                status = set_prop_border_x_style_from_value (a_this, value,
                                                             DIR_LEFT);
                break;

        case PROP_ID_BORDER_STYLE:
                status = set_prop_border_style_from_value (a_this, value) ;
                break ;

        case PROP_ID_BORDER_TOP_COLOR:
                status = set_prop_border_x_color_from_value (a_this, value,
                                                             DIR_TOP);
                break;

        case PROP_ID_BORDER_RIGHT_COLOR:
                status = set_prop_border_x_color_from_value (a_this, value,
                                                             DIR_RIGHT);
                break;

        case PROP_ID_BORDER_BOTTOM_COLOR:
                status = set_prop_border_x_color_from_value (a_this, value,
                                                             DIR_BOTTOM);
                break;

        case PROP_ID_BORDER_LEFT_COLOR:
                status = set_prop_border_x_color_from_value (a_this, value,
                                                             DIR_BOTTOM);
                break;

        case PROP_ID_BORDER_TOP:
                status = set_prop_border_x_from_value (a_this, value,
                                                       DIR_TOP);
                break;

        case PROP_ID_BORDER_RIGHT:
                status = set_prop_border_x_from_value (a_this, value,
                                                       DIR_RIGHT);
                break;

        case PROP_ID_BORDER_BOTTOM:
                status = set_prop_border_x_from_value (a_this, value,
                                                       DIR_BOTTOM);
                break;

        case PROP_ID_BORDER_LEFT:
                status = set_prop_border_x_from_value (a_this, value,
                                                       DIR_LEFT);
                break;

        case PROP_ID_MARGIN_TOP:
                status = set_prop_margin_x_from_value (a_this, value,
                                                       DIR_TOP);
                break;

        case PROP_ID_BORDER:
                status = set_prop_border_from_value (a_this, value);
                break;

        case PROP_ID_MARGIN_RIGHT:
                status = set_prop_margin_x_from_value (a_this, value,
                                                       DIR_RIGHT);
                break;

        case PROP_ID_MARGIN_BOTTOM:
                status = set_prop_margin_x_from_value (a_this, value,
                                                       DIR_BOTTOM);
                break;

        case PROP_ID_MARGIN_LEFT:
                status = set_prop_margin_x_from_value (a_this, value,
                                                       DIR_LEFT);
                break;

        case PROP_ID_MARGIN:
                status = set_prop_margin_from_value (a_this, value);
                break;

        case PROP_ID_DISPLAY:
                status = set_prop_display_from_value (a_this, value);
                break;

        case PROP_ID_POSITION:
                status = set_prop_position_from_value (a_this, value);
                break;

        case PROP_ID_TOP:
                status = set_prop_x_from_value (a_this, value, DIR_TOP);
                break;

        case PROP_ID_RIGHT:
                status = set_prop_x_from_value (a_this, value, DIR_RIGHT);
                break;

        case PROP_ID_BOTTOM:
                status = set_prop_x_from_value (a_this, value, DIR_BOTTOM);
                break;

        case PROP_ID_LEFT:
                status = set_prop_x_from_value (a_this, value, DIR_LEFT);
                break;

        case PROP_ID_FLOAT:
                status = set_prop_float (a_this, value);
                break;

        case PROP_ID_WIDTH:
                status = set_prop_width (a_this, value);
                break;

        case PROP_ID_COLOR:
                status = set_prop_color (a_this, value);
                break;

        case PROP_ID_BACKGROUND_COLOR:
                status = set_prop_background_color (a_this, value);
                break;

        case PROP_ID_FONT_FAMILY:
                status = set_prop_font_family_from_value (a_this, value);
                break;

        case PROP_ID_FONT_SIZE:
                status = set_prop_font_size_from_value (a_this, value);
                break;

        case PROP_ID_FONT_STYLE:
                status = set_prop_font_style_from_value (a_this, value);
                break;

        case PROP_ID_FONT_WEIGHT:
                status = set_prop_font_weight_from_value (a_this, value);
                break;

	case PROP_ID_WHITE_SPACE:
		status = set_prop_white_space_from_value(a_this, value);
		break;

        default:
                return CR_UNKNOWN_TYPE_ERROR;

        }

        return status;
}

/**
 *Increases the reference count
 *of the current instance of #CRStyle.
 *@param a_this the current instance of #CRStyle.
 *@return CR_OK upon successfull completion, an error code
 *otherwise.
 */
enum CRStatus
cr_style_ref (CRStyle * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        a_this->ref_count++;
        return CR_OK;
}

/**
 *Decreases the reference count of
 *the current instance of #CRStyle.
 *If the reference count reaches 0, the
 *instance of #CRStyle is destoyed.
 *@param a_this the current instance of #CRStyle.
 *@return TRUE if the instance has been destroyed, FALSE
 *otherwise.
 */
gboolean
cr_style_unref (CRStyle * a_this)
{
        g_return_val_if_fail (a_this, FALSE);

        if (a_this->ref_count)
                a_this->ref_count--;

        if (!a_this->ref_count) {
                cr_style_destroy (a_this);
                return TRUE;
        }

        return FALSE;
}

/**
 *Duplicates the current instance of #CRStyle .
 *The newly created instance of #CRStyle must be
 *freed using cr_style_destroy ().
 *@param a_this the current instance of #CRStyle.
 *@return the newly duplicated instance of #CRStyle.
 */
CRStyle *
cr_style_dup (CRStyle * a_this)
{
        CRStyle *result = NULL;

        g_return_val_if_fail (a_this, NULL);

        result = cr_style_new (FALSE);
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        cr_style_copy (result, a_this);
        return result;
}

/**
 *Copies a style data structure into another.
 *TODO: this is actually broken because it's based
 *on memcpy although some data stuctures of CRStyle should
 *be properly duplicated.
 *@param a_dest the destination style datastructure
 *@param a_src the source style datastructure.
 *@return CR_OK upon succesfull completion, an error code otherwise
 */
enum CRStatus
cr_style_copy (CRStyle * a_dest, CRStyle * a_src)
{
        g_return_val_if_fail (a_dest && a_src, CR_BAD_PARAM_ERROR);

        memcpy (a_dest, a_src, sizeof (CRStyle));
        return CR_OK;
}

/**
 *dump a CRNumpPropVal in a string.
 *@param a_prop_val the numerical property value to dump
 *@param a_str the string to dump the numerical propertie into.
 *Note that the string value is appended to a_str.
 *@param a_nb_indent the number white chars of indentation.
 */
enum CRStatus
cr_style_num_prop_val_to_string (CRNumPropVal * a_prop_val,
                                 GString * a_str, guint a_nb_indent)
{
        enum CRStatus status = CR_OK;
        guchar *tmp_str = NULL;
        GString *str = NULL;

        g_return_val_if_fail (a_prop_val && a_str, CR_BAD_PARAM_ERROR);

        str = g_string_new (NULL);
        cr_utils_dump_n_chars2 (' ', str, a_nb_indent);
        g_string_append (str, "NumPropVal {");
        tmp_str = cr_num_to_string (&a_prop_val->sv);
        if (!tmp_str) {
                status = CR_ERROR;
                goto cleanup;
        }
        g_string_append_printf (str, "sv: %s ", tmp_str);
        g_free (tmp_str);
        tmp_str = NULL;
        
        tmp_str = cr_num_to_string (&a_prop_val->cv);
        if (!tmp_str) {
                status = CR_ERROR;
                goto cleanup;
        }
        g_string_append_printf (str, "cv: %s ", tmp_str);
        g_free (tmp_str);
        tmp_str = NULL;

        tmp_str = cr_num_to_string (&a_prop_val->av);
        if (!tmp_str) {
                status = CR_ERROR;
                goto cleanup;
        }
        g_string_append_printf (str, "av: %s ", tmp_str);
        g_free (tmp_str);
        tmp_str = NULL;
        g_string_append (str, "}");
        g_string_append (a_str, str->str);
        status = CR_OK;
      cleanup:

        if (tmp_str) {
                g_free (tmp_str);
                tmp_str = NULL;
        }
        if (str) {
                g_string_free (str, TRUE);
        }
        return status;
}

enum CRStatus
cr_style_rgb_prop_val_to_string (CRRgbPropVal * a_prop_val,
                                 GString * a_str, guint a_nb_indent)
{
        enum CRStatus status = CR_OK;
        guchar *tmp_str = NULL;
        GString *str = NULL;

        g_return_val_if_fail (a_prop_val && a_str, CR_BAD_PARAM_ERROR);

        str = g_string_new (NULL);

        cr_utils_dump_n_chars2 (' ', str, a_nb_indent);
        g_string_append (str, "RGBPropVal {");
        tmp_str = cr_rgb_to_string (&a_prop_val->sv);
        if (!tmp_str) {
                status = CR_ERROR;
                goto cleanup;
        }
        g_string_append_printf (str, "sv: %s ", tmp_str);
        g_free (tmp_str);
        tmp_str = NULL;
        tmp_str = cr_rgb_to_string (&a_prop_val->cv);
        if (!tmp_str) {
                status = CR_ERROR;
                goto cleanup;
        }
        g_string_append_printf (str, "cv: %s ", tmp_str);
        g_free (tmp_str);
        tmp_str = NULL;
        tmp_str = cr_rgb_to_string (&a_prop_val->av);
        if (!tmp_str) {
                status = CR_ERROR;
                goto cleanup;
        }
        g_string_append_printf (str, "av: %s ", tmp_str);
        g_free (tmp_str);
        tmp_str = NULL;

        g_string_append (str, "}");
        g_string_append (a_str, str->str);
        status = CR_OK;
      cleanup:

        if (tmp_str) {
                g_free (tmp_str);
                tmp_str = NULL;
        }
        if (str) {
                g_string_free (str, TRUE);
        }
        return status;
}

enum CRStatus
cr_style_border_style_to_string (enum CRBorderStyle a_prop,
                                 GString * a_str, guint a_nb_indent)
{
        gchar *str = NULL;

        g_return_val_if_fail (a_str, CR_BAD_PARAM_ERROR);

        switch (a_prop) {
        case BORDER_STYLE_NONE:
                str = (gchar *) "border-style-none";
                break;
        case BORDER_STYLE_HIDDEN:
                str = (gchar *) "border-style-hidden";
                break;
        case BORDER_STYLE_DOTTED:
                str = (gchar *) "border-style-dotted";
                break;
        case BORDER_STYLE_DASHED:
                str = (gchar *) "border-style-dashed";
                break;
        case BORDER_STYLE_SOLID:
                str = (gchar *) "border-style-solid";
                break;
        case BORDER_STYLE_DOUBLE:
                str = (gchar *) "border-style-double";
                break;
        case BORDER_STYLE_GROOVE:
                str = (gchar *) "border-style-groove";
                break;
        case BORDER_STYLE_RIDGE:
                str = (gchar *) "border-style-ridge";
                break;
        case BORDER_STYLE_INSET:
                str = (gchar *) "border-style-inset";
                break;
        case BORDER_STYLE_OUTSET:
                str = (gchar *) "border-style-outset";
                break;
        default:
                str = (gchar *) "unknown border style";
                break;
        }
        cr_utils_dump_n_chars2 (' ', a_str, a_nb_indent);
        g_string_append (a_str, str);
        return CR_OK;
}

enum CRStatus
cr_style_display_type_to_string (enum CRDisplayType a_code,
                                 GString * a_str, guint a_nb_indent)
{
        gchar *str = NULL;

        g_return_val_if_fail (a_str, CR_BAD_PARAM_ERROR);

        switch (a_code) {
        case DISPLAY_NONE:
                str = (gchar *) "display-none";
                break;
        case DISPLAY_INLINE:
                str = (gchar *) "display-inline";
                break;
        case DISPLAY_BLOCK:
                str = (gchar *) "display-block";
                break;
        case DISPLAY_LIST_ITEM:
                str = (gchar *) "display-list-item";
                break;
        case DISPLAY_RUN_IN:
                str = (gchar *) "display-run-in";
                break;
        case DISPLAY_COMPACT:
                str = (gchar *) "display-compact";
                break;
        case DISPLAY_MARKER:
                str = (gchar *) "display-marker";
                break;
        case DISPLAY_TABLE:
                str = (gchar *) "display-table";
                break;
        case DISPLAY_INLINE_TABLE:
                str = (gchar *) "display-inline-table";
                break;
        case DISPLAY_TABLE_ROW_GROUP:
                str = (gchar *) "display-table-row-group";
                break;
        case DISPLAY_TABLE_HEADER_GROUP:
                str = (gchar *) "display-table-header-group";
                break;
        case DISPLAY_TABLE_FOOTER_GROUP:
                str = (gchar *) "display-table-footer-group";
                break;
        case DISPLAY_TABLE_ROW:
                str = (gchar *) "display-table-row";
                break;
        case DISPLAY_TABLE_COLUMN_GROUP:
                str = (gchar *) "display-table-column-group";
                break;
        case DISPLAY_TABLE_COLUMN:
                str = (gchar *) "display-table-column";
                break;
        case DISPLAY_TABLE_CELL:
                str = (gchar *) "display-table-cell";
                break;
        case DISPLAY_TABLE_CAPTION:
                str = (gchar *) "display-table-caption";
                break;
        case DISPLAY_INHERIT:
                str = (gchar *) "display-inherit";
                break;
        default:
                str = (gchar *) "unknown display property";
                break;
        }
        cr_utils_dump_n_chars2 (' ', a_str, a_nb_indent);
        g_string_append (a_str, str);
        return CR_OK;

}

enum CRStatus
cr_style_position_type_to_string (enum CRPositionType a_code,
                                  GString * a_str, guint a_nb_indent)
{
        gchar *str = NULL;

        g_return_val_if_fail (a_str, CR_BAD_PARAM_ERROR);

        switch (a_code) {
        case POSITION_STATIC:
                str = (gchar *) "position-static";
                break;
        case POSITION_RELATIVE:
                str = (gchar *) "position-relative";
                break;
        case POSITION_ABSOLUTE:
                str = (gchar *) "position-absolute";
                break;
        case POSITION_FIXED:
                str = (gchar *) "position-fixed";
                break;
        case POSITION_INHERIT:
                str = (gchar *) "position-inherit";
                break;
        default:
                str = (gchar *) "unknown static property";
        }
        cr_utils_dump_n_chars2 (' ', a_str, a_nb_indent);
        g_string_append (a_str, str);
        return CR_OK;
}

enum CRStatus
cr_style_float_type_to_string (enum CRFloatType a_code,
                               GString * a_str, guint a_nb_indent)
{
        gchar *str = NULL;

        g_return_val_if_fail (a_str, CR_BAD_PARAM_ERROR);

        switch (a_code) {
        case FLOAT_NONE:
                str = (gchar *) "float-none";
                break;
        case FLOAT_LEFT:
                str = (gchar *) "float-left";
                break;
        case FLOAT_RIGHT:
                str = (gchar *) "float-right";
                break;
        case FLOAT_INHERIT:
                str = (gchar *) "float-inherit";
                break;
        default:
                str = (gchar *) "unknown float property value";
                break;
        }
        cr_utils_dump_n_chars2 (' ', a_str, a_nb_indent);
        g_string_append (a_str, str);
        return CR_OK;
}

enum CRStatus
cr_style_white_space_type_to_string (enum CRWhiteSpaceType a_code,
                                     GString * a_str, guint a_nb_indent)
{
        gchar *str = NULL;

        g_return_val_if_fail (a_str, CR_BAD_PARAM_ERROR);

        switch (a_code) {
        case WHITE_SPACE_NORMAL:
                str = (gchar *) "normal";
		break;
	case WHITE_SPACE_PRE:
		str = (gchar *) "pre";
		break;
	case WHITE_SPACE_NOWRAP:
		str = (gchar *) "nowrap";
		break;
	case WHITE_SPACE_INHERIT:
		str = (gchar *) "inherited";
		break;
	default:
		str = (gchar *) "unknown white space property value";
		break;
	}
	cr_utils_dump_n_chars2 (' ', a_str, a_nb_indent);
	g_string_append (a_str, str);
	return CR_OK;
}
 
/**
 *Serializes in instance of #CRStyle into
 *a string
 *@param a_this the instance of #CRStyle to serialize
 *@param a_str the string to serialise the style into.
 *if *a_str is NULL, a new GString is instanciated, otherwise
 *the style serialisation is appended to the existed *a_str
 *@param the number of white space char to use for indentation.
 *@return CR_OK upon successful completion, an error code otherwise.
 */
enum CRStatus
cr_style_to_string (CRStyle * a_this, GString ** a_str, guint a_nb_indent)
{
        const gint INTERNAL_INDENT = 2;
        gint indent = a_nb_indent + INTERNAL_INDENT;
        gchar *tmp_str = NULL;
        GString *str = NULL;
        gint i = 0;

        g_return_val_if_fail (a_this && a_str, CR_BAD_PARAM_ERROR);

        if (!*a_str) {
                str = g_string_new (NULL);
        } else {
                str = *a_str;
        }
        cr_utils_dump_n_chars2 (' ', str, a_nb_indent);
        g_string_append (str, "style {\n");

        /*loop over the num_props and to_string() them */
        for (i = NUM_PROP_TOP; i < NB_NUM_PROPS; i++) {
                /*
                 *to_string() the name of the num_prop
                 *(using num_prop_code_to_string)
                 *before outputing it value
                 */
                cr_utils_dump_n_chars2 (' ', str, indent);
                tmp_str = (gchar *) num_prop_code_to_string (i);
                if (tmp_str) {
                        g_string_append_printf (str, "%s: ", tmp_str);
                } else {
                        g_string_append (str, "NULL");
                }
                tmp_str = NULL;
                cr_style_num_prop_val_to_string (&a_this->num_props[i], str,
                                                 a_nb_indent +
                                                 INTERNAL_INDENT);
                g_string_append (str, "\n");
        }
        /*loop over the rgb_props and to_string() them all */
        for (i = RGB_PROP_BORDER_TOP_COLOR; i < NB_RGB_PROPS; i++) {
                tmp_str = (gchar *) rgb_prop_code_to_string (i);
                cr_utils_dump_n_chars2 (' ', str, indent);
                if (tmp_str) {
                        g_string_append_printf (str, "%s: ", tmp_str);
                } else {
                        g_string_append (str, "NULL: ");
                }
                tmp_str = NULL;
                cr_style_rgb_prop_val_to_string (&a_this->rgb_props[i], str,
                                                 a_nb_indent +
                                                 INTERNAL_INDENT);
                g_string_append (str, "\n");
        }
        /*loop over the border_style_props and to_string() them */
        for (i = BORDER_STYLE_PROP_TOP; i < NB_BORDER_STYLE_PROPS; i++) {
                tmp_str = (gchar *) border_style_prop_code_to_string (i);
                cr_utils_dump_n_chars2 (' ', str, indent);
                if (tmp_str) {
                        g_string_append_printf (str, "%s: ", tmp_str);
                } else {
                        g_string_append (str, "NULL: ");
                }
                tmp_str = NULL;
                cr_style_border_style_to_string (a_this->
                                                 border_style_props[i], str,
                                                 0);
                g_string_append (str, "\n");
        }
        cr_utils_dump_n_chars2 (' ', str, indent);
        g_string_append (str, "display: ");
        cr_style_display_type_to_string (a_this->display, str, 0);
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        g_string_append (str, "position: ");
        cr_style_position_type_to_string (a_this->position, str, 0);
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        g_string_append (str, "float-type: ");
        cr_style_float_type_to_string (a_this->float_type, str, 0);
        g_string_append (str, "\n");

	cr_utils_dump_n_chars2 (' ', str, indent);
	g_string_append (str, "white-space: ");
	cr_style_white_space_type_to_string (a_this->white_space, str, 0);
	g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        g_string_append (str, "font-family: ");
        tmp_str = (gchar *) cr_font_family_to_string (a_this->font_family, TRUE);
        if (tmp_str) {
                g_string_append (str, tmp_str);
                g_free (tmp_str);
                tmp_str = NULL;
        } else {
                g_string_append (str, "NULL");
        }
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        tmp_str = cr_font_size_to_string (&a_this->font_size.sv);
        if (tmp_str) {
                g_string_append_printf (str, "font-size {sv:%s, ",
                                        tmp_str) ;
        } else {
                g_string_append (str, "font-size {sv:NULL, ");
        }
        tmp_str = cr_font_size_to_string (&a_this->font_size.cv);
        if (tmp_str) {
                g_string_append_printf (str, "cv:%s, ", tmp_str);
        } else {
                g_string_append (str, "cv:NULL, ");
        }
        tmp_str = cr_font_size_to_string (&a_this->font_size.av);
        if (tmp_str) {
                g_string_append_printf (str, "av:%s}", tmp_str);
        } else {
                g_string_append (str, "av:NULL}");
        }

        tmp_str = NULL;
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        tmp_str = cr_font_size_adjust_to_string (a_this->font_size_adjust);
        if (tmp_str) {
                g_string_append_printf (str, "font-size-adjust: %s", tmp_str);
        } else {
                g_string_append (str, "font-size-adjust: NULL");
        }
        tmp_str = NULL;
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        tmp_str = (gchar *) cr_font_style_to_string (a_this->font_style);
        if (tmp_str) {
                g_string_append_printf (str, "font-style: %s", tmp_str);
        } else {
                g_string_append (str, "font-style: NULL");
        }
        tmp_str = NULL;
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        tmp_str = (gchar *) cr_font_variant_to_string (a_this->font_variant);
        if (tmp_str) {
                g_string_append_printf (str, "font-variant: %s", tmp_str);
        } else {
                g_string_append (str, "font-variant: NULL");
        }
        tmp_str = NULL;
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        tmp_str = (gchar *) cr_font_weight_to_string (a_this->font_weight);
        if (tmp_str) {
                g_string_append_printf (str, "font-weight: %s", tmp_str);
        } else {
                g_string_append (str, "font-weight: NULL");
        }
        tmp_str = NULL;
        g_string_append (str, "\n");

        cr_utils_dump_n_chars2 (' ', str, indent);
        tmp_str = (gchar *) cr_font_stretch_to_string (a_this->font_stretch);
        if (tmp_str) {
                g_string_append_printf (str, "font-stretch: %s", tmp_str);
        } else {
                g_string_append (str, "font-stretch: NULL");
        }
        tmp_str = NULL;
        g_string_append (str, "\n");


        cr_utils_dump_n_chars2 (' ', str, a_nb_indent);
        g_string_append (str, "}");

        return CR_OK;
}

/**
 *Destructor of the #CRStyle class.
 *@param a_this the instance to destroy.
 */
void
cr_style_destroy (CRStyle * a_this)
{
        g_return_if_fail (a_this);

        g_free (a_this);
}

