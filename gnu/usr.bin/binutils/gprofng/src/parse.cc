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
#include <sys/param.h>
#include <sys/mman.h>

#include "util.h"
#include "DbeFile.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "Emsg.h"
#include "Function.h"
#include "LoadObject.h"
#include "Module.h"
#include "PRBTree.h"
#include "Sample.h"
#include "Elf.h"
#include "StringBuilder.h"

void
Experiment::mrec_insert (MapRecord *mrec)
{
  int sz = mrecs->size ();
  MapRecord *tmp = sz > 0 ? mrecs->fetch (sz - 1) : NULL;

  // The following should work in most cases
  if (tmp == NULL || tmp->ts <= mrec->ts)
    {
      mrecs->append (mrec);
      return;
    }

  // If it didn't...
  int lo = 0;
  int hi = sz - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      tmp = mrecs->fetch (md);
      if (tmp->ts < mrec->ts)
	lo = md + 1;
      else
	hi = md - 1;
    }
  mrecs->insert (lo, mrec);
}

int
Experiment::process_arglist_cmd (char *, char *arglist)
{
  uarglist = arglist;

  // find argv[0], and extract its basename
  if (strcmp (uarglist, NTXT ("(fork)")) == 0)
    return 0; // leaving target name NULL
  char *p = uarglist;
  char *pp = uarglist;
  char *pl;
  for (;;)
    {
      if (*p == '/')
	pp = p + 1;
      if (*p == ' ' || *p == 0)
	{
	  pl = p;
	  break;
	}
      p++;
    }
  size_t len = pl - pp;
  if (len > 0)
    utargname = dbe_sprintf (NTXT ("%.*s"), (int) len, pp);
  return 0;
}

int
Experiment::process_desc_start_cmd (char *, hrtime_t ts, char *flavor,
				    char *nexp, int follow, char *txt)
{
  char *str;
  Emsg *m;

  if (follow == 1)
    str = dbe_sprintf (GTXT ("Starting %s %ld.%09ld, exp %s.er, \"%s\""),
		       flavor, (long) (ts / NANOSEC), (long) (ts % NANOSEC),
		       nexp, txt);
  else
    str = dbe_sprintf (GTXT ("Starting %s %ld.%09ld, no experiment, \"%s\""),
		       flavor, (long) (ts / NANOSEC), (long) (ts % NANOSEC),
		       txt);
  m = new Emsg (CMSG_COMMENT, str);
  free (str);
  runlogq->append (m);

  free (flavor);
  free (nexp);
  free (txt);
  return 0;
}

int
Experiment::process_desc_started_cmd (char *, hrtime_t ts, char *flavor,
				      char *nexp, int follow, char *txt)
{
  char *str;
  Emsg *m;

  if (follow == 1)
    str = dbe_sprintf (GTXT ("Started  %s %ld.%09ld, exp %s.er, \"%s\""),
		       flavor, (long) (ts / NANOSEC), (long) (ts % NANOSEC),
		       nexp, txt);
  else
    str = dbe_sprintf (GTXT ("Started  %s %ld.%09ld, no experiment, \"%s\""),
		       flavor, (long) (ts / NANOSEC), (long) (ts % NANOSEC),
		       txt);
  m = new Emsg (CMSG_COMMENT, str);
  free (str);
  runlogq->append (m);
  free (flavor);
  free (nexp);
  free (txt);
  return 0;
}

LoadObject *
Experiment::get_dynfunc_lo (const char *loName)
{
  LoadObject *lo = loadObjMap->get (loName);
  if (lo == NULL)
    {
      lo = createLoadObject (loName, expIdx);// DYNFUNC_SEGMENT is always unique
      lo->dbeFile->filetype |= DbeFile::F_FICTION;
      lo->flags |= SEG_FLAG_DYNAMIC;
      lo->type = LoadObject::SEG_TEXT;
      lo->set_platform (platform, wsize);
      append (lo);
    }
  return lo;
}

