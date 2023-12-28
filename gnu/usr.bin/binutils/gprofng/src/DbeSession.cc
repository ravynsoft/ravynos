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
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/param.h>

#include "util.h"
#include "Application.h"
#include "Experiment.h"
#include "ExpGroup.h"
#include "Expression.h"
#include "DataObject.h"
#include "Elf.h"
#include "Function.h"
#include "DbeSession.h"
#include "LoadObject.h"
#include "DbeSyncMap.h"
#include "DbeThread.h"
#include "ClassFile.h"
#include "IndexObject.h"
#include "PathTree.h"
#include "Print.h"

// Bison 3.0 doesn't define YY_NULLPTR. I copied this from QLParser.tab.cc.
// Why this is not in QLParser.tab.hh ? YY_NULLPTR is used in QLParser.tab.hh
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif
#include "QLParser.tab.hh"
#include "DbeView.h"
#include "MemorySpace.h"
#include "Module.h"
#include "SourceFile.h"
#include "StringBuilder.h"
#include "BaseMetric.h"
#include "BaseMetricTreeNode.h"
#include "Command.h"
#include "UserLabel.h"
#include "StringMap.h"
#include "DbeFile.h"
#include "DbeJarFile.h"
#include "IOActivity.h"
#include "HeapActivity.h"

// This is a universal List structure to organize objects
// of various types, even if different.
struct List
{
  List *next;
  void *val;
};

struct Countable
{
  Countable (void *_item)
  {
    item = _item;
    ref_count = 0;
  }

  void *item;
  int ref_count;
};

Platform_t DbeSession::platform =
#if ARCH(SPARC)
	Sparc;
#elif ARCH(Aarch64)
	Aarch64;
#else   // ARCH(Intel)
	Intel;
#endif

// This constant determines the size of the data object name hash table.
static const int HTableSize         = 8192;
static int DEFAULT_TINY_THRESHOLD   = -1;

unsigned int mpmt_debug_opt = 0;
DbeSession *dbeSession = NULL;

DbeSession::DbeSession (Settings *_settings, bool _ipc_mode, bool _rdt_mode)
{
  dbeSession = this;
  ipc_mode = _ipc_mode;
  rdt_mode = _rdt_mode;
  settings = new Settings (_settings);
  views = new Vector<DbeView*>;
  exps = new Vector<Experiment*>;
  lobjs = new Vector<LoadObject*>;
  objs = new Vector<Histable*>;
  dobjs = new Vector<DataObject*>;
  metrics = new Vector<Countable*>;
  reg_metrics = new Vector<BaseMetric*>;
  hwcentries = NULL;
  reg_metrics_tree = NULL; // BaseMetric() requires DbeSession::ql_parse
  idxobjs = new Vector<HashMap<uint64_t, Histable*>*>;
  tmp_files = new Vector<char*>;
  search_path = new Vector<char*>;
  classpath = new Vector<char*>;
  classpath_df = NULL;
  expGroups = new Vector<ExpGroup*>;
  sourcesMap = new HashMap<char*, SourceFile*>;
  sources = new Vector<SourceFile*>;
  comp_lobjs = new HashMap<char*, LoadObject*>;
  comp_dbelines = new HashMap<char*, DbeLine*>;
  comp_sources = new HashMap<char*, SourceFile*>;
  loadObjMap = new DbeSyncMap<LoadObject>;
  f_special = new Vector<Function*>(LastSpecialFunction);
  omp_functions = new Vector<Function*>(OMP_LAST_STATE);
  interactive = false;
  lib_visibility_used = false;

  // Define all known property names
  propNames = new Vector<PropDescr*>;
  propNames_name_store (PROP_NONE, NTXT (""));
  propNames_name_store (PROP_ATSTAMP, NTXT ("ATSTAMP"));
  propNames_name_store (PROP_ETSTAMP, NTXT ("ETSTAMP"));
  propNames_name_store (PROP_TSTAMP, NTXT ("TSTAMP"));
  propNames_name_store (PROP_THRID, NTXT ("THRID"));
  propNames_name_store (PROP_LWPID, NTXT ("LWPID"));
  propNames_name_store (PROP_CPUID, NTXT ("CPUID"));
  propNames_name_store (PROP_FRINFO, NTXT ("FRINFO"));
  propNames_name_store (PROP_EVT_TIME, NTXT ("EVT_TIME"));

  // Samples
  propNames_name_store (PROP_SMPLOBJ, NTXT ("SMPLOBJ"));
  propNames_name_store (PROP_SAMPLE, NTXT ("SAMPLE"));

  // GCEvents
  propNames_name_store (PROP_GCEVENTOBJ, NTXT ("GCEVENTOBJ"));
  propNames_name_store (PROP_GCEVENT, NTXT ("GCEVENT"));

  // Metadata used by some packet types
  propNames_name_store (PROP_VOIDP_OBJ, NTXT ("VOIDP_OBJ"),
			NULL, TYPE_UINT64, DDFLAG_NOSHOW);

  // Clock profiling properties
  propNames_name_store (PROP_UCPU, NTXT ("UCPU"));
  propNames_name_store (PROP_SCPU, NTXT ("SCPU"));
  propNames_name_store (PROP_TRAP, NTXT ("TRAP"));
  propNames_name_store (PROP_TFLT, NTXT ("TFLT"));
  propNames_name_store (PROP_DFLT, NTXT ("DFLT"));
  propNames_name_store (PROP_KFLT, NTXT ("KFLT"));
  propNames_name_store (PROP_ULCK, NTXT ("ULCK"));
  propNames_name_store (PROP_TSLP, NTXT ("TSLP"));
  propNames_name_store (PROP_WCPU, NTXT ("WCPU"));
  propNames_name_store (PROP_TSTP, NTXT ("TSTP"));

  propNames_name_store (PROP_MSTATE, NTXT ("MSTATE"));
  propNames_name_store (PROP_NTICK, NTXT ("NTICK"));
  propNames_name_store (PROP_OMPSTATE, NTXT ("OMPSTATE"));

  // Synchronization tracing properties
  propNames_name_store (PROP_SRQST, NTXT ("SRQST"));
  propNames_name_store (PROP_SOBJ, NTXT ("SOBJ"));

  // Hardware counter profiling properties
  propNames_name_store (PROP_HWCTAG, NTXT ("HWCTAG"));
  propNames_name_store (PROP_HWCINT, NTXT ("HWCINT"));
  propNames_name_store (PROP_VADDR, NTXT ("VADDR"));
  propNames_name_store (PROP_PADDR, NTXT ("PADDR"));
  propNames_name_store (PROP_VIRTPC, NTXT ("VIRTPC"));
  propNames_name_store (PROP_PHYSPC, NTXT ("PHYSPC"));
  propNames_name_store (PROP_LWP_LGRP_HOME, NTXT ("LWP_LGRP_HOME"));
  propNames_name_store (PROP_PS_LGRP_HOME, NTXT ("PS_LGRP_HOME"));
  propNames_name_store (PROP_EA_PAGESIZE, NTXT ("EA_PAGESIZE"));
  propNames_name_store (PROP_EA_LGRP, NTXT ("EA_LGRP"));
  propNames_name_store (PROP_PC_PAGESIZE, NTXT ("PC_PAGESIZE"));
  propNames_name_store (PROP_PC_LGRP, NTXT ("PC_LGRP"));
  propNames_name_store (PROP_HWCDOBJ, NTXT ("HWCDOBJ"));
  propNames_name_store (PROP_MEM_LAT, NTXT ("MEM_LAT"));
  propNames_name_store (PROP_MEM_SRC, NTXT ("MEM_SRC"));

  // Heap tracing properties
  propNames_name_store (PROP_HTYPE, NTXT ("HTYPE"));
  propNames_name_store (PROP_HSIZE, NTXT ("HSIZE"));
  propNames_name_store (PROP_HVADDR, NTXT ("HVADDR"));
  propNames_name_store (PROP_HOVADDR, NTXT ("HOVADDR"));
  propNames_name_store (PROP_HLEAKED, NTXT ("HLEAKED"),
			GTXT ("Leaked bytes"), TYPE_UINT64, 0);
  propNames_name_store (PROP_HMEM_USAGE, NTXT ("HMEM_USAGE"));
  propNames_name_store (PROP_HFREED, NTXT ("HFREED"),
			GTXT ("Freed bytes"), TYPE_UINT64, 0);
  propNames_name_store (PROP_HCUR_ALLOCS, NTXT ("HCUR_ALLOCS"),
			GTXT ("Current allocations"), TYPE_INT64, 0);
  propNames_name_store (PROP_HCUR_NET_ALLOC, NTXT ("HCUR_NET_ALLOC"),
			NULL, TYPE_INT64, DDFLAG_NOSHOW);
  propNames_name_store (PROP_HCUR_LEAKS, NTXT ("HCUR_LEAKS"),
			GTXT ("Current leaks"), TYPE_UINT64, 0);
  propNames_name_store (PROP_DDSCR_LNK, NTXT ("DDSCR_LNK"),
			NULL, TYPE_UINT64, DDFLAG_NOSHOW);

  // IO tracing properties
  propNames_name_store (PROP_IOTYPE, NTXT ("IOTYPE"));
  propNames_name_store (PROP_IOFD, NTXT ("IOFD"));
  propNames_name_store (PROP_IONBYTE, NTXT ("IONBYTE"));
  propNames_name_store (PROP_IORQST, NTXT ("IORQST"));
  propNames_name_store (PROP_IOOFD, NTXT ("IOOFD"));
  propNames_name_store (PROP_IOFNAME, NTXT ("IOFNAME"));
  propNames_name_store (PROP_IOVFD, NTXT ("IOVFD"));
  propNames_name_store (PROP_IOFSTYPE, NTXT ("IOFSTYPE"));

  // omptrace raw properties
  propNames_name_store (PROP_CPRID, NTXT ("CPRID"));
  propNames_name_store (PROP_PPRID, NTXT ("PPRID"));
  propNames_name_store (PROP_TSKID, NTXT ("TSKID"));
  propNames_name_store (PROP_PTSKID, NTXT ("PTSKID"));
  propNames_name_store (PROP_PRPC, NTXT ("PRPC"));

  // Data race detection properties
  propNames_name_store (PROP_RID, NTXT ("RID"));
  propNames_name_store (PROP_RTYPE, NTXT ("RTYPE"));
  propNames_name_store (PROP_LEAFPC, NTXT ("LEAFPC"));
  propNames_name_store (PROP_RVADDR, NTXT ("RVADDR"));
  propNames_name_store (PROP_RCNT, NTXT ("RCNT"));

  // Deadlock detection properties
  propNames_name_store (PROP_DID, NTXT ("DID"));
  propNames_name_store (PROP_DLTYPE, NTXT ("DLTYPE"));
  propNames_name_store (PROP_DTYPE, NTXT ("DTYPE"));
  propNames_name_store (PROP_DVADDR, NTXT ("DVADDR"));

  // Synthetic properties (queries only)
  propNames_name_store (PROP_STACK, NTXT ("STACK"));
  propNames_name_store (PROP_MSTACK, NTXT ("MSTACK"));
  propNames_name_store (PROP_USTACK, NTXT ("USTACK"));
  propNames_name_store (PROP_XSTACK, NTXT ("XSTACK"));
  propNames_name_store (PROP_HSTACK, NTXT ("HSTACK"));
  propNames_name_store (PROP_STACKID, NTXT ("STACKID"));
  //propNames_name_store( PROP_CPRID,   NTXT("CPRID") );
  //propNames_name_store( PROP_TSKID,   NTXT("TSKID") );
  propNames_name_store (PROP_JTHREAD, NTXT ("JTHREAD"),
			GTXT ("Java thread number"), TYPE_UINT64, 0);

  propNames_name_store (PROP_LEAF, NTXT ("LEAF"));
  propNames_name_store (PROP_DOBJ, NTXT ("DOBJ"));
  propNames_name_store (PROP_SAMPLE_MAP, NTXT ("SAMPLE_MAP"));
  propNames_name_store (PROP_GCEVENT_MAP, NTXT ("GCEVENT_MAP"));
  propNames_name_store (PROP_PID, NTXT ("PID"),
			GTXT ("Process id"), TYPE_UINT64, 0);
  propNames_name_store (PROP_EXPID, NTXT ("EXPID"),
			GTXT ("Experiment id"), TYPE_UINT64, DDFLAG_NOSHOW);
  propNames_name_store (PROP_EXPID_CMP, NTXT ("EXPID_CMP"),
			GTXT ("Comparable Experiment Id"), TYPE_UINT64,
			DDFLAG_NOSHOW); //YXXX find better description
  propNames_name_store (PROP_EXPGRID, NTXT ("EXPGRID"),
			GTXT ("Comparison Group id"), TYPE_UINT64, 0);
  propNames_name_store (PROP_PARREG, NTXT ("PARREG"));
  propNames_name_store (PROP_TSTAMP_LO, NTXT ("TSTAMP_LO"),
			GTXT ("Start Timestamp (nanoseconds)"), TYPE_UINT64, 0);
  propNames_name_store (PROP_TSTAMP_HI, NTXT ("TSTAMP_HI"),
			GTXT ("End Timestamp (nanoseconds)"), TYPE_UINT64, 0);
  propNames_name_store (PROP_TSTAMP2, NTXT ("TSTAMP2"),
			GTXT ("End Timestamp (nanoseconds)"), TYPE_UINT64,
			DDFLAG_NOSHOW);
  propNames_name_store (PROP_FREQ_MHZ, NTXT ("FREQ_MHZ"),
			GTXT ("CPU Frequency, MHz"), TYPE_UINT32, 0);
  propNames_name_store (PROP_NTICK_USEC, NTXT ("NTICK_USEC"),
			GTXT ("Clock Profiling Interval, Microseconds"),
			TYPE_UINT64, 0);

  propNames_name_store (PROP_IOHEAPBYTES, NTXT ("IOHEAPBYTES"));

  propNames_name_store (PROP_STACKL, NTXT ("STACKL"));
  propNames_name_store (PROP_MSTACKL, NTXT ("MSTACKL"));
  propNames_name_store (PROP_USTACKL, NTXT ("USTACKL"));
  propNames_name_store (PROP_XSTACKL, NTXT ("XSTACKL"));

  propNames_name_store (PROP_STACKI, NTXT ("STACKI"));
  propNames_name_store (PROP_MSTACKI, NTXT ("MSTACKI"));
  propNames_name_store (PROP_USTACKI, NTXT ("USTACKI"));
  propNames_name_store (PROP_XSTACKI, NTXT ("XSTACKI"));

  // Make sure predefined names are not used for dynamic properties
  propNames_name_store (PROP_LAST, NTXT (""));

  localized_SP_UNKNOWN_NAME = GTXT ("(unknown)");

  // define Index objects
  dyn_indxobj = new Vector<IndexObjType_t*>();
  dyn_indxobj_indx = 0;
  char *s = dbe_sprintf (NTXT ("((EXPID_CMP<<%llu) | THRID)"),
			 (unsigned long long) IndexObject::INDXOBJ_EXPID_SHIFT);
  indxobj_define (NTXT ("Threads"), GTXT ("Threads"), s, NULL, NULL);
  free (s);
  indxobj_define (NTXT ("CPUs"), GTXT ("CPUs"), NTXT ("(CPUID)"), NULL, NULL);
  indxobj_define (NTXT ("Samples"), GTXT ("Samples"), NTXT ("(SAMPLE_MAP)"),
		  NULL, NULL);
  indxobj_define (NTXT ("GCEvents"), GTXT ("GCEvents"), NTXT ("(GCEVENT_MAP)"),
		  NULL, NULL);
  indxobj_define (NTXT ("Seconds"), GTXT ("Seconds"),
		  NTXT ("(TSTAMP/1000000000)"), NULL, NULL);
  indxobj_define (NTXT ("Processes"), GTXT ("Processes"), NTXT ("(EXPID_CMP)"),
		  NULL, NULL);
  s = dbe_sprintf (NTXT ("((EXPGRID<<%llu) | (EXPID<<%llu))"),
		   (unsigned long long) IndexObject::INDXOBJ_EXPGRID_SHIFT,
		   (unsigned long long) IndexObject::INDXOBJ_EXPID_SHIFT);
  indxobj_define (NTXT ("Experiment_IDs"), GTXT ("Experiment_IDs"), s, NULL, NULL);
  free (s);
  indxobj_define (NTXT ("Datasize"), GTXT ("Datasize"),
		  "(IOHEAPBYTES==0?0:"
		  "((IOHEAPBYTES<=(1<<0)?(1<<0):"
		  "((IOHEAPBYTES<=(1<<2)?(1<<2):"
		  "((IOHEAPBYTES<=(1<<4)?(1<<4):"
		  "((IOHEAPBYTES<=(1<<6)?(1<<6):"
		  "((IOHEAPBYTES<=(1<<8)?(1<<8):"
		  "((IOHEAPBYTES<=(1<<10)?(1<<10):"
		  "((IOHEAPBYTES<=(1<<12)?(1<<12):"
		  "((IOHEAPBYTES<=(1<<14)?(1<<14):"
		  "((IOHEAPBYTES<=(1<<16)?(1<<16):"
		  "((IOHEAPBYTES<=(1<<18)?(1<<18):"
		  "((IOHEAPBYTES<=(1<<20)?(1<<20):"
		  "((IOHEAPBYTES<=(1<<22)?(1<<22):"
		  "((IOHEAPBYTES<=(1<<24)?(1<<24):"
		  "((IOHEAPBYTES<=(1<<26)?(1<<26):"
		  "((IOHEAPBYTES<=(1<<28)?(1<<28):"
		  "((IOHEAPBYTES<=(1<<30)?(1<<30):"
		  "((IOHEAPBYTES<=(1<<32)?(1<<32):"
		  "((IOHEAPBYTES<=(1<<34)?(1<<34):"
		  "((IOHEAPBYTES<=(1<<36)?(1<<36):"
		  "((IOHEAPBYTES<=(1<<38)?(1<<38):"
		  "((IOHEAPBYTES<=(1<<40)?(1<<40):"
		  "((IOHEAPBYTES<=(1<<42)?(1<<42):"
		  "((IOHEAPBYTES<=(1<<44)?(1<<44):"
		  "((IOHEAPBYTES<=(1<<46)?(1<<46):"
		  "((IOHEAPBYTES<=(1<<48)?(1<<48):"
		  "((IOHEAPBYTES<=(1<<50)?(1<<50):"
		  "(IOHEAPBYTES==-1?-1:(1<<50|1)"
		  "))))))))))))))))))))))))))))))))))))))))))))))))))))))",
		  NULL, NULL);
  indxobj_define (NTXT ("Duration"), GTXT ("Duration"),
		  "((TSTAMP_HI-TSTAMP_LO)==0?0:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=1000?1000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=10000?10000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=100000?100000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=1000000?1000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=10000000?10000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=100000000?100000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=1000000000?1000000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=10000000000?10000000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=100000000000?100000000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=1000000000000?1000000000000:"
		  "(((TSTAMP_HI-TSTAMP_LO)<=10000000000000?10000000000000:"
		  "(10000000000001))))))))))))))))))))))))", NULL, NULL);
  dyn_indxobj_indx_fixed = dyn_indxobj_indx;
  Elf::elf_init ();
  defExpName = NULL;
  mach_model_loaded = NULL;
  tmp_dir_name = NULL;
  settings->read_rc (ipc_mode || rdt_mode);

  init ();
}

