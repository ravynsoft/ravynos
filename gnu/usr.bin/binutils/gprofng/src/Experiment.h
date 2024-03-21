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

#ifndef _EEXPERIMENT_H
#define _EEXPERIMENT_H

// The experiment class is responsible for managing all the data
//  for an individual experiment

#include "Metric.h"
#include "Histable.h"
#include "Stats_data.h"
#include "DefaultMap.h"
#include "HeapMap.h"

class Data_window;
class DbeFile;
class CallStack;
class JMethod;
class Sample;
class SegMem;
class LoadObject;
class SourceFile;
class UserLabel;
class PRBTree;
class Emsg;
class Emsgqueue;
struct JThread;
struct GCEvent;
class FileData;
class Module;
class Experiment;
template <class ITEM> class Vector;

#define JTHREAD_DEFAULT     ((JThread*)0)
#define JTHREAD_NONE        ((JThread*)-1)

// When we perform the pipelined optimization on resolve_frame_info() and add_stack()
// this is the number of iterations one phase works on before passing on the work to
// the next phase

#define CSTCTX_CHUNK_SZ 10000
#define PIPELINE_QUEUE_SZ_HI 8
#define PIPELINE_QUEUE_SZ_LOW 2

// the add_stack_ctx structure contains the intermediate state (context) after
// CSTCTX_CHUNK_SZ number of iterations to pass on the work to another thread to
// operate on the next stage
typedef struct
{
  Vector<DbeInstr*> *natpcs;
  Vector<Histable*> *jpcs;
  long idx;
  FramePacket *frp;
  hrtime_t tstamp;
  uint32_t thrid;
  bool last_ctx;
} cstk_ctx;

// To minimize threadpool overhead, the granularity of a job submitted is made larger:
// containing a chunk of iterations (of size CSTCTX_CHUNK_SZ)
typedef struct
{
  cstk_ctx* cstCtxAr[CSTCTX_CHUNK_SZ];
  int last_idx;
  long idx_begin;
  long idx_end;
  DataDescriptor *dDscr;
  Experiment *exp;
  void *cstk;
} cstk_ctx_chunk;

class Experiment : public Histable, public DbeMessages
{
public:

  enum Exp_status
  {
    SUCCESS,
    INCOMPLETE,
    FAILURE
  };

  Experiment ();
  virtual ~Experiment ();

  virtual Histable_type
  get_type ()
  {
    return EXPERIMENT;
  };
  virtual Vector<Histable*> *get_comparable_objs ();

  int groupId;
  Experiment *founder_exp;              // parent of this experiment
  Vector<Experiment*> *children_exps;   // children of this experiment

  // Configuration Information
  char *hostname;       // Hosthame (e.g. mymachine)
  hrtime_t start_sec;       // Starting timeval secs.
  char *username;       // name of person performing the test
  char *architecture;   // Architecture name ("sun4")
  Platform_t platform;  // Sparc,Sparcv9,Intel
  WSize_t wsize;        // word size: may be w32 or w64
  int clock;            // CPU clock frequency, Mhz
  int varclock;         // Set if CPU clock frequency can change: turbo-mode
  int maxclock;         // max. CPU clock frequency on MP machine
  int minclock;         // min. CPU clock frequency on MP machine
  int ncpus;            // count of CPUs where expt was recorded
  int hw_cpuver;        // CPU version from libcpc
  char *machinemodel;   // machine model of machine on which experiment was recorded
  char *os_version;     // Operating system name
  int page_size;        // Page size (bytes)
  int npages;           // Number of page size
  int exp_maj_version;  // major version number of current experiment
  int exp_min_version;  // minor version number of current experiment
  int hex_field_width;  // number of digits in hex form of address
			// for current experiment, i.e. 8 for 32bit addresses
  int broken;           // If SP_JCMD_RUN line not seen
  int obsolete;         // If pointer file experiment detected
  bool hwc_default;     // True if HW counters were enabled by default
  int hwc_bogus;        // Count of bogus HWC packets
  int hwc_lost_int;     // Count of packets reflecting lost interrupt
  int hwc_scanned;      // If the HWC packets have been scanned
  int invalid_packet;   // Count of invalid packets
  bool exec_started;    // True if exec was called, and exec error not yet seen
  bool dataspaceavail;  // True if dataspace data is in the experiment
  bool leaklistavail;   // True if leaklist data is in the experiment
  bool heapdataavail;   // True if heap data is in the experiment
  bool racelistavail;   // true if there are race events in the experiment
  bool iodataavail;     // true if there are io events in the experiment
  bool deadlocklistavail; // true if there are deadlock events in the experiment
  bool timelineavail;   // true if there are valid timestamps in the experiment
  bool ifreqavail;      // True if instruction-frequency data is in the experiment
  bool ompavail;        // true if there is OpenMP data in the experiment
  bool has_java;
  char *uarglist;       // argv[] array, as a string
  char *utargname;      // basename of argv[0] extracted from uarglist
  char *ucwd;           // working directory
  char *cversion;       // collector version string
  char *dversion;       // driver version string (er_kernel)
  char *jversion;       // Java version string (java profiling)

