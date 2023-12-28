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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <libintl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"
#include "Dbe.h"
#include "StringBuilder.h"
#include "DbeSession.h"
#include "DbeView.h"
#include "Settings.h"
#include "Print.h"
#include "DbeView.h"
#include "Experiment.h"
#include "MetricList.h"
#include "Module.h"
#include "Function.h"
#include "DataSpace.h"
#include "DataObject.h"
#include "FilterExp.h"
#include "LoadObject.h"
#include "Emsg.h"
#include "Table.h"
#include "DbeFile.h"
#include "CallStack.h"

int
er_print_common_display::open (Print_params *params)
{
  pr_params = *params;
  pr_params.name = dbe_strdup (params->name);
  if (params->dest == DEST_PRINTER)
    {
      tmp_file = dbeSession->get_tmp_file_name (NTXT ("print"), false);
      dbeSession->tmp_files->append (strdup (tmp_file));
      out_file = fopen (tmp_file, NTXT ("w"));
    }
  else if (params->dest == DEST_OPEN_FILE)
    out_file = pr_params.openfile;
  else
    out_file = fopen (pr_params.name, NTXT ("w"));

  if (out_file == NULL)
    // Failure
    return 1;
  return 0;
}

bool
er_print_common_display::print_output ()
{
  char *sys_call;
  bool ret = true;
  if (pr_params.dest != DEST_OPEN_FILE)
    fclose (out_file);

  if (pr_params.dest == DEST_PRINTER)
    {
      if (streq ((char *) pr_params.name, NTXT ("")))
	sys_call = dbe_sprintf ("(/usr/bin/lp -c -n%d %s) 2>/dev/null 1>&2",
				pr_params.ncopies, tmp_file);
      else
	sys_call = dbe_sprintf ("(/usr/bin/lp -c -d%s -n%d %s) 2>/dev/null 1>&2",
				pr_params.name, pr_params.ncopies, tmp_file);
      if (system (sys_call) != 0)
	ret = false;
      unlink (tmp_file);
      free (sys_call);
    }

  return ret;
}

// Return the report. If the report size is greater than max, return truncated report
// Allocates memory, so the caller should free this memory.

char *
er_print_common_display::get_output (int maxsize)
{
  off_t max = (off_t) maxsize;
  if (out_file != (FILE *) NULL)
    {
      fclose (out_file); // close tmp_file
      out_file = (FILE *) NULL;
    }
  struct stat sbuf;
  int st = stat (tmp_file, &sbuf);
  if (st == 0)
    {
      off_t sz = sbuf.st_size;
      if (sz > max)
	return dbe_sprintf (GTXT ("Error: report is too long.\n"));
      if (sz <= 0)
	return dbe_sprintf (GTXT ("Error: empty temporary file: %s\n"),
			    tmp_file);
      max = sz;
    }

  FILE *f = fopen (tmp_file, "r");
  if (f == NULL)
    return dbe_sprintf (GTXT ("Error: cannot open temporary file: %s\n"),
			tmp_file);
  char *report = (char *) malloc (max);
  if (report)
    {
      if (1 != fread (report, max - 1, 1, f))
	{
	  fclose (f);
	  free (report);
	  return dbe_sprintf (GTXT ("Error: cannot read temporary file: %s\n"),
			      tmp_file);
	}
      report[max - 1] = 0;
    }
  fclose (f);
  return report;
}

void
er_print_common_display::header_dump (int exp_idx)
{
  if (load && (exp_idx == exp_idx1))
    {
      load = false;
      print_load_object (out_file);
    }
  print_header (dbeSession->get_exp (exp_idx), out_file);
}

char *
pr_load_objects (Vector<LoadObject*> *loadobjects, char *lead)
{
  int size, i;
  LoadObject *lo;
  Emsg *m;
  char *msg;
  StringBuilder sb;
  char *lo_name;
  size = loadobjects->size ();
  for (i = 0; i < size; i++)
    {
      lo = loadobjects->fetch (i);
      lo_name = lo->get_name ();
      if (lo_name != NULL)
	{
	  size_t len = strlen (lo_name);
	  if (len > 7 && streq (lo_name + len - 7, NTXT (".class>")))
	    continue;
	}

      // print the segment name
      sb.append (lead);
      sb.append (NTXT (" "));
      sb.append (lo->get_name ());
      sb.append (NTXT (" ("));
      sb.append (lo->get_pathname ());
      sb.append (NTXT (")\n"));

      // and any warnings
      m = lo->fetch_warnings ();
      if (m != NULL)
	{
	  msg = pr_mesgs (m, NULL, NTXT ("       "));
	  sb.append (msg);
	  free (msg);
	}
    }
  return sb.toString ();
}

char *
pr_mesgs (Emsg *msg, const char *null_str, const char *lead)
{
  Emsg *m;
  StringBuilder sb;
  if (msg == NULL)
    return dbe_strdup (null_str);
  for (m = msg; m; m = m->next)
    {
      sb.append (lead);
      sb.append (m->get_msg ());
      sb.append (NTXT ("\n"));
    }
  return sb.toString ();
}

void
print_load_object (FILE *out_file)
{
  Vector<LoadObject*> *loadobjects = dbeSession->get_text_segments ();
  char *msg = pr_load_objects (loadobjects, NTXT ("\t"));
  fprintf (out_file, GTXT ("Load Object Coverage:\n"));
  fprintf (out_file, NTXT ("%s"), msg);
  fprintf (out_file,
	   "----------------------------------------------------------------\n");
  free (msg);
  delete loadobjects;
}

void
print_header (Experiment *exp, FILE *out_file)
{
  fprintf (out_file, GTXT ("Experiment: %s\n"), exp->get_expt_name ());
  char *msg = pr_mesgs (exp->fetch_notes (), NTXT (""), NTXT (""));
  fprintf (out_file, NTXT ("%s"), msg);
  free (msg);

  msg = pr_mesgs (exp->fetch_errors (), GTXT ("No errors\n"), NTXT (""));
  fprintf (out_file, NTXT ("%s"), msg);
  free (msg);

  msg = pr_mesgs (exp->fetch_warnings (), GTXT ("No warnings\n"), NTXT (""));
  fprintf (out_file, NTXT ("%s"), msg);
  free (msg);

  msg = pr_mesgs (exp->fetch_comments (), NTXT (""), NTXT (""));
  fprintf (out_file, NTXT ("%s"), msg);
  free (msg);

  msg = pr_mesgs (exp->fetch_pprocq (), NTXT (""), NTXT (""));
  fprintf (out_file, NTXT ("%s"), msg);
  free (msg);
}

static char *
delTrailingBlanks (char *s)
{
  for (int i = (int) strlen (s) - 1; i >= 0 && s[i] == ' '; i--)
    s[i] = 0;
  return s;
}

/**
 * Print the 3-line header with column heads for the metrics
 * Return offset of "Name" column (this is needed to print Callers-Callees)
 */
int
print_label (FILE *out_file, MetricList *metrics_list,
	     Metric::HistMetric *hist_metric, int space)
{
  char line0[2 * MAX_LEN], line1[2 * MAX_LEN];
  char line2[2 * MAX_LEN], line3[2 * MAX_LEN];
  int name_offset = 0;
  *line0 = *line1 = *line2 = *line3 = '\0';
  Vector<Metric*> *mlist = metrics_list->get_items ();
  for (int index = 0, mlist_sz = mlist->size (); index < mlist_sz; index++)
    {
      Metric *mitem = mlist->fetch (index);
      if (mitem->is_visible () || mitem->is_tvisible () || mitem->is_pvisible ())
	{
	  Metric::HistMetric *hitem = hist_metric + index;
	  const char *s;
	  if (index > 0 && mitem->get_type () == Metric::ONAME)
	    {
	      s = " ";
	      name_offset = strlen (line1);
	    }
	  else
	    s = "";
	  int width = (int) hitem->width;
	  size_t len = strlen (line1);
	  snprintf (line1 + len, sizeof (line1) - len, "%s%-*s", s, width,
		    hitem->legend1);
	  len = strlen (line2);
	  snprintf (line2 + len, sizeof (line2) - len, "%s%-*s", s, width,
		    hitem->legend2);
	  len = strlen (line3);
	  snprintf (line3 + len, sizeof (line3) - len, "%s%-*s", s, width,
		    hitem->legend3);
	  len = strlen (line0);
	  snprintf (line0 + len, sizeof (line0) - len, "%s%-*s", s, width,
		    mitem->legend ? mitem->legend : NTXT (""));
	}
    }
  char *s = delTrailingBlanks (line0);
  if (*s)
    fprintf (out_file, NTXT ("%*s%s\n"), space, NTXT (""), s);
  fprintf (out_file, NTXT ("%*s%s\n"), space, NTXT (""), delTrailingBlanks (line1));
  fprintf (out_file, NTXT ("%*s%s\n"), space, NTXT (""), delTrailingBlanks (line2));
  fprintf (out_file, NTXT ("%*s%s\n"), space, NTXT (""), delTrailingBlanks (line3));
  return name_offset;
}

er_print_histogram::er_print_histogram (DbeView *_dbev, Hist_data *data,
					MetricList *metrics_list,
					Print_mode disp_type, int limit,
					char *sort_name, Histable *sobj,
					bool show_load, bool show_header)
{
  hist_data = data;
  mlist = metrics_list;
  type = disp_type;
  number_entries = limit;
  sort_metric = sort_name;
  sel_obj = sobj;
  dbev = _dbev;
  exp_idx1 = 0;
  exp_idx2 = dbeSession->nexps () - 1;
  load = show_load;
  header = show_header;
}

void
er_print_histogram::dump_list (int limit)
{
  Histable::NameFormat nfmt = dbev->get_name_format ();
  StringBuilder sb;
  char *title = NULL; // No title for some formats
  enum PrintMode pm = dbev->get_printmode ();

  // create a header line, except for delimiter-separated list output
  if (pm != PM_DELIM_SEP_LIST)
    {
      if (hist_data->type == Histable::FUNCTION)
	sb.append (GTXT ("Functions sorted by metric: "));
      else if (hist_data->type == Histable::INSTR)
	sb.append (GTXT ("PCs sorted by metric: "));
      else if (hist_data->type == Histable::LINE)
	sb.append (GTXT ("Lines sorted by metric: "));
      else if (hist_data->type == Histable::DOBJECT)
	sb.append (GTXT ("Dataobjects sorted by metric: "));
      else
	sb.append (GTXT ("Objects sorted by metric: "));
      sb.append (sort_metric);
      title = sb.toString ();
    }

  switch (pm)
    {
    case PM_TEXT:
      {
	Metric::HistMetric *hist_metric = hist_data->get_histmetrics ();
	fprintf (out_file, NTXT ("%s\n\n"), title); //print title
	hist_data->print_label (out_file, hist_metric, 0);
	hist_data->print_content (out_file, hist_metric, limit);
	fprintf (out_file, nl);
	break;
      }
    case PM_HTML:
      {
	print_html_title (out_file, title);
	print_html_label (out_file, mlist);
	print_html_content (out_file, hist_data, mlist, limit, nfmt);
	print_html_trailer (out_file);
	break;
      }
    case PM_DELIM_SEP_LIST:
      {
	char delim = dbev->get_printdelimiter ();
	print_delim_label (out_file, mlist, delim);
	print_delim_content (out_file, hist_data, mlist, limit, nfmt, delim);
	print_delim_trailer (out_file, delim);
	break;
      }
    }
  free (title);
}

void
er_print_histogram::dump_annotated_dataobjects (Vector<int> *marks,
						int ithreshold)
{
  if (!dbeSession->is_datamode_available ())
    fprintf (out_file,
                    GTXT ("No dataspace information recorded in experiments\n\n"));

  Hist_data *layout_data = dbev->get_data_space ()->get_layout_data (hist_data, marks, ithreshold);
  Metric::HistMetric *hist_metric = layout_data->get_histmetrics ();

//  snprintf (hist_metric[name_index].legend2, MAX_LEN, GTXT ("* +offset .element"));
  layout_data->print_label (out_file, hist_metric, 3);
  fprintf (out_file, nl);
  StringBuilder sb;

  for (long i = 0; i < layout_data->size (); i++)
    {
      sb.setLength (0);
      if (marks->find (i) != -1)
	sb.append ("## ");
      else
	sb.append ("   ");
      layout_data->print_row (&sb, i, hist_metric, " ");
      sb.toFileLn (out_file);
    }
  fprintf (out_file, nl);
  delete layout_data;
}

static int
max_length(size_t len, size_t str_len)
{
  if (str_len > len)
    return str_len;
  return len;
}

