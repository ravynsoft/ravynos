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

#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#include "gp-defs.h"
#include "Command.h"
#include "DbeSession.h"
#include "MemorySpace.h"
#include "i18n.h"
#include "StringBuilder.h"

const char *Command::DEFAULT_CMD = "default";   // token for default
const char *Command::ALL_CMD     = "all";       // token for all
const char *Command::ANY_CMD     = "any";       // token for any
const char *Command::NONE_CMD    = "none";      // token for none
const char *Command::HWC_CMD     = "hwc";       // token for all HWC
const char *Command::BIT_CMD     = "bit";  // token for any bit-generated metric
const char *Command::DEFAULT_METRICS = "ei.user:name";  // if no .rc files read
const char *Command::DEFAULT_SORT    = "e.user:name";   // if no .rc files read

static char *fhdr, *cchdr, *lahdr, *iohdr, *sdhdr, *lsthdr, *lohdr;
static char *methdr, *othdr, *mischdr, *deflthdr;
static char *selhdr, *filthdr, *outhdr, *exphdr, *obj_allhdr;
static char *unsuphdr, *indxobjhdr;
static char *helphdr, *rahdr, *ddhdr, *typehdr, *typehdr2;

//  This is the list of commands, which governs the parser scan, as
//	well as the help command.
//  A line with the tag NO_CMD is skipped in parsing, but is used
//	to provide subheadings for the help
//  The HELP line must be the last one in the list of commands
//	to be shown by "-help"; The HHELP line must be the
//	last one to be shown by "-xhelp"
//  The LAST_CMD line must be the last one recognized by the parser
//
//  The ordering of this list should match the ordering in the man
//	page, and the subheader lines should match the subheadings in
//	the man page.

static char *desc[LAST_CMD];

