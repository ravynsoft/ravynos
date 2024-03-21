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
#include <unistd.h>
#include <sys/param.h>

#include "enums.h"
#include "Settings.h"
#include "DbeSession.h"
#include "Command.h"
#include "Application.h"
#include "MemorySpace.h"
#include "StringBuilder.h"
#include "Table.h"
#include "Emsg.h"
#include "util.h"
#include "i18n.h"

// Commands for compiler commentary
static const char *comp_cmd[] = {
  NTXT ("basic"),
  NTXT ("version"),
  NTXT ("warn"),
  NTXT ("parallel"),
  NTXT ("query"),
  NTXT ("loop"),
  NTXT ("pipe"),
  NTXT ("inline"),
  NTXT ("memops"),
  NTXT ("fe"),
  NTXT ("codegen"),
  NTXT ("src"),
  NTXT ("asrc"),
  NTXT ("nosrc"),
  NTXT ("hex"),
  NTXT ("nohex"),
  NTXT ("threshold"),
  NTXT ("cf")
};

static const int comp_vis[] = {
  CCMV_BASIC,
  CCMV_VER,
  CCMV_WARN,
  CCMV_PAR,
  CCMV_QUERY,
  CCMV_LOOP,
  CCMV_PIPE,
  CCMV_INLINE,
  CCMV_MEMOPS,
  CCMV_FE,
  CCMV_CG,
  COMP_SRC,
  COMP_SRC_METRIC,
  COMP_NOSRC,
  COMP_HEX,
  COMP_NOHEX,
  COMP_THRESHOLD,
  COMP_CMPLINE
};

const int comp_size = sizeof (comp_cmd) / sizeof (char *);

// Commands for timeline
typedef enum
{
  TLCMD_INVALID,
  TLCMD_ENTITY_MODE,
  TLCMD_ALIGN,
  TLCMD_DEPTH
} TLModeSubcommand;

typedef struct
{
  const char * cmdText;
  TLModeSubcommand cmdType;
  int cmdId;
} TLModeCmd;
static const TLModeCmd tlmode_cmd[] = {
  // MODE commands
  {NTXT ("lwp"),        TLCMD_ENTITY_MODE, PROP_LWPID},
  {NTXT ("thread"),     TLCMD_ENTITY_MODE, PROP_THRID},
  {NTXT ("cpu"),        TLCMD_ENTITY_MODE, PROP_CPUID},
  {NTXT ("experiment"), TLCMD_ENTITY_MODE, PROP_EXPID},
  // ALIGN commands
  {NTXT ("root"),       TLCMD_ALIGN, TLSTACK_ALIGN_ROOT},
  {NTXT ("leaf"),       TLCMD_ALIGN, TLSTACK_ALIGN_LEAF},
  // DEPTH commands
  {NTXT ("depth"),      TLCMD_DEPTH, 0 /* don't care */}
};

static const int tlmode_size = sizeof (tlmode_cmd) / sizeof (TLModeCmd);

// Constructor

Settings::Settings (Application *_app)
{
  // Remember the application
  app = _app;

  // Clear all default strings
  str_vmode = NULL;
  str_en_desc = NULL;
  str_datamode = NULL;
  str_scompcom = NULL;
  str_sthresh = NULL;
  str_dcompcom = NULL;
  str_dthresh = NULL;
  str_dmetrics = NULL;
  str_dsort = NULL;
  str_tlmode = NULL;
  str_tldata = NULL;
  str_tabs = NULL;
  str_rtabs = NULL;
  str_search_path = NULL;
  str_name_format = NULL;
  str_limit = NULL;
  str_printmode = NULL;
  str_compare = NULL;
  preload_libdirs = NULL;
  pathmaps = new Vector<pathmap_t*>;
  lo_expands = new Vector<lo_expand_t*>;
  lo_expand_default = LIBEX_SHOW;
  is_loexpand_default = true;
  tabs_processed = false;

  // set default-default values
  name_format = Histable::NA;
  view_mode = VMODE_USER;
  en_desc = false;
  en_desc_cmp = NULL;
  en_desc_usr = NULL;
  src_compcom = 2147483647;
  dis_compcom = 2147483647;
#define DEFAULT_SRC_DIS_THRESHOLD 75
  threshold_src = DEFAULT_SRC_DIS_THRESHOLD;
  threshold_dis = DEFAULT_SRC_DIS_THRESHOLD;
  src_visible = true;
  srcmetric_visible = false;
  hex_visible = false;
  cmpline_visible = true;
  funcline_visible = true;
  tldata = NULL;
  tlmode = 0;
  stack_align = 0;
  stack_depth = 0;
  limit = 0;
  // print mode is initialized after the .rc files are read
  print_delim = ',';
  compare_mode = CMP_DISABLE;
  machinemodel = NULL;
  ignore_no_xhwcprof = false;
  ignore_fs_warn = false;

  // construct the master list of tabs
  buildMasterTabList ();

  indx_tab_state = new Vector<bool>;
  indx_tab_order = new Vector<int>;
  mem_tab_state = new Vector<bool>;
  mem_tab_order = new Vector<int>;

  // note that the .rc files are not read here, but later
}

// Constructor for duplicating an existing Settings class