  // Open the named experiment record and process log file
  Exp_status open (char *directory_name);

  // Update experiment (read and process new data)
  Exp_status update ();

  // Returns collector parameters for the current sample selection
  Collection_params *
  get_params ()
  {
    return &coll_params;
  }

  Exp_status
  get_status ()
  {
    return status;
  }

  // Returns the number of samples. For use by FilterNumeric
  int
  nsamples ()
  {
    return samples->size ();
  }

  // Release any releasable memory.
  void purge ();

  void resetShowHideStack ();
  int save_notes (char*, bool);
  int delete_notes (bool);
  Experiment *getBaseFounder (); // returns topmost founder or this if no descendents

  hrtime_t
  getStartTime ()
  {
    return exp_start_time;
  }
  hrtime_t getRelativeStartTime (); // delta between start and founder's start

  hrtime_t
  getWallStartSec ()
  {
    return start_sec;
  }

  hrtime_t
  getLastEvent ()
  {
    if (last_event != ZERO_TIME)
      return last_event;
    return exp_start_time;
  }

  hrtime_t
  getGCDuration ()
  {
    return gc_duration;
  }

  int
  getPID ()
  {
    return pid;
  }

  int
  getUserExpId ()
  {
    return userExpId;
  }

  int
  getExpIdx ()
  {
    return expIdx;
  }

  void
  setExpIdx (int idx)
  {
    expIdx = idx;
  }

  void
  setUserExpId (int idx)
  {
    userExpId = idx;
  }

  void
  setTinyThreshold (int limit)
  {
    tiny_threshold = limit;
  }

  bool
  isDiscardedTinyExperiment ()
  {
    return discardTiny;
  }

  Exp_status open_epilogue ();
  void read_experiment_data (bool read_ahead);
  static int copy_file_to_archive (const char *name, const char *aname, int hide_msg);
  static int copy_file_to_common_archive (const char *name, const char *aname,
	       int hide_msg, const char *common_archive, int relative_path = 0);
  static int copy_file (char *name, char *aname, int hide_msg,
			char *common_archive = NULL, int relative_path = 0);

  // get_raw_events()
  // action: get unfiltered packets, loading them if required
  // parameters: data_id (see ProfData_type)
  DataDescriptor *get_raw_events (int data_id);
  Vector<DataDescriptor*> *getDataDescriptors ();

  // Some DATA_* types are derived from others, e.g. DATA_HEAPSZ is derived from DATA_HEAP
  // The following hooks support derived DataViews
  int base_data_id (int data_id); // returns base data_id type  (ProfData_type DATA_*)
  DataView *create_derived_data_view (int data_id, DataView *dview);

  Vector<BaseMetric*>*
  get_metric_list ()
  {
    return metrics;
  }

  char *
  get_expt_name ()
  {
    return expt_name;   // Return the pathname to the experiment
  };

  Vector<char*> *get_descendants_names ();
  char *get_fndr_arch_name ();
  char *get_arch_name ();
  char *getNameInArchive (const char *fname, bool archiveFile = false);
  char *checkFileInArchive (const char *fname, bool archiveFile = false);
  DbeFile *findFileInArchive (const char *className, const char *runTimePath);
  DbeFile *findFileInArchive (const char *fname);
  bool create_dir (char *dname);

  Vaddr
  ret_stack_base ()
  {
    return stack_base;
  };

  // Map a virtual address to a PC pair
  DbeInstr *map_Vaddr_to_PC (Vaddr addr, hrtime_t ts);
  DbeInstr *map_jmid_to_PC (Vaddr mid, int lineno, hrtime_t ts);
  Sample *map_event_to_Sample (hrtime_t ts);
  GCEvent *map_event_to_GCEvent (hrtime_t ts);

  DataView *
  getOpenMPdata ()
  {
    return openMPdata;
  }

  time_t
  get_mtime ()
  {
    return mtime;
  }

