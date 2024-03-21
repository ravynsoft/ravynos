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

#ifndef _MODULE_H
#define _MODULE_H

// A Module object represents a .o file that was used to build up a segement.
//	Its main function is to compute source and disassembly annotations
// Text reordering and/or function outlining imply that a module may
//	not be contiguous.

#include "Histable.h"
#include "Hist_data.h"

#define MOD_FLAG_UNKNOWN 0x01

class LoadObject;
class MetricList;
class ComC;
class Disasm;
class Hist_data;
class Stabs;
class SourceFile;
class DataObject;
class JMethod;
template <class ITEM> class Vector;

class InlinedSubr
{
public:
  InlinedSubr ();
  DbeLine *dbeLine;
  Function *func;
  char *fname;
  uint64_t low_pc;
  uint64_t high_pc;
  int level;

  bool
  contains (InlinedSubr *p)
  {
    return low_pc <= p->low_pc && high_pc >= p->high_pc;
  }

  bool
  contains (uint64_t pc)
  {
    return low_pc <= pc && high_pc > pc;
  }
};

class Module : public HistableFile
{
public:
  // Annotated Source or Disassembly
  enum Anno_Errors
  {
    AE_OK,
    AE_NOTREAD,
    AE_NOSRC,
    AE_NOOBJ,
    AE_NOLOBJ,
    AE_NOSTABS,
    AE_NOSYMTAB,
    AE_TIMESRC,
    AE_TIMEDIS,
    AE_TIMESTABS,
    AE_TIMESTABS_DIFF,
    AE_OTHER
  };

  // The following enums are duplicated in Java
  enum Anno_Types
  {
    AT_LIST = 0,
    AT_SRC,
    AT_SRC_ONLY,
    AT_DIS,
    AT_COM,
    AT_QUOTE,
    AT_FUNC,
    AT_EMPTY,
    AT_DIS_ONLY
  };

  Module ();
  virtual ~Module ();
  virtual int64_t get_size ();
  virtual void set_name (char *str);
  virtual Vector<Histable*> *get_comparable_objs ();
  virtual int readFile ();

  virtual Histable_type
  get_type ()
  {
    return MODULE;
  }

  inline Anno_Errors
  get_status ()
  {
    return status;
  }

  inline void
  set_file_name (char *fnm)
  {
    free (file_name);
    file_name = fnm;
  }

  // get error string
  char *anno_str (char *fnm = NULL);

  // generate annotated source/disassembly data
  Hist_data *get_data (DbeView *dbev, MetricList *mlist,
		       Histable::Type type, TValue *ftotal, SourceFile *srcFile,
		       Function *func, Vector<int> *marks, int threshold,
		       int vis_bits, int src_visible, bool hex_visible,
		       bool func_scope, bool src_only,
		       Vector<int_pair_t> *marks2d = NULL,
		       Vector<int_pair_t> *marks2d_inc = NULL);

  Vector<uint64_t> *getAddrs (Function *func);
  SourceFile *setIncludeFile (char *includeFile);

  SourceFile *
  getIncludeFile ()
  {
    return curr_inc;
  }

  SourceFile *
  getMainSrc ()
  {
    return main_source;
  }

  char *
  getResolvedObjectPath ()
  {
    return stabsPath ? stabsPath : get_name ();
  }

  char *
  getDebugPath ()
  {
    setFile ();
    return stabsPath;
  }

  void read_stabs (bool all = true);
  void dump_dataobjects (FILE *out);
  DataObject *get_dobj (uint32_t dtype_id);
  void reset_datatypes ();
  void read_hwcprof_info ();
  bool is_fortran ();
  SourceFile *findSource (const char *fname, bool create);
  bool openStabs (bool all = true);
  LoadObject *createLoadObject (const char *lo_name);
  JMethod *find_jmethod (const char *nm, const char *sig);