Settings::Settings (Settings * _settings)
{
  int index;
  app = _settings->app;

  // Copy all default strings
  str_vmode = dbe_strdup (_settings->str_vmode);
  str_en_desc = dbe_strdup (_settings->str_en_desc);
  str_datamode = dbe_strdup (_settings->str_datamode);
  str_scompcom = dbe_strdup (_settings->str_scompcom);
  str_sthresh = dbe_strdup (_settings->str_sthresh);
  str_dcompcom = dbe_strdup (_settings->str_dcompcom);
  str_dthresh = dbe_strdup (_settings->str_dthresh);
  str_dmetrics = dbe_strdup (_settings->str_dmetrics);
  str_dsort = dbe_strdup (_settings->str_dsort);
  str_tlmode = dbe_strdup (_settings->str_tlmode);
  str_tldata = dbe_strdup (_settings->str_tldata);
  str_tabs = dbe_strdup (_settings->str_tabs);
  str_rtabs = dbe_strdup (_settings->str_rtabs);
  str_search_path = dbe_strdup (_settings->str_search_path);
  str_name_format = dbe_strdup (_settings->str_name_format);
  str_limit = dbe_strdup (_settings->str_limit);
  str_printmode = dbe_strdup (_settings->str_printmode);
  str_compare = dbe_strdup (_settings->str_compare);
  preload_libdirs = dbe_strdup (_settings->preload_libdirs);

  // replicate the pathmap vector
  pathmap_t *thismap;
  pathmap_t *newmap;
  pathmaps = new Vector<pathmap_t*>;

  Vec_loop (pathmap_t*, _settings->pathmaps, index, thismap)
  {
    newmap = new pathmap_t;
    newmap->old_prefix = dbe_strdup (thismap->old_prefix);
    newmap->new_prefix = dbe_strdup (thismap->new_prefix);
    pathmaps->append (newmap);
  }

  // replicate the lo_expand vector and default
  lo_expand_t *this_lo_ex;
  lo_expand_t *new_lo_ex;
  lo_expand_default = _settings->lo_expand_default;
  is_loexpand_default = _settings->is_loexpand_default;
  lo_expands = new Vector<lo_expand_t*>;

  Vec_loop (lo_expand_t*, _settings->lo_expands, index, this_lo_ex)
  {
    new_lo_ex = new lo_expand_t;
    new_lo_ex->libname = dbe_strdup (this_lo_ex->libname);
    new_lo_ex->expand = this_lo_ex->expand;
    lo_expands->append (new_lo_ex);
  }
  tabs_processed = _settings->tabs_processed;

  // Copy the various values from the _settings instance
  name_format = _settings->name_format;
  view_mode = _settings->view_mode;
  en_desc = false;
  en_desc_cmp = NULL;
  en_desc_usr = NULL;
  if (_settings->en_desc_usr)
    set_en_desc (_settings->en_desc_usr, true);
  src_compcom = _settings->src_compcom;
  dis_compcom = _settings->dis_compcom;
  threshold_src = _settings->threshold_src;
  threshold_dis = _settings->threshold_dis;
  src_visible = _settings->src_visible;
  srcmetric_visible = _settings->srcmetric_visible;
  hex_visible = _settings->hex_visible;
  cmpline_visible = _settings->cmpline_visible;
  funcline_visible = _settings->funcline_visible;
  tldata = dbe_strdup (_settings->tldata);
  tlmode = _settings->tlmode;
  stack_align = _settings->stack_align;
  stack_depth = _settings->stack_depth;
  limit = _settings->limit;
  print_mode = _settings->print_mode;
  print_delim = _settings->print_delim;
  compare_mode = _settings->compare_mode;
  machinemodel = dbe_strdup (_settings->machinemodel);
  ignore_no_xhwcprof = _settings->ignore_no_xhwcprof;
  ignore_fs_warn = _settings->ignore_fs_warn;

  // copy the tab list, too
  tab_list = new Vector<DispTab*>;
  DispTab *dsptab;

  Vec_loop (DispTab*, _settings->tab_list, index, dsptab)
  {
    DispTab *ntab;
    ntab = new DispTab (dsptab->type, dsptab->order, dsptab->visible, dsptab->cmdtoken);
    ntab->setAvailability (dsptab->available);
    tab_list->append (ntab);
  }

  // construct the master list of memory tabs & copy order
  index = _settings->mem_tab_state->size ();
  mem_tab_state = new Vector<bool>(index);
  mem_tab_order = new Vector<int>(index);
  for (int i = 0; i < index; i++)
    {
      mem_tab_state->append (false);
      mem_tab_order->append (_settings->mem_tab_order->fetch (i));
    }

  // construct the master list of index tabs & copy order
  index = _settings->indx_tab_state->size ();
  indx_tab_state = new Vector<bool>(index);
  indx_tab_order = new Vector<int>(index);
  for (int i = 0; i < index; i++)
    indx_tab_order->append (_settings->indx_tab_order->fetch (i));
  set_IndxTabState (_settings->indx_tab_state);
}

Settings::~Settings ()
{
  for (int i = 0; i < pathmaps->size (); ++i)
    {
      pathmap_t *pmap = pathmaps->fetch (i);
      free (pmap->old_prefix);
      free (pmap->new_prefix);
      delete pmap;
    }
  delete pathmaps;

  for (int i = 0; i < lo_expands->size (); ++i)
    {
      lo_expand_t *lo_ex = lo_expands->fetch (i);
      free (lo_ex->libname);
      delete lo_ex;
    }
  delete lo_expands;

  tab_list->destroy ();
  delete tab_list;
  delete indx_tab_state;
  delete indx_tab_order;
  delete mem_tab_state;
  delete mem_tab_order;

  free (str_vmode);
  free (str_en_desc);
  free (str_datamode);
  free (str_scompcom);
  free (str_sthresh);
  free (str_dcompcom);
  free (str_dthresh);
  free (str_dmetrics);
  free (str_dsort);
  free (str_tlmode);
  free (str_tldata);
  free (str_tabs);
  free (str_rtabs);
  free (str_search_path);
  free (str_name_format);
  free (str_limit);
  free (str_compare);
  free (str_printmode);
  free (preload_libdirs);
  free (tldata);
  free (en_desc_usr);
  if (en_desc_cmp)
    {
      regfree (en_desc_cmp);
      delete en_desc_cmp;
    }
}

/**
 * Read .er.rc file from the specified location
 * @param path
 * @return
 */
char *
Settings::read_rc (char *path)
{
  StringBuilder sb;
  Emsgqueue *commentq = new Emsgqueue (NTXT ("setting_commentq"));

  // Check file name
  if (NULL == path)
    return dbe_strdup (GTXT ("Error: empty file name"));
  bool override = true;
  set_rc (path, true, commentq, override);
  Emsg *msg = commentq->fetch ();
  while (msg != NULL)
    {
      char *str = msg->get_msg ();
      sb.append (str);
      msg = msg->next;
    }
  return sb.toString ();
}

