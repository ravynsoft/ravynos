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

#ifndef _HISTABLE_H
#define _HISTABLE_H

//
// The Histable class hierarchy is used to build up a representation of
// the codeobjects (functions, modules, loadObjects, etc.) that make up the
// text address space of a program.  The hierarchy is as follows:
//
//	Histable (public)
//		LoadObject (public)
//		Module (public)
//		Function (public)
//
// Dataobjects are objects from the data address space of a program.
// The reason for calling the base class "Histable" is because these
// objects are all valid objects for computing histograms on.

// A Histable object represents an object in the program text or data.

#include "dbe_structs.h"
#include "Emsg.h"
#include "Expression.h"

class DataObject;
class Function;
class SourceFile;
class DbeFile;
class DbeLine;
template <class ITEM> class Vector;

class Histable
{
  friend class Hist_data;
public:

  enum Type
  {
    INSTR, LINE, FUNCTION, MODULE, LOADOBJECT,
    EADDR, MEMOBJ, INDEXOBJ, PAGE, DOBJECT,
    SOURCEFILE, IOACTFILE, IOACTVFD, IOCALLSTACK,
    HEAPCALLSTACK, EXPERIMENT, OTHER
  };

  // NameFormat for functions and function based objects

  enum NameFormat
  {
    NA, LONG, SHORT, MANGLED, SONAME = 0x10
  };

  static NameFormat
  make_fmt (int fnfmt, bool sofmt = false)
  {
    return (NameFormat) (sofmt ? fnfmt | SONAME : fnfmt);
  }

  static int
  fname_fmt (NameFormat fmt)
  {
    return (fmt & ~SONAME);
  }

  static bool
  soname_fmt (NameFormat fmt)
  {
    return (fmt & SONAME);
  }

  Histable ();
  char *dump ();

  virtual ~Histable ();

  virtual char *
  get_name (NameFormat = NA)
  {
    return name;    // Return the name of the object
  }

  virtual void
  set_name (char * _name)
  {
    name = _name;
  }

  virtual void set_name_from_context (Expression::Context *) { }
  virtual Type get_type () = 0;

  virtual int64_t
  get_size ()
  {
    return 0;
  }

  virtual uint64_t
  get_addr ()
  {
    return 0ULL;
  }

  virtual Vector<Histable*> *get_comparable_objs ();
  Histable *get_compare_obj ();

  virtual Histable *
  convertto (Type, Histable* = NULL)
  {
    return this;
  }

  Vector<Histable*> *comparable_objs;
  int64_t id;       // A unique id of this object, within its specific Type

protected:
  char *name;       // Object name
  int phaseCompareIdx;
  void update_comparable_objs ();
  void dump_comparable_objs ();
  char *type_to_string ();
  void delete_comparable_objs ();
};

typedef Histable::Type Histable_type;

// An Other object represents some random histable object
class Other : public Histable
{
public:

  virtual Type
  get_type ()
  {
    return OTHER;
  }

  uint64_t value64;
  uint32_t tag;
};

// DbeInstr represents an instruction.
//
//   Used by Analyzer for: Disassembly, PCs, Timeline, and Event tabs.
//
class DbeInstr : public Histable
{
public:
  DbeInstr (uint64_t _id, int _flags, Function *_func, uint64_t _addr);

  virtual Type
  get_type ()
  {
    return INSTR;
  }

  virtual char *get_name (NameFormat = NA);
  virtual int64_t get_size ();
  virtual uint64_t get_addr ();
  virtual Histable *convertto (Type type, Histable *obj = NULL);
  DbeLine *mapPCtoLine (SourceFile *sf);
  void add_inlined_info (StringBuilder *sb);
  char *get_descriptor ();
  int pc_cmp (DbeInstr *instr2);

  uint64_t addr;
  uint64_t img_offset;      // file offset of the image
  int flags;
  Function *func;
  int lineno;
  int inlinedInd;
  int64_t size;
  bool isUsed;

private:
  NameFormat current_name_format;
};