Function *
Experiment::create_dynfunc (Module *mod, char *fname, int64_t vaddr,
			    int64_t fsize)
{
  Function *f = dbeSession->createFunction ();
  f->set_name (fname);
  f->flags |= FUNC_FLAG_DYNAMIC;
  f->size = fsize;
  f->img_offset = vaddr;
  f->module = mod;
  mod->functions->append (f);
  mod->loadobject->functions->append (f);
  return f;
}

static int
func_cmp (const void *a, const void *b)
{
  Function *fp1 = *((Function **) a);
  Function *fp2 = *((Function **) b);
  uint64_t i1 = fp1->img_offset;
  uint64_t i2 = fp2->img_offset;
  return i1 < i2 ? -1 : i1 == i2 ? 0 : 1;
}

int
Experiment::process_fn_load_cmd (Module *mod, char *fname, Vaddr vaddr,
				 int fsize, hrtime_t ts)
{
  Dprintf (DEBUG_MAPS,
	   "process_fn_load_cmd:%s (%s) vaddr=0x%llx msize=%lld ts=%lld\n",
	   STR (mod ? mod->get_name () : NULL), STR (fname),
	   (unsigned long long) vaddr, (long long) fsize, (long long) ts);
  if (mod != NULL)
    {
      mod->functions->sort (func_cmp);
      uint64_t lastVaddr = vaddr;
      for (int i = 0, sz = mod->functions->size (); i < sz; i++)
	{
	  Function *f = mod->functions->fetch (i);
	  if (lastVaddr < f->img_offset)
	    {
	      char *fnm = dbe_sprintf (GTXT ("<static>@0x%llx (%s)"),
				       (unsigned long long) lastVaddr, fname);
	      create_dynfunc (mod, fnm, lastVaddr, f->img_offset - lastVaddr);
	      free (fnm);
	    }
	  lastVaddr = f->img_offset + f->size;
	}
      if (lastVaddr < vaddr + fsize)
	{
	  char *fnm = dbe_sprintf (GTXT ("<static>@0x%llx (%s)"),
				   (unsigned long long) lastVaddr, fname);
	  create_dynfunc (mod, fnm, lastVaddr, vaddr + fsize - lastVaddr);
	  free (fnm);
	}
      mod->functions->sort (func_cmp);
      for (int i = 0, sz = mod->functions->size (); i < sz; i++)
	{
	  Function *f = mod->functions->fetch (i);
	  MapRecord *mrec = new MapRecord;
	  mrec->kind = MapRecord::LOAD;
	  mrec->obj = f;
	  mrec->base = f->img_offset;
	  mrec->size = f->size;
	  mrec->ts = ts;
	  mrec->foff = 0;
	  mrec_insert (mrec);
	}
      return 0;
    }

  LoadObject *ds = get_dynfunc_lo (DYNFUNC_SEGMENT);
  Function *dfunc = create_dynfunc (ds->noname, fname, vaddr, fsize);

  // check for special functions, USER, IDLE, TRUNC to disable offsets in disassembly
  // XXX -- check based on name now
  // Optimization: use pre-initialized localized strings
  static const char * localized_USER_MODE = NULL;
  static const char * localized_IDLE = NULL;
  static const char * localized_TRUNCATED_STACK = NULL;
  if (localized_USER_MODE == NULL)
    {
      localized_USER_MODE = GTXT ("<USER_MODE>");
      localized_IDLE = GTXT ("<IDLE>");
      localized_TRUNCATED_STACK = GTXT ("<TRUNCATED_STACK>");
    }
  if (strcmp (fname, localized_USER_MODE) == 0
      || strcmp (fname, localized_IDLE) == 0
      || strcmp (fname, localized_TRUNCATED_STACK) == 0)
    dfunc->flags |= FUNC_FLAG_NO_OFFSET;

  MapRecord *mrec = new MapRecord;
  mrec->kind = MapRecord::LOAD;
  mrec->obj = dfunc;
  mrec->base = vaddr;
  mrec->size = fsize;
  mrec->ts = ts;
  mrec->foff = 0;
  mrec_insert (mrec);
  return 0;
}

