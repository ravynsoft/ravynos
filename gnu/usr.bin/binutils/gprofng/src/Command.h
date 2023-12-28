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

#ifndef _COMMAND_H
#define _COMMAND_H


#include <stdio.h>
#include <sys/types.h>

#include "Metric.h"
#include "Hist_data.h"
#include "dbe_types.h"
#include "vec.h"
#include "enums.h"

// This enum lists all the commands parsed by er_print
//  The ordering here is not important, but LAST_CMD must
//  be defined as the last command for which a help line will exist
//  Command.cc has a matching list, and the ordering in
//  that list determines what shows up under the help and xhelp commands.
//  In particular, the entry for HELP is the last one printed
//  for the help command, and the entry for HHELP is the last
//  one printed for xhelp.

enum CmdType
{
  // Pathtree-related commands
  FUNCS = 0,
  HOTPCS,
  HOTLINES,
  FDETAIL,
  OBJECTS,
  LDETAIL,
  PDETAIL,
  SOURCE,
  DISASM,
  METRIC_LIST,
  METRICS,
  SORT,
  GPROF,
  GMETRIC_LIST,
  FSINGLE,
  CSINGLE,
  CPREPEND,
  CAPPEND,
  CRMFIRST,
  CRMLAST,
  CALLTREE,
  CALLFLAME,

  // Source/disassembly control commands
  SCOMPCOM,
  STHRESH,
  DCOMPCOM,
  COMPCOM,
  DTHRESH,

  // Heap trace-related commands
  LEAKS,
  ALLOCS,
  HEAP,
  HEAPSTAT,

  // I/O trace-related commands
  IOACTIVITY,
  IOVFD,
  IOCALLSTACK,
  IOSTAT,

  // Race detection related commands
  RACE_EVNTS,
  RACE_SUM,

  // Deadlock detection commands
  DEADLOCK_EVNTS,
  DEADLOCK_SUM,

  // DataSpace commands
  DOBJECTS,
  DO_SINGLE,
  DO_LAYOUT,
  DO_METRIC_LIST,

  // MemorySpace commands
  MEMOBJ,
  MEMOBJLIST,
  MEMOBJDEF,
  MEMOBJDROP,
  MACHINEMODEL,

  // Custom tab commands
  INDXOBJDEF,
  INDXOBJLIST,
  INDXOBJ,
  INDX_METRIC_LIST,

  // Old-style filtering commands
  OBJECT_LIST,
  OBJECT_SELECT,
  SAMPLE_LIST,
  SAMPLE_SELECT,
  THREAD_LIST,
  THREAD_SELECT,
  LWP_LIST,
  LWP_SELECT,
  CPU_LIST,
  CPU_SELECT,

  // Shared Object display commands
  OBJECT_SHOW,
  OBJECT_HIDE,
  OBJECT_API,
  OBJECTS_DEFAULT,

  // the new filtering commands
  FILTERS,

  // Miscellaneous commands
  COMPARE,
  PRINTMODE,
  HEADER,
  OVERVIEW_NEW,
  SAMPLE_DETAIL,
  STATISTICS,
  EXP_LIST,
  DESCRIBE,
  OUTFILE,
  APPENDFILE,
  LIMIT,
  NAMEFMT,
  VIEWMODE,
  EN_DESC,
  SETPATH,
  ADDPATH,
  PATHMAP,
  LIBDIRS,
  SCRIPT,
  VERSION_cmd,
  QUIT,
  PROCSTATS,

  // Experiments handling commands
  ADD_EXP,
  DROP_EXP,
  OPEN_EXP,

  // .rc-only Commands
  DMETRICS,
  DSORT,
  TLMODE,
  TLDATA,
  TABS,
  TIMELINE,
  MPI_TIMELINE,
  MPI_CHART,
  TIMELINE_CLASSIC_TBR,
  SOURCE_V2,
  DISASM_V2,
  RTABS,
  DUALSOURCE,
  SOURCEDISAM,

  HELP,             // this is the last of the commands listed with "help"
  IFREQ,
  DUMPNODES,
  DUMPSTACKS,
  DUMPUNK,
  DUMPFUNC,
  DUMPDOBJS,
  DUMPMAP,
  DUMPENTITIES,
  DUMP_PROFILE,
  DUMP_SYNC,
  DUMP_HWC,
  DUMP_HEAP,
  DUMP_IOTRACE,
  RACE_ACCS,
  DMPI_FUNCS,
  DMPI_MSGS,
  DMPI_EVENTS,
  DMEM,
  DUMP_GC,
  DKILL,
  IGNORE_NO_XHWCPROF,
  IGNORE_FS_WARN,
  QQUIT,
  HHELP,            // this is the last command listed with "xhelp"
  NO_CMD,           // Dummy command, used for headers in help
  DUMMY_CMD,        // Dummy command, used for help

  // unused commands
  LOADOBJECT,
  LOADOBJECT_LIST,
  LOADOBJECT_SELECT,

  // Internal-only Commands
  LAST_CMD,         // No more commands for which a help line is possible
  STDIN,
  COMMENT,
  WHOAMI,

  // Error return "commands"
  AMBIGUOUS_CMD,
  UNKNOWN_CMD
};

typedef struct
{
  const CmdType token;      // command key
  const char *str;          // command string
  const char *alt;          // alternate command string
  const char *arg;          // argument string for help
  const int arg_count;      // no. of arguments
  char **desc;              // description for help
} Cmdtable;

// Command class: never instantiated, completely static
class Command
{
public:

  // look up a string in the command table, return type, set number of args
  static CmdType get_command (char *cmd, int &arg_count, int &param);
  static const char *get_cmd_str (CmdType type);
  static void print_help (char *prog_name, bool cmd_line, bool usermode, FILE *outf);
  static char *get_err_string (Cmd_status err);

  static const char *DEFAULT_METRICS;   // default if no .rc files read
  static const char *DEFAULT_SORT;      // default if no .rc files read
  static const char *DEFAULT_CMD;       // token for default
  static const char *ALL_CMD;           // token for all
  static const char *ANY_CMD;           // token for any
  static const char *NONE_CMD;          // token for none
  static const char *HWC_CMD;           // token for all HWC
  static const char *BIT_CMD;           // token for any bit-derived metric

private:
  static const int user_no;             // the last user command
  static const int hidden_no;           // the last hidden command
  static const int command_no;          // the last parsable command

  static void init_desc ();
  static char *fmt_help (int nc, char head);
};

// Analyzer display tabs
struct DispTab
{
  DispTab (int ntype, int num, bool vis, CmdType token)
  {
    type = ntype;
    order = num;
    visible = vis;
    available = true;
    cmdtoken = token;
  }

  void setAvailability (bool val)       { available = val; }

  int type;             // Display type
  int order;            // Order in which tabs should appear in GUI
  bool visible;         // Is Tab visible
  bool available;       // Is tab available for this experiment
  CmdType cmdtoken;     // command token
  int param;            // command parameter (used for memory space)
};

#endif /* ! _COMMAND_H */