void
er_print_histogram::dump_detail (int limit)
{
  Histable *obj;
  Hist_data *current_data;
  Histable::Type htype;
  TValue *values;
  double dvalue, percent;
  MetricList *prop_mlist = new MetricList (mlist);
  Metric *mitem;
  int index, i;
  Module *module;
  LoadObject *loadobject;
  char *sname, *oname, *lname, *alias, *mangle;

  Histable::NameFormat nfmt = dbev->get_name_format ();

  // Check max. length of metrics names
  size_t len = 0, slen = 0;
  Vec_loop (Metric*, prop_mlist->get_items (), index, mitem)
  {
    mitem->set_vvisible (true);
    if (mitem->get_vtype () == VT_LABEL)
      continue;

    if (mitem->get_subtype () != Metric::STATIC)
      {
	mitem->set_pvisible (true);
	len = max_length (len, hist_data->value_maxlen (index));
	slen = max_length (slen, strlen (mitem->get_name ()));
      }
  }

  // now get the length of the other (non-performance-data) messages
  if (hist_data->type == Histable::FUNCTION)
    {
      slen = max_length (slen, strlen (GTXT ("Source File")));
      slen = max_length (slen, strlen (GTXT ("Object File")));
      slen = max_length (slen, strlen (GTXT ("Load Object")));
      slen = max_length (slen, strlen (GTXT ("Mangled Name")));
      slen = max_length (slen, strlen (GTXT ("Aliases")));
    }
  else if (hist_data->type == Histable::DOBJECT)
    {
      slen = max_length (slen, strlen (GTXT ("Scope")));
      slen = max_length (slen, strlen (GTXT ("Type")));
      slen = max_length (slen, strlen (GTXT ("Member of")));
      slen = max_length (slen, strlen (GTXT ("Offset (bytes)")));
      slen = max_length (slen, strlen (GTXT ("Size (bytes)")));
      slen = max_length (slen, strlen (GTXT ("Elements")));
    }
  int max_len = (int) len;
  int smax_len = (int) slen;

#define PR_TITLE(t)   fprintf (out_file, "\t%*s:", smax_len, t)
#define PR(title, nm) PR_TITLE(title); \
		      if (nm) \
		        fprintf (out_file, " %s", nm); \
		      fprintf (out_file, "\n")

  // now loop over the objects
  int num_printed_items = 0;
  for (i = 0; i < hist_data->size (); i++)
    {
      if (hist_data->type == Histable::FUNCTION)
	{
	  if (num_printed_items >= limit)
	    break;
	  obj = sel_obj ? sel_obj : hist_data->fetch (i)->obj;
	  htype = obj->get_type ();

	  // ask the view for all the data for the object
	  // xxxxx may be expensive to rescan all packets via get_hist_data()
	  current_data = dbev->get_hist_data (prop_mlist,
					      htype, 0, Hist_data::SELF, obj);
	  if (current_data->size () == 0)
	    continue;
	  values = current_data->fetch (0)->value;
	}
      else
	{
	  obj = hist_data->fetch (i)->obj;
	  DataObject *dobj = (DataObject*) obj;
	  if (sel_obj)
	    {
	      // print selected item and its members
	      if (sel_obj != obj
		  && (DataObject*) sel_obj != dobj->get_parent ())
		// not a match, advance to next item
		continue;
	    }
	  else if (num_printed_items >= limit)
	    break;
	  htype = obj->get_type ();
	  values = hist_data->fetch (i)->value;
	  current_data = hist_data;
	}

      if (num_printed_items)
	// if this isn't the first one, add a blank line
	fprintf (out_file, NTXT ("\n"));
      num_printed_items++;

      // Print full object name
      if (htype != Histable::DOBJECT)
	fprintf (out_file, NTXT ("%s\n"), obj->get_name (nfmt));
      else
	{
	  DataObject *dobj = (DataObject*) obj;
	  if (!dobj->get_parent ())
	    fprintf (out_file, NTXT ("%s\n"), obj->get_name (nfmt));
	  else
	    fprintf (out_file, NTXT ("    %s\n"), obj->get_name (nfmt));
	}

      Vec_loop (Metric*, prop_mlist->get_items (), index, mitem)
      {
	if (mitem->get_vtype () == VT_LABEL)
	  continue;
	if (mitem->get_subtype () == Metric::STATIC
	    && htype == Histable::DOBJECT)
	  continue;
	PR_TITLE (mitem->get_name ());

	char buf[128];
	char *s = values[index].to_str (buf, sizeof (buf));
	if (mitem->get_value_styles () & VAL_PERCENT)
	  {
	    dvalue = values[index].to_double ();
	    percent = 100.0 * current_data->get_percentage (dvalue, index);
	    if (!mitem->is_time_val ())
	      {
		fprintf (out_file, " %*s", max_len, s);
		if (dvalue == 0.)
		  fprintf (out_file, " (  0. %%)\n");
		else
		  fprintf (out_file, " (%5.1f%%)\n", percent);
		continue;
	      }

	    TValue v;
	    v.tag = VT_DOUBLE;
	    v.sign = false;
	    v.d = dvalue / (1.e+6 * dbeSession->get_clock (-1));
	    char buf1[128];
	    char *s1 = v.to_str (buf1, sizeof (buf1));
	    fprintf (out_file, " %*s", max_len, s1);
	    if (dvalue == 0.)
	      fprintf (out_file, " (  0. %%)\n");
	    else
	      fprintf (out_file, " (%5.1f%%)\n", percent);
	    PR_TITLE (GTXT ("Count"));
	  }

	int max_len1 = max_len;
	for (int j = (int) strlen (s) - 1; j >= 0 && s[j] == ' '; j--)
	  {
	    s[j] = 0;
	    max_len1--;
	  }
	fprintf (out_file, " %*s\n", max_len1, s);
      }

      // now add the descriptive information about the object
      if (htype != Histable::DOBJECT)
	{
	  Function *func = (Function*) obj->convertto (Histable::FUNCTION);
	  if (func && func->get_type () == Histable::FUNCTION)
	    {
	      // Print the source/object/load-object files & aliases
	      oname = lname = alias = NULL;
	      sname = func->getDefSrcName ();
	      mangle = func->get_mangled_name ();
	      if (mangle && streq (func->get_name (), mangle))
		mangle = NULL;
	      module = func->module;
	      if (module)
		{
		  oname = module->get_name ();
		  loadobject = module->loadobject;
		  if (loadobject)
		    {
		      lname = loadobject->get_pathname ();
		      alias = loadobject->get_alias (func);
		    }
		}

	      if (htype == Histable::INSTR && dbeSession->is_datamode_available ())
		alias = ((DbeInstr*) obj)->get_descriptor ();

	      PR (GTXT ("Source File"), sname);
	      PR (GTXT ("Object File"), oname);
	      PR (GTXT ("Load Object"), lname);
	      PR (GTXT ("Mangled Name"), mangle);
	      PR (GTXT ("Aliases"), alias);
	    }
	}
      else
	{
	  // Print the dataobject information
	  DataObject *dobj = (DataObject*) obj;
	  Histable *scope = dobj->get_scope ();

	  // print the scope
	  PR_TITLE (GTXT ("Scope"));
	  if (!scope)
	    fprintf (out_file, GTXT ("(Global)\n"));
	  else switch (scope->get_type ())
	      {
	      case Histable::FUNCTION:
		fprintf (out_file, NTXT ("%s(%s)\n"),
			 ((Function*) scope)->module->get_name (),
			 scope->get_name ());
		break;
	      case Histable::LOADOBJECT:
	      case Histable::MODULE:
	      default:
		fprintf (out_file, NTXT ("%s\n"), scope->get_name ());
	      }

	  // print the type name
	  PR_TITLE (GTXT ("Type"));
	  if (dobj->get_typename ())
	    fprintf (out_file, NTXT ("%s\n"), dobj->get_typename ());
	  else
	    fprintf (out_file, GTXT ("(Synthetic)\n"));

	  // print the offset
	  if (dobj->get_offset () != -1)
	    {
	      if (dobj->get_parent ())
		{
		  PR_TITLE (GTXT ("Member of"));
		  fprintf (out_file, NTXT ("%s\n"), dobj->get_parent ()->get_name ());
		}
	      PR_TITLE (GTXT ("Offset (bytes)"));
	      fprintf (out_file, NTXT ("%lld\n"), (long long) dobj->get_offset ());
	    }
	  // print the size
	  if (dobj->get_size ())
	    {
	      PR_TITLE (GTXT ("Size (bytes)"));
	      fprintf (out_file, NTXT ("%lld\n"), (long long) dobj->get_size ());
	    }
	}
      if (hist_data->type == Histable::FUNCTION)
	delete current_data;
    }
  if (num_printed_items == 0 && sel_obj)
    fprintf (stderr,
	     GTXT ("Error: Specified item `%s' had no recorded metrics.\n"),
	     sel_obj->get_name ());
  delete prop_mlist;
}

static Metric::HistMetric *
allocateHistMetric (int no_metrics)
{
  Metric::HistMetric *hist_metric = new Metric::HistMetric[no_metrics];
  for (int i = 0; i < no_metrics; i++)
    {
      Metric::HistMetric *hm = &hist_metric[i];
      hm->init ();
    }
  return hist_metric;
}

void
er_print_histogram::dump_gprof (int limit)
{
  StringBuilder sb;
  Histable *obj;
  Hist_data *callers;
  Hist_data *callees;
  Hist_data *center;

  int no_metrics = mlist->get_items ()->size ();
  Metric::HistMetric *hist_metric = allocateHistMetric (no_metrics);
  for (int i = 0; i < limit; i++)
    {
      obj = sel_obj ? sel_obj : hist_data->fetch (i)->obj;
      callers = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
				     Hist_data::CALLERS, obj);
      callees = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
				     Hist_data::CALLEES, obj);
      center = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
				    Hist_data::SELF, obj);
      callers->update_max (hist_metric);
      callees->update_max (hist_metric);
      center->update_max (hist_metric);
      callers->update_legend_width (hist_metric);
      callers->print_label (out_file, hist_metric, 0);
      callers->print_content (out_file, hist_metric, callers->size ());

      if (center->size () > 0)
	{
	  center->update_total (callers->get_totals ());
	  sb.setLength (0);
	  center->print_row (&sb, 0, hist_metric, NTXT ("*"));
	  sb.toFileLn (out_file);
	}
      callees->print_content (out_file, hist_metric, callees->size ());
      fprintf (out_file, nl);
      delete callers;
      delete callees;
      delete center;
    }
  delete[] hist_metric;
}

