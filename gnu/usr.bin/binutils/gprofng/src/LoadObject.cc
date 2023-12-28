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
#include <errno.h>

#include "util.h"
#include "StringBuilder.h"
#include "Application.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "Exp_Layout.h"
#include "DataObject.h"
#include "Elf.h"
#include "Function.h"
#include "Module.h"
#include "ClassFile.h"
#include "Stabs.h"
#include "LoadObject.h"
#include "dbe_types.h"
#include "DbeFile.h"
#include "ExpGroup.h"

enum
{
  LO_InstHTableSize     = 4096,
  HTableSize            = 1024
};

LoadObject *
LoadObject::create_item (const char *nm, int64_t chksum)
{
  LoadObject *lo = new LoadObject (nm);
  lo->checksum = chksum;
  dbeSession->append (lo);
  return lo;
}

LoadObject *
LoadObject::create_item (const char *nm, const char *_runTimePath, DbeFile *df)
{
  LoadObject *lo = new LoadObject (nm);
  lo->runTimePath = dbe_strdup (_runTimePath);
  lo->dbeFile->orig_location = dbe_strdup (_runTimePath);
  if (df)
    {
      if ((df->filetype & DbeFile::F_JAR_FILE) != 0)
	{
	  if (lo->dbeFile->find_in_jar_file (nm, df->get_jar_file ()))
	    {
	      lo->dbeFile->inArchive = df->inArchive;
	      lo->dbeFile->container = df;
	    }
	}
      else
	{
	  lo->dbeFile->set_location (df->get_location ());
	  lo->dbeFile->sbuf = df->sbuf;
	  lo->dbeFile->inArchive = df->inArchive;
	}
    }
  dbeSession->append (lo);
  return lo;
}

LoadObject::LoadObject (const char *loname)
{
  flags = 0;
  size = 0;
  type = SEG_UNKNOWN;
  isReadStabs = false;
  need_swap_endian = false;
  instHTable = new DbeInstr*[LO_InstHTableSize];
  for (int i = 0; i < LO_InstHTableSize; i++)
    instHTable[i] = NULL;

  functions = new Vector<Function*>;
  funcHTable = new Function*[HTableSize];
  for (int i = 0; i < HTableSize; i++)
    funcHTable[i] = NULL;

  seg_modules = new Vector<Module*>;
  modules = new HashMap<char*, Module*>;
  platform = Unknown;
  noname = dbeSession->createUnknownModule (this);
  modules->put (noname->get_name (), noname);
  pathname = NULL;
  arch_name = NULL;
  runTimePath = NULL;
  objStabs = NULL;
  firstExp = NULL;
  seg_modules_map = NULL;
  comp_funcs = NULL;
  warnq = new Emsgqueue (NTXT ("lo_warnq"));
  commentq = new Emsgqueue (NTXT ("lo_commentq"));
  elf_lo = NULL;
  elf_inited = false;
  checksum = 0;
  isUsed = false;
  h_function = NULL;
  h_instr = NULL;

  char *nm = (char *) loname;
  if (strncmp (nm, NTXT ("./"), 2) == 0)
    nm += 2;
  set_name (nm);
  dbeFile = new DbeFile (nm);
  dbeFile->filetype |= DbeFile::F_LOADOBJ | DbeFile::F_FILE;
}

LoadObject::~LoadObject ()
{
  delete seg_modules_map;
  delete functions;
  delete[] instHTable;
  delete[] funcHTable;
  delete seg_modules;
  delete modules;
  delete elf_lo;
  free (pathname);
  free (arch_name);
  free (runTimePath);
  delete objStabs;
  delete warnq;
  delete commentq;
  delete h_instr;
}

Elf *
LoadObject::get_elf ()
{
  if (elf_lo == NULL)
    {
      if (dbeFile->get_need_refind ())
	elf_inited = false;
      if (elf_inited)
	return NULL;
      elf_inited = true;
      char *fnm = dbeFile->get_location ();
      if (fnm == NULL)
	{
	  append_msg (CMSG_ERROR, GTXT ("Cannot find file: `%s'"),
		      dbeFile->get_name ());
	  return NULL;
	}
      Elf::Elf_status st = Elf::ELF_ERR_CANT_OPEN_FILE;
      elf_lo = Elf::elf_begin (fnm, &st);
      if (elf_lo == NULL)
	switch (st)
	  {
	  case Elf::ELF_ERR_CANT_OPEN_FILE:
	    append_msg (CMSG_ERROR, GTXT ("Cannot open ELF file `%s'"), fnm);
	    break;
	  case Elf::ELF_ERR_BAD_ELF_FORMAT:
	  default:
	    append_msg (CMSG_ERROR, GTXT ("Cannot read ELF header of `%s'"),
			fnm);
	    break;
	  }
    }
  return elf_lo;
}