void
Settings::read_rc (bool ipc_or_rdt_mode)
{
  bool override = false;

  // Read file from the current working directory
  char *rc_path = realpath (NTXT ("./.gprofng.rc"), NULL);
  if (rc_path)
    set_rc (rc_path, true, app->get_comments_queue (), override, ipc_or_rdt_mode);

  // Read file from the user's home directory
  char *home = getenv (NTXT ("HOME"));
  if (home)
    {
      char *strbuf = dbe_sprintf (NTXT ("%s/.gprofng.rc"), home);
      char *home_rc_path = realpath (strbuf, NULL);
      if (home_rc_path)
	{
	  if (rc_path == NULL || strcmp (rc_path, home_rc_path) != 0)
	    set_rc (home_rc_path, true, app->get_comments_queue (), override, ipc_or_rdt_mode);
	  free (home_rc_path);
	}
      free (strbuf);
    }
  free (rc_path);

  // Read system-wide file
  const char *sysconfdir = getenv("GPROFNG_SYSCONFDIR");
  if (sysconfdir == NULL)
    sysconfdir = SYSCONFDIR;
  rc_path = dbe_sprintf (NTXT ("%s/gprofng.rc"), sysconfdir);
  if (access (rc_path, R_OK | F_OK) != 0)
    {
      StringBuilder sb;
      sb.sprintf (GTXT ("Warning: Default gprofng.rc file (%s) missing; configuration error "), rc_path);
      Emsg *m = new Emsg (CMSG_COMMENT, sb);
      app->get_comments_queue ()->append (m);
    }
  else
    set_rc (rc_path, false, app->get_comments_queue (), override);
  free (rc_path);
  is_loexpand_default = true;
  if (str_printmode == NULL)
    {
      // only if there's none set
      print_mode = PM_TEXT;
      str_printmode = dbe_strdup (NTXT ("text"));
    }
}


//  Handle various settings from reading the name .rc file
//	This function is called for each .rc file read, and, for
//	some settings, it accumulates the strings from the files.
//	For others, it accepts the first appearance for a setting in a
//	.rc file, and ignores subsequent appearances from other files.
//  Error messages are appended to the Emsgqueue specified by the caller

#define MAXARGS 20

