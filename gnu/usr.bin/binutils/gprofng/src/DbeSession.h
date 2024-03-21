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

/* The DbeSession class is instantiated by a DbeApplication, and contains
 *	all the data referring to a set of loaded experiments
 *
 *	It manages a set of tables for the Experiments, for the LoadObjects
 *	referenced in them, and for the other objects representing the
 *	elements in the program hierarchy that can have metrics associated
 *	with them.  It also has a master list of all the metrics available
 *	from all the loaded experiments.
 *
 *	It gets an instance of the Settings class, instantiated as a copy
 *	of the one in the DbeApplication that instantiated the DbeSession.
 *
 *	In addition, it manages a vector of DbeView's (q.v.); each DbeView
 *	represents a window into the DbeSession, and has its own set of
 *	Settings, and FilterSets for the Experiments, and is the access point
 *	for all processed data.
 */

#ifndef _DBESESSION_H
#define _DBESESSION_H


#include <stdio.h>
#include "dbe_structs.h"
#include "vec.h"
#include "Hist_data.h"
#include "Histable.h"
#include "BaseMetric.h"
#include "BaseMetricTreeNode.h"
#include "MemorySpace.h"
#include "hwcentry.h"
#include "dbe_types.h"
#include "Settings.h"
#include "HashMap.h"
#include "Table.h"
#include "Experiment.h"

class DbeSession;
class Experiment;
class Expression;
class ExpGroup;
class Function;
class JMethod;
class Histable;
class DbeView;
class Module;
class LoadObject;
class DataObject;
class SourceFile;
class Settings;
class StringBuilder;
class UserLabel;
class DbeFile;
class DbeJarFile;
class FileData;
class HeapData;
template <typename ITEM> class DbeSyncMap;
template <class ITEM> class Vector;

struct DispTab;
struct List;
struct Countable;
class IndexObjType_t;

typedef struct
{
  char *path;
  Experiment *exp;
  DbeSession *ds;
  bool read_ahead;
} exp_ctx;

class DbeSession
{
public:
  DbeSession (Settings *_settings, bool _ipc_mode, bool _rdt_mode);
  ~DbeSession ();

  void reset ();
  void reset_data ();

  void
  set_interactive (bool _interactive)
  {
    interactive = _interactive;
  }

  bool
  is_interactive ()
  {
    return interactive;
  }

  bool is_datamode_available ();
  bool is_leaklist_available ();
  bool is_heapdata_available ();
  bool is_iodata_available ();
  bool is_racelist_available ();
  bool is_deadlocklist_available ();
  bool is_timeline_available ();
  bool is_ifreq_available ();
  bool is_omp_available ();
  bool has_java ();
  bool has_ompavail ();

  // XXX  get_clock should be removed, to support cpus with different clocks
  // XXX    means reworking time-convertible HWC code
  int get_clock (int id);

  // Access functions for DbeView's
  int createView ();
  int createView (int index, int cloneindex);
  DbeView *getView (int index);
  void dropView (int index);

  // Access functions controlling the experiment list
  Vector<char*> *get_group_or_expt (char *path); // load an experiment or group

  void open_experiment (Experiment *exp, char *path);
  Experiment *get_exp (int exp_ind);
  Vector<Vector<char*>*> *getExperimensGroups ();
  char *setExperimentsGroups (Vector<Vector<char*>*> *groups);
  char *drop_experiment (int exp_ind);
  int find_experiment (char *path);

  int
  nexps ()
  {
    return exps->size ();
  }
  int ngoodexps ();

  // Access functions controlling the DataObject list
  DataObject *createDataObject ();
  DataObject *createDataObject (DataObject *d, DataObject *p = NULL);
  DataObject *createMasterDataObject (DataObject *);
  Vector<DataObject*> *get_dobj_elements (DataObject *);

  DataObject *
  get_Total_DataObject ()
  {
    return d_total;
  };

  DataObject *
  get_Unknown_DataObject ()
  {
    return d_unknown;
  };

  DataObject *
  get_Scalars_DataObject ()
  {
    return d_scalars;
  };

