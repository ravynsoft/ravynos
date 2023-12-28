/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <values.h>
#include <assert.h>

#include "comp_com.h"

/*
 * To add a new message _FORMAT_ please perform the following tasks:
 * 1) Insert it into the list below, with the matching comment.
 *    The table is sorted by parameter type.  In increasing order
 *    they are: String, Procedure, Variable, Loop, Region, Integer.
 * 2) Insert the corresponding information into the following
 *    procedures in this file: ccm_num_params(), ccm_paramlist_index(),
 *    ccm_param_primtype(), and ccm_param_hightype().
 * 3) If you are also creating a new high-type or primitive-type,
 *    extend the corresponding enum, update this comment and make sure
 *    to update any code in the analyzer, iropt, cg or ube that depends
 *    on knowing the limited set of types.
 */

typedef enum ccm_fmt {
    CCMFMT_NONE,		/* none */
    CCMFMT_S1,			/* s1 */
    CCMFMT_S1S2,		/* s1, s2 */
    CCMFMT_S1L2,		/* s1, l2 */
    CCMFMT_S1L2VV3,		/* s1, l2, v3, v4, ... */
    CCMFMT_S1R2VV3,		/* s1, r2, v3, v4, ... */
    CCMFMT_S1X2,		/* s1, x2 */
    CCMFMT_P1,			/* p1 */
    CCMFMT_P1S2,		/* p1, s2 */
    CCMFMT_P1S2P3,		/* p1, s2, p3 */
    CCMFMT_P1S2P3I4,		/* p1, s2, p3, i4 */
    CCMFMT_P1S2I3,		/* p1, s2, i3 */
    CCMFMT_P1P2,		/* p1, p2 */
    CCMFMT_P1L2,		/* p1, l2 */
    CCMFMT_P1I2,		/* p1, i2 */
    CCMFMT_P1I2L3,		/* p1, i2, l3 */
    CCMFMT_P1I2LL3,		/* p1, i2, l3, l4 ... */
    CCMFMT_P1I2I3,		/* p1, i2, i3 */
    CCMFMT_PP1,			/* p1, p2, ... */
    CCMFMT_V1,			/* v1 */
    CCMFMT_V1V2,		/* v1, v2 */
    CCMFMT_V1L2,		/* v1, l2 */
    CCMFMT_VV1,			/* v1, v2, ... */
    CCMFMT_L1,			/* l1 */
    CCMFMT_L1S2,		/* l1, s2 */
    CCMFMT_L1S2L3,		/* l1, s2, l3 */
    CCMFMT_L1P2,		/* l1, p2 */
    CCMFMT_L1P2I3,		/* l1, p2, i3 */
    CCMFMT_L1PP2,		/* l1, p2, p3, ... */
    CCMFMT_L1VV2,		/* l1, v2, v3, ... */
    CCMFMT_L1L2,		/* l1, l2 */
    CCMFMT_L1L2L3,		/* l1, l2, l3 */
    CCMFMT_LL1,			/* l1, l2, ... */
    CCMFMT_L1R2,		/* l1, r2 */
    CCMFMT_L1I2,		/* l1, i2 */
    CCMFMT_L1I2L3,		/* l1, i2, l3 */
    CCMFMT_L1I2LL3,		/* l1, i2, l3, l4, ... */
    CCMFMT_L1I2I3L4,		/* l1, i2, i3, l4 */
    CCMFMT_L1I2I3I4I5,		/* l1, i2, ..., i5 */
    CCMFMT_L1I2I3I4I5I6I7,	/* l1, i2, ..., i7 */
    CCMFMT_L1I2I3I4I5I6I7I8I9,	/* l1, i2, ..., i9 */
    CCMFMT_L1II2,		/* l1, i2, i3, ... */
    CCMFMT_R1,			/* r1 */
    CCMFMT_R1VV2,		/* r1, v2, v3, ... */
    CCMFMT_I1,			/* i1 */
    CCMFMT_I1P2I3,		/* i1, p2, i3 */
    CCMFMT_I1V2,		/* i1, v2 */
    CCMFMT_I1V2V3,		/* i1, v2, v3 */
    CCMFMT_I1L2,		/* i1, l2 */
    CCMFMT_I1LL2,		/* i1, l2, l3, ... */
    CCMFMT_I1I2I3I4,		/* i1, i2, i3, i4 */
    CCMFMT_I1I2I3I4I5I6,	/* i1, i2, ..., i6 */
    CCMFMT_I1I2I3I4I5I6I7I8,	/* i1, i2, ..., i8 */
    CCMFMT_LAST
} Ccm_Fmttype_t;

/*
 * Low- and high-level types for commentary parameters.
 */

typedef enum ccm_primtype
{
  CCM_PRIMTYPE_NONE,
  CCM_PRIMTYPE_STRING,
  CCM_PRIMTYPE_INTEGER,
  CCM_PRIMTYPE_HEXSTRING
} Ccm_Primtype_t;

typedef enum ccm_hightype
{
  CCM_HITYPE_NONE,
  CCM_HITYPE_STRING,
  CCM_HITYPE_PROCEDURE,
  CCM_HITYPE_VARIABLE,
  CCM_HITYPE_LOOPTAG,
  CCM_HITYPE_REGIONTAG,
  CCM_HITYPE_HEXSTRING,
  CCM_HITYPE_INTEGER
} Ccm_Hitype_t;

typedef struct ccm_attrs
{
  char *msg;            /* I18N msg string */
  const char *name;     /* Print name for this message ID */
  int32_t vis;          /* Visibility bits */
  Ccm_Fmttype_t fmt;    /* Format type */
} Ccm_Attr_t;

static Ccm_Attr_t *ccm_attrs;            /* Table of per-msg attributes */
static nl_catd ccm_catd = (nl_catd) - 1; /* messages id */

/*
 * map COMPMSG_ID to table indices
 */
static int
ccm_vis_index (COMPMSG_ID m)
{
  int32_t high = m >> 8;
  int32_t low = m & 0xFF;
  for (int i = 0; i < 24; i++, high >>= 1)
    if (high <= 1)
      return (i << 8) + low + 1;
  return 0;
}

/*
 * Return # parameters for this message; MAXINT for messages with
 * parameter lists.
 */
static int
ccm_num_params (COMPMSG_ID m)
{
  int vindex;
  int res;
  vindex = ccm_vis_index (m);
  switch (ccm_attrs[vindex].fmt)
    {
    case CCMFMT_NONE:
      res = 0;
      break;
    case CCMFMT_S1:
    case CCMFMT_P1:
    case CCMFMT_V1:
    case CCMFMT_L1:
    case CCMFMT_R1:
    case CCMFMT_I1:
      res = 1;
      break;
    case CCMFMT_S1S2:
    case CCMFMT_S1L2:
    case CCMFMT_S1X2:
    case CCMFMT_P1S2:
    case CCMFMT_P1P2:
    case CCMFMT_P1L2:
    case CCMFMT_P1I2:
    case CCMFMT_V1V2:
    case CCMFMT_V1L2:
    case CCMFMT_L1S2:
    case CCMFMT_L1P2:
    case CCMFMT_L1L2:
    case CCMFMT_L1R2:
    case CCMFMT_L1I2:
    case CCMFMT_I1V2:
    case CCMFMT_I1L2:
      res = 2;
      break;
    case CCMFMT_P1S2P3:
    case CCMFMT_P1S2I3:
    case CCMFMT_P1I2L3:
    case CCMFMT_P1I2I3:
    case CCMFMT_L1S2L3:
    case CCMFMT_L1P2I3:
    case CCMFMT_L1L2L3:
    case CCMFMT_L1I2L3:
    case CCMFMT_I1P2I3:
    case CCMFMT_I1V2V3:
      res = 3;
      break;
    case CCMFMT_P1S2P3I4:
    case CCMFMT_L1I2I3L4:
    case CCMFMT_I1I2I3I4:
      res = 4;
      break;
    case CCMFMT_L1I2I3I4I5:
      res = 5;
      break;
    case CCMFMT_I1I2I3I4I5I6:
      res = 6;
      break;
    case CCMFMT_L1I2I3I4I5I6I7:
      res = 7;
      break;
    case CCMFMT_I1I2I3I4I5I6I7I8:
      res = 8;
      break;
    case CCMFMT_L1I2I3I4I5I6I7I8I9:
      res = 9;
      break;
    case CCMFMT_S1L2VV3:
    case CCMFMT_S1R2VV3:
    case CCMFMT_PP1:
    case CCMFMT_P1I2LL3:
    case CCMFMT_VV1:
    case CCMFMT_L1PP2:
    case CCMFMT_L1VV2:
    case CCMFMT_LL1:
    case CCMFMT_L1I2LL3:
    case CCMFMT_L1II2:
    case CCMFMT_R1VV2:
    case CCMFMT_I1LL2:
      res = MAXINT;
      break;
    case CCMFMT_LAST:
    default:
      /* programming failure */
      /* if(1) is hack to get around warning from C++ compiler */
      if (1) assert (0);
      break;
    }
  return res;
}