  Emsg *fetch_comments (void);  // fetch the queue of comment messages
  Emsg *fetch_runlogq (void);   // fetch the queue of run log messages
  Emsg *fetch_errors (void);    // fetch the queue of error messages
  Emsg *fetch_warnings (void);  // fetch the queue of warning messages
  Emsg *fetch_notes (void);     // fetch the queue of notes messages
  Emsg *fetch_ifreq (void);     // fetch the queue of ifreq messages
  Emsg *fetch_pprocq (void);    // fetch the queue of post-processing messages

  // message queues
  Emsgqueue *commentq;  // comments for the experiment header
  Emsgqueue *runlogq;   // used temporarily; after log file processing,
  // messages are appended to the commentq
  Emsgqueue *errorq;    // error messages
  Emsgqueue *warnq;     // warning messages
  Emsgqueue *notesq;    // user-written notes messages
  Emsgqueue *pprocq;    // postprocessing messages
  Emsgqueue *ifreqq;    // Instruction frequency data, from count experiment
  Map<const char*, LoadObject*> *loadObjMap;
  Vector<LoadObject*> *loadObjs;
  void append (LoadObject *lo);
  LoadObject *createLoadObject (const char *path, uint64_t chksum = 0);
  LoadObject *createLoadObject (const char *path, const char *runTimePath);
  SourceFile *get_source (const char *path);
  void set_clock (int clk);

  CallStack *
  callTree ()
  {
    return cstack;
  }

  CallStack *
  callTreeShowHide ()
  {
    return cstackShowHide;
  }

  uint32_t mapTagValue (Prop_type, uint64_t value);
  Histable *getTagObj (Prop_type, uint32_t idx);
  Vector<Histable*> *getTagObjs (Prop_type);

  JThread *map_pckt_to_Jthread (uint32_t tid, hrtime_t tstamp);
  JThread *get_jthread (uint32_t tid);

  Vector<JThread*> *
  get_jthreads ()
  {
    return jthreads;
  }

  Vector<GCEvent*> *
  get_gcevents ()
  {
    return gcevents;
  }

  bool need_swap_endian;
  Collection_params coll_params; // Collection params

  // Ranges for threads, lwps, cpu
  uint64_t min_thread;
  uint64_t max_thread;
  uint64_t thread_cnt;
  uint64_t min_lwp;
  uint64_t max_lwp;
  uint64_t lwp_cnt;
  uint64_t min_cpu;
  uint64_t max_cpu;
  uint64_t cpu_cnt;
  uint64_t dsevents;        // count of dataspace events
  uint64_t dsnoxhwcevents;  /* count of ds events that could be be validated
			     * because of no branch target info */

  PacketDescriptor *newPacketDescriptor (int kind, DataDescriptor *dDscr);
  PacketDescriptor *getPacketDescriptor (int kind);

  // debugging aids -- dump_stacks, dump_map
  void dump_stacks (FILE *);
  void dump_map (FILE *);

  // These methods are used in nightly performance regression testing
  void DBG_memuse (Sample *);
  void DBG_memuse (const char *sname);
  void init_cache ();

  DefaultMap<int64_t, FileData*> *
  getFDataMap ()
  {
    return fDataMap;
  }
  CallStack *cstack;

protected:

  Exp_status status;        // Error status
  Vector<SegMem*> *seg_items; // Master list of seg_items
  CallStack *cstackShowHide;
  PRBTree *maps;            // All maps in (Vaddr,time)

  hrtime_t gc_duration;     // wall-clock hrtime of total GC intervals
  hrtime_t exp_start_time;  // wall-clock hrtime at exp start
  hrtime_t last_event;      // wall-clock hrtime of last known sample or log.xml entry
  hrtime_t non_paused_time; // sum of periods where data collection is active (not paused)
  hrtime_t resume_ts;       // tracks log.xml start/resume times
  void update_last_event (hrtime_t ts /*wall time (not 0-based)*/);

  char *expt_name;      // name of experiment
  char *arch_name;      // <experiment>/archive
  char *fndr_arch_name; // <founder_experiment>/archive
  char *dyntext_name;   // <experiment>/dyntext

  int yyparse ();       // Allow yyparse actions to access
  Vaddr stack_base;     // Stack base

  // Write experiment header to comment queue
  void write_header ();
  void write_coll_params ();

  Exp_status find_expdir (char *directory_name);