// dump an annotated file
void
dump_anno_file (FILE *fp, Histable::Type type, Module *module, DbeView *dbev,
		MetricList *mlist, TValue *ftotal, const char *srcFile,
		Function *func, Vector<int> *marks, int threshold, int vis_bits,
		int src_visible, bool hex_visible, bool src_only)
{
  int lspace, mspace, tspace, remain, mindex, next_mark, hidx, index;
  Metric *mitem;
  char buf[MAX_LEN];
  Hist_data::HistItem *item;

  SourceFile *srcContext = NULL;
  bool func_scope = dbev == NULL ? false : dbev->get_func_scope ();
  if (srcFile)
    {
      srcContext = module->findSource (srcFile, false);
      if (srcContext == NULL)
	{
	  Vector<SourceFile*> *includes = module->includes;
	  char *bname = get_basename (srcFile);
	  for (int i = 0, sz = includes ? includes->size () : 0; i < sz; i++)
	    {
	      SourceFile *sf = includes->fetch (i);
	      if (streq (get_basename (sf->get_name ()), bname))
		{
		  srcContext = sf;
		  break;
		}
	    }
	}
      if (func)
	func_scope = true;
    }
  else if (func)
    srcContext = func->getDefSrc ();

  Hist_data *hdata = module->get_data (dbev, mlist, type, ftotal, srcContext,
				       func, marks, threshold, vis_bits,
				       src_visible, hex_visible,
				       func_scope, src_only);

  if (hdata == NULL)
    return;

  // force the name metric to be invisible
  MetricList *nmlist = hdata->get_metric_list ();
  nmlist->find_metric (GTXT ("name"), Metric::STATIC)->clear_all_visbits ();
  Metric::HistMetric *hist_metric = hdata->get_histmetrics ();

  // lspace is for max line number that's inserted; use to set width
  int max_lineno = 0;
  Vec_loop (Hist_data::HistItem*, hdata, hidx, item)
  {
    if (!item->obj)
      continue;
    if (item->obj->get_type () == Histable::LINE
	&& ((DbeLine*) item->obj)->lineno > max_lineno)
      max_lineno = ((DbeLine*) item->obj)->lineno;
    else if (item->obj->get_type () == Histable::INSTR
	     && ((DbeInstr*) item->obj)->lineno > max_lineno)
      max_lineno = ((DbeInstr*) item->obj)->lineno;
  }

  lspace = snprintf (buf, sizeof (buf), NTXT ("%d"), max_lineno);

  // mspace is the space needed for all metrics, and the mark, if any
  mspace = 0;
  if (nmlist->get_items ()->size () > 0)
    {
      mspace = 3; // mark "## "
      Vec_loop (Metric*, nmlist->get_items (), index, mitem)
      {
	if (mitem->is_visible () || mitem->is_tvisible ()
	    || mitem->is_pvisible ())
	  mspace += (int) hist_metric[index].width;
      }
    }
  tspace = 0;
  remain = (mspace + lspace + 3) % 8; // " " before, ". " after line#
  if (remain)
    { // tab alignment
      tspace = 8 - remain;
      mspace += tspace;
    }
  mindex = 0;
  next_mark = (mindex < marks->size ()) ? marks->fetch (mindex) : -1;

  // Print the header for this list
  SourceFile *sf = srcContext ? srcContext : module->getMainSrc ();
  char *src_name = sf->dbeFile->get_location_info ();
  DbeFile *df = module->dbeFile;
  if (df == NULL || (df->filetype & DbeFile::F_JAVACLASS) == 0)
    df = module->loadobject->dbeFile;
  char *lo_name = df->get_location_info ();
  char *dot_o_name = lo_name;
  if (module->dot_o_file)
    dot_o_name = module->dot_o_file->dbeFile->get_location_info ();
  fprintf (fp, GTXT ("Source file: %s\nObject file: %s\nLoad Object: %s\n\n"),
	   src_name, dot_o_name, lo_name);

  // Print metric labels
  if (nmlist->get_items ()->size () != 0)
    print_label (fp, nmlist, hist_metric, 3);

  // determine the name metric (not printed as a metric, though)
  int lind = nmlist->get_listorder (GTXT ("name"), Metric::STATIC);

  // now loop over the data rows -- the lines in the annotated source/disasm,
  //	including index lines, compiler commentary, etc.
  StringBuilder sb;
  Vec_loop (Hist_data::HistItem*, hdata, hidx, item)
  {
    sb.setLength (0);
    if (item->type == Module::AT_DIS || item->type == Module::AT_QUOTE
	|| item->type == Module::AT_SRC)
      {
	// does this line get a high-metric mark?
	if (hidx == next_mark)
	  {
	    sb.append (NTXT ("## "));
	    mindex++;
	    next_mark = (mindex < marks->size ()) ? marks->fetch (mindex) : -1;
	  }
	else
	  sb.append (NTXT ("   "));

	hdata->print_row (&sb, hidx, hist_metric, NTXT (" "));
	sb.toFile (fp);
	for (int i = sb.length (); i < mspace; i++)
	  {
	    fputc (' ', fp);
	  }
      }
    else
      // this line does not get any metrics; insert blanks in lieu of them
      for (int i = 0; i < mspace; i++)
	fputc (' ', fp);

    switch (item->type)
      {
      case Module::AT_SRC_ONLY:
	if (item->obj == NULL)
	  fprintf (fp, NTXT ("%*s. "), lspace + 1, "?");
	else
	  fprintf (fp, "%*d. ", lspace + 1, ((DbeLine*) item->obj)->lineno);
	break;

      case Module::AT_SRC:
	fprintf (fp, "%*d. ", lspace + 1, ((DbeLine*) item->obj)->lineno);
	break;
      case Module::AT_FUNC:
      case Module::AT_QUOTE:
	fprintf (fp, NTXT ("%*c"), lspace + 3, ' ');
	break;
      case Module::AT_DIS:
      case Module::AT_DIS_ONLY:
	if (item->obj == NULL || ((DbeInstr*) item->obj)->lineno == -1)
	  fprintf (fp, "%*c[%*s] ", lspace + 3, ' ', lspace, "?");
	else
	  fprintf (fp, "%*c[%*d] ", lspace + 3, ' ', lspace,
		   ((DbeInstr*) item->obj)->lineno);
	break;
      case Module::AT_COM:
      case Module::AT_EMPTY:
	break;

      }
    if (item->value[lind].l == NULL)
      item->value[lind].l = dbe_strdup (GTXT ("INTERNAL ERROR: missing line text"));
    fprintf (fp, NTXT ("%s\n"), item->value[lind].l);
  }
  delete hdata;
}

void
er_print_histogram::dump_annotated ()
{
  Vector<int> *marks = new Vector<int>;
  Function *anno_func = (Function *) sel_obj;
  Module *module = anno_func ? anno_func->module : NULL;

  if (hist_data->type == Histable::DOBJECT)
    dump_annotated_dataobjects (marks, number_entries); // threshold
  else if (number_entries == 0)
      // Annotated source
    dump_anno_file (out_file, Histable::LINE, module, dbev, mlist,
		    hist_data->get_totals ()->value, NULL, anno_func, marks,
		    dbev->get_thresh_src (), dbev->get_src_compcom (),
		    dbev->get_src_visible (), dbev->get_hex_visible (), true);
  else
    // Annotated disassembly
    dump_anno_file (out_file, Histable::INSTR, module, dbev, mlist,
		    hist_data->get_totals ()->value, NULL, anno_func, marks,
		    dbev->get_thresh_dis (), dbev->get_dis_compcom (),
		    dbev->get_src_visible (), dbev->get_hex_visible (), true);
}

void
er_print_histogram::data_dump ()
{
  int limit;
  if (hist_data->get_status () == Hist_data::SUCCESS)
    {
      if (sort_metric[0] == '\n')
	{ // csingle Callers-Callees entry
	  sort_metric++;
	  fprintf (out_file, NTXT ("%s\n\n"), sort_metric);
	}
      else if (!sel_obj && type != MODE_LIST)
	{
	  if (hist_data->type == Histable::FUNCTION)
	    fprintf (out_file,
		     GTXT ("Functions sorted by metric: %s\n\n"), sort_metric);
	  else if (hist_data->type == Histable::DOBJECT)
	    fprintf (out_file, GTXT ("Dataobjects sorted by metric: %s\n\n"),
		     sort_metric);
	  else
	    fprintf (out_file,
		     GTXT ("Objects sorted by metric: %s\n\n"), sort_metric);
	}
      limit = hist_data->size ();
      if ((number_entries > 0) && (number_entries < limit))
	limit = number_entries;

      switch (type)
	{
	case MODE_LIST:
	  dump_list (limit);
	  break;
	case MODE_DETAIL:
	  dump_detail (limit);
	  break;
	case MODE_GPROF:
	  dump_gprof (limit);
	  break;
	case MODE_ANNOTATED:
	  dump_annotated ();
	  break;
	}
    }
  else
    fprintf (out_file, GTXT ("Get_Hist_data call failed %d\n"),
	     (int) hist_data->get_status ());
}

/*
 * Class er_print_ctree to print functions call tree
 */
er_print_ctree::er_print_ctree (DbeView *_dbev, Vector<Histable*> *_cstack,
				Histable *_sobj, int _limit)
{
  dbev = _dbev;
  cstack = _cstack;
  sobj = _sobj;
  limit = _limit;
  print_row = 0;
  exp_idx1 = 0;
  exp_idx2 = dbeSession->nexps () - 1;
  load = false;
  header = false;
}

void
er_print_ctree::data_dump ()
{
  StringBuilder sb;
  Hist_data::HistItem *total;
  sb.append (GTXT ("Functions Call Tree. Metric: "));
  char *s = dbev->getSort (MET_CALL_AGR);
  sb.append (s);
  free (s);
  sb.toFileLn (out_file);
  fprintf (out_file, NTXT ("\n"));
  mlist = dbev->get_metric_list (MET_CALL_AGR);

  // Change cstack: add sobj to the end of cstack
  cstack->append (sobj);
  Hist_data *center = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					   Hist_data::SELF, cstack);
  Hist_data *callers = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					    Hist_data::CALLERS, cstack);
  Hist_data *callees = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					    Hist_data::CALLEES, cstack);

  // Restore cstack
  int last = cstack->size () - 1;
  cstack->remove (last);

  // Prepare formats
  int no_metrics = mlist->size ();

  // calculate max. width using data from callers, callees, center
  hist_metric = allocateHistMetric (no_metrics);
  callers->update_max (hist_metric);
  callees->update_max (hist_metric);
  center->update_max (hist_metric);
  callers->update_legend_width (hist_metric);
  callers->print_label (out_file, hist_metric, 0); // returns Name column offset

  print_row = 0;
  // Pass real total to print_children()
  total = center->get_totals ();
  print_children (center, 0, sobj, NTXT (" "), total);

  // Free memory
  cstack->reset ();
  delete callers;
  delete callees;
  delete center;
  delete[] hist_metric;
}

/*
 * Recursive method print_children prints Call Tree elements.
 */
void
er_print_ctree::print_children (Hist_data *data, int index, Histable *my_obj,
				char * prefix, Hist_data::HistItem *total)
{
  StringBuilder buf;
  const char *P0 = "+-";
  const char *P2 = "  |";
  const char *P1 = "  ";

  // If limit exceeded - return
  ++print_row;
  if (limit > 0 && print_row > limit)
    return;

  if (my_obj == NULL)
    return; // should never happen

  // Prepare prefix
  buf.append (prefix);
  if (buf.endsWith (P2))
    {
      int len = buf.length () - 1;
      buf.setLength (len);
    }
  buf.append (P0);

  // Change cstack: add my_obj to the end of cstack
  cstack->append (my_obj);

  // Print current node info
  char * my_prefix = buf.toString ();

  // Replace parent's total values with real total values
  data->update_total (total); // Needed to to calculate percentage only
  buf.setLength (0);
  data->print_row (&buf, index, hist_metric, my_prefix);
  buf.toFileLn (out_file);
  free (my_prefix);

  // Get children
  Hist_data *callees = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					    Hist_data::CALLEES, cstack);
  int nc = callees->size ();
  if (nc > 0)
    {
      // Print children
      Hist_data::HistItem *item;
      Histable *ch_obj;
      char *ch_prefix;
      buf.setLength (0);
      buf.append (prefix);
      buf.append (P2);
      ch_prefix = buf.toString ();
      for (int i = 0; i < nc - 1; i++)
	{
	  item = callees->fetch (i);
	  ch_obj = item->obj;
	  print_children (callees, i, ch_obj, ch_prefix, total);
	}
      free (ch_prefix);
      buf.setLength (0);
      buf.append (prefix);
      buf.append (P1);
      ch_prefix = buf.toString ();
      item = callees->fetch (nc - 1);
      ch_obj = item->obj;
      print_children (callees, nc - 1, ch_obj, ch_prefix, total);
      free (ch_prefix);
    }

  // Restore cstack
  int last = cstack->size () - 1;
  cstack->remove (last);
  delete callees;
  return;
}

er_print_gprof::er_print_gprof (DbeView *_dbev, Vector<Histable*> *_cstack)
{
  dbev = _dbev;
  cstack = _cstack;
  exp_idx1 = 0;
  exp_idx2 = dbeSession->nexps () - 1;
  load = false;
  header = false;
}

void
er_print_gprof::data_dump ()
{
  StringBuilder sb;
  sb.append (GTXT ("Callers and callees sorted by metric: "));
  char *s = dbev->getSort (MET_CALL);
  sb.append (s);
  free (s);
  sb.toFileLn (out_file);
  fprintf (out_file, NTXT ("\n"));

  MetricList *mlist = dbev->get_metric_list (MET_CALL);
  Hist_data *center = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					   Hist_data::SELF, cstack);
  Hist_data *callers = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					    Hist_data::CALLERS, cstack);
  Hist_data *callees = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					    Hist_data::CALLEES, cstack);

  mlist = center->get_metric_list ();
  int no_metrics = mlist->get_items ()->size ();

  // update max. width for callers/callees/center function item
  Metric::HistMetric *hist_metric = allocateHistMetric (no_metrics);
  callers->update_max (hist_metric);
  callees->update_max (hist_metric);
  center->update_max (hist_metric);

  callers->update_legend_width (hist_metric);
  int name_offset = callers->print_label (out_file, hist_metric, 0); // returns Name column offset
  // Print Callers
  sb.setLength (0);
  for (int i = 0; i < name_offset; i++)
    sb.append (NTXT ("="));
  if (name_offset > 0)
    sb.append (NTXT (" "));
  char *line1 = sb.toString ();
  char *line2;
  if (callers->size () > 0)
    line2 = GTXT ("Callers");
  else
    line2 = GTXT ("No Callers");
  fprintf (out_file, NTXT ("%s%s\n"), line1, line2);
  callers->print_content (out_file, hist_metric, callers->size ());

  // Print Stack Fragment
  line2 = GTXT ("Stack Fragment");
  fprintf (out_file, NTXT ("\n%s%s\n"), line1, line2);

  for (long i = 0, last = cstack->size () - 1; i <= last; ++i)
    {
      sb.setLength (0);
      if (i == last && center->size () > 0)
	{
	  center->update_total (callers->get_totals ()); // Needed to to calculate percentage only
	  center->print_row (&sb, center->size () - 1, hist_metric, NTXT (" "));
	}
      else
	{
	  for (int n = name_offset; n > 0; n--)
	    sb.append (NTXT (" "));
	  if (name_offset > 0)
	    sb.append (NTXT (" "));
	  sb.append (cstack->get (i)->get_name ());
	}
      sb.toFileLn (out_file);
    }

  // Print Callees
  if (callees->size () > 0)
    line2 = GTXT ("Callees");
  else
    line2 = GTXT ("No Callees");
  fprintf (out_file, NTXT ("\n%s%s\n"), line1, line2);
  callees->print_content (out_file, hist_metric, callees->size ());
  fprintf (out_file, nl);
  free (line1);
  delete callers;
  delete callees;
  delete center;
  delete[] hist_metric;
}