static Cmdtable cmd_lst[] = {   // list of commands
  // User Commands
  { NO_CMD, "", NULL, NULL, 0, &fhdr},
  { FUNCS, "functions", NULL, NULL, 0, &desc[FUNCS]},
  { METRICS, "metrics", NULL, "metric_spec", 1, &desc[METRICS]},
  { SORT, "sort", NULL, "metric_spec", 1, &desc[SORT]},
  { FDETAIL, "fsummary", NULL, NULL, 0, &desc[FDETAIL]},
  { FSINGLE, "fsingle", NULL, "function_name #", 2, &desc[FSINGLE]},

  { NO_CMD, "", NULL, NULL, 0, &cchdr},
  { GPROF, "callers-callees", "gprof", NULL, 0, &desc[GPROF]},
  { CSINGLE, "csingle", NULL, "function_name #", 2, &desc[CSINGLE]},
  { CPREPEND, "cprepend", NULL, "function_name #", 2, &desc[CPREPEND]},
  { CAPPEND, "cappend", NULL, "function_name #", 2, &desc[CAPPEND]},
  { CRMFIRST, "crmfirst", NULL, NULL, 0, &desc[CRMFIRST]},
  { CRMLAST, "crmlast", NULL, NULL, 0, &desc[CRMLAST]},
  { CALLTREE, "calltree", "ctree", NULL, 0, &desc[CALLTREE]},

  { NO_CMD, "", NULL, NULL, 0, &lahdr},
  { LEAKS, "leaks", NULL, NULL, 0, &desc[LEAKS]},
  { ALLOCS, "allocs", NULL, NULL, 0, &desc[ALLOCS]},
  { HEAP, "heap", NULL, NULL, 0, &desc[HEAP]},
  { HEAPSTAT, "heapstat", NULL, NULL, 0, &desc[HEAPSTAT]},

  { NO_CMD, "", NULL, NULL, 0, &iohdr},
  { IOACTIVITY, "ioactivity", NULL, NULL, 0, &desc[IOACTIVITY]},
  { IOVFD, "iodetail", NULL, NULL, 0, &desc[IOVFD]},
  { IOCALLSTACK, "iocallstack", NULL, NULL, 0, &desc[IOCALLSTACK]},
  { IOSTAT, "iostat", NULL, NULL, 0, &desc[IOSTAT]},

  // PC, line, source and dissassembly commands
  { NO_CMD, "", NULL, NULL, 0, &sdhdr},
  { HOTPCS, "pcs", NULL, NULL, 0, &desc[HOTPCS]},
  { PDETAIL, "psummary", NULL, NULL, 0, &desc[PDETAIL]},
  { HOTLINES, "lines", NULL, NULL, 0, &desc[HOTLINES]},
  { LDETAIL, "lsummary", NULL, NULL, 0, &desc[LDETAIL]},
  { SOURCE, "source", NULL, "func/file #", 2, &desc[SOURCE]},
  { DISASM, "disasm", NULL, "func/file #", 2, &desc[DISASM]},
  { SCOMPCOM, "scc", NULL, "com_spec", 1, &desc[SCOMPCOM]},
  { STHRESH, "sthresh", NULL, "value", 1, &desc[STHRESH]},
  { DCOMPCOM, "dcc", NULL, "com_spec", 1, &desc[DCOMPCOM]},
  { COMPCOM, "cc", NULL, "com_spec", 1, &desc[COMPCOM]},
  { DTHRESH, "dthresh", NULL, "value", 1, &desc[DTHRESH]},
  { SETPATH, "setpath", NULL, "path_list", 1, &desc[SETPATH]},
  { ADDPATH, "addpath", NULL, "path_list", 1, &desc[ADDPATH]},
  { PATHMAP, "pathmap", NULL, "old_prefix new_prefix", 2, &desc[PATHMAP]},
  { LIBDIRS, "preload_libdirs", NULL, NULL, 1, &desc[PATHMAP]},

  // Index Object commands
  { NO_CMD, "", NULL, NULL, 0, &indxobjhdr},
  { INDXOBJ, "indxobj", NULL, "type", 1, &desc[INDXOBJ]},
  { INDXOBJLIST, "indxobj_list", NULL, NULL, 0, &desc[INDXOBJLIST]},
  { INDXOBJDEF, "indxobj_define", NULL, "type \"index-expr\"", 2, &desc[INDXOBJDEF]},

  // Deadlock detection commands
  { NO_CMD, "", NULL, NULL, 0, &ddhdr},
  { DEADLOCK_EVNTS, "deadlocks", NULL, NULL, 0, &desc[DEADLOCK_EVNTS]},
  { DEADLOCK_SUM, "dsummary", NULL, "{deadlock_id|all}", 1, &desc[DEADLOCK_SUM]},

  { NO_CMD, "", NULL, NULL, 0, &lsthdr},
  { EXP_LIST, "experiment_list", "exp_list", NULL, 0, &desc[EXP_LIST]},
  { SAMPLE_LIST, "sample_list", NULL, NULL, 0, &desc[SAMPLE_LIST]},
  { LWP_LIST, "lwp_list", NULL, NULL, 0, &desc[LWP_LIST]},
  { THREAD_LIST, "thread_list", NULL, NULL, 0, &desc[THREAD_LIST]},
  { CPU_LIST, "cpu_list", NULL, NULL, 0, &desc[CPU_LIST]},

  { NO_CMD, "", NULL, NULL, 0, &filthdr},
  { FILTERS, "filters", NULL, "filter-specification", 1, &desc[FILTERS]},
  { DESCRIBE, "describe", NULL, NULL, 0, &desc[DESCRIBE]},

  { NO_CMD, "", NULL, NULL, 0, &selhdr},
  { SAMPLE_SELECT, "sample_select", NULL, "sample_spec", 1, &desc[SAMPLE_SELECT]},
  { LWP_SELECT, "lwp_select", NULL, "lwp_spec", 1, &desc[LWP_SELECT]},
  { THREAD_SELECT, "thread_select", NULL, "thread_spec", 1, &desc[THREAD_SELECT]},
  { CPU_SELECT, "cpu_select", NULL, "cpu_spec", 1, &desc[CPU_SELECT]},

  { NO_CMD, "", NULL, NULL, 0, &lohdr},
  { OBJECT_LIST, "object_list", NULL, NULL, 0, &desc[OBJECT_LIST]},
  { OBJECT_SHOW, "object_show", NULL, "obj1,...", 1, &desc[OBJECT_SHOW]},
  { OBJECT_HIDE, "object_hide", NULL, "obj1,...", 1, &desc[OBJECT_HIDE]},
  { OBJECT_API, "object_api", NULL, "obj1,...", 1, &desc[OBJECT_API]},
  { DUMMY_CMD, " ", NULL, NULL, 0, &obj_allhdr},
  { OBJECTS_DEFAULT, "objects_default", NULL, NULL, 1, &desc[OBJECTS_DEFAULT]},

  { OBJECT_SELECT, "object_select", NULL, "obj1,...", 1, &desc[OBJECT_SELECT]},

  { NO_CMD, "", NULL, NULL, 0, &methdr},
  { METRIC_LIST, "metric_list", NULL, NULL, 0, &desc[METRIC_LIST]},
  { GMETRIC_LIST, "cmetric_list", "gmetric_list", NULL, 0, &desc[GMETRIC_LIST]},
  { INDX_METRIC_LIST, "indx_metric_list", NULL, NULL, 1, &desc[INDX_METRIC_LIST]},

  { NO_CMD, "", NULL, NULL, 0, &outhdr},
  { OUTFILE, "outfile", NULL, "filename", 1, &desc[OUTFILE]},
  { APPENDFILE, "appendfile", NULL, "filename", 1, &desc[APPENDFILE]},
  { LIMIT, "limit", NULL, "n", 1, &desc[LIMIT]},
  { NAMEFMT, "name", NULL, "{long|short|mangled}[:{soname|nosoname}]", 1, &desc[NAMEFMT]},
  { VIEWMODE, "viewmode", NULL, "{user|expert|machine}", 1, &desc[VIEWMODE]},
  { COMPARE, "compare", NULL, "{on|off|delta|ratio}", 1, &desc[COMPARE]},
  { PRINTMODE, "printmode", NULL, "string", 1, &desc[PRINTMODE]},

  { NO_CMD, "", NULL, NULL, 0, &othdr},
  { HEADER, "header", NULL, "exp_id", 1, &desc[HEADER]},
  { OBJECTS, "objects", NULL, NULL, 0, &desc[OBJECTS]},
  { OVERVIEW_NEW, "overview", NULL, NULL, 0, &desc[OVERVIEW_NEW]},
  { SAMPLE_DETAIL, "sample_detail", NULL, "exp_id", 1, &desc[SAMPLE_DETAIL]},
  { STATISTICS, "statistics", NULL, "exp_id", 1, &desc[STATISTICS]},

  { NO_CMD, "", NULL, NULL, 0, &exphdr},
  { OPEN_EXP, "open_exp", NULL, "experiment", 1, &desc[OPEN_EXP]},
  { ADD_EXP, "add_exp", NULL, "experiment", 1, &desc[ADD_EXP]},
  { DROP_EXP, "drop_exp", NULL, "experiment", 1, &desc[DROP_EXP]},

  { NO_CMD, "", NULL, NULL, 0, &deflthdr},
  { DMETRICS, "dmetrics", NULL, "metric_spec", 1, &desc[DMETRICS]},
  { DSORT, "dsort", NULL, "metric_spec", 1, &desc[DSORT]},
  { EN_DESC, "en_desc", NULL, "{on|off|=<regex>}", 1, &desc[EN_DESC]},

  { NO_CMD, "", NULL, NULL, 0, &mischdr},
  { DUMMY_CMD, "<type>", NULL, NULL, 0, &typehdr},
  { DUMMY_CMD, " ", NULL, NULL, 0, &typehdr2},

  { IFREQ, "ifreq", NULL, NULL, 0, &desc[IFREQ]},
  { PROCSTATS, "procstats", NULL, NULL, 0, &desc[PROCSTATS]},
  { SCRIPT, "script", NULL, "file", 1, &desc[SCRIPT]},
  { VERSION_cmd, "version", NULL, NULL, 0, &desc[VERSION_cmd]},
  { QUIT, "quit", "exit", NULL, 0, &desc[QUIT]},

  { NO_CMD, "", NULL, NULL, 0, &helphdr},
  { HELP, "help", NULL, NULL, 0, &desc[HELP]},

  { NO_CMD, "", NULL, NULL, 0, &unsuphdr},
  { HELP, "-help", NULL, NULL, 0, &desc[HELP]},
  { DUMPFUNC, "dfuncs", NULL, "string", 1, &desc[DUMPFUNC]},
  { DUMPDOBJS, "ddobjs", NULL, "string", 1, &desc[DUMPDOBJS]},
  { DUMPNODES, "dnodes", NULL, NULL, 0, &desc[DUMPNODES]},
  { DUMPSTACKS, "dstacks", NULL, NULL, 0, &desc[DUMPSTACKS]},
  { DUMPUNK, "dunkpc", NULL, NULL, 0, &desc[DUMPUNK]},
  { DUMPMAP, "dmap", NULL, NULL, 0, &desc[DUMPMAP]},
  { DUMPENTITIES, "dentities", NULL, NULL, 0, &desc[DUMPENTITIES]},
  { IGNORE_NO_XHWCPROF, "ignore_no_xhwcprof", NULL, NULL, 0, &desc[IGNORE_NO_XHWCPROF]},
  { IGNORE_FS_WARN, "ignore_fs_warn", NULL, NULL, 0, &desc[IGNORE_FS_WARN]},

  { DUMP_PROFILE, "dprofile", NULL, NULL, 0, &desc[DUMP_PROFILE]},
  { DUMP_SYNC, "dsync", NULL, NULL, 0, &desc[DUMP_SYNC]},
  { DUMP_IOTRACE, "diotrace", NULL, NULL, 0, &desc[DUMP_IOTRACE]},
  { DUMP_HWC, "dhwc", NULL, NULL, 0, &desc[DUMP_HWC]},
  { DUMP_HEAP, "dheap", NULL, NULL, 0, &desc[DUMP_HEAP]},
  { RACE_ACCS, "r_accs", NULL, NULL, 0, &desc[RACE_ACCS]},

  { DMPI_FUNCS, "dmpi_funcs", NULL, NULL, 0, &desc[DMPI_FUNCS]},
  { DMPI_MSGS, "dmpi_msgs", NULL, NULL, 0, &desc[DMPI_MSGS]},
  { DMPI_EVENTS, "dmpi_events", NULL, NULL, 0, &desc[DMPI_EVENTS]},

  { DMEM, "dmem", NULL, NULL, 1, &desc[DMEM]},
  { DUMP_GC, "dumpgc", NULL, NULL, 0, &desc[DUMP_GC]},
  { DKILL, "dkill", NULL, NULL, 2, &desc[DKILL]},

  { QQUIT, "xquit", NULL, NULL, 0, &desc[QQUIT]},
  // use xquit for memory leak detection in dbe; it's
  // like quit, but deletes all data loaded

  { HHELP, "xhelp", NULL, NULL, 0, &desc[HHELP]},
  { WHOAMI, "whoami", NULL, NULL, 0, NULL},

  // these are not recognized at this point
  { LOADOBJECT, "segments", "pmap", NULL, 0, &desc[LOADOBJECT]},
  { LOADOBJECT_LIST, "segment_list", NULL, NULL, 0, &desc[LOADOBJECT_LIST]},
  { LOADOBJECT_SELECT, "segment_select", NULL, "seg1,...", 1, &desc[LOADOBJECT_SELECT]},

  { LAST_CMD, "xxxx", NULL, NULL, 0, NULL}
};