  // Invoke the parser to process a file.
  void read_data_file (const char*, const char*);
  int read_log_file ();
  void read_labels_file ();
  void read_notes_file ();
  void read_archives ();
  int read_java_classes_file ();
  void read_map_file ();
  int read_overview_file ();
  int read_dyntext_file ();
  void read_omp_file ();
  void read_omp_preg ();
  void read_omp_task ();
  void read_ifreq_file ();
  void read_frameinfo_file ();

  // Functions to process the log and loadobjects file entries
  // They are deliberately made virtual to overload them
  // in er_export.
  virtual int process_arglist_cmd (char *, char *);
  virtual int process_desc_start_cmd (char *, hrtime_t, char *, char *, int, char *);
  virtual int process_desc_started_cmd (char *, hrtime_t, char *, char *, int, char *);
  virtual int process_fn_load_cmd (Module *mod, char *fname, Vaddr vaddr, int fsize, hrtime_t ts);
  virtual int process_fn_unload_cmd (char *, Vaddr, hrtime_t);
  virtual int process_hwcounter_cmd (char *, int, char *, char *, int, int, int, char *);
  virtual int process_hwsimctr_cmd (char *, int, char *, char *, char*, int, int, int, int, int);
  virtual int process_jcm_load_cmd (char*, Vaddr, Vaddr, int, hrtime_t);
  virtual int process_jcm_unload_cmd (char*, Vaddr, hrtime_t);
  virtual int process_Linux_kernel_cmd (hrtime_t);
  virtual int process_jthr_end_cmd (char *, uint64_t, Vaddr, Vaddr, hrtime_t);
  virtual int process_jthr_start_cmd (char *, char *, char *, char *, uint64_t, Vaddr, Vaddr, hrtime_t);
  virtual int process_gc_end_cmd (hrtime_t);
  virtual int process_gc_start_cmd (hrtime_t);
  virtual int process_sample_cmd (char *, hrtime_t, int id, char *lbl);
  virtual int process_sample_sig_cmd (char *, int);
  virtual int process_seg_map_cmd (char *, hrtime_t, Vaddr, int, int, int64_t, int64_t, int64_t, char *);
  virtual int process_seg_unmap_cmd (char *, hrtime_t, Vaddr);

  // creation time for experiment
  time_t mtime;
  hrtime_t exp_rel_start_time;      // start of exp. relative to founder
  bool exp_rel_start_time_set;
  Vector<UserLabel*> *userLabels;   // List of er_labels
  int userExpId;                    // user value for EXPID
  int expIdx;                       // DbeSession exp identifier
  PRBTree *jmaps;                   // JAVA_CLASSES: (id,time)->Histable
  Experiment* baseFounder;  // outermost experiment (null until lazily computed)

  // Represents a file in experiment
  class ExperimentFile;

  // XML handler to parse various experiment files
  class ExperimentHandler;
  class ExperimentLabelsHandler;

  uint64_t readPacket (Data_window *dwin, Data_window::Span *span);
  void readPacket (Data_window *dwin, char *ptr, PacketDescriptor *pDscr,
		   DataDescriptor *dDscr, int arg, uint64_t pktsz);

  // read data
  DataDescriptor *get_profile_events ();
  DataDescriptor *get_sync_events ();
  DataDescriptor *get_hwc_events ();
  DataDescriptor *get_heap_events ();
  DataDescriptor *get_heapsz_events ();
  DataDescriptor *get_iotrace_events ();
  DataDescriptor *get_race_events ();
  DataDescriptor *get_deadlock_events ();
  DataDescriptor *get_sample_events ();
  DataDescriptor *get_gc_events ();
  DataDescriptor *getDataDescriptor (int data_id);
  DataDescriptor *newDataDescriptor (int data_id, int flags = 0,
				     DataDescriptor *master_dDscr = NULL);

  // Frame info data structures and methods
  struct UIDnode;
  struct RawFramePacket;

  Vector<RawFramePacket*>*frmpckts; // frame info data
  static int frUidCmp (const void*, const void*);
  RawFramePacket *find_frame_packet (uint64_t uid);

  static const int CHUNKSZ = 16384;
  long nnodes;
  long nchunks;
  UIDnode **chunks;
  UIDnode **uidHTable;
  Vector<UIDnode*> *uidnodes;
  bool resolveFrameInfo;
  bool discardTiny;
  int tiny_threshold; /* optimize away tiny experiments which ran
		       * for less than specified time (ms): default 0 */

