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

#ifndef _PRINT_H
#define _PRINT_H


// Include files
#include <stdio.h>
#include <stdlib.h>
#include "dbe_types.h"
#include "Metric.h"
#include "Hist_data.h"
#include "Ovw_data.h"
#include "Stats_data.h"
#include "Emsg.h"
#include "Exp_Layout.h"
#include "DefaultMap.h"
#include "FileData.h"
#include "HeapData.h"
#include "HashMap.h"

const char nl[] = "\n";
const char tab[] = "\t";

// Printing options.
enum Print_destination
{
  DEST_PRINTER      = 0,
  DEST_FILE         = 1,
  DEST_OPEN_FILE    = 2
};

enum Print_mode
{
  MODE_LIST,
  MODE_DETAIL,
  MODE_GPROF,
  MODE_ANNOTATED
};

struct Print_params
{
  Print_destination dest;   // printer or file
  char *name;               // of printer or file
  int ncopies;              // # of copies
  bool header;              // print header first
  FILE *openfile;           // if destination is DEST_OPEN_FILE
};

class Experiment;
class MetricList;
class DbeView;
class Stack_coverage;
class Function;
class LoadObject;

//  Class Definitions
class er_print_common_display
{
public:
  er_print_common_display ()
  {
    out_file = NULL;
    pr_params.header = false;
  }

  virtual ~er_print_common_display () { }

  //  Open the file/printer to write to
  int open (Print_params *);

  void
  set_out_file (FILE *o)
  {
    out_file = o;
    pr_params.dest = DEST_FILE;
  }

  //  Print the final output data.  This function calls
  //  data_dump() to actually do the dumping of data.
  bool print_output ();

  //  Print the output in the appropriate format.
  virtual void data_dump () = 0;

  void header_dump (int exp_idx);

  // Return the report. If the report size is greater than max, return truncated report
  // Allocates memory, so the caller should free this memory.
  char *get_output (int max);

protected:
  DbeView *dbev;
  FILE *out_file;
  Print_params pr_params;
  char *tmp_file;
  int exp_idx1, exp_idx2;
  bool load;
  bool header;
};

class er_print_histogram : public er_print_common_display
{
public:
  er_print_histogram (DbeView *dbv, Hist_data *data, MetricList *metrics_list,
		      Print_mode disp_type, int limit, char *sort_name,
		      Histable *sobj, bool show_load, bool show_header);
  void data_dump ();

private:
  void dump_list (int limit);
  void dump_detail (int limit);
  void get_gprof_width (Metric::HistMetric *hist_metric, int limit);
  void dump_gprof (int limit);
  void dump_annotated_dataobjects (Vector<int> *marks, int threshold);
  void dump_annotated ();

  Stack_coverage *stack_cov;
  Hist_data *hist_data;
  MetricList *mlist;
  Print_mode type;
  int number_entries;
  char *sort_metric;
  Histable *sel_obj;
};

class er_print_ctree : public er_print_common_display
{
public:
  er_print_ctree (DbeView *dbv, Vector<Histable*> *cstack, Histable *sobj,
		  int limit);
  void data_dump ();
  void print_children (Hist_data *data, int index, Histable *obj, char *prefix,
		       Hist_data::HistItem *total);

private:
  Vector<Histable*> *cstack;
  Histable *sobj;
  MetricList *mlist;
  Metric::HistMetric *hist_metric;
  int limit;
  int print_row;
};

class er_print_gprof : public er_print_common_display
{
public:
  er_print_gprof (DbeView *dbv, Vector<Histable*> *cstack);
  void data_dump ();
private:
  Vector<Histable*> *cstack;
};

class er_print_leaklist : public er_print_common_display
{
public:
  er_print_leaklist (DbeView *dbv, bool show_leak,
		     bool show_alloca, int limit);
  void data_dump ();

private:
  bool leak;
  bool alloca;
  int limit;
};

class er_print_heapactivity : public er_print_common_display
{
public:
  er_print_heapactivity (DbeView *_dbev, Histable::Type _type,
			 bool _printStat, int _limit);
  void data_dump ();

private:
  void printStatistics (Hist_data *hist_data);
  void printCallStacks (Hist_data *hist_data);

  Histable::Type type;
  bool printStat;
  int limit;
};

class er_print_ioactivity : public er_print_common_display
{
public:
  er_print_ioactivity (DbeView *_dbev, Histable::Type _type,
		       bool _printStat, int _limit);
  void data_dump ();

private:
  void printStatistics (Hist_data *hist_data);
  void printCallStacks (Hist_data *hist_data);

  Histable::Type type;
  bool printStat;
  int limit;
};

class er_print_experiment : public er_print_common_display
{
public:
  er_print_experiment (DbeView *me, int bgn_idx, int end_idx, bool show_load,
	   bool show_header, bool show_stat, bool show_over, bool show_odetail);
  void data_dump ();

private:
  int max_len1, max_len2, max_len3;
  void overview_sum (int &maxlen);
  void overview_dump (int exp_idx, int &maxlen);
  void overview_summary (Ovw_data *ovw_data, int &maxlen);
  void overview_item (Ovw_data::Ovw_item *ovw_item,
		      Ovw_data::Ovw_item *ovw_item_labels);
  void overview_value (Value *value, ValueTag value_tag, double total_value);
  void statistics_sum (int &maxlen);
  void statistics_dump (int exp_idx, int &maxlen);
  void statistics_item (Stats_data *stats_data);

  bool stat;
  bool over;
  bool odetail;
};

//  Print the header.  Experiment name and the sample
//  selection, along with the percentage.
char *pr_load_objects (Vector<LoadObject*> *loadobjects, char *lead);
char *pr_samples (Experiment *exp);
char *pr_mesgs (Emsg *msg, const char *null_str, const char *lead);
void print_load_object (FILE *out_file);
void print_header (Experiment *exp, FILE *out_file);

// Print Function metrics
int print_label (FILE *out_file, MetricList *metrics_list,
		 Metric::HistMetric *hist_metric, int space);
void print_anno_file (char *name, const char *sel, const char *srcFile,
		      bool isDisasm, FILE *dis_file, FILE *inp_file,
		      FILE *out_file, DbeView *dbev, bool xdefault);
void print_html_title (FILE *out_file, char *title);
void print_html_label (FILE *out_file, MetricList *metrics_list);
void print_html_content (FILE *out_file, Hist_data *d, MetricList *metrics_list,
			 int limit, Histable::NameFormat nfmt);
void print_html_one (FILE *out_file, Hist_data *data, Hist_data::HistItem *item,
		     MetricList *metrics_list, Histable::NameFormat nfmt);
void print_html_trailer (FILE* out_file);
char *html_ize_name (char *name);
void print_delim_label (FILE *out_file, MetricList *metrics_list, char delim);
void print_delim_content (FILE *out_file, Hist_data *data,
			  MetricList *metrics_list, int limit,
			  Histable::NameFormat nfmt, char delim);
void print_delim_one (FILE *out_file, Hist_data *data, Hist_data::HistItem *item,
	       MetricList *metrics_list, Histable::NameFormat nfmt, char delim);
void print_delim_trailer (FILE* out_file, char delim);
char *csv_ize_name (char *name, char delim);
char *split_metric_name (char *name);

#endif