DbeSession::~DbeSession ()
{
  Destroy (views);
  Destroy (exps);
  Destroy (dobjs);
  Destroy (metrics);
  Destroy (search_path);
  Destroy (classpath);
  Destroy (propNames);
  Destroy (expGroups);
  Destroy (userLabels);
  if (hwcentries)
    {
      for (long i = 0, sz = hwcentries->size (); i < sz; i++)
	{
	  Hwcentry *h = hwcentries->get (i);
	  free (h->int_name);
	  free (h->name);
	  delete h;
	}
      delete hwcentries;
    }

  if (idxobjs)
    {
      for (int i = 0; i < idxobjs->size (); ++i)
	{
	  HashMap<uint64_t, Histable*> *hMap = idxobjs->get (i);
	  if (hMap)
	    {
	      hMap->values ()->destroy ();
	      delete hMap;
	    }
	}
      delete idxobjs;
    }

  for (int i = 0; i < HTableSize; i++)
    {
      List *list = dnameHTable[i];
      while (list)
	{
	  List *tmp = list;
	  list = list->next;
	  delete tmp;
	}
    }
  delete[] dnameHTable;
  delete classpath_df;
  Destroy (objs);
  Destroy (reg_metrics);
  Destroy (dyn_indxobj);
  delete lobjs;
  delete f_special;
  destroy_map (DbeFile *, dbeFiles);
  destroy_map (DbeJarFile *, dbeJarFiles);
  delete loadObjMap;
  delete omp_functions;
  delete sourcesMap;
  delete sources;
  delete comp_lobjs;
  delete comp_dbelines;
  delete comp_sources;
  delete reg_metrics_tree;
  delete settings;
  free (mach_model_loaded);

  if (defExpName != NULL)
    {
      StringBuilder *sb = new StringBuilder ();
      sb->append (NTXT ("/bin/rm -rf "));
      sb->append (defExpName);
      char *cmd = sb->toString ();
      system (cmd);
      free (cmd);
      delete sb;
      free (defExpName);
    }
  unlink_tmp_files ();
  delete tmp_files;
  dbeSession = NULL;
}

void
DbeSession::unlink_tmp_files ()
{
  if (tmp_files)
    {
      for (int i = 0, sz = tmp_files->size (); i < sz; i++)
	unlink (tmp_files->fetch (i));
      tmp_files->destroy ();
      delete tmp_files;
      tmp_files = NULL;
    }
  if (tmp_dir_name)
    {
      char *cmd = dbe_sprintf (NTXT ("/bin/rm -rf %s"), tmp_dir_name);
      system (cmd);
      free (cmd);
      free (tmp_dir_name);
      tmp_dir_name = NULL;
    }
}

char *
DbeSession::get_tmp_file_name (const char *nm, bool for_java)
{
  if (tmp_dir_name == NULL)
    {
      tmp_dir_name = dbe_sprintf (NTXT ("/tmp/analyzer.%llu.%lld"),
			 (unsigned long long) getuid (), (long long) getpid ());
      mkdir (tmp_dir_name, S_IRWXU);
    }
  char *fnm = dbe_sprintf (NTXT ("%s/%s"), tmp_dir_name, nm);
  if (for_java)
    for (char *s = fnm + strlen (tmp_dir_name) + 1; *s; s++)
      if (*s == '/')
	*s = '.';
  return fnm;
}

void
DbeSession::init ()
{
  user_exp_id_counter = 0;
  status_ompavail = 0;
  archive_mode = 0;

#if DEBUG
  char *s = getenv (NTXT ("MPMT_DEBUG"));
  if (s)
    mpmt_debug_opt = atoi (s);
#endif /* DEBUG */
  dbeFiles = new StringMap<DbeFile*>();
  dbeJarFiles = new StringMap<DbeJarFile*>(128, 128);

  // set up the initial (after .rc file reading) search path
  set_search_path (settings->str_search_path, true);
  userLabels = NULL;

  // Preset all objects as they may reuse each other
  lo_unknown = NULL;
  f_unknown = NULL;
  j_unknown = NULL;
  lo_total = NULL;
  sf_unknown = NULL;
  f_total = NULL;
  f_jvm = NULL;
  d_total = NULL;
  d_scalars = NULL;
  d_unknown = NULL;
  expGroups->destroy ();
  f_special->reset ();
  for (int i = 0; i < LastSpecialFunction; i++)
    f_special->append (NULL);

  lo_omp = NULL;
  omp_functions->reset ();
  for (int i = 0; i < OMP_LAST_STATE; i++)
    omp_functions->append (NULL);

  // make sure the metric list is initialized
  register_metric (Metric::SIZES);
  register_metric (Metric::ADDRESS);
  register_metric (Metric::ONAME);

  // This is needed only to maintain loadobject id's
  // for <Total> and <Unknown> in tests
  (void) get_Unknown_LoadObject ();
  (void) get_Total_LoadObject ();

  // Create the data object name hash table.
  dnameHTable = new List*[HTableSize];
  for (int i = 0; i < HTableSize; i++)
    dnameHTable[i] = NULL;

  d_total = createDataObject ();
  d_total->set_name (NTXT ("<Total>"));

  // XXXX <Scalars> only appropriate for Program/Data-oriented analyses
  d_scalars = createDataObject ();
  d_scalars->set_name (GTXT ("<Scalars>"));

  d_unknown = createDataObject ();
  d_unknown->set_name (GTXT ("<Unknown>"));

  // assign d_unknown's children so data_olayout has consistent sorting
  for (unsigned pp_code = 1; pp_code < NUM_ABS_PP_CODES + 2; pp_code++)
    {
      char *errcode;
      DataObject* dobj = createDataObject ();
      switch (pp_code)
	{
	case NUM_ABS_PP_CODES + 1:
	  errcode = PTXT (DOBJ_UNDETERMINED);
	  break;
	case NUM_ABS_PP_CODES:
	  errcode = PTXT (DOBJ_UNSPECIFIED);
	  break;
	case NUM_ABS_PP_CODES - 1:
	  errcode = PTXT (DOBJ_UNIDENTIFIED);
	  break;
	default:
	  errcode = PTXT (ABS_PP_CODES[pp_code]);
	}
      dobj->parent = d_unknown;
      dobj->set_dobjname (errcode, NULL); // dobj->parent must already be set
    }

  for (unsigned rt_code = 1; rt_code < NUM_ABS_RT_CODES - 1; rt_code++)
    {
      DataObject* dobj = createDataObject ();
      dobj->parent = d_unknown;
      dobj->set_dobjname (PTXT (ABS_RT_CODES[rt_code]), NULL); // dobj->parent must already be set
    }
}

void
DbeSession::reset_data ()
{
  for (long i = 0, sz = VecSize (idxobjs); i < sz; ++i)
    if (idxobjs->get (i))
      idxobjs->get (i)->reset ();
}

void
DbeSession::reset ()
{
  loadObjMap->reset ();
  DbeView *dbev;
  int index;

  Vec_loop (DbeView*, views, index, dbev)
  {
    dbev->reset ();
  }

  destroy_map (DbeFile *, dbeFiles);
  destroy_map (DbeJarFile *, dbeJarFiles);
  exps->destroy ();
  lobjs->reset ();      // all LoadObjects belong to objs
  dobjs->destroy ();    // deletes d_unknown and d_total as well
  objs->destroy ();
  comp_lobjs->clear ();
  comp_dbelines->clear ();
  comp_sources->clear ();
  sourcesMap->clear ();
  sources->reset ();

  // Delete the data object name hash table.
  for (int i = 0; i < HTableSize; i++)
    {
      List *list = dnameHTable[i];
      while (list)
	{
	  List *tmp = list;
	  list = list->next;
	  delete tmp;
	}
    }
  delete[] dnameHTable;

  // IndexObect definitions remain, objects themselves may go
  for (int i = 0; i < idxobjs->size (); ++i)
    {
      HashMap<uint64_t, Histable*> *v = idxobjs->fetch (i);
      if (v != NULL)
	{
	  v->values ()->destroy ();
	  v->clear ();
	}
    }
  init ();
}

