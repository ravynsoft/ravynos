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
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "demangle.h"
#include "util.h"
#include "DbeSession.h"
#include "Function.h"
#include "Module.h"
#include "LoadObject.h"
#include "Settings.h"
#include "DbeFile.h"
#include "DbeView.h"

struct SrcInfo
{
  DbeLine *src_line;
  SrcInfo *included_from;
  SrcInfo *next;
};

struct PCInfo
{
  int64_t offset;
  int64_t size;
  SrcInfo *src_info;
};

Function::Function (uint64_t _id)
{
  id = _id;
  instr_id = id << 32;
  derivedNode = NULL;
  module = NULL;
  line_first = line_last = -1;
  isOutlineFunction = false;
  name = NULL;
  mangled_name = NULL;
  match_name = NULL;
  comparable_name = NULL;
  img_fname = NULL;
  img_offset = 0;
  chksum = 0;
  flags = 0;
  size = 0;
  save_addr = FUNC_NO_SAVE;
  linetab = NULL;
  def_source = NULL;
  indexStabsLink = NULL;
  elfSym = NULL;
  sources = NULL;
  instrs = new Vector<DbeInstr*>;
  addrs = NULL;
  name_buf = NULL;
  current_name_format = Histable::NA;
  curr_srcinfo = NULL;
  curr_srcfile = NULL;
  srcinfo_list = NULL;
  defaultDbeLine = NULL;
  usrfunc = NULL;
  alias = NULL;
  instHTable = NULL;
  addrIndexHTable = NULL;
  isUsed = false;
  isHideFunc = false;
  inlinedSubr = NULL;
  inlinedSubrCnt = 0;
}

Function::~Function ()
{
  free (mangled_name);
  free (match_name);
  free (comparable_name);
  free (name_buf);
  Destroy (linetab);
  Destroy (instrs);

  while (srcinfo_list)
    {
      SrcInfo *t = srcinfo_list;
      srcinfo_list = t->next;
      delete t;
    }
  delete sources;
  delete addrs;
  delete[] instHTable;
  delete[] addrIndexHTable;
  if (indexStabsLink)
    // Remove a link to the current function
    indexStabsLink->indexStabsLink = NULL;
}

char *
Function::get_name (NameFormat nfmt)
{
  if (nfmt == Histable::NA)
    {
      DbeView *dbeView = dbeSession->getView (0);
      if (dbeView)
	nfmt = dbeView->get_name_format ();
    }
  if (name_buf && (nfmt == current_name_format || nfmt == Histable::NA))
    return name_buf;
  free (name_buf);
  current_name_format = nfmt;

  bool soname_fmt = Histable::soname_fmt (nfmt);
  int fname_fmt = Histable::fname_fmt (nfmt);
  if (fname_fmt == Histable::MANGLED)
    name_buf = strdup (mangled_name);
  else
    {
      if (module && module->is_fortran ()
	  && (streq (name, "MAIN") || streq (name, "MAIN_")))
	name_buf = strdup (match_name);
      else
	name_buf = strdup (name);

      if (fname_fmt == Histable::SHORT)
	{
	  int i = get_paren (name_buf);
	  if (i != -1)
	    name_buf[i] = (char) 0;
	}
    }
  if (soname_fmt)
    {
      char *fname = dbe_sprintf (NTXT ("%s [%s]"), name_buf, module->loadobject->get_name ());
      free (name_buf);
      name_buf = fname;
    }
  return name_buf;
}

uint64_t
Function::get_addr ()
{
  LoadObject *lo = module ? module->loadobject : NULL;
  int seg_idx = lo ? lo->seg_idx : -1;
  return MAKE_ADDRESS (seg_idx, img_offset);
}

Histable *
Function::convertto (Histable_type type, Histable *obj)
{
  Histable *res = NULL;
  SourceFile *source = (SourceFile*) obj;
  switch (type)
    {
    case INSTR:
      res = find_dbeinstr (0, 0);
      break;
    case LINE:
      {
	// mapPCtoLine will implicitly read line info if necessary
	res = mapPCtoLine (0, source);
	break;
      }
    case FUNCTION:
      res = this;
      break;
    case SOURCEFILE:
      res = def_source;
      break;
    default:
      assert (0);
    }
  return res;
}

