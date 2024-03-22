/*
 * Copyright (C) 2018-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_LIMA_PARSER
#define H_LIMA_PARSER

static const char *PIPE_COMPARE_FUNC_STRING[] = {
   "NEVER",    /* 0 */
   "LESS",     /* 1 */
   "EQUAL",    /* 2 */
   "LEQUAL",   /* 3 */
   "GREATER",  /* 4 */
   "NOTEQUAL", /* 5 */
   "GEQUAL",   /* 6 */
   "ALWAYS",   /* 7 */
};

static const char *PIPE_STENCIL_OP_STRING[] = {
   "KEEP",      /* 0 */
   "REPLACE",   /* 1 */
   "ZERO",      /* 2 */
   "INVERT",    /* 3 */
   "INCR_WRAP", /* 4 */
   "DECR_WRAP", /* 5 */
   "INCR",      /* 6 */
   "DECR",      /* 7 */
};

static const char *PIPE_BLEND_FUNC_STRING[] = {
   "SUBTRACT",     /* 0 */
   "REV_SUBTRACT", /* 1 */
   "ADD",          /* 2 */
   "UNKNOWN_3",    /* 3 */
   "BLEND_MIN",    /* 4 */
   "BLEND_MAX",    /* 5 */
};

static const char *PIPE_BLENDFACTOR_STRING[] = {
   "SRC_COLOR",        /* 0 */
   "DST_COLOR",        /* 1 */
   "CONST_COLOR",      /* 2 */
   "ZERO",             /* 3 */
   "UNKNOWN_4",        /* 4 */
   "SRC2_COLOR",       /* 5 */
   "UNKNOWN_6",        /* 6 */
   "SRC_ALPHA_SAT",    /* 7 */
   "INV_SRC_COLOR",    /* 8 */
   "INV_DST_COLOR",    /* 9 */
   "INV_CONST_COLOR",  /* 10 */
   "ONE",              /* 11 */
   "UNKNOWN_12",       /* 12 */
   "INV_SRC2_COLOR",   /* 13 */
   "UNKNOWN_14",       /* 14 */
   "UNKNOWN_15",       /* 15 */
   "SRC_ALPHA",        /* 16 */
   "DST_ALPHA",        /* 17 */
   "CONST_ALPHA",      /* 18 */
   "UNKNOWN_19",       /* 19 */
   "UNKNOWN_20",       /* 20 */
   "SRC2_ALPHA",       /* 21 */
   "UNKNOWN_22",       /* 22 */
   "UNKNOWN_23",       /* 23 */
   "INV_SRC_ALPHA",    /* 24 */
   "INV_DST_ALPHA",    /* 25 */
   "INV_CONST_ALPHA",  /* 26 */
   "UNKNOWN_27",       /* 27 */
   "UNKNOWN_28",       /* 28 */
   "INV_SRC2_ALPHA",   /* 29 */
};

static const char *LIMA_WRAP_MODE_STRING[] = {
   "TEX_WRAP_REPEAT",                  /* 0 */
   "TEX_WRAP_CLAMP_TO_EDGE",           /* 1 */
   "TEX_WRAP_CLAMP",                   /* 2 */
   "TEX_WRAP_CLAMP_TO_BORDER",         /* 3 */
   "TEX_WRAP_MIRROR_REPEAT",           /* 4 */
   "TEX_WRAP_MIRROR_CLAMP_TO_EDGE",    /* 5 */
   "TEX_WRAP_MIRROR_CLAMP",            /* 6 */
   "TEX_WRAP_MIRROR_CLAMP_TO_BORDER",  /* 7 */
};

static inline const char
*lima_get_compare_func_string(int func) {
   if ((func >= 0) && (func <= 7))
      return PIPE_COMPARE_FUNC_STRING[func];
   else
      return "UNKNOWN";
}

static inline const char
*lima_get_stencil_op_string(int func) {
   if ((func >= 0) && (func <= 7))
      return PIPE_STENCIL_OP_STRING[func];
   else
      return "UNKNOWN";
}

static inline const char
*lima_get_blend_func_string(int func) {
   if ((func >= 0) && (func <= 5))
      return PIPE_BLEND_FUNC_STRING[func];
   else
      return "UNKNOWN";
}

static inline const char
*lima_get_blendfactor_string(int func) {
   if ((func >= 0) && (func <= 26))
      return PIPE_BLENDFACTOR_STRING[func];
   else
      return "UNKNOWN";
}

static inline const char
*lima_get_wrap_mode_string(int mode) {
   if ((mode >= 0) && (mode <= 7))
      return LIMA_WRAP_MODE_STRING[mode];
   else
      return "UNKNOWN";
}

void lima_parse_shader(FILE *fp, uint32_t *data, int size, bool is_frag);
void lima_parse_vs(FILE *fp, uint32_t *data, int size, uint32_t start);
void lima_parse_plbu(FILE *fp, uint32_t *data, int size, uint32_t start);
void lima_parse_render_state(FILE *fp, uint32_t *data, int size, uint32_t start);
void lima_parse_texture_descriptor(FILE *fp, uint32_t *data, int size, uint32_t start, uint32_t offset);

#endif