Stabs *
LoadObject::openDebugInfo (char *fname, Stabs::Stab_status *stp)
{
  if (objStabs == NULL)
    {
      if (fname == NULL)
	return NULL;
      objStabs = new Stabs (fname, get_pathname ());
      Stabs::Stab_status st = objStabs->get_status ();
      if ((st == Stabs::DBGD_ERR_NONE) && (checksum != 0))
	{
	  Elf *elf = get_elf ();
	  if (elf && (checksum != elf->elf_checksum ()))
	    {
	      char *buf = dbe_sprintf (GTXT ("*** Note: '%s' has an unexpected checksum value; perhaps it was rebuilt. File ignored"),
				       fname);
	      commentq->append (new Emsg (CMSG_ERROR, buf));
	      delete buf;
	      st = Stabs::DBGD_ERR_CHK_SUM;
	    }
	}
      if (stp)
	*stp = st;
      if (st != Stabs::DBGD_ERR_NONE)
	{
	  delete objStabs;
	  objStabs = NULL;
	}
    }
  return objStabs;
}

uint64_t
LoadObject::get_addr ()
{
  return MAKE_ADDRESS (seg_idx, 0);
}

bool
LoadObject::compare (const char *_path, int64_t _checksum)
{
  return _checksum == checksum && dbe_strcmp (_path, get_pathname ()) == 0;
}

int
LoadObject::compare (const char *_path, const char *_runTimePath, DbeFile *df)
{
  int ret = 0;
  if (dbe_strcmp (_path, get_pathname ()) != 0)
    return ret;
  ret |= CMP_PATH;
  if (_runTimePath)
    {
      if (dbe_strcmp (_runTimePath, runTimePath) != 0)
	return ret;
      ret |= CMP_RUNTIMEPATH;
    }
  if (df && dbeFile->compare (df))
    ret |= CMP_CHKSUM;
  return ret;
}

void
LoadObject::set_platform (Platform_t pltf, int wsz)
{
  switch (pltf)
    {
    case Sparc:
    case Sparcv9:
    case Sparcv8plus:
      platform = (wsz == W64) ? Sparcv9 : Sparc;
      break;
    case Intel:
    case Amd64:
      platform = (wsz == W64) ? Amd64 : Intel;
      break;
    default:
      platform = pltf;
      break;
    }
};

void
LoadObject::set_name (char *string)
{
  char *p;
  pathname = dbe_strdup (string);

  p = get_basename (pathname);
  if (p[0] == '<')
    name = dbe_strdup (p);
  else    // set a short name  to "<basename>"
    name = dbe_sprintf (NTXT ("<%s>"), p);
}

void
LoadObject::dump_functions (FILE *out)
{
  int index;
  Function *fitem;
  char *sname, *mname;
  if (platform == Java)
    {
      JMethod *jmthd;
      Vector<JMethod*> *jmethods = (Vector<JMethod*>*)functions;
      Vec_loop (JMethod*, jmethods, index, jmthd)
      {
	fprintf (out, "id %6llu, @0x%llx sz-%lld %s (module = %s)\n",
		 (unsigned long long) jmthd->id, (long long) jmthd->get_mid (),
		 (long long) jmthd->size, jmthd->get_name (),
		 jmthd->module ? jmthd->module->file_name : noname->file_name);
      }
    }
  else
    {
      Vec_loop (Function*, functions, index, fitem)
      {
	if (fitem->alias && fitem->alias != fitem)
	  fprintf (out, "id %6llu, @0x%llx -        %s == alias of '%s'\n",
		   (ull_t) fitem->id, (ull_t) fitem->img_offset,
		   fitem->get_name (), fitem->alias->get_name ());
	else
	  {
	    mname = fitem->module ? fitem->module->file_name : noname->file_name;
	    sname = fitem->getDefSrcName ();
	    fprintf (out,
		     "id %6llu, @0x%llx - 0x%llx [save 0x%llx] o-%lld sz-%lld %s (module = %s)",
		     (ull_t) fitem->id, (ull_t) fitem->img_offset,
		     (ull_t) (fitem->img_offset + fitem->size),
		     (ull_t) fitem->save_addr, (ull_t) fitem->img_offset,
		     (ll_t) fitem->size, fitem->get_name (), mname);
	    if (sname && !streq (sname, mname))
	      fprintf (out, " (Source = %s)", sname);
	    fprintf (out, "\n");
	  }
      }
    }
}

int
LoadObject::get_index (Function *func)
{
  Function *fp;
  uint64_t offset;
  int x;
  int left = 0;
  int right = functions->size () - 1;
  offset = func->img_offset;
  while (left <= right)
    {
      x = (left + right) / 2;
      fp = functions->fetch (x);

      if (left == right)
	{
	  if (offset >= fp->img_offset + fp->size)
	    return -1;
	  if (offset >= fp->img_offset)
	    return x;
	  return -1;
	}
      if (offset < fp->img_offset)
	right = x - 1;
      else if (offset >= fp->img_offset + fp->size)
	left = x + 1;
      else
	return x;
    }
  return -1;
}

