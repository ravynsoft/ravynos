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

#ifndef _LOADOBJECT_H
#define _LOADOBJECT_H

// A Segment object represents a segment of the program text.

#include "Histable.h"
#include "Stabs.h"
#include "DbeLock.h"

#define JAVA_COMPILED_METHODS   "JAVA_COMPILED_METHODS"
#define DYNFUNC_SEGMENT         "DYNAMIC_FUNCTIONS"
#define SEG_FLAG_DYNAMIC    0x01
#define SEG_FLAG_JVM        0x02
#define SEG_FLAG_OMP        0x04
#define SEG_FLAG_EXE        0x08
#define SEG_FLAG_REORDER    0x10

/* Hash name for all comparable executions */
#define COMP_EXE_NAME       "<COMP_EXE_NAME>"

class Emsg;
class Elf;
class Experiment;
class Function;
class Module;
template <typename Key_t, typename Value_t> class HashMap;
template <typename Key_t, typename Value_t> class Map;
template <class ITEM> class Vector;

enum
{
  CMP_PATH          = 1,
  CMP_RUNTIMEPATH   = 2,
  CMP_CHKSUM        = 4
};

class LoadObject : public HistableFile, public DbeLock
{
public:

  // The various segments types.
  enum seg_type
  {
    SEG_TEXT,
    SEG_DATA,
    SEG_BSS,
    SEG_HEAP,
    SEG_STACK,
    SEG_DEVICE,
    SEG_UNKNOWN
  };

  // These codes are stored in *.archive files
  enum Arch_status
  {
    ARCHIVE_SUCCESS,
    ARCHIVE_EXIST,
    ARCHIVE_BAD_STABS,
    ARCHIVE_ERR_SEG,
    ARCHIVE_ERR_OPEN,
    ARCHIVE_ERR_MAP,
    ARCHIVE_WARN_MTIME,
    ARCHIVE_WARN_HOST,
    ARCHIVE_ERR_VERSION,
    ARCHIVE_NO_STABS,
    ARCHIVE_WRONG_ARCH,
    ARCHIVE_NO_LIBDWARF,
    ARCHIVE_NO_DWARF,
    ARCHIVE_WARN_CHECKSUM
  };

  LoadObject (const char *loname);

  static LoadObject *create_item (const char *nm, int64_t chksum);
  static LoadObject *create_item (const char *nm, const char *_runTimePath, DbeFile *df);

  virtual ~LoadObject ();
  virtual void set_name (char *string);
  virtual uint64_t get_addr ();
  virtual Vector<Histable*> *get_comparable_objs ();

  virtual Histable_type
  get_type ()
  {
    return LOADOBJECT;
  };

  virtual int64_t
  get_size ()
  {
    return size;
  }

  char *
  get_pathname ()
  {
    return pathname;
  }

  void
  set_archname (char *aname)
  {
    free (arch_name);
    arch_name = aname;
  }

  bool
  is_relocatable ()
  {
    return isRelocatable;
  }

  bool compare (const char *nm, int64_t _checksum);
  int compare (const char *_path, const char *_runTimePath, DbeFile *df);
  void set_platform (Platform_t pltf, int wsz);
  void dump_functions (FILE *);
  int get_index (Function *func);
  char *get_alias (Function *func);
  DbeInstr *find_dbeinstr (uint64_t file_off);
  Function *find_function (uint64_t offset);
  Function *find_function (char *fname);
  Function *find_function (char *fname, unsigned int chksum);
  Module *find_module (char *mname);
  Module *get_comparable_Module (Module *mod);
  void append_module (Module *mod);
  Elf *get_elf ();
  Stabs *openDebugInfo (char *fname, Stabs::Stab_status *stp = NULL);
  Arch_status read_stabs ();
  Arch_status sync_read_stabs ();
  void post_process_functions ();
  char *status_str (Arch_status rv, char *arg = NULL);
  Function *get_hide_function ();
  DbeInstr *get_hide_instr (DbeInstr *instr);
  uint32_t get_checksum ();

  Emsg *
  fetch_warnings (void) // fetch the queue of warning messages
  {
    return warnq->fetch ();
  }

  Emsg *
  fetch_comments (void) // fetch the queue of comment messages
  {
    return commentq->fetch ();
  }

  unsigned int flags;           // SEG_FLAG_*
  bool isReadStabs;
  bool need_swap_endian;
  int seg_idx;                  // for compatibility (ADDRESS)
  seg_type type;
  int64_t size;                 // size of loadobject in bytes
  int64_t max_size;             // Maximum size of loadobject in bytes
  int64_t min_size;             // Min size of loadobject in bytes.
  Vector<Function*> *functions; // Ordered list of functions
  Vector<Module*> *seg_modules; // list of modules
  HashMap<char*, Module*> *modules;
  Module *noname;               // Module pointer to unknown name
  Platform_t platform;          // Sparc, Sparcv9, Intel
  WSize_t wsize;                // word size: 32,64
  Stabs *objStabs;
  HashMap<char*, Function*> *comp_funcs;    // list of comparable functions
  Experiment *firstExp;
  char *runTimePath;
  time_t mtime;                 // file timestamp (in seconds)
  int64_t checksum;             // file checksum

private:
  Elf *elf_lo;
  bool elf_inited;
  DbeInstr **instHTable;        // hash table for DbeInstr
  char *pathname;               // User name of object file
  ino64_t inode;                // inode number of segment file
  bool isRelocatable;           // is relocatable .o
  char *arch_name;              // .archive name
  Emsgqueue *warnq;
  Emsgqueue *commentq;
  Function **funcHTable;        // hash table for functions
  Function *h_function;         // hide pseudo function
  DbeInstr *h_instr;            // hide pseudo instr
  HashMap<char*, Module*> *seg_modules_map; // to find a comparable module

  static int func_compare (const void *p1, const void *p2);
  int read_archive ();
  void init_datatypes ();
  void update_datatypes (Module*, Vaddr, uint32_t datatype_id);
};

#endif /* _LOADOBJECT_H */