void
Function::set_name (char *string)
{
  if (string == NULL)
    return;
  set_mangled_name (string);

  // strip away any globalization prefix, and save result for matching
  char *mname = string;
  if (strncmp (string, "$X", 2) == 0 || strncmp (string, ".X", 2) == 0)
    {
      // name was globalized
      char *n = strchr (string + 2, (int) '.');
      if (n != NULL)
	mname = n + 1;
    }
  set_match_name (mname);
  name = NULL;
  if (module)
    {
      if (name == NULL && *match_name == '_')
	{
	  int flag = DMGL_PARAMS;
	  if (module->lang_code == Sp_lang_java)
	    flag |= DMGL_JAVA;
	  name = cplus_demangle (match_name, flag);
	}
    }
  if (name == NULL)     // got demangled string
    name = dbe_strdup (match_name);
  set_comparable_name (name);
}

void
Function::set_mangled_name (const char *string)
{
  if (string)
    {
      free (mangled_name);
      mangled_name = dbe_strdup (string);
    }
}

void
Function::set_match_name (const char *string)
{
  if (string)
    {
      free (match_name);
      match_name = dbe_strdup (string);
    }
}

void
Function::set_comparable_name (const char *string)
{
  if (string)
    {
      free (comparable_name);
      comparable_name = dbe_strdup (string);

      // remove blanks from comparable_name
      for (char *s = comparable_name, *s1 = comparable_name;;)
	{
	  if (*s == 0)
	    {
	      *s1 = 0;
	      break;
	    }
	  else if (*s != ' ')
	    {
	      *s1 = *s;
	      s1++;
	    }
	  s++;
	}
    }
}

//  This function looks at the name of a function, and determines whether
//	or not it may be a derived function -- outline, mtask, or clone --
//	If it is, it writes the function name as demangled,
//	and sets a pointer to the function from which it was derived
void
Function::findDerivedFunctions ()

{
  MPFuncTypes ftype;
  int index;
  Function *fitem;
  unsigned long long line_no;
  char *namefmt;
  char *subname = mangled_name;
  char *demname;

  // see if we've already done this
  if ((flags & FUNC_FLAG_RESDER) != 0)
      return;

  // set flag for future
  flags = flags | FUNC_FLAG_RESDER;
  if (module == NULL)
    return;
  if (*subname != '_' || subname[1] != '$')   // Not a specially named function
    return;

  // look for the current versions of naming
  if (strncmp (subname + 2, NTXT ("d1"), 2) == 0)    // doall function
    ftype = MPF_DOALL;
  else if (strncmp (subname + 2, "p1", 2) == 0)     // parallel region function
    ftype = MPF_PAR;
  else if (strncmp (subname + 2, "l1", 2) == 0)     // single thread loop setup
    ftype = MPF_DOALL;
  else if (strncmp (subname + 2, "s1", 2) == 0)     // parallel section function
    ftype = MPF_SECT;
  else if (strncmp (subname + 2, "t1", 2) == 0)     // task function
    ftype = MPF_TASK;
  else if (strncmp (subname + 2, "o1", 2) == 0)     // outline function
    {
      ftype = MPF_OUTL;
      isOutlineFunction = true;
   }
  else if (strncmp (subname + 2, "c1", 2) == 0)     // clone function
    ftype = MPF_CLONE;
  else    // Not an encoded name, just return
    return;

  // we know it's one of the above prefixes
  char *sub = dbe_strdup (name + 4); // starting with base-26 number
  char *p = sub;

  // skip the base-26 number, and extract the line number
  while (isalpha ((int) (*p)) != 0 && *p != 0)
    p++;
  line_no = atoll (p);

  // skip past the number to to the .
  while (*p != '.' && *p != 0)
    p++;
  if (*p == 0)
    {
      // can't be right
      free (sub);
      return;
    }
  // skip the trailing .
  p++;
  subname = p;
  bool foundmatch = false;

  // Find the function from which it is derived -- the one that matched subname
  Vec_loop (Function*, module->functions, index, fitem)
  {
    if (streq (subname, fitem->mangled_name))
      { // found it
	foundmatch = true;
	usrfunc = fitem;

	// set the derived node
	if ((fitem->flags & FUNC_FLAG_RESDER) == 0)
	  // ensure that it, too, is resolved if derived
	  fitem->findDerivedFunctions ();

	// Build a demangled name
	switch (ftype)
	  {
	  case MPF_OUTL:
	    isOutlineFunction = true;
	    namefmt = GTXT ("%s -- outline code from line %lld [%s]");
	    derivedNode = fitem->find_dbeinstr (PCLineFlag, line_no);
	    break;
	  case MPF_PAR:
	    namefmt = GTXT ("%s -- OMP parallel region from line %lld [%s]");
	    break;
	  case MPF_DOALL:
	    namefmt = GTXT ("%s -- Parallel loop from line %lld [%s]");
	    break;
	  case MPF_SECT:
	    namefmt = GTXT ("%s -- OMP sections from line %lld [%s]");
	    break;
	  case MPF_CLONE:
	    // Note that clones are handled differently -- no line number and
	    //	clones are NOT shown as called from the original
	    //	so after constructing the name, just return
	    //	later, establish link from clone to parent
	    demname = dbe_sprintf (GTXT ("%s -- cloned version [%s]"),
				   fitem->get_name (), name);
	    free (name);
	    // set the name to the demangled version
	    name = demname;
	    free (sub);
	    derivedNode = fitem->find_dbeinstr (PCLineFlag, line_no);
	    return;
	  case MPF_TASK:
	    namefmt = GTXT ("%s -- OMP task from line %lld [%s]");
	    break;
	  default:
	    free (sub);
	    return;

	  }

	// Finally, construct the demangled name
	demname = dbe_sprintf (namefmt, fitem->get_name (), line_no, name);
	free (name);
	name = demname;
	setLineFirst ((int) line_no);
	break;
      }
  }

  if (foundmatch == false && ftype == MPF_OUTL)
    {
      // Even if derived node was not found, we can demangle
      demname = dbe_sprintf (GTXT ("%s -- outline code [%s]"), subname,
			     mangled_name);
      free (name);
      name = demname;
    }
  free (sub);
}