  unsigned int flags;               // flags used for marking traversals
  Sp_lang_code lang_code;           // What is source lang. in module
  char *file_name;                  // Full path to actual source file
  Vector<Function*> *functions;     // Unordered list of functions
  LoadObject *loadobject;           // Parent loadobject
  LoadObject *dot_o_file;           // The .o file with debug information
  unsigned fragmented;              // -xF used when compiling module
  int real_timestamp;               // Linked timestamp from N_OPT stab
  int curr_timestamp;               // Current object timestamp from N_OPT stab
  char *comp_flags;                 // compiler flags used to compile module
  char *comp_dir;                   // directory used to compile module
  char *linkerStabName;             // Name from 'N_UNDF' stab
  Stabs *objStabs;                  // stabs of object file
  bool readStabs;
  bool hasStabs;
  bool hasDwarf;
  uint64_t hdrOffset;               // offset in .debug_info
  unsigned hwcprof;                 // hwcprof info status
  Vector<inst_info_t*> *infoList;   // merged list
  Vector<memop_info_t*> ldMemops;   // load instructions
  Vector<memop_info_t*> stMemops;   // store instructions
  Vector<memop_info_t*> pfMemops;   // prefetch instructions
  Vector<target_info_t*> bTargets;  // branch targets
  Vector<datatype_t*> *datatypes;   // object type descriptors
  Vector<SourceFile*> *includes;
  Module *indexStabsLink;           // correspondent module for the .o file
  InlinedSubr *inlinedSubr;

protected:
  void removeStabsTmp (); // Remove temporary *.o (got from *.a)

  // Check timestamp, warn users if src/dis/stabs later than exp.
  Anno_Errors checkTimeStamp (bool chkDis);

  // Set paths for reading Stabs and Symbols
  bool read_ar (int ar, int obj, char *obj_base);
  bool setFile ();

  // Open appropriate symbol tables, construct set of PC ranges,
  //	and maps to source lines for each PC
  Stabs *openDebugInfo ();

  // Construct PC index table
  bool openDisPC ();

  // Compute data--scan data to compute metrics per-src-line/dis-line
  bool computeMetrics (DbeView *dbev, Function *func, MetricList *mlist,
		       Histable::Type type, bool src_metric,
		       bool func_scope, SourceFile *source);
  void init_line ();
  void init_index (Hist_data *witems, int &wlindex, int &wmsize, int &wmindex);

  void set_src_data (Function *func, int vis_bits, int cmpline_visible,
		     int funcline_visible);
  void set_dis_data (Function *func, int vis_bits, int cmpline_visible,
		     int src_visible, bool hex_vis, bool func_scope,
		     int funcline_visible);
  void set_src (Anno_Types type, DbeLine *dbeline);
  void set_dis (DbeInstr *instr, Anno_Types type, bool nextFile, char *dis_str);
  void set_MPSlave ();
  void set_one (Hist_data::HistItem *org_item, Anno_Types type, const char *text);
  void set_ComCom (int vis_bits);

  virtual char *get_disasm (uint64_t inst_address, uint64_t end_address,
			    uint64_t start_address, uint64_t f_offset,
			    int64_t &inst_size);

  Anno_Errors status;
  Anno_Errors openSourceFlag;
  bool hexVisible;          // show hex code in disasm
  time_t disMTime;          // Creating time for disassembly
  time_t stabsMTime;        // Creating time for stabs
  SourceFile *main_source;
  SourceFile *curr_inc;     // pointer to include file or NULL
  SourceFile *srcContext;
  Vector<ComC*> *comComs;   // table of compiler comments
  Disasm *disasm;
  Hist_data *src_items;
  Hist_data *dis_items;
  Hist_data *data_items;
  DbeView * cur_dbev;
  TValue *total;
  TValue *maximum;
  TValue *maximum_inc;
  TValue *empty;
  int name_idx;         // index of name metric in list for src/dis
  int size_index;       // index of size metric in list for src/dis
  int addr_index;       // index of address metric in list for src/dis

  int curline;          // line# of next source line to be processed
  int cindex, cline;    // index and src line of next compiler-comment
  int sindex, sline;    // index and src line of next item in src_items
  int dindex;
  DbeInstr *daddr;      // pointer to next DbeInstr with metrics
  int mindex;           // MP index and src line of next metric-value
  int mline;            // MP line to be processed by source

  char *disPath;        // path for disassembly
  char *stabsPath;      // path for reading stabs
  char *stabsTmp;       // temporary *.o from *.a
  char *disName;        // library/path for disassembly
  char *stabsName;      // library/path for stabs
};

#endif /* _MODULE_H */