Vector<SourceFile*> *
DbeSession::get_sources ()
{
  return sources;
}

DbeFile *
DbeSession::getDbeFile (char *filename, int filetype)
{
  Dprintf (DEBUG_DBE_FILE, NTXT ("DbeSession::getDbeFile  filetype=0x%x %s\n"), filetype, filename);
  if (strncmp (filename, NTXT ("./"), 2) == 0)
    filename += 2;
  DbeFile *dbeFile = dbeFiles->get (filename);
  if (dbeFile == NULL)
    {
      dbeFile = new DbeFile (filename);
      dbeFiles->put (filename, dbeFile);
    }
  dbeFile->filetype |= filetype;
  return dbeFile;
}

LoadObject *
DbeSession::get_Total_LoadObject ()
{
  if (lo_total == NULL)
    {
      lo_total = createLoadObject (NTXT ("<Total>"));
      lo_total->dbeFile->filetype |= DbeFile::F_FICTION;
    }
  return lo_total;
}

Function *
DbeSession::get_Total_Function ()
{
  if (f_total == NULL)
    {
      f_total = createFunction ();
      f_total->flags |= FUNC_FLAG_SIMULATED | FUNC_FLAG_NO_OFFSET;
      f_total->set_name (NTXT ("<Total>"));
      Module *mod = get_Total_LoadObject ()->noname;
      f_total->module = mod;
      mod->functions->append (f_total);
    }
  return f_total;
}

LoadObject *
DbeSession::get_Unknown_LoadObject ()
{
  if (lo_unknown == NULL)
    {
      lo_unknown = createLoadObject (GTXT ("<Unknown>"));
      lo_unknown->type = LoadObject::SEG_TEXT; // makes it expandable
      lo_unknown->dbeFile->filetype |= DbeFile::F_FICTION;

      // force creation of the <Unknown> function
      (void) get_Unknown_Function ();
    }
  return lo_unknown;
}

SourceFile *
DbeSession::get_Unknown_Source ()
{
  if (sf_unknown == NULL)
    {
      sf_unknown = createSourceFile (localized_SP_UNKNOWN_NAME);
      sf_unknown->dbeFile->filetype |= DbeFile::F_FICTION;
      sf_unknown->flags |= SOURCE_FLAG_UNKNOWN;
    }
  return sf_unknown;
}

Function *
DbeSession::get_Unknown_Function ()
{
  if (f_unknown == NULL)
    {
      f_unknown = createFunction ();
      f_unknown->flags |= FUNC_FLAG_SIMULATED;
      f_unknown->set_name (GTXT ("<Unknown>"));
      Module *mod = get_Unknown_LoadObject ()->noname;
      f_unknown->module = mod;
      mod->functions->append (f_unknown);
    }
  return f_unknown;
}

// LIBRARY_VISIBILITY

Function *
DbeSession::create_hide_function (LoadObject *lo)
{
  Function *h_function = createFunction ();
  h_function->set_name (lo->get_name ());
  h_function->module = lo->noname;
  h_function->isHideFunc = true;
  lo->noname->functions->append (h_function);
  return h_function;
}

Function *
DbeSession::get_JUnknown_Function ()
{
  if (j_unknown == NULL)
    {
      j_unknown = createFunction ();
      j_unknown->flags |= FUNC_FLAG_SIMULATED;
      j_unknown->set_name (GTXT ("<no Java callstack recorded>"));
      Module *mod = get_Unknown_LoadObject ()->noname;
      j_unknown->module = mod;
      mod->functions->append (j_unknown);
    }
  return j_unknown;
}

Function *
DbeSession::get_jvm_Function ()
{
  if (f_jvm == NULL)
    {
      f_jvm = createFunction ();
      f_jvm->flags |= FUNC_FLAG_SIMULATED | FUNC_FLAG_NO_OFFSET;
      f_jvm->set_name (GTXT ("<JVM-System>"));

      // Find the JVM LoadObject
      LoadObject *jvm = get_Unknown_LoadObject ();
      for (int i = 0; i < lobjs->size (); ++i)
	{
	  LoadObject *lo = lobjs->fetch (i);
	  if (lo->flags & SEG_FLAG_JVM)
	    {
	      jvm = lo;
	      break;
	    }
	}
      Module *mod = jvm->noname;
      f_jvm->module = mod;
      mod->functions->append (f_jvm);
      // XXXX is it required? no consistency among all special functions
      // jvm->functions->append( f_jvm );
    }
  return f_jvm;
}

Function *
DbeSession::getSpecialFunction (SpecialFunction kind)
{
  if (kind < 0 || kind >= LastSpecialFunction)
    return NULL;

  Function *func = f_special->fetch (kind);
  if (func == NULL)
    {
      char *fname;
      switch (kind)
	{
	case TruncatedStackFunc:
	  fname = GTXT ("<Truncated-stack>");
	  break;
	case FailedUnwindFunc:
	  fname = GTXT ("<Stack-unwind-failed>");
	  break;
	default:
	  return NULL;
	}
      func = createFunction ();
      func->flags |= FUNC_FLAG_SIMULATED | FUNC_FLAG_NO_OFFSET;
      Module *mod = get_Total_LoadObject ()->noname;
      func->module = mod;
      mod->functions->append (func);
      func->set_name (fname);
      f_special->store (kind, func);
    }
  return func;
}

LoadObject *
DbeSession::get_OMP_LoadObject ()
{
  if (lo_omp == NULL)
    {
      for (int i = 0, sz = lobjs->size (); i < sz; i++)
	{
	  LoadObject *lo = lobjs->fetch (i);
	  if (lo->flags & SEG_FLAG_OMP)
	    {
	      lo_omp = lo;
	      return lo_omp;
	    }
	}
      lo_omp = createLoadObject (GTXT ("<OMP>"));
      lo_omp->type = LoadObject::SEG_TEXT;
      lo_omp->dbeFile->filetype |= DbeFile::F_FICTION;
    }
  return lo_omp;
}

Function *
DbeSession::get_OMP_Function (int n)
{
  if (n < 0 || n >= OMP_LAST_STATE)
    return NULL;

  Function *func = omp_functions->fetch (n);
  if (func == NULL)
    {
      char *fname;
      switch (n)
	{
	case OMP_OVHD_STATE:
	  fname = GTXT ("<OMP-overhead>");
	  break;
	case OMP_IDLE_STATE:
	  fname = GTXT ("<OMP-idle>");
	  break;
	case OMP_RDUC_STATE:
	  fname = GTXT ("<OMP-reduction>");
	  break;
	case OMP_IBAR_STATE:
	  fname = GTXT ("<OMP-implicit_barrier>");
	  break;
	case OMP_EBAR_STATE:
	  fname = GTXT ("<OMP-explicit_barrier>");
	  break;
	case OMP_LKWT_STATE:
	  fname = GTXT ("<OMP-lock_wait>");
	  break;
	case OMP_CTWT_STATE:
	  fname = GTXT ("<OMP-critical_section_wait>");
	  break;
	case OMP_ODWT_STATE:
	  fname = GTXT ("<OMP-ordered_section_wait>");
	  break;
	case OMP_ATWT_STATE:
	  fname = GTXT ("<OMP-atomic_wait>");
	  break;
	default:
	  return NULL;
	}
      func = createFunction ();
      func->flags |= FUNC_FLAG_SIMULATED | FUNC_FLAG_NO_OFFSET;
      func->set_name (fname);

      LoadObject *omp = get_OMP_LoadObject ();
      func->module = omp->noname;
      omp->noname->functions->append (func);
      omp->functions->append (func);
      omp_functions->store (n, func);
    }
  return func;
}

// Divide the original createExperiment() into two steps
// In part1, we just create the data structure, in part2, if
// we decide to keep the experiment around, add it to various
// lists in DbeSession
Experiment *
DbeSession::createExperimentPart1 ()
{
  Experiment *exp = new Experiment ();
  return exp;
}

void
DbeSession::createExperimentPart2 (Experiment *exp)
{
  int ind = expGroups->size ();
  if (ind > 0)
    {
      ExpGroup *gr = expGroups->fetch (ind - 1);
      exp->groupId = gr->groupId;
      gr->append (exp);
    }
  exp->setExpIdx (exps->size ());
  exp->setUserExpId (++user_exp_id_counter);
  exps->append (exp);
}

Experiment *
DbeSession::createExperiment ()
{
  Experiment *exp = new Experiment ();
  append (exp);
  return exp;
}

void
DbeSession::append (Experiment *exp)
{
  exp->setExpIdx (exps->size ());
  exp->setUserExpId (++user_exp_id_counter);
  exps->append (exp);
  if (exp->founder_exp)
    {
      if (exp->founder_exp->children_exps == NULL)
	exp->founder_exp->children_exps = new Vector<Experiment *>;
      exp->founder_exp->children_exps->append (exp);
      if (exp->founder_exp->groupId > 0)
	{
	  exp->groupId = exp->founder_exp->groupId;
	  expGroups->get (exp->groupId - 1)->append (exp);
	}
    }
  if (exp->groupId == 0)
    {
      long ind = VecSize (expGroups);
      if (ind > 0)
	{
	  ExpGroup *gr = expGroups->get (ind - 1);
	  exp->groupId = gr->groupId;
	  gr->append (exp);
	}
    }
}

void
DbeSession::append (Hwcentry *h)
{
  if (hwcentries == NULL)
    hwcentries = new Vector<Hwcentry*>;
  hwcentries->append (h);
}

int
DbeSession::ngoodexps ()
{
  return exps->size ();
}

int
DbeSession::createView (int index, int cloneindex)
{
  // ensure that there is no view with that index
  DbeView *dbev = getView (index);
  if (dbev != NULL)
    abort ();

  // find the view to be cloned
  dbev = getView (cloneindex);
  DbeView *newview;
  if (dbev == NULL)
    newview = new DbeView (theApplication, settings, index);
  else
    newview = new DbeView (dbev, index);
  views->append (newview);
  return index;
}

DbeView *
DbeSession::getView (int index)
{
  int i;
  DbeView *dbev;
  Vec_loop (DbeView*, views, i, dbev)
  {
    if (dbev->vindex == index)
      return dbev;
  }
  return NULL;
}

void
DbeSession::dropView (int index)
{
  int i;
  DbeView *dbev;

  Vec_loop (DbeView*, views, i, dbev)
  {
    if (dbev->vindex == index)
      {
	views->remove (i);
	delete dbev;
	return;
      }
  }
  // view not found; ignore for now
}

Vector<char*> *
DbeSession::get_group_or_expt (char *path)
{
  Vector<char*> *exp_list = new Vector<char*>;
  FILE *fptr;
  char *new_path, buf[MAXPATHLEN], name[MAXPATHLEN];

  fptr = fopen (path, NTXT ("r"));
  if (!fptr || !fgets (buf, (int) sizeof (buf), fptr)
      || strncmp (buf, SP_GROUP_HEADER, strlen (SP_GROUP_HEADER)))
    {
      // it's not an experiment group
      new_path = dbe_strdup (path);
      new_path = canonical_path (new_path);
      exp_list->append (new_path);
    }
  else
    {
      // it is an experiment group, read the list to get them all
      while (fgets (buf, (int) sizeof (buf), fptr))
	{
	  if ((*buf != '#') && (sscanf (buf, NTXT ("%s"), name) == 1))
	    {
	      new_path = dbe_strdup (name);
	      new_path = canonical_path (new_path);
	      exp_list->append (new_path);
	    }
	}
    }
  if (fptr)
    fclose (fptr);
  return exp_list;
}

#define GET_INT_VAL(v, s, len) \
    for (v = len = 0; isdigit(*s); s++, len++) { v = v * 10 + (*s -'0'); }

static int
dir_name_cmp (const void *a, const void *b)
{
  char *s1 = *((char **) a);
  char *s2 = *((char **) b);
  while (*s1)
    {
      if (isdigit (*s1) && isdigit (*s2))
	{
	  int v1, v2, len1, len2;
	  GET_INT_VAL (v1, s1, len1);
	  GET_INT_VAL (v2, s2, len2);
	  if (v1 != v2)
	    return v1 - v2;
	  if (len1 != len2)
	    return len2 - len1;
	  continue;
	}
      if (*s1 != *s2)
	break;
      s1++;
      s2++;
    }
  return *s1 - *s2;
}

static int
read_experiment_data_in_parallel (void *arg)
{
  exp_ctx *ctx = (exp_ctx *) arg;
  Experiment *dexp = ctx->exp;
  bool read_ahead = ctx->read_ahead;
  dexp->read_experiment_data (read_ahead);
  free (ctx);
  return 0;
}