char *
LoadObject::get_alias (Function *func)
{
  Function *fp, *alias;
  int index, nsize;
  static char buf[1024];
  if (func->img_offset == 0 || func->alias == NULL)
    return NULL;
  int fid = get_index (func);
  if (fid == -1)
    return NULL;

  nsize = functions->size ();
  alias = func->alias;
  for (index = fid; index < nsize; index++)
    {
      fp = functions->fetch (index);
      if (fp->alias != alias)
	{
	  fid = index;
	  break;
	}
    }

  *buf = '\0';
  for (index--; index >= 0; index--)
    {
      fp = functions->fetch (index);
      if (fp->alias != alias)
	break;
      if (fp != alias)
	{
	  size_t len = strlen (buf);
	  if (*buf != '\0')
	    {
	      snprintf (buf + len, sizeof (buf) - len, NTXT (", "));
	      len = strlen (buf);
	    }
	  snprintf (buf + len, sizeof (buf) - len, "%s", fp->get_name ());
	}
    }
  return buf;
}

DbeInstr*
LoadObject::find_dbeinstr (uint64_t file_off)
{
  int hash = (((int) file_off) >> 2) & (LO_InstHTableSize - 1);
  DbeInstr *instr = instHTable[hash];
  if (instr && instr->img_offset == file_off)
    return instr;
  Function *fp = find_function (file_off);
  if (fp == NULL)
    fp = dbeSession->get_Unknown_Function ();
  uint64_t func_off = file_off - fp->img_offset;
  instr = fp->find_dbeinstr (0, func_off);
  instHTable[hash] = instr;
  return instr;
}

Function *
LoadObject::find_function (uint64_t foff)
{
  // Look up in the hash table
  int hash = (((int) foff) >> 6) & (HTableSize - 1);
  Function *func = funcHTable[hash];
  if (func && foff >= func->img_offset && foff < func->img_offset + func->size)
    return func->alias ? func->alias : func;

  // Use binary search
  func = NULL;
  int left = 0;
  int right = functions->size () - 1;
  while (left <= right)
    {
      int x = (left + right) / 2;
      Function *fp = functions->fetch (x);
      assert (fp != NULL);

      if (foff < fp->img_offset)
	right = x - 1;
      else if (foff >= fp->img_offset + fp->size)
	left = x + 1;
      else
	{
	  func = fp;
	  break;
	}
    }

  // Plug the hole with a static function
  char *func_name = NULL;
  Size low_bound = 0, high_bound = 0;
  if (func == NULL)
    {
      int last = functions->size () - 1;
      uint64_t usize = size < 0 ? 0 : (uint64_t) size;
      if (last < 0)
	high_bound = foff >= usize ? foff : usize;
      else if (left == 0)
	high_bound = functions->fetch (left)->img_offset;
      else if (left < last)
	{
	  Function *fp = functions->fetch (left - 1);
	  low_bound = fp->img_offset + fp->size;
	  high_bound = functions->fetch (left)->img_offset;
	}
      else
	{
	  Function *fp = functions->fetch (last);
	  if (fp->flags & FUNC_FLAG_SIMULATED)
	    {
	      // Function is already created
	      func = fp;
	      uint64_t sz = func->size < 0 ? 0 : func->size;
	      if (sz + func->img_offset < foff)
		func->size = foff - func->img_offset;
	    }
	  else
	    {
	      low_bound = fp->img_offset + fp->size;
	      high_bound = foff > usize ? foff : usize;
	    }
	}
    }

  if (func == NULL)
    {
      func = dbeSession->createFunction ();
      func->flags |= FUNC_FLAG_SIMULATED;
      func->size = (unsigned) (high_bound - low_bound);
      func->module = noname;
      func->img_fname = get_pathname ();
      func->img_offset = (off_t) low_bound;
      noname->functions->append (func); // unordered
      if (func_name == NULL)
	func_name = dbe_sprintf (GTXT ("<static>@0x%llx (%s)"), low_bound,
				     name);
      func->set_name (func_name);
      free (func_name);

      // now insert the function
      functions->insert (left, func);
    }

  // Update the hash table
  funcHTable[hash] = func;
  return func->alias ? func->alias : func;
}

