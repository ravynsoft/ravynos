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

#ifndef _TABLE_H
#define _TABLE_H

#include "vec.h"
#include "Map2D.h"

#include "dbe_structs.h"

class FilterExp;
struct PropDescr;
struct FieldDescr;
class PacketDescriptor;
class DataDescriptor;
class DataView;

// Note: order must match VTYPE_TYPE_NAMES, below

enum VType_type
{
  TYPE_NONE,
  TYPE_INT32,
  TYPE_UINT32,
  TYPE_INT64,
  TYPE_UINT64,
  TYPE_STRING,
  TYPE_DOUBLE,
  TYPE_OBJ,
  TYPE_DATE, // Used in FieldDescr only, mapped to TYPE_UINT64 in PropDescr
  TYPE_BOOL, // Used only to describe filter props
  TYPE_ENUM, // Used only to describe filter props

  TYPE_LAST
};

#define VTYPE_TYPE_NAMES \
{ \
    NTXT("NONE"), \
    NTXT("INT32"), \
    NTXT("UINT32"), \
    NTXT("INT64"), \
    NTXT("UINT64"), \
    NTXT("STRING"), \
    NTXT("DOUBLE"), \
    NTXT("OBJECT"), \
    NTXT("DATE"), \
    NTXT("BOOL"), \
    NTXT("ENUM") \
}

// Note: order must match PROFDATA_TYPE_NAMES and PROFDATA_TYPE_UNAMES, below

enum ProfData_type
{ // a.k.a "data_id" (not the same as Pckt_type "kind")
  DATA_SAMPLE,      // Traditional collect "Samples"
  DATA_GCEVENT,     // Java Garbage Collection events
  DATA_HEAPSZ,      // heap size tracking based on heap tracing data
  DATA_CLOCK,       // clock profiling data
  DATA_HWC,         // hardware counter profiling data
  DATA_SYNCH,       // synchronization tracing data
  DATA_HEAP,        // heap tracing data
  DATA_MPI,         // MPI tracing data
  DATA_RACE,        // data race detection data
  DATA_DLCK,        // deadlock detection data
  DATA_OMP,         // OpenMP profiling data (fork events)
  DATA_OMP2,        // OpenMP profiling data (enter thread events)
  DATA_OMP3,        // OpenMP profiling data (enter task events)
  DATA_OMP4,        // OpenMP profiling data (parreg descriptions)
  DATA_OMP5,        // OpenMP profiling data (task descriptions)
  DATA_IOTRACE,     // IO tracing data
  DATA_LAST
};

extern char *get_prof_data_type_name (int t);
extern char *
get_prof_data_type_uname (int t);

enum Prop_type
{
  PROP_NONE,
  // commonly used properties (libcollector modules, er_print)
  PROP_ATSTAMP,     // hrtime_t, Filter: system HRT timestamp;
		    // "Absolute TSTAMP"
  PROP_ETSTAMP,     // hrtime_t, Filter: nanoseconds from subexperiment start;
		    // "subExperiment TSTAMP"
  PROP_TSTAMP,      // hrtime_t, Packet: system HRT timestamp
		    // Filter: nanoseconds from founder start
  PROP_THRID,       // mapped to uint32_t by readPacket
  PROP_LWPID,       // mapped to uint32_t by readPacket
  PROP_CPUID,       // mapped to uint32_t by readPacket
  PROP_FRINFO,      // uint64_t	frinfo
  PROP_EVT_TIME,    // hrtime_t Filter: Time delta
  // If TSTAMP taken at end of event, EVT_TIME will be positive
  // If TSTAMP taken at start of event, EVT_TIME will be negative
  // Note: clock and hwc profile events set EVT_TIME=0
  //    except Solaris Microstate events where NTICK>1:
  //    These will use EVT_TIME=(NTICK-1)*<tick duration>

  // DATA_SAMPLE
  PROP_SAMPLE,      // uint64_t sample number
  PROP_SMPLOBJ,     // Sample*

  // DATA_GCEVENT
  PROP_GCEVENT,     // uint64_t event id
  PROP_GCEVENTOBJ,  // GCEvent*

  // DATA_CLOCK
  PROP_MSTATE,      // unsigned	ProfilePacket::mstate
  PROP_NTICK,       // unsigned	ProfilePacket::value
  PROP_OMPSTATE,    // int ProfilePacket::ompstate
  PROP_MPISTATE,    // int ProfilePacket::mpistate