void
DbeSession::open_experiment (Experiment *exp, char *path)
{
  exp->open (path);
  if (exp->get_status () != Experiment::FAILURE)
    exp->read_experiment_data (false);
  exp->open_epilogue ();

  // Update all DbeViews
  for (int i = 0, sz = views->size (); i < sz; i++)
    {
      DbeView *dbev = views->fetch (i);
      dbev->add_experiment (exp->getExpIdx (), true);
    }

  if (exp->get_status () == Experiment::FAILURE)
    {
      check_tab_avail ();
      return;
    }

  char *discard_tiny = getenv (NTXT ("SP_ANALYZER_DISCARD_TINY_EXPERIMENTS"));
  int user_specified_tiny_threshold = DEFAULT_TINY_THRESHOLD; // in milliseconds
  if (discard_tiny != NULL)
    {
      user_specified_tiny_threshold = (atoi (discard_tiny));
      if (user_specified_tiny_threshold < 0)
	user_specified_tiny_threshold = DEFAULT_TINY_THRESHOLD;
    }

  // Open descendant experiments
  DIR *exp_dir = opendir (path);
  if (exp_dir == NULL)
    {
      check_tab_avail ();
      return;
    }

  Vector<char*> *exp_names = new Vector<char*>();
  struct dirent *entry = NULL;
  while ((entry = readdir (exp_dir)) != NULL)
    {
      if (entry->d_name[0] != '_')
	continue;
      size_t len = strlen (entry->d_name);
      if (len < 3 || strcmp (entry->d_name + len - 3, NTXT (".er")) != 0)
	continue;
      exp_names->append (dbe_strdup (entry->d_name));
    }
  closedir (exp_dir);
  exp_names->sort (dir_name_cmp);
  Experiment **t_exp_list = new Experiment *[exp_names->size ()];
  int nsubexps = 0;

  for (int j = 0, jsz = exp_names->size (); j < jsz; j++)
    {
      t_exp_list[j] = NULL;

      char *lineage_name = exp_names->fetch (j);
      struct stat64 sbuf;
      char *dpath = dbe_sprintf (NTXT ("%s/%s"), path, lineage_name);

      // look for experiments with no profile collected
      if (user_specified_tiny_threshold == DEFAULT_TINY_THRESHOLD)
	{
	  char *frinfoname = dbe_sprintf (NTXT ("%s/%s"), dpath, "data." SP_FRINFO_FILE);
	  int st = dbe_stat (frinfoname, &sbuf);
	  free (frinfoname);
	  if (st == 0)
	    {
	      // if no profile/trace data do not process this experiment any further
	      if (sbuf.st_size == 0)
		{
		  free (dpath);
		  continue;
		}
	    }
	}
      else
	{ // check if dpath is a directory
	  if (dbe_stat (dpath, &sbuf) != 0)
	    {
	      free (dpath);
	      continue;
	    }
	  else if (!S_ISDIR (sbuf.st_mode))
	    {
	      free (dpath);
	      continue;
	    }
	}
      size_t lineage_name_len = strlen (lineage_name);
      lineage_name[lineage_name_len - 3] = 0; /* remove .er */
      Experiment *dexp = new Experiment ();
      dexp->founder_exp = exp;
      if (user_specified_tiny_threshold > DEFAULT_TINY_THRESHOLD)
	{
	  dexp->setTinyThreshold (user_specified_tiny_threshold);
	  dexp->open (dpath);
	  if (dexp->isDiscardedTinyExperiment ())
	    {
	      delete dexp;
	      free (dpath);
	      continue;
	    }
	}
      else
	dexp->open (dpath);
      append (dexp);
      t_exp_list[j] = dexp;
      nsubexps++;
      dexp->set_clock (exp->clock);

      // DbeView add_experiment() is split into two parts
      // add_subexperiment() is called repeeatedly for
      // all sub_experiments, later add_experiment_epilogue() finishes up the task
      for (int i = 0, sz = views->size (); i < sz; i++)
	{
	  DbeView *dbev = views->fetch (i);
	  bool enabled = settings->check_en_desc (lineage_name, dexp->utargname);
	  dbev->add_subexperiment (dexp->getExpIdx (), enabled);
	}
      free (dpath);
    }

  for (int i = 0, sz = views->size (); i < sz; i++)
    {
      DbeView *dbev = views->fetch (i);
      dbev->add_experiment_epilogue ();
    }

  DbeThreadPool * threadPool = new DbeThreadPool (-1);
  for (int j = 0, jsz = exp_names->size (); j < jsz; j++)
    {
      if (t_exp_list[j] == NULL) continue;
      Experiment *dexp = t_exp_list[j];
      exp_ctx *new_ctx = (exp_ctx*) malloc (sizeof (exp_ctx));
      new_ctx->path = NULL;
      new_ctx->exp = dexp;
      new_ctx->ds = this;
      new_ctx->read_ahead = true;
      DbeQueue *q = new DbeQueue (read_experiment_data_in_parallel, new_ctx);
      threadPool->put_queue (q);
    }
  threadPool->wait_queues ();
  delete threadPool;

  for (long j = 0, jsz = exp_names->size (); j < jsz; j++)
    {
      if (t_exp_list[j] == NULL) continue;
      Experiment *dexp = t_exp_list[j];
      dexp->open_epilogue ();
    }
  exp_names->destroy ();
  delete[] t_exp_list;
  delete exp_names;

  // update setting for leaklist and dataspace
  check_tab_avail ();
}

void
DbeSession::append_mesgs (StringBuilder *sb, char *path, Experiment *exp)
{
  if (exp->fetch_errors () != NULL)
    {
      // yes, there were errors
      char *ststr = pr_mesgs (exp->fetch_errors (), NTXT (""), NTXT (""));
      sb->append (path);
      sb->append (NTXT (": "));
      sb->append (ststr);
      free (ststr);
    }

  Emsg *m = exp->fetch_warnings ();
  if (m != NULL)
    {
      sb->append (path);
      sb->append (NTXT (": "));
      if (!is_interactive ())
	sb->append (GTXT ("Experiment has warnings, see header for details\n"));
      else
	sb->append (GTXT ("Experiment has warnings, see experiment panel for details\n"));
    }

  // Check for descendant experiments that are not loaded
  int num_desc = VecSize (exp->children_exps);
  if ((num_desc > 0) && !settings->check_en_desc (NULL, NULL))
    {
      char *s;
      if (!is_interactive ())
	s = dbe_sprintf (GTXT ("Has %d descendant(s), use commands controlling selection to load descendant data\n"), num_desc);
      else
	s = dbe_sprintf (GTXT ("Has %d descendant(s), use filter panel to load descendant data\n"), num_desc);
      sb->append (path);
      sb->append (NTXT (": "));
      sb->append (s);
      free (s);
    }
}

Experiment *
DbeSession::get_exp (int exp_ind)
{
  if (exp_ind < 0 || exp_ind >= exps->size ())
    return NULL;
  Experiment *exp = exps->fetch (exp_ind);
  exp->setExpIdx (exp_ind);
  return exp;
}

Vector<Vector<char*>*> *
DbeSession::getExperimensGroups ()
{
  if (dbeSession->expGroups == NULL || dbeSession->expGroups->size () == 0)
    return NULL;
  bool compare_mode = expGroups->size () > 1;
  Vector<Vector<char*>*> *groups = new Vector<Vector<char*>*> (
					 compare_mode ? expGroups->size () : 1);
  for (int i = 0; i < expGroups->size (); i++)
    {
      ExpGroup *grp = expGroups->fetch (i);
      Vector<Experiment*> *founders = grp->get_founders ();
      if (founders && founders->size () != 0)
	{
	  Vector<char *> *names = new Vector<char*> (founders->size ());
	  for (int j = 0; j < founders->size (); j++)
	    {
	      Experiment *exp = founders->fetch (j);
	      names->append (dbe_strdup (exp->get_expt_name ()));
	    }
	  if (compare_mode || groups->size () == 0)
	    groups->append (names);
	  else
	    groups->fetch (0)->addAll (names);
	}
      delete founders;
    }
  return groups;
}

char *
DbeSession::setExperimentsGroups (Vector<Vector<char*>*> *groups)
{
  StringBuilder sb;
  for (int i = 0; i < groups->size (); i++)
    {
      Vector<char *> *names = groups->fetch (i);
      ExpGroup *grp;
      if (names->size () == 1)
	grp = new ExpGroup (names->fetch (0));
      else
	{
	  char *nm = dbe_sprintf (GTXT ("Group %d"), i + 1);
	  grp = new ExpGroup (nm);
	  free (nm);
	}
      expGroups->append (grp);
      grp->groupId = expGroups->size ();

      for (int j = 0; j < names->size (); j++)
	{
	  char *path = names->fetch (j);
	  size_t len = strlen (path);
	  if ((len > 4) && !strcmp (path + len - 4, NTXT (".erg")))
	    {
	      Vector<char*> *lst = get_group_or_expt (path);
	      for (int j1 = 0; j1 < lst->size (); j1++)
		{
		  Experiment *exp = new Experiment ();
		  append (exp);
		  open_experiment (exp, lst->get (j1));
		  if (exp->get_status () == Experiment::FAILURE)
		    append_mesgs (&sb, path, exp);
		}
	      lst->destroy ();
	      delete lst;
	    }
	  else
	    {
	      Experiment *exp = new Experiment ();
	      append (exp);
	      open_experiment (exp, path);
	      if (exp->get_status () == Experiment::FAILURE)
		append_mesgs (&sb, path, exp);
	    }
	}
    }

  for (int i = 0, sz = views->size (); i < sz; i++)
    {
      DbeView *dbev = views->fetch (i);
      dbev->update_advanced_filter ();
      int cmp = dbev->get_settings ()->get_compare_mode ();
      dbev->set_compare_mode (CMP_DISABLE);
      dbev->set_compare_mode (cmp);
    }
  return sb.length () == 0 ? NULL : sb.toString ();
}

char *
DbeSession::drop_experiment (int exp_ind)
{
  DbeView *dbev;
  int index;
  Experiment *exp2;

  status_ompavail = -1;
  Experiment *exp = exps->fetch (exp_ind);

  // If this is a sub experiment, don't do it
  if (exp->founder_exp != NULL)     // this is a sub experiment; don't do it
    return (dbe_strdup (GTXT ("Can not drop subexperiments")));

  if (VecSize (exp->children_exps) > 0)
    for (;;)
      {
	// search the list of experiments to find all that have this one as founder
	bool found = false;
	Vec_loop (Experiment*, exps, index, exp2)
	{
	  if (exp2->founder_exp == exp)
	    {
	      exp2->founder_exp = NULL;
	      drop_experiment (index);
	      found = true;
	      break;
	    }
	}
	if (found == false)
	  break;
      }

  // then proceed to finish the drop
  Vec_loop (DbeView*, views, index, dbev)
  {
    dbev->drop_experiment (exp_ind);
  }

  int old_cnt = expGroups->size ();
  for (int i = 0; i < old_cnt; i++)
    {
      ExpGroup *gr = expGroups->fetch (i);
      if (gr->groupId == exp->groupId)
	{
	  gr->drop_experiment (exp);
	  if ((gr->founder == NULL) && (gr->exps->size () == 0))
	    {
	      delete gr;
	      expGroups->remove (i);
	    }
	  break;
	}
    }
  delete exps->remove (exp_ind);
  if (old_cnt != expGroups->size ())
    {
      for (int i = 0, sz = expGroups->size (); i < sz; i++)
	{
	  ExpGroup *gr = expGroups->fetch (i);
	  gr->groupId = i + 1;
	  Vector<Experiment*> *expList = gr->exps;
	  for (int i1 = 0, sz1 = expList->size (); i1 < sz1; i1++)
	    expList->fetch (i1)->groupId = gr->groupId;
	}
      for (int i = 0, sz = views->size (); i < sz; i++)
	{
	  dbev = views->fetch (i);
	  int cmp = dbev->get_compare_mode ();
	  dbev->set_compare_mode (CMP_DISABLE);
	  dbev->set_compare_mode (cmp);
	}
    }
  check_tab_avail ();   // update tab availability
  return NULL;
}

int
DbeSession::find_experiment (char *path)
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (strcmp (exp->get_expt_name (), path) == 0)
      return exp->getExpIdx ();
  }
  return -1;
}

LoadObject *
DbeSession::createLoadObject (const char *nm, int64_t cksum)
{
  return loadObjMap->sync_create_item (nm, cksum);
}

LoadObject *
DbeSession::createLoadObject (const char *nm, const char *runTimePath, DbeFile *df)
{
  return loadObjMap->sync_create_item (nm, runTimePath, df);
}

void
DbeSession::append (LoadObject *lobj)
{
  Histable *obj = lobj; // workaround for a C++ problem
  objs->append (obj);
  lobj->id = objs->size () - 1;
  lobjs->append (lobj);
  lobj->seg_idx = lobjs->size () - 1;
  char *loname = lobj->get_pathname ();
  dbeFiles->put (loname, lobj->dbeFile);
}

DbeJarFile *
DbeSession::get_JarFile (const char *name)
{
  DbeJarFile *jf = dbeJarFiles->get (name);
  if (jf == NULL)
    {
      jf = new DbeJarFile (name);
      dbeJarFiles->put (name, jf);
    }
  return jf;
}

Module *
DbeSession::createModule (LoadObject *lo, const char *nm)
{
  Module *mod = new Module ();
  Histable *obj = mod; // workaround for a C++ problem
  objs->append (obj);
  mod->id = objs->size () - 1;
  mod->loadobject = lo;
  mod->set_name (dbe_strdup (nm ? nm : localized_SP_UNKNOWN_NAME));
  lo->seg_modules->append (mod);
  return mod;
}

Module *
DbeSession::createUnknownModule (LoadObject *lo)
{
  Module *mod = createModule (lo, localized_SP_UNKNOWN_NAME);
  mod->flags |= MOD_FLAG_UNKNOWN;
  mod->set_file_name (dbe_strdup (localized_SP_UNKNOWN_NAME));
  return mod;
}

SourceFile *
DbeSession::createSourceFile (const char *_path)
{
  char *path = (char *) _path;
  if (strncmp (path, NTXT ("./"), 2) == 0)
    path += 2;
  SourceFile *source = sourcesMap->get (path);
  if (source == NULL)
    {
      source = new SourceFile (path);
      (void) sourcesMap->put (path, source);
      append (source);
    }
  return source;
}

Function *
DbeSession::createFunction ()
{
  Function *func = new Function (objs->size ());
  Histable *obj = func; // workaround for a C++ problem
  objs->append (obj);
  return func;
}

JMethod *
DbeSession::createJMethod ()
{
  JMethod *jmthd = new JMethod (objs->size ());
  Histable *obj = jmthd; // workaround for a C++ problem
  objs->append (obj);
  return jmthd;
}