static void
fixFuncAlias (Vector<Function*> *SymLst)
{
  int ind, i, k;
  int64_t len, bestLen, maxSize;
  Function *sym, *bestAlias;

  // XXXX it is a clone of Stabs::fixSymtabAlias()
  ind = SymLst->size () - 1;
  for (i = 0; i < ind; i++)
    {
      bestAlias = SymLst->fetch (i);
      if (bestAlias->img_offset == 0) // Ignore this bad symbol
	continue;
      sym = SymLst->fetch (i + 1);
      if (bestAlias->img_offset != sym->img_offset)
	{
	  if (bestAlias->size == 0
	      || sym->img_offset < bestAlias->img_offset + bestAlias->size)
	    bestAlias->size = (int) (sym->img_offset - bestAlias->img_offset);
	  continue;
	}

      // Find a "best" alias
      bestLen = strlen (bestAlias->get_name ());
      maxSize = bestAlias->size;
      for (k = i + 1; k <= ind; k++)
	{
	  sym = SymLst->fetch (k);
	  if (bestAlias->img_offset != sym->img_offset)
	    { // no more aliases
	      if ((maxSize == 0) ||
		  (sym->img_offset < bestAlias->img_offset + maxSize))
		maxSize = sym->img_offset - bestAlias->img_offset;
	      break;
	    }
	  if (maxSize < sym->size)
	    maxSize = sym->size;
	  len = strlen (sym->get_name ());
	  if (len < bestLen)
	    {
	      bestAlias = sym;
	      bestLen = len;
	    }
	}
      for (; i < k; i++)
	{
	  sym = SymLst->fetch (i);
	  sym->alias = bestAlias;
	  sym->size = maxSize;
	}
      i--;
    }
}

void
LoadObject::post_process_functions ()
{
  if (flags & SEG_FLAG_DYNAMIC || platform == Java)
    return;

  char *msg = GTXT ("Processing Load Object Data");
  if (dbeSession->is_interactive ())
    theApplication->set_progress (1, msg);

  // First sort the functions
  functions->sort (func_compare);
  fixFuncAlias (functions);

  Module *mitem;
  int index;
  Vec_loop (Module*, seg_modules, index, mitem)
  {
    mitem->functions->sort (func_compare);
  }

  // Find any derived functions, and set their derivedNode
  Function *fitem;
  Vec_loop (Function*, functions, index, fitem)
  {
    if (dbeSession->is_interactive () && index % 5000 == 0)
      {
	int percent = (int) (100.0 * index / functions->size ());
	theApplication->set_progress (percent, (percent != 0) ? NULL : msg);
      }
    fitem->findDerivedFunctions ();
  }

  // 4987698: get the alias name for MAIN_
  fitem = find_function (NTXT ("MAIN_"));
  if (fitem)
    fitem->module->read_stabs ();
  fitem = find_function (NTXT ("@plt"));
  if (fitem)
    fitem->flags |= FUNC_FLAG_PLT;
  if (dbeSession->is_interactive ())
    theApplication->set_progress (0, NTXT (""));
}

int
LoadObject::func_compare (const void *p1, const void *p2)
{
  Function *f1 = *(Function **) p1;
  Function *f2 = *(Function **) p2;
  if (f1->img_offset != f2->img_offset)
    return f1->img_offset > f2->img_offset ? 1 : -1;

  // annotated source not available for weak symbols.
  if ((f1->module->flags & MOD_FLAG_UNKNOWN) != 0)
    {
      if ((f2->module->flags & MOD_FLAG_UNKNOWN) == 0)
	return -1;
    }
  else if ((f2->module->flags & MOD_FLAG_UNKNOWN) != 0)
    return 1;
  return strcoll (f1->get_name (), f2->get_name ());
}

Function *
LoadObject::find_function (char *fname)
{
  Function *fitem;
  int index;
  Vec_loop (Function*, functions, index, fitem)
  {
    if (strcmp (fitem->get_name (), fname) == 0)
      return fitem;
  }
  return (Function *) NULL;
}

Function *
LoadObject::find_function (char *fname, unsigned int chksum)
{
  Function *fitem;
  int index;
  Vec_loop (Function*, functions, index, fitem)
  {
    if (fitem->chksum == chksum && strcmp (fitem->get_name (), fname) == 0)
      return fitem;
  }
  return (Function *) NULL;
}

Module *
LoadObject::find_module (char *mname)
{
  for (int i = 0, sz = seg_modules ? seg_modules->size () : 0; i < sz; i++)
    {
      Module *module = seg_modules->fetch (i);
      if (strcmp (module->get_name (), mname) == 0)
	return module;
    }
  return (Module *) NULL;
}

LoadObject::Arch_status
LoadObject::sync_read_stabs ()
{
  Arch_status st = ARCHIVE_SUCCESS;
  if (!isReadStabs)
    {
      aquireLock ();
      if (!isReadStabs)
	{
	  st = read_stabs ();
	  post_process_functions ();
	  isReadStabs = true;
	}
      releaseLock ();
    }
  return st;
}