  // DATA_SAMPLE     // see PrUsage class, see PROP_MSTATE - TBR?
  PROP_UCPU,
  PROP_SCPU,
  PROP_TRAP,
  PROP_TFLT,
  PROP_DFLT,
  PROP_KFLT,
  PROP_ULCK,
  PROP_TSLP,
  PROP_WCPU,
  PROP_TSTP,

  // DATA_SYNCH
  PROP_SRQST,       // hrtime_t SyncPacket::requested
  PROP_SOBJ,        // Vaddr SyncPacket::objp

  // DATA_HWC
  PROP_HWCTAG,      // uint32_t HWCntrPacket::tag;
  PROP_HWCINT,      // uint64_t HWCntrPacket::interval
  PROP_VADDR,       // Vaddr HWCntrPacket::dbeVA->eaddr
  PROP_PADDR,       // Vaddr HWCntrPacket::dbePA->eaddr
  PROP_HWCDOBJ,     // DataObject* HWCntrPacket::dobj
  PROP_VIRTPC,      // Vaddr HWCntrPacket::eventVPC
  PROP_PHYSPC,      // Vaddr HWCntrPacket::eventPPC
  PROP_EA_PAGESIZE, // uint32_t HWCntrPacket::ea_pagesize
  PROP_PC_PAGESIZE, // uint32_t HWCntrPacket::pc_pagesize
  PROP_EA_LGRP,     // uint32_t HWCntrPacket::ea_lgrp
  PROP_PC_LGRP,     // uint32_t HWCntrPacket::pc_lgrp
  PROP_LWP_LGRP_HOME, // uint32_t HWCntrPacket::lwp_lgrp_home
  PROP_PS_LGRP_HOME,  // uint32_t HWCntrPacket::ps_lgrp_home
  PROP_MEM_LAT,     // uint64_t HWCntrPacket::latency
  PROP_MEM_SRC,     // uint64_t HWCntrPacket::data_source

  // DATA_HEAP
  PROP_HTYPE,       // Heap_type HeapPacket::mtype
  PROP_HSIZE,       // Size HeapPacket::size (bytes alloc'd by this event)
  PROP_HVADDR,      // Vaddr HeapPacket::vaddr
  PROP_HOVADDR,     // Vaddr HeapPacket::ovaddr
  PROP_HLEAKED,     // Size HeapPacket::leaked (net bytes leaked)
  PROP_HMEM_USAGE,  // Size heap memory usage
  PROP_HFREED,      // Size (bytes freed by this event)
  PROP_HCUR_ALLOCS, // int64_t (net allocations running total.  Recomputed after each filter)
  PROP_HCUR_NET_ALLOC, // int64_t (net allocation for this packet.  Recomputed after each filter)
  PROP_HCUR_LEAKS,  // Size (net leaks running total.  Recomputed after each filter)

  // DATA_IOTRACE
  PROP_IOTYPE,      // IOTrace_type IOTracePacket::iotype
  PROP_IOFD,        // int32_t IOTracePacket::fd
  PROP_IONBYTE,     // Size_type IOTracePacket::nbyte
  PROP_IORQST,      // hrtime_t IOTracePacket::requested
  PROP_IOOFD,       // int32_t IOTracePacket::ofd
  PROP_IOFSTYPE,    // FileSystem_type IOTracePacket::fstype
  PROP_IOFNAME,     // char IOTracePacket::fname
  PROP_IOVFD,       // int32_t virtual file descriptor

  // DATA_MPI
  PROP_MPITYPE,     // MPI_type MPIPacket::mpitype
  PROP_MPISCOUNT,   // Size MPIPacket::scount
  PROP_MPISBYTES,   // Size MPIPacket::sbytes
  PROP_MPIRCOUNT,   // Size MPIPacket::rcount
  PROP_MPIRBYTES,   // Size MPIPacket::rbytes

  // DATA_OMP*
  PROP_CPRID,       // uint64_t (Note: not same as "PROP_CPRID" below)
  PROP_PPRID,       // uint64_t OMPPacket::omp_pprid
  PROP_TSKID,       // uint64_t (Note: not same as "PROP_CPRID" below)
  PROP_PTSKID,      // uint64_t OMPPacket::omp_ptskid
  PROP_PRPC,        // uint64_t OMPPacket::omp_prpc