Module *
DbeSession::createClassFile (char *className)
{
  ClassFile *cls = new ClassFile ();
  cls->set_name (className);
  char *clpath = cls->get_java_file_name (className, true);
  cls->dbeFile = getDbeFile (clpath, DbeFile::F_JAVACLASS);
  free (clpath);
  Histable *obj = cls; // workaround for a C++ problem
  objs->append (obj);
  cls->id = objs->size () - 1;
  return cls;
}

Histable *
DbeSession::createHistObject (Histable::Type type)
{
  switch (type)
    {
    case Histable::DOBJECT:
      {
	DataObject *dataobj = new DataObject ();
	dobjs->append (dataobj);
	dataobj->id = dobjs->size () - 1;
	return dataobj;
      }
    default:
      assert (0);
    }
  return NULL;
}

DataObject *
DbeSession::createDataObject ()
{
  DataObject *dataobj = new DataObject ();
  dobjs->append (dataobj);
  dataobj->id = dobjs->size () - 1;
  return dataobj;
}

DataObject *
DbeSession::createDataObject (DataObject *dobj, DataObject *parent)
{
  DataObject *dataobj = new DataObject ();
  dataobj->size = dobj->size;
  dataobj->offset = dobj->offset;
  dataobj->parent = parent;
  dataobj->set_dobjname (dobj->get_typename (), dobj->get_instname ());
  dobjs->append (dataobj);
  dataobj->id = dobjs->size () - 1;
  return dataobj;
}

DataObject *
DbeSession::createMasterDataObject (DataObject *dobj)
{
  DataObject *parent = NULL;
  if (dobj->parent)
    { // define master parent first
      parent = find_dobj_master (dobj->parent);
      if (!parent)
	{ // clone master from this dataobject parent
	  parent = createDataObject (dobj->parent);
	  parent->scope = NULL; // master is scope-less
	  Dprintf (DEBUG_DATAOBJ,
		   "Master DataObject(%llu) cloned from (%llu) %s\n",
		   (ull_t) parent->id, (ull_t) dobj->parent->id,
		   dobj->parent->get_name ());
	  // clone master DataObject elements
	  Vector<DataObject*> *delem = get_dobj_elements (dobj->parent);
	  int element_index = 0;
	  DataObject *element = NULL;
	  Vec_loop (DataObject*, delem, element_index, element)
	  {
	    DataObject *master_element = createDataObject (element, parent);
	    master_element->scope = NULL; // master is scope-less
	    Dprintf (DEBUG_DATAOBJ,
		     "Member DataObject(%llu) cloned from (%llu) %s\n",
		     (ull_t) master_element->id, (ull_t) element->id,
		     element->get_name ());
	  }
	}
      else
	Dprintf (DEBUG_DATAOBJ, "Master DataObject(%llu) clone found (%llu) %s\n",
		 (ull_t) parent->id, (ull_t) dobj->parent->id,
		 dobj->parent->get_name ());
    }

  DataObject *master = find_dobj_master (dobj);
  if (!master)
    { // clone master from this dataobject
      master = createDataObject (dobj, parent);
      master->scope = NULL; // master is scope-less
      Dprintf (DEBUG_DATAOBJ, "Master DataObject(%llu) cloned from (%llu) %s\n",
	       (ull_t) master->id, (ull_t) dobj->id, dobj->get_name ());
    }
  else
    Dprintf (DEBUG_DATAOBJ, "Master DataObject(%llu) clone found (%llu) %s\n",
	     (ull_t) master->id, (ull_t) dobj->id, dobj->get_name ());
  return master;
}

void
DbeSession::insert_metric (BaseMetric *mtr, Vector<BaseMetric*> *mlist)
{
  if ((mtr->get_flavors () & Metric::STATIC) == 0)
    {
      // insert in front of the first STATIC
      for (int i = 0, mlist_sz = mlist->size (); i < mlist_sz; i++)
	{
	  BaseMetric *m = mlist->fetch (i);
	  if (m->get_flavors () & Metric::STATIC)
	    {
	      mlist->insert (i, mtr);
	      return;
	    }
	}
    }
  mlist->append (mtr);
}

BaseMetricTreeNode*
DbeSession::get_reg_metrics_tree ()
{
  if (reg_metrics_tree == NULL)
    // Can't init earlier because BaseMetric() requires DbeSession::ql_parse
    reg_metrics_tree = new BaseMetricTreeNode ();
  return reg_metrics_tree;
}

void
DbeSession::update_metric_tree (BaseMetric *m)
{
  get_reg_metrics_tree ()->register_metric (m);
}

BaseMetric *
DbeSession::register_metric_expr (BaseMetric::Type type, char *cmd, char *expr_spec)
{
  BaseMetric *m = find_metric (type, cmd, expr_spec);
  if (m)
    return m;
  BaseMetric *bm = find_metric (type, cmd, NULL); // clone this version
  m = new BaseMetric (*bm);
  m->set_expr_spec (expr_spec);
  insert_metric (m, reg_metrics);
  return m;
}

BaseMetric *
DbeSession::register_metric (BaseMetric::Type type)
{
  BaseMetric *m = find_metric (type, NULL, NULL);
  if (m)
    return m;
  m = new BaseMetric (type);
  insert_metric (m, reg_metrics);
  update_metric_tree (m);
  return m;
}

BaseMetric *
DbeSession::register_metric (Hwcentry *ctr, const char* aux, const char* username)
{
  BaseMetric *m = find_metric (BaseMetric::HWCNTR, aux, NULL);
  if (m)
    // That may be a problem when metrics aren't an exact match.
    // For example, memoryspace is disabled in one experiment and not in another.
    return m;
  if (ctr->timecvt)
    {
      char *time_cmd = dbe_sprintf (NTXT ("t%s"), aux);
      char *time_username = dbe_sprintf (GTXT ("%s Time"),
				       ctr->metric ? ctr->metric :
				       (ctr->name ? ctr->name : ctr->int_name));
      BaseMetric *m1;
      if (ipc_mode)
	{
	  // Two visible metrics are presented in GUI
	  m1 = new BaseMetric (ctr, aux, time_cmd, time_username, VAL_TIMEVAL);
	  insert_metric (m1, reg_metrics);
	  update_metric_tree (m1);
	  m = new BaseMetric (ctr, aux, username, VAL_VALUE, m1);
	}
      else
	{
	  // Only one visible metric is presented in er_print
	  m1 = new BaseMetric (ctr, aux, time_cmd, time_username, VAL_TIMEVAL | VAL_INTERNAL);
	  insert_metric (m1, reg_metrics);
	  m = new BaseMetric (ctr, aux, username, VAL_TIMEVAL | VAL_VALUE, m1);
	}
      free (time_cmd);
      free (time_username);
    }
  else
    m = new BaseMetric (ctr, aux, username, VAL_VALUE);
  insert_metric (m, reg_metrics);
  update_metric_tree (m);
  return m;
}

BaseMetric *
DbeSession::register_metric (char *name, char *username, char *_def)
{
  BaseMetric *m = find_metric (BaseMetric::DERIVED, name, NULL);
  if (m)
    return m;
  Definition *p = Definition::add_definition (_def);
  if (p == NULL)
    return NULL;
  m = new BaseMetric (name, username, p);
  insert_metric (m, reg_metrics);
  update_metric_tree (m);
  return m;
}

void
DbeSession::drop_metric (BaseMetric *mtr)
{
  Countable *cnt;
  int index;

  Vec_loop (Countable*, metrics, index, cnt)
  {
    if (mtr == (BaseMetric *) cnt->item)
      {
	cnt->ref_count--;
	if (cnt->ref_count == 0)
	  {
	    // Remove this metric from all views
	    DbeView *dbev;
	    int index2;
	    Vec_loop (DbeView*, views, index2, dbev)
	    {
	      dbev->reset_metrics ();
	    }
	    delete metrics->remove (index);
	    delete mtr;
	    return;
	  }
      }
  }
}

BaseMetric *
DbeSession::find_metric (BaseMetric::Type type, const char *cmd, const char *expr_spec)
{
  for (int i = 0, sz = reg_metrics->size (); i < sz; i++)
    {
      BaseMetric *bm = reg_metrics->fetch (i);
      if (bm->get_type () == type && dbe_strcmp (bm->get_expr_spec (), expr_spec) == 0)
	{
	  if ((type == BaseMetric::DERIVED || type == BaseMetric::HWCNTR)
	       && dbe_strcmp (bm->get_cmd (), cmd) != 0)
	    continue;
	  return bm;
	}
    }
  return NULL;
}

BaseMetric *
DbeSession::find_base_reg_metric (char * mcmd)
{
  for (int i = 0, sz = reg_metrics->size (); i < sz; i++)
    {
      BaseMetric *bm = reg_metrics->fetch (i);
      if (bm->get_expr_spec () != NULL)
	continue; // skip compare metrics
      if (dbe_strcmp (bm->get_cmd (), mcmd) == 0)
	return bm;
    }
  return NULL;
}

Vector<BaseMetric*> *
DbeSession::get_base_reg_metrics ()
{
  Vector<BaseMetric*> *mlist = new Vector<BaseMetric*>;
  Vector<BaseMetric*> *ml = get_all_reg_metrics ();
  for (int i = 0, sz = ml->size (); i < sz; i++)
    {
      BaseMetric *m = ml->fetch (i);
      if (m->get_expr_spec () == NULL)
	mlist->append (m);
    }
  return mlist;
}

void
DbeSession::check_tab_avail ()
{
  DbeView *dbev;
  int index;
  // tell the views to update their tab lists
  Vec_loop (DbeView*, views, index, dbev)
  {
    dbev->get_settings ()->updateTabAvailability ();
  }
}

bool
DbeSession::is_datamode_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->dataspaceavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_leaklist_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->leaklistavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_heapdata_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->heapdataavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_iodata_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->iodataavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_racelist_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->racelistavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_deadlocklist_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->deadlocklistavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_timeline_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->timelineavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_ifreq_available ()
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    if (exp->ifreqavail)
      return true;
  }
  return false;
}

bool
DbeSession::is_omp_available ()
{
  if (status_ompavail == -1)
    {
      status_ompavail = 0;
      for (int i = 0, sz = exps ? exps->size () : 0; i < sz; i++)
	{
	  Experiment *exp = exps->fetch (i);
	  if (exp->ompavail)
	    {
	      status_ompavail = 1;
	      break;
	    }
	}
    }
  return status_ompavail == 1;
}

bool
DbeSession::has_java ()
{
  int status_has_java = 0;
  for (int i = 0, sz = exps ? exps->size () : 0; i < sz; i++)
    {
      Experiment *exp = exps->fetch (i);
      if (exp->has_java)
	{
	  status_has_java = 1;
	  break;
	}
    }
  return status_has_java == 1;
}

bool
DbeSession::has_ompavail ()
{
  int status_has_ompavail = 0;
  for (int i = 0, sz = exps ? exps->size () : 0; i < sz; i++)
    {
      Experiment *exp = exps->fetch (i);
      if (exp->ompavail)
	{
	  status_has_ompavail = 1;
	  break;
	}
    }
  return status_has_ompavail == 1;
}

int
DbeSession::get_clock (int whichexp)
{
  // XXXX clock frequency should be an attribute of each CPU,
  // XXX  and not a property of the session
  // if whichexp is -1, pick the first exp that has a clock
  // otherwise return the clock from the numbered experiment
  Experiment *exp;
  if (whichexp != -1)
    {
      exp = get_exp (whichexp);
      if (exp != NULL)
	return exp->clock;
      return 0;
    }
  int n = nexps ();
  for (int i = 0; i < n; i++)
    {
      exp = get_exp (i);
      if (exp != NULL && exp->clock != 0)
	return exp->clock;
    }
  return 0;
}

LoadObject *
DbeSession::find_lobj_by_name (const char *lobj_name, int64_t cksum)
{
  return loadObjMap->get (lobj_name, cksum);
}

static unsigned
hash (char *s)
{
  unsigned res = 0;
  for (int i = 0; i < 64 && *s; i++)
    res = res * 13 + *s++;
  return res;
}

// This method is introduced to fix performance
// problems with the data space profiling in the
// current release. A better design is desired.
void
DbeSession::dobj_updateHT (DataObject *dobj)
{
  unsigned index = hash (dobj->get_unannotated_name ()) % HTableSize;
  List *list = new List;
  list->val = (void*) dobj;
  list->next = dnameHTable[index];
  dnameHTable[index] = list;
}

DataObject *
DbeSession::find_dobj_by_name (char *dobj_name)
{
  unsigned index = hash (dobj_name) % HTableSize;
  List *list = dnameHTable[index];
  for (; list; list = list->next)
    {
      DataObject *d = (DataObject*) list->val;
      if (strcmp (d->get_unannotated_name (), dobj_name) == 0)
	return d;
    }
  return (DataObject *) NULL;
}

DataObject *
DbeSession::find_dobj_match (DataObject *dobj)
{
  char *dobj_name = dobj->get_unannotated_name ();
  unsigned index = hash (dobj_name) % HTableSize;
  List *list = dnameHTable[index];
  for (; list; list = list->next)
    {
      DataObject *d = (DataObject*) list->val;
      if (strcmp (d->get_unannotated_name (), dobj_name) == 0
	  && d->size == dobj->size && d->offset == dobj->offset
	  && d->scope == dobj->scope)
	return d;
    }
  return (DataObject *) NULL;
}

DataObject *
DbeSession::find_dobj_master (DataObject *dobj)
{
  char *dobj_name = dobj->get_unannotated_name ();
  unsigned index = hash (dobj_name) % HTableSize;
  List *list = dnameHTable[index];
  for (; list; list = list->next)
    {
      DataObject *d = (DataObject*) list->val;
      // XXXX should parent also match?
      if (strcmp (d->get_unannotated_name (), dobj_name) == 0
	  && d->size == dobj->size && d->offset == dobj->offset
	  && d->master == NULL && d->scope == NULL)
	return d;
    }
  return (DataObject *) NULL;
}