CmdType
Command::get_command (char *cmd, int &arg_count, int &cparam)
{
  int i;
  int len = (int) strlen (cmd);
  bool got = false;
  CmdType token = UNKNOWN_CMD;
  arg_count = 0;
  cparam = -1;
  if (*cmd == '\0') // - command
    return STDIN;
  if (*cmd == '#') // comment
    return COMMENT;
  if (strcmp (cmd, "V") == 0 || strcmp (cmd, "-version") == 0)
    return VERSION_cmd;
  if (strcmp (cmd, "-help") == 0)
    return HELP;
  if (strncmp (cmd, NTXT ("-whoami="), 8) == 0)
    {
      cparam = 8;
      return WHOAMI;
    }

  if (*cmd == '-')
    cmd++;
  for (i = 0;; i++)
    {
      if (cmd_lst[i].token == LAST_CMD)
	break;
      if (!strncasecmp (cmd, cmd_lst[i].str, len) ||
	  (cmd_lst[i].alt && !strncasecmp (cmd, cmd_lst[i].alt, len)))
	{
	  // Is it unambiguous?
	  if (!strcasecmp (cmd, cmd_lst[i].str)
	      || (cmd_lst[i].alt && !strcasecmp (cmd, cmd_lst[i].alt)))
	    {
	      // exact, full-length match
	      token = cmd_lst[i].token;
	      arg_count = cmd_lst[i].arg_count;
	      return token;
	    }
	  if (got)
	    return AMBIGUOUS_CMD;
	  got = true;
	  token = cmd_lst[i].token;
	  arg_count = cmd_lst[i].arg_count;
	}
    }

  // Did we find it?
  if (token != UNKNOWN_CMD)
    return token;

  // See if it's the name of a index object
  if (dbeSession)
    {
      int indxtype = dbeSession->findIndexSpaceByName (cmd);
      if (indxtype >= 0)
	{
	  // found it
	  cparam = indxtype;
	  return INDXOBJ;
	}
    }
  return token;
}