static int
ccm_paramlist_index (COMPMSG_ID m)
{
  int res;
  int vindex = ccm_vis_index (m);
  switch (ccm_attrs[vindex].fmt)
    {
    case CCMFMT_NONE:
    case CCMFMT_S1:
    case CCMFMT_S1S2:
    case CCMFMT_S1L2:
    case CCMFMT_S1X2:
    case CCMFMT_P1:
    case CCMFMT_P1S2:
    case CCMFMT_P1S2P3:
    case CCMFMT_P1S2P3I4:
    case CCMFMT_P1S2I3:
    case CCMFMT_P1P2:
    case CCMFMT_P1L2:
    case CCMFMT_P1I2:
    case CCMFMT_P1I2L3:
    case CCMFMT_P1I2I3:
    case CCMFMT_V1:
    case CCMFMT_V1V2:
    case CCMFMT_V1L2:
    case CCMFMT_L1:
    case CCMFMT_L1S2:
    case CCMFMT_L1S2L3:
    case CCMFMT_L1P2:
    case CCMFMT_L1P2I3:
    case CCMFMT_L1L2:
    case CCMFMT_L1L2L3:
    case CCMFMT_L1R2:
    case CCMFMT_L1I2:
    case CCMFMT_L1I2L3:
    case CCMFMT_L1I2I3L4:
    case CCMFMT_L1I2I3I4I5:
    case CCMFMT_L1I2I3I4I5I6I7:
    case CCMFMT_L1I2I3I4I5I6I7I8I9:
    case CCMFMT_R1:
    case CCMFMT_I1:
    case CCMFMT_I1P2I3:
    case CCMFMT_I1V2:
    case CCMFMT_I1V2V3:
    case CCMFMT_I1L2:
    case CCMFMT_I1I2I3I4:
    case CCMFMT_I1I2I3I4I5I6:
    case CCMFMT_I1I2I3I4I5I6I7I8:
      res = 0;
      break;
    case CCMFMT_PP1:
    case CCMFMT_VV1:
    case CCMFMT_LL1:
      res = 1;
      break;
    case CCMFMT_L1PP2:
    case CCMFMT_L1VV2:
    case CCMFMT_L1II2:
    case CCMFMT_R1VV2:
    case CCMFMT_I1LL2:
      res = 2;
      break;
    case CCMFMT_S1L2VV3:
    case CCMFMT_S1R2VV3:
    case CCMFMT_P1I2LL3:
    case CCMFMT_L1I2LL3:
      res = 3;
      break;
    case CCMFMT_LAST:
    default:
      /* programming failure */
      /* if(1) is hack to get around warning from C++ compiler */
      if (1) assert (0);
      break;
    }
  return res;
}

static Ccm_Primtype_t
ccm_param_primtype (COMPMSG_ID m, int param_idx)
{
  int vindex;
  Ccm_Primtype_t res;
  if (param_idx <= 0 || param_idx > ccm_num_params (m))
    return CCM_PRIMTYPE_NONE;

  res = CCM_PRIMTYPE_NONE; /* should always be updated */
  vindex = ccm_vis_index (m);
  switch (ccm_attrs[vindex].fmt)
    {
      /*
       * Sort cases by:
       * 1) # parameters
       * 2) Strings before Integers
       * 3) Enum tags
       */
    case CCMFMT_NONE:
      /* programming failure */
      /* if(1) is hack to get around warning from C++ compiler */
      if (1)
	assert (0);
      break;
    case CCMFMT_S1:
    case CCMFMT_P1:
    case CCMFMT_V1:
    case CCMFMT_L1:
    case CCMFMT_R1:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_I1:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_S1S2:
    case CCMFMT_S1L2:
    case CCMFMT_P1S2:
    case CCMFMT_P1P2:
    case CCMFMT_P1L2:
    case CCMFMT_V1V2:
    case CCMFMT_V1L2:
    case CCMFMT_L1S2:
    case CCMFMT_L1P2:
    case CCMFMT_L1L2:
    case CCMFMT_L1R2:
      if (param_idx == 1 || param_idx == 2)
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_S1X2:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 2)
	res = CCM_PRIMTYPE_HEXSTRING;
      break;
    case CCMFMT_P1I2:
    case CCMFMT_L1I2:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 2)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_I1V2:
    case CCMFMT_I1L2:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_INTEGER;
      else if (param_idx == 2)
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_P1S2P3:
    case CCMFMT_L1S2L3:
    case CCMFMT_L1L2L3:
      if (param_idx >= 1 && param_idx <= 3)
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_P1S2I3:
    case CCMFMT_L1P2I3:
      if (param_idx == 1 || param_idx == 2)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 3)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_P1I2L3:
    case CCMFMT_L1I2L3:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 2)
	res = CCM_PRIMTYPE_INTEGER;
      break;
   case CCMFMT_P1I2I3:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 2  || param_idx == 3)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_I1V2V3:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_INTEGER;
      else if (param_idx == 2 || param_idx == 3)
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_I1P2I3:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_PRIMTYPE_INTEGER;
      else if (param_idx == 2)
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_L1I2I3L4:
      if (param_idx == 1 || param_idx == 4)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 2 || param_idx == 3)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_P1S2P3I4:
      if (param_idx >= 1 && param_idx <= 3)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx == 4)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_I1I2I3I4:
      if (param_idx >= 1 && param_idx <= 4)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_L1I2I3I4I5:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx >= 2 && param_idx <= 5)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_I1I2I3I4I5I6:
      if (param_idx >= 1 && param_idx <= 6)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_L1I2I3I4I5I6I7:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx >= 2 && param_idx <= 7)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_I1I2I3I4I5I6I7I8:
      if (param_idx >= 1 && param_idx <= 8)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_L1I2I3I4I5I6I7I8I9:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else if (param_idx >= 2 && param_idx <= 9)
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_S1L2VV3:
    case CCMFMT_S1R2VV3:
    case CCMFMT_PP1:
    case CCMFMT_VV1:
    case CCMFMT_L1PP2:
    case CCMFMT_L1VV2:
    case CCMFMT_LL1:
    case CCMFMT_R1VV2:
      res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_P1I2LL3:
    case CCMFMT_L1I2LL3:
      if (param_idx == 2)
	res = CCM_PRIMTYPE_INTEGER;
      else
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_L1II2:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_STRING;
      else
	res = CCM_PRIMTYPE_INTEGER;
      break;
    case CCMFMT_I1LL2:
      if (param_idx == 1)
	res = CCM_PRIMTYPE_INTEGER;
      else
	res = CCM_PRIMTYPE_STRING;
      break;
    case CCMFMT_LAST:
    default:
      /* programming failure */
      /* if(1) is hack to get around warning from C++ compiler */
      if (1)
	assert (0);
      break;
    }
  return res;
}