Vector<DataObject*>*
DbeSession::get_dobj_elements (DataObject *dobj)
{
  DataObject *d;
  int index;
  Vector<DataObject*> *elements = new Vector<DataObject*>;
  if (dobj == d_total)
    return elements;
  Vec_loop (DataObject*, dobjs, index, d)
  {
    if (d->get_parent () && d->get_parent () == dobj)
      elements->append (d);
  }
  return elements;
}

Vector<LoadObject*>*
DbeSession::get_text_segments ()
{
  LoadObject *lo;
  int index;
  Vector<LoadObject*> *tlobjs = new Vector<LoadObject*>;
  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    if (lo->type == LoadObject::SEG_TEXT)
      tlobjs->append (lo);
  }
  return tlobjs;
}

static long long
getNumber (const char *s, char * &last)
{
  long long val;
  char *sp;
  errno = 0;
  val = strtoll (s, &sp, 0);
  if (errno == EINVAL)
    last = NULL;
  else
    {
      while (isspace (*sp))
	sp++;
      last = sp;
    }
  return (val);
}

bool
DbeSession::find_obj (FILE *dis_file, FILE *inp_file, Histable *&obj,
		      char *name, const char *sel, Histable::Type type, bool xdefault)
{
  Vector<Histable*> *obj_lst;
  int which = -1;
  char *last = NULL;
  if (type != Histable::FUNCTION && sel)
    {
      // check that a number has been provided
      which = (int) getNumber (sel, last);
      if (last == NULL || *last != '\0')
	{
	  fprintf (stderr, GTXT ("Error: Invalid number entered: %s\n"), sel);
	  sel = NULL;
	  which = 0;
	}
      which--;
    }
  obj_lst = new Vector<Histable*>;
  switch (type)
    {
    case Histable::FUNCTION:
      obj = map_NametoFunction (name, obj_lst, sel);
      break;
    case Histable::MODULE:
      obj = map_NametoModule (name, obj_lst, which);
      break;
    case Histable::LOADOBJECT:
      obj = map_NametoLoadObject (name, obj_lst, which);
      break;
    case Histable::DOBJECT:
      obj = map_NametoDataObject (name, obj_lst, which);
      break;
    default:
      abort (); // unexpected Histable!
    }

  if ((obj == NULL) && (obj_lst->size () > 0))
    {
      if (obj_lst->size () == 1)
	which = 0;
      else
	{
	  if (sel && (which < 0 || which >= obj_lst->size ()))
	    fprintf (stderr, GTXT ("Error: Invalid number entered: %s\n"), sel);
	  if (xdefault)
	    {
	      fprintf (stderr, GTXT ("Default selection \"1\" made\n"));
	      which = 0;
	    }
	  else
	    {
	      which = ask_which (dis_file, inp_file, obj_lst, name);
	      if (which == -1)
		{
		  delete obj_lst;
		  return false;
		}
	    }
	}
      obj = obj_lst->fetch (which);
    }
  delete obj_lst;
  return true;
}

int
DbeSession::ask_which (FILE *dis_file, FILE *inp_file,
		       Vector<Histable*> *list, char *name)
{
  Histable *hitem;
  Function *func;
  Module *module;
  int which, index, index1;
  char *item_name, *lo_name, *fname, *last;
  char buf[BUFSIZ];
  for (;;)
    {
      fprintf (dis_file, GTXT ("Available name list:\n"));
      fprintf (dis_file, GTXT ("%8d) Cancel\n"), 0);
      Vec_loop (Histable*, list, index, hitem)
      {
	index1 = index + 1;
	item_name = hitem->get_name ();
	switch (hitem->get_type ())
	  {
	  case Histable::FUNCTION:
	    func = (Function *) hitem;
	    module = func->module;

	    // id == -1 indicates er_src invocation
	    if (module == NULL || (module->lang_code == Sp_lang_java
				   && module->loadobject->id == -1))
		fprintf (dis_file, NTXT ("%8d) %s\n"), index1, item_name);
	    else
	      {
		lo_name = module->loadobject->get_pathname ();
		fname = (module->file_name && *module->file_name) ?
			module->file_name : module->get_name ();
		if (fname && *fname)
		  fprintf (dis_file, NTXT ("%8d) %s %s:0x%llx (%s)\n"), index1,
			   item_name, lo_name, (ull_t) func->img_offset, fname);
		else
		  fprintf (dis_file, NTXT ("%8d) %s %s:0x%llx\n"), index1,
			   item_name, lo_name, (ull_t) func->img_offset);
	      }
	    break;
	  case Histable::MODULE:
	    module = (Module *) hitem;
	    lo_name = module->loadobject->get_pathname ();
	    if (name[strlen (name) - 1] ==
		module->file_name[strlen (module->file_name) - 1])
	      fprintf (dis_file, NTXT ("%8d) %s(%s)\n"), index1,
		       module->file_name, lo_name);
	    else
	      fprintf (dis_file, NTXT ("%8d) %s(%s)\n"), index1, item_name,
		       lo_name);
	    break;
	  default:
	    fprintf (dis_file, NTXT ("%8d) %s\n"), index1, item_name);
	    break;
	  }
      }
      if (inp_file != stdin)
	return -1;
      fprintf (dis_file, GTXT ("Enter selection: "));
      if (fgets (buf, (int) sizeof (buf), inp_file) == NULL)
	{
	  fprintf (stderr, GTXT ("Error: Invalid number entered:\n"));
	  return -1;
	}
      which = (int) getNumber (buf, last);
      if (last && *last == '\0')
	if (which >= 0 && which <= list->size ())
	  return which - 1;
      fprintf (stderr, GTXT ("Error: Invalid number entered: %s\n"), buf);
    }
}

static bool
match_basename (char *name, char *full_name, int len = -1)
{
  if (full_name == NULL)
    return false;
  if (!strchr (name, '/'))
    full_name = get_basename (full_name);
  if (len == -1)
    return streq (name, full_name);
  return strncmp (name, full_name, len) == 0;
}

LoadObject *
DbeSession::map_NametoLoadObject (char *name, Vector<Histable*> *list, int which)
{
  // Search the tree to find the first module whose module name
  //	matches "name" or whose source file name matches "name"
  //  Issues: is the name a pathname, or a base name?
  //	Should we look at suffix to disambiguate?
  LoadObject *loitem;
  int index;
  Vec_loop (LoadObject*, lobjs, index, loitem)
  {
    // try pathname first
    // if failed, try object name next
    if (match_basename (name, loitem->get_pathname ()) ||
	match_basename (name, loitem->get_name ()))
      {
	if (which == list->size ())
	  return loitem;
	list->append (loitem);
      }
  }
  return (LoadObject *) NULL;
}

Module *
DbeSession::map_NametoModule (char *name, Vector<Histable*> *list, int which)
{
  // Search the tree to find the first loadobject whose loadobject name
  //	matches "name".

  //  Issues: is the name a pathname, or a base name?
  //	Should we look at suffix to disambiguate?
  LoadObject *loitem;
  Module *mitem;
  int index1, index2;
  Vec_loop (LoadObject*, lobjs, index1, loitem)
  {
    Vec_loop (Module*, loitem->seg_modules, index2, mitem)
    {
      // try source name first
      // if failed, try object name next
      if (match_basename (name, mitem->file_name) ||
	  match_basename (name, mitem->get_name ()))
	{
	  if (which == list->size ())
	    return mitem;
	  list->append (mitem);
	}
    }
  }
  return (Module *) NULL;
}

Function *
DbeSession::map_NametoFunction (char *name, Vector<Histable*> *list,
				const char *sel)
{
  // Search the tree to find the first function whose
  //	name matches "name".
  //  Issues: is the name a full name, or a short name?
  //	Is it a demangled name?  If so, what about spaces
  //		within the name?
  //	Is there a way to return all names that match?
  //	How can the user specify a particular function of that name?
  LoadObject *loitem;
  Function *fitem, *main_func = NULL;
  Module *mitem, *main_mod = NULL;
  int index1, index2, index3, which = -1;
  if (sel)
    {
      char *last = NULL;
      if (*sel == '@')
	{ // 'sel' is "@seg_num:address"
	  which = (int) getNumber (sel + 1, last);
	  if (last == NULL || *last != ':' || (which < 0) || (which >= lobjs->size ()))
	    {
	      fprintf (stderr, GTXT ("Error: Invalid number entered: %s\n"), sel);
	      return NULL;
	    }
	  uint64_t address = getNumber (last + 1, last);
	  if (last == NULL || *last != '\0')
	    {
	      fprintf (stderr, GTXT ("Error: Invalid number entered: %s\n"), sel);
	      return NULL;
	    }
	  loitem = lobjs->fetch (which);
	  Vec_loop (Module*, loitem->seg_modules, index2, mitem)
	  {
	    Vec_loop (Function*, mitem->functions, index3, fitem)
	    {
	      if (address == fitem->img_offset && match_FName (name, fitem))
		return fitem;
	    }
	  }
	  return NULL;
	}

      which = (int) getNumber (sel, last);
      if (last == NULL || *last != '\0')
	{
	  fprintf (stderr, GTXT ("Error: Invalid number entered: %s\n"), sel);
	  return NULL;
	}
      which--;
    }

  int len_path = 0;
  char *with_path = name;
  name = StrRchr (name, '`');
  if (name != with_path)
    len_path = (int) (name - with_path);
  else
    with_path = NULL;

  Vec_loop (LoadObject*, lobjs, index1, loitem)
  {
    Vec_loop (Module*, loitem->seg_modules, index2, mitem)
    {
      if (with_path)
	{ // with file name
	  // try source name first
	  // if failed, try object name next
	  if (!match_basename (with_path, mitem->file_name, len_path) &&
	      !match_basename (with_path, mitem->get_name (), len_path))
	    continue;
	}
      Vec_loop (Function*, mitem->functions, index3, fitem)
      {
	if (match_FName (name, fitem))
	  {
	    if (which == list->size ())
	      return fitem;
	    list->append (fitem);
	    continue;
	  }
	if (streq (fitem->get_name (), NTXT ("MAIN_")) && mitem->is_fortran ())
	  {
	    main_func = fitem;
	    main_mod = mitem;
	  }
      }
    }
  }

  if (main_mod && main_func)
    {
      main_mod->read_stabs ();
      if (streq (main_func->get_match_name (), name) && which <= 1)
	return main_func;
    }
  return (Function *) NULL;
}

DataObject *
DbeSession::map_NametoDataObject (char *name, Vector<Histable*> *list,
				  int which)
{
  // Search master list to find dataobjects whose names match "name"
  // selecting only the entry corresponding to "which" if it is not -1.
  // Issues: is the name fully qualified or only partially?
  DataObject *ditem = NULL;
  int index;
  char *full_name;
  Vec_loop (DataObject*, dobjs, index, ditem)
  {
    if (ditem->scope) continue; // skip non-master dataobjects

    // try fully-qualified dataobject name first
    if ((full_name = ditem->get_name ()) != NULL)
      {
	if (streq (name, full_name))
	  {
	    if (which == list->size ())
	      return ditem;
	    list->append (ditem);
	  }
      }
  }
  if (list->size () > 0)
    return ditem; // return fully-qualified match

  // if fully-qualified name doesn't match anything, try a partial match
  Vec_loop (DataObject*, dobjs, index, ditem)
  {
    if (ditem->scope) continue; // skip non-master dataobjects

    // try fully-qualified dataobject name first
    if ((full_name = ditem->get_name ()) != NULL)
      {
	if (strstr (full_name, name))
	  {
	    if (which == list->size ())
	      return ditem;
	    list->append (ditem);
	  }
      }
  }
  return (DataObject *) NULL;
}