void
Settings::set_rc (const char *path, bool msg, Emsgqueue *commentq,
		  bool override, bool ipc_or_rdt_mode)
{
  CmdType cmd_type;
  int arg_count, cparam;
  char *cmd, *end_cmd, *strbuf;
  char *arglist[MAXARGS];
  StringBuilder sb;

  FILE *fptr = fopen (path, NTXT ("r"));
  if (fptr == NULL)
    return;

  if (msg)
    {
      sb.sprintf (GTXT ("Processed %s for default settings"), path);
      Emsg *m = new Emsg (CMSG_COMMENT, sb);
      commentq->append (m);
    }
  int line_no = 0;
  end_cmd = NULL;
  while (!feof (fptr))
    {
      char *script = read_line (fptr);
      if (script == NULL)
	continue;
      line_no++;
      strtok (script, NTXT ("\n"));

      // extract the command
      cmd = strtok (script, NTXT (" \t"));
      if (cmd == NULL || *cmd == '#' || *cmd == '\n')
	{
	  free (script);
	  continue;
	}
      char *remainder = strtok (NULL, NTXT ("\n"));
      // now extract the arguments
      int nargs = 0;
      for (;;)
	{
	  if (nargs >= MAXARGS)
	    {
	      if (!msg)
		{
		  msg = true; // suppress repeats of header
		  Emsg *m = new Emsg (CMSG_COMMENT, GTXT ("Processed system gprofng.rc file for default settings"));
		  commentq->append (m);
		}
	      sb.sprintf (GTXT ("Warning: more than %d arguments to %s command, line %d\n"),
			  MAXARGS, cmd, line_no);
	      Emsg *m = new Emsg (CMSG_COMMENT, sb);
	      commentq->append (m);
	      break;
	    }

	  char *nextarg = strtok (remainder, NTXT ("\n"));
	  if (nextarg == NULL || *nextarg == '#')
	    break;
	  arglist[nargs++] = parse_qstring (nextarg, &end_cmd);
	  remainder = end_cmd;
	  if (remainder == NULL)
	    break;
	  // skip any blanks or tabs to get to next argument
	  while (*remainder == ' ' || *remainder == '\t')
	    remainder++;
	}
      cmd_type = Command::get_command (cmd, arg_count, cparam);
      // check for extra arguments
      if ((cmd_type != UNKNOWN_CMD && cmd_type != INDXOBJDEF) && (nargs > arg_count))
	{
	  if (!msg)
	    {
	      msg = true; // suppress repeats of header
	      Emsg *m = new Emsg (CMSG_COMMENT, GTXT ("Processed system gprofng.rc file for default settings"));
	      commentq->append (m);
	    }
	  sb.sprintf (GTXT ("Warning: extra arguments to %s command, line %d\n"), cmd, line_no);
	  Emsg *m = new Emsg (CMSG_COMMENT, sb);
	  commentq->append (m);
	}
      if (nargs < arg_count)
	{
	  if (!msg)
	    {
	      msg = true; // suppress repeats of header
	      Emsg *m = new Emsg (CMSG_COMMENT, GTXT ("Processed system gprofng.rc file for default settings"));
	      commentq->append (m);
	    }
	  sb.sprintf (GTXT ("Error: missing arguments to %s command, line %d\n"),
		      cmd, line_no);
	  Emsg *m = new Emsg (CMSG_COMMENT, sb);
	  commentq->append (m);

	  // ignore this command
	  free (script);
	  continue;
	}
      if (ipc_or_rdt_mode && (cmd_type != ADDPATH) && (cmd_type != PATHMAP))
	{
	  free (script);
	  continue;
	}
      switch (cmd_type)
	{
	case SCOMPCOM:
	  if (!str_scompcom || override)
	    {
	      str_scompcom = dbe_strdup (arglist[0]);
	      proc_compcom (arglist[0], true, true);
	    }
	  break;
	case STHRESH:
	  if (!str_sthresh || override)
	    {
	      str_sthresh = dbe_strdup (arglist[0]);
	      proc_thresh (arglist[0], true, true);
	      break;
	    }
	  break;
	case DCOMPCOM:
	  if (!str_dcompcom || override)
	    {
	      str_dcompcom = dbe_strdup (arglist[0]);
	      proc_compcom (arglist[0], false, true);
	    }
	  break;
	case COMPCOM:
	  // process as if it were for both source and disassembly
	  //	note that if it is set, subsequent SCOMPCOM and DCOMPCOM
	  //	will be ignored
	  if (!str_scompcom || override)
	    {
	      str_scompcom = dbe_strdup (arglist[0]);
	      proc_compcom (arglist[0], true, true);
	    }
	  if (!str_dcompcom || override)
	    {
	      str_dcompcom = dbe_strdup (arglist[0]);
	      proc_compcom (arglist[0], false, true);
	    }
	  break;
	case DTHRESH:
	  if (!str_dthresh || override)
	    {
	      str_dthresh = dbe_strdup (arglist[0]);
	      proc_thresh (arglist[0], false, true);
	    }
	  break;
	case DMETRICS:
	  // append new settings to old, if necessary
	  if (str_dmetrics)
	    {
	      char *name = strstr (str_dmetrics, ":name");
	      if (name == NULL)
		strbuf = dbe_sprintf ("%s:%s", str_dmetrics, arglist[0]);
	      else
		{
		  char * next = strstr (name + 1, ":");
		  if (next == NULL)
		    {
		      name[0] = '\0';
		      strbuf = dbe_sprintf ("%s:%s:name", str_dmetrics, arglist[0]);
		    }
		  else
		    strbuf = dbe_sprintf ("%s:%s", str_dmetrics, arglist[0]);
		}
	      free (str_dmetrics);
	      str_dmetrics = strbuf;
	    }
	  else
	    str_dmetrics = dbe_strdup (arglist[0]);
	  break;
	case DSORT:
	  // append new settings to old, if necessary
	  if (str_dsort)
	    {
	      strbuf = dbe_sprintf (NTXT ("%s:%s"), str_dsort, arglist[0]);
	      free (str_dsort);
	      str_dsort = strbuf;
	    }
	  else
	    str_dsort = dbe_strdup (arglist[0]);
	  break;
	case TLMODE:
	  if (!str_tlmode || override)
	    {
	      str_tlmode = dbe_strdup (arglist[0]);
	      proc_tlmode (arglist[0], true);
	    }
	  break;
	case TLDATA:
	  if (!str_tldata || override)
	    {
	      str_tldata = dbe_strdup (arglist[0]);
	      proc_tldata (arglist[0], true);
	    }
	  break;
	case TABS:
	  if (!str_tabs || override)
	    // the string is processed later, after all .rc files are read
	    str_tabs = dbe_strdup (arglist[0]);
	  break;
	case RTABS:
	  if (!str_rtabs || override)
	    // the string is processed later, after all .rc files are read
	    str_rtabs = dbe_strdup (arglist[0]);
	  break;
	case ADDPATH:
	  if (str_search_path)
	    {
	      strbuf = dbe_sprintf (NTXT ("%s:%s"), str_search_path, arglist[0]);
	      free (str_search_path);
	      str_search_path = strbuf;
	    }
	  else
	    str_search_path = dbe_strdup (arglist[0]);
	  break;
	case PATHMAP:
	  {
	    char *err = add_pathmap (pathmaps, arglist[0], arglist[1]);
	    free (err);     // XXX error is not reported
	    break;
	  }
	case LIBDIRS:
	  if (preload_libdirs == NULL)
	    preload_libdirs = dbe_strdup (arglist[0]);
	  break;
	case NAMEFMT:
	  if (name_format == Histable::NA)
	    set_name_format (arglist[0]);
	  break;
	case VIEWMODE:
	  if (!str_vmode || override)
	    {
	      str_vmode = dbe_strdup (arglist[0]);
	      set_view_mode (arglist[0], true);
	    }
	  break;
	case EN_DESC:
	  if (!str_en_desc || override)
	    {
	      str_en_desc = dbe_strdup (arglist[0]);
	      set_en_desc (arglist[0], true);
	    }
	  break;
	case LIMIT:
	  if (!str_limit || override)
	    {
	      str_limit = dbe_strdup (arglist[0]);
	      set_limit (arglist[0], true);
	    }
	  break;
	case PRINTMODE:
	  if (!str_printmode || override)
	    set_printmode (arglist[0]);
	  break;
	case COMPARE:
	  if (!str_compare || override)
	    {
	      char *s = arglist[0];
	      if (s)
		str_compare = dbe_strdup (s);
	      else
		s = NTXT ("");
	      if (strcasecmp (s, NTXT ("OFF")) == 0
		  || strcmp (s, NTXT ("0")) == 0)
		set_compare_mode (CMP_DISABLE);
	      else if (strcasecmp (s, NTXT ("ON")) == 0
		       || strcmp (s, NTXT ("1")) == 0)
		set_compare_mode (CMP_ENABLE);
	      else if (strcasecmp (s, NTXT ("DELTA")) == 0)
		set_compare_mode (CMP_DELTA);
	      else if (strcasecmp (s, NTXT ("RATIO")) == 0)
		set_compare_mode (CMP_RATIO);
	      else
		{
		  sb.sprintf (GTXT ("   .er.rc:%d The argument of 'compare' should be 'on', 'off', 'delta', or 'ratio'"),
			      (int) line_no);
		  Emsg *m = new Emsg (CMSG_COMMENT, sb);
		  commentq->append (m);
		}
	    }
	  break;

	case INDXOBJDEF:
	  {
	    char *ret = dbeSession->indxobj_define (arglist[0], NULL, arglist[1], (nargs >= 3) ? PTXT (arglist[2]) : NULL, (nargs >= 4) ? PTXT (arglist[3]) : NULL);
	    if (ret != NULL)
	      {
		sb.sprintf (GTXT ("   %s: line %d `%s %s %s'\n"),
			    ret, line_no, cmd, arglist[0], arglist[1]);
		Emsg *m = new Emsg (CMSG_COMMENT, sb);
		commentq->append (m);
	      }
	    break;
	  }
#ifdef sparc
	  //XXX: should be conditional on the experiment ARCH, not dbe ARCH
	case IGNORE_NO_XHWCPROF:
	  // ignore absence of -xhwcprof info for dataspace profiling
	  set_ignore_no_xhwcprof (true);
	  break;
#endif // sparc
	case IGNORE_FS_WARN:
	  // ignore file system warning in experiments
	  set_ignore_fs_warn (true);
	  break;
	case OBJECT_SHOW:
	  // Add the named libraries to the lib_expands array
	  set_libexpand (arglist[0], LIBEX_SHOW, true);
	  break;
	case OBJECT_HIDE:
	  // Add the named libraries to the lib_expands array
	  set_libexpand (arglist[0], LIBEX_HIDE, true);
	  break;
	case OBJECT_API:
	  // Add the named libraries to the lib_expands array
	  set_libexpand (arglist[0], LIBEX_API, true);
	  break;
	case COMMENT:
	  // ignore the line
	  break;
	default:
	  {
	    // unexpected command in an rc file
	    if (!msg)
	      {
		// if quiet, can remain so no longer
		msg = true;
		Emsg *m = new Emsg (CMSG_COMMENT, GTXT ("Processed system gprofng.rc file for default settings"));
		commentq->append (m);
	      }
	    sb.sprintf (GTXT ("   Unrecognized .gprofng.rc command on line %d: `%.64s'"),
			line_no, cmd);
	    Emsg *m = new Emsg (CMSG_COMMENT, sb);
	    commentq->append (m);
	    break;
	  }
	}
      free (script);
    }
  fclose (fptr);
}