LoadObject::Arch_status
LoadObject::read_stabs ()
{
  if ((dbeFile->filetype & DbeFile::F_FICTION) != 0)
    return ARCHIVE_SUCCESS;
  Arch_status stabs_status = ARCHIVE_ERR_OPEN;
  if (platform == Java)
    {
      Module *cf = NULL;
      for (int i = 0, sz = seg_modules ? seg_modules->size () : 0; i < sz; i++)
	{
	  Module *mod = seg_modules->fetch (i);
	  if (mod->dbeFile
	      && (mod->dbeFile->filetype & DbeFile::F_JAVACLASS) != 0)
	    {
	      cf = mod;
	      break;
	    }
	}
      if (cf)
	{
	  int status = cf->readFile ();
	  switch (status)
	    {
	    case Module::AE_OK:
	      stabs_status = ARCHIVE_SUCCESS;
	      break;
	    case Module::AE_NOSTABS:
	      stabs_status = ARCHIVE_NO_STABS;
	      break;
	    case Module::AE_NOTREAD:
	    default:
	      stabs_status = ARCHIVE_ERR_OPEN;
	      break;
	    }
	}
    }
  else if (strchr (pathname, '`'))
    return ARCHIVE_SUCCESS;
  else
    {
      Arch_status st = ARCHIVE_WRONG_ARCH;
      Elf *elf = get_elf ();
      if (elf == NULL)
	{
	  if (read_archive () == 0)
	    st = ARCHIVE_SUCCESS;
	  else
	    {
	      char *msg = dbe_sprintf (GTXT ("*** Warning: Can't open file: %s"),
				       dbeFile->get_name ());
	      warnq->append (new Emsg (CMSG_ERROR, msg));
	      delete msg;
	    }
	}
      else if (checksum != 0 && checksum != elf->elf_checksum ())
	{
	  if (read_archive () == 0)
	    st = ARCHIVE_SUCCESS;
	  else
	    {
	      char *msg = dbe_sprintf (
				       GTXT ("*** Note: '%s' has an unexpected checksum value; perhaps it was rebuilt. File ignored"),
				       dbeFile->get_location ());
	      commentq->append (new Emsg (CMSG_ERROR, msg));
	      delete msg;
	    }
	}
      if (st == ARCHIVE_SUCCESS)    // An old archive is used
	return st;

      Stabs::Stab_status status = Stabs::DBGD_ERR_CANT_OPEN_FILE;
      char *location = dbeFile->get_location (true);
      if (location == NULL)
	return ARCHIVE_ERR_OPEN;

      if (openDebugInfo (location, &status))
	{
	  status = objStabs->read_archive (this);
	  isRelocatable = objStabs->is_relocatable ();
	  size = objStabs->get_textsz ();
	  platform = objStabs->get_platform ();
	  wsize = objStabs->get_class ();
	}

      switch (status)
	{
	case Stabs::DBGD_ERR_NONE:
	  stabs_status = ARCHIVE_SUCCESS;
	  break;
	case Stabs::DBGD_ERR_CANT_OPEN_FILE:
	  stabs_status = ARCHIVE_ERR_OPEN;
	  break;
	case Stabs::DBGD_ERR_BAD_ELF_LIB:
	case Stabs::DBGD_ERR_BAD_ELF_FORMAT:
	  stabs_status = ARCHIVE_BAD_STABS;
	  break;
	case Stabs::DBGD_ERR_NO_STABS:
	  stabs_status = ARCHIVE_NO_STABS;
	  break;
	case Stabs::DBGD_ERR_NO_DWARF:
	  stabs_status = ARCHIVE_NO_DWARF;
	  break;
	default:
	  stabs_status = ARCHIVE_BAD_STABS;
	  break;
	}
    }
  return stabs_status;
}

#define ARCH_STRLEN(s)      ((strlen(s) + 4) & ~0x3 )

static int
offsetCmp (const void *a, const void *b)
{
  uint32_t o1 = ((inst_info_t *) a)->offset;
  uint32_t o2 = ((inst_info_t *) b)->offset;
  return o1 == o2 ? 0 : (o1 < o2 ? -1 : 1);
}