bool
DbeSession::match_FName (char *name, Function *func)
{
  size_t len;
  char buf[MAXDBUF];
  char *full_name;
  if (streq (func->get_name (), name)) // try full name comparison
    return true;
  if (streq (func->get_mangled_name (), name)) // try mangled name
    return true;
  if (streq (func->get_match_name (), name)) // try match name
    return true;

  Module *md = func->module; // try FORTRAN name
  if (md && md->is_fortran ())
    {
      char *mangled_name = func->get_mangled_name ();
      len = strlen (name);
      if (((len + 1) == strlen (mangled_name)) &&
	  (strncmp (name, mangled_name, len) == 0))
	return true;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s"), func->get_name ());
  full_name = buf;
  char *arg = NULL; // find modifier and C++ class name
  int i = get_paren (buf);
  if (i >= 0)
    {
      arg = buf + i;
      *arg = '\0';
    }

  char *mod = strchr (full_name, ' ');
  char *cls = strchr (full_name, ':');

  if (mod)
    {
      len = mod - full_name + 1;
      if (!strncmp (full_name, name, len))
	name += len;
      full_name += len;
      if (streq (full_name, name)) // try without modifier
	return true;
    }

  size_t len_cmp = strlen (name);
  if (arg)
    {
      *arg = '(';
      len = arg - full_name; // try without 'args'
      if (len_cmp == len && !strncmp (full_name, name, len))
	return true;
      if (cls)
	{
	  len = arg - cls - 2; // and without 'class name'
	  if ((len_cmp == len) && !strncmp (cls + 2, name, len))
	    return true;
	}
    }

  if (cls)
    {
      len = cls - full_name; // try C++ class name only
      if (len_cmp == len && !strncmp (full_name, name, len))
	return true;
      if (streq (cls + 2, name)) // try without 'class name'
	return true;
    }
  return false;
}

bool
DbeSession::add_path (char *path)
{
  return add_path (path, get_search_path ());
}

bool
DbeSession::add_classpath (char *path)
{
  return add_path (path, classpath);
}

Vector<DbeFile*> *
DbeSession::get_classpath ()
{
  if (classpath_df == NULL)
    classpath_df = new Vector<DbeFile*>;
  for (int i = classpath_df->size (), sz = classpath->size (); i < sz; i++)
    classpath_df->store (i, getDbeFile (classpath->fetch (i),
					DbeFile::F_DIR_OR_JAR));
  return classpath_df;
}

bool
DbeSession::add_path (char *path, Vector<char*> *pathes)
{
  bool result = false;
  Vector <char *> *tokens = split_str (path, ':');
  for (long j = 0, jsz = VecSize (tokens); j < jsz; j++)
    {
      char *spath = tokens->get (j);
      // Don't append path if it's already there
      bool got = false;
      for (int i = 0, sz = pathes->size (); i < sz; i++)
	{
	  char *nm = pathes->get (i);
	  if (streq (nm, spath))
	    {
	      got = true;
	      break;
	    }
	}
      if (!got)
	{
	  pathes->append (spath);
	  result = true;
	}
      else
	free (spath);
    }
  delete tokens;
  return result;
}

void
DbeSession::set_need_refind ()
{
  Vector<DbeFile*> *f_list = dbeFiles->values ();
  for (long i = 0, sz = f_list == NULL ? 0 : f_list->size (); i < sz; i++)
    {
      DbeFile *f = f_list->get (i);
      f->set_need_refind (true);
    }
  delete f_list;
  for (long i = 0, sz = sources == NULL ? 0 : sources->size (); i < sz; i++)
    {
      SourceFile *f = sources->get (i);
      if (f && f->dbeFile)
	f->dbeFile->set_need_refind (true);
    }
}

void
DbeSession::set_search_path (Vector<char*> *path, bool reset)
{
  if (reset)
    search_path->destroy ();
  for (int i = 0, sz = path == NULL ? 0 : path->size (); i < sz; i++)
    {
      char *name = path->fetch (i);
      if (add_path (name))
	reset = true;
    }
  if (reset)
    {
      set_need_refind ();

      // now reset the string setting for it
      StringBuilder sb;
      for (int i = 0, sz = search_path == NULL ? 0 : search_path->size (); i < sz; i++)
	{
	  char *name = search_path->fetch (i);
	  if (sb.length () != 0)
	    sb.append (':');
	  sb.append (name);
	}
      free (settings->str_search_path);
      settings->str_search_path = sb.toString ();
    }
}

void
DbeSession::set_search_path (char *_lpath, bool reset)
{
  Vector<char *> *path = new Vector<char*>;
  char *lpath = dbe_strdup (_lpath);
  for (char *s = lpath; s;)
    {
      path->append (s);
      s = strchr (s, ':');
      if (s)
	{
	  *s = 0;
	  s++;
	}
    }
  set_search_path (path, reset);
  delete path;
  free (lpath);
}

void
DbeSession::set_pathmaps (Vector<pathmap_t*> *newPathMap)
{
  set_need_refind ();
  settings->set_pathmaps (newPathMap);
}

Vector<pathmap_t*> *
DbeSession::get_pathmaps ()
{
  return settings->pathmaps;
}

void
DbeSession::mobj_define (MemObjType_t *mobj)
{
  settings->mobj_define (mobj, false);
  DbeView *dbev;
  int index;
  Vec_loop (DbeView*, views, index, dbev)
  {
    dbev->get_settings ()->mobj_define (mobj, false);
  }
}

void
DbeSession::dump_segments (FILE *out)
{
  int index;
  LoadObject *loitem;
  Vec_loop (LoadObject*, lobjs, index, loitem)
  {
    fprintf (out, NTXT ("Segment %d -- %s -- %s\n\n"),
	     index, loitem->get_name (), loitem->get_pathname ());
    loitem->dump_functions (out);
    fprintf (out, NTXT ("\n End Segment %d -- %s -- %s\n\n"),
	     index, loitem->get_name (), loitem->get_pathname ());
  }
}

void
DbeSession::dump_dataobjects (FILE *out)
{
  DataObject *ditem;
  int index;

  fprintf (out, NTXT ("\nMaster list of DataObjects:\n"));
  Vec_loop (DataObject*, dobjs, index, ditem)
  {
    Histable* scope = ditem->get_scope ();
    DataObject* parent = ditem->get_parent ();
    DataObject* master = ditem->get_master ();
    if (parent != NULL)
      fprintf (out, "id %6lld: [%4lld] parent = %6lld, offset = %+4lld %s\n",
	       (ll_t) ditem->id, (ll_t) ditem->get_size (),
	       (ll_t) parent->id, (ll_t) ditem->get_offset (),
	       ditem->get_name ());
    else
      {
	// parent is NULL
	fprintf (out, NTXT ("id %6lld: [%4lld] %s "),
		 (ll_t) ditem->id, (ll_t) ditem->get_size (),
		 ditem->get_name ());
	if (master != NULL)
	  fprintf (out, NTXT (" master=%lld "), (ll_t) master->id);
	else if (scope != NULL)
	  fprintf (out, NTXT (" master=?? "));
	else
	  fprintf (out, NTXT (" MASTER "));
#if DEBUG
	if (scope != NULL)
	  {
	    switch (scope->get_type ())
	      {
	      case Histable::LOADOBJECT:
	      case Histable::FUNCTION:
		fprintf (out, NTXT ("%s"), scope->get_name ());
		break;
	      case Histable::MODULE:
		{
		  char *filename = get_basename (scope->get_name ());
		  fprintf (out, NTXT ("%s"), filename);
		  break;
		}
	      default:
		fprintf (out, NTXT (" Unexpected scope %d:%s"),
			 scope->get_type (), scope->get_name ());
	      }
	  }
#endif
	fprintf (out, NTXT ("\n"));
      }
  }
}

void
DbeSession::dump_map (FILE *out)
{
  Experiment *exp;
  int index;
  Vec_loop (Experiment*, exps, index, exp)
  {
    exp->dump_map (out);
  }
}

void
DbeSession::dump_stacks (FILE *outfile)
{
  Experiment *exp;
  int n = nexps ();
  FILE *f = (outfile == NULL ? stderr : outfile);
  for (int i = 0; i < n; i++)
    {
      exp = get_exp (i);
      fprintf (f, GTXT ("Experiment %d -- %s\n"), i, exp->get_expt_name ());
      exp->dump_stacks (f);
    }
}

void
DbeSession::propNames_name_store (int propId, const char *propName)
{
  PropDescr *prop = new PropDescr (propId, propName);
  prop->flags = PRFLAG_NOSHOW; // do not show descriptions
  propNames->store (propId, prop);
}

void
DbeSession::propNames_name_store (int propId, const char* propName,
				  const char* propUname, VType_type dataType,
				  int flags)
{
  PropDescr *prop = new PropDescr (propId, propName);
  prop->vtype = dataType;
  prop->uname = dbe_strdup (propUname);
  prop->flags = flags;
  propNames->store (propId, prop);
}

char *
DbeSession::propNames_name_fetch (int i)
{
  PropDescr *prop = propNames->fetch (i);
  if (prop)
    return prop->name;
  return NULL;
}

int
DbeSession::registerPropertyName (const char *name)
{
  if (name == NULL)
    return PROP_NONE;
  for (int i = 0; i < propNames->size (); i++)
    {
      char *pname = propNames_name_fetch (i);
      if (pname && strcasecmp (pname, name) == 0)
	return i;
    }
  int propId = propNames->size ();
  propNames_name_store (propId, name);
  return propId;
}

int
DbeSession::getPropIdByName (const char *name)
{
  if (name == NULL)
    return PROP_NONE;
  for (int i = 0; i < propNames->size (); i++)
    {
      char *pname = propNames_name_fetch (i);
      if (pname && strcasecmp (pname, name) == 0)
	return i;
    }
  return PROP_NONE;
}

char *
DbeSession::getPropName (int propId)
{
  if (!propNames)
    return NULL;
  if (propId < 0 || propId >= propNames->size ())
    return NULL;
  return dbe_strdup (propNames_name_fetch (propId));
}

char *
DbeSession::getPropUName (int propId)
{
  if (!propNames)
    return NULL;
  if (propId < 0 || propId >= propNames->size ())
    return NULL;
  PropDescr *prop = propNames->fetch (propId);
  if (prop)
    return dbe_strdup (prop->uname);
  return NULL;
}

void
DbeSession::append (UserLabel *lbl)
{
  if (lbl->expr)
    {
      if (userLabels == NULL)
	 userLabels = new Vector<UserLabel*> ();
      userLabels->append (lbl);
    }
}

void
DbeSession::append (SourceFile *sf)
{
  sources->append (sf);
  objs->append (sf);
}

UserLabel *
DbeSession::findUserLabel (const char *name)
{
  for (int i = 0, sz = userLabels ? userLabels->size () : 0; i < sz; i++)
    {
      UserLabel *lbl = userLabels->fetch (i);
      if (strcasecmp (lbl->name, name) == 0)
	return lbl;
    }
  return NULL;
}

Expression *
DbeSession::findObjDefByName (const char *name)
{
  Expression *expr = NULL;

  MemObjType_t *mot = MemorySpace::findMemSpaceByName (name);
  if (mot != NULL)
    {
      char *index_expr_str = mot->index_expr;
      expr = ql_parse (index_expr_str);
    }

  if (expr == NULL)
    {
      int indxtype = findIndexSpaceByName (name);
      expr = getIndexSpaceExpr (indxtype);
    }
  if (expr == NULL)
    {
      UserLabel *ulbl = findUserLabel (name);
      if (ulbl)
	expr = ulbl->expr;
    }
  return expr;
}

Expression *
DbeSession::ql_parse (const char *expr_spec)
{
  if (expr_spec == NULL)
    expr_spec = "";
  QL::Result result (expr_spec);
  QL::Parser qlparser (result);
  if (qlparser.parse () != 0)
    return NULL;
  return result ();
}

Vector<void*> *
DbeSession::getIndxObjDescriptions ()
{
  int size = dyn_indxobj_indx;
  if (size == 0)
    return NULL;
  Vector<int> *type = new Vector<int>(dyn_indxobj_indx);
  Vector<char*> *desc = new Vector<char*>(dyn_indxobj_indx);
  Vector<char*> *i18ndesc = new Vector<char*>(dyn_indxobj_indx);
  Vector<char> *mnemonic = new Vector<char>(dyn_indxobj_indx);
  Vector<int> *orderList = new Vector<int>(dyn_indxobj_indx);
  Vector<char*> *exprList = new Vector<char*>(dyn_indxobj_indx);
  Vector<char*> *sdesc = new Vector<char*>(dyn_indxobj_indx);
  Vector<char*> *ldesc = new Vector<char*>(dyn_indxobj_indx);

  for (long i = 0, sz = VecSize (dyn_indxobj); i < sz; i++)
    {
      IndexObjType_t *tot = dyn_indxobj->get (i);
      if (tot->memObj == NULL)
	{
	  type->append ((int) tot->type);
	  desc->append (dbe_strdup (tot->name));
	  i18ndesc->append (dbe_strdup (tot->i18n_name));
	  sdesc->append (dbe_strdup (tot->short_description));
	  ldesc->append (dbe_strdup (tot->long_description));
	  mnemonic->append (tot->mnemonic);
	  orderList->append (settings->indx_tab_order->fetch (i));
	  exprList->append (dbe_strdup (tot->index_expr_str));
	}
    }
  Vector<void*> *res = new Vector<void*>(8);
  res->store (0, type);
  res->store (1, desc);
  res->store (2, mnemonic);
  res->store (3, i18ndesc);
  res->store (4, orderList);
  res->store (5, exprList);
  res->store (6, sdesc);
  res->store (7, ldesc);
  return (res);
}

// Static function to get a vector of custom index object definitions
Vector<void*> *
DbeSession::getCustomIndxObjects ()
{
  Vector<char*> *name = new Vector<char*>;
  Vector<char*> *formula = new Vector<char*>;
  for (long i = dyn_indxobj_indx_fixed, sz = VecSize (dyn_indxobj); i < sz; i++)
    {
      IndexObjType_t *tot = dyn_indxobj->get (i);
      if (tot->memObj == NULL)
	{
	  name->append (dbe_strdup (tot->name));
	  formula->append (dbe_strdup (tot->index_expr_str));
	}
    }
  Vector<void*> *res = new Vector<void*>(2);
  res->store (0, name);
  res->store (1, formula);
  return (res);
}

// Static function to define a new index object type
char *
DbeSession::indxobj_define (const char *mname, char *i18nname, const char *index_expr_str, char *short_description, char *long_description)
{
  if (mname == NULL)
    return dbe_strdup (GTXT ("No index object type name has been specified."));
  if (isalpha ((int) (mname[0])) == 0)
    return dbe_sprintf (GTXT ("Index Object type name %s does not begin with an alphabetic character"),
			  mname);
  const char *p = mname;
  while (*p != 0)
    {
      if ((isalnum ((int) (*p)) == 0) && (*p != '_'))
	return dbe_sprintf (GTXT ("Index Object type name %s contains a non-alphanumeric character"),
			    mname);
      p++;
    }

  // make sure the name is not in use
  if (MemorySpace::findMemSpaceByName (mname) != NULL)
    return dbe_sprintf (GTXT ("Memory/Index Object type name %s is already defined"),
			  mname);

  int idxx = findIndexSpaceByName (mname);
  if (idxx >= 0)
    {
      IndexObjType_t *mt = dyn_indxobj->fetch (idxx);
      if (strcmp (mt->index_expr_str, index_expr_str) == 0)
	// It's a redefinition, but the new definition is the same
	return NULL;
      return dbe_sprintf (GTXT ("Memory/Index Object type name %s is already defined"),
			  mname);
    }
  if (index_expr_str == NULL)
    return dbe_strdup (GTXT ("No index-expr has been specified."));
  if (strlen (index_expr_str) == 0)
    return dbe_sprintf (GTXT ("Index Object index expression is invalid: %s"),
			index_expr_str);

  // verify that the index expression parses correctly
  char *expr_str = dbe_strdup (index_expr_str);
  Expression *expr = ql_parse (expr_str);
  if (expr == NULL)
    return dbe_sprintf (GTXT ("Index Object index expression is invalid: %s"),
			expr_str);

  // It's OK, create the new table entry
  IndexObjType_t *tot = new IndexObjType_t;
  tot->type = dyn_indxobj_indx++;
  tot->name = dbe_strdup (mname);
  tot->i18n_name = dbe_strdup (i18nname);
  tot->short_description = dbe_strdup (short_description);
  tot->long_description = dbe_strdup (long_description);
  tot->index_expr_str = expr_str;
  tot->index_expr = expr;
  tot->mnemonic = mname[0];

  // add it to the list
  dyn_indxobj->append (tot);
  idxobjs->append (new HashMap<uint64_t, Histable*>);

  // tell the session
  settings->indxobj_define (tot->type, false);

  DbeView *dbev;
  int index;
  Vec_loop (DbeView*, views, index, dbev)
  {
    dbev->addIndexSpace (tot->type);
  }
  return NULL;
}

char *
DbeSession::getIndexSpaceName (int index)
{
  if (index < 0 || index >= dyn_indxobj->size ())
    return NULL;
  return dyn_indxobj->fetch (index)->name;
}

char *
DbeSession::getIndexSpaceDescr (int index)
{
  if (index < 0 || index >= dyn_indxobj->size ())
    return NULL;
  return dyn_indxobj->fetch (index)->i18n_name;
}

Expression *
DbeSession::getIndexSpaceExpr (int index)
{
  if (index < 0 || index >= dyn_indxobj->size ())
    return NULL;
  return dyn_indxobj->fetch (index)->index_expr;
}

char *
DbeSession::getIndexSpaceExprStr (int index)
{
  if (index < 0 || index >= dyn_indxobj->size ())
    return NULL;
  return dyn_indxobj->fetch (index)->index_expr_str;
}

int
DbeSession::findIndexSpaceByName (const char *mname)
{
  int idx;
  IndexObjType_t *mt;
  Vec_loop (IndexObjType_t*, dyn_indxobj, idx, mt)
  {
    if (strcasecmp (mt->name, mname) == 0)
      return idx;
  }
  return -1;
}

void
DbeSession::removeIndexSpaceByName (const char *mname)
{
  IndexObjType_t *indObj = findIndexSpace (mname);
  if (indObj)
    indObj->name[0] = 0;
}

IndexObjType_t *
DbeSession::getIndexSpace (int index)
{
  return ((index < 0) || (index >= VecSize (dyn_indxobj))) ? NULL : dyn_indxobj->get (index);
}

IndexObjType_t *
DbeSession::findIndexSpace (const char *mname)
{
  return getIndexSpace (findIndexSpaceByName (mname));
}

void
DbeSession::get_filter_keywords (Vector<void*> *res)
{
  Vector <char*> *kwCategory = (Vector<char*>*) res->fetch (0);
  Vector <char*> *kwCategoryI18N = (Vector<char*>*) res->fetch (1);
  Vector <char*> *kwDataType = (Vector<char*>*) res->fetch (2);
  Vector <char*> *kwKeyword = (Vector<char*>*) res->fetch (3);
  Vector <char*> *kwFormula = (Vector<char*>*) res->fetch (4);
  Vector <char*> *kwDescription = (Vector<char*>*) res->fetch (5);
  Vector <void*> *kwEnumDescs = (Vector<void*>*) res->fetch (6);

  char *vtypeNames[] = VTYPE_TYPE_NAMES;
  for (long i = 0, sz = userLabels ? userLabels->size () : 0; i < sz; i++)
    {
      UserLabel *lbl = userLabels->fetch (i);
      kwCategory->append (dbe_strdup (NTXT ("FK_LABEL")));
      kwCategoryI18N->append (dbe_strdup (GTXT ("Labels")));
      kwDataType->append (dbe_strdup (vtypeNames[TYPE_BOOL]));
      kwKeyword->append (dbe_strdup (lbl->name));
      kwFormula->append (dbe_strdup (lbl->str_expr));
      kwDescription->append (dbe_strdup (lbl->comment));
      kwEnumDescs->append (NULL);
    }

  for (long i = 0, sz = propNames ? propNames->size () : 0; i < sz; i++)
    {
      PropDescr *prop = propNames->fetch (i);
      char *pname = prop ? prop->name : NULL;
      if (pname == NULL || *pname == 0 || prop->flags & PRFLAG_NOSHOW)
	continue;
      int vtypeNum = prop->vtype;
      if (vtypeNum < 0 || vtypeNum >= TYPE_LAST)
	vtypeNum = TYPE_NONE;
      kwCategory->append (dbe_strdup (NTXT ("FK_EVTPROP"))); //Event Property
      kwCategoryI18N->append (dbe_strdup (GTXT ("Misc. Definitions")));
      kwDataType->append (dbe_strdup (vtypeNames[vtypeNum]));
      kwKeyword->append (dbe_strdup (pname));
      kwFormula->append (NULL);
      kwDescription->append (dbe_strdup (prop->uname));
      kwEnumDescs->append (NULL);
    }

  for (long i = 0, sz = dyn_indxobj ? dyn_indxobj->size () : 0; i < sz; i++)
    {
      IndexObjType_t *obj = dyn_indxobj->get (i);
      if (obj->memObj)
	continue;
      kwCategory->append (dbe_strdup (NTXT ("FK_IDXOBJ")));
      kwCategoryI18N->append (dbe_strdup (GTXT ("Index Object Definitions")));
      kwDataType->append (dbe_strdup (vtypeNames[TYPE_INT64]));
      kwKeyword->append (dbe_strdup (obj->name));
      kwFormula->append (dbe_strdup (obj->index_expr_str));
      kwDescription->append (dbe_strdup (obj->i18n_name));
      kwEnumDescs->append (NULL);
    }
}

Histable *
DbeSession::findIndexObject (int idxtype, uint64_t idx)
{
  HashMap<uint64_t, Histable*> *iobjs = idxobjs->fetch (idxtype);
  return iobjs->get (idx);
}

Histable *
DbeSession::createIndexObject (int idxtype, int64_t idx)
{
  HashMap<uint64_t, Histable*> *iobjs = idxobjs->fetch (idxtype);

  Histable *idxobj = iobjs->get (idx);
  if (idxobj == NULL)
    {
      idxobj = new IndexObject (idxtype, idx);
      if (idx == -1)
	idxobj->set_name (dbe_strdup (GTXT ("<Unknown>")));
      iobjs->put (idx, idxobj);
    }

  return idxobj;
}

Histable *
DbeSession::createIndexObject (int idxtype, Histable *hobj)
{
  HashMap<uint64_t, Histable*> *iobjs = idxobjs->fetch (idxtype);
  int64_t idx = hobj ? hobj->id : -1;
  Histable *idxobj = iobjs->get (idx);
  if (idxobj == NULL)
    {
      idxobj = new IndexObject (idxtype, hobj);
      if (idx == -1)
	idxobj->set_name (dbe_strdup (GTXT ("<Unknown>")));
      iobjs->put (idx, idxobj);
    }

  return idxobj;
}

Histable *
DbeSession::findObjectById (Histable::Type type, int subtype, uint64_t id)
{
  switch (type)
    {
    case Histable::FUNCTION:
    case Histable::MODULE:
    case Histable::LOADOBJECT:
      return ( id < (uint64_t) objs->size ()) ? objs->fetch ((int) id) : NULL;
    case Histable::INDEXOBJ:
      return findIndexObject (subtype, id);
      // ignoring the following cases
    case Histable::INSTR:
    case Histable::LINE:
    case Histable::EADDR:
    case Histable::MEMOBJ:
    case Histable::PAGE:
    case Histable::DOBJECT:
    case Histable::SOURCEFILE:
    case Histable::IOACTFILE:
    case Histable::IOACTVFD:
    case Histable::IOCALLSTACK:
    case Histable::HEAPCALLSTACK:
    case Histable::OTHER:
    case Histable::EXPERIMENT:
      break;
    }
  return NULL;
}

// return a vector of Functions that match the regular expression input string
Vector<JThread *> *
DbeSession::match_java_threads (char *ustr, int matchParent,
				Vector<uint64_t> * &grids,
				Vector<uint64_t> * &expids)
{
  if (ustr == NULL)
    return NULL;

  char *str = dbe_sprintf (NTXT ("^%s$"), ustr);
  regex_t regex_desc;
  int rc = regcomp (&regex_desc, str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  free (str);
  if (rc)   // syntax error in parsing string
    return NULL;

  // allocate the new vector
  Vector<JThread *> *ret = new Vector<JThread*>;
  grids = new Vector<uint64_t>;
  expids = new Vector<uint64_t>;

  int index;
  JThread *jthread;
  int expid;
  Experiment* exp;
  Vec_loop (Experiment*, exps, expid, exp)
  {

    Vec_loop (JThread*, exp->get_jthreads (), index, jthread)
    {
      const char * name;
      if (matchParent)
	name = jthread->parent_name;
      else
	name = jthread->group_name;
      if (name == NULL)
	name = "";
      if (!regexec (&regex_desc, name, 0, NULL, 0))
	{
	  // this one matches
	  ret->append (jthread);
	  grids->append (exp->groupId);
	  expids->append (exp->getUserExpId ());
	}
    }
  }

  regfree (&regex_desc);
  return ret;
}

// return a vector of Functions that match the regular expression input string
Vector<Function *> *
DbeSession::match_func_names (const char *ustr, Histable::NameFormat nfmt)
{
  if (ustr == NULL)
    return NULL;
  char *str = dbe_sprintf (NTXT ("^%s$"), ustr);
  regex_t regex_desc;
  int rc = regcomp (&regex_desc, str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  free (str);
  if (rc)   // syntax error in parsing string
    return NULL;

  // allocate the new vector
  Vector<Function *> *ret = new Vector<Function*>;

  int index;
  Histable *obj;
  Vec_loop (Histable*, objs, index, obj)
  {
    if (obj->get_type () == Histable::FUNCTION)
      {
	Function *func = (Function*) obj;
	if (!regexec (&regex_desc, func->get_name (nfmt), 0, NULL, 0))
	  // this one matches
	  ret->append (func);
      }
  }
  regfree (&regex_desc);
  return ret;
}

// return a vector of Functions that match the regular expression input string
Vector<FileData *> *
DbeSession::match_file_names (char *ustr, Histable::NameFormat nfmt)
{
  if (ustr == NULL)
    return NULL;
  char *str = dbe_sprintf (NTXT ("^%s$"), ustr);
  regex_t regex_desc;
  int rc = regcomp (&regex_desc, str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  free (str);
  if (rc)   // syntax error in parsing string
    return NULL;

  // allocate the new vector
  Vector<FileData *> *ret = new Vector<FileData*>;
  int numExps = nexps ();
  DefaultMap<int64_t, FileData*>* fDataMap;
  Vector<FileData *> *fDataObjs;
  FileData *fData;
  int size;
  for (int i = 0; i < numExps; i++)
    {
      Experiment *exp = get_exp (i);
      fDataMap = exp->getFDataMap ();
      fDataObjs = fDataMap->values ();
      size = fDataObjs->size ();
      for (int j = 0; j < size; j++)
	{
	  fData = fDataObjs->fetch (j);
	  if (fData
	      && !regexec (&regex_desc, fData->get_raw_name (nfmt), 0, NULL, 0))
	    // this one matches
	    ret->append (fData);
	}
    }
  regfree (&regex_desc);
  return ret;
}

// return a vector of DataObjects that match the regular expression input string
Vector<DataObject *> *
DbeSession::match_dobj_names (char *ustr)
{
  if (ustr == NULL)
    return NULL;
  char *str = dbe_sprintf (NTXT ("^%s$"), ustr);
  regex_t regex_desc;
  int rc = regcomp (&regex_desc, str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  free (str);
  if (rc)   // syntax error in parsing string
    return NULL;

  // allocate the new vector
  Vector<DataObject *> *ret = new Vector<DataObject*>;
  int index;
  DataObject *ditem;
  Vec_loop (DataObject*, dobjs, index, ditem)
  {
    // does this one match
    if (!regexec (&regex_desc, ditem->get_name (), 0, NULL, 0))
      // this one matches
      ret->append (ditem);
  }
  regfree (&regex_desc);
  return ret;
}

void
DbeSession::dump (char *msg, Vector<BaseMetric*> *mlist)
{
  if (msg)
    fprintf (stderr, "%s\n", msg);
  int sz = mlist ? mlist->size () : -1;
  for (int i = 0; i < sz; i++)
    {
      BaseMetric *m = mlist->fetch (i);
      char *s = m->dump ();
      fprintf (stderr, "%2d %s\n", i, s);
      free (s);
    }
  fprintf (stderr, "======END of mlist[%d] =========\n", sz);
}

void
DbeSession::dump (char *msg, Vector<Metric*> *mlist)
{
  if (msg)
    fprintf (stderr, "%s\n", msg);
  int sz = mlist ? mlist->size () : -1;
  for (int i = 0; i < sz; i++)
    {
      Metric *m = mlist->fetch (i);
      char *s = m->dump ();
      fprintf (stderr, "%2d %s\n", i, s);
      free (s);
    }
  fprintf (stderr, "======END of mlist[%d] =========\n", sz);
}