Cmd_status
Settings::set_view_mode (char *arg, bool rc)
{
  if (!strcasecmp (arg, NTXT ("user")))
    view_mode = VMODE_USER;
  else if (!strcasecmp (arg, NTXT ("expert")))
    view_mode = VMODE_EXPERT;
  else if (!strcasecmp (arg, NTXT ("machine")))
    view_mode = VMODE_MACHINE;
  else if (!rc)
    return CMD_BAD_ARG;
  return CMD_OK;
}

Cmd_status
Settings::set_en_desc (char *arg, bool rc)
{
  regex_t *regex_desc = NULL;

  // cases below should be similar to Coll_Ctrl::set_follow_mode() cases
  if (!strcasecmp (arg, NTXT ("on")))
    en_desc = true;
  else if (!strcasecmp (arg, NTXT ("off")))
    en_desc = false;
  else if (arg[0] == '=' && arg[1] != 0)
    {
      // user has specified a string matching specification
      int ercode;
      { // compile regex_desc
	char * str = dbe_sprintf (NTXT ("^%s$"), arg + 1);
	regex_desc = new regex_t;
	memset (regex_desc, 0, sizeof (regex_t));
	ercode = regcomp (regex_desc, str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
	free (str);
      }
      if (ercode)
	{
	  // syntax error in parsing string
	  delete regex_desc;
	  if (!rc)
	    return CMD_BAD_ARG;
	  return CMD_OK;
	}
      en_desc = true;
    }
  else
    {
      if (!rc)
	return CMD_BAD_ARG;
      return CMD_OK;
    }
  free (en_desc_usr);
  en_desc_usr = dbe_strdup (arg);
  if (en_desc_cmp)
    {
      regfree (en_desc_cmp);
      delete en_desc_cmp;
    }
  en_desc_cmp = regex_desc;
  return CMD_OK;
}

// See if a descendant matches either the lineage or the executable name
bool
Settings::check_en_desc (const char *lineage, const char *targname)
{
  bool rc;
  if (en_desc_cmp == NULL)
    return en_desc;     // no specification was set, use the binary on/off value
  if (lineage == NULL)  // user doesn't care about specification
    return en_desc;     // use the binary on/off specification
  if (!regexec (en_desc_cmp, lineage, 0, NULL, 0))
    rc = true;          // this one matches user specification
  else if (targname == NULL)
    rc = false;         //a NULL name does not match any expression
  else if (!regexec (en_desc_cmp, targname, 0, NULL, 0))
    rc = true;          // this one matches the executable name
  else
    rc = false;
  return rc;
}

char *
Settings::set_limit (char *arg, bool)
{
  limit = (int) strtol (arg, (char **) NULL, 10);
  return NULL;
}

char *
Settings::set_printmode (char *arg)
{
  if (arg == NULL)
    return dbe_sprintf (GTXT ("The argument to '%s' must be '%s' or '%s' or a single-character"),
			NTXT ("printmode"), NTXT ("text"), NTXT ("html"));
  if (strlen (arg) == 1)
    {
      print_mode = PM_DELIM_SEP_LIST;
      print_delim = arg[0];
    }
  else if (!strcasecmp (arg, NTXT ("text")))
    print_mode = PM_TEXT;
  else if (!strcasecmp (arg, NTXT ("html")))
    print_mode = PM_HTML;
  else
    return dbe_sprintf (GTXT ("The argument to '%s' must be '%s' or '%s' or a single-character"),
			NTXT ("printmode"), NTXT ("text"), NTXT ("html"));
  free (str_printmode);
  str_printmode = dbe_strdup (arg);
  return NULL;
}

Cmd_status
Settings::proc_compcom (const char *cmd, bool isSrc, bool rc)
{
  int ck_compcom_bits, ck_threshold;
  bool ck_hex_visible = false;
  bool ck_src_visible = false;
  bool ck_srcmetric_visible = false;
  bool got_compcom_bits, got_threshold, got_src_visible, got_srcmetric_visible;
  bool got_hex_visible, got;
  int len, i;
  char *mcmd, *param;
  int flag, value = 0;
  Cmd_status status;
  char buf[BUFSIZ], *list;

  if (cmd == NULL)
    return CMD_BAD;
  ck_compcom_bits = 0;
  ck_threshold = 0;
  got_compcom_bits = got_threshold = got_src_visible = false;
  got_srcmetric_visible = got_hex_visible = false;
  snprintf (buf, sizeof (buf), NTXT ("%s"), cmd);
  list = buf;
  while ((mcmd = strtok (list, NTXT (":"))) != NULL)
    {
      list = NULL;
      // if "all" or "none"
      if (!strcasecmp (mcmd, Command::ALL_CMD))
	{
	  got_compcom_bits = true;
	  ck_compcom_bits = CCMV_ALL;
	  continue;
	}
      else if (!strcasecmp (mcmd, Command::NONE_CMD))
	{
	  got_compcom_bits = true;
	  ck_compcom_bits = 0;
	  continue;
	}

      // Find parameter after '='
      param = strchr (mcmd, '=');
      if (param)
	{
	  *param = '\0';
	  param++;
	}
      status = CMD_OK;
      got = false;
      flag = 0;
      len = (int) strlen (mcmd);
      for (i = 0; status == CMD_OK && i < comp_size; i++)
	if (!strncasecmp (mcmd, comp_cmd[i], len))
	  {
	    if (got) // Ambiguous comp_com command
	      status = CMD_AMBIGUOUS;
	    else
	      {
		got = true;
		flag = comp_vis[i];
		// Check argument
		if (flag == COMP_THRESHOLD)
		  {
		    if (param == NULL)
		      status = CMD_BAD_ARG;
		    else
		      {
			value = (int) strtol (param, &param, 10);
			if (value < 0 || value > 100)
			  status = CMD_OUTRANGE;
		      }
		  }
		else if (param != NULL)
		  status = CMD_BAD_ARG;
	      }
	  }

      // Not valid comp_com command
      if (!got)
	status = CMD_INVALID;
      if (status != CMD_OK)
	{
	  if (!rc)
	    return status;
	  continue;
	}

      // Set bits
      switch (flag)
	{
	case COMP_CMPLINE:
	  cmpline_visible = true;
	  break;
	case COMP_FUNCLINE:
	  funcline_visible = true;
	  break;
	case COMP_THRESHOLD:
	  got_threshold = true;
	  ck_threshold = value;
	  break;
	case COMP_SRC:
	  got_src_visible = true;
	  ck_src_visible = true;
	  break;
	case COMP_SRC_METRIC:
	  got_srcmetric_visible = true;
	  ck_srcmetric_visible = true;
	  got_src_visible = true;
	  ck_src_visible = true;
	  break;
	case COMP_NOSRC:
	  got_src_visible = true;
	  ck_src_visible = false;
	  break;
	case COMP_HEX:
	  got_hex_visible = true;
	  ck_hex_visible = true;
	  break;
	case COMP_NOHEX:
	  got_hex_visible = true;
	  ck_hex_visible = false;
	  break;
	case CCMV_BASIC:
	  got_compcom_bits = true;
	  ck_compcom_bits = CCMV_BASIC;
	  break;
	default:
	  got_compcom_bits = true;
	  ck_compcom_bits |= flag;
	}
    }

  // No error, update
  if (got_compcom_bits)
    {
      if (isSrc)
	src_compcom = ck_compcom_bits;
      else
	dis_compcom = ck_compcom_bits;
    }
  if (got_threshold)
    {
      if (isSrc)
	threshold_src = ck_threshold;
      else
	threshold_dis = ck_threshold;
    }
  if (got_src_visible)
      src_visible = ck_src_visible;
  if (got_srcmetric_visible)
      srcmetric_visible = ck_srcmetric_visible;
  if (got_hex_visible)
      hex_visible = ck_hex_visible;
  return CMD_OK;
}

// Process a threshold setting
Cmd_status
Settings::proc_thresh (char *cmd, bool isSrc, bool rc)
{
  int value;
  if (cmd == NULL)
    value = DEFAULT_SRC_DIS_THRESHOLD; // the default
  else
    value = (int) strtol (cmd, &cmd, 10);
  if (value < 0 || value > 100)
    {
      if (!rc)
	return CMD_OUTRANGE;
      value = DEFAULT_SRC_DIS_THRESHOLD;
    }
  if (isSrc)
    threshold_src = value;
  else
    threshold_dis = value;
  return CMD_OK;
}

// return any error string from processing visibility settings
char *
Settings::get_compcom_errstr (Cmd_status status, const char *cmd)
{
  int i;
  StringBuilder sb;
  switch (status)
    {
    case CMD_BAD:
      sb.append (GTXT ("No commentary classes has been specified."));
      break;
    case CMD_AMBIGUOUS:
      sb.append (GTXT ("Ambiguous commentary classes: "));
      break;
    case CMD_BAD_ARG:
      sb.append (GTXT ("Invalid argument for commentary classes: "));
      break;
    case CMD_OUTRANGE:
      sb.append (GTXT ("Out of range commentary classes argument: "));
      break;
    case CMD_INVALID:
      sb.append (GTXT ("Invalid commentary classes: "));
      break;
    case CMD_OK:
      break;
    }
  if (cmd)
    sb.append (cmd);
  sb.append (GTXT ("\nAvailable commentary classes: "));
  for (i = 0; i < comp_size; i++)
    {
      sb.append (comp_cmd[i]);
      if (i == comp_size - 1)
	sb.append (NTXT ("=#\n"));
      else
	sb.append (NTXT (":"));
    }
  return sb.toString ();
}

// Process a timeline-mode setting
Cmd_status
Settings::proc_tlmode (char *cmd, bool rc)
{
  bool got_tlmode, got_stack_align, got_stack_depth, got;
  int ck_tlmode = 0, ck_stack_align = 0, ck_stack_depth = 0;
  int len, i;
  char *mcmd, *param;
  int cmd_id, value = 0;
  TLModeSubcommand cmd_type;
  Cmd_status status;
  char buf[BUFSIZ], *list;
  if (cmd == NULL)
    return CMD_BAD;
  got_tlmode = got_stack_align = got_stack_depth = false;
  snprintf (buf, sizeof (buf), NTXT ("%s"), cmd);
  list = buf;
  while ((mcmd = strtok (list, NTXT (":"))) != NULL)
    {
      list = NULL;

      // Find parameter after '='
      param = strchr (mcmd, '=');
      if (param)
	{
	  *param = '\0';
	  param++;
	}
      status = CMD_OK;
      got = false;
      cmd_id = 0;
      cmd_type = TLCMD_INVALID;
      len = (int) strlen (mcmd);
      for (i = 0; status == CMD_OK && i < tlmode_size; i++)
	{
	  if (!strncasecmp (mcmd, tlmode_cmd[i].cmdText, len))
	    {
	      if (got) // Ambiguous timeline mode
		status = CMD_AMBIGUOUS;
	      else
		{
		  got = true;
		  cmd_type = tlmode_cmd[i].cmdType;
		  cmd_id = tlmode_cmd[i].cmdId;

		  // Check argument
		  if (cmd_type == TLCMD_DEPTH)
		    {
		      if (param == NULL)
			status = CMD_BAD_ARG;
		      else
			{
			  value = (int) strtol (param, &param, 10);
			  if (value <= 0 || value > 256)
			    status = CMD_OUTRANGE;
			}
		    }
		  else if (param != NULL)
		    status = CMD_BAD_ARG;
		}
	    }
	}

      // Not valid timeline mode
      if (!got)
	status = CMD_INVALID;
      if (status != CMD_OK)
	{
	  if (!rc)
	    return status;
	  continue;
	}

      // Set bits
      switch (cmd_type)
	{
	case TLCMD_ENTITY_MODE:
	  got_tlmode = true;
	  ck_tlmode = cmd_id;
	  break;
	case TLCMD_ALIGN:
	  got_stack_align = true;
	  ck_stack_align = cmd_id;
	  break;
	case TLCMD_DEPTH:
	  got_stack_depth = true;
	  ck_stack_depth = value;
	  break;
	default:
	  break;
	}
    }

  // No error, update
  if (got_tlmode)
    tlmode = ck_tlmode;
  if (got_stack_align)
    stack_align = ck_stack_align;
  if (got_stack_depth)
    stack_depth = ck_stack_depth;
  return CMD_OK;
}

// Process timeline data specification
Cmd_status
Settings::proc_tldata (const char *cmd, bool /* if true, ignore any error */)
{
  free (tldata);
  tldata = dbe_strdup (cmd); // let GUI parse it
  return CMD_OK;
}

void
Settings::set_tldata (const char* _tldata_str)
{
  free (tldata);
  tldata = dbe_strdup (_tldata_str);
}

char*
Settings::get_tldata ()
{
  return dbe_strdup (tldata);
}

Cmd_status
Settings::set_name_format (char *arg)
{
  char *colon = strchr (arg, ':');
  size_t arg_len = (colon) ? (colon - arg) : strlen (arg);
  Histable::NameFormat fname_fmt = Histable::NA;
  if (!strncasecmp (arg, NTXT ("long"), arg_len))
    fname_fmt = Histable::LONG;
  else if (!strncasecmp (arg, NTXT ("short"), arg_len))
    fname_fmt = Histable::SHORT;
  else if (!strncasecmp (arg, NTXT ("mangled"), arg_len))
    fname_fmt = Histable::MANGLED;
  else
    return CMD_BAD_ARG;

  bool soname_fmt = false;
  if (colon)
    {
      colon++;
      if (!strcasecmp (colon, NTXT ("soname")))
	soname_fmt = true;
      else if (!strcasecmp (colon, NTXT ("nosoname")))
	soname_fmt = false;
      else
	return CMD_BAD_ARG;
    }
  name_format = Histable::make_fmt (fname_fmt, soname_fmt);
  return CMD_OK;
}

void
Settings::buildMasterTabList ()
{
  tab_list = new Vector<DispTab*>;
  int i = -1;

  // Add tabs for all the known reports
  tab_list->append (new DispTab (DSP_DEADLOCKS, i, false, DEADLOCK_EVNTS));
  tab_list->append (new DispTab (DSP_FUNCTION, i, false, FUNCS));
  tab_list->append (new DispTab (DSP_TIMELINE, i, false, TIMELINE));
  tab_list->append (new DispTab (DSP_CALLTREE, i, false, CALLTREE));
  tab_list->append (new DispTab (DSP_CALLFLAME, i, false, CALLFLAME));
  tab_list->append (new DispTab (DSP_DUALSOURCE, i, false, DUALSOURCE));
  tab_list->append (new DispTab (DSP_SOURCE_DISASM, i, false, SOURCEDISAM));
  tab_list->append (new DispTab (DSP_SOURCE, i, false, SOURCE));
  tab_list->append (new DispTab (DSP_LINE, i, false, HOTLINES));
  tab_list->append (new DispTab (DSP_DISASM, i, false, DISASM));
  tab_list->append (new DispTab (DSP_PC, i, false, HOTPCS));
  tab_list->append (new DispTab (DSP_LEAKLIST, i, false, LEAKS));
  tab_list->append (new DispTab (DSP_IOACTIVITY, i, false, IOACTIVITY));
  tab_list->append (new DispTab (DSP_HEAPCALLSTACK, i, false, HEAP));
  tab_list->append (new DispTab (DSP_IFREQ, i, false, IFREQ));
  tab_list->append (new DispTab (DSP_CALLER, i, false, GPROF));
  tab_list->append (new DispTab (DSP_STATIS, i, false, STATISTICS));
  tab_list->append (new DispTab (DSP_EXP, i, false, HEADER));
}

// Update tablist based on data availability
void
Settings::updateTabAvailability ()
{
  int index;
  DispTab *dsptab;

  Vec_loop (DispTab*, tab_list, index, dsptab)
  {
    if (dsptab->type == DSP_DATAOBJ)
      dsptab->setAvailability (dbeSession->is_datamode_available ());
    else if (dsptab->type == DSP_DLAYOUT)
      dsptab->setAvailability (dbeSession->is_datamode_available ());
    else if (dsptab->type == DSP_LEAKLIST)
      dsptab->setAvailability (false);
    else if (dsptab->type == DSP_IOACTIVITY)
      dsptab->setAvailability (dbeSession->is_iodata_available ());
    else if (dsptab->type == DSP_HEAPCALLSTACK)
      dsptab->setAvailability (dbeSession->is_heapdata_available ());
    else if (dsptab->type == DSP_TIMELINE)
      dsptab->setAvailability (dbeSession->is_timeline_available ());
    else if (dsptab->type == DSP_IFREQ)
      dsptab->setAvailability (dbeSession->is_ifreq_available ());
    else if (dsptab->type == DSP_RACES)
      dsptab->setAvailability (dbeSession->is_racelist_available ());
    else if (dsptab->type == DSP_DEADLOCKS)
      dsptab->setAvailability (dbeSession->is_deadlocklist_available ());
    else if (dsptab->type == DSP_DUALSOURCE)
      dsptab->setAvailability (dbeSession->is_racelist_available ()
			       || dbeSession->is_deadlocklist_available ());
  }
}

// Process a tab setting
Cmd_status
Settings::proc_tabs (bool _rdtMode)
{
  int arg_cnt, cparam;
  int count = 0;
  int index;
  DispTab *dsptab;
  char *cmd;
  if (tabs_processed == true)
    return CMD_OK;
  tabs_processed = true;
  if (_rdtMode == true)
    {
      if (str_rtabs == NULL)
	str_rtabs = strdup ("header");
      cmd = str_rtabs;
    }
  else
    {
      if (str_tabs == NULL)
	str_tabs = strdup ("header");
      cmd = str_tabs;
    }
  if (strcmp (cmd, NTXT ("none")) == 0)
    return CMD_OK;
  Vector <char *> *tokens = split_str (cmd, ':');
  for (long j = 0, sz = VecSize (tokens); j < sz; j++)
    {
      char *tabname = tokens->get (j);
      // search for this tab command token
      CmdType c = Command::get_command (tabname, arg_cnt, cparam);
      if (c == INDXOBJ)
	{
	  // set the bit for this subtype
	  indx_tab_state->store (cparam, true);
	  indx_tab_order->store (cparam, count++);
	}
      else
	{
	  // search for this tab type in the regular tabs
	  Vec_loop (DispTab*, tab_list, index, dsptab)
	  {
	    if (dsptab->cmdtoken == c)
	      {
		dsptab->visible = true;
		dsptab->order = count++;
		break;
	      }
	  }
	}
      free (tabname);
    }
  delete tokens;
  return CMD_OK;
}

void
Settings::set_MemTabState (Vector<bool>*selected)
{
  if (selected->size () == 0)
    return;
  for (int j = 0; j < mem_tab_state->size (); j++)
    mem_tab_state->store (j, selected->fetch (j));
}

// define a new memory object type

void
Settings::mobj_define (MemObjType_t */* mobj */, bool state)
{
  if (mem_tab_state->size () == 0)
    state = true;
  mem_tab_state->append (state);
  mem_tab_order->append (-1);
}

void
Settings::set_IndxTabState (Vector<bool>*selected)
{
  for (int j = 0; j < selected->size (); j++)
    indx_tab_state->store (j, selected->fetch (j));
}

// define a new index object type
void
Settings::indxobj_define (int type, bool state)
{
  indx_tab_state->store (type, state);
  indx_tab_order->store (type, -1);
}

void
Settings::set_pathmaps (Vector<pathmap_t*> *newPathMap)
{
  if (pathmaps)
    {
      pathmaps->destroy ();
      delete pathmaps;
    }
  pathmaps = newPathMap;
}

static char *
get_canonical_name (const char *fname)
{
  char *nm = dbe_strdup (fname);
  for (size_t len = strlen (nm); (len > 0) && (nm[len - 1] == '/'); len--)
    nm[len - 1] = 0;
  return nm;
}

char *
Settings::add_pathmap (Vector<pathmap_t*> *v, const char *from, const char *to)
{
  // Check for errors
  if (from == NULL || to == NULL)
    return dbe_strdup (GTXT ("Pathmap can have neither from nor to as NULL\n"));
  if (strcmp (from, to) == 0)
    return dbe_strdup (GTXT ("Pathmap from must differ from to\n"));
  char *old_prefix = get_canonical_name (from);
  char *new_prefix = get_canonical_name (to);

  // Check the pathmap list
  for (int i = 0, sz = v->size (); i < sz; i++)
    {
      pathmap_t *pmp = v->get (i);
      if ((strcmp (pmp->old_prefix, old_prefix) == 0) &&(strcmp (pmp->new_prefix, new_prefix) == 0))
	{
	  char *s = dbe_sprintf (GTXT ("Pathmap from `%s' to `%s' already exists\n"), old_prefix, new_prefix);
	  free (old_prefix);
	  free (new_prefix);
	  return s;
	}
    }
  // construct a map for this pair
  pathmap_t *thismap = new pathmap_t;
  thismap->old_prefix = old_prefix;
  thismap->new_prefix = new_prefix;
  v->append (thismap);
  return NULL;
}

// Set all shared object expands back to .rc file defaults,
//	as stored in the DbeSession Settings
bool
Settings::set_libdefaults ()
{
  // See if this is unchanged
  if (is_loexpand_default == true)
    return false; // no change

  // replicate the DbeSession's lo_expand vector and default settings
  lo_expand_t *this_lo_ex;
  lo_expand_t *new_lo_ex;
  int index;
  lo_expand_default = dbeSession->get_settings ()->lo_expand_default;
  lo_expands = new Vector<lo_expand_t*>;
  Vec_loop (lo_expand_t*, dbeSession->get_settings ()->lo_expands, index, this_lo_ex)
  {
    new_lo_ex = new lo_expand_t;
    new_lo_ex->libname = dbe_strdup (this_lo_ex->libname);
    new_lo_ex->expand = this_lo_ex->expand;
    lo_expands->append (new_lo_ex);
  }
  is_loexpand_default = true;
  return true;
}

bool
Settings::set_libexpand (char *cov, enum LibExpand expand, bool rc)
{
  int index;
  lo_expand_t *loe;
  bool change = false;
  if (cov == NULL || !strcasecmp (cov, Command::ALL_CMD))
    { // set all libraries
      // set the default
      if (lo_expand_default != expand)
	{
	  lo_expand_default = expand;
	  change = true;
	  is_loexpand_default = false;
	}

      // and force any explicit settings to match, too
      Vec_loop (lo_expand_t*, lo_expands, index, loe)
      {
	if (loe->expand != expand)
	  {
	    loe->expand = expand;
	    change = true;
	    is_loexpand_default = false;
	  }
      }

    }
  else
    { // parsing coverage
      Vector <char *> *tokens = split_str (cov, ',');
      for (long j = 0, sz = VecSize (tokens); j < sz; j++)
	{
	  char *lo_name = tokens->get (j);
	  char *newname = get_basename (lo_name);
	  bool found = false;
	  Vec_loop (lo_expand_t*, lo_expands, index, loe)
	  {
	    if (strcmp (loe->libname, newname) == 0)
	      {
		if (loe->expand != expand)
		  {
		    if (rc == false)
		      {
			loe->expand = expand;
			change = true;
			is_loexpand_default = false;
		      }
		  }
		found = true;
		break;
	      }
	  }

	  if (found == false)
	    {
	      // construct a map for this pair
	      lo_expand_t *thisloe;
	      thisloe = new lo_expand_t;
	      thisloe->libname = dbe_strdup (newname);
	      thisloe->expand = expand;
	      change = true;
	      is_loexpand_default = false;

	      // add it to the vector
	      lo_expands->append (thisloe);
	    }
	  free (lo_name);
	}
      delete tokens;
    }
  return change;
}

enum LibExpand
Settings::get_lo_setting (char *name)
{
  int index;
  lo_expand_t *loe;
  char *lo_name = get_basename (name);
  Vec_loop (lo_expand_t*, lo_expands, index, loe)
  {
    if (strcmp (loe->libname, lo_name) == 0)
      return loe->expand;
  }
  return lo_expand_default;
}