SrcInfo *
Function::new_srcInfo ()
{
  SrcInfo *t = new SrcInfo ();
  t->src_line = NULL;
  t->included_from = NULL;
  t->next = srcinfo_list;
  srcinfo_list = t;
  return t;
}

void
Function::pushSrcFile (SourceFile* source, int /*lineno*/)
{
  // create new file stack
  if (curr_srcfile == NULL)
    {
      curr_srcfile = source;
      return;
    }

  SrcInfo *src_info = new_srcInfo ();
  // In the ideal world, we need a DbeLine(III) here,
  // but right now it would make us later believe that there are
  // instructions generated for #include lines. To avoid that,
  // we ask for a DbeLine(II).
  src_info->src_line = curr_srcfile->find_dbeline (this, 0 /*lineno*/);
  if (src_info->src_line)
    {
      src_info->included_from = curr_srcinfo;
      curr_srcinfo = src_info;
    }
  curr_srcfile = source;
  setSource ();
}

SourceFile *
Function::popSrcFile ()
{
  if (curr_srcinfo != NULL)
    {
      curr_srcfile = curr_srcinfo->src_line->sourceFile;
      curr_srcinfo = curr_srcinfo->included_from;
    }
  else
    curr_srcfile = NULL;
  return curr_srcfile;
}

void
Function::copy_PCInfo (Function *from)
{
  if (line_first <= 0)
    line_first = from->line_first;
  if (line_last <= 0)
    line_last = from->line_last;
  if (def_source == NULL)
    def_source = from->def_source;
  for (int i = 0, sz = from->linetab ? from->linetab->size () : 0; i < sz; i++)
    {
      PCInfo *pcinf = from->linetab->fetch (i);
      DbeLine *dbeLine = pcinf->src_info->src_line;
      add_PC_info (pcinf->offset, dbeLine->lineno, dbeLine->sourceFile);
    }
}

