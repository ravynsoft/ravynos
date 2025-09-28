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

#ifndef _BASEMETRIC_H
#define _BASEMETRIC_H

#include "dbe_structs.h"
#include "hwcentry.h"
#include "Table.h"

// METRIC_*_PRECISION determine the least threshold value
// for time measured metrics. Any event that counts for less
// than 1sec/METRIC_PRECISION is discarded.
#define METRIC_SIG_PRECISION    MICROSEC
#define METRIC_HR_PRECISION     MICROSEC

class Expression;
class Definition;
class Histable;
template <class ITEM> class Vector;

class BaseMetric
{
public:
  // sync enum changes with AnMetric.java
  enum Type
  { // Subtype==STATIC metrics:
    ONAME = 1,  //ONAME must be 1
    SIZES,
    ADDRESS,
    // Clock Profiling, Derived Metrics:
    CP_TOTAL,
    CP_TOTAL_CPU,
    // Clock profiling, Solaris Microstates (LMS_* defines)
    CP_LMS_USER,
    CP_LMS_SYSTEM,
    CP_LMS_TRAP,
    CP_LMS_TFAULT,
    CP_LMS_DFAULT,
    CP_LMS_KFAULT,
    CP_LMS_USER_LOCK,
    CP_LMS_SLEEP,
    CP_LMS_WAIT_CPU,
    CP_LMS_STOPPED,
    // Kernel clock profiling
    CP_KERNEL_CPU,
    // Sync Tracing
    SYNC_WAIT_TIME,
    SYNC_WAIT_COUNT,
    // HWC
    HWCNTR,
    // Heap Tracing:
    HEAP_ALLOC_CNT,
    HEAP_ALLOC_BYTES,
    HEAP_LEAK_CNT,
    HEAP_LEAK_BYTES,
    // I/O Tracing:
    IO_READ_BYTES,
    IO_READ_CNT,
    IO_READ_TIME,
    IO_WRITE_BYTES,
    IO_WRITE_CNT,
    IO_WRITE_TIME,
    IO_OTHER_CNT,
    IO_OTHER_TIME,
    IO_ERROR_CNT,
    IO_ERROR_TIME,
    // MPI Tracing:
    MPI_TIME,
    MPI_SEND,
    MPI_BYTES_SENT,
    MPI_RCV,
    MPI_BYTES_RCVD,
    MPI_OTHER,
    // OMP states:
    OMP_NONE,
    OMP_OVHD,
    OMP_WORK,
    OMP_IBAR,
    OMP_EBAR,
    OMP_WAIT,
    OMP_SERL,
    OMP_RDUC,
    OMP_LKWT,
    OMP_CTWT,
    OMP_ODWT,
    OMP_MSTR,
    OMP_SNGL,
    OMP_ORDD,
    OMP_MASTER_THREAD,
    // MPI states:
    MPI_WORK,
    MPI_WAIT,
    // Races and Deadlocks
    RACCESS,
    DEADLOCKS,
    // Derived Metrics
    DERIVED
  };

  // sync enum changes with AnMetric.java
  enum SubType
  {
    STATIC      = 1, // Type==SIZES, ADDRESS, ONAME
    EXCLUSIVE   = 2,
    INCLUSIVE   = 4,
    ATTRIBUTED  = 8,
    DATASPACE   = 16 // Can be accessed in dataspace views
  };

  BaseMetric (Type t);
  BaseMetric (Hwcentry *ctr, const char* _aux, const char* _cmdname,
	      const char* _username, int v_styles); // depended bm
  BaseMetric (Hwcentry *ctr, const char* _aux, const char* _username,
	      int v_styles, BaseMetric* _depended_bm = NULL); // master bm
  BaseMetric (const char *_cmd, const char *_username, Definition *def); // derived metrics
  BaseMetric (const BaseMetric& m);
  virtual ~BaseMetric ();

