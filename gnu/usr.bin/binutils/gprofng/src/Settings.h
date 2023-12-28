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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <stdio.h>
#include <regex.h>

#include "gp-defs.h"
#include "Histable.h"
#include "MemorySpace.h"
#include "Metric.h"
#include "dbe_types.h"
#include "dbe_structs.h"
#include "enums.h"
#include "vec.h"

class Emsgqueue;
class Application;

struct DispTab;

// Settings object

class Settings
{
public:
  friend class DbeView;
  friend class DbeSession;

  Settings (Application *_app);
  Settings (Settings *_settings);
  virtual ~Settings ();
  void read_rc (bool ipc_or_rdt_mode);  // read all rc files
  char *read_rc (char *path);           // read rc file
  void buildMasterTabList ();       // build list of Tabs that can be invoked
  void updateTabAvailability ();    // update for datamode, leaklist
  Cmd_status set_name_format (char *str); // from a string

  Vector<DispTab*> *
  get_TabList ()        // Get the list of tabs for this view
  {
    return tab_list;
  }

  Vector<bool> *
  get_MemTabState ()    // Get the list and order of memory tabs for this view
  {
    return mem_tab_state;
  }

  Vector<int> *
  get_MemTabOrder ()
  {
    return mem_tab_order;
  }

  // Set the list of memory tabs for this view
  void set_MemTabState (Vector<bool>*sel);

  // add a newly-defined memory object tab
  void mobj_define (MemObjType_t *, bool state);

  // add a newly-defined index object tab
  void indxobj_define (int type, bool state);

  Vector<bool> *
  get_IndxTabState ()   // Get the list and order of index tabs for this view
  {
    return indx_tab_state;
  }

  Vector<int> *
  get_IndxTabOrder ()
  {
    return indx_tab_order;
  }

  // Set the list of index tabs for this view
  void set_IndxTabState (Vector<bool>*sel);

  void
  set_name_format (int fname_fmt, bool soname_fmt)
  {
    name_format = Histable::make_fmt (fname_fmt, soname_fmt);
  }

  Histable::NameFormat
  get_name_format ()
  {
    return name_format;
  }

  // public methods for setting and accessing the settings
  Cmd_status set_view_mode (char *str, bool rc); // from a string

  void
  set_view_mode (VMode mode)
  {
    view_mode = mode;
  }

  VMode
  get_view_mode ()
  {
    return view_mode;
  }

  // set the en_desc expression/on/off
  Cmd_status set_en_desc (char *str, bool rc); // from a string
  // check if the lineage or the target name matches the en_desc expression
  bool check_en_desc (const char *lineage, const char *targname);

  char *set_limit (char *str, bool rc); // from a string

  char *
  set_limit (int _limit)
  {
    limit = _limit;
    return NULL;
  }

  int
  get_limit ()
  {
    return limit;
  }

  char *set_printmode (char *_pmode);

  // processing compiler commentary visibility bits
  Cmd_status proc_compcom (const char *cmd, bool isSrc, bool rc);

  // return any error string from processing visibility settings
  char *get_compcom_errstr (Cmd_status status, const char *cmd);

  // methods for setting and getting strings, and individual settings

  char *
  get_str_scompcom ()
  {
    return str_scompcom;
  }

  char *
  get_str_dcompcom ()
  {
    return str_dcompcom;
  }

  int
  get_src_compcom ()
  {
    return src_compcom;
  }

  int
  get_dis_compcom ()
  {
    return dis_compcom;
  }

  void
  set_cmpline_visible (bool v)
  {
    cmpline_visible = v;
  }

  void
  set_funcline_visible (bool v)
  {
    funcline_visible = v;
  }

  void
  set_src_visible (int v)
  {
    src_visible = v;
  }

  int
  get_src_visible ()
  {
    return src_visible;
  }

  void
  set_srcmetric_visible (bool v)
  {
    srcmetric_visible = v;
  }

  bool
  get_srcmetric_visible ()
  {
    return srcmetric_visible;
  }

  void
  set_hex_visible (bool v)
  {
    hex_visible = v;
  }

  bool
  get_hex_visible ()
  {
    return hex_visible;
  }

  // processing and accessing the threshold settings
  Cmd_status proc_thresh (char *cmd, bool isSrc, bool rc);

  int
  get_thresh_src ()
  {
    return threshold_src;
  }

  int
  get_thresh_dis ()
  {
    return threshold_dis;
  }

  // process a tlmode setting
  Cmd_status proc_tlmode (char *cmd, bool rc);

  void
  set_tlmode (int _tlmode)
  {
    tlmode = _tlmode;
  }