void
Function::add_PC_info (uint64_t offset, int lineno, SourceFile *cur_src)
{
  if (lineno <= 0 || size < 0 || offset >= (uint64_t) size)
    return;
  if (cur_src == NULL)
    cur_src = curr_srcfile ? curr_srcfile : def_source;
  if (linetab == NULL)
    linetab = new Vector<PCInfo*>;

  int left = 0;
  int right = linetab->size () - 1;
  DbeLine *dbeline;
  while (left <= right)
    {
      int x = (left + right) / 2;
      PCInfo *pcinf = linetab->fetch (x);
      uint64_t pcinf_offset = ((uint64_t) pcinf->offset);
      if (offset == pcinf_offset)
	{
	  dbeline = cur_src->find_dbeline (this, lineno);
	  dbeline->init_Offset (offset);
	  pcinf->src_info->src_line = dbeline;
	  // Ignore duplicate offset
	  return;
	}
      else if (offset > pcinf_offset)
	left = x + 1;
      else
	right = x - 1;
    }
  PCInfo *pcinfo = new PCInfo;
  pcinfo->offset = offset;

  // Form new SrcInfo
  SrcInfo *srcInfo = new_srcInfo ();
  dbeline = cur_src->find_dbeline (this, lineno);
  dbeline->init_Offset (offset);
  srcInfo->src_line = dbeline;
  // For now don't build included_from list.
  // We need better compiler support for that.
  //srcInfo->included_from = curr_srcinfo;
  srcInfo->included_from = NULL;
  pcinfo->src_info = srcInfo;

  // Update the size of the current line in both structures:
  // current PCInfo and corresponding DbeLine.
  if (left < linetab->size ())
    pcinfo->size = linetab->fetch (left)->offset - offset;
  else
    pcinfo->size = size - offset;
  pcinfo->src_info->src_line->size += pcinfo->size;

  // If not the first line, update the size of the previous line
  if (left > 0)
    {
      PCInfo *pcinfo_prev = linetab->fetch (left - 1);
      int64_t delta = (offset - pcinfo_prev->offset) - pcinfo_prev->size;
      pcinfo_prev->size += delta;
      pcinfo_prev->src_info->src_line->size += delta;
    }

  linetab->insert (left, pcinfo);
  if (cur_src == def_source)
    {
      if (line_first <= 0)
	setLineFirst (lineno);
      if (line_last <= 0 || lineno > line_last)
	line_last = lineno;
    }
}

PCInfo *
Function::lookup_PCInfo (uint64_t offset)
{
  module->read_stabs ();
  if (linetab == NULL)
    linetab = new Vector<PCInfo*>;

  int left = 0;
  int right = linetab->size () - 1;
  while (left <= right)
    {
      int x = (left + right) / 2;
      PCInfo *pcinfo = linetab->fetch (x);
      if (offset >= ((uint64_t) pcinfo->offset))
	{
	  if (offset < (uint64_t) (pcinfo->offset + pcinfo->size))
	    return pcinfo;
	  left = x + 1;
	}
      else
	right = x - 1;
    }
  return NULL;
}

DbeInstr*
Function::mapLineToPc (DbeLine *dbeLine)
{
  if (dbeLine && linetab)
    {
      DbeLine *dbl = dbeLine->dbeline_base;
      for (int i = 0, sz = linetab->size (); i < sz; i++)
	{
	  PCInfo *pcinfo = linetab->get (i);
	  if (pcinfo->src_info
	      && (pcinfo->src_info->src_line->dbeline_base == dbl))
	    {
	      DbeInstr *dbeInstr = find_dbeinstr (PCLineFlag, pcinfo->offset);
	      if (dbeInstr)
		{
		  dbeInstr->lineno = dbeLine->lineno;
		  return dbeInstr;
		}
	    }
	}
    }
  return NULL;
}

DbeLine*
Function::mapPCtoLine (uint64_t addr, SourceFile *src)
{
  PCInfo *pcinfo = lookup_PCInfo (addr);
  if (pcinfo == NULL)
    {
      if (defaultDbeLine == NULL)
	defaultDbeLine = getDefSrc ()->find_dbeline (this, 0);
      return defaultDbeLine;
    }
  DbeLine *dbeline = pcinfo->src_info->src_line;

  // If source-context is not specified return the line
  // from which this pc has been generated.
  if (src == NULL)
    return dbeline;
  if (dbeline->sourceFile == src)
    return dbeline->dbeline_base;
  return src->find_dbeline (this, 0);
}

