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

#ifndef _DBE_FUNCTION_H
#define _DBE_FUNCTION_H

// A Function object represents an individual function in a .o file.

#include "util.h"
#include "Histable.h"
#include "SourceFile.h"

class Module;
class Symbol;
class InlinedSubr;
struct SrcInfo;
struct PCInfo;
template <class ITEM> class Vector;

const uint64_t FUNC_NO_SAVE = (uint64_t) - 1;
const uint64_t FUNC_ROOT = (uint64_t) - 2;

enum
{
  FUNC_FLAG_PLT = 1,
  FUNC_FLAG_DYNAMIC = 2,
  FUNC_FLAG_RESDER = 4, // set if derived function resolution has been done
  FUNC_FLAG_NO_OFFSET = 8, // set if disassembly should not show offset from function
  FUNC_FLAG_SIMULATED = 16, // not a real function like <Total>, <Unknown>, etc.
  FUNC_FLAG_NATIVE = 32, // no byte code for JMethod
  FUNC_NOT_JNI = 64, // a function name is not "Java_..."
  FUNC_JNI_CHECKED = 128 // already checked for "Java_..."
};

const int MAXDBUF = 32768; // the longest demangled name handled

class Function : public Histable
{
public:

  enum MPFuncTypes
  {
    MPF_DOALL,
    MPF_PAR,
    MPF_SECT,
    MPF_TASK,
    MPF_CLONE,
    MPF_OUTL
  };

  Function (uint64_t _id);
  virtual ~Function ();

  virtual uint64_t get_addr ();
  virtual char *get_name (NameFormat = NA);
  virtual Vector<Histable*> *get_comparable_objs ();
  virtual void set_name (char *);   // Set the demangled name
  virtual Histable *convertto (Histable_type type, Histable *obj = NULL);

  virtual Histable_type
  get_type ()
  {
    return FUNCTION;
  };

  virtual int64_t
  get_size ()
  {
    return size;
  };

  void set_comparable_name (const char *string);
  void set_mangled_name (const char *string);
  void set_match_name (const char *string);

  // Find any derived functions, and set their derivedNode
  void findDerivedFunctions ();
  void findKrakatoaDerivedFunctions ();
  void add_PC_info (uint64_t offset, int lineno, SourceFile *cur_src = NULL);
  void pushSrcFile (SourceFile* source, int lineno);
  SourceFile *popSrcFile ();
  int func_cmp (Function *func, SourceFile *srcContext = NULL);
  void copy_PCInfo (Function *f);
  DbeLine *mapPCtoLine (uint64_t addr, SourceFile *src = NULL);
  DbeInstr *mapLineToPc (DbeLine *dbeLine);
  DbeInstr *find_dbeinstr (int flag, uint64_t addr);
  DbeInstr *create_hide_instr (DbeInstr *instr);
  uint64_t find_previous_addr (uint64_t addr);
  SourceFile *getDefSrc ();
  char *getDefSrcName ();
  void setDefSrc (SourceFile *sf);
  void setLineFirst (int lineno);
  Vector<SourceFile*> *get_sources ();

  char *
  get_mangled_name ()
  {
    return mangled_name;
  }

  char *
  get_match_name ()
  {
    return match_name;
  }

  inline Function *
  cardinal ()
  {
    return alias ? alias : this;
  }

  unsigned int flags;       // FUNC_FLAG_*
  Module *module;           // pointer to module containing source
  int line_first;           // range of line numbers for function
  int line_last;
  int64_t size;             // size of the function in bytes
  uint64_t save_addr;       // used for detection of leaf routines
  DbeInstr *derivedNode;    // If derived from another function
  bool isOutlineFunction;   // if outline (save assumed)
  unsigned int chksum;      // check sum of instructions
  char *img_fname;          // file containing function image
  uint64_t img_offset;      // file offset of the image
  SourceFile *curr_srcfile;
  DbeLine *defaultDbeLine;
  Function *usrfunc;        // User function
  Function *alias;
  bool isUsed;
  bool isHideFunc;
  SourceFile *def_source;
  Function *indexStabsLink; // correspondent function for the .o file
  Symbol *elfSym;
  InlinedSubr *inlinedSubr;
  int inlinedSubrCnt;

private:
  DbeInstr **instHTable;    // hash table for DbeInstr
  int *addrIndexHTable;     // hash table for addrs index
  void setSource ();
  PCInfo *lookup_PCInfo (uint64_t offset);
  SrcInfo *new_srcInfo ();

  char *mangled_name;
  char *match_name;      // mangled name, with globalization stripped
  char *comparable_name; // demangled name, with globalization and blanks stripped
  char *name_buf;
  NameFormat current_name_format;
  Vector<PCInfo*> *linetab;
  Vector<DbeInstr*> *instrs;
  Vector<uint64_t> *addrs;
  uint64_t instr_id;
  Vector<SourceFile*> *sources;
  SrcInfo *curr_srcinfo;    // the current source stack of the function
  SrcInfo *srcinfo_list;    // master list for SrcInfo
};

class JMethod : public Function
{
public:
  JMethod (uint64_t _id);
  virtual ~JMethod ();
  virtual void set_name (char *);
  virtual uint64_t get_addr ();

  void
  set_addr (Vaddr _addr)
  {
    addr = _addr;
  }

  uint64_t
  get_mid ()
  {
    return mid;
  }

  void
  set_mid (uint64_t _mid)
  {
    mid = _mid;
  }

  char *
  get_signature ()
  {
    return signature;
  }

  void
  set_signature (const char *s)
  {
    signature = dbe_strdup (s);
  }

  // Returns true if func's name matches method's as its JNI implementation
  bool jni_match (Function *func);

private:
  uint64_t mid;
  Vaddr addr;
  char *signature;
  Function *jni_function;
};

#endif /* _DBE_FUNCTION_H */