int
LoadObject::read_archive ()
{
  if (arch_name == NULL)
    return 1;
  Module *mod = NULL;
  Function *func = NULL;
  char *buf;
  Data_window *dwin = new Data_window (arch_name);
  if (dwin->not_opened ())
    {
      delete dwin;
      buf = dbe_sprintf (GTXT ("*** Warning: Error opening file for reading: %s: %s"),
			 arch_name, strerror (errno));
      warnq->append (new Emsg (CMSG_ERROR, buf));
      delete buf;
      return 1;
    }
  dwin->need_swap_endian = need_swap_endian;

  // Prevent reading earlier archive files, which didn't support versioning.
  int64_t offset = 0;
  ARCH_common *cpkt = (ARCH_common*) dwin->bind (offset, sizeof (ARCH_common));
  uint16_t v16;
  if (cpkt)
    {
      v16 = (uint16_t) cpkt->type;
      if (dwin->decode (v16) != ARCH_SEGMENT)
	cpkt = NULL;
    }
  if (cpkt == NULL)
    {
      buf = dbe_sprintf (GTXT ("archive file malformed %s"), arch_name);
      warnq->append (new Emsg (CMSG_WARN, buf));
      delete buf;
      return 1;
    }

  char *msg = NULL;
  unsigned long long pointer_invalid = 0;
  for (int64_t last_offset = -5000;;)
    {
      cpkt = (ARCH_common*) dwin->bind (offset, sizeof (ARCH_common));
      if (cpkt == NULL)
	break;
      v16 = (uint16_t) cpkt->size;
      uint32_t cpktsize = dwin->decode (v16);
      cpkt = (ARCH_common*) dwin->bind (offset, cpktsize);
      if ((cpkt == NULL) || (cpktsize == 0))
	{
	  buf = dbe_sprintf (GTXT ("archive file malformed %s"), arch_name);
	  warnq->append (new Emsg (CMSG_WARN, buf));
	  delete buf;
	  break;
	}

      // Update the progress bar
      if (dbeSession->is_interactive () && ((offset - last_offset) >= 5000))
	{
	  last_offset = offset;
	  int percent = (int) (100.0 * offset / dwin->get_fsize ());
	  if (msg == NULL)
	    msg = dbe_sprintf (GTXT ("Reading Load Object Data: %s"), name);
	  theApplication->set_progress (percent, (percent != 0) ? NULL : msg);
	}
      char *ptr = (char *) cpkt;
      v16 = (uint16_t) cpkt->type;
      switch (dwin->decode (v16))
	{
	case ARCH_SEGMENT:
	  {
	    ARCH_segment *aseg = (ARCH_segment*) cpkt;
	    if (dwin->decode (aseg->version) != ARCH_VERSION)
	      {
		buf = dbe_sprintf (GTXT ("Archive file version mismatch for %s"), arch_name);
		warnq->append (new Emsg (CMSG_ERROR, buf));
		delete buf;
		if (dbeSession->is_interactive ())
		  theApplication->set_progress (0, "");
		return 1;
	      }
	    if (size == 0)
	      size = dwin->decode (aseg->textsz);
	    Platform_t pltf = (Platform_t) dwin->decode (aseg->platform);
	    if (pltf != Unknown)
	      {
		platform = pltf; // override if known
		wsize = (platform == Sparcv9 || platform == Amd64) ? W64 : W32;
	      }
	    break;
	  }
	case ARCH_MSG:
	  {
	    ARCH_message *amsg = (ARCH_message*) cpkt;
	    buf = status_str ((Arch_status) dwin->decode (amsg->errcode));
	    commentq->append (new Emsg (CMSG_ARCHIVE, buf));
	    free (buf);
	    break;
	  }
	case ARCH_INF:
	  {
	    ARCH_info *ainf = (ARCH_info*) cpkt;
	    Emsg *m = new Emsg (CMSG_ARCHIVE, (char*) (ainf + 1));
	    commentq->append (m);
	    break;
	  }
	case ARCH_MODULE:
	  {
	    ARCH_module *amod = (ARCH_module*) cpkt;
	    char *str = ((char*) amod) + sizeof (ARCH_module);
	    if (streq (str, SP_UNKNOWN_NAME) &&
		streq (str + ARCH_STRLEN (str), SP_UNKNOWN_NAME))
	      {
		mod = noname;
		break;
	      }
	    mod = dbeSession->createModule (this, str);
	    mod->lang_code = (Sp_lang_code) dwin->decode (amod->lang_code);
	    mod->fragmented = dwin->decode (amod->fragmented);
	    str += ARCH_STRLEN (str);
	    mod->set_file_name (dbe_strdup (str));
	    modules->put (get_basename (str), mod);
	    break;
	  }
	case ARCH_FUNCTION:
	  {
	    if (mod == NULL)
	      break;
	    ARCH_function *afnc = (ARCH_function*) cpkt;
	    func = dbeSession->createFunction ();
	    func->img_offset = dwin->decode (afnc->offset);
	    func->size = dwin->decode (afnc->size);
	    func->save_addr = dwin->decode (afnc->save_addr)
		    - dwin->decode (afnc->offset);
	    func->module = mod;
	    func->set_name (((char*) afnc) + sizeof (ARCH_function));
	    mod->functions->append (func);
	    functions->append (func);
	    break;
	  }
	case ARCH_LDINSTR:
	  if (mod == NULL)
	    break;
	  Dprintf (DEBUG_LOADOBJ, "LDINSTR list for %s\n", mod->get_name ());
	  if (mod->infoList == NULL)
	    mod->infoList = new Vector<inst_info_t*>;
	  for (memop_info_t *mp = (memop_info_t*) (ptr + sizeof (ARCH_aninfo));
		  (char*) mp < ptr + cpktsize; mp++)
	    {
	      memop_info_t *memop = new memop_info_t;
	      memop->offset = dwin->decode (mp->offset);
	      memop->id = dwin->decode (mp->id);
	      memop->signature = dwin->decode (mp->signature);
	      memop->datatype_id = dwin->decode (mp->datatype_id);
	      mod->ldMemops.append (memop);

	      inst_info_t *instop = new inst_info_t;
	      instop->type = CPF_INSTR_TYPE_LD;
	      instop->offset = memop->offset;
	      instop->memop = memop;
	      mod->infoList->incorporate (instop, offsetCmp);
	      Dprintf (DEBUG_LOADOBJ,
		       "ld: offset=0x%04x id=0x%08x sig=0x%08x dtid=0x%08x\n",
		       memop->offset, memop->id, memop->signature,
		       memop->datatype_id);
	    }
	  Dprintf (DEBUG_LOADOBJ, "LDINSTR list of %lld for %s\n",
		   (long long) mod->ldMemops.size (), mod->get_name ());
	  break;
	case ARCH_STINSTR:
	  if (mod == NULL)
	    break;
	  Dprintf (DEBUG_LOADOBJ, NTXT ("STINSTR list for %s\n"), mod->get_name ());
	  if (mod->infoList == NULL)
	    mod->infoList = new Vector<inst_info_t*>;
	  for (memop_info_t *mp = (memop_info_t*) (ptr + sizeof (ARCH_aninfo));
		  ((char *) mp) < ptr + cpktsize; mp++)
	    {
	      memop_info_t *memop = new memop_info_t;
	      memop->offset = dwin->decode (mp->offset);
	      memop->id = dwin->decode (mp->id);
	      memop->signature = dwin->decode (mp->signature);
	      memop->datatype_id = dwin->decode (mp->datatype_id);
	      mod->stMemops.append (memop);

	      inst_info_t *instop = new inst_info_t;
	      instop->type = CPF_INSTR_TYPE_ST;
	      instop->offset = memop->offset;
	      instop->memop = memop;
	      mod->infoList->incorporate (instop, offsetCmp);
	      Dprintf (DEBUG_LOADOBJ,
		       "st: offset=0x%04x id=0x%08x sig=0x%08x dtid=0x%08x\n",
		       memop->offset, memop->id, memop->signature,
		       memop->datatype_id);
	    }
	  Dprintf (DEBUG_LOADOBJ, "STINSTR list of %lld for %s\n",
		   (long long) mod->stMemops.size (), mod->get_name ());
	  break;
	case ARCH_PREFETCH:
	  if (mod == NULL)
	    break;
	  Dprintf (DEBUG_LOADOBJ, "PFINSTR list for %s\n", mod->get_name ());
	  if (mod->infoList == NULL)
	    mod->infoList = new Vector<inst_info_t*>;
	  for (memop_info_t *mp = (memop_info_t*) (ptr + sizeof (ARCH_aninfo));
		  ((char*) mp) < ptr + cpkt->size; mp++)
	    {
	      memop_info_t *memop = new memop_info_t;
	      memop->offset = dwin->decode (mp->offset);
	      memop->id = dwin->decode (mp->id);
	      memop->signature = dwin->decode (mp->signature);
	      memop->datatype_id = dwin->decode (mp->datatype_id);
	      mod->pfMemops.append (memop);

	      inst_info_t *instop = new inst_info_t;
	      instop->type = CPF_INSTR_TYPE_PREFETCH;
	      instop->offset = memop->offset;
	      instop->memop = memop;
	      mod->infoList->incorporate (instop, offsetCmp);
	      Dprintf (DEBUG_LOADOBJ,
		       "pf: offset=0x%04x id=0x%08x sig=0x%08x dtid=0x%08x\n",
		       memop->offset, memop->id, memop->signature,
		       memop->datatype_id);
	    }
	  Dprintf (DEBUG_LOADOBJ, "PFINSTR list of %lld for %s\n",
		   (long long) mod->pfMemops.size (), mod->get_name ());
	  break;
	case ARCH_BRTARGET:
	  if (mod == NULL)
	    break;
	  for (target_info_t *tp = (target_info_t*) (ptr + sizeof (ARCH_aninfo));
		  ((char*) tp) < ptr + cpkt->size; tp++)
	    {
	      target_info_t *bTarget = new target_info_t;
	      bTarget->offset = dwin->decode (tp->offset);
	      mod->bTargets.append (bTarget);
	    }
	  Dprintf (DEBUG_LOADOBJ, "BRTARGET list of %lld for %s\n",
		   (long long) mod->infoList->size (), mod->get_name ());
	  break;
	default:
	  /* Check if the prointer is valid - should be even. */
	  pointer_invalid = (unsigned long long) (offset + cpktsize) & 1;
	  break; // ignore unknown packets
	}
      if (pointer_invalid)
	break;
      offset += cpktsize;
    }
  delete msg;
  delete dwin;

  if (dbeSession->is_interactive ())
    theApplication->set_progress (0, NTXT (""));
  return 0;
}