DbeInstr *
Function::find_dbeinstr (int flag, uint64_t addr)
{
  DbeInstr *instr;

  enum
  {
    FuncInstHTableSize = 128
  };

  int hash = (((int) addr) >> 2) & (FuncInstHTableSize - 1);
  if (instHTable == NULL)
    {
      if (size > 2048)
	{
	  instHTable = new DbeInstr*[FuncInstHTableSize];
	  for (int i = 0; i < FuncInstHTableSize; i++)
	    instHTable[i] = NULL;
	}
    }
  else
    {
      instr = instHTable[hash];
      if (instr && instr->addr == addr && instr->flags == flag)
	return instr;
    }

  int left = 0;
  int right = instrs->size () - 1;
  while (left <= right)
    {
      int index = (left + right) / 2;
      instr = instrs->fetch (index);
      if (addr < instr->addr)
	right = index - 1;
      else if (addr > instr->addr)
	left = index + 1;
      else
	{
	  if (flag == instr->flags)
	    {
	      if (instHTable)
		instHTable[hash] = instr;
	      return instr;
	    }
	  else if (flag < instr->flags)
	    right = index - 1;
	  else
	    left = index + 1;
	}
    }

  // None found, create a new one
  instr = new DbeInstr (instr_id++, flag, this, addr);
  instrs->insert (left, instr);
  if (instHTable)
    instHTable[hash] = instr;
  return instr;
}

// LIBRARY_VISIBILITY
DbeInstr *
Function::create_hide_instr (DbeInstr *instr)
{
  DbeInstr *new_instr = new DbeInstr (instr_id++, 0, this, instr->addr);
  return new_instr;
}

uint64_t
Function::find_previous_addr (uint64_t addr)
{
  if (addrs == NULL)
    {
      addrs = module->getAddrs (this);
      if (addrs == NULL)
	return addr;
    }

  int index = -1, not_found = 1;

  enum
  {
    FuncAddrIndexHTableSize = 128
  };
  int hash = (((int) addr) >> 2) & (FuncAddrIndexHTableSize - 1);
  if (addrIndexHTable == NULL)
    {
      if (size > 2048)
	{
	  addrIndexHTable = new int[FuncAddrIndexHTableSize];
	  for (int i = 0; i < FuncAddrIndexHTableSize; i++)
	    addrIndexHTable[i] = -1;
	}
    }
  else
    {
      index = addrIndexHTable[hash];
      if (index >= 0 && addrs->fetch (index) == addr)
	not_found = 0;
    }

  int left = 0;
  int right = addrs->size () - 1;
  while (not_found && left <= right)
    {
      index = (left + right) / 2;
      uint64_t addr_test = addrs->fetch (index);
      if (addr < addr_test)
	right = index - 1;
      else if (addr > addr_test)
	left = index + 1;
      else
	{
	  if (addrIndexHTable)
	    addrIndexHTable[hash] = index;
	  not_found = 0;
	}
    }
  if (not_found)
    return addr;
  if (index > 0)
    index--;
  return addrs->fetch (index);
}

void
Function::setSource ()
{
  SourceFile *sf = module->getIncludeFile ();
  if (sf == NULL)
    sf = getDefSrc ();
  if (def_source == NULL)
    setDefSrc (sf);
  if (sf == def_source)
    return;
  if (sources == NULL)
    {
      sources = new Vector<SourceFile*>;
      sources->append (def_source);
      sources->append (sf);
    }
  else if (sources->find (sf) < 0)
    sources->append (sf);
}

void
Function::setDefSrc (SourceFile *sf)
{
  if (sf)
    {
      def_source = sf;
      if (line_first > 0)
	add_PC_info (0, line_first, def_source);
    }
}

void
Function::setLineFirst (int lineno)
{
  if (lineno > 0)
    {
      line_first = lineno;
      if (line_last <= 0)
	line_last = lineno;
      if (def_source)
	add_PC_info (0, line_first, def_source);
    }
}

Vector<SourceFile*> *
Function::get_sources ()
{
  if (module)
    module->read_stabs ();
  if (sources == NULL)
    {
      sources = new Vector<SourceFile*>;
      sources->append (getDefSrc ());
    }
  return sources;
}

SourceFile*
Function::getDefSrc ()
{
  if (module)
    module->read_stabs ();
  if (def_source == NULL)
    setDefSrc (module->getMainSrc ());
  return def_source;
}

char *
Function::getDefSrcName ()
{
  SourceFile *sf = getDefSrc ();
  if (sf)
    return sf->dbeFile->getResolvedPath ();
  if (module)
    return module->file_name;
  sf = dbeSession->get_Unknown_Source ();
  return sf->get_name ();
}