int
Experiment::process_fn_unload_cmd (char *, Vaddr vaddr, hrtime_t ts)
{
  MapRecord *mrec = new MapRecord;
  mrec->kind = MapRecord::UNLOAD;
  mrec->base = vaddr;
  mrec->ts = ts;
  mrec_insert (mrec);
  return 0;
}

void
Experiment::register_metric (Metric::Type type)
{
  BaseMetric *mtr = dbeSession->register_metric (type);
  metrics->append (mtr);
}

void
Experiment::register_metric (Hwcentry *ctr, const char* aux, const char* uname)
{
  BaseMetric *mtr = dbeSession->register_metric (ctr, aux, uname);
  metrics->append (mtr);
  if (mtr->get_dependent_bm ())
    metrics->append (mtr->get_dependent_bm ());
}

int
Experiment::process_hwcounter_cmd (char *, int cpuver, char *counter,
				   char * int_name, int interval, int tag,
				   int i_tpc, char *modstr)
{
  char *str;
  Emsg *m;
  Hwcentry *ctr;
  ABST_type tpc = (ABST_type) i_tpc;

  // Use previously ignored tag to associate counter packets.
  if (tag < 0 || tag >= MAX_HWCOUNT)
    {
      // invalid tag specified, warn user
      str = dbe_sprintf (GTXT ("*** Error: HW counter tag %d out of range [%d - %d]; ignored"),
			 tag, 0, MAX_HWCOUNT - 1);
      m = new Emsg (CMSG_ERROR, str);
      free (str);
      errorq->append (m);
      free (counter);
      return 0;
    }
  if (coll_params.hw_aux_name[tag])
    {
      // duplicate tag used, warn user
      str = dbe_sprintf (GTXT ("*** Error: Duplicate HW counter tag %d specified; ignored"),
			 tag);
      m = new Emsg (CMSG_ERROR, str);
      free (str);
      errorq->append (m);
      free (counter);
      return 0;
    }
  hw_cpuver = cpuver;

  // map it to a machinemodel string
  if (hw_cpuver != CPUVER_UNDEFINED)
    {
      free (machinemodel);
      if (hw_cpuver == 1104)
	machinemodel = dbe_strdup (NTXT ("t4"));
      else if (hw_cpuver == 1110)
	machinemodel = dbe_strdup (NTXT ("t5"));
      else if (hw_cpuver == 1204)
	machinemodel = dbe_strdup (NTXT ("m4"));
      else if (hw_cpuver == 1210)
	machinemodel = dbe_strdup (NTXT ("m5"));
      else if (hw_cpuver == 1220)
	machinemodel = dbe_strdup (NTXT ("m6"));
      else if (hw_cpuver == 1230)
	machinemodel = dbe_strdup (NTXT ("m7"));
      else
	machinemodel = dbe_strdup (NTXT ("generic"));
    }

  // Find the entry in the machine table, and dup it
  ctr = new Hwcentry;
  dbeSession->append (ctr);
  hwc_post_lookup (ctr, counter, int_name, cpuver);
  ctr->sort_order = tag;
  ctr->memop = tpc;

  // Check if HWC name is to be modified
  if (modstr != NULL)
    {
      char *s = ctr->name;
      ctr->name = dbe_sprintf (NTXT ("%s%s"), modstr, s);
      s = ctr->int_name;
      ctr->int_name = dbe_sprintf (NTXT ("%s%s"), modstr, s);
      s = ctr->metric;
      if (s)
	ctr->metric = dbe_sprintf (NTXT ("%s%s"), modstr, s);
    }

  char * cname = dbe_strdup (ctr->name);
  char * uname = dbe_strdup (hwc_i18n_metric (ctr));
  coll_params.hw_aux_name[tag] = cname;
  coll_params.hw_username[tag] = uname;
  coll_params.hw_interval[tag] = interval;
  coll_params.hw_tpc[tag] = tpc;
  coll_params.hw_cpu_ver[tag] = cpuver;

  // set hw_mode and xhw_mode?
  coll_params.hw_mode = 1;
  if (ABST_MEMSPACE_ENABLED (tpc))
    {
      // yes, dataspace data available
      coll_params.xhw_mode = 1;

      // set dataspace available
      dataspaceavail = true;
    }
  register_metric (ctr, cname, uname);
  free (counter);
  return 0;
}