  static int uidNodeCmp (const void *a, const void *b);
  UIDnode *add_uid (Data_window *dwin, uint64_t uid, int size, uint32_t *array, uint64_t link_uid);
  UIDnode *add_uid (Data_window *dwin, uint64_t uid, int size, uint64_t *array, uint64_t link_uid);
  UIDnode *new_uid_node (uint64_t uid, uint64_t val);
  UIDnode *get_uid_node (uint64_t uid, uint64_t val);
  UIDnode *get_uid_node (uint64_t uid);
  UIDnode *find_uid_node (uint64_t uid);

  ExperimentFile *logFile;

  // Data descriptors
  Vector<DataDescriptor*> *dataDscrs;
  Vector<PacketDescriptor*> *pcktDscrs;
  long blksz; // binary data file block size

  // Processed data packets
  DataView *openMPdata; // OMP fork events

  // Map events to OpenMP parallel regions and tasks
  Map2D<uint32_t, hrtime_t, uint64_t> *mapPRid;
  Map2D<uint32_t, hrtime_t, void*> *mapPReg;
  Map2D<uint32_t, hrtime_t, void*> *mapTask;

  // Archive content
  Map<const char*, DbeFile *> *archiveMap;
  Map<const char*, SourceFile*>*sourcesMap;

  void init ();
  void fini ();
  void post_process ();
  void constructJavaStack (FramePacket *, UIDnode *, Map<uint64_t, uint64_t> *);
  void resolve_frame_info (DataDescriptor*);
  void cleanup_cstk_ctx_chunk ();
  void register_metric (Metric::Type type);
  void register_metric (Hwcentry *ctr, const char* aux, const char* username);

  Sample *sample_last_used;
  GCEvent *gcevent_last_used;
  char *first_sample_label;
  Module *get_jclass (const char *className, const char *fileName);
  LoadObject *get_j_lo (const char *className, const char *fileName);

  Vector<BaseMetric*> *metrics;
  Vector<JThread*> *jthreads;       // master list of Java threads
  Vector<JThread*> *jthreads_idx;   // index in the master list
  Vector<GCEvent*> *gcevents;
  Vector<UnmapChunk*> *heapUnmapEvents;
  Vector<Sample*> *samples;         // Array of Sample pointers

  DefaultMap<int64_t, FileData*> *fDataMap; // list of FileData objects using the virtual File descriptor as the key
  DefaultMap<int, int64_t> *vFdMap; // list of virtual file descrptors using the file descriptor as the key

  Vector<Vector<Histable*>*> *tagObjs; // tag objects
  bool sparse_threads;

  SegMem **smemHTable; // hash table for SegMem's
  DbeInstr **instHTable; // hash table for DbeInstr
  Map<unsigned long long, JMethod*> *jmidHTable; // hash table for jmid

  // identity of target process
  int pid;
  int ppid;
  int pgrp;
  int sid;

  // Map file processing related data
  struct MapRecord
  {

    enum
    {
      LOAD, UNLOAD
    } kind;
    Histable *obj;
    Vaddr base;
    Size size;
    hrtime_t ts;
    uint64_t foff;
  };

  void mrec_insert (MapRecord *mrec);
  SegMem *update_ts_in_maps (Vaddr addr, hrtime_t ts);
  int read_warn_file ();
  LoadObject *get_dynfunc_lo (const char *loName);
  Function *create_dynfunc (Module *mod, char *fname, int64_t vaddr, int64_t fsize);
  char *get_archived_name (const char *fname, bool archiveFile = false);

  Vector<MapRecord*> *mrecs;

private:
  void add_evt_time_to_profile_events (DataDescriptor *dDscr);
  DataView *create_heapsz_data_view (DataView *heap_dview);
  void compute_heapsz_data_view (DataView *heapsz_dview);
};

struct JThread
{
  JThread *next;
  char *name;
  char *group_name;
  char *parent_name;
  uint32_t tid;     // system thread id
  Vaddr jthr;       // recorded Java thread id
  Vaddr jenv;       // recorded JNIEnv id
  uint32_t jthr_id; // internal JThread object id
  hrtime_t start;
  hrtime_t end;

  JThread ()
  {
    name = NULL;
    group_name = NULL;
    parent_name = NULL;
  }

  ~JThread ()
  {
    free (name);
    free (group_name);
    free (parent_name);
  }
  bool is_system ();
};

struct GCEvent
{

  GCEvent ()
  {
    id = -1;
  }

  ~GCEvent () { }

  hrtime_t start;
  hrtime_t end;
  int id;
};

class ExperimentLoadCancelException
{
public:

  ExperimentLoadCancelException () { };

  ~ExperimentLoadCancelException () { };
};


#endif  /* _EEXPERIMENT_H */