er_print_leaklist::er_print_leaklist (DbeView *_dbev, bool show_leak,
				      bool show_alloca, int _limit)
{
  dbev = _dbev;
  leak = show_leak;
  alloca = show_alloca;
  limit = _limit;
}

// Output routine for leak list only
void
er_print_leaklist::data_dump ()
{
  CStack_data *lam;
  CStack_data::CStack_item *lae;
  int index;
  if (!dbeSession->is_leaklist_available ())
    fprintf (out_file, GTXT ("No leak or allocation information recorded in experiments\n\n"));

  MetricList *origmlist = dbev->get_metric_list (MET_NORMAL);
  if (leak)
    {
      // make a copy of the metric list, and set metrics for leaks
      MetricList *nmlist = new MetricList (origmlist);
      nmlist->set_metrics ("e.heapleakbytes:e.heapleakcnt:name", true,
			   dbev->get_derived_metrics ());

      // now make a compacted version of it to get the right indices
      MetricList *mlist = new MetricList (nmlist);
      delete nmlist;

      // fetch the callstack data
      lam = dbev->get_cstack_data (mlist);

      // now print it
      if (lam && lam->size () != 0)
	{
	  fprintf (out_file, GTXT ("Summary Results: Distinct Leaks = %d, Total Instances = %lld, Total Bytes Leaked = %lld\n\n"),
		   (int) lam->size (), lam->total->value[1].ll,
		   lam->total->value[0].ll);

	  Vec_loop (CStack_data::CStack_item*, lam->cstack_items, index, lae)
	  {
	    fprintf (out_file,
		     GTXT ("Leak #%d, Instances = %lld, Bytes Leaked = %lld\n"),
		     index + 1, lae->value[1].ll, lae->value[0].ll);
	    if (lae->stack != NULL)
	      for (int i = lae->stack->size () - 1; i >= 0; i--)
		{
		  DbeInstr *instr = lae->stack->fetch (i);
		  fprintf (out_file, NTXT ("  %s\n"), instr->get_name ());
		}
	    fprintf (out_file, NTXT ("\n"));
	    if (index + 1 == limit) break;
	  }
	}
      else
	fprintf (out_file, GTXT ("No leak information\n\n"));
      delete lam;
      delete mlist;
    }

  if (alloca)
    {
      // make a copy of the metric list, and set metrics for leaks
      MetricList *nmlist = new MetricList (origmlist);
      nmlist->set_metrics ("e.heapallocbytes:e.heapalloccnt:name",
			   true, dbev->get_derived_metrics ());

      // now make a compacted version of it to get the right indices
      MetricList *mlist = new MetricList (nmlist);
      delete nmlist;

      // fetch the callstack data
      lam = dbev->get_cstack_data (mlist);

      // now print it
      if (lam && lam->size () != 0)
	{
	  fprintf (out_file, GTXT ("Summary Results: Distinct Allocations = %d, Total Instances = %lld, Total Bytes Allocated = %lld\n\n"),
		   (int) lam->size (), lam->total->value[1].ll,
		   lam->total->value[0].ll);
	  Vec_loop (CStack_data::CStack_item*, lam->cstack_items, index, lae)
	  {
	    fprintf (out_file, GTXT ("Allocation #%d, Instances = %lld, Bytes Allocated = %lld\n"),
		     index + 1, lae->value[1].ll, lae->value[0].ll);
	    if (lae->stack != NULL)
	      for (int i = lae->stack->size () - 1; i >= 0; i--)
		{
		  DbeInstr *instr = lae->stack->fetch (i);
		  fprintf (out_file, NTXT ("  %s\n"), instr->get_name ());
		}
	    fprintf (out_file, NTXT ("\n"));
	    if (index + 1 == limit) break;
	  }
	}
      else
	fprintf (out_file, GTXT ("No allocation information\n\n"));
      delete lam;
      delete mlist;
    }
}

er_print_heapactivity::er_print_heapactivity (DbeView *_dbev,
					      Histable::Type _type,
					      bool _printStat, int _limit)
{
  dbev = _dbev;
  type = _type;
  printStat = _printStat;
  limit = _limit;
}

void
er_print_heapactivity::printCallStacks (Hist_data *hist_data)
{
  Hist_data::HistItem *hi;
  HeapData *hData;
  long stackId;
  int size = hist_data->size ();
  if (limit > 0 && limit < size)
    size = limit;

  Histable::NameFormat fmt = dbev->get_name_format ();
  for (int i = 0; i < size; i++)
    {
      hi = hist_data->fetch (i);
      hData = (HeapData*) hi->obj;
      stackId = hData->id;
      if (i != 0)
	fprintf (out_file, NTXT ("\n"));

      fprintf (out_file, NTXT ("%s\n"), hData->get_name (fmt));
      if (hData->getAllocCnt () > 0)
	{
	  fprintf (out_file, GTXT ("Instances = %d  "),
		   (int) (hData->getAllocCnt ()));
	  fprintf (out_file, GTXT ("Bytes Allocated = %lld\n"),
		   (long long) hData->getAllocBytes ());
	}

      if (hData->getLeakCnt () > 0)
	{
	  fprintf (out_file, GTXT ("Instances = %d  "),
		   (int) (hData->getLeakCnt ()));
	  fprintf (out_file, GTXT ("Bytes Leaked = %lld\n"),
		   (long long) hData->getLeakBytes ());
	}

      // There is no stack trace for <Total>
      if (i == 0)
	continue;

      // LIBRARY VISIBILITY pass extra argument if necessary to get hide stack
      Vector<Histable*> *instrs = CallStack::getStackPCs ((void *) stackId);
      if (instrs != NULL)
	{
	  int stSize = instrs->size ();
	  for (int j = 0; j < stSize; j++)
	    {
	      Histable *instr = instrs->fetch (j);
	      if (instr != NULL)
		fprintf (out_file, NTXT ("  %s\n"), instr->get_name ());
	    }
	  delete instrs;
	}
    }
}

void
er_print_heapactivity::printStatistics (Hist_data *hist_data)
{
  Hist_data::HistItem *hi;
  HeapData *hDataTotal;
  hi = hist_data->fetch (0);
  hDataTotal = (HeapData*) hi->obj;
  Vector<hrtime_t> *pTimestamps;
  if (hDataTotal->getPeakMemUsage () > 0)
    {
      fprintf (out_file, GTXT ("\nProcess With Highest Peak Memory Usage\n"));
      fprintf (out_file,
	       "-------------------------------------------------------\n");
      fprintf (out_file, GTXT ("Heap size bytes                   %lld\n"),
	       (long long) hDataTotal->getPeakMemUsage ());
      fprintf (out_file, GTXT ("Experiment Id                     %d\n"),
	       (int) (hDataTotal->getUserExpId ()));
      fprintf (out_file, GTXT ("Process Id                        %d\n"),
	       (int) (hDataTotal->getPid ()));
      pTimestamps = hDataTotal->getPeakTimestamps ();
      if (pTimestamps != NULL)
	for (int i = 0; i < pTimestamps->size (); i++)
	  fprintf (out_file,
		   GTXT ("Time of peak                      %.3f (secs.)\n"),
		   (double) (pTimestamps->fetch (i) / (double) NANOSEC));
    }

  if (hDataTotal->getAllocCnt () > 0)
    {
      fprintf (out_file, GTXT ("\nMemory Allocations Statistics\n"));
      fprintf (out_file,
	       GTXT ("Allocation Size Range             Allocations          \n"));
      fprintf (out_file,
	       "-------------------------------------------------------\n");
      if (hDataTotal->getA0KB1KBCnt () > 0)
	fprintf (out_file, NTXT ("  0KB - 1KB                       %d\n"),
		 hDataTotal->getA0KB1KBCnt ());
      if (hDataTotal->getA1KB8KBCnt () > 0)
	fprintf (out_file, NTXT ("  1KB - 8KB                       %d\n"),
		 hDataTotal->getA1KB8KBCnt ());
      if (hDataTotal->getA8KB32KBCnt () > 0)
	fprintf (out_file, NTXT ("  8KB - 32KB                      %d\n"),
		 hDataTotal->getA8KB32KBCnt ());
      if (hDataTotal->getA32KB128KBCnt () > 0)
	fprintf (out_file, NTXT ("  32KB - 128KB                    %d\n"),
		 hDataTotal->getA32KB128KBCnt ());
      if (hDataTotal->getA128KB256KBCnt () > 0)
	fprintf (out_file, NTXT ("  128KB - 256KB                   %d\n"),
		 hDataTotal->getA128KB256KBCnt ());
      if (hDataTotal->getA256KB512KBCnt () > 0)
	fprintf (out_file, NTXT ("  256KB - 512KB                   %d\n"),
		 hDataTotal->getA256KB512KBCnt ());
      if (hDataTotal->getA512KB1000KBCnt () > 0)
	fprintf (out_file, NTXT ("  512KB - 1000KB                  %d\n"),
		 hDataTotal->getA512KB1000KBCnt ());
      if (hDataTotal->getA1000KB10MBCnt () > 0)
	fprintf (out_file, NTXT ("  1000KB - 10MB                   %d\n"),
		 hDataTotal->getA1000KB10MBCnt ());
      if (hDataTotal->getA10MB100MBCnt () > 0)
	fprintf (out_file, NTXT ("  10MB - 100MB                    %d\n"),
		 hDataTotal->getA10MB100MBCnt ());
      if (hDataTotal->getA100MB1GBCnt () > 0)
	fprintf (out_file, NTXT ("  100MB - 1GB                     %d\n"),
		 hDataTotal->getA100MB1GBCnt ());
      if (hDataTotal->getA1GB10GBCnt () > 0)
	fprintf (out_file, NTXT ("  1GB - 10GB                      %d\n"),
		 hDataTotal->getA1GB10GBCnt ());
      if (hDataTotal->getA10GB100GBCnt () > 0)
	fprintf (out_file, NTXT ("  10GB - 100GB                    %d\n"),
		 hDataTotal->getA10GB100GBCnt ());
      if (hDataTotal->getA100GB1TBCnt () > 0)
	fprintf (out_file, NTXT ("  100GB - 1TB                     %d\n"),
		 hDataTotal->getA100GB1TBCnt ());
      if (hDataTotal->getA1TB10TBCnt () > 0)
	fprintf (out_file, NTXT ("  1TB - 10TB                      %d\n"),
		 hDataTotal->getA1TB10TBCnt ());
      fprintf (out_file, GTXT ("\nSmallest allocation bytes         %lld\n"),
	       (long long) hDataTotal->getASmallestBytes ());
      fprintf (out_file, GTXT ("Largest allocation bytes          %lld\n"),
	       (long long) hDataTotal->getALargestBytes ());
      fprintf (out_file, GTXT ("Total allocations                 %d\n"),
	       hDataTotal->getAllocCnt ());
      fprintf (out_file, GTXT ("Total bytes                       %lld\n"),
	       (long long) hDataTotal->getAllocBytes ());
    }

  if (hDataTotal->getLeakCnt () > 0)
    {
      fprintf (out_file, GTXT ("\nMemory Leaks Statistics\n"));
      fprintf (out_file,
	       GTXT ("Leak Size Range                   Leaks              \n"));
      fprintf (out_file,
	       "-------------------------------------------------------\n");
      if (hDataTotal->getL0KB1KBCnt () > 0)
	fprintf (out_file, NTXT ("  0KB - 1KB                       %d\n"),
		 hDataTotal->getL0KB1KBCnt ());
      if (hDataTotal->getL1KB8KBCnt () > 0)
	fprintf (out_file, NTXT ("  1KB - 8KB                       %d\n"),
		 hDataTotal->getL1KB8KBCnt ());
      if (hDataTotal->getL8KB32KBCnt () > 0)
	fprintf (out_file, NTXT ("  8KB - 32KB                      %d\n"),
		 hDataTotal->getL8KB32KBCnt ());
      if (hDataTotal->getL32KB128KBCnt () > 0)
	fprintf (out_file, NTXT ("  32KB - 128KB                    %d\n"),
		 hDataTotal->getL32KB128KBCnt ());
      if (hDataTotal->getL128KB256KBCnt () > 0)
	fprintf (out_file, NTXT ("  128KB - 256KB                   %d\n"),
		 hDataTotal->getL128KB256KBCnt ());
      if (hDataTotal->getL256KB512KBCnt () > 0)
	fprintf (out_file, NTXT ("  256KB - 512KB                   %d\n"),
		 hDataTotal->getL256KB512KBCnt ());
      if (hDataTotal->getL512KB1000KBCnt () > 0)
	fprintf (out_file, NTXT ("  512KB - 1000KB                  %d\n"),
		 hDataTotal->getL512KB1000KBCnt ());
      if (hDataTotal->getL1000KB10MBCnt () > 0)
	fprintf (out_file, NTXT ("  1000KB - 10MB                   %d\n"),
		 hDataTotal->getL1000KB10MBCnt ());
      if (hDataTotal->getL10MB100MBCnt () > 0)
	fprintf (out_file, NTXT ("  10MB - 100MB                    %d\n"),
		 hDataTotal->getL10MB100MBCnt ());
      if (hDataTotal->getL100MB1GBCnt () > 0)
	fprintf (out_file, NTXT ("  100MB - 1GB                     %d\n"),
		 hDataTotal->getL100MB1GBCnt ());
      if (hDataTotal->getL1GB10GBCnt () > 0)
	fprintf (out_file, NTXT ("  1GB - 10GB                      %d\n"),
		 hDataTotal->getL1GB10GBCnt ());
      if (hDataTotal->getL10GB100GBCnt () > 0)
	fprintf (out_file, NTXT ("  10GB - 100GB                    %d\n"),
		 hDataTotal->getL10GB100GBCnt ());
      if (hDataTotal->getL100GB1TBCnt () > 0)
	fprintf (out_file, NTXT ("  100GB - 1TB                     %d\n"),
		 hDataTotal->getL100GB1TBCnt ());
      if (hDataTotal->getL1TB10TBCnt () > 0)
	fprintf (out_file, NTXT ("  1TB - 10TB                      %d\n"),
		 hDataTotal->getL1TB10TBCnt ());
      fprintf (out_file, GTXT ("\nSmallest leaked bytes             %lld\n"),
	       (long long) hDataTotal->getLSmallestBytes ());
      fprintf (out_file, GTXT ("Largest leaked bytes              %lld\n"),
	       (long long) hDataTotal->getLLargestBytes ());
      fprintf (out_file, GTXT ("Total leaked                      %d \n"),
	       hDataTotal->getLeakCnt ());
      fprintf (out_file, GTXT ("Total bytes                       %lld\n"),
	       (long long) hDataTotal->getLeakBytes ());
    }
  fprintf (out_file, NTXT ("\n"));
}