// TBR:?

int
Experiment::process_hwsimctr_cmd (char *, int cpuver, char *nm, char *int_name,
				  char *metric, int reg,
				  int interval, int timecvt, int i_tpc, int tag)
{
  char *str;
  Emsg *m;
  Hwcentry *ctr;
  ABST_type tpc = (ABST_type) i_tpc;

  // Use previously ignored tag to associate counter packets.
  if (tag < 0 || tag >= MAX_HWCOUNT)
    {
      // invalid tag specified, warn user
      str = dbe_sprintf (GTXT ("*** Error: HW counter tag %d out of range [%d - %d]; ignored"),
			 tag, 0, MAX_HWCOUNT - 1);
      m = new Emsg (CMSG_ERROR, str);
      free (str);
      errorq->append (m);

      free (nm);
      free (int_name);
      free (metric);
      return 0;
    }
  if (coll_params.hw_aux_name[tag])
    {
      // duplicate tag used, warn user
      str = dbe_sprintf (GTXT ("*** Error: Duplicate HW counter tag %d specified; ignored"),
			 tag);
      m = new Emsg (CMSG_ERROR, str);
      free (str);
      errorq->append (m);
      free (nm);
      free (int_name);
      free (metric);
      return 0;
    }
  hw_cpuver = cpuver;
  ctr = new Hwcentry;
  {
    static Hwcentry empty;
    *ctr = empty;
  }
  ctr->name = nm;
  ctr->int_name = int_name;
  ctr->metric = metric;
  ctr->reg_num = reg;
  ctr->val = interval;
  ctr->timecvt = timecvt;
  ctr->memop = tpc;
  ctr->sort_order = tag;

  char *cname = dbe_strdup (ctr->name);
  char *uname = dbe_strdup (hwc_i18n_metric (ctr));

  coll_params.hw_aux_name[tag] = cname;
  coll_params.hw_username[tag] = uname;
  coll_params.hw_interval[tag] = interval;
  coll_params.hw_tpc[tag] = tpc;
  coll_params.hw_cpu_ver[tag] = cpuver;

  // set hw_mode and xhw_mode?
  coll_params.hw_mode = 1;
  if (ABST_MEMSPACE_ENABLED (tpc))
    {
      coll_params.xhw_mode = 1;
      // set dataspace available
      if (getenv ("ANALYZER_DATASPACE_COUNT") != 0)
	dataspaceavail = true;
    }

  register_metric (ctr, cname, uname);
  return 0;
}