#define cmpValue(a, b) ((a) > (b) ? 1 : (a) == (b) ? 0 : -1)

int
Function::func_cmp (Function *func, SourceFile *srcContext)
{
  if (def_source != func->def_source)
    {
      if (srcContext == NULL)
	srcContext = getDefSrc ();
      if (def_source == srcContext)
	return -1;
      if (func->def_source == srcContext)
	return 1;
      return cmpValue (img_offset, func->img_offset);
    }

  if (line_first == func->line_first)
    return cmpValue (img_offset, func->img_offset);
  if (line_first <= 0)
    {
      if (func->line_first > 0)
	return 1;
      return cmpValue (img_offset, func->img_offset);
    }
  if (func->line_first <= 0)
    return -1;
  return cmpValue (line_first, func->line_first);
}

Vector<Histable*> *
Function::get_comparable_objs ()
{
  update_comparable_objs ();
  if (comparable_objs || dbeSession->expGroups->size () <= 1 || module == NULL)
    return comparable_objs;
  if (module == NULL || module->loadobject == NULL)
    return NULL;
  Vector<Histable*> *comparableModules = module->get_comparable_objs ();
  if (comparableModules == NULL)
    {
      return NULL;
    }
  comparable_objs = new Vector<Histable*>(comparableModules->size ());
  for (long i = 0, sz = comparableModules->size (); i < sz; i++)
    {
      Function *func = NULL;
      comparable_objs->store (i, func);
      Module *mod = (Module*) comparableModules->fetch (i);
      if (mod == NULL)
	continue;
      if (mod == module)
	func = this;
      else
	{
	  for (long i1 = 0, sz1 = VecSize (mod->functions); i1 < sz1; i1++)
	    {
	      Function *f = mod->functions->get (i1);
	      if ((f->comparable_objs == NULL)
		   && (strcmp (f->comparable_name, comparable_name) == 0))
		{
		  func = f;
		  func->comparable_objs = comparable_objs;
		  break;
		}
	    }
	}
      comparable_objs->store (i, func);
    }
  Vector<Histable*> *comparableLoadObjs =
	  module->loadobject->get_comparable_objs ();
  if (VecSize (comparableLoadObjs) == VecSize (comparable_objs))
    {
      for (long i = 0, sz = VecSize (comparableLoadObjs); i < sz; i++)
	{
	  LoadObject *lo = (LoadObject *) comparableLoadObjs->get (i);
	  Function *func = (Function *) comparable_objs->get (i);
	  if (func || (lo == NULL))
	    continue;
	  if (module->loadobject == lo)
	    func = this;
	  else
	    {
	      for (long i1 = 0, sz1 = VecSize (lo->functions); i1 < sz1; i1++)
		{
		  Function *f = lo->functions->fetch (i1);
		  if ((f->comparable_objs == NULL)
		       && (strcmp (f->comparable_name, comparable_name) == 0))
		    {
		      func = f;
		      func->comparable_objs = comparable_objs;
		      break;
		    }
		}
	    }
	  comparable_objs->store (i, func);
	}
    }
  dump_comparable_objs ();
  return comparable_objs;
}

JMethod::JMethod (uint64_t _id) : Function (_id)
{
  mid = 0LL;
  addr = (Vaddr) 0;
  signature = NULL;
  jni_function = NULL;
}

JMethod::~JMethod ()
{
  free (signature);
}

uint64_t
JMethod::get_addr ()
{
  if (addr != (Vaddr) 0)
    return addr;
  else
    return Function::get_addr ();
}

typedef struct
{
  size_t used_in;
  size_t used_out;
} MethodField;

static void
write_buf (char* buf, char* str)
{
  while ((*buf++ = *str++));
}

/** Translate one field from the nane buffer.
 * return how many chars were read from name and how many bytes were used in buf.
 */