char *
LoadObject::status_str (Arch_status rv, char */*arg*/)
{
  switch (rv)
    {
    case ARCHIVE_SUCCESS:
    case ARCHIVE_EXIST:
      return NULL;
    case ARCHIVE_BAD_STABS:
      return dbe_sprintf (GTXT ("Error: unable to read symbol table of %s"),
			  name);
    case ARCHIVE_ERR_SEG:
      return dbe_sprintf (GTXT ("Error: unable to read load object file %s"),
			  pathname);
    case ARCHIVE_ERR_OPEN:
      return dbe_sprintf (GTXT ("Error: unable to open file %s"),
			  pathname);
    case ARCHIVE_ERR_MAP:
      return dbe_sprintf (GTXT ("Error: unable to map file %s"),
			  pathname);
    case ARCHIVE_WARN_CHECKSUM:
      return dbe_sprintf (GTXT ("Note: checksum differs from that recorded in experiment for %s"),
			  name);
    case ARCHIVE_WARN_MTIME:
      return dbe_sprintf (GTXT ("Warning: last-modified time differs from that recorded in experiment for %s"),
			  name);
    case ARCHIVE_WARN_HOST:
      return dbe_sprintf (GTXT ("Try running er_archive -F on the experiment, on the host where it was recorded"));
    case ARCHIVE_ERR_VERSION:
      return dbe_sprintf (GTXT ("Error: Wrong version of archive for %s"),
			  pathname);
    case ARCHIVE_NO_STABS:
      return dbe_sprintf (GTXT ("Note: no stabs or dwarf information in %s"),
			  name);
    case ARCHIVE_WRONG_ARCH:
#if ARCH(SPARC)
      return dbe_sprintf (GTXT ("Error: file %s is built for Intel, and can't be read on SPARC"),
			  name);
#else
      return dbe_sprintf (GTXT ("Error: file %s is built for SPARC, and can't be read on Intel"),
			  name);
#endif
    case ARCHIVE_NO_LIBDWARF:
      return dbe_strdup (GTXT ("Warning: no libdwarf found to read DWARF symbol tables"));
    case ARCHIVE_NO_DWARF:
      return dbe_sprintf (GTXT ("Note: no DWARF symbol table in %s"), name);
    default:
      return dbe_sprintf (GTXT ("Warning: unexpected archive error %d"),
			  (int) rv);
    }
}