  int
  get_tlmode ()
  {
    return tlmode;
  }

  void
  set_stack_align (int _stack_align)
  {
    stack_align = _stack_align;
  }

  int
  get_stack_align ()
  {
    return stack_align;
  }

  void
  set_stack_depth (int _stack_depth)
  {
    stack_depth = _stack_depth;
  }

  int
  get_stack_depth ()
  {
    return stack_depth;
  }

  // process a tabs setting: called when the tab list is requested
  Cmd_status proc_tabs (bool _rdtMode);

  Cmd_status proc_tldata (const char *cmd, bool rc); // process a tldata setting
  void set_tldata (const char* tldata_string);
  char *get_tldata ();

  char *
  get_default_metrics ()
  {
    return str_dmetrics;
  }

  char *
  get_default_sort ()
  {
    return str_dsort;
  }

  void
  set_ignore_no_xhwcprof (bool v)   // ignore no xhwcprof errors for dataspace
  {
    ignore_no_xhwcprof = v;
  }

  bool
  get_ignore_no_xhwcprof ()
  {
    return ignore_no_xhwcprof;
  }

  void
  set_ignore_fs_warn (bool v)   // ignore filesystem warnings in experiments
  {
    ignore_fs_warn = v;
  }

  bool
  get_ignore_fs_warn ()
  {
    return ignore_fs_warn;
  }

  // add a pathmap
  static char *add_pathmap (Vector<pathmap_t*> *v, const char *from, const char *to);
  void set_pathmaps (Vector<pathmap_t*> *newPathMap);

  // add a LoadObject expansion setting
  bool set_libexpand (char *, enum LibExpand, bool);
  enum LibExpand get_lo_setting (char *);

  // set LoadObject expansion defaults back to .rc specifications
  bool set_libdefaults ();

  void
  set_compare_mode (int mode)
  {
    compare_mode = mode;
  }

  int
  get_compare_mode ()
  {
    return compare_mode;
  }

  char *
  get_machinemodel ()
  {
    return dbe_strdup (machinemodel);
  }

  char *preload_libdirs;

protected: // data
  Application *app;

  // default strings from .rc file
  char *str_vmode;
  char *str_en_desc;
  char *str_datamode;
  char *str_scompcom;
  char *str_sthresh;
  char *str_dcompcom;
  char *str_dthresh;
  char *str_dmetrics;
  char *str_dsort;
  char *str_tlmode;
  char *str_tldata;
  char *str_tabs;
  char *str_rtabs;
  char *str_search_path;
  char *str_name_format;
  char *str_limit;
  char *str_printmode;
  char *str_compare;

  bool tabs_processed;

  // Processed settings
  bool en_desc;             // controls for reading descendant processes
  char * en_desc_usr;       // selective descendants: user specificaton
  regex_t * en_desc_cmp;    // selective descendants: compiled specification
  Histable::NameFormat name_format; // long/short/mangled naming for C++/Java
  VMode view_mode;          // Java mode
  int src_compcom;          // compiler commentary visibility for anno-src
  int dis_compcom;          // compiler commentary visibility for anno-dis
  int threshold_src;        // threshold for anno-src
  int threshold_dis;        // threshold for anno-dis
  int cmpline_visible;      // show compile-line flags
  int funcline_visible;     // show compile-line flags
  int src_visible;          // show source in disasm
  bool srcmetric_visible;   // show metrics for source in disasm
  bool hex_visible;         // show hex code in disasm
  char* tldata;             // timeline data type string
  int tlmode;               // timeline mode for bars
  int stack_align;          // timeline stack alignment
  int stack_depth;          // timeline stack depth
  int limit;                // print limit
  enum PrintMode print_mode;// print mode
  char print_delim;         // the delimiter, if print mode = PM_DELIM_SEP_LIST
  int compare_mode;         // compare mode

  char *machinemodel; // machine model for Memory Objects

  bool ignore_no_xhwcprof; // ignore no -xhwcprof data in dataspace
  bool ignore_fs_warn; // ignore file-system recording warning

  void set_rc (const char *path, bool msg, Emsgqueue *commentq,
	       bool override, bool ipc_or_rdt_mode = false);

  Vector<DispTab*> *tab_list;
  Vector<pathmap_t*> *pathmaps;
  Vector<lo_expand_t*> *lo_expands;
  enum LibExpand lo_expand_default;
  bool is_loexpand_default;
  Vector<bool> *mem_tab_state;
  Vector<int> *mem_tab_order;
  Vector<bool> *indx_tab_state;
  Vector<int> *indx_tab_order;
};

#endif /* ! _SETTINGS_H */