class DbeEA : public Histable
{
public:

  DbeEA (DataObject *_dobj, Vaddr _eaddr)
  {
    dobj = _dobj;
    eaddr = _eaddr;
  };

  virtual Type
  get_type ()
  {
    return EADDR;
  };

  virtual int64_t
  get_size ()
  {
    return 1;
  };

  virtual uint64_t
  get_addr ()
  {
    return eaddr;
  };

  virtual char *get_name (NameFormat = NA);
  virtual Histable *convertto (Type type, Histable *obj = NULL);

  DataObject *dobj;
  Vaddr eaddr;
};

// DbeLine represents a line in a source file.
//
//   For each top-level DbeLine instance, there are three DbeLine subtypes:
//
//   A The top-level DbeLine is associated with a sourceFile & lineno, but
//     not any particular function.  This form of DbeLine is used
//     to represent Analyzer Source tab lines.
//
//   B Function-specific lines, those associated with a function in addition
//     to the the sourceFile & lineno, are stored in a linked list.
//     (see "dbeline_func_next").
//     This subtype is used to differentiate a line found in #included source
//     that is referenced by multiple functions.
//     It is used in the Analyzer Lines tab.
//
//   C Function-specific "lines" that don't have line number info are referenced
//     from each linked-list element's "dbeline_func_pseudo" field.
//     This subtype is needed when a binary doesn't identify line numbers.
//     It is used in the Analyzer Lines tab.
//
//   When the user switches views between tabs, a selected object in the old
//   tab must be translated to an approprate object in the new tab.
//   When switching to the Source Tab, the top-level DbeLine (dbeline_base)
//     should be used.
//   When switching to the Lines Tab, a function-specific dbeline_func_*
//     should be used.
//

class DbeLine : public Histable
{
public:

  enum Flag
  {
    OMPPRAGMA = 1
  };

  DbeLine (Function *_func, SourceFile *sf, int _lineno);
  virtual ~DbeLine ();
  virtual char *get_name (NameFormat = NA);
  virtual int64_t get_size ();
  virtual uint64_t get_addr ();
  virtual Histable *convertto (Type type, Histable *obj = NULL);

  void init_Offset (uint64_t p_offset);
  int line_cmp (DbeLine *dbl);

  virtual Type
  get_type ()
  {
    return LINE;
  }

  void
  set_flag (Flag flag)
  {
    flags |= flag;
  }

  bool
  is_set (Flag flag)
  {
    return (flags & flag) != 0;
  }

  Function *func;           // note: will be NULL in the base (head) dbeline
  int lineno;
  int64_t size;
  SourceFile *sourceFile;   // Default source file
  SourceFile *include;      // included source file or NULL

  DbeLine *dbeline_base;
  // Head of list, a dbeline associated with sourceFile & lineno, but not func:
  //   dbeline_base->lineno:                 non-zero
  //   dbeline_base->sourceFile:             non-null
  //   dbeline_base->func:                   NULL
  //   dbeline_base->dbeline_base:           this
  //   dbeline_base->dbeline_func_next:      first func-specific dbeline

  DbeLine *dbeline_func_next;
  // If non-null, pointer to a function-specific dbeline where:
  //   dbeline_func_next->lineno:            same as dbeline_base->lineno
  //   dbeline_func_next->sourceFile:        same as dbeline_base->sourceFile
  //   dbeline_func_next->func:              pointer to unique function
  //   dbeline_func_next->dbeline_base:      head of the linked list.
  //   dbeline_func_next->dbeline_func_next: next function-specific dbeline.

private:
  int current_name_format;
  int64_t offset;
  int flags;
};

class HistableFile : public Histable, public DbeMessages
{
public:
  HistableFile ();

  bool isUsed;
  DbeFile *dbeFile;
};

#endif /* _HISTABLE_H */