static MethodField
translate_method_field (const char* name, char* buf)
{
  MethodField out, t;
  switch (*name)
    {
    case 'L':
      name++;
      out.used_in = 1;
      out.used_out = 0;
      while (*name != ';')
	{
	  *buf = *name++;
	  if (*buf == '/')
	    *buf = '.';
	  buf++;
	  out.used_in++;
	  out.used_out++;
	}
      out.used_in++; /* the ';' is also used. */
      break;
    case 'Z':
      write_buf (buf, NTXT ("boolean"));
      out.used_out = 7;
      out.used_in = 1;
      break;
    case 'B':
      write_buf (buf, NTXT ("byte"));
      out.used_out = 4;
      out.used_in = 1;
      break;
    case 'C':
      write_buf (buf, NTXT ("char"));
      out.used_out = 4;
      out.used_in = 1;
      break;
    case 'S':
      write_buf (buf, NTXT ("short"));
      out.used_out = 5;
      out.used_in = 1;
      break;
    case 'I':
      write_buf (buf, NTXT ("int"));
      out.used_out = 3;
      out.used_in = 1;
      break;
    case 'J':
      write_buf (buf, NTXT ("long"));
      out.used_out = 4;
      out.used_in = 1;
      break;
    case 'F':
      write_buf (buf, NTXT ("float"));
      out.used_out = 5;
      out.used_in = 1;
      break;
    case 'D':
      write_buf (buf, NTXT ("double"));
      out.used_out = 6;
      out.used_in = 1;
      break;
    case 'V':
      write_buf (buf, NTXT ("void"));
      out.used_out = 4;
      out.used_in = 1;
      break;
    case '[':
      t = translate_method_field (name + 1, buf);
      write_buf (buf + t.used_out, NTXT ("[]"));
      out.used_out = t.used_out + 2;
      out.used_in = t.used_in + 1;
      break;
    default:
      out.used_out = 0;
      out.used_in = 0;
    }
  return out;
}

/**
 * translate method name to full method signature
 * into the output buffer (buf).
 * ret_type - true for printing result type
 */
static bool
translate_method (char* mname, char *signature, bool ret_type, char* buf)
{
  MethodField p;
  size_t l;
  int first = 1;
  if (signature == NULL)
    return false;

  const char *c = strchr (signature, ')');
  if (c == NULL)
    return false;
  if (ret_type)
    {
      p = translate_method_field (++c, buf);
      buf += p.used_out;
      *buf++ = ' ';
    }

  l = strlen (mname);
  memcpy (buf, mname, l + 1);
  buf += l;
  // *buf++ = ' '; // space before ()
  *buf++ = '(';

  c = signature + 1;
  while (*c != ')')
    {
      if (!first)
	{
	  *buf++ = ',';
	  *buf++ = ' ';
	}
      first = 0;
      p = translate_method_field (c, buf);
      c += p.used_in;
      buf += p.used_out;
    }

  *buf++ = ')';
  *buf = '\0';
  return true;
}

void
JMethod::set_name (char *string)
{
  if (string == NULL)
    return;
  set_mangled_name (string);

  char buf[MAXDBUF];
  *buf = '\0';
  if (translate_method (string, signature, false, buf))
    {
      name = dbe_strdup (buf); // got translated string
      Dprintf (DUMP_JCLASS_READER,
	      "JMethod::set_name: true name=%s string=%s signature=%s\n",
	       STR (name), STR (string), STR (signature));
    }
  else
    {
      name = dbe_strdup (string);
      Dprintf (DUMP_JCLASS_READER,
	       "JMethod::set_name: false name=%s signature=%s\n",
	       STR (name), STR (signature));
    }
  set_match_name (name);
  set_comparable_name (name);
}

bool
JMethod::jni_match (Function *func)
{
  if (func == NULL || (func->flags & FUNC_NOT_JNI) != 0)
    return false;
  if (jni_function == func)
    return true;

  char *fname = func->get_name ();
  if ((func->flags & FUNC_JNI_CHECKED) == 0)
    {
      func->flags |= FUNC_JNI_CHECKED;
      if (strncmp (func->get_name (), NTXT ("Java_"), 5) != 0)
	{
	  func->flags |= FUNC_NOT_JNI;
	  return false;
	}
    }

  char *d = name;
  char *s = fname + 5;
  while (*d && *d != '(' && *d != ' ')
    {
      if (*d == '.')
	{
	  if (*s++ != '_')
	    return false;
	  d++;
	}
      else if (*d == '_')
	{
	  if (*s++ != '_')
	    return false;
	  if (*s++ != '1')
	    return false;
	  d++;
	}
      else if (*d++ != *s++)
	return false;
    }
  jni_function = func;
  return true;
}