const char *
Command::get_cmd_str (CmdType type)
{
  for (int i = 0;; i++)
    {
      if (cmd_lst[i].token == LAST_CMD)
	break;
      if (type == cmd_lst[i].token)
	return cmd_lst[i].str;
    }
  return "xxxx";
}

char *
Command::get_err_string (Cmd_status err)
{
  switch (err)
    {
    case CMD_OK:
      return NULL;
    case CMD_BAD:
      return GTXT ("command bad");
    case CMD_AMBIGUOUS:
      return GTXT ("command ambiguous");
    case CMD_BAD_ARG:
      return GTXT ("Invalid argument to command");
    case CMD_OUTRANGE:
      return GTXT ("argument to command is out-of-range");
    case CMD_INVALID:
      return GTXT ("invalid command");
    }
  return NULL;
}

void
Command::print_help (char *prog_name, bool cmd_line, bool usermode, FILE *outf)
{
  char *fmt, *msg;
  int i;
  StringBuilder sb;
  enum CmdType nc;
  init_desc ();
  if (usermode)  // show the hidden ones, too
    nc = HELP;
  else
    nc = HHELP;

  if (cmd_line)
    fprintf (outf, GTXT ("Usage: %s [ -script script | -command | - ] exper_1 ... exper_n\n"),
	     prog_name);
  fprintf (outf, GTXT ("An alternate spelling for a command is shown in [], where applicable.\n\n"
	  "Those commands followed by a * may appear in .rc files.\n\n"
	  "Those commands followed by a $ can only appear in .rc files.\n\n"));
  fmt = fmt_help (nc, ' ');
  for (i = 0;; i++)
    {
      // check for end of list
      if (cmd_lst[i].token == LAST_CMD)
	break;
      if (cmd_lst[i].token == NO_CMD)   // this is a header line
	fprintf (outf, NTXT (" %s\n"), *cmd_lst[i].desc);
      else
	{
	  if (strlen (cmd_lst[i].str) == 0)
	    continue;
	  // this is a real command line
	  sb.setLength (0);
	  sb.append (cmd_lst[i].str);
	  if (cmd_lst[i].alt)
	    {
	      sb.append ('[');
	      sb.append (cmd_lst[i].alt);
	      sb.append (']');
	    }
	  if (cmd_lst[i].arg)
	    {
	      sb.append (' ');
	      sb.append (cmd_lst[i].arg);
	    }
	  msg = sb.toString ();
	  fprintf (outf, fmt, msg, *cmd_lst[i].desc);
	  free (msg);
	}
      // check for end of list
      if (cmd_lst[i].token == nc)
	break;
    }
}