int
Experiment::process_jcm_load_cmd (char *, Vaddr mid, Vaddr vaddr,
				  int msize, hrtime_t ts)
{
  if (jmaps == NULL)
    return 1;

  JMethod *jfunc = (JMethod*) jmaps->locate_exact_match (mid, ts);
  if (jfunc == NULL || jfunc->get_type () != Histable::FUNCTION)
    return 1;

  LoadObject *ds = get_dynfunc_lo (JAVA_COMPILED_METHODS);
  Module *jmodule = jfunc->module;
  Module *dmodule = ds->noname;
  if (jmodule)
    {
      dmodule = dbeSession->createModule (ds, jmodule->get_name ());
      dmodule->lang_code = Sp_lang_java;
      dmodule->set_file_name (dbe_strdup (jmodule->file_name));
    }

  JMethod *dfunc = dbeSession->createJMethod ();
  dfunc->flags |= FUNC_FLAG_DYNAMIC;
  dfunc->size = msize;
  dfunc->module = dmodule;
  dfunc->usrfunc = jfunc;
  dfunc->set_addr (vaddr);
  dfunc->set_mid (mid);
  dfunc->set_signature (jfunc->get_signature ());
  dfunc->set_name (jfunc->get_mangled_name ());
  ds->functions->append (dfunc);
  dmodule->functions->append (dfunc);
  MapRecord *mrec = new MapRecord;
  mrec->kind = MapRecord::LOAD;
  mrec->obj = dfunc;
  mrec->base = vaddr;
  mrec->size = msize;
  mrec->ts = ts;
  mrec->foff = 0;
  mrec_insert (mrec);
  return 0;
}

int
Experiment::process_jcm_unload_cmd (char *, Vaddr /*mid*/, hrtime_t /*ts*/)
{
  if (jmaps == NULL)
    return 1;

  // We are ignoring this record because of the flaw in
  // JVMPI desing that doesn't distinguish between two or more
  // compiled instances of a method when an unload event is
  // generated:
  //     JVMPI_COMPILED_METHOD_LOAD( mid, addr1, ... )
  //     JVMPI_COMPILED_METHOD_LOAD( mid, addr2, ... )
  //     JVMPI_COMPILED_METHOD_UNLOAD( mid ) -- which one?
  // We rely on the ability of the PRBTree algorithms to
  // perform mapping appropriately based on timestamps.
  return 0;
}

int
Experiment::process_jthr_end_cmd (char *, uint64_t tid64, Vaddr jthr,
				  Vaddr jenv, hrtime_t ts)
{
  int lt = 0;
  int rt = jthreads_idx->size () - 1;
  uint32_t ttid = mapTagValue (PROP_THRID, tid64);
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      JThread *jthread = jthreads_idx->fetch (md);
      if (jthread->tid < ttid)
	lt = md + 1;
      else if (jthread->tid > ttid)
	rt = md - 1;
      else
	{
	  for (; jthread; jthread = jthread->next)
	    {
	      if (jthread->jenv == jenv)
		{
		  jthread->end = ts;
		  return 0;
		}
	    }
	  return 0;
	}
    }
  JThread *jthread = new JThread;
  jthread->tid = mapTagValue (PROP_THRID, tid64);
  jthread->jthr = jthr;
  jthread->jenv = jenv;
  jthread->jthr_id = jthreads->size ();
  jthread->start = ZERO_TIME;
  jthread->end = ts;
  jthread->next = NULL;
  jthreads->append (jthread);
  if (lt == jthreads_idx->size ())
    jthreads_idx->append (jthread);
  else
    jthreads_idx->insert (lt, jthread);
  return 0;
}

int
Experiment::process_jthr_start_cmd (char *, char *thread_name, char *group_name,
				    char *parent_name, uint64_t tid64,
				    Vaddr jthr, Vaddr jenv, hrtime_t ts)
{
  JThread *jthread = new JThread;
  jthread->name = thread_name;
  jthread->group_name = group_name;
  jthread->parent_name = parent_name;
  jthread->tid = mapTagValue (PROP_THRID, tid64);
  jthread->jthr = jthr;
  jthread->jenv = jenv;
  jthread->jthr_id = jthreads->size ();
  jthread->start = ts;
  jthread->end = MAX_TIME;
  jthread->next = NULL;

  jthreads->append (jthread);

  int lt = 0;
  int rt = jthreads_idx->size () - 1;
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      JThread *jtmp = jthreads_idx->fetch (md);
      if (jtmp->tid < jthread->tid)
	lt = md + 1;
      else if (jtmp->tid > jthread->tid)
	rt = md - 1;
      else
	{
	  jthread->next = jtmp;
	  jthreads_idx->store (md, jthread);
	  return 0;
	}
    }
  if (lt == jthreads_idx->size ())
    jthreads_idx->append (jthread);
  else
    jthreads_idx->insert (lt, jthread);
  return 0;
}