void
er_print_heapactivity::data_dump ()
{
  // get the list of heap events from DbeView
  int numExps = dbeSession->nexps ();
  if (!numExps)
    {
      fprintf (out_file,
	       GTXT ("There is no heap event information in the experiments\n"));
      return;
    }
  MetricList *mlist = dbev->get_metric_list (MET_HEAP);
  Hist_data *hist_data;
  hist_data = dbev->get_hist_data (mlist, type, 0, Hist_data::ALL);
  if (printStat)
    printStatistics (hist_data);
  else
    printCallStacks (hist_data);
}

er_print_ioactivity::er_print_ioactivity (DbeView *_dbev, Histable::Type _type,
					  bool _printStat, int _limit)
{
  dbev = _dbev;
  type = _type;
  printStat = _printStat;
  limit = _limit;
}

void
er_print_ioactivity::printCallStacks (Hist_data *hist_data)
{
  Hist_data::HistItem *hi;
  FileData *fData;
  long stackId;
  int size = hist_data->size ();
  if (limit > 0 && limit < size)
    size = limit;

  for (int i = 0; i < size; i++)
    {
      hi = hist_data->fetch (i);
      fData = (FileData*) hi->obj;
      stackId = fData->id;
      if (i != 0)
	fprintf (out_file, NTXT ("\n"));
      fprintf (out_file, NTXT ("%s\n"), fData->getFileName ());
      if (fData->getWriteCnt () > 0)
	{
	  fprintf (out_file, GTXT ("Write Time=%.6f (secs.)  "),
		   (double) (fData->getWriteTime () / (double) NANOSEC));
	  fprintf (out_file, GTXT ("Write Bytes=%lld  "),
		   (long long) fData->getWriteBytes ());
	  fprintf (out_file, GTXT ("Write Count=%d\n"),
		   (int) (fData->getWriteCnt ()));
	}
      if (fData->getReadCnt () > 0)
	{
	  fprintf (out_file, GTXT ("Read Time=%.6f (secs.)  "),
		   (double) (fData->getReadTime () / (double) NANOSEC));
	  fprintf (out_file, GTXT ("Read Bytes=%lld  "),
		   (long long) fData->getReadBytes ());
	  fprintf (out_file, GTXT ("Read Count=%d\n"),
		   (int) fData->getReadCnt ());
	}
      if (fData->getOtherCnt () > 0)
	{
	  fprintf (out_file, GTXT ("Other I/O Time=%.6f (secs.)  "),
		   (double) (fData->getOtherTime () / (double) NANOSEC));
	  fprintf (out_file, GTXT ("Other I/O Count=%d\n"),
		   (int) (fData->getOtherCnt ()));
	}
      if (fData->getErrorCnt () > 0)
	{
	  fprintf (out_file, GTXT ("I/O Error Time=%.6f (secs.)  "),
		   (double) (fData->getErrorTime () / (double) NANOSEC));
	  fprintf (out_file, GTXT ("I/O Error Count=%d\n"),
		   (int) (fData->getErrorCnt ()));
	}

      // There is no stack trace for <Total>
      if (i == 0)
	continue;

      // LIBRARY VISIBILITY pass extra argument if necessary to get hide stack
      Vector<Histable*> *instrs = CallStack::getStackPCs ((void *) stackId);
      if (instrs != NULL)
	{
	  int stSize = instrs->size ();
	  for (int j = 0; j < stSize; j++)
	    {
	      Histable *instr = instrs->fetch (j);
	      if (instr != NULL)
		fprintf (out_file, "  %s\n", instr->get_name ());
	    }
	  delete instrs;
	}
    }
}

void
er_print_ioactivity::printStatistics (Hist_data *hist_data)
{
  Hist_data::HistItem *hi;
  FileData *fDataTotal;

  hi = hist_data->fetch (0);
  fDataTotal = (FileData*) hi->obj;

  if (fDataTotal->getWriteCnt () > 0)
    {
      fprintf (out_file,
	       GTXT ("\nWrite Statistics\n"));
      fprintf (out_file,
	       GTXT ("I/O Size Range                    Write Calls          \n"));
      fprintf (out_file,
	       "-------------------------------------------------------\n");
      if (fDataTotal->getW0KB1KBCnt () > 0)
	fprintf (out_file, NTXT ("  0KB - 1KB                       %d\n"),
		 fDataTotal->getW0KB1KBCnt ());
      if (fDataTotal->getW1KB8KBCnt () > 0)
	fprintf (out_file, NTXT ("  1KB - 8KB                       %d\n"),
		 fDataTotal->getW1KB8KBCnt ());
      if (fDataTotal->getW8KB32KBCnt () > 0)
	fprintf (out_file, NTXT ("  8KB - 32KB                      %d\n"),
		 fDataTotal->getW8KB32KBCnt ());
      if (fDataTotal->getW32KB128KBCnt () > 0)
	fprintf (out_file, NTXT ("  32KB - 128KB                    %d\n"),
		 fDataTotal->getW32KB128KBCnt ());
      if (fDataTotal->getW128KB256KBCnt () > 0)
	fprintf (out_file, NTXT ("  128KB - 256KB                   %d\n"),
		 fDataTotal->getW128KB256KBCnt ());
      if (fDataTotal->getW256KB512KBCnt () > 0)
	fprintf (out_file, NTXT ("  256KB - 512KB                   %d\n"),
		 fDataTotal->getW256KB512KBCnt ());
      if (fDataTotal->getW512KB1000KBCnt () > 0)
	fprintf (out_file, NTXT ("  512KB - 1000KB                  %d\n"),
		 fDataTotal->getW512KB1000KBCnt ());
      if (fDataTotal->getW1000KB10MBCnt () > 0)
	fprintf (out_file, NTXT ("  1000KB - 10MB                   %d\n"),
		 fDataTotal->getW1000KB10MBCnt ());
      if (fDataTotal->getW10MB100MBCnt () > 0)
	fprintf (out_file, NTXT ("  10MB - 100MB                    %d\n"),
		 fDataTotal->getW10MB100MBCnt ());
      if (fDataTotal->getW100MB1GBCnt () > 0)
	fprintf (out_file, NTXT ("  100MB - 1GB                     %d\n"),
		 fDataTotal->getW100MB1GBCnt ());
      if (fDataTotal->getW1GB10GBCnt () > 0)
	fprintf (out_file, NTXT ("  1GB - 10GB                     %d\n"),
		 fDataTotal->getW1GB10GBCnt ());
      if (fDataTotal->getW10GB100GBCnt () > 0)
	fprintf (out_file, NTXT ("  10GB - 100GB                   %d\n"),
		 fDataTotal->getW10GB100GBCnt ());
      if (fDataTotal->getW100GB1TBCnt () > 0)
	fprintf (out_file, NTXT ("  100GB - 1TB                    %d\n"),
		 fDataTotal->getW100GB1TBCnt ());
      if (fDataTotal->getW1TB10TBCnt () > 0)
	fprintf (out_file, NTXT ("  1TB - 10TB                     %d\n"),
		 fDataTotal->getW1TB10TBCnt ());
      fprintf (out_file,
	       GTXT ("\nLongest write                     %.6f (secs.)\n"),
	       (double) (fDataTotal->getWSlowestBytes () / (double) NANOSEC));
      fprintf (out_file, GTXT ("Smallest write bytes              %lld\n"),
	       (long long) fDataTotal->getWSmallestBytes ());
      fprintf (out_file, GTXT ("Largest write bytes               %lld\n"),
	       (long long) fDataTotal->getWLargestBytes ());
      fprintf (out_file,
	       GTXT ("Total time                        %.6f (secs.)\n"),
	       (double) (fDataTotal->getWriteTime () / (double) NANOSEC));
      fprintf (out_file, GTXT ("Total calls                       %d\n"),
	       fDataTotal->getWriteCnt ());
      fprintf (out_file, GTXT ("Total bytes                       %lld\n"),
	       (long long) fDataTotal->getWriteBytes ());
    }

  if (fDataTotal->getReadCnt () > 0)
    {
      fprintf (out_file,
	       GTXT ("\nRead Statistics\n"));
      fprintf (out_file,
	       GTXT ("I/O Size Range                    Read Calls         \n"));
      fprintf (out_file,
	       "------------------------------------------------------\n");
      if (fDataTotal->getR0KB1KBCnt () > 0)
	fprintf (out_file, NTXT ("  0KB - 1KB                       %d\n"),
		 fDataTotal->getR0KB1KBCnt ());
      if (fDataTotal->getR1KB8KBCnt () > 0)
	fprintf (out_file, NTXT ("  1KB - 8KB                       %d\n"),
		 fDataTotal->getR1KB8KBCnt ());
      if (fDataTotal->getR8KB32KBCnt () > 0)
	fprintf (out_file, NTXT ("  8KB - 32KB                      %d\n"),
		 fDataTotal->getR8KB32KBCnt ());
      if (fDataTotal->getR32KB128KBCnt () > 0)
	fprintf (out_file, NTXT ("  32KB - 128KB                    %d\n"),
		 fDataTotal->getR32KB128KBCnt ());
      if (fDataTotal->getR128KB256KBCnt () > 0)
	fprintf (out_file, NTXT ("  128KB - 256KB                   %d\n"),
		 fDataTotal->getR128KB256KBCnt ());
      if (fDataTotal->getR256KB512KBCnt () > 0)
	fprintf (out_file, NTXT ("  256KB - 512KB                   %d\n"),
		 fDataTotal->getR256KB512KBCnt ());
      if (fDataTotal->getR512KB1000KBCnt () > 0)
	fprintf (out_file, NTXT ("  512KB - 1000KB                  %d\n"),
		 fDataTotal->getR512KB1000KBCnt ());
      if (fDataTotal->getR1000KB10MBCnt () > 0)
	fprintf (out_file, NTXT ("  1000KB - 10MB                   %d\n"),
		 fDataTotal->getR1000KB10MBCnt ());
      if (fDataTotal->getR10MB100MBCnt () > 0)
	fprintf (out_file, NTXT ("  10MB - 100MB                    %d\n"),
		 fDataTotal->getR10MB100MBCnt ());
      if (fDataTotal->getR100MB1GBCnt () > 0)
	fprintf (out_file, NTXT ("  100MB - 1GB                     %d\n"),
		 fDataTotal->getR100MB1GBCnt ());
      if (fDataTotal->getR1GB10GBCnt () > 0)
	fprintf (out_file, NTXT ("  1GB - 10GB                      %d\n"),
		 fDataTotal->getR1GB10GBCnt ());
      if (fDataTotal->getR10GB100GBCnt () > 0)
	fprintf (out_file, NTXT ("  10GB - 100GB                    %d\n"),
		 fDataTotal->getR10GB100GBCnt ());
      if (fDataTotal->getR100GB1TBCnt () > 0)
	fprintf (out_file, NTXT ("  100GB - 1TB                     %d\n"),
		 fDataTotal->getR100GB1TBCnt ());
      if (fDataTotal->getR1TB10TBCnt () > 0)
	fprintf (out_file, NTXT ("  1TB - 10TB                      %d\n"),
		 fDataTotal->getR1TB10TBCnt ());
      fprintf (out_file,
	       GTXT ("\nLongest time                      %.6f (secs.)\n"),
	       (double) (fDataTotal->getRSlowestBytes () / (double) NANOSEC));
      fprintf (out_file, GTXT ("Smallest read bytes               %lld\n"),
	       (long long) fDataTotal->getRSmallestBytes ());
      fprintf (out_file, GTXT ("Largest read bytes                %lld\n"),
	       (long long) fDataTotal->getRLargestBytes ());
      fprintf (out_file,
	       GTXT ("Total time                        %.6f (secs.)\n"),
	       (double) (fDataTotal->getReadTime () / (double) NANOSEC));
      fprintf (out_file, GTXT ("Total calls                       %d\n"),
	       fDataTotal->getReadCnt ());
      fprintf (out_file, GTXT ("Total bytes                       %lld\n"),
	       (long long) fDataTotal->getReadBytes ());
    }

  if (fDataTotal->getOtherCnt () > 0)
    {
      fprintf (out_file, GTXT ("\nOther I/O Statistics\n"));
      fprintf (out_file,
	       "-----------------------------------------------------\n");
      fprintf (out_file,
	       GTXT ("Total time                        %.6f (secs.)\n"),
	       (double) (fDataTotal->getOtherTime () / (double) NANOSEC));
      fprintf (out_file, GTXT ("Total calls                       %d \n"),
	       fDataTotal->getOtherCnt ());
    }
  if (fDataTotal->getErrorCnt () > 0)
    {
      fprintf (out_file, GTXT ("\nI/O Error Statistics\n"));
      fprintf (out_file,
	       "-----------------------------------------------------\n");
      fprintf (out_file,
	       GTXT ("Total time                        %.6f (secs.)\n"),
	       (double) (fDataTotal->getErrorTime () / (double) NANOSEC));
      fprintf (out_file, GTXT ("Total calls                       %d \n"),
	       fDataTotal->getErrorCnt ());
    }
  fprintf (out_file, NTXT ("\n"));
}