  // DATA_RACE
  PROP_RTYPE,       // Race_type RacePacket::rtype
  PROP_RID,         // uint32_t RacePacket::id
  PROP_RVADDR,      // Vaddr RacePacket::vaddr
  PROP_RCNT,        // uint32_t RacePacket::count
  PROP_LEAFPC,      // Vaddr CommonPacket::leafpc

  // DATA_DLCK
  PROP_DID,         // uint32_t DeadlockPacket::id
  PROP_DTYPE,       // Deadlock_Lock_type DeadlockPacket::lock_type
  PROP_DLTYPE,      // Deadlock_type DeadlockPacket::dl_type
  PROP_DVADDR,      // Vaddr DeadlockPacket::lock_addr

  // Synthetic properties (queries only)
  PROP_STACKID,
  PROP_STACK,       // void* Generic; mapped to M, U, or XSTACK
  PROP_MSTACK,      // void* machine stack
  PROP_USTACK,      // void* user_stack
  PROP_XSTACK,      // void* expert_stack
  PROP_HSTACK,      // void* hide_stack
  //PROP_CPRID,       // void* (Note: not same as "PROP_CPRID" above)
  //PROP_TSKID,       // void* (Note: not same as "PROP_TSKID" above)
  PROP_JTHREAD,     // JThread* CommonPacket::jthread
  PROP_LEAF,        // uint64_t stack leaf function
  PROP_DOBJ,        // "DOBJ" DataObject*
  PROP_SAMPLE_MAP,  // Map events to SAMPLE using sample's time range
  PROP_GCEVENT_MAP, // Map events to GCEVENT using gcevent's time range
  PROP_PID,         // int unix getpid()
  PROP_EXPID,       // int Experiment->getUserExpId(), AKA process number, >=1.
  PROP_EXPID_CMP,   // int "Comparable PROP_EXPID".  In compare mode, if this
  //              process has been matched to another groups' process,
  //              returns PROP_EXPID of the matching process with the
  //              lowest PROP_EXPGRID value.  Otherwise returns PROP_EXPID.
  PROP_EXPGRID,     // int Comparison group number.  >=0, 0 is Baseline.
  PROP_PARREG,      // "PARREG" uint64_t (see 6436500) TBR?
  PROP_TSTAMP_LO,   // hrtime_t Filter: Event's low TSTAMP
  PROP_TSTAMP_HI,   // hrtime_t Filter: Event's high TSTAMP
  PROP_TSTAMP2,     // hrtime_t Filter: End TSTAMP (TSTAMP<=TSTAMP2)
  PROP_FREQ_MHZ,    // int frequency in MHZ (for converting HWC profiling cycles to time)
  PROP_NTICK_USEC,  // hrtime_t Clock profiling interval, microseconds (PROP_NTICK * Experiment->ptimer_usec)
  PROP_IOHEAPBYTES, // Size PROP_HSIZE or PROP_IONBYTE
  PROP_STACKL,      // void* Generic; mapped to M, U, or XSTACK for DbeLine
  PROP_MSTACKL,     // void* machine stack
  PROP_USTACKL,     // void* user_stack
  PROP_XSTACKL,     // void* expert_stack
  PROP_STACKI,      // void* Generic; mapped to M, U, or XSTACK for DbeInstr
  PROP_MSTACKI,     // void* machine stack
  PROP_USTACKI,     // void* user_stack
  PROP_XSTACKI,     // void* expert_stack
  PROP_DDSCR_LNK,   // long long index into DataDescriptor table for a related event
  PROP_VOIDP_OBJ,   // void* pointer to object containing metadata
  PROP_LAST
};

enum Prop_flag
{
  PRFLAG_NOSHOW     = 0x40
};

struct PropDescr
{
  PropDescr (int propID, const char *name);
  virtual ~PropDescr ();

  void addState (int value, const char *stname, const char *stuname);
  char *getStateName (int value);
  char *getStateUName (int value);

  int
  getMaxState ()
  {
    return stateNames ? stateNames->size () : 0;
  }

  int propID;
  char *name;
  char *uname;
  VType_type vtype;
  int flags;

private:
  Vector<char*>*stateNames;
  Vector<char*>*stateUNames;
};