int
Experiment::process_gc_end_cmd (
				hrtime_t ts)
{
  if (gcevents->size () == 0)
    {
      GCEvent *gcevent = new GCEvent;
      gcevent->start = ZERO_TIME;
      gcevent->end = ts;
      gcevent->id = gcevents->size () + 1;
      gcevents->append (gcevent);
      return 0;
    }
  GCEvent *gcevent = gcevents->fetch (gcevents->size () - 1);
  if (gcevent->end == MAX_TIME)
    gcevent->end = ts;
  else
    // Weird: gc_end followed by another gc_end
    gcevent->end = ts; // extend the previous event
  return 0;
}

int
Experiment::process_gc_start_cmd (
				  hrtime_t ts)
{
  if (gcevents->size () != 0)
    {
      GCEvent *gcevent = gcevents->fetch (gcevents->size () - 1);
      // Weird: gc_start followed by another gc_start
      if (gcevent->end == MAX_TIME)
	return 0; // ignore nested gc_starts
    }
  GCEvent *gcevent = new GCEvent;
  gcevent->start = ts;
  gcevent->end = MAX_TIME;
  gcevent->id = gcevents->size () + 1;
  gcevents->append (gcevent);
  return 0;
}

int
Experiment::process_sample_cmd (char */*cmd*/, hrtime_t /*log_xml_time*/,
				int sample_number, char *label)
{
  // sample 0 is not a sample but the starting point
  if (sample_number == 0)
    {
      first_sample_label = label;
      return 0;
    }
  Sample *prev_sample = samples->size () > 0 ?
	  samples->fetch (samples->size () - 1) : NULL;
  char *start_lable = prev_sample ?
	  prev_sample->end_label : first_sample_label;
  Sample *sample = new Sample (sample_number);
  sample->start_label = dbe_strdup (start_lable);
  sample->end_label = label;
  samples->append (sample);
  return 0;
}

int
Experiment::process_sample_sig_cmd (char *, int sig)
{
  char *str;
  Emsg *m;
  str = dbe_sprintf (GTXT ("Sample signal %d"), sig);
  m = new Emsg (CMSG_COMMENT, str);
  free (str);
  runlogq->append (m);
  return 0;
}