void
er_print_ioactivity::data_dump ()
{
  // get the list of io events from DbeView
  int numExps = dbeSession->nexps ();
  if (!numExps)
    {
      fprintf (out_file,
	       GTXT ("There is no IO event information in the experiments\n"));
      return;
    }

  MetricList *mlist = dbev->get_metric_list (MET_IO);
  Hist_data *hist_data = dbev->get_hist_data (mlist, type, 0, Hist_data::ALL);
  if (type == Histable::IOCALLSTACK)
    printCallStacks (hist_data);
  else if (printStat)
    printStatistics (hist_data);
  else
    {
      Metric::HistMetric *hist_metric = hist_data->get_histmetrics ();
      hist_data->print_label (out_file, hist_metric, 0);
      hist_data->print_content (out_file, hist_metric, limit);
      fprintf (out_file, nl);
    }
}

er_print_experiment::er_print_experiment (DbeView *_dbev, int bgn_idx,
					  int end_idx, bool show_load,
					  bool show_header, bool show_stat,
					  bool show_over, bool show_odetail)
{
  dbev = _dbev;
  exp_idx1 = bgn_idx;
  exp_idx2 = end_idx;
  load = show_load;
  header = show_header;
  stat = show_stat;
  over = show_over;
  odetail = show_odetail;
}

void
er_print_experiment::data_dump ()
{
  int index, maxlen;

  maxlen = 0;

  if (stat)
    {
      max_len1 = 50;
      if (exp_idx2 > exp_idx1)
	{
	  statistics_sum (maxlen);
	  fprintf (out_file, nl);
	}

      for (index = exp_idx1; index <= exp_idx2; index++)
	statistics_dump (index, maxlen);
    }
  else if (over)
    {
      max_len1 = 50;
      if (exp_idx2 > exp_idx1)
	{
	  overview_sum (maxlen);
	  fprintf (out_file, nl);
	}

      for (index = exp_idx1; index <= exp_idx2; index++)
	overview_dump (index, maxlen);
    }
  else if (header)
    for (index = exp_idx1; index <= exp_idx2; index++)
      {
	if (index != exp_idx1)
	  fprintf (out_file,
		   "----------------------------------------------------------------\n");
       header_dump (index);
      }
}

void
er_print_experiment::overview_sum (int &maxlen)
{
  int index;
  Ovw_data *sum_data = new Ovw_data ();
  for (index = exp_idx1; index <= exp_idx2; index++)
    {
      Ovw_data *ovw_data = dbev->get_ovw_data (index);
      if (ovw_data == NULL)
	continue;
      sum_data->sum (ovw_data);
      delete ovw_data;
    }

  fprintf (out_file, GTXT ("<Sum across selected experiments>"));
  fprintf (out_file, nl);
  overview_summary (sum_data, maxlen);
  fprintf (out_file, nl);
  delete sum_data;
}

void
er_print_experiment::overview_dump (int exp_idx, int &maxlen)
{
  Ovw_data *ovw_data;
  Ovw_data::Ovw_item ovw_item_labels;
  Ovw_data::Ovw_item ovw_item;
  int index;
  int size;

  ovw_data = dbev->get_ovw_data (exp_idx);
  if (ovw_data == NULL)
    return;
  if (pr_params.header)
    header_dump (exp_idx);
  else if (odetail)
    fprintf (out_file, GTXT ("Experiment: %s\n"),
	     dbeSession->get_exp (exp_idx)->get_expt_name ());

  overview_summary (ovw_data, maxlen);
  if (!odetail)
    {
      delete ovw_data;
      return;
    }

  //Get the collection params for the sample selection and display them.
  fprintf (out_file, "\n\n%*s\n\n", max_len1, GTXT ("Individual samples"));

  size = ovw_data->size ();
  ovw_item_labels = ovw_data->get_labels ();

  for (index = 0; index < size; index++)
    {
      ovw_item = ovw_data->fetch (index);
      fprintf (out_file, "%*s: %d\n\n", max_len1, GTXT ("Sample Number"),
	       ovw_item.number);
      overview_item (&ovw_item, &ovw_item_labels);
      fprintf (out_file, nl);
    }

  delete ovw_data;
}

void
er_print_experiment::overview_summary (Ovw_data *ovw_data, int &maxlen)
{
  char buf[128];
  int len;
  Ovw_data::Ovw_item totals;
  Ovw_data::Ovw_item ovw_item_labels;
  totals = ovw_data->get_totals ();
  len = snprintf (buf, sizeof (buf), "%.3lf", tstodouble (totals.total.t));
  if (maxlen < len)
    maxlen = len;
  max_len2 = maxlen;
  max_len3 = maxlen;
  fprintf (out_file, "%*s\n\n", max_len1,
	   GTXT ("Aggregated statistics for selected samples"));

  ovw_item_labels = ovw_data->get_labels ();
  overview_item (&totals, &ovw_item_labels);
}

void
er_print_experiment::overview_item (Ovw_data::Ovw_item *ovw_item,
				    Ovw_data::Ovw_item *ovw_item_labels)
{
  double start, end, total_value;
  int index, size;
  timestruc_t total_time = {0, 0};

  start = tstodouble (ovw_item->start);
  end = tstodouble (ovw_item->end);

  fprintf (out_file, "%*s: %s\n", max_len1, GTXT ("Start Label"),
	   ovw_item->start_label);
  fprintf (out_file, "%*s: %s\n", max_len1, GTXT ("End Label"),
	   ovw_item->end_label);

  fprintf (out_file, "%*s: ", max_len1, GTXT ("Start Time (sec.)"));
  if (start == -1.0)
    fprintf (out_file, GTXT ("N/A"));
  else
    fprintf (out_file, "%*.3f", max_len2, start);
  fprintf (out_file, nl);
  fprintf (out_file, "%*s: ", max_len1, GTXT ("End Time (sec.)"));
  if (end == -1.0)
    fprintf (out_file, GTXT ("N/A"));
  else
    fprintf (out_file, "%*.3f", max_len2, end);
  fprintf (out_file, nl);
  fprintf (out_file, "%*s: ", max_len1, GTXT ("Duration (sec.)"));
  fprintf (out_file, "%*.3f", max_len2, tstodouble (ovw_item->duration));
  fprintf (out_file, NTXT ("\n"));

  size = ovw_item->size;
  for (index = 0; index < size; index++)
    tsadd (&total_time, &ovw_item->values[index].t);

  total_value = tstodouble (total_time);
  fprintf (out_file, "%*s: %*.3f", max_len1, GTXT ("Total Thread Time (sec.)"),
	   max_len2, tstodouble (ovw_item->tlwp));
  fprintf (out_file, NTXT ("\n"));
  fprintf (out_file, "%*s: ", max_len1, GTXT ("Average number of Threads"));
  if (tstodouble (ovw_item->duration) != 0)
    fprintf (out_file, "%*.3f", max_len2, ovw_item->nlwp);
  else
    fprintf (out_file, GTXT ("N/A"));
  fprintf (out_file, NTXT ("\n\n"));
  fprintf (out_file, "%*s:\n", max_len1, GTXT ("Process Times (sec.)"));
  for (index = 1; index < size; index++)
    {
      overview_value (&ovw_item_labels->values[index], ovw_item_labels->type,
		      total_value);
      overview_value (&ovw_item->values[index], ovw_item->type,
		      total_value);
      fprintf (out_file, NTXT ("\n"));
    }
}

void
er_print_experiment::overview_value (Value *value, ValueTag value_tag,
				     double total_value)
{
  double dvalue;
  switch (value_tag)
    {
    case VT_LABEL:
      fprintf (out_file, "%*s: ", max_len1, value->l);
      break;
    case VT_HRTIME:
      dvalue = tstodouble (value->t);
      if (dvalue == 0.0)
	fprintf (out_file, "%*s (  0. %%)", max_len3, "0.   ");
      else
	fprintf (out_file, "%*.3f (%5.1f%%)", max_len3, dvalue,
		 100.0 * dvalue / total_value);
      break;
    case VT_INT:
      fprintf (out_file, NTXT ("%d"), value->i);
      break;
    default:
      fprintf (out_file, "%*.3f", max_len3, total_value);
    }
}

void
er_print_experiment::statistics_sum (int &maxlen)
{
  int index;
  int size, len;
  Stats_data *sum_data = new Stats_data ();
  for (index = exp_idx1; index <= exp_idx2; index++)
    {
      Stats_data *stats_data = dbev->get_stats_data (index);
      if (stats_data == NULL)
	continue;
      sum_data->sum (stats_data);
      delete stats_data;
    }

  // get the maximum width of values
  size = sum_data->size ();
  for (index = 0; index < size; index++)
    {
      len = (int) sum_data->fetch (index).value.get_len ();
      if (maxlen < len)
	maxlen = len;
    }

  // print overview average
  overview_sum (maxlen);

  // print statistics data
  max_len2 = maxlen;
  statistics_item (sum_data);
  delete sum_data;
}

void
er_print_experiment::statistics_dump (int exp_idx, int &maxlen)
{
  Stats_data *stats_data;
  int index;
  int size, len;
  stats_data = dbev->get_stats_data (exp_idx);
  if (stats_data == NULL)
    return;
  if (pr_params.header)
    {
      header_dump (exp_idx);
      fprintf (out_file, nl);
    }
  else
    fprintf (out_file, GTXT ("Experiment: %s\n"),
	     dbeSession->get_exp (exp_idx)->get_expt_name ());

  // get the maximum width of values
  size = stats_data->size ();
  for (index = 0; index < size; index++)
    {
      len = (int) stats_data->fetch (index).value.get_len ();
      if (maxlen < len)
	maxlen = len;
    }

  // print overview average
  overview_dump (exp_idx, maxlen);
  fprintf (out_file, nl);

  // print statistics data
  max_len2 = maxlen;
  statistics_item (stats_data);
  delete stats_data;
}

void
er_print_experiment::statistics_item (Stats_data *stats_data)
{
  int size, index;
  Stats_data::Stats_item stats_item;
  char buf[256];
  size = stats_data->size ();
  for (index = 0; index < size; index++)
    {
      stats_item = stats_data->fetch (index);
      fprintf (out_file, "%*s: %*s\n", max_len1, stats_item.label,
	       max_len2, stats_item.value.to_str (buf, sizeof (buf)));
    }
  fprintf (out_file, nl);
}