  int get_id ()                     { return id; }
  Type get_type ()                  { return type; }
  Hwcentry *get_hw_ctr ()           { return hw_ctr; }
  char *get_aux ()                  { return aux; }
  char *get_username ()             { return username; }
  char *get_cmd ()                  { return cmd; }
  int get_flavors ()                { return flavors; }
  int get_clock_unit ()             { return clock_unit; }
  long long get_precision ()        { return precision; }
  ValueTag get_vtype ()             { return valtype; }
  int get_value_styles ()           { return value_styles; }
  bool is_zeroThreshold ()          { return zeroThreshold; }
  ProfData_type get_packet_type ()  { return packet_type; }
  Expression *get_cond ()           { return cond; }
  Expression *get_val ()            { return val; }
  Expression *get_expr ()           { return expr; }
  char *get_expr_spec ()            { return expr_spec; }
  Definition *get_definition ()     { return definition; };
  BaseMetric *get_dependent_bm ()   { return dependent_bm; };

  bool
  comparable ()
  {
    return val_spec != NULL || type == DERIVED || type == SIZES || type == ADDRESS;
  }

  // setters..
  void set_default_visbits (SubType subtype, int _visbits);
  void set_id (int _id)         { id = _id; }   //TBR, if possible
  // For comparison, set which packets to eval:
  void set_expr_spec (char *_expr_spec);
  void set_cond_spec (char *_cond_spec);
  int get_default_visbits (SubType subtype);
  char *dump ();
  Histable *get_comparable_obj (Histable *obj);
  bool is_internal ();          // Invisible for users

  char *legend;                 // for comparison: add'l column text

private:
  BaseMetric *dependent_bm;     // for HWCs only: a link to the timecvt metric
  Expression *cond;             // determines which packets to evaluate
  char *cond_spec;              // used to generate "cond"
  Expression *val;              // determines the numeric value for packet
  char *val_spec;               // used to generate "val"
  Expression *expr; // for comparison: an additional expression to determine
		    // which packets to eval. Should be NULL otherwise.
  char *expr_spec;                  // used to generate "expr"
  int id;                           // unique id (assigned to last_id @ "new")
  Type type;                        // e.g. HWCNTR
  char *aux;                        // for HWCs only: Hwcentry ctr->name
  char *cmd;                        // the .rc metric command, e.g. "total"
  char *username;                   // e.g. "GTXT("Total Wait Time")"
  int flavors;                      // bitmask of SubType capabilities
  int value_styles;                 // bitmask of ValueType capabilities
  static const int NSUBTYPES = 2;   // STATIC/EXCLUSIVE, INCLUSIVE
  int default_visbits[NSUBTYPES];   // ValueType, e.g. VAL_VALUE|VAL_TIMEVAL
  ValueTag valtype;                 // e.g. VT_LLONG
  long long precision;              // e.g. METRIC_SIG_PRECISION, 1, etc.
  Hwcentry *hw_ctr;                 // HWC definition
  ProfData_type packet_type;        // e.g. DATA_HWC, or -1 for N/A
  bool zeroThreshold;               // deadlock stuff
  Presentation_clock_unit clock_unit;

  static int last_id;           // incremented by 1 w/ every "new". Not MT-safe
  Definition *definition;

  void hwc_init (Hwcentry *ctr, const char* _aux, const char* _cmdname, const char* _username, int v_styles);
  void init (Type t);
  char *get_basetype_name ();
  void specify ();
  void specify_metric (char *_cond_spec, char *_val_spec);
  void set_val_spec (char *_val_spec);
  void specify_mstate_metric (int st);
  void specify_ompstate_metric (int st);
  void specify_prof_metric (char *_cond_spec);
};

class Definition
{
public:

  enum opType
  {
    opNULL,
    opPrimitive,
    opDivide
  };

  Definition (opType _op);
  ~Definition ();
  static Definition *add_definition (char *_def);
  Vector<BaseMetric *> *get_dependencies ();
  long *get_map ();
  double eval (long *indexes, TValue *values);

  opType op;
  Definition *arg1;
  Definition *arg2;
  char *def;

private:
  BaseMetric *bm;
  long *map;
  Vector<BaseMetric *> *dependencies;
  long index;
};

#endif  /* _BASEMETRIC_H */

