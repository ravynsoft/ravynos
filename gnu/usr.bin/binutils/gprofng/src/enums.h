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

#ifndef _PERFAN_ENUMS_H
#define _PERFAN_ENUMS_H

#include "comp_com.h"

enum Cmd_status
{
  CMD_OK = 0,
  CMD_BAD,
  CMD_AMBIGUOUS,
  CMD_BAD_ARG,
  CMD_OUTRANGE,
  CMD_INVALID
};

enum LibExpand
{
  LIBEX_SHOW    = 0,
  LIBEX_HIDE    = 1,
  LIBEX_API     = 2
};

enum SrcVisible
{
  SRC_NA        = 0,
  SRC_CODE      = 1,
  SRC_METRIC    = 2
};

enum MetricType
{ // sync enum changes with Settings.java
  MET_NORMAL = 0,   // functions, lines, pcs; src & disasm (non-compare)
  MET_CALL,         // callers-callees
  MET_DATA,         // dataspace
  MET_INDX,         // index objects
  MET_CALL_AGR,     // call tree
  MET_COMMON,       // Analyzer uses for DSP_DISASM, DSP_SOURCE, ...
  MET_IO,           // IO activity
  MET_SRCDIS,       // src & disasm (non comparison mode)
  MET_HEAP          // Heap leaked list
};

enum ValueType
{ // Bitmask     (!) sync enum changes with AnMetric.java
  VAL_NA        = 0,  // nothing specified (use this enum instead of 0)
  VAL_TIMEVAL   = 1,
  VAL_VALUE     = 2,
  VAL_PERCENT   = 4,
  VAL_DELTA     = 8,
  VAL_RATIO     = 16,
  VAL_INTERNAL  = 32,
  VAL_HIDE_ALL  = 64  // hide all, but allows settings to be remembered
};

enum CompCom
{ // no value here can be the same as CCMV_
  COMP_SRC = CCMV_BASIC + 1,
  COMP_SRC_METRIC,
  COMP_NOSRC,
  COMP_HEX,
  COMP_NOHEX,
  COMP_THRESHOLD,
  COMP_CMPLINE,
  COMP_FUNCLINE
};

enum TLStack_align
{
  TLSTACK_ALIGN_ROOT = 1,
  TLSTACK_ALIGN_LEAF
};

enum Reorder_status
{
  REORDER_SUCCESS,
  REORDER_FAIL,
  REORDER_ZERO,
  REORDER_ONE_FUNC,
  REORDER_FILE_OPEN,
  REORDER_FILE_WRITE,
  REORDER_COMP,
  REORDER_NO_LOAD_OBJ,
  REORDER_NO_OBJECT,
  REORDER_INVALID
};

enum AnUtility_state
{
  EXP_SUCCESS     = 0,
  EXP_FAILURE     = 1,
  EXP_INCOMPLETE  = 2,
  EXP_BROKEN      = 4,
  EXP_OBSOLETE    = 8
};

enum Presentation_align_type
{
  TEXT_LEFT     = 1,
  TEXT_CENTER   = 2,
  TEXT_RIGHT    = 3
};

enum Message_type
{
  ERROR_MSG     = 1,
  WARNING_MSG   = 2,
  PSTAT_MSG     = 3,
  PWARN_MSG     = 4
};

enum Presentation_clock_unit
{
  CUNIT_NULL    = -1,
  CUNIT_BYTES   = -2,
  CUNIT_TIME    = -3
};

enum FuncListDisp_type
{
  DSP_FUNCTION      = 1,
  DSP_LINE          = 2,
  DSP_PC            = 3,
  DSP_SOURCE        = 4,
  DSP_DISASM        = 5,
  DSP_SELF          = 6, // not a tab; ID for Callers-Callees fragment data
  DSP_CALLER        = 7,
  DSP_CALLEE        = 8, // not a tab; ID for Callers-Callees callees data
  DSP_CALLTREE      = 9,
  DSP_TIMELINE      = 10,
  DSP_STATIS        = 11,
  DSP_EXP           = 12,
  DSP_LEAKLIST      = 13,
  DSP_MEMOBJ        = 14, // requires a specific subtype to define a tab
  DSP_DATAOBJ       = 15,
  DSP_DLAYOUT       = 16,
  DSP_SRC_FILE      = 17, // not a tab; Details information (?)
  DSP_IFREQ         = 18,
  DSP_RACES         = 19,
  DSP_INDXOBJ       = 20, // requires a specific subtype to define a tab
  DSP_DUALSOURCE    = 21,
  DSP_SOURCE_DISASM = 22,
  DSP_DEADLOCKS     = 23,
  DSP_MPI_TL        = 24,
  DSP_MPI_CHART     = 25,
  //DSP_TIMELINE_CLASSIC_TBR   = 26,
  DSP_SOURCE_V2     = 27, // comparison
  DSP_DISASM_V2     = 28, // comparison
  //DSP_THREADS_TL    = 29;
  //DSP_THREADS_CHART = 30;
  DSP_IOACTIVITY    = 31,
  DSP_OVERVIEW      = 32,
  DSP_IOVFD         = 33,
  DSP_IOCALLSTACK   = 34,
  DSP_MINICALLER    = 37,
  DSP_HEAPCALLSTACK = 39,
  DSP_CALLFLAME     = 40,
  DSP_SAMPLE        = 99
};

enum CmpMode
{
  CMP_DISABLE   = 0,
  CMP_ENABLE    = 1,
  CMP_RATIO     = 2,
  CMP_DELTA     = 4
};

enum PrintMode
{
  PM_TEXT = 0,
  PM_HTML = 1,
  PM_DELIM_SEP_LIST = 2
};

#endif // _ENUMS_H
