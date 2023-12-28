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

#ifndef _ABS_H
#define _ABS_H

/*
 * Apropos Backtracking Scheme definitions.
 * Class: com_sun_forte_st_mpmt_timeline_HWCEvent
 */

/* ABS failure codes */
typedef enum
{
  ABS_NULL          = 0x00,     /* undefined/disabled/inactive */
  ABS_UNSUPPORTED   = 0x01,     /* inappropriate HWC event type */
  ABS_BLOCKED       = 0x02,     /* runtime backtrack blocker reached */
  ABS_INCOMPLETE    = 0x03,     /* runtime backtrack limit reached */
  ABS_REG_LOSS      = 0x04,     /* address register contaminated */
  ABS_INVALID_EA    = 0x05,     /* invalid effective address value */
  ABS_NO_CTI_INFO   = 0x10,     /* no AnalyzerInfo for validation */
  ABS_INFO_FAILED   = 0x20,     /* info failed to validate backtrack */
  ABS_CTI_TARGET    = 0x30,     /* CTI target invalidated backtrack */
  ABS_CODE_RANGE    = 0xFF      /* reserved ABS code range in Vaddr */
} ABS_code;

enum {
  NUM_ABS_RT_CODES = 7,
  NUM_ABS_PP_CODES = 5
};

extern const char *ABS_RT_CODES[NUM_ABS_RT_CODES];
extern char *ABS_PP_CODES[NUM_ABS_PP_CODES];

/* libcollector will mark HWC overflow values that appear to be invalid */
/* dbe should check HWC values for errors */
#define HWCVAL_ERR_FLAG         (1ULL<<63)
#define HWCVAL_SET_ERR(ctr)     ((ctr) | HWCVAL_ERR_FLAG)
#define HWCVAL_HAS_ERR(ctr)     (((ctr) & HWCVAL_ERR_FLAG) != 0)
#define HWCVAL_CLR_ERR(ctr)     ((ctr) & ~HWCVAL_ERR_FLAG)

#define ABS_GET_RT_CODE(EA)     ((EA) & 0x0FLL)
#define ABS_GET_PP_CODE(EA)     (((EA) & 0xF0LL) / 0xF)

#endif /* _ABS_H */