  DataObject *find_dobj_by_name (char *dobj_name);
  DataObject *find_dobj_match (DataObject *dobj);
  DataObject *find_dobj_master (DataObject *dobj);

  int
  ndobjs ()
  {
    return dobjs->size ();
  }

  // check if no -xhwcprof should be ignored
  bool
  check_ignore_no_xhwcprof ()
  {
    return settings->get_ignore_no_xhwcprof ();
  };

  // check if FS warning should be comment, or real warning
  bool
  check_ignore_fs_warn ()
  {
    return settings->get_ignore_fs_warn ();
  };

  // Access functions controlling the LoadObject list
  DbeSyncMap<LoadObject> *loadObjMap;
  void append (LoadObject *lobj);
  LoadObject *createLoadObject (const char *nm, int64_t cksum = 0);
  LoadObject *createLoadObject (const char *nm, const char *runTimePath, DbeFile *df);

  Vector<LoadObject *> *
  get_LoadObjects ()
  {
    return lobjs;
  };

  void dobj_updateHT (DataObject *dobj);
  LoadObject *get_Total_LoadObject ();
  Vector<LoadObject*> *get_text_segments ();
  LoadObject *get_Unknown_LoadObject ();
  LoadObject *find_lobj_by_name (const char *lobj_name, int64_t cksum = 0);

  // Access functions controlling the Tab list
  Vector<DispTab*> *
  get_TabList ()
  {
    return settings->get_TabList ();
  };

  Vector<bool> *
  get_MemTabList ()
  {
    return settings->get_MemTabState ();
  };

  void mobj_define (MemObjType_t *);

  // Access functions controlling metrics
  BaseMetric *find_base_reg_metric (char *mcmd);
  Vector<BaseMetric*> *get_base_reg_metrics (); // excludes comparison (expr) variants

  Vector<BaseMetric*> *
  get_all_reg_metrics ()
  {
    return reg_metrics;     // includes comparison (expr) variants
  };

  BaseMetricTreeNode *get_reg_metrics_tree ();
  BaseMetric *register_metric_expr (BaseMetric::Type type, char *aux, char *expr_spec);
  BaseMetric *register_metric (BaseMetric::Type type);
  BaseMetric *register_metric (char *name, char *username, char *_def);
  BaseMetric *register_metric (Hwcentry *ctr, const char* cmdname, const char* username);
  void drop_metric (BaseMetric *);
  Module *createModule (LoadObject *lo, const char *nm);
  Module *createUnknownModule (LoadObject *lo);
  Module *createClassFile (char *className);
  DbeFile *getDbeFile (char *filename, int filetype);
  SourceFile *get_Unknown_Source ();
  SourceFile *createSourceFile (const char *path);
  Histable *createHistObject (Histable::Type);
  Function *createFunction ();
  Function *create_hide_function (LoadObject *lo);
  Function *get_Total_Function ();
  Function *get_Unknown_Function ();
  Function *get_JUnknown_Function ();
  Function *get_jvm_Function ();
  LoadObject *get_OMP_LoadObject ();
  Function *get_OMP_Function (int);
  JMethod *createJMethod ();
  Histable *createIndexObject (int idxtype, int64_t idx);
  Histable *createIndexObject (int idxtype, Histable *hobj);

  enum SpecialFunction
  {
    TruncatedStackFunc,
    FailedUnwindFunc,
    LastSpecialFunction
  };
  Function *getSpecialFunction (SpecialFunction);

  Histable *
  findObjectById (uint64_t _id)
  {
    long id = (long) _id;
    return (id >= 0 && id < objs->size ()) ? objs->fetch (id) : NULL;
  }

  Histable *findObjectById (Histable::Type type, int subtype, uint64_t id);