struct FieldDescr
{
  FieldDescr (int propID, const char *name);
  virtual ~FieldDescr ();

  int propID;
  char *name;
  int offset;
  VType_type vtype;
  char *format;
};

class PacketDescriptor
{
public:
  PacketDescriptor (DataDescriptor*);
  virtual ~PacketDescriptor ();

  DataDescriptor *
  getDataDescriptor ()
  {
    return ddscr;
  }

  Vector<FieldDescr*> *
  getFields ()
  {
    return fields;
  }

  void addField (FieldDescr*);

private:
  DataDescriptor *ddscr;
  Vector<FieldDescr*> *fields;
};

struct Datum
{

  void
  setUINT32 (uint32_t vv)
  {
    type = TYPE_UINT32;
    i = vv;
  }

  void
  setUINT64 (uint64_t vv)
  {
    type = TYPE_UINT64;
    ll = vv;
  }

  void
  setSTRING (char* vv)
  {
    type = TYPE_STRING;
    l = vv;
  }

  void
  setDOUBLE (double vv)
  {
    type = TYPE_DOUBLE;
    d = vv;
  }

  void
  setOBJ (void* vv)
  {
    type = TYPE_OBJ;
    p = vv;
  }

  VType_type type;
  union
  {
    int i;
    double d;
    char *l;
    void *p;
    unsigned long long ll;
  };
};

class Data
{
public:
  static Data *newData (VType_type);

  virtual
  ~Data () { }

  virtual VType_type
  type ()
  {
    return TYPE_NONE;
  }
  virtual void reset () = 0;
  virtual long getSize () = 0;
  virtual int fetchInt (long i) = 0;
  virtual unsigned long long fetchULong (long i) = 0;
  virtual long long fetchLong (long i) = 0;
  virtual char *fetchString (long i) = 0;
  virtual double fetchDouble (long i) = 0;
  virtual void *fetchObject (long i) = 0;
  virtual void setDatumValue (long, const Datum*) = 0;
  virtual void setValue (long, uint64_t) = 0;
  virtual void setObjValue (long, void*) = 0;
  virtual int cmpValues (long idx1, long idx2) = 0;
  virtual int cmpDatumValue (long idx, const Datum *val) = 0;
};

enum Data_flag
{
  DDFLAG_NOSHOW = 0x01
};

class DataDescriptor
{
  /*
   * An instance of this class stores the data packets for a specific
   * type of profiling, for example, clock profiling.
   *
   * Each packet consists of values for various properties.
   * For example, a timestamp is a property which is accessed with PROP_TSTAMP.
   *
   * Ideally, DataDescriptor contents are considered immutable after the
   * data is read in.  setValue() should only be used during creation.
   * - The packets are in fixed order.  This allows DataDescriptor <pkt_id>
   *   to be treated as a stable handle.
   * - Sorting/filtering is handled by the DataView class
   * - In the future, if we need to add the ability to append new packets,
   *   we might add a flag to show when the class is immutable and/or appendible
   */
public:

  DataDescriptor (int id, const char* name, const char* uname, int flags = 0); // master
  DataDescriptor (int id, const char* name, const char* uname, DataDescriptor*); // reference copy
  ~DataDescriptor ();

  // packets' descriptions
  int
  getId ()
  {
    return id;
  }

  char *
  getName ()
  {
    return name;
  }

  char *
  getUName ()
  {
    return uname;
  }

  Vector<PropDescr*> *
  getProps ()
  {
    return props;       // packet properties
  }
  PropDescr *getProp (int prop_id);     // packet property

  long
  getSize ()
  {
    return *ref_size;   // number of packets
  }

  long
  getFlags ()
  {
    return flags;
  }

  // class to provide sorting and filtering
  DataView *createView ();
  DataView *createImmutableView ();
  DataView *createExtManagedView ();

  // packet property values (<pkt_id> is stable packet handle)
  int getIntValue (int prop_id, long pkt_id);
  unsigned long long getULongValue (int prop_id, long pkt_id);
  long long getLongValue (int prop_id, long pkt_id);
  void *getObjValue (int prop_id, long pkt_id);
  Vector<long long> *getSet (int prop_id); // list of sorted, unique values