//  construct format for printing help
char *
Command::fmt_help (int nc, char head)
{
  int len, max_len, i;
  static char fmt[BUFSIZ];

  max_len = 0;
  for (i = 0; i < nc; i++)
    {
      len = (int) strlen (cmd_lst[i].str);
      if (cmd_lst[i].alt)
	len += (int) strlen (cmd_lst[i].alt) + 2;
      if (cmd_lst[i].arg)
	len += (int) strlen (cmd_lst[i].arg) + 2;
      if (max_len < len)
	max_len = len;
    }
  snprintf (fmt, sizeof (fmt), NTXT ("    %c%%-%ds %%s\n"), head, max_len + 1);
  return fmt;
}

void
Command::init_desc ()
{
  if (desc[0] != NULL)
    return;
  desc[FUNCS] = GTXT ("display functions with current metrics");
  desc[HOTPCS] = GTXT ("display hot PC's with current metrics");
  desc[HOTLINES] = GTXT ("display hot lines with current metrics");
  desc[FDETAIL] = GTXT ("display summary metrics for each function");
  desc[OBJECTS] = GTXT ("display object list with errors or warnings");
  desc[COMPARE] = GTXT ("enable comparison mode for experiments *");
  desc[PRINTMODE] = GTXT ("set the mode for printing tables *");
  desc[LDETAIL] = GTXT ("display summary metrics for each hot line");
  desc[PDETAIL] = GTXT ("display summary metrics for each hot PC");
  desc[SOURCE] = GTXT ("display annotated source for function/file");
  desc[DISASM] = GTXT ("display annotated disassembly for function/file");
  desc[SCOMPCOM] = GTXT ("set compiler commentary classes for source *");
  desc[STHRESH] = GTXT ("set highlight threshold for source *");
  desc[DCOMPCOM] = GTXT ("set compiler commentary classes for disasm *");
  desc[COMPCOM] = GTXT ("set compiler commentary classes for both source and disasm *");
  desc[DTHRESH] = GTXT ("set highlight threshold for disasm *");
  desc[METRIC_LIST] = GTXT ("display the available metrics and dmetrics keywords");
  desc[METRICS] = GTXT ("set a new list of metrics");
  desc[SORT] = GTXT ("sort tables by the specified metric");
  desc[GPROF] = GTXT ("display the callers-callees for each function");
  desc[CALLTREE] = GTXT ("display the tree of function calls");
  desc[CALLFLAME] = GTXT ("request calltree flame chart -- not a command, but used in the tabs command");
  desc[GMETRIC_LIST] = GTXT ("display the available callers-callees metrics");
  desc[FSINGLE] = GTXT ("display the summary metrics for specified function");
  desc[CSINGLE] = GTXT ("display the callers-callees for the specified function");
  desc[CPREPEND] = GTXT ("add specified function to the head of the callstack fragment");
  desc[CAPPEND] = GTXT ("add specified function to the end of the callstack fragment");
  desc[CRMFIRST] = GTXT ("remove the first function from the callstack fragment");
  desc[CRMLAST] = GTXT ("remove the last function from the callstack fragment");
  desc[LEAKS] = GTXT ("display memory leaks, aggregated by callstack");
  desc[ALLOCS] = GTXT ("display allocations, aggregated by callstack");
  desc[HEAP] = GTXT ("display memory allocations and leaks, aggregated by callstack");
  desc[HEAPSTAT] = GTXT ("display heap statistics report");
  desc[IOACTIVITY] = GTXT ("display I/O activity report, aggregated by file name");
  desc[IOVFD] = GTXT ("display I/O activity report, aggregated by file descriptor");
  desc[IOCALLSTACK] = GTXT ("display I/O activity report, aggregated by callstack");
  desc[IOSTAT] = GTXT ("display I/O statistics report");
  desc[RACE_ACCS] = GTXT ("dump race access events");
  desc[DMPI_MSGS] = GTXT ("dump mpi messages");
  desc[DMPI_FUNCS] = GTXT ("dump mpi function calls");
  desc[DMPI_EVENTS] = GTXT ("dump mpi trace events");
  desc[DMEM] = GTXT ("debug command for internal use");
  desc[DUMP_GC] = GTXT ("dump Java garbage collector events");
  desc[DKILL] = GTXT ("send process p signal s");
  desc[DEADLOCK_EVNTS] = GTXT ("display deadlock events");
  desc[DEADLOCK_SUM] = GTXT ("display summary for the deadlock event");
  desc[HEADER] = GTXT ("display information about the experiment");
  desc[OVERVIEW_NEW] = GTXT ("display the overview of all loaded experiments");
  desc[SAMPLE_DETAIL] = GTXT ("display the current sample list with data");
  desc[STATISTICS] = GTXT ("display the execution statistics data");
  desc[EXP_LIST] = GTXT ("display the existing experiments");
  desc[DESCRIBE] = GTXT ("describe recorded data and tokens available for filtering data");
  desc[OBJECT_SHOW] = GTXT ("set load objects to show all functions *");
  desc[OBJECT_HIDE] = GTXT ("set load objects to hide functions *");
  desc[OBJECT_API] = GTXT ("set load objects to show API (entry point) only *");
  desc[OBJECTS_DEFAULT] = GTXT ("reset load objects show to defaults");
  desc[OBJECT_LIST] = GTXT ("display load objects, functions-shown flag");
  desc[OBJECT_SELECT] = GTXT ("set list of load objects whose functions are shown");
  desc[SAMPLE_LIST] = GTXT ("display the list of existing samples");
  desc[SAMPLE_SELECT] = GTXT ("set a new list of samples");
  desc[THREAD_LIST] = GTXT ("display the list of existing threads");
  desc[THREAD_SELECT] = GTXT ("set a new list of threads");
  desc[LWP_LIST] = GTXT ("display the list of existing LWPs");
  desc[LWP_SELECT] = GTXT ("set a new list of LWPs");
  desc[CPU_LIST] = GTXT ("display the list of CPUs");
  desc[CPU_SELECT] = GTXT ("set a new list of CPUs");
  desc[OUTFILE] = GTXT ("open filename for subsequent output");
  desc[APPENDFILE] = GTXT ("open filename for subsequent appended output");
  desc[LIMIT] = GTXT ("limit output to the first n entries (n=0 for no limit)");
  desc[NAMEFMT] = GTXT ("set long/short/mangled names for functions *");
  desc[VIEWMODE] = GTXT ("set viewmode user|expert|machine *");
  desc[EN_DESC] = GTXT ("enable descendant processes on|off|regex matches lineage or program name $");
  desc[SETPATH] = GTXT ("set search path for annotated src/dis");
  desc[ADDPATH] = GTXT ("add search path for annotated src/dis *");
  desc[PATHMAP] = GTXT ("remap path prefix for annotated src/dis *");
  desc[LIBDIRS] = GTXT ("set path where the gprofng libraries are installed");
  desc[SCRIPT] = GTXT ("read er_print commands from script file");
  desc[PROCSTATS] = GTXT ("display processing statistics");
  desc[ADD_EXP] = GTXT ("add experiment or group");
  desc[DROP_EXP] = GTXT ("drop experiment");
  desc[OPEN_EXP] = GTXT ("open experiment or group (drops all loaded experiments first)");
  desc[VERSION_cmd] = GTXT ("display the current release version");
  desc[HELP] = GTXT ("display the list of available commands");
  desc[QUIT] = GTXT ("terminate processing and exit");
  desc[DMETRICS] = GTXT ("set default function list metrics $");
  desc[DSORT] = GTXT ("set default function list sort metric $");
  desc[TLMODE] = GTXT ("set default timeline mode, align, depth $");
  desc[TLDATA] = GTXT ("set default timeline visible data $");
  desc[TABS] = GTXT ("set default visible tabs $");
  desc[RTABS] = GTXT ("set default visible tabs for Thread Analyzer Experiment $");
  desc[INDXOBJ] = GTXT ("display index objects of a specified type with current metrics");
  desc[INDXOBJLIST] = GTXT ("display list of index objects");
  desc[INDXOBJDEF] = GTXT ("define a new index object type *");
  desc[INDX_METRIC_LIST] = GTXT ("display the available index object metrics");
  desc[IFREQ] = GTXT ("display instruction-frequency report");
  desc[TIMELINE] = GTXT ("request timeline -- not a command, but used in the tabs command");
  desc[MPI_TIMELINE] = GTXT ("request mpi-timeline -- not a command, but used in the tabs command");
  desc[MPI_CHART] = GTXT ("request mpi chart -- not a command, but used in the tabs command");
  desc[DUALSOURCE] = GTXT ("request dualsource tab -- not a command, but used in the tabs command");
  desc[SOURCEDISAM] = GTXT ("request source/disassembly tab -- not a command, but used in the tabs command");
  desc[DUMPNODES] = GTXT ("dump pathtree node table");
  desc[DUMPSTACKS] = GTXT ("dump Experiment callstack tables");
  desc[DUMPUNK] = GTXT ("dump <Unknown> PCs");
  desc[DUMPFUNC] = GTXT ("dump functions whose name matches string");
  desc[DUMPDOBJS] = GTXT ("dump dataobjects whose name matches string");
  desc[DUMPMAP] = GTXT ("dump load-object map");
  desc[DUMPENTITIES] = GTXT ("dump threads, lwps, cpus");
  desc[DUMP_PROFILE] = GTXT ("dump clock profile events");
  desc[DUMP_SYNC] = GTXT ("dump synchronization trace events");
  desc[DUMP_IOTRACE] = GTXT ("dump IO trace events");
  desc[DUMP_HWC] = GTXT ("dump HWC profile events");
  desc[DUMP_HEAP] = GTXT ("dump heap trace events");
  desc[IGNORE_NO_XHWCPROF] = GTXT ("ignore absence of -xhwcprof info in dataspace profiling $");
  desc[IGNORE_FS_WARN] = GTXT ("ignore filesystem (nfs, ...) warning $");
  desc[HHELP] = GTXT ("display help including unsupported commands");
  desc[QQUIT] = GTXT ("terminate processing and exit");
  desc[LOADOBJECT] = GTXT ("display the address map with current metrics");
  desc[LOADOBJECT_LIST] = GTXT ("display segments, indicating which are selected");
  desc[LOADOBJECT_SELECT] = GTXT ("set a new list of segments");
  desc[FILTERS] = GTXT ("define a filter");

  fhdr = GTXT ("\nCommands controlling the function list:");
  cchdr = GTXT ("\nCommands controlling the callers-callees and calltree lists:");
  lahdr = GTXT ("\nCommands controlling the leak and allocation lists:");
  iohdr = GTXT ("\nCommand controlling the I/O activity report:");
  rahdr = GTXT ("\nCommands controlling the race events lists:");
  ddhdr = GTXT ("\nCommands controlling the deadlock events lists:");
  typehdr = GTXT ("equivalent to \"memobj type\", or \"indxobj type\"");
  typehdr2 = GTXT ("  where type is a memory object or index object type");
  sdhdr = GTXT ("\nCommands controlling the source and disassembly listings:");
  lsthdr = GTXT ("\nCommands listing experiments, samples and threads:");
  lohdr = GTXT ("\nCommands controlling load object selection:");
  obj_allhdr = GTXT ("  the special object name `all' refers to all load objects");
  methdr = GTXT ("\nCommands that list metrics:");
  othdr = GTXT ("\nCommands that print other displays:");
  outhdr = GTXT ("\nCommands that control output:");
  mischdr = GTXT ("\nMiscellaneous commands:");
  exphdr = GTXT ("\nCommands for experiments (scripts and interactive mode only):");
  deflthdr = GTXT ("\nDefault-setting commands:");
  selhdr = GTXT ("\nCommands controlling old-style filters/selection:");
  filthdr = GTXT ("\nCommands controlling filters:");
  indxobjhdr = GTXT ("\nCommands controlling the index objects:");
  unsuphdr = GTXT ("\nUnsupported commands:");
  helphdr = GTXT ("\nHelp command:");
}