// Print annotated source or disassembly -- called by er_print only
void
print_anno_file (char *name, const char *sel, const char *srcFile,
		 bool isDisasm, FILE *dis_file, FILE *inp_file, FILE *out_file,
		 DbeView *dbev, bool xdefault)
{
  Histable *obj;
  Function *func;
  Module *module;
  Vector<int> *marks;
  Hist_data *hist_data;
  char *errstr;
  int index;
  SourceFile *fitem;
  int threshold;
  int compcom_bits;
  int src_visible;
  bool hex_visible;
  bool srcmetrics_visible;

  if ((name == NULL) || (strlen (name) == 0))
    {
      fprintf (stderr, GTXT ("Error: No function or file has been specified.\n"));
      return;
    }

  // find the function from the name
  if (!dbeSession->find_obj (dis_file, inp_file, obj, name, sel,
			     Histable::FUNCTION, xdefault))
    return;

  if (obj != NULL)
    {
      // source or disassembly for <Total>, <Unknown>, or @plt
      if (obj->get_type () != Histable::FUNCTION)
	{
	  fprintf (stderr,
		   GTXT ("Error: %s is not a real function; no source or disassembly available.\n"),
		   name);
	  return;
	}

      func = (Function *) obj;
      if (func->flags & FUNC_FLAG_SIMULATED)
	{
	  fprintf (stderr,
		   GTXT ("Error: %s is not a real function; no source or disassembly available.\n"),
		   name);
	  return;
	}
      else if (dbev != NULL && isDisasm)
	dbev->set_func_scope (true);

      // function found, set module
      module = func->module;
      int ix = module->loadobject->seg_idx;
      if (dbev->get_lo_expand (ix) == LIBEX_HIDE)
	{
	  char *lo_name = module->loadobject->get_name ();
	  fprintf (stderr,
		   GTXT ("Error: No source or disassembly available for hidden object %s.\n"),
		   lo_name);
	  return;
	}

      if (srcFile)
	{
	  Vector<SourceFile*> *sources = func->get_sources ();
	  bool found = false;
	  if (sources == NULL)
	    {
	      fitem = func->getDefSrc ();
	      found = (func->line_first > 0)
		      && strcmp (get_basename (srcFile),
				 get_basename (fitem->get_name ())) == 0;
	    }
	  else
	    {
	      Vec_loop (SourceFile*, sources, index, fitem)
	      {
		if (strcmp (get_basename (srcFile), get_basename (fitem->get_name ())) == 0)
		  {
		    found = true;
		    break;
		  }
	      }
	    }
	  if (!found)
	    {
	      fprintf (stderr, GTXT ("Error: Source file context %s does not contribute to function `%s'.\n"),
		       srcFile, name);
	      return;
	    }
	}
    }
  else
    {
      // function not found
      if (sel && strrchr (sel, ':'))
	{
	  // 'sel' was "@seg_num:address" or "file_name:address"
	  fprintf (stderr,
		   GTXT ("Error: No function with given name `%s %s' found.\n"),
		   name, sel);
	  return;
	}
      // search for a file of that name
      if (!dbeSession->find_obj (dis_file, inp_file, obj, name, sel,
				 Histable::MODULE, xdefault))
	return;

      if (obj == NULL)
	{ // neither function nor file found
	  fprintf (stderr, GTXT ("Error: No function or file with given name `%s' found.\n"),
		   name);
	  return;
	}

      func = NULL;
      module = (Module *) obj;
      int ix = module->loadobject->seg_idx;
      if (dbev->get_lo_expand (ix) == LIBEX_HIDE)
	{
	  char *lo_name = module->loadobject->get_name ();
	  fprintf (stderr, GTXT ("Error: No source or disassembly available for hidden object %s.\n"),
		   lo_name);
	  return;
	}
      if (name)
	srcFile = name;
    }

  if (module == NULL || module->get_name () == NULL)
    {
      fprintf (stderr, GTXT ("Error: Object name not recorded in experiment\n"));
      return;
    }
  module->read_stabs ();

  if (!isDisasm && (module->file_name == NULL
		    || (module->flags & MOD_FLAG_UNKNOWN) != 0
		    || *module->file_name == 0))
    {
      fprintf (stderr, GTXT ("Error: Source location not recorded in experiment\n"));
      return;
    }

  MetricList *metric_list = dbev->get_metric_list (MET_NORMAL);
  int sort_ref_index = metric_list->get_sort_ref_index ();
  if (isDisasm)
    metric_list->set_sort_ref_index (-1);

  // Ask DbeView to generate function-level data
  //	MSI: I think this is used only to get totals to compute percentages
  hist_data = dbev->get_hist_data (metric_list, Histable::FUNCTION, 0,
				   Hist_data::ALL);
  MetricList *nmlist = hist_data->get_metric_list ();
  metric_list->set_sort_ref_index (sort_ref_index);
  if (nmlist->get_items ()->size () != 0
      && hist_data->get_status () != Hist_data::SUCCESS)
    {
      errstr = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
      if (errstr)
	{
	  fprintf (stderr, GTXT ("Error: %s\n"), errstr);
	  free (errstr);
	}
      return;
    }

  marks = new Vector<int>;
  if (isDisasm)
    {
      threshold = dbev->get_thresh_dis ();
      compcom_bits = dbev->get_dis_compcom ();
      src_visible = dbev->get_src_visible ();
      hex_visible = dbev->get_hex_visible ();
      srcmetrics_visible = dbev->get_srcmetric_visible ();
    }
  else
    {
      threshold = dbev->get_thresh_src ();
      compcom_bits = dbev->get_src_compcom ();
      src_visible = SRC_NA;
      hex_visible = false;
      srcmetrics_visible = false;
    }

  dump_anno_file (out_file, isDisasm ? Histable::INSTR : Histable::LINE,
		  module, dbev, nmlist, hist_data->get_totals ()->value,
		  srcFile, func, marks, threshold, compcom_bits,
		  src_visible, hex_visible, srcmetrics_visible);

  delete marks;

  errstr = module->anno_str ();
  if (errstr)
    {
      fprintf (stderr, GTXT ("Error: %s\n"), errstr);
      free (errstr);
    }
  delete hist_data;
}

void
print_html_title (FILE *out_file, char *title)
{
  // This will print a header row for the report
  fprintf (out_file, "<html><title>%s</title>\n", title);
  fprintf (out_file, "<center><h3>%s</h3></center>\n", title);
}

void
print_html_label (FILE *out_file, MetricList *metrics_list)
{
  int mlist_sz;

  // This will print  a header row for the metrics
  Vector<Metric*> *mlist = metrics_list->get_items ();
  mlist_sz = mlist->size ();

  fprintf (out_file, "<style type=\"text/css\">\n");
  fprintf (out_file, "<!--\nBODY\n");
  fprintf (out_file, ".th_C   { text-align:center; background-color:lightgoldenrodyellow; }\n");
  fprintf (out_file, ".th_CG  { text-align:center; background-color:#ffff33; }\n");
  fprintf (out_file, ".th_L   { text-align:left; background-color:lightgoldenrodyellow; }\n");
  fprintf (out_file, ".th_LG  { text-align:left; background-color:#ffff33; }\n");
  fprintf (out_file, ".td_R   { text-align:right;  }\n");
  fprintf (out_file, ".td_RG  { text-align:right; background-color:#ffff33; }\n");
  fprintf (out_file, ".td_L   { text-align:left; }\n");
  fprintf (out_file, ".td_LG  { text-align:left;  background-color:#ffff33; }\n");
  fprintf (out_file, "-->\n</style>");
  fprintf (out_file, "<center><table border=1 cellspacing=2>\n<tr>");

  for (int index = 0; index < mlist_sz; index++)
    {
      Metric *mitem = mlist->fetch (index);
      int ncols = 0;
      if (mitem->is_visible ())
	ncols++;
      if (mitem->is_tvisible ())
	ncols++;
      if (mitem->is_pvisible ())
	ncols++;
      if (ncols == 0)
	continue;
      char *name = strdup (mitem->get_name ());
      char *name2 = split_metric_name (name);
      const char *style = index == metrics_list->get_sort_ref_index () ? "G" : "";

      // start the column, with colspan setting, legend, and sort metric indicator
      if (ncols == 1)
	{
	  if (mitem->get_vtype () == VT_LABEL)
	    // left-adjust the name metric
	    fprintf (out_file,
		     "<th class=\"th_L%s\">%s&nbsp;<br>%s&nbsp;%s&nbsp;<br>%s&nbsp;</th>",
		     style, mitem->legend == NULL ? "&nbsp;" : mitem->legend,
		     (index == metrics_list->get_sort_ref_index ()) ? "&nabla;" : "&nbsp;",
		     name, name2 == NULL ? "&nbsp;" : name2);
	  else
	    // but center the others
	    fprintf (out_file,
		     "<th class=\"th_C%s\">%s&nbsp;<br>%s&nbsp;%s&nbsp;<br>%s&nbsp;</th>",
		     style, mitem->legend == NULL ? "&nbsp;" : mitem->legend,
		     (index == metrics_list->get_sort_ref_index ()) ?
			 "&nabla;" : "&nbsp;",
		     name, name2 == NULL ? NTXT ("&nbsp;") : name2);
	}
      else
	// name metric can't span columns
	fprintf (out_file,
		 "<th colspan=%d class=\"th_C%s\">%s&nbsp;<br>%s&nbsp;%s&nbsp;<br>%s&nbsp;</th>",
		 ncols, style,
		 mitem->legend == NULL ? "&nbsp;" : mitem->legend,
		 index == metrics_list->get_sort_ref_index () ?
		     "&nabla;" : "&nbsp;",
		 name, name2 == NULL ? "&nbsp;" : name2);

      free (name);
    }

  // end this row, start the units row
  fprintf (out_file, NTXT ("</tr>\n<tr>"));

  // now do the units row
  for (int index = 0; index < mlist_sz; index++)
    {
      Metric *mitem = mlist->fetch (index);
      const char *style = index == metrics_list->get_sort_ref_index () ? "G" : "";

      if (mitem->is_tvisible ())
	fprintf (out_file, "<th class=\"th_C%s\">&nbsp;(%s)</th>", style,
		 GTXT ("sec."));
      if (mitem->is_visible ())
	{
	  if (mitem->get_abbr_unit () == NULL)
	    fprintf (out_file, "<th class=\"th_C%s\">&nbsp;</th>", style);
	  else
	    fprintf (out_file, "<th class=\"th_C%s\">(%s)</th>", style,
		     mitem->get_abbr_unit () == NULL ? "&nbsp;"
		     : mitem->get_abbr_unit ());
	}
      if (mitem->is_pvisible ())
	fprintf (out_file, "<th class=\"th_C%s\">&nbsp;(%%)</th>", style);
    }
  fprintf (out_file, NTXT ("</tr>\n"));
}

void
print_html_content (FILE *out_file, Hist_data *data, MetricList *metrics_list,
		    int limit, Histable::NameFormat nfmt)
{
  Hist_data::HistItem *item;

  // printing contents.
  for (int i = 0; i < limit; i++)
    {
      item = data->fetch (i);
      print_html_one (out_file, data, item, metrics_list, nfmt);
    }
}

void
print_html_one (FILE *out_file, Hist_data *data, Hist_data::HistItem *item,
		MetricList *metrics_list, Histable::NameFormat nfmt)
{
  Metric *mitem;
  int index;
  int visible, tvisible, pvisible;
  TValue *value;
  double percent;

  fprintf (out_file, NTXT ("<tr>"));
  Vec_loop (Metric*, metrics_list->get_items (), index, mitem)
  {
    visible = mitem->is_visible ();
    tvisible = mitem->is_tvisible ();
    pvisible = mitem->is_pvisible ();
    const char *style = index == metrics_list->get_sort_ref_index () ? "G" : "";

    if (tvisible)
      {
	value = &(item->value[index]);
	if (value->ll == 0LL)
	  fprintf (out_file,
		   "<td class=\"td_R%s\"><tt>0.&nbsp;&nbsp;&nbsp;</tt></td>",
		   style);
	else
	  fprintf (out_file, "<td class=\"td_R%s\"><tt>%4.3lf</tt></td>",
		   style, 1.e-6 * value->ll / dbeSession->get_clock (-1));
      }

    if (visible)
      {
	if (mitem->get_vtype () == VT_LABEL)
	  {
	    value = &(item->value[index]);
	    char *r;
	    if (value->tag == VT_OFFSET)
	      r = ((DataObject*) (item->obj))->get_offset_name ();
	    else
	      r = item->obj->get_name (nfmt);
	    char *n = html_ize_name (r);
	    fprintf (out_file, NTXT ("<td class=\"td_L%s\">%s</td>"), style, n);
	    free (n);
	  }
	else
	  {
	    value = &(item->value[index]);
	    switch (value->tag)
	      {
	      case VT_DOUBLE:
		if (value->d == 0.0)
		  fprintf (out_file,
		      "<td class=\"td_R%s\"><tt>0.&nbsp;&nbsp;&nbsp;</tt></td>",
			   style);
		else
		  fprintf (out_file,
			   "<td  class=\"td_R%s\"><tt>%4.3lf</tt></td>", style,
			   value->d);
		break;
	      case VT_INT:
		fprintf (out_file, "<td  class=\"td_R%s\"><tt>%d</tt></td>",
			 style, value->i);
		break;
	      case VT_LLONG:
		fprintf (out_file, "<td  class=\"td_R%s\"><tt>%lld</td></tt>",
			 style, value->ll);
		break;
	      case VT_ULLONG:
		fprintf (out_file, "<td  class=\"td_R%s\"><tt>%llu</td></tt>",
			 style, value->ull);
		break;
	      case VT_ADDRESS:
		fprintf (out_file,
			 "<td  class=\"td_R%s\"><tt>%u:0x%08x</tt></td>", style,
			 ADDRESS_SEG (value->ll), ADDRESS_OFF (value->ll));
		break;
	      case VT_FLOAT:
		if (value->f == 0.0)
		  fprintf (out_file,
			   "<td  class=\"td_R%s\"><tt>0.&nbsp;&nbsp;&nbsp;</tt></td>",
			   style);
		else
		  fprintf (out_file,
			   "<td  class=\"td_R%s\"><tt>%4.3f</tt></td>",
			   style, value->f);
		break;
	      case VT_SHORT:
		fprintf (out_file, "<td  class=\"td_R%s\"><tt>%d</tt></td>",
			 style, value->s);
		break;
		// ignoring the following cases (why?)
	      case VT_HRTIME:
	      case VT_LABEL:
	      case VT_OFFSET:
		break;
	      }
	  }
      }

    if (pvisible)
      {
	percent = data->get_percentage (item->value[index].to_double (), index);
	if (percent == 0.0)
	  // adjust to change format from xx.yy%
	  fprintf (out_file, "<td class=\"td_R%s\">0.&nbsp;&nbsp;&nbsp;</td>",
		   style);
	else
	  // adjust format below to change format from xx.yy%
	  fprintf (out_file, "<td class=\"td_R%s\">%3.2f</td>", style,
		   (100.0 * percent));
      }
  }
  fprintf (out_file, NTXT ("</tr>\n"));
}