int
Experiment::process_seg_map_cmd (char */*cmd*/, hrtime_t ts, Vaddr vaddr,
				 int mapsize, int /*pagesize*/, int64_t offset,
				 int64_t modeflags, int64_t chk, char *nm)
{
  if (nm == NULL ||
      strncmp (nm + 1, SP_MAP_UNRESOLVABLE, strlen (SP_MAP_UNRESOLVABLE)) == 0)
    return 0;

  LoadObject *lo = loadObjMap->get (nm);
  if (lo == NULL)
    {
      if (chk == 0)
	{
	  char *archName = checkFileInArchive (nm, false);
	  if (archName)
	    {
	      Elf *elf = new Elf (archName);
	      if (elf->status == Elf::ELF_ERR_NONE)
		{
		  chk = elf->elf_checksum ();
		}
	      free (archName);
	      delete elf;
	    }
	}
      lo = dbeSession->find_lobj_by_name (nm, chk);
      if (lo == NULL)
	{
	  // Skip non-text segments
	  if (modeflags != (PROT_READ | PROT_EXEC))
	    return 0;
	  // A new segment
	  lo = createLoadObject (nm, chk);
	  if (strstr (nm, NTXT ("libjvm.so")))
	    {
	      lo->flags |= SEG_FLAG_JVM;
	      // Make sure <JVM-System> is created
	      (void) dbeSession->get_jvm_Function ();
	    }
	  else if (strstr (nm, NTXT ("libmtsk.so")))
	    {
	      lo->flags |= SEG_FLAG_OMP;
	      // Make sure all pseudo functions are created
	      for (int i = 0; i < OMP_LAST_STATE; i++)
		(void) dbeSession->get_OMP_Function (i);
	    }
	  else if (dbe_strcmp (utargname, get_basename (nm)) == 0)
	    {
	      lo->flags |= SEG_FLAG_EXE;
	      (void) dbeSession->comp_lobjs->get ((char *) COMP_EXE_NAME, lo);
	    }
	  lo->checksum = chk;
	  //  This is the default segment type
	  lo->type = LoadObject::SEG_TEXT;
	  lo->flags = lo->flags | SEG_FLAG_REORDER;
	  lo->set_platform (platform, wsize);
	}
      if (lo->dbeFile->get_location (false) == NULL)
	{
	  char *archName = checkFileInArchive (nm, false);
	  if (archName)
	    {
	      lo->dbeFile->set_location (archName);
	      lo->dbeFile->inArchive = true;
	      lo->dbeFile->check_access (archName); // init 'sbuf'
	      lo->dbeFile->sbuf.st_mtime = 0; // Don't check timestamps
	      free (archName);
	    }
	  else
	    {
	      archName = checkFileInArchive (nm, true);
	      if (archName)
		{
		  lo->set_archname (archName);
		  lo->need_swap_endian = need_swap_endian;
		}
	    }
	  if (!dbeSession->archive_mode)
	    lo->sync_read_stabs ();
	}
      append (lo);
    }
  if (lo->size == 0)
    lo->size = mapsize;
  MapRecord *mrec = new MapRecord;
  mrec->kind = MapRecord::LOAD;
  mrec->obj = lo;
  mrec->base = vaddr;
  mrec->size = mapsize;
  mrec->ts = ts;
  mrec->foff = offset;
  mrec_insert (mrec);
  return 0;
}

int
Experiment::process_seg_unmap_cmd (char */*cmd*/, hrtime_t ts, Vaddr vaddr)
{
  MapRecord *mrec = new MapRecord;
  mrec->kind = MapRecord::UNLOAD;
  mrec->base = vaddr;
  mrec->ts = ts;
  mrec_insert (mrec);
  return 0;
}

static bool
strstarts (const char *var, const char *x)
{
  return strncmp (var, x, strlen (x)) == 0;
}