  // table creation/reset
  void addProperty (PropDescr*); // add property to all packets
  long addRecord ();            // add packet
  Data *getData (int prop_id);  // get all packets
  void setDatumValue (int prop_id, long pkt_id, const Datum *val);
  void setValue (int prop_id, long pkt_id, uint64_t val);
  void setObjValue (int prop_id, long pkt_id, void *val);
  void reset ();                // remove all packets (ym: TBR?)

  void
  setResolveFrInfoDone ()
  {
    *ref_resolveFrameInfoDone = true;
  }

  bool
  isResolveFrInfoDone ()
  {
    return *ref_resolveFrameInfoDone;
  }


private:
  bool isMaster;
  int flags;        // see Data_flag enum
  int id;
  char *name;
  char *uname;

  // the following should only be accessed if parent==NULL
  long master_size;
  bool master_resolveFrameInfoDone;

  // the following point to the master DataDescriptor's fields
  long *ref_size;
  bool *ref_resolveFrameInfoDone;
  Vector<PropDescr*> *props;
  Vector<Data*> *data;
  Vector<Vector<long long>*> *setsTBR; // Sets of unique values
};

typedef struct
{
  long begin;
  long end;
  long orig_ddsize;
  DataView *tmpView;
  long *idxArr;
  FilterExp *fltr;
} fltr_dbe_ctx;

class DataView
{
  /*
   * Provides sorting and filtering of DataDescriptor packets
   */
public:

  enum Relation
  {
    REL_LT,
    REL_LTEQ,
    REL_EQ,
    REL_GTEQ,
    REL_GT
  };

  enum DataViewType
  {
    DV_NORMAL,      // filterable, sortable
    DV_IMMUTABLE,   // reflects exact data in DataDescriptor
    DV_EXT_MANAGED  // sortable.  index[] entries managed externally.
  };

  DataView (DataDescriptor*);
  DataView (DataDescriptor*, DataViewType);
  virtual ~DataView ();

  Vector<PropDescr*> *getProps ();
  PropDescr *getProp (int prop_id);
  long getSize ();      // number of post-filter packets

  // packet property values accessed by sort index (not DataDescriptor pkt_id)
  int getIntValue (int prop_id, long idx);
  unsigned long long getULongValue (int prop_id, long idx);
  long long getLongValue (int prop_id, long idx);
  void *getObjValue (int prop_id, long idx);
  long getIdByIdx (long idx);   // returns DataDescriptor pkt_id

  // define sort/filter
  void sort (const int props[], int prop_count);
  void sort (int prop);
  void sort (int prop1, int prop2);
  void sort (int prop1, int prop2, int prop3);
  void setFilter (FilterExp*);

  // search packets
  // - sort must already be defined
  // - requires the user to provide all properties used in current sort.
  // - For a match, the all but the last sort property (the "leaf")
  //   must match exactly.
  long getIdxByVals (const Datum valColumns[], Relation rel);
  long getIdxByVals (const Datum valColumns[], Relation rel,
		     long minIdx, long maxIdx); //limit idx search range
  bool idxRootDimensionsMatch (long idx, const Datum valColumns[]);
  // packet at idx matches all non-leaf values in valColumns

  // use during table creation, updates underlying DataDescriptor
  void setDatumValue (int prop_id, long idx, const Datum *val);
  void setValue (int prop_id, long idx, uint64_t val);
  void setObjValue (int prop_id, long idx, void *val);

  DataDescriptor *
  getDataDescriptor ()
  {
    return ddscr;
  }

  void removeDbeViewIdx (long idx);

  // for use with DV_EXT_MANAGED DataViews:
  void appendDataDescriptorId (long pkt_id);
  void setDataDescriptorValue (int prop_id, long pkt_id, uint64_t val);
  long long getDataDescriptorValue (int prop_id, long pkt_id);

private:
  bool checkUpdate ();
  void init (DataDescriptor*, DataViewType);

  static void filter_in_chunks (fltr_dbe_ctx *dctx);
  DataDescriptor *ddscr;
  long ddsize;
  Vector<long> *index; // sorted vector of data_id (index into dDscr)
#define MAX_SORT_DIMENSIONS 10
#define DATA_SORT_EOL ((Data *) -1)     /* marks end of sortedBy[] array */
  Data *sortedBy[MAX_SORT_DIMENSIONS + 1]; // columns for sort
  FilterExp *filter;
  DataViewType type;
};

#endif /* _TABLE_H */