void
print_html_trailer (FILE *out_file)
{
  fprintf (out_file, NTXT ("</table></center></html>\n"));
}

static char *
del_delim (char *s)
{
  size_t len = strlen (s);
  if (len > 0)
    s[len - 1] = 0;
  return s;
}

void
print_delim_label (FILE *out_file, MetricList *metrics_list, char delim)
{
  char line0[2 * MAX_LEN], line1[2 * MAX_LEN];
  char line2[2 * MAX_LEN], line3[2 * MAX_LEN];
  size_t len;

  // This will print four header rows for the metrics
  line0[0] = 0;
  line1[0] = 0;
  line2[0] = 0;
  line3[0] = 0;
  Vector<Metric*> *mlist = metrics_list->get_items ();
  for (int index = 0, mlist_sz = mlist->size (); index < mlist_sz; index++)
    {
      Metric *mitem = mlist->fetch (index);
      if (!(mitem->is_visible () || mitem->is_tvisible ()
	    || mitem->is_pvisible ()))
	continue;
      char *name = strdup (mitem->get_name ());
      char *name2 = split_metric_name (name);

      if (mitem->is_tvisible ())
	{
	  len = strlen (line0);
	  snprintf (line0 + len, sizeof (line0) - len, NTXT ("\"%s\"%c"),
		    mitem->legend == NULL ? NTXT ("") : mitem->legend, delim);
	  len = strlen (line1);
	  snprintf (line1 + len, sizeof (line1) - len, NTXT ("\"%s\"%c"),
		    name, delim);
	  len = strlen (line2);
	  snprintf (line2 + len, sizeof (line2) - len, NTXT ("\"%s\"%c"),
		    name2 == NULL ? NTXT ("") : name2, delim);
	  len = strlen (line3);
	  if (index == metrics_list->get_sort_ref_index ())
	    snprintf (line3 + len, sizeof (line3) - len, NTXT ("\"V  %s\"%c"),
		      GTXT ("(sec.)"), delim);
	  else
	    snprintf (line3 + len, sizeof (line3) - len, NTXT ("\"   %s\"%c"),
		      GTXT ("(sec.)"), delim);
	}
      if (mitem->is_visible ())
	{
	  len = strlen (line0);
	  snprintf (line0 + len, sizeof (line0) - len, "\"%s\"%c",
		    mitem->legend == NULL ? "" : mitem->legend, delim);

	  len = strlen (line1);
	  snprintf (line1 + len, sizeof (line1) - len, "\"%s\"%c",
		    name, delim);

	  len = strlen (line2);
	  snprintf (line2 + len, sizeof (line2) - len, "\"%s\"%c",
		    name2 == NULL ? NTXT ("") : name2, delim);

	  len = strlen (line3);
	  char *au = mitem->get_abbr_unit ();

	  if (index == metrics_list->get_sort_ref_index ())
	    {
	      if (au == NULL)
		snprintf (line3 + len, sizeof (line3) - len, "\"V  \"%c", delim);
	      else
		snprintf (line3 + len, sizeof (line3) - len, "\"V  (%s)\"%c",
			    au, delim);
	    }
	  else
	    {
	      if (au == NULL)
		snprintf (line3 + len, sizeof (line3) - len, "\"   \"%c",
			  delim);
	      else
		snprintf (line3 + len, sizeof (line3) - len, "\"   (%s)\"%c",
			  au, delim);
	    }
	}
      if (mitem->is_pvisible ())
	{
	  len = strlen (line0);
	  snprintf (line0 + len, sizeof (line0) - len, NTXT ("\"%s\"%c"),
		    mitem->legend == NULL ? NTXT ("") : mitem->legend, delim);

	  len = strlen (line1);
	  snprintf (line1 + len, sizeof (line1) - len, NTXT ("\"%s\"%c"),
		    name, delim);

	  len = strlen (line2);
	  snprintf (line2 + len, sizeof (line2) - len, NTXT ("\"%s\"%c"),
		    name2 == NULL ? NTXT ("") : name2, delim);

	  len = strlen (line3);
	  if (index == metrics_list->get_sort_ref_index ())
	    snprintf (line3 + len, sizeof (line3) - len, NTXT ("\"V  %s\"%c"),
		      NTXT ("%%"), delim);
	  else
	    snprintf (line3 + len, sizeof (line3) - len, NTXT ("\"   %s\"%c"),
		      NTXT ("%%"), delim);
	}
      free (name);
    }
  // now remove the trailing delimiter, and print the four lines
  fprintf (out_file, NTXT ("%s\n"), del_delim (line0));
  fprintf (out_file, NTXT ("%s\n"), del_delim (line1));
  fprintf (out_file, NTXT ("%s\n"), del_delim (line2));
  fprintf (out_file, NTXT ("%s\n"), del_delim (line3));
}

void
print_delim_content (FILE *out_file, Hist_data *data, MetricList *metrics_list,
		     int limit, Histable::NameFormat nfmt, char delim)
{
  Hist_data::HistItem *item;
  int i;

  // printing contents.
  for (i = 0; i < limit; i++)
    {
      item = data->fetch (i);
      print_delim_one (out_file, data, item, metrics_list, nfmt, delim);
    }
}

void
print_delim_trailer (FILE */*out_file*/, char /*delim*/) { }

// EUGENE does this function work properly when "-compare ratio" is used?
//   how about when the ratio is nonzero-divided-by-zero?
// EUGENE actually, review this entire file

void
print_delim_one (FILE *out_file, Hist_data *data, Hist_data::HistItem *item,
		 MetricList *metrics_list, Histable::NameFormat nfmt,
		 char delim)
{
  Metric *mitem;
  int index;
  int visible, tvisible, pvisible;
  TValue *value;
  double percent;
  size_t len;

  char line1[2 * MAX_LEN];
  *line1 = 0;
  Vec_loop (Metric*, metrics_list->get_items (), index, mitem)
  {
    visible = mitem->is_visible ();
    tvisible = mitem->is_tvisible ();
    pvisible = mitem->is_pvisible ();
    if (tvisible)
      {
	value = &(item->value[index]);
	len = strlen (line1);
	if (value->ll == 0LL)
	  snprintf (line1 + len, sizeof (line1) - len, "\"0.\"%c", delim);
	else
	  snprintf (line1 + len, sizeof (line1) - len, "\"%4.3lf\"%c",
		    1.e-6 * value->ll / dbeSession->get_clock (-1),
		    delim);
      }

    if (visible)
      {
	len = strlen (line1);
	if (mitem->get_vtype () == VT_LABEL)
	  {
	    value = &(item->value[index]);
	    char *r;
	    if (value->tag == VT_OFFSET)
	      r = ((DataObject*) (item->obj))->get_offset_name ();
	    else
	      r = item->obj->get_name (nfmt);
	    char *p = csv_ize_name (r, delim);
	    snprintf (line1 + len, sizeof (line1) - len, "\"%s\"%c", p, delim);
	    free (p);
	  }
	else
	  {
	    value = &(item->value[index]);
	    switch (value->tag)
	      {
	      case VT_DOUBLE:
		if (value->d == 0.0)
		  snprintf (line1 + len, sizeof (line1) - len, "\"0.\"%c",
			    delim);
		else
		  snprintf (line1 + len, sizeof (line1) - len, "\"%4.3lf\"%c",
			    value->d, delim);
		break;
	      case VT_INT:
		snprintf (line1 + len, sizeof (line1) - len, "\"%d\"%c",
			  value->i, delim);
		break;
	      case VT_LLONG:
		snprintf (line1 + len, sizeof (line1) - len, "\"%lld\"%c",
			  value->ll, delim);
		break;
	      case VT_ULLONG:
		snprintf (line1 + len, sizeof (line1) - len, "\"%llu\"%c",
			  value->ull, delim);
		break;
	      case VT_ADDRESS:
		snprintf (line1 + len, sizeof (line1) - len, "\"%u:0x%08x\"%c",
			  ADDRESS_SEG (value->ll),
			  ADDRESS_OFF (value->ll), delim);
		break;
	      case VT_FLOAT:
		if (value->f == 0.0)
		  snprintf (line1 + len, sizeof (line1) - len, "\"0.\"%c",
			    delim);
		else
		  snprintf (line1 + len, sizeof (line1) - len, "\"%4.3f\"%c",
			    value->f, delim);
		break;
	      case VT_SHORT:
		snprintf (line1 + len, sizeof (line1) - len, "\"%d\"%c",
			  value->s, delim);
		break;
		// ignoring the following cases (why?)
	      case VT_HRTIME:
	      case VT_LABEL:
	      case VT_OFFSET:
		break;
	      }
	  }
      }

    if (pvisible)
      {
	len = strlen (line1);
	percent = data->get_percentage (item->value[index].to_double (), index);
	if (percent == 0.0)
	  // adjust to change format from xx.yy%
	  snprintf (line1 + len, sizeof (line1) - len, "\"0.\"%c", delim);
	else
	  // adjust format below to change format from xx.yy%
	  snprintf (line1 + len, sizeof (line1) - len, "\"%3.2f\"%c",
		    (100.0 * percent), delim);
      }
  }
  fprintf (out_file, NTXT ("%s\n"), del_delim (line1));
}

char *
html_ize_name (char *name)
{
  StringBuilder sb;
  for (size_t i = 0; i < strlen (name); i++)
    {
      switch (name[i])
	{
	case ' ': sb.append (NTXT ("&nbsp;"));
	  break;
	case '"': sb.append (NTXT ("&quot;"));
	  break;
	case '&': sb.append (NTXT ("&amp;"));
	  break;
	case '<': sb.append (NTXT ("&lt;"));
	  break;
	case '>': sb.append (NTXT ("&gt;"));
	  break;
	default: sb.append (name[i]);
	  break;
	}
    }
  char *ret = sb.toString ();
  return ret;
}

char *
csv_ize_name (char *name, char /*delim*/)
{
  StringBuilder sb;
  for (size_t i = 0; i < strlen (name); i++)
    sb.append (name[i]);
  char *ret = sb.toString ();
  return ret;
}

// Split a metric name into two parts, replacing a blank with
//	a zero and returning pointer to the rest of the string, or
//	leaving the string unchanged, and returning NULL;

char *
split_metric_name (char *name)
{
  // figure out the most even split of the name
  size_t len = strlen (name);
  char *middle = &name[len / 2];

  // find the first blank
  char *first = strchr (name, (int) ' ');
  if (first == NULL)  // no blanks
    return NULL;
  char *last = first;
  char *p = first;
  for (;;)
    {
      p = strchr (p + 1, (int) ' ');
      if (p == NULL)
	break;
      if (p < middle)
	{
	  first = p;
	  last = p;
	}
      else
	{
	  last = p;
	  break;
	}
    }
  // pick the better of the two
  char *ret;
  int f = (int) (middle - first);
  int l = (int) (last - middle);
  if ((first == last) || (f <= l))
    {
      *first = '\0';
      ret = first + 1;
    }
  else
    {
      *last = '\0';
      ret = last + 1;
    }
  return ret;
}