int
Experiment::process_Linux_kernel_cmd (hrtime_t ts)
{
  LoadObject *lo = createLoadObject ("LinuxKernel");
  lo->flags |= SEG_FLAG_EXE;
  lo->type = LoadObject::SEG_TEXT;
  lo->set_platform (platform, wsize);
  append (lo);
  long long unsigned lo_min = (long long unsigned) (-1);
  long long unsigned lo_max = 0;
  Module *mod = dbeSession->createModule (lo, "LinuxKernel");
  /*
   * XXX need to review mod initialization
   * A specific issue is mod->file_name.  Options include:
   *     *) NULL
   *            This leads to seg faults in, e.g., Timeline view.
   *     *) "/lib/modules/$(uname -r)/kernel/kernel/ctf/ctf.ko"
   *            This leads to garbage in the Source view.
   *     *) "/boot/vmlinuz-$(uname -r)"
   *            This cannot be parsed for DWARF and is sometimes not found,
   *            but the Analyzer seems to handle such problems.
   *     *) "LinuxKernel"
   *            This is not a proper file name,
   *            but again Analyzer handles the case of not finding the file or not reading DWARF from it.
   */
  mod->set_file_name (dbe_strdup ("LinuxKernel"));
  char kallmodsyms_copy[MAXPATHLEN];
  snprintf (kallmodsyms_copy, sizeof (kallmodsyms_copy), "%s/kallmodsyms",
	    expt_name);
  FILE *fd = fopen (kallmodsyms_copy, "r");
  if (fd == NULL)
    {
      char *s = dbe_sprintf (GTXT ("*** Error: Cannot find kernel module symbols file %s; ignored"),
			     kallmodsyms_copy);
      Emsg *m = new Emsg (CMSG_ERROR, s);
      free (s);
      errorq->append (m);
      lo_min = 0;
    }
  else
    {
      size_t line_n = 0;
      char *line = NULL;
      while (getline (&line, &line_n, fd) > 0)
	{
	  long long unsigned sym_addr;
	  long long unsigned sym_size;
	  char sym_type;
	  int sym_text;
	  char sym_name[256];
	  char mod_name[256] = "vmlinux]"; /* note trailing ] */
	  sscanf (line, "%llx %llx %c %s [%s", &sym_addr, &sym_size, &sym_type,
		  sym_name, mod_name);
	  if (line[0] == '\n' || line[0] == 0)
	    continue;
	  sym_text = (sym_type == 't' || sym_type == 'T');
	  mod_name[strlen (mod_name) - 1] = '\0'; /* chop trailing ] */
	  if (strcmp (mod_name, "ctf") == 0)
	    strcpy (mod_name, "shared_ctf");

	  if (strcmp (sym_name, "__per_cpu_start") == 0
	      || strcmp (sym_name, "__per_cpu_end") == 0
	      || strstarts (sym_name, "__crc_")
	      || strstarts (sym_name, "__ksymtab_")
	      || strstarts (sym_name, "__kcrctab_")
	      || strstarts (sym_name, "__kstrtab_")
	      || strstarts (sym_name, "__param_")
	      || strstarts (sym_name, "__syscall_meta__")
	      || strstarts (sym_name, "__p_syscall_meta__")
	      || strstarts (sym_name, "__event_")
	      || strstarts (sym_name, "event_")
	      || strstarts (sym_name, "ftrace_event_")
	      || strstarts (sym_name, "types__")
	      || strstarts (sym_name, "args__")
	      || strstarts (sym_name, "__tracepoint_")
	      || strstarts (sym_name, "__tpstrtab_")
	      || strstarts (sym_name, "__tpstrtab__")
	      || strstarts (sym_name, "__initcall_")
	      || strstarts (sym_name, "__setup_")
	      || strstarts (sym_name, "__pci_fixup_")
	      || strstarts (sym_name, "__dta_")
	      || strstarts (sym_name, "__dtrace_probe_")
	      || (strstr (sym_name, ".") != NULL
		  &&  strstr (sym_name, ".clone.") == NULL))
	    continue;

	  if (sym_text)
	    {
	      StringBuilder sb;
	      sb.appendf ("%s`%s", mod_name, sym_name);
	      char *fname = sb.toString ();
	      Function *func = dbeSession->createFunction ();
	      func->set_name (fname);
	      free (fname);
	      func->size = sym_size;
	      func->img_offset = sym_addr;
	      func->module = mod;
	      lo->functions->append (func);
	      mod->functions->append (func);
	      if (lo_min > sym_addr)
		lo_min = sym_addr;
	      if (lo_max < sym_addr + sym_size)
		lo_max = sym_addr + sym_size;
	    }
	}
      fclose (fd);
      free (line);
    }
  lo->size = lo_max;
  lo->functions->sort (func_cmp);
  mod->functions->sort (func_cmp);

  MapRecord *mrec = new MapRecord;
  mrec->kind = MapRecord::LOAD;
  mrec->obj = lo;
  mrec->base = lo_min;
  mrec->size = lo_max - lo_min;
  mrec->ts = ts;
  mrec->foff = lo_min;
  mrec_insert (mrec);
  return 0;
}