static Ccm_Hitype_t
ccm_param_hightype (COMPMSG_ID m, int param_idx)
{
  int vindex;
  Ccm_Hitype_t res;

  if (param_idx <= 0 || param_idx > ccm_num_params (m))
    return CCM_HITYPE_NONE;
  res = CCM_HITYPE_NONE; /* should always be updated */
  vindex = ccm_vis_index (m);
  switch (ccm_attrs[vindex].fmt)
    {
    case CCMFMT_NONE:
      /* programming failure */
      /* if(1) is hack to get around warning from C++ compiler */
      if (1)
	assert (0);
      break;
    case CCMFMT_S1:
      if (param_idx == 1)
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_S1S2:
      if (param_idx == 1 || param_idx == 2)
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_S1L2:
      if (param_idx == 1)
	res = CCM_HITYPE_STRING;
      else if (param_idx == 2)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_S1L2VV3:
      if (param_idx == 1)
	res = CCM_HITYPE_STRING;
      else if (param_idx == 2)
	res = CCM_HITYPE_LOOPTAG;
      else
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_S1R2VV3:
      if (param_idx == 1)
	res = CCM_HITYPE_STRING;
      else if (param_idx == 2)
	res = CCM_HITYPE_REGIONTAG;
      else
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_S1X2:
      if (param_idx == 1)
	res = CCM_HITYPE_STRING;
      else if (param_idx == 2)
	res = CCM_HITYPE_HEXSTRING;
      break;
    case CCMFMT_P1:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      break;
    case CCMFMT_P1S2:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_P1S2P3:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_P1S2P3I4:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_STRING;
      else if (param_idx == 4)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_P1S2I3:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_STRING;
      else if (param_idx == 3)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_P1P2:
      if (param_idx == 1 || param_idx == 2)
	res = CCM_HITYPE_PROCEDURE;
      break;
    case CCMFMT_P1L2:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_P1I2:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_P1I2L3:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_INTEGER;
      else if (param_idx == 3)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_P1I2I3:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2 || param_idx == 3)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_P1I2LL3:
      if (param_idx == 1)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 2)
	res = CCM_HITYPE_INTEGER;
      else
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_PP1:
      res = CCM_HITYPE_PROCEDURE;
      break;
    case CCMFMT_V1:
      if (param_idx == 1)
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_V1V2:
      if (param_idx == 1 || param_idx == 2)
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_V1L2:
      if (param_idx == 1)
	res = CCM_HITYPE_VARIABLE;
      else if (param_idx == 2)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_VV1:
      res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_L1:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_L1S2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_L1S2L3:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_STRING;
      break;
    case CCMFMT_L1P2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_PROCEDURE;
      break;
    case CCMFMT_L1P2I3:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_PROCEDURE;
      else if (param_idx == 3)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1PP2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else
	res = CCM_HITYPE_PROCEDURE;
      break;
    case CCMFMT_L1VV2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_L1L2:
      if (param_idx == 1 || param_idx == 2)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_L1L2L3:
      if (param_idx >= 1 && param_idx <= 3)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_LL1:
      res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_L1R2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_REGIONTAG;
      break;
    case CCMFMT_L1I2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1I2L3:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1I2LL3:
      if (param_idx == 2)
	res = CCM_HITYPE_INTEGER;
      else
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_L1I2I3L4:
      if (param_idx == 1 || param_idx == 4)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx == 2 || param_idx == 3)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1I2I3I4I5:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx >= 2 && param_idx <= 5)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1I2I3I4I5I6I7:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx >= 2 && param_idx <= 7)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1I2I3I4I5I6I7I8I9:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else if (param_idx >= 2 && param_idx <= 9)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_L1II2:
      if (param_idx == 1)
	res = CCM_HITYPE_LOOPTAG;
      else
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_R1:
      if (param_idx == 1)
	res = CCM_HITYPE_REGIONTAG;
      break;
    case CCMFMT_R1VV2:
      if (param_idx == 1)
	res = CCM_HITYPE_REGIONTAG;
      else
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_I1:
      if (param_idx == 1)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_I1P2I3:
      if (param_idx == 1 || param_idx == 3)
	res = CCM_HITYPE_INTEGER;
      else if (param_idx == 2)
	res = CCM_HITYPE_PROCEDURE;
      break;
    case CCMFMT_I1V2:
      if (param_idx == 1)
	res = CCM_HITYPE_INTEGER;
      else if (param_idx == 2)
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_I1V2V3:
      if (param_idx == 1)
	res = CCM_HITYPE_INTEGER;
      else if (param_idx == 2 || param_idx == 3)
	res = CCM_HITYPE_VARIABLE;
      break;
    case CCMFMT_I1L2:
      if (param_idx == 1)
	res = CCM_HITYPE_INTEGER;
      else if (param_idx == 2)
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_I1LL2:
      if (param_idx == 1)
	res = CCM_HITYPE_INTEGER;
      else
	res = CCM_HITYPE_LOOPTAG;
      break;
    case CCMFMT_I1I2I3I4:
      if (param_idx >= 1 && param_idx <= 4)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_I1I2I3I4I5I6:
      if (param_idx >= 1 && param_idx <= 6)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_I1I2I3I4I5I6I7I8:
      if (param_idx >= 1 && param_idx <= 8)
	res = CCM_HITYPE_INTEGER;
      break;
    case CCMFMT_LAST:
    default:
      /* programming failure */
      /* if(1) is hack to get around warning from C++ compiler */
      if (1)
	assert (0);
      break;
    }
  return res;
}