  // Other access functions
  bool find_obj (FILE *dis_file, FILE *inp_file, Histable *&obj, char *name,
		 const char *sel, Histable::Type type, bool xdefault);
  int ask_which (FILE *dis_file, FILE *inp_file, Vector<Histable*> *list, char *name);
  LoadObject *map_NametoLoadObject (char *name, Vector<Histable*> *, int which);
  Module *map_NametoModule (char *name, Vector<Histable*> *, int which);
  Function *map_NametoFunction (char *, Vector<Histable*> *, const char *);
  DataObject *map_NametoDataObject (char *name, Vector<Histable*> *, int which);
  bool match_FName (char *name, Function *func);

  // Functions to convert a string to all matching Functions/DataObjects
  Vector<Function *> *match_func_names (const char *ustr, Histable::NameFormat nfmt);
  Vector<DataObject *> *match_dobj_names (char *);

  // Functions to convert a string to all matching JThreads
  Vector<JThread*> *match_java_threads (char *ustr, int matchParent,
					Vector<uint64_t> * &grids,
					Vector<uint64_t> * &expids);
  // Function to convert a string to all matching File names
  Vector<FileData *> *match_file_names (char *ustr, Histable::NameFormat nfmt);

  // Access functions concerning the search path
  Vector<char*> *
  get_search_path ()
  {
    return search_path;
  }

  Vector<DbeFile*>*get_classpath ();
  void set_search_path (Vector<char*> *path, bool reset);
  void set_search_path (char *lpath, bool reset);
  bool add_classpath (char *path);
  bool add_path (char *path);
  void set_pathmaps (Vector<pathmap_t*> *newPathMap);
  Vector<pathmap_t*> *get_pathmaps ();

  // functions to aid debugging
  void dump_stacks (FILE *);
  void dump_dataobjects (FILE *);
  void dump_segments (FILE *);
  void dump_map (FILE *);

  // Find dynamic property by name
  int registerPropertyName (const char *name);
  int getPropIdByName (const char *name);
  char* getPropName (int propId);
  char* getPropUName (int propId);

  Vector<UserLabel*> *userLabels; // List of er_labels
  UserLabel *findUserLabel (const char *name);
  DbeJarFile *get_JarFile (const char *name);
  void append (UserLabel *lbl);
  void append (SourceFile *sf);
  void append (Experiment *exp);
  void append (Hwcentry *exp);
  void set_need_refind ();

  // Find user defined object by name
  Expression *findObjDefByName (const char *);
  void get_filter_keywords (Vector<void*> *res);

  // Get the Settings class object
  Settings *
  get_settings ()
  {
    return settings;
  }

  // static members, used to define or fetch the various IndexSpaces
  Vector<void*> *getIndxObjDescriptions (void);
  Vector<void*> *getCustomIndxObjects (void);
  char *indxobj_define (const char *, char *, const char *, char *, char *);
  char *getIndexSpaceName (int index);
  char *getIndexSpaceDescr (int index);
  Expression *getIndexSpaceExpr (int index);
  char *getIndexSpaceExprStr (int index);
  int findIndexSpaceByName (const char *mname);
  void removeIndexSpaceByName (const char *mname);
  IndexObjType_t *getIndexSpace (int index);
  IndexObjType_t *findIndexSpace (const char *mname);
  Expression *ql_parse (const char *expr_spec);
  BaseMetric *find_metric (BaseMetric::Type type, const char *cmd, const char *expr_spec = NULL);
  static void dump (char *msg, Vector<Metric*> *mlist);
  static void dump (char *msg, Vector<BaseMetric*> *mlist);
  static Platform_t platform;               // Sparc, Intel
  Vector<ExpGroup *> *expGroups;
  HashMap<char*, LoadObject *> *comp_lobjs; // list of comparable LoadObjects
  HashMap<char*, DbeLine *> *comp_dbelines; // list of comparable DbeLines
  HashMap<char*, SourceFile*>*comp_sources; // list of comparable SourceFiles
  char *localized_SP_UNKNOWN_NAME;

  void
  set_lib_visibility_used ()
  {
    lib_visibility_used = true;
  }

  bool
  is_lib_visibility_used ()
  {
    return lib_visibility_used;
  }

  void unlink_tmp_files ();
  char *get_tmp_file_name (const char *nm, bool for_java);

