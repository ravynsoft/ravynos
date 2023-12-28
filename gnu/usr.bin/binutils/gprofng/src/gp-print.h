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

#ifndef _GP_PRINT_H
#define _ER_PRINT_H

#include "Command.h"
#include "DbeApplication.h"
#include "Histable.h"
#include "Print.h"

void ipc_mainLoop (int argc, char *argv[]);

class DbeView;
template <class ITEM> class Vector;

// er_print object
class er_print : public DbeApplication
{
public:

  er_print (int argc, char *argv[]);
  virtual ~er_print ();
  void start (int argc, char *argv[]);
  bool free_memory_before_exit ();

private:

  char *error_msg;
  DbeView *dbev;
  char *out_fname;
  FILE *inp_file;
  FILE *dis_file;
  FILE *out_file;
  int dbevindex;
  char *cov_string;
  int limit;
  Vector<Histable*> *cstack;
  bool was_QQUIT;

  // override methods in base class
  int check_args (int argc, char *argv[]);
  void usage ();

  int is_valid_seg_name (char *seg_name, int prev);
  int cmp_seg_name (char *full_name, char *seg_name);
  int process_object_select (char *cov);
  int set_libexpand (char *cov, enum LibExpand expand);
  int set_libdefaults ();

  bool end_command (char *cmd);
  void run (int argc, char *argv[]);
  void proc_script ();
  void proc_cmd (CmdType cmd_type, int cparam, char *arg1, char *arg2,
		 char *arg3 = NULL, char *arg4 = NULL, bool xdefault = true);
  void disp_list (int no_header, int size, int align[],
		  char *header[], char **lists[]);
  void exp_list ();
  void describe ();
  void obj_list ();
  void seg_list ();
  void print_objects ();
  void print_overview ();
  void print_overview_nodes (Vector<void*> *data, int level,
			     Vector<char *> *metric_cmds, Vector<char *> *non_metric_cmds);
  void print_overview_tree (Vector<void*> *data, int level, Vector<void*> *values,
			    Vector<char *> *metric_cmds, Vector<char *> *non_metric_cmds);
  void print_segments ();
  void filter_list (CmdType cmd_type);
  int check_exp_id (int exp_id, char *sel);
  int get_exp_id (char *sel, int &bgn_index, int &end_index);
  void print_func (Histable::Type type, Print_mode mode,
		   MetricList *mlist1, MetricList *mlist2,
		   char *func_name = NULL, char *sel = NULL);
  void print_gprof (CmdType cmd_type, char *func_name, char *sel);
  void print_ctree (CmdType cmd_type);
  void print_dobj (Print_mode type, MetricList *mlist1,
		   char *dobj_name = NULL, char *sel = NULL);
  void memobj (char *, int);
  void mo_list (bool showtab, FILE *outf);
  void mo_define (char *, char *, char *, char *, char *);
  void indxobj (char *, int);
  void indxo_list (bool showtab, FILE *outf);
  void indxo_define (char *, char *, char *, char *);
  void ifreq ();
  void dump_nodes ();
  void dump_stacks ();
  void dump_unk_pcs ();
  void dump_funcs (char *);
  void dump_dataobjects (char *);
  void dump_map ();
  void dump_entities ();
  void dump_stats ();
  void dump_proc_warnings ();
  void send_signal ();
  void print_cmd (er_print_common_display *);
  FILE *set_outfile (char *cmd, FILE *&set_file, bool append);
  void gen_mapfile (char *seg_name, char *cmd);
};

#endif /* _ER_PRINT_H */