static void
ccm_vis_init ()
{
  int size, vindex;
  static int done = 0;
  if (done)
    return;
  done = 1;
  size = ccm_vis_index ((COMPMSG_ID) (CCMV_BASIC << 8));
  ccm_attrs = (Ccm_Attr_t *) calloc (size, sizeof (Ccm_Attr_t));
  if (ccm_attrs == NULL)
    exit (1);
  vindex = ccm_vis_index (CCM_MODDATE);
  ccm_attrs[vindex].vis = CCMV_VER | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MODDATE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Source file %s, last modified on date %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1S2;

  vindex = ccm_vis_index (CCM_COMPVER);
  ccm_attrs[vindex].vis = CCMV_VER | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_COMPVER";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Component %s, version %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1S2;

  vindex = ccm_vis_index (CCM_COMPDATE);
  ccm_attrs[vindex].vis = CCMV_VER | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_COMPDATE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Compilation date %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_COMPOPT);
  ccm_attrs[vindex].vis = CCMV_VER | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_COMPOPT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Compilation options %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_ACOMPOPT);
  ccm_attrs[vindex].vis = CCMV_VER | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_ACOMPOPT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Actual Compilation options %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_VAR_ALIAS);
  ccm_attrs[vindex].vis = CCMV_WARN | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_VAR_ALIAS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variable %s aliased to %s");
  ccm_attrs[vindex].fmt = CCMFMT_V1V2;

  vindex = ccm_vis_index (CCM_FBIRDIFF);
  ccm_attrs[vindex].vis = CCMV_WARN | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_FBIRDIFF";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Profile feedback data inconsistent with"
				   " intermediate representation file; check compiler"
				   " version, flags and source file");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_OPTRED_SWAP);
  ccm_attrs[vindex].vis = CCMV_WARN | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_OPTRED_SWAP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Optimization level for %s reduced from %d to"
				   " %d due to insufficient swap space");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2I3;

  vindex = ccm_vis_index (CCM_OPTRED_CPLX);
  ccm_attrs[vindex].vis = CCMV_WARN | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_OPTRED_CPLX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Optimization level for %s reduced from %d to"
				   " %d due to program complexity");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2I3;

  vindex = ccm_vis_index (CCM_UNKNOWN);
  ccm_attrs[vindex].vis = CCMV_WARN | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNKNOWN";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Unexpected compiler comment %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_UNPAR_CALL);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_CALL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it contains a"
				   " call to %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_PAR_SER);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PAR_SER";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Both serial and parallel versions generated for"
				   " loop below");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_PAR_SER_VER);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PAR_SER_VER";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Both serial and parallel versions generated for"
				   " loop below; with parallel version used if %s,"
				   " serial otherwise");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_PAR_DRECTV);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PAR_DRECTV";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below parallelized by explicit user"
				   " directive");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_APAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_APAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below autoparallelized");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_AUTOPAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_AUTOPAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below autoparallelized; equivalent"
				   " explict directive is %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_UNPAR_DD);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_DD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below could not be parallelized because of a"
				   " data dependency on %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_UNPAR_DDA);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_DDA";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below could not be parallelized because of a"
				   " data dependency or aliasing of %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_UNPAR_ANONDD);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_ANONDD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below could not be parallelized because of"
				   " an anonymous data dependency");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_ANONDDA);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_ANONDDA";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below could not be parallelized because of"
				   " an anonymous data dependency or aliasing");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_PAR_WORK);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PAR_WORK";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below parallelized, but might not contain"
				   " enough work to be efficiently run in parallel");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_EXIT);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_EXIT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it contains"
				   " multiple exit points");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_STRNG);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_STRNG";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it contains a"
				   " strange flow of control");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_IO);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_IO";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it contains"
				   " I/O or other MT-unsafe calls");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_PAR_BODY_NAME);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PAR_BODY_NAME";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Parallel loop-body code is in function %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_UNPAR_NLOOPIDX);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_NLOOPIDX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because loop index"
				   " not found");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_DRECTV);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_DRECTV";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because of explicit"
				   " user directive");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_NOTPROFIT);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_NOTPROFIT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it was not"
				   " profitable to do so");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_NEST);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_NEST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it was"
				   " nested in a parallel loop");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_NOAUTO);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_NOAUTO";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because"
				   " autoparallelization is not enabled");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_PR_L_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PR_L_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Private variables in loop below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_SH_L_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_SH_L_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Shared variables in loop below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_TP_L_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_TP_L_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Threadprivate variables in loop below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_RV_L_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_RV_L_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Reduction variables in loop below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_IM_L_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IM_L_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Implicit variables in loop below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_PR_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PR_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Private variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_SH_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_SH_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Shared variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_TP_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_TP_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Threadprivate variables in OpenMP construct"
				   " below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_RV_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_RV_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Reduction variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_IM_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IM_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Implicit variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_UNPAR_IN_OMP);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_IN_OMP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below not parallelized because it is inside"
				   " an OpenMP region");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_FP_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_FP_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Firstprivate variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_LP_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LP_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Lastprivate variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_CP_O_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_CP_O_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Copyprivate variables in OpenMP construct below:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_PR_OAS_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PR_OAS_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as PRIVATE in OpenMP"
				   " construct below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_SH_OAS_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_SH_OAS_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as SHARED in OpenMP"
				   " construct below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_FP_OAS_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_FP_OAS_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as FIRSTPRIVATE in OpenMP"
				   " construct below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_LP_OAS_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LP_OAS_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as LASTPRIVATE in OpenMP"
				   " construct below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_RV_OAS_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_RV_OAS_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as REDUCTION in OpenMP"
				   " construct below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_FAIL_OAS_VAR);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_WARN | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_FAIL_OAS_VAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables cannot be autoscoped in OpenMP"
				   " construct below: %s");
  ccm_attrs[vindex].fmt = CCMFMT_VV1;

  vindex = ccm_vis_index (CCM_SERIALIZE_OAS);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_WARN | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_SERIALIZE_OAS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "OpenMP parallel region below is serialized"
				   " because autoscoping has failed");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_UNPAR_CALL_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_CALL_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it contains calls"
				   " to: %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1PP2;

  vindex = ccm_vis_index (CCM_PAR_DRECTV_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PAR_DRECTV_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s parallelized by explicit user directive");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_APAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_APAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s autoparallelized");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_AUTOPAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_AUTOPAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s autoparallelized; equivalent"
				   " explict directive is %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1S2;

  vindex = ccm_vis_index (CCM_UNPAR_DD_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_DD_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be parallelized because of"
				   " data dependences on: %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1VV2;

  vindex = ccm_vis_index (CCM_UNPAR_DDA_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_DDA_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be parallelized because of a"
				   " data dependence or aliasing of: %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1VV2;

  vindex = ccm_vis_index (CCM_UNPAR_ANONDD_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_ANONDD_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be parallelized because of an"
				   " anonymous data dependence");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_ANONDDA_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_ANONDDA_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be parallelized because of an"
				   " anonymous data dependence or aliasing");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_PAR_WORK_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PAR_WORK_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s parallelized, but might not contain"
				   " enough work to run efficiently in parallel");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_EXIT_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_EXIT_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it contains"
				   " multiple exit points");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_STRANGE_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_STRANGE_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it contains a"
				   " strange flow of control");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_IO_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_IO_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it contains"
				   " I/O or other MT-unsafe calls");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_PAR_BODY_NAME_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP;
  ccm_attrs[vindex].name = "CCM_PAR_BODY_NAME_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s parallel loop-body code placed in"
				   " function %s along with %d inner loops");
  ccm_attrs[vindex].fmt = CCMFMT_L1P2I3;

  vindex = ccm_vis_index (CCM_UNPAR_NLOOPIDX_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_NLOOPIDX_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because loop index not"
				   " found");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_DRECTV_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_DRECTV_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because of explicit"
				   " user directive");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_NOTPROFIT_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_NOTPROFIT_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it was not"
				   " profitable to do so");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_NEST_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_NEST_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it was"
				   " nested within a parallel loop");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_UNPAR_NOAUTO_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNPAR_NOAUTO_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because"
				   " autoparallelization is not enabled");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_PR_L_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PR_L_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Private variables in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1VV2;

  vindex = ccm_vis_index (CCM_SH_L_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_SH_L_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Shared variables in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1VV2;

  vindex = ccm_vis_index (CCM_TP_L_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_TP_L_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Threadprivate variables in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1VV2;

  vindex = ccm_vis_index (CCM_RV_L_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_RV_L_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Reduction variables of operator %s in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1L2VV3;

  vindex = ccm_vis_index (CCM_IM_L_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IM_L_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Implicit variables in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1VV2;

  vindex = ccm_vis_index (CCM_PR_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PR_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Private variables in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_SH_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_SH_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Shared variables in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_TP_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_TP_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Threadprivate variables in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_RV_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_RV_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Reduction variables of operator %s in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1R2VV3;

  vindex = ccm_vis_index (CCM_IM_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IM_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Implicit variables in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_UNPAR_IN_OMP_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNPAR_IN_OMP_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s not parallelized because it is inside"
				   " OpenMP region %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1R2;

  vindex = ccm_vis_index (CCM_FP_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_FP_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Firstprivate variables in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_LP_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LP_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Lastprivate variables in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_CP_O_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_CP_O_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Copyprivate variables in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_PR_OAS_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PR_OAS_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as PRIVATE in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_SH_OAS_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_SH_OAS_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as SHARED in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_FP_OAS_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_FP_OAS_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as FIRSTPRIVATE in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_LP_OAS_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LP_OAS_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as LASTPRIVATE in %s:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_RV_OAS_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_RV_OAS_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables autoscoped as REDUCTION of operator"
				   " %s in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1R2VV3;

  vindex = ccm_vis_index (CCM_FAIL_OAS_VAR_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_WARN;
  ccm_attrs[vindex].name = "CCM_FAIL_OAS_VAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variables treated as shared because they cannot"
				   " be autoscoped in %s: %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1VV2;

  vindex = ccm_vis_index (CCM_SERIALIZE_OAS_2);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC | CCMV_WARN;
  ccm_attrs[vindex].name = "CCM_SERIALIZE_OAS_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s will be executed by a single thread because"
				   " autoscoping for some variables was not successful");
  ccm_attrs[vindex].fmt = CCMFMT_R1;

  vindex = ccm_vis_index (CCM_QPERMVEC);
  ccm_attrs[vindex].vis = CCMV_QUERY | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_QPERMVEC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Is %s a permutation vector during execution of"
				   " %s?");
  ccm_attrs[vindex].fmt = CCMFMT_V1L2;

  vindex = ccm_vis_index (CCM_QEXPR);
  ccm_attrs[vindex].vis = CCMV_QUERY | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_QEXPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Is expression %s true for %s?");
  ccm_attrs[vindex].fmt = CCMFMT_S1L2;

  vindex = ccm_vis_index (CCM_QSAFECALL);
  ccm_attrs[vindex].vis = CCMV_QUERY | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_QSAFECALL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Is subroutine %s MP-safe as used in %s?");
  ccm_attrs[vindex].fmt = CCMFMT_P1L2;

  vindex = ccm_vis_index (CCM_LCOST);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LCOST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below estimated to cost %d cycles per"
				   " iteration");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_UNROLL);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_UNROLL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below unrolled %d times");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_IMIX);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IMIX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below has %d loads, %d stores,"
				   " %d prefetches, %d FPadds, %d FPmuls, and"
				   " %d FPdivs per iteration");
  ccm_attrs[vindex].fmt = CCMFMT_I1I2I3I4I5I6;

  vindex = ccm_vis_index (CCM_SPILLS);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_UNIMPL | CCMV_WANT | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_SPILLS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below required %d integer register spills,"
				   " %d FP register spills, and used"
				   " %d integer registers and %d FP registers");
  ccm_attrs[vindex].fmt = CCMFMT_I1I2I3I4;

  vindex = ccm_vis_index (CCM_LFISSION);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LFISSION";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below fissioned into %d loops");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_LPEEL);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LPEEL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below had iterations peeled off for better"
				   " unrolling and/or parallelization");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_LBLOCKED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LBLOCKED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below blocked by %d for improved cache"
				   " performance");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_LTILED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LTILED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below tiled for better performance");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_LUNRJAM);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LUNRJAM";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below unrolled and jammed");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_LWHILE2DO);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LWHILE2DO";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Bounds test for loop below moved to top of loop");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_L2CALL);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_L2CALL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below replaced by a call to %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_LDEAD);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LDEAD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below deleted as dead code");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_LINTRCHNG);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LINTRCHNG";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below interchanged with loop on line %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_FUSEDTO);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_FUSEDTO";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below fused with loop on line %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_FUSEDFROM);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_FUSEDFROM";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop from line %d fused with loop below");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_VECINTRNSC);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_VECINTRNSC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below transformed to use calls to vector"
				   " intrinsic %s");
  ccm_attrs[vindex].fmt = CCMFMT_PP1;

  vindex = ccm_vis_index (CCM_LSTRIPMINE);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LSTRIPMINE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below strip-mined");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_LNEST2LOOPS);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LNEST2LOOPS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below collapsed with loop on line %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_LREVERSE);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LREVERSE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below has had its iteration direction"
				   " reversed");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_IMIX2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IMIX2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below has %d loads, %d stores,"
				   " %d prefetches, %d FPadds, %d FPmuls,"
				   " %d FPdivs, %d FPsubs, and %d FPsqrts per"
				   " iteration");
  ccm_attrs[vindex].fmt = CCMFMT_I1I2I3I4I5I6I7I8;

  vindex = ccm_vis_index (CCM_LUNRFULL);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_LUNRFULL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below fully unrolled");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_ELIM_NOAMORTINST);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_ELIM_NOAMORTINST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below was eliminated as it contains no"
				   " non-amortizable instructions");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_COMP_DALIGN);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_COMP_DALIGN";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Performance of loop below could be improved"
				   " by compiling with -dalign");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_INTIMIX);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_INTIMIX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below has %d int-loads, %d int-stores,"
				   " %d alu-ops, %d muls, %d int-divs and"
				   " %d shifts per iteration");
  ccm_attrs[vindex].fmt = CCMFMT_I1I2I3I4I5I6;

  vindex = ccm_vis_index (CCM_LMULTI_VERSION);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LMULTI_VERSION";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s multi-versioned.  Specialized version"
				   " is %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1L2;

  vindex = ccm_vis_index (CCM_LCOST_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LCOST_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s estimated to cost %d cycles per iteration");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_UNROLL_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_UNROLL_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s unrolled %d times");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_IMIX_B);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_IMIX_B";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s has %d loads, %d stores,"
				   " %d prefetches, %d FPadds, %d FPmuls, and"
				   " %d FPdivs per iteration");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2I3I4I5I6I7;

  vindex = ccm_vis_index (CCM_SPILLS_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_SPILLS_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s required %d integer register spills,"
				   " %d FP register spills, and used"
				   " %d integer registers and %d FP registers");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2I3I4I5;

  vindex = ccm_vis_index (CCM_LFISSION_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LFISSION_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s fissioned into %d loops, generating:"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2LL3;

  vindex = ccm_vis_index (CCM_LFISSION_FRAG);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LFISSION_FRAG";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s contains code from lines: %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1II2;

  vindex = ccm_vis_index (CCM_LPEEL_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LPEEL_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s had iterations peeled off for better"
				   " unrolling and/or parallelization");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_LBLOCKED_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LBLOCKED_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s blocked by %d for improved memory"
				   " hierarchy performance, new inner loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2L3;

  vindex = ccm_vis_index (CCM_LOUTER_UNROLL);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LOUTER_UNROLL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s is outer-unrolled %d times as part"
				   " of unroll and jam");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_LJAMMED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LJAMMED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "All %d copies of %s are fused together"
				   " as part of unroll and jam");
  ccm_attrs[vindex].fmt = CCMFMT_I1L2;

  vindex = ccm_vis_index (CCM_LWHILE2DO_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LWHILE2DO_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Bounds test for %s moved to top of loop");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_L2CALL_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_L2CALL_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s replaced by a call to %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1P2;

  vindex = ccm_vis_index (CCM_LDEAD_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LDEAD_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s deleted as dead code");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_LINTRCHNG_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LINTRCHNG_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s interchanged with %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1L2;

  vindex = ccm_vis_index (CCM_LINTRCHNG_ORDER);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LINTRCHNG_ORDER";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "For loop nest below, the final order of loops"
				   " after interchanging and subsequent"
				   " transformations is: %s");
  ccm_attrs[vindex].fmt = CCMFMT_LL1;

  vindex = ccm_vis_index (CCM_FUSED_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_FUSED_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s fused with %s, new loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1L2L3;

  vindex = ccm_vis_index (CCM_VECINTRNSC_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_VECINTRNSC_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s transformed to use calls to vector"
				   " intrinsics: %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1PP2;

  vindex = ccm_vis_index (CCM_LSTRIPMINE_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LSTRIPMINE_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s strip-mined by %d, new inner loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2L3;

  vindex = ccm_vis_index (CCM_LNEST2LOOPS_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LNEST2LOOPS_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s collapsed with %s, new loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1L2L3;

  vindex = ccm_vis_index (CCM_LREVERSE_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LREVERSE_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s has had its iteration direction reversed");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_IMIX2_B);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_IMIX2_B";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s has %d loads, %d stores,"
				   " %d prefetches, %d FPadds, %d FPmuls,"
				   " %d FPdivs, %d FPsubs, and %d FPsqrts per"
				   " iteration");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2I3I4I5I6I7I8I9;

  vindex = ccm_vis_index (CCM_LUNRFULL_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LUNRFULL_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s fully unrolled");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_ELIM_NOAMORTINST_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_ELIM_NOAMORTINST_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s was eliminated as it contains no"
				   " non-amortizable instructions");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_COMP_DALIGN_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_COMP_DALIGN_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Performance of %s could be improved by"
				   " compiling with -dalign");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_INTIMIX_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INTIMIX_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s has %d int-loads, %d int-stores,"
				   " %d alu-ops, %d muls, %d int-divs and"
				   " %d shifts per iteration");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2I3I4I5I6I7;

  vindex = ccm_vis_index (CCM_OMP_REGION);
  ccm_attrs[vindex].vis = CCMV_PAR | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_OMP_REGION";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Source OpenMP region below has tag %s");
  ccm_attrs[vindex].fmt = CCMFMT_R1;

  vindex = ccm_vis_index (CCM_LMICROVECTORIZE);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LMICROVECTORIZE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s is micro-vectorized");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_LMULTI_VERSION_2);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LMULTI_VERSION_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s multi-versioned for %s."
				   " Specialized version is %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1S2L3;

  vindex = ccm_vis_index (CCM_LCLONED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LCLONED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s cloned for %s.  Clone is %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1S2L3;

  vindex = ccm_vis_index (CCM_LUNSWITCHED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LUNSWITCHED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s is unswitched.  New loops"
				   " are %s and %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1L2L3;

  vindex = ccm_vis_index (CCM_LRESWITCHED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LRESWITCHED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loops %s and %s and their surrounding"
				   " conditional code have been merged to"
				   " form loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1L2L3;

  vindex = ccm_vis_index (CCM_LSKEWBLOCKED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LSKEWBLOCKED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s skew-blocked by %d with slope"
				   " %d for improved memory hierarchy"
				   " performance, new inner loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2I3L4;

  vindex = ccm_vis_index (CCM_IVSUB);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_IVSUB";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Induction variable substitution performed on %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_ONEITER_REPLACED);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_ONEITER_REPLACED";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s determined to have a trip count of 1;"
				   " converted to straight-line code");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_IMIX3_B);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_IMIX3_B";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s has %d loads, %d stores,"
				   " %d prefetches, %d FPadds, %d FPmuls,"
				   " %d FPmuladds, %d FPdivs, and %d FPsqrts per"
				   " iteration");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2I3I4I5I6I7I8I9;

  vindex = ccm_vis_index (CCM_PIPELINE);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PIPELINE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below pipelined");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_PIPESTATS);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PIPESTATS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below scheduled with steady-state cycle"
				   " count = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_NOPIPE_CALL);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_CALL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " calls");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_INTCC);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INTCC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it sets"
				   " multiple integer condition codes.");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_MBAR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_MBAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains a"
				   " memory barrier instruction");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_MNMX);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_MNMX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " a minimum or a maximum operation");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_U2FLT);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_U2FLT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " an unsigned to float conversion");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_GOT);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_GOT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it sets the"
				   " Global Offset Table pointer");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_IDIV);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_IDIV";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " an integer divide");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_PRFTCH);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_PRFTCH";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " a prefetch operation");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_EXIT);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_EXIT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " an exit operation");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_REG);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_REG";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it contains"
				   " instructions that set the %%gsr or %%fsr register");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_UNS);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_UNS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it has an"
				   " unsigned loop counter");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_UNSUIT);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_UNSUIT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop was unsuitable for pipelining");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_INTRINSIC);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INTRINSIC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined because it has an"
				   " intrinsic call to %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NOPIPE_BIG);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_BIG";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined as it is too big");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NOPIPE_INVINTPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INVINTPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined as it contains too"
				   " many loop invariant integers = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_NOPIPE_INVFLTPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INVFLTPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined as it contains too"
				   " many loop invariant floats = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_NOPIPE_INVDBLPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INVDBLPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined as it contains too"
				   " many loop invariant doubles = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_PIPE_SCHEDAFIPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PIPE_SCHEDAFIPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below was adversely affected by high"
				   " integer register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_PIPE_SCHEDAFDPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PIPE_SCHEDAFDPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below was adversely affected by high"
				   " double register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_PIPE_SCHEDAFFPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_PIPE_SCHEDAFFPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop below was adversely affected by high"
				   " float register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_NOPIPE_INTPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INTPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined due to high"
				   " integer register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_NOPIPE_DBLPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_DBLPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined due to high"
				   " double register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_NOPIPE_FLTPR);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_NOPIPE_FLTPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop could not be pipelined due to high"
				   " float register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_PIPELINE_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PIPELINE_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s pipelined");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_PIPESTATS_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PIPESTATS_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s scheduled with steady-state cycle"
				   " count = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_NOPIPE_CALL_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_CALL_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " calls");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_INTCC_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INTCC_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it sets"
				   " multiple integer condition codes.");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_MBAR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_MBAR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " a memory barrier instruction");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_MNMX_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_MNMX_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " a minimum or a maximum operation");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_U2FLT_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_U2FLT_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " an unsigned to float conversion");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_GOT_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP;
  ccm_attrs[vindex].name = "CCM_NOPIPE_GOT_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it sets the"
				   " Global Offset Table pointer");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_IDIV_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_IDIV_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " an integer divide");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_PRFTCH_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_PRFTCH_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " a prefetch operation");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_EXIT_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_EXIT_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " an exit operation");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_REG_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP;
  ccm_attrs[vindex].name = "CCM_NOPIPE_REG_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " instructions that set the %%gsr or %%fsr register");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_UNS_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_UNS_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it has an"
				   " unsigned loop counter");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_UNSUIT_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_UNSUIT_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s is unsuitable for pipelining");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_INTRINSIC_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INTRINSIC_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined because it contains"
				   " a call to intrinsic %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1P2;

  vindex = ccm_vis_index (CCM_NOPIPE_BIG_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_BIG_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined as it is too big");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_NOPIPE_INVINTPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INVINTPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined as it contains too"
				   " many loop invariant integers = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_NOPIPE_INVFLTPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INVFLTPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined as it contains too"
				   " many loop invariant floats = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_NOPIPE_INVDBLPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INVDBLPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined as it contains too"
				   " many loop invariant doubles = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_PIPE_SCHEDAFIPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PIPE_SCHEDAFIPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s was adversely affected by high"
				   " integer register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_PIPE_SCHEDAFDPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PIPE_SCHEDAFDPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s was adversely affected by high"
				   " double register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_PIPE_SCHEDAFFPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PIPE_SCHEDAFFPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s was adversely affected by high"
				   " float register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_NOPIPE_INTPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_INTPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined due to high"
				   " integer register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_NOPIPE_DBLPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_DBLPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined due to high"
				   " double register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_NOPIPE_FLTPR_2);
  ccm_attrs[vindex].vis = CCMV_PIPE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NOPIPE_FLTPR_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "%s could not be pipelined due to high"
				   " float register pressure = %d");
  ccm_attrs[vindex].fmt = CCMFMT_L1I2;

  vindex = ccm_vis_index (CCM_INLINE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_INLINE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from source file %s into"
				   " the code for the following line");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2;

  vindex = ccm_vis_index (CCM_INLINE2);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_INLINE2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from source file %s into"
				   " inline copy of function %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2P3;

  vindex = ccm_vis_index (CCM_INLINE_TMPLT);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INLINE_TMPLT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from template file %s"
				   " into the code for the following line");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2;

  vindex = ccm_vis_index (CCM_INLINE_TMPLT2);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INLINE_TMPLT2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from template file %s"
				   " into inline copy of function %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2P3;

  vindex = ccm_vis_index (CCM_INLINE_OUT_COPY);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INLINE_OUT_COPY";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Out-of-line copy of inlined function %s from"
				   " source file %s generated");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2;

  vindex = ccm_vis_index (CCM_NINLINE_REC);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_REC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Recursive function %s inlined only up to"
				   " depth %d");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2;

  vindex = ccm_vis_index (CCM_NINLINE_NEST);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_NEST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because inlining is"
				   " already nested too deeply");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CMPLX);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CMPLX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains"
				   " too many operations");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_FB);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_FB";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because the"
				   " profile-feedback execution count is too low");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_PAR);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_PAR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains"
				   " explicit parallel pragmas");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_OPT);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_OPT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is"
				   " compiled with optimization level <= 2");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_USR);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_USR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because either command"
				   " line option or source code pragma prohibited it,"
				   " or it's not safe to inline it");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_AUTO);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_AUTO";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because doing so"
				   " would make automatic storage for %s too large");
  ccm_attrs[vindex].fmt = CCMFMT_P1P2;

  vindex = ccm_vis_index (CCM_NINLINE_CALLS);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALLS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains"
				   " too many calls");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_ACTUAL);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_ACTUAL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it has more"
				   " actual parameters than formal parameters");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_FORMAL);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_FORMAL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it has more"
				   " formal parameters than actual parameters");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_TYPE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_TYPE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because formal"
				   " argument type does not match actual type");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_ATYPE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_ATYPE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because array formal"
				   " argument does not match reshaped array actual"
				   " argument type");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_RETTYPE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_RETTYPE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because return type"
				   " does not match");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_EXCPT);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_EXCPT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it"
				   " guarded by an exception handler");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_UNSAFE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_UNSAFE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it might be"
				   " unsafe (call alloca(), etc)");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_ALIAS);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_ALIAS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because inlining it"
				   " will make the alias analysis in the calling"
				   " function more conservative");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_FEMARK);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_FEMARK";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains"
				   " setjmp/longjmp, or indirect goto, etc");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_RAREX);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_RAREX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is known"
				   " to be rarely executed");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_CLONING);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_CLONING";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s from source file %s cloned,"
				   " creating cloned function %s; constant"
				   " parameters propagated to clone");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2P3;

  vindex = ccm_vis_index (CCM_INLINE_B);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INLINE_B";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from source file %s into"
				   " the code for the following line.  %d loops"
				   " inlined");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2I3;

  vindex = ccm_vis_index (CCM_INLINE2_B);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INLINE2_B";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from source file %s into"
				   " inline copy of function %s.  %d loops inlined");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2P3I4;

  vindex = ccm_vis_index (CCM_INLINE_LOOP);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_LOOP | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_INLINE_LOOP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Loop in function %s, line %d has"
				   " tag %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2L3;

  vindex = ccm_vis_index (CCM_NINLINE_MULTIENTRY);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_MULTIENTRY";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it"
				   " contains an ENTRY statement");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_VARARGS);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_VARARGS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because variable"
				   " argument routines cannot be inlined");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_UNSEEN_BODY);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_UNSEEN_BODY";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because the compiler"
				   " has not seen the body of the function.  Use"
				   " -xcrossfile or -xipo in order to inline it");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_UPLEVEL);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_UPLEVEL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is a"
				   " nested routine containing references to"
				   " variables defined in an outer function");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CMDLINE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CMDLINE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because either"
				   " -xinline or source code pragma prohibited it");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_CMPLX);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_CMPLX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because of the"
				   " complexity of the calling routine");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_LANG_MISMATCH);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_LANG_MISMATCH";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because it is in"
				   " a different language");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_RTN_WEAK);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_RTN_WEAK";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it"
				   " is marked weak");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_WEAKFILE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_WEAKFILE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because it is"
				   " in a different file and it contains a"
				   " call to a weak routine");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_TRYCATCH);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_TRYCATCH";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because it is"
				   " in a different file and contains an"
				   " explicit try/catch");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_REGP);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_REGP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because it would"
				   " cause excessive register pressure");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_RTN_REGP);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_RTN_REGP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it would"
				   " cause excessive register pressure");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_XPENSV);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_XPENSV";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because analysis"
				   " exceeds the compilation time limit");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_READONLYIR);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_READONLYIR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is in a file"
				   " specified as read-only by -xipo_archive=readonly"
				   " and it contains calls to static functions");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_THUNK);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_THUNK";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because it is in a"
				   " compiler-generated function that does not"
				   " permit inlining");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CALL_XTARGETS);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CALL_XTARGETS";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Indirect callsite has too many targets;"
				   " callsite marked do not inline");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NINLINE_SELFTAIL_RECURSIVE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_SELFTAIL_RECURSIVE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because"
				   " of a recursive tail-call to itself");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_PRAGMA);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_PRAGMA";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains"
				   " explicit parallel or alias pragmas");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CMPLX2);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CMPLX2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains too"
				   " many operations.  Increase max_inst_hard in order"
				   " to inline it: -xinline_param=max_inst_hard:n");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_RARE);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_RARE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because the call"
				   " is rarely executed");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_PAR2);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_PAR2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is called"
				   " within a region guarded by an explicit"
				   " parallel pragmas");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_G_LIMIT);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_G_LIMIT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it would exceed"
				   " the permitted global code size growth limit.  Try"
				   " to increase max_growth in order to inline it:"
				   " -xinline_param=max_growth:n");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_L_LIMIT);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_L_LIMIT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it would exceed"
				   " the maximum function size growth limit.  Increase"
				   " max_function_inst in order to inline it:"
				   " -xinline_param=max_function_inst:n");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_REC2);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_REC2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Recursive function %s is inlined only up to"
				   " %d levels and up to %d size.  Increase"
				   " max_recursive_deptha or max_recursive_inst in"
				   " order to inline it:"
				   " -xinline_param=max_recursive_depth:n,"
				   " -xinline_param=max_recursive_inst:n");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2I3;

  vindex = ccm_vis_index (CCM_NINLINE_FB2);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_FB2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because the"
				   " profile-feedback execution count is too"
				   " low.  Decrease min_counter in order to inline it:"
				   " -xinline_param:min_counter:n");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_CS_CMPLX);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_CS_CMPLX";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because called"
				   " function's size is too big.  Increase"
				   " max_inst_soft in order to inline it:"
				   " -xinline_param=max_inst_soft:n");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_R_EXCPT);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_R_EXCPT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it contains"
				   " an exception handler");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_ASM);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_ASM";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because"
				   " it contains asm statements");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_R_READONLYIR);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_R_READONLYIR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is in a file"
				   " specified as read-only by -xipo_archive=readonly"
				   " and it is a static function");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_C_READONLYIR);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_C_READONLYIR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s not inlined because the calling"
				   " function is in a file specified as read-only"
				   " by -xipo_archive=readonly");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NINLINE_NEVERRETURN);
  ccm_attrs[vindex].vis = CCMV_INLINE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NINLINE_NEVERRETURN";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it"
				   " never returns");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_MPREFETCH);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MPREFETCH";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Prefetch of %s inserted");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_MPREFETCH_LD);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MPREFETCH_LD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Prefetch of %s inserted for load at %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1X2;

  vindex = ccm_vis_index (CCM_MPREFETCH_ST);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MPREFETCH_ST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Prefetch of %s inserted for store at %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1X2;

  vindex = ccm_vis_index (CCM_MPREFETCH_FB);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MPREFETCH_FB";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Prefetch of %s inserted based on feedback data");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_MPREFETCH_FB_LD);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MPREFETCH_FB_LD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Prefetch of %s inserted for load at %s based"
				   " on feedback data");
  ccm_attrs[vindex].fmt = CCMFMT_S1X2;

  vindex = ccm_vis_index (CCM_MPREFETCH_FB_ST);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MPREFETCH_FB_ST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Prefetch of %s inserted for store at %s based"
				   " on feedback data");
  ccm_attrs[vindex].fmt = CCMFMT_S1X2;

  vindex = ccm_vis_index (CCM_MLOAD);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MLOAD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Load below refers to %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_MSTORE);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MSTORE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Store below refers to %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_MLOAD_P);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MLOAD_P";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Load below refers to %s, and was prefetched"
				   " at %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1X2;

  vindex = ccm_vis_index (CCM_MSTORE_P);
  ccm_attrs[vindex].vis = CCMV_MEMOPS | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MSTORE_P";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Store below refers to %s, and was prefetched"
				   " at %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1X2;

  vindex = ccm_vis_index (CCM_COPYIN);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_COPYIN";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Parameter %d caused a copyin in the following"
				   " call");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_COPYOUT);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_COPYOUT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Parameter %d caused a copyout in the following"
				   " call");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_COPYINOUT);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_COPYINOUT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Parameter %d caused both a copyin and copyout"
				   " in the following call");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_PADDING);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_PADDING";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Padding of %d bytes inserted before"
				   " array %s");
  ccm_attrs[vindex].fmt = CCMFMT_I1V2;

  vindex = ccm_vis_index (CCM_PADCOMMON);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_PADCOMMON";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Padding of %d bytes inserted before"
				   " array %s in common block %s");
  ccm_attrs[vindex].fmt = CCMFMT_I1V2V3;

  vindex = ccm_vis_index (CCM_ALIGN_EQ);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_ALIGN_EQ";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Variable/array %s can not be double-aligned,"
				   " because it is equivalenced");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_ALIGN_PERF);
  ccm_attrs[vindex].vis = CCMV_FE;
  ccm_attrs[vindex].name = "CCM_ALIGN_PERF";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Alignment of variables in common block may cause"
				   " performance degradation");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_ALIGN_STRUCT);
  ccm_attrs[vindex].vis = CCMV_FE;
  ccm_attrs[vindex].name = "CCM_ALIGN_STRUCT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Alignment of component %s in numeric sequence"
				   " structure %s may cause performance degradation");
  ccm_attrs[vindex].fmt = CCMFMT_S1S2;

  vindex = ccm_vis_index (CCM_TMP_COPY);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TMP_COPY";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %s copied to a temporary");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_TMP_COPYM);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TMP_COPYM";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %s might be copied to a temporary;"
				   " runtime decision made");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_PROC_MISMATCH);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_PROC_MISMATCH";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %d to subprogram %s differs from"
				   " reference on line %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1P2I3;

  vindex = ccm_vis_index (CCM_PROC_MISMATCH2);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_PROC_MISMATCH2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Scalar argument %d to subprogram %s is"
				   " referred to as an array on line %d");
  ccm_attrs[vindex].fmt = CCMFMT_I1P2I3;

  vindex = ccm_vis_index (CCM_PROC_MISMATCH3);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_PROC_MISMATCH3";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Return type/rank from subprogram %s differs"
				   " from return on line %d");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2;

  vindex = ccm_vis_index (CCM_DO_EXPR);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_DO_EXPR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "DO statement bounds lead to no executions of the"
				   " loop");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_AUTO_BND);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_AUTO_BND";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "The bounds for automatic variable %s are not"
				   " available at all entry points; zero-length"
				   " variable might be allocated");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_LIT_PAD);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_LIT_PAD";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "The character string literal %s padded"
				   " to the length specified for the dummy argument");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_ARRAY_LOOP);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_ARRAY_LOOP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Array statement below generated a loop");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_ARRAY_LOOPNEST);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_OBS;
  ccm_attrs[vindex].name = "CCM_ARRAY_LOOPNEST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Array statement below generated %d nested loops");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_ALIGN_PERF2);
  ccm_attrs[vindex].vis = CCMV_FE;
  ccm_attrs[vindex].name = "CCM_ALIGN_PERF2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Alignment of variable %s in common block %s"
				   " may cause a performance degradation");
  ccm_attrs[vindex].fmt = CCMFMT_V1V2;

  vindex = ccm_vis_index (CCM_ALIGN_PERF3);
  ccm_attrs[vindex].vis = CCMV_FE;
  ccm_attrs[vindex].name = "CCM_ALIGN_PERF3";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Alignment of variable %s in blank common may"
				   " cause a performance degradation");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_IO_LOOP_ARRAY);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_IO_LOOP_ARRAY";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "I/O implied do item below generated an array"
				   " section");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_TMPCONST);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_TMPCONST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Implicit invocation of class %s constructor for"
				   " temporary");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_TMPDEST);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_TMPDEST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Implicit invocation of class %s destructor for"
				   " temporary");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_DBL_CONST);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_DBL_CONST";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Double constant %s used in float expression");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_MINLINE);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MINLINE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s inlined from source file %s by"
				   " front-end");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2;

  vindex = ccm_vis_index (CCM_MINLINE2);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MINLINE2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s from source file %s inlined into"
				   " inline copy of method %s by front-end");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2P3;

  vindex = ccm_vis_index (CCM_MINLINE3);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MINLINE3";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it uses keyword"
				   " %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1S2;

  vindex = ccm_vis_index (CCM_MINLINE4);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC | CCMV_UNIMPL;
  ccm_attrs[vindex].name = "CCM_MINLINE4";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s not inlined because it is too"
				   " complex");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_TMP_COPYOUT);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TMP_COPYOUT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %s copied from a temporary");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_TMP_COPYOUTM);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TMP_COPYOUTM";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %s might be copied from a temporary;"
				   " runtime decision made");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_TMP_COPYINOUT);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TMP_COPYINOUT";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %s copied in and out of a temporary");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_TMP_COPYINOUTM);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TMP_COPYINOUTM";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Argument %s might be copied in and out of"
				   " a temporary; runtime decision made");
  ccm_attrs[vindex].fmt = CCMFMT_V1;

  vindex = ccm_vis_index (CCM_ARRAY_LOOP_2);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_ARRAY_LOOP_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Array statement below generated loop %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_ARRAY_LOOPNEST_2);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_ARRAY_LOOPNEST_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Array statement below generated %d nested"
				   " loops: %s");
  ccm_attrs[vindex].fmt = CCMFMT_I1LL2;

  vindex = ccm_vis_index (CCM_IO_LOOP_ARRAY_2);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_IO_LOOP_ARRAY_2";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "I/O implied do item below generated an array"
				   " section: %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_USER_LOOP);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_USER_LOOP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Source loop below has tag %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_FOUND_LOOP);
  ccm_attrs[vindex].vis = CCMV_FE | CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_FOUND_LOOP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Discovered loop below has tag %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_MFUNCTION_LOOP);
  ccm_attrs[vindex].vis = CCMV_LOOP | CCMV_BASIC | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_MFUNCTION_LOOP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Copy in M-function of loop below has tag %s");
  ccm_attrs[vindex].fmt = CCMFMT_L1;

  vindex = ccm_vis_index (CCM_FSIMPLE);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_FSIMPLE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Transformations for fsimple=%d applied");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_STACK);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_STACK";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Function %s requires %d Mbytes of stack"
				   " storage");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2;

  vindex = ccm_vis_index (CCM_TAILRECUR);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_TAILRECUR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Recursive tail call in %s optimized to jump to"
				   " entry point");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_TAILCALL);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC | CCMV_UNIMPL | CCMV_WANT;
  ccm_attrs[vindex].name = "CCM_TAILCALL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to function %s was tail-call optimized");
  ccm_attrs[vindex].fmt = CCMFMT_P1;

  vindex = ccm_vis_index (CCM_NI_EXIT_OR_PSEUDO);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_EXIT_OR_PSEUDO";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains the pseudo instruction %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_NI_BAD_UNARY_OPC);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_UNARY_OPC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains the instruction opcode %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_NI_INT_LDD_ON_V9);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_INT_LDD_ON_V9";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains integer ldd instructions, which are"
				   " deprecated in the v9 architecture");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_LATE_INL_OPC);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_LATE_INL_OPC";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains the instruction opcode %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_NI_BAD_IMM_OP);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_IMM_OP";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because the"
				   " relocation or immediate operand %s is not well"
				   " understood by the optimizer");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_NI_BAD_STATELEAF);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_STATELEAF";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " references the state register %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_NI_BAD_ASR_19);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_ASR_19";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because"
				   " %%asr19 is not supported in pre v8plus code");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_BAD_FSR_USE);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_FSR_USE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because"
				   " references to %%fsr can only be optimized when the"
				   " -iaopts flag is used");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_BAD_REGISTER);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_REGISTER";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " references the register %s");
  ccm_attrs[vindex].fmt = CCMFMT_S1;

  vindex = ccm_vis_index (CCM_NI_NO_RET_VAL);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_NO_RET_VAL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " does not return the value declared");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_DELAY);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_DELAY";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains a non nop delay slot");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_SCALL);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_SCALL";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " calls a function which returns a structure");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_CASE_POSITION);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_CASE_POSITION";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Case block below was placed at position %d"
				   " based on execution frequency");
  ccm_attrs[vindex].fmt = CCMFMT_I1;

  vindex = ccm_vis_index (CCM_CALL_WITH_CODE);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_CALL_WITH_CODE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Call to %s replaced with inline code.  %d"
				   " loops created: %s");
  ccm_attrs[vindex].fmt = CCMFMT_P1I2LL3;

  vindex = ccm_vis_index (CCM_NI_BAD_SP_ADDR);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_SP_ADDR";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains a %%sp+reg address");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_BAD_SP_USAGE);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_BAD_SP_USAGE";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " uses/defines the stack pointer in a non-load/store instruction");
  ccm_attrs[vindex].fmt = CCMFMT_NONE;

  vindex = ccm_vis_index (CCM_NI_MIXED_REG_TYPES);
  ccm_attrs[vindex].vis = CCMV_CG | CCMV_BASIC;
  ccm_attrs[vindex].name = "CCM_NI_MIXED_REG_TYPES";
  ccm_attrs[vindex].msg = catgets (ccm_catd, 99, vindex,
				   "Template could not be early inlined because it"
				   " contains register %s used as both x-register and register pair");
  ccm_attrs[vindex].fmt = CCMFMT_S1;
}