  Vector<char *> *tmp_files;
  int status_ompavail;
  int archive_mode;
  bool ipc_mode;
  bool rdt_mode;

  // data and methods concerning the machine model
  //    methods are in source file MachineModel.cc
  Vector<char*> *list_mach_models (); // scan . and system lib directory for models
  char *load_mach_model (char *);

  char *
  get_mach_model ()
  {
    return dbe_strdup (mach_model_loaded);
  };
  Vector<SourceFile *> *get_sources ();

private:
  void init ();
  void check_tab_avail ();
  bool add_path (char *path, Vector<char*> *pathes);
  Experiment *createExperiment ();

  // Divide the regular createExperiment into two parts -
  // Part1 creates just the Experiment data structure
  // Part2 updates related fields and vectors
  Experiment *createExperimentPart1 ();
  void createExperimentPart2 (Experiment *exp);

  Histable *findIndexObject (int idxtype, uint64_t idx);
  void append_mesgs (StringBuilder *sb, char *path, Experiment *exp);
  static void insert_metric (BaseMetric *mtr, Vector<BaseMetric*> *mlist);
  void update_metric_tree (BaseMetric *m);

  char *find_mach_model (char *);   // fine machine model file by name
  Vector<Experiment*> *exps;        // Master list of experiments
  Vector<Histable*> *objs;          // Master list of Functions,Modules,Segments
  Vector<DataObject*> *dobjs;       // Master list of DataObjects
  Vector<LoadObject*> *lobjs;       // Auxiliary list of LoadObjects
  Vector<Hwcentry*> *hwcentries;
  Vector<HashMap<uint64_t, Histable*>*> *idxobjs; // Master list of IndexObjects
  HashMap<char*, SourceFile*> *sourcesMap;  // list of Source which were not archived
  Vector<SourceFile*> *sources;         // list of SourceFiles
  Map<const char*, DbeJarFile*>*dbeJarFiles;
  Vector<Countable*> *metrics;
  Vector<BaseMetric*> *reg_metrics;     // Master list of BaseMetrics
  BaseMetricTreeNode* reg_metrics_tree; // Hierarchy of BaseMetrics
  Vector<char*> *search_path;
  Vector<char*> *classpath;
  Vector<DbeFile*> *classpath_df;
  Map<const char*, DbeFile*>*dbeFiles;
  Vector<DbeView*> *views;              // Master list of DbeViews
  bool interactive;                     // interactive mode
  bool lib_visibility_used;
  LoadObject *lo_total;                 // Total LoadObject
  Function *f_total;                    // Total function
  LoadObject *lo_unknown;               // Unknown LoadObject
  Function *f_unknown;                  // Unknown function
  SourceFile *sf_unknown;               // Unknown source file
  Function *f_jvm;                      // pseudo-function <JVM-System>
  Vector<Function*> *f_special;         // pseudo-functions
  Function *j_unknown;                  // pseudo-function <no Java callstack>
  LoadObject *lo_omp;                   // OMP LoadObject (libmtsk)
  Vector<Function*> *omp_functions;     // OMP-overhead, etc.
  DataObject *d_unknown;                // Unknown dataobject
  DataObject *d_scalars;                // Scalars dataobject
  DataObject *d_total;                  // Total dataobject
  List **dnameHTable;                   // DataObject name hash table
  Settings *settings;                   // setting/defaults structure
  Vector<IndexObjType_t*> *dyn_indxobj; // Index Object definitions
  int dyn_indxobj_indx;
  int dyn_indxobj_indx_fixed;

  void propNames_name_store (int propId, const char *propName);
  void propNames_name_store (int propId, const char *propName,
			     const char *propUName, VType_type vType, int flags);
  char* propNames_name_fetch (int propId);
  Vector<PropDescr*> *propNames;
  char *defExpName;
  int user_exp_id_counter;
  char *mach_model_loaded;
  char *tmp_dir_name;
};

// For now, there's only one, so keep its pointer
extern DbeSession *dbeSession;

extern Vector<char *> *split_str (char *str, char delimiter);
#endif  /* _DBESESSION_H */