uint32_t
LoadObject::get_checksum ()
{
  char *errmsg = NULL;
  uint32_t crcval = get_cksum (pathname, &errmsg);
  if (0 == crcval && errmsg)
    {
      warnq->append (new Emsg (CMSG_ERROR, errmsg));
      free (errmsg);
    }
  return crcval;
}

static char*
get_module_map_key (Module *mod)
{
  return mod->lang_code == Sp_lang_java ? mod->get_name () : mod->file_name;
}

Module *
LoadObject::get_comparable_Module (Module *mod)
{
  if (mod->loadobject == this)
    return mod;
  if (get_module_map_key (mod) == NULL)
    return NULL;
  if (seg_modules_map == NULL)
    {
      seg_modules_map = new HashMap<char*, Module*>;
      for (int i = 0; i < seg_modules->size (); i++)
	{
	  Module *m = seg_modules->fetch (i);
	  char *key = get_module_map_key (m);
	  if (key)
	    {
	      seg_modules_map->put (m->file_name, m);
	      char *bname = get_basename (key);
	      if (bname != key)
		seg_modules_map->put (bname, m);
	    }
	}
    }

  char *key = get_module_map_key (mod);
  Module *cmpMod = seg_modules_map->get (key);
  if (cmpMod && cmpMod->comparable_objs == NULL)
    return cmpMod;
  char *bname = get_basename (key);
  if (bname != key)
    {
      cmpMod = seg_modules_map->get (bname);
      if (cmpMod && cmpMod->comparable_objs == NULL)
	return cmpMod;
    }
  return NULL;
}

Vector<Histable*> *
LoadObject::get_comparable_objs ()
{
  update_comparable_objs ();
  if (comparable_objs || dbeSession->expGroups->size () <= 1)
    return comparable_objs;
  comparable_objs = new Vector<Histable*>(dbeSession->expGroups->size ());
  for (int i = 0, sz = dbeSession->expGroups->size (); i < sz; i++)
    {
      ExpGroup *gr = dbeSession->expGroups->fetch (i);
      Histable *h = gr->get_comparable_loadObject (this);
      comparable_objs->append (h);
      if (h)
	h->comparable_objs = comparable_objs;
    }
  dump_comparable_objs ();
  return comparable_objs;
}

void
LoadObject::append_module (Module *mod)
{
  seg_modules->append (mod);
  if (seg_modules_map == NULL)
    seg_modules_map = new HashMap<char*, Module*>;
  char *key = get_module_map_key (mod);
  if (key)
    {
      seg_modules_map->put (key, mod);
      char *bname = get_basename (key);
      if (bname != key)
	seg_modules_map->put (bname, mod);
    }
}

// LIBRARY_VISIBILITY
Function *
LoadObject::get_hide_function ()
{
  if (h_function == NULL)
    h_function = dbeSession->create_hide_function (this);
  return h_function;
}

DbeInstr *
LoadObject::get_hide_instr (DbeInstr *instr)
{
  if (h_instr == NULL)
    {
      Function *hf = get_hide_function ();
      h_instr = hf->create_hide_instr (instr);
    }
  return h_instr;
}
