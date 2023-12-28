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
#include <sys/param.h>

#include "util.h"
#include "Elf.h"
#include "Dwarf.h"
#include "stab.h"
#include "DbeSession.h"
#include "CompCom.h"
#include "Stabs.h"
#include "LoadObject.h"
#include "Module.h"
#include "Function.h"
#include "info.h"
#include "StringBuilder.h"
#include "DbeFile.h"
#include "StringMap.h"

#define DISASM_REL_NONE     0     /* symtab search only */
#define DISASM_REL_ONLY     1     /* relocation search only */
#define DISASM_REL_TARG     2     /* relocatoin then symtab */

///////////////////////////////////////////////////////////////////////////////
// class StabReader
class StabReader
{
public:
  StabReader (Elf *_elf, Platform_t platform, int StabSec, int StabStrSec);
  ~StabReader () { };
  char *get_type_name (int t);
  char *get_stab (struct stab *np, bool comdat);
  void parse_N_OPT (Module *mod, char *str);
  int stabCnt;
  int stabNum;

private:
  Elf *elf;
  char *StabData;
  char *StabStrtab;
  char *StabStrtabEnd;
  int StrTabSize;
  int StabEntSize;
};

///////////////////////////////////////////////////////////////////////////////
// class Symbol

class Symbol
{
public:
  Symbol (Vector<Symbol*> *vec = NULL);

  ~Symbol ()
  {
    free (name);
  }

  inline Symbol *
  cardinal ()
  {
    return alias ? alias : this;
  }

  static void dump (Vector<Symbol*> *vec, char*msg);

  Function *func;
  Sp_lang_code lang_code;
  uint64_t value; // st_value used in sym_name()
  uint64_t save;
  int64_t size;
  uint64_t img_offset; // image offset in the ELF file
  char *name;
  Symbol *alias;
  int local_ind;
  int flags;
  bool defined;
};

Symbol::Symbol (Vector<Symbol*> *vec)
{
  func = NULL;
  lang_code = Sp_lang_unknown;
  value = 0;
  save = 0;
  size = 0;
  img_offset = 0;
  name = NULL;
  alias = NULL;
  local_ind = -1;
  flags = 0;
  defined = false;
  if (vec)
    vec->append (this);
}

void
Symbol::dump (Vector<Symbol*> *vec, char*msg)
{
  if (!DUMP_ELF_SYM || vec == NULL || vec->size () == 0)
    return;
  printf (NTXT ("======= Symbol::dump: %s =========\n"
		"         value |    img_offset     | flags|local_ind|\n"), msg);
  for (int i = 0; i < vec->size (); i++)
    {
      Symbol *sp = vec->fetch (i);
      printf (NTXT ("  %3d %8lld |0x%016llx |%5d |%8d |%s\n"),
	      i, (long long) sp->value, (long long) sp->img_offset, sp->flags,
	      sp->local_ind, sp->name ? sp->name : NTXT ("NULL"));
    }
  printf (NTXT ("\n===== END of Symbol::dump: %s =========\n\n"), msg);
}

// end of class Symbol
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// class Reloc
class Reloc
{
public:
  Reloc ();
  ~Reloc ();
  uint64_t type;
  uint64_t value;
  uint64_t addend;
  char *name;
};

Reloc::Reloc ()
{
  type = 0;
  value = 0;
  addend = 0;
  name = NULL;
}

Reloc::~Reloc ()
{
  free (name);
}
// end of class Reloc
///////////////////////////////////////////////////////////////////////////////

enum
{
  SYM_PLT       = 1 << 0,
  SYM_UNDEF     = 1 << 1
};

enum Section_type
{
  COMM1_SEC = 0x10000000,
  COMM_SEC  = 0x20000000,
  INFO_SEC  = 0x30000000,
  LOOP_SEC  = 0x40000000
};

struct cpf_stabs_t
{
  uint32_t type;    // Archive::AnalyzerInfoType
  uint32_t offset;  // offset in .__analyzer_info
  Module *module;   // table for appropriate Module
};

static char *get_info_com (int type, int32_t copy_inout);
static char *get_lp_com (unsigned hints, int parallel, char *dep);
static int ComCmp (const void *a, const void *b);
static ino64_t _src_inode = 0;
static char *_src_name;

// Comparing name
static int
SymNameCmp (const void *a, const void *b)
{
  Symbol *item1 = *((Symbol **) a);
  Symbol *item2 = *((Symbol **) b);
  return (item1->name == NULL) ? -1 :
	  (item2->name == NULL) ? 1 : strcmp (item1->name, item2->name);
}

// Comparing value: for sorting
static int
SymValueCmp (const void *a, const void *b)
{
  Symbol *item1 = *((Symbol **) a);
  Symbol *item2 = *((Symbol **) b);
  return (item1->value > item2->value) ? 1 :
	  (item1->value == item2->value) ? SymNameCmp (a, b) : -1;
}

// Comparing value: for searching (source name is always NULL)
static int
SymFindCmp (const void *a, const void *b)
{
  Symbol *item1 = *((Symbol **) a);
  Symbol *item2 = *((Symbol **) b);
  if (item1->value < item2->value)
    return -1;
  if (item1->value < item2->value + item2->size
      || item1->value == item2->value) // item2->size == 0
    return 0;
  return 1;
}

// Comparing value for sorting. It is used only for searching aliases.
static int
SymImgOffsetCmp (const void *a, const void *b)
{
  Symbol *item1 = *((Symbol **) a);
  Symbol *item2 = *((Symbol **) b);
  return (item1->img_offset > item2->img_offset) ? 1 :
	  (item1->img_offset == item2->img_offset) ? SymNameCmp (a, b) : -1;
}

static int
RelValueCmp (const void *a, const void *b)
{
  Reloc *item1 = *((Reloc **) a);
  Reloc *item2 = *((Reloc **) b);
  return (item1->value > item2->value) ? 1 :
	  (item1->value == item2->value) ? 0 : -1;
}

Stabs *
Stabs::NewStabs (char *_path, char *lo_name)
{
  Stabs *stabs = new Stabs (_path, lo_name);
  if (stabs->status != Stabs::DBGD_ERR_NONE)
    {
      delete stabs;
      return NULL;
    }
  return stabs;
}

Stabs::Stabs (char *_path, char *_lo_name)
{
  path = dbe_strdup (_path);
  lo_name = dbe_strdup (_lo_name);
  SymLstByName = NULL;
  pltSym = NULL;
  SymLst = new Vector<Symbol*>;
  RelLst = new Vector<Reloc*>;
  RelPLTLst = new Vector<Reloc*>;
  LocalLst = new Vector<Symbol*>;
  LocalFile = new Vector<char*>;
  LocalFileIdx = new Vector<int>;
  last_PC_to_sym = NULL;
  dwarf = NULL;
  elfDbg = NULL;
  elfDis = NULL;
  stabsModules = NULL;
  textsz = 0;
  wsize = Wnone;
  st_check_symtab = st_check_relocs = false;
  status = DBGD_ERR_NONE;

  if (openElf (false) == NULL)
    return;
  switch (elfDis->elf_getclass ())
    {
    case ELFCLASS32:
      wsize = W32;
      break;
    case ELFCLASS64:
      wsize = W64;
      break;
    }
  isRelocatable = elfDis->elf_getehdr ()->e_type == ET_REL;
  for (unsigned int pnum = 0; pnum < elfDis->elf_getehdr ()->e_phnum; pnum++)
    {
      Elf_Internal_Phdr *phdr = elfDis->get_phdr (pnum);
      if (phdr->p_type == PT_LOAD && phdr->p_flags == (PF_R | PF_X))
	{
	  if (textsz == 0)
	    textsz = phdr->p_memsz;
	  else
	    {
	      textsz = 0;
	      break;
	    }
	}
    }
}

Stabs::~Stabs ()
{
  delete pltSym;
  delete SymLstByName;
  Destroy (SymLst);
  Destroy (RelLst);
  Destroy (RelPLTLst);
  Destroy (LocalFile);
  delete elfDis;
  delete dwarf;
  delete LocalLst;
  delete LocalFileIdx;
  delete stabsModules;
  free (path);
  free (lo_name);
}

Elf *
Stabs::openElf (char *fname, Stab_status &st)
{
  Elf::Elf_status elf_status;
  Elf *elf = Elf::elf_begin (fname, &elf_status);
  if (elf == NULL)
    {
      switch (elf_status)
	{
	case Elf::ELF_ERR_CANT_OPEN_FILE:
	case Elf::ELF_ERR_CANT_MMAP:
	case Elf::ELF_ERR_BIG_FILE:
	  st = DBGD_ERR_CANT_OPEN_FILE;
	  break;
	case Elf::ELF_ERR_BAD_ELF_FORMAT:
	default:
	  st = DBGD_ERR_BAD_ELF_FORMAT;
	  break;
	}
      return NULL;
    }
  if (elf->elf_version (EV_CURRENT) == EV_NONE)
    {
      // ELF library out of date
      delete elf;
      st = DBGD_ERR_BAD_ELF_LIB;
      return NULL;
    }

  Elf_Internal_Ehdr *ehdrp = elf->elf_getehdr ();
  if (ehdrp == NULL)
    {
      // check machine
      delete elf;
      st = DBGD_ERR_BAD_ELF_FORMAT;
      return NULL;
    }
  switch (ehdrp->e_machine)
    {
    case EM_SPARC:
      platform = Sparc;
      break;
    case EM_SPARC32PLUS:
      platform = Sparcv8plus;
      break;
    case EM_SPARCV9:
      platform = Sparcv9;
      break;
    case EM_386:
      //    case EM_486:
      platform = Intel;
      break;
    case EM_X86_64:
      platform = Amd64;
      break;
    case EM_AARCH64:
      platform = Aarch64;
      break;
    default:
      platform = Unknown;
      break;
    }
  return elf;
}

Elf *
Stabs::openElf (bool dbg_info)
{
  if (status != DBGD_ERR_NONE)
    return NULL;
  if (elfDis == NULL)
    {
      elfDis = openElf (path, status);
      if (elfDis == NULL)
	return NULL;
    }
  if (!dbg_info)
    return elfDis;
  if (elfDbg == NULL)
    {
      elfDbg = elfDis->find_ancillary_files (lo_name);
      if (elfDbg == NULL)
	elfDbg = elfDis;
    }
  return elfDbg;
}

bool
Stabs::read_symbols (Vector<Function*> *functions)
{
  if (openElf (true) == NULL)
    return false;
  check_Symtab ();
  check_Relocs ();
  if (functions)
    {
      Function *fp;
      int index;
      Vec_loop (Function*, functions, index, fp)
      {
	fp->img_fname = path;
      }
    }
  return true;
}

char *
Stabs::sym_name (uint64_t target, uint64_t instr, int flag)
{
  long index;
  if (flag == DISASM_REL_ONLY || flag == DISASM_REL_TARG)
    {
      Reloc *relptr = new Reloc;
      relptr->value = instr;
      index = RelLst->bisearch (0, -1, &relptr, RelValueCmp);
      if (index >= 0)
	{
	  delete relptr;
	  return RelLst->fetch (index)->name;
	}
      if (!is_relocatable ())
	{
	  relptr->value = target;
	  index = RelPLTLst->bisearch (0, -1, &relptr, RelValueCmp);
	  if (index >= 0)
	    {
	      delete relptr;
	      return RelPLTLst->fetch (index)->name;
	    }
	}
      delete relptr;
    }
  if (flag == DISASM_REL_NONE || flag == DISASM_REL_TARG || !is_relocatable ())
    {
      Symbol *sptr;
      sptr = map_PC_to_sym (target);
      if (sptr && sptr->value == target)
	return sptr->name;
    }
  return NULL;
}

Symbol *
Stabs::map_PC_to_sym (uint64_t pc)
{
  if (pc == 0)
    return NULL;
  if (last_PC_to_sym && last_PC_to_sym->value <= pc
      && last_PC_to_sym->value + last_PC_to_sym->size > pc)
    return last_PC_to_sym;
  Symbol *sym = new Symbol;
  sym->value = pc;
  long index = SymLst->bisearch (0, -1, &sym, SymFindCmp);
  delete sym;
  if (index >= 0)
    {
      last_PC_to_sym = SymLst->fetch (index)->cardinal ();
      return last_PC_to_sym;
    }
  return NULL;
}

Function *
Stabs::map_PC_to_func (uint64_t pc, uint64_t &low_pc, Vector<Function*> *functions)
{
  int index;
  Function *func;
  Symbol *sptr = map_PC_to_sym (pc);
  if (sptr == NULL)
    return NULL;
  if (sptr->func)
    {
      low_pc = sptr->value;
      return sptr->func;
    }
  if (functions)
    {
      Vec_loop (Function*, functions, index, func)
      {
	if (func->img_offset == sptr->img_offset)
	  {
	    sptr->func = func->cardinal ();
	    low_pc = sptr->value;
	    return sptr->func;
	  }
      }
    }
  return NULL;
}

Stabs::Stab_status
Stabs::read_stabs (ino64_t srcInode, Module *module, Vector<ComC*> *comComs,
		   bool readDwarf)
{
  if (module)
    module->setIncludeFile (NULL);

  if (openElf (true) == NULL)
    return status;
  check_Symtab ();

  // read compiler commentary from .compcom1, .compcom,
  // .info, .loops, and .loopview sections
  if (comComs)
    {
      _src_inode = srcInode;
      _src_name = module && module->file_name ? get_basename (module->file_name) : NULL;
      if (!check_Comm (comComs))
	// .loops, and .loopview are now in .compcom
	check_Loop (comComs);

      // should not read it after .info goes into .compcom
      check_Info (comComs);
      comComs->sort (ComCmp);
    }

  // get stabs info
  Stab_status statusStabs = DBGD_ERR_NO_STABS;
#define SRC_LINE_STABS(sec, secStr, comdat) \
    if ((elfDbg->sec)  && (elfDbg->secStr) && \
	srcline_Stabs(module, elfDbg->sec, elfDbg->secStr, comdat) == DBGD_ERR_NONE) \
	statusStabs = DBGD_ERR_NONE

  SRC_LINE_STABS (stabExcl, stabExclStr, false);
  SRC_LINE_STABS (stab, stabStr, false);
  SRC_LINE_STABS (stabIndex, stabIndexStr, true);

  // read Dwarf, if any sections found
  if (elfDbg->dwarf && readDwarf)
    {
      openDwarf ()->srcline_Dwarf (module);
      if (dwarf && dwarf->status == DBGD_ERR_NONE)
	return DBGD_ERR_NONE;
    }
  return statusStabs;
}

static int
ComCmp (const void *a, const void *b)
{
  ComC *item1 = *((ComC **) a);
  ComC *item2 = *((ComC **) b);
  return (item1->line > item2->line) ? 1 :
	  (item1->line < item2->line) ? -1 :
	  (item1->sec > item2->sec) ? 1 :
	  (item1->sec < item2->sec) ? -1 : 0;
}

static int
check_src_name (char *srcName)
{
  if (_src_name && srcName && streq (_src_name, get_basename (srcName)))
    return 1;
  if (_src_inode == (ino64_t) - 1)
    return 0;
  DbeFile *dbeFile = dbeSession->getDbeFile (srcName, DbeFile::F_SOURCE);
  char *path = dbeFile->get_location ();
  return (path == NULL || dbeFile->sbuf.st_ino != _src_inode) ? 0 : 1;
}

bool
Stabs::check_Comm (Vector<ComC*> *comComs)
{
  int sz = comComs->size ();
  Elf *elf = openElf (true);
  if (elf == NULL)
    return false;

  for (unsigned int sec = 1; sec < elf->elf_getehdr ()->e_shnum; sec++)
    {
      char *name = elf->get_sec_name (sec);
      if (name == NULL)
	continue;
      Section_type sec_type;
      if (streq (name, NTXT (".compcom")))
	sec_type = COMM_SEC;
      else if (streq (name, NTXT (".compcom1")))
	sec_type = COMM1_SEC;
      else
	continue;

      // find header, set messages id & visibility if succeed
      CompComment *cc = new CompComment (elf, sec);
      int cnt = cc->compcom_open ((CheckSrcName) check_src_name);
      // process messages
      for (int index = 0; index < cnt; index++)
	{
	  int visible;
	  compmsg msg;
	  char *str = cc->compcom_format (index, &msg, visible);
	  if (str)
	    {
	      ComC *citem = new ComC;
	      citem->sec = sec_type + index;
	      citem->type = msg.msg_type;
	      citem->visible = visible;
	      citem->line = (msg.lineno < 1) ? 1 : msg.lineno;
	      citem->com_str = str;
	      comComs->append (citem);
	    }
	}
      delete cc;
    }
  return (sz != comComs->size ());
}

static int
targetOffsetCmp (const void *a, const void *b)
{
  uint32_t o1 = ((target_info_t *) a)->offset;
  uint32_t o2 = ((target_info_t *) b)->offset;
  return (o1 >= o2);
}

void
Stabs::check_AnalyzerInfo ()
{
  Elf *elf = openElf (true);
  if ((elf == NULL) || (elf->analyzerInfo == 0))
    {
      Dprintf (DEBUG_STABS, NTXT ("Stabs::check_AnalyzerInfo: Null AnalyzerInfo section\n"));
      return; // inappropriate, but ignored anyway
    }
  Elf_Data *data = elf->elf_getdata (elf->analyzerInfo);
  int InfoSize = (int) data->d_size;
  char *InfoData = (char *) data->d_buf;
  int InfoAlign = (int) data->d_align;
  AnalyzerInfoHdr h;
  unsigned infoHdr_sz = sizeof (AnalyzerInfoHdr);
  int table, entry;
  int read = 0;
  Module *mitem;
  int index = 0;
  if (InfoSize <= 0)
    return;
  uint64_t baseAddr = elf->get_baseAddr ();
  Dprintf (DEBUG_STABS, NTXT ("Stabs::check_AnalyzerInfo size=%d @0x%lx (align=%d) base=0x%llx\n"),
	   InfoSize, (ul_t) InfoData, InfoAlign, (long long) baseAddr);
  Dprintf (DEBUG_STABS, NTXT ("analyzerInfoMap has %lld entries\n"), (long long) analyzerInfoMap.size ());
  if (analyzerInfoMap.size () == 0)
    {
      Dprintf (DEBUG_STABS, NTXT ("No analyzerInfoMap available!\n"));
      return;
    }

  // verify integrity of analyzerInfoMap before reading analyzerInfo
  unsigned count = 0;
  Module *lastmod = NULL;
  for (index = 0; index < analyzerInfoMap.size (); index++)
    {
      cpf_stabs_t map = analyzerInfoMap.fetch (index);
      if (map.type > 3)
	{
	  Dprintf (DEBUG_STABS, NTXT ("analyzerInfo contains table of unknown type %d for %s\n"),
		   map.type, map.module->get_name ());
	  return;
	}
      if (map.module != lastmod)
	{
	  if (lastmod != NULL)
	    Dprintf (DEBUG_STABS, "analyzerInfo contains %d 0x0 offset tables for %s\n",
		     count, lastmod->get_name ());
	  count = 0;
	}
      count += (map.offset == 0x0); // only check for 0x0 tables for now
      if (count > 4)
	{
	  Dprintf (DEBUG_STABS, NTXT ("analyzerInfo contains too many 0x0 offset tables for %s\n"),
		   map.module->get_name ());
	  return;
	}
      lastmod = map.module;
    }

  index = 0;
  while ((index < analyzerInfoMap.size ()) && (read < InfoSize))
    {
      for (table = 0; table < 3; table++)
	{ // memory operations (ld, st, prefetch)
	  // read the table header
	  memcpy ((void *) &h, (const void *) InfoData, infoHdr_sz);
	  InfoData += infoHdr_sz;
	  read += infoHdr_sz;

	  // use map for appropriate module
	  cpf_stabs_t map = analyzerInfoMap.fetch (index);
	  index++;
	  mitem = map.module;
	  Dprintf (DEBUG_STABS, "Table %d offset=0x%04x "
		   "text_labelref=0x%08llx entries=%d version=%d\n"
		   "itype %d offset=0x%04x module=%s\n", table, read,
		   (long long) (h.text_labelref - baseAddr), h.entries,
		   h.version, map.type, map.offset, map.module->get_name ());
	  // read the table entries
	  for (entry = 0; entry < h.entries; entry++)
	    {
	      memop_info_t *m = new memop_info_t;
	      unsigned memop_info_sz = sizeof (memop_info_t);
	      memcpy ((void *) m, (const void *) InfoData, memop_info_sz);
	      InfoData += memop_info_sz;
	      read += memop_info_sz;
	      m->offset += (uint32_t) (h.text_labelref - baseAddr);
	      Dprintf (DEBUG_STABS, NTXT ("%4d(%d): offset=0x%04x id=0x%08x sig=0x%08x dtid=0x%08x\n"),
		       entry, table, m->offset, m->id, m->signature, m->datatype_id);
	      switch (table)
		{
		case CPF_INSTR_TYPE_LD:
		  mitem->ldMemops.append (m);
		  break;
		case CPF_INSTR_TYPE_ST:
		  mitem->stMemops.append (m);
		  break;
		case CPF_INSTR_TYPE_PREFETCH:
		  mitem->pfMemops.append (m);
		  break;
		}
	    }
	  // following re-alignment should be redundant
	  //InfoData+=(read%InfoAlign); read+=(read%InfoAlign); // re-align
	}
      for (table = 3; table < 4; table++)
	{ // branch targets
	  memcpy ((void *) &h, (const void *) InfoData, infoHdr_sz);
	  InfoData += infoHdr_sz;
	  read += infoHdr_sz;

	  // use map for appropriate module
	  cpf_stabs_t map = analyzerInfoMap.fetch (index);
	  index++;
	  mitem = map.module;
	  Dprintf (DEBUG_STABS, "Table %d offset=0x%04x "
		   "text_labelref=0x%08llx entries=%d version=%d\n"
		   "itype %d offset=0x%04x module=%s\n", table, read,
		   (long long) (h.text_labelref - baseAddr), h.entries,
		   h.version, map.type, map.offset, map.module->get_name ());
	  for (entry = 0; entry < h.entries; entry++)
	    {
	      target_info_t *t = new target_info_t;
	      unsigned target_info_sz = sizeof (target_info_t);
	      memcpy ((void *) t, (const void *) InfoData, target_info_sz);
	      InfoData += target_info_sz;
	      read += target_info_sz;
	      t->offset += (uint32_t) (h.text_labelref - baseAddr);
	      Dprintf (DEBUG_STABS, NTXT ("%4d(%d): offset=0x%04x\n"), entry,
		       table, t->offset);
	      // the list of branch targets needs to be in offset sorted order
	      // and doing it here before archiving avoids the need to do it
	      // each time the archive is read.
	      mitem->bTargets.incorporate (t, targetOffsetCmp);
	    }
	  Dprintf (DEBUG_STABS, NTXT ("bTargets for %s has %lld items (last=0x%04x)\n"),
		   mitem->get_name (), (long long) mitem->bTargets.size (),
		   (mitem->bTargets.fetch (mitem->bTargets.size () - 1))->offset);
	  Dprintf (DEBUG_STABS, "read=%d at end of bTargets (InfoData=0x%lx)\n",
		   read, (ul_t) InfoData);
	  InfoData += (read % InfoAlign);
	  read += (read % InfoAlign); // re-align
	  Dprintf (DEBUG_STABS, "read=%d at end of bTargets (InfoData=0x%lx)\n",
		   read, (ul_t) InfoData);
	}
      Dprintf (DEBUG_STABS, "Stabs::check_AnalyzerInfo bytes read=%lld (index=%lld/%lld)\n",
	       (long long) read, (long long) index,
	       (long long) analyzerInfoMap.size ());
    }
}

void
Stabs::check_Info (Vector<ComC*> *comComs)
{
  Elf *elf = openElf (true);
  if (elf == NULL || elf->info == 0)
    return;
  Elf_Data *data = elf->elf_getdata (elf->info);
  uint64_t InfoSize = data->d_size;
  char *InfoData = (char *) data->d_buf;
  bool get_src = false;
  for (int h_num = 0; InfoSize; h_num++)
    {
      if (InfoSize < sizeof (struct info_header))
	return;
      struct info_header *h = (struct info_header*) InfoData;
      if (h->endian != '\0' || h->magic[0] != 'S' || h->magic[1] != 'U'
	  || h->magic[2] != 'N')
	return;
      if (h->len < InfoSize || h->len < sizeof (struct info_header) || (h->len & 3))
	return;

      char *fname = InfoData + sizeof (struct info_header);
      InfoData += h->len;
      InfoSize -= h->len;
      get_src = check_src_name (fname);
      for (uint32_t e_num = 0; e_num < h->cnt; ++e_num)
	{
	  if (InfoSize < sizeof (struct entry_header))
	    return;
	  struct entry_header *e = (struct entry_header*) InfoData;
	  if (InfoSize < e->len)
	    return;
	  int32_t copy_inout = 0;
	  if (e->len > sizeof (struct entry_header))
	    if (e->type == F95_COPYINOUT)
	      copy_inout = *(int32_t*) (InfoData + sizeof (struct entry_header));
	  InfoData += e->len;
	  InfoSize -= e->len;
	  if (get_src)
	    {
	      ComC *citem = new ComC;
	      citem->sec = INFO_SEC + h_num;
	      citem->type = e->msgnum & 0xFFFFFF;
	      citem->visible = CCMV_ALL;
	      citem->line = e->line;
	      citem->com_str = get_info_com (citem->type, copy_inout);
	      comComs->append (citem);
	    }
	}
      if (get_src)
	break;
    }
}

static char *
get_info_com (int type, int32_t copy_inout)
{
  switch (type)
    {
    case 1:
      return dbe_sprintf (GTXT ("In the call below, parameter number %d caused a copy-in -- loop(s) inserted"),
			  copy_inout);
    case 2:
      return dbe_sprintf (GTXT ("In the call below, parameter number %d caused a copy-out -- loop(s) inserted"),
			  copy_inout);
    case 3:
      return dbe_sprintf (GTXT ("In the call below, parameter number %d caused a copy-in and a copy-out -- loops inserted"),
			  copy_inout);
    case 4:
      return dbe_strdup (GTXT ("Alignment of variables in common block may cause performance degradation"));
    case 5:
      return dbe_strdup (GTXT ("DO statement bounds lead to no executions of the loop"));
    default:
      return dbe_strdup (NTXT (""));
    }
}

void
Stabs::check_Loop (Vector<ComC*> *comComs)
{
  Elf *elf = openElf (true);
  if (elf == NULL)
    return;

  StringBuilder sb;
  for (unsigned int sec = 1; sec < elf->elf_getehdr ()->e_shnum; sec++)
    {
      char *name = elf->get_sec_name (sec);
      if (name == NULL)
	continue;
      if (!streq (name, NTXT (".loops")) && !streq (name, NTXT (".loopview")))
	continue;

      Elf_Data *data = elf->elf_getdata (sec);
      size_t LoopSize = (size_t) data->d_size, len;
      char *LoopData = (char *) data->d_buf;
      int remainder, i;
      char src[2 * MAXPATHLEN], buf1[MAXPATHLEN], buf2[MAXPATHLEN];
      char **dep_str = NULL;
      bool get_src = false;
      while ((LoopSize > 0) && !get_src &&
	     (strncmp (LoopData, NTXT ("Source:"), 7) == 0))
	{
	  // The first three items in a .loops subsection are three strings.
	  //	Source: ...
	  //	Version: ...
	  //	Number of loops: ...
	  sscanf (LoopData, NTXT ("%*s%s"), src);
	  len = strlen (LoopData) + 1;
	  LoopData += len;
	  LoopSize -= len;
	  sscanf (LoopData, NTXT ("%*s%*s%s"), buf1);
	  //	double version   = atof(buf1);
	  len = strlen (LoopData) + 1;
	  LoopData += len;
	  LoopSize -= len;
	  get_src = check_src_name (src);
	  sscanf (LoopData, NTXT ("%*s%*s%*s%s%s"), buf1, buf2);
	  int n_loop = atoi (buf1);
	  int n_depend = atoi (buf2);
	  len = strlen (LoopData) + 1;
	  LoopData += len;
	  LoopSize -= len;
	  if (get_src && (n_loop > 0))
	    {
	      dep_str = new char*[n_loop];
	      for (i = 0; i < n_loop; i++)
		dep_str[i] = NULL;
	    }

	  // printf("Source: %s\nVersion: %f\nLoop#: %d\nDepend#: %d\n",
	  //	src, version, n_loop, n_depend);

	  // Read in the strings that contain the list of variables that cause
	  // data dependencies inside of loops. Not every loop has such a list
	  // of variables.
	  //
	  //	Example: if loop #54 has data dependencies caused by the
	  //	variables named i, j and foo, then the string that represents
	  //	this in the .loops section looks like this:
	  //
	  //	.asciz "54:i.j.foo"
	  //
	  //	The variable names are delimited with .
	  //
	  //	For now, store these strings in an array, and add them into
	  //	the loop structure when we read in the numeric loop info
	  //	(that's what we read in next.)
	  //
	  // printf("\tDependenncies:\n");
	  for (i = 0; i < n_depend; i++)
	    {
	      len = strlen (LoopData) + 1;
	      LoopData += len;
	      LoopSize -= len;
	      if (dep_str != NULL)
		{
		  char *dep_buf1 = dbe_strdup (LoopData);
		  char *ptr = strtok (dep_buf1, NTXT (":"));
		  if (ptr != NULL)
		    {
		      int index = atoi (ptr);
		      bool dep_first = true;
		      sb.setLength (0);
		      while ((ptr = strtok (NULL, NTXT (", "))) != NULL)
			{
			  if (dep_first)
			    dep_first = false;
			  else
			    sb.append (NTXT (", "));
			  sb.append (ptr);
			}
		      if (sb.length () > 0 && index < n_loop)
			dep_str[index] = sb.toString ();
		    }
		  free (dep_buf1);
		}
	    }

	  // Adjust Data pointer so that it is word aligned.
	  remainder = (int) (((unsigned long) LoopData) % 4);
	  if (remainder != 0)
	    {
	      len = 4 - remainder;
	      LoopData += len;
	      LoopSize -= len;
	    }

	  // Read in the loop info, one loop at a time.
	  for (i = 0; i < n_loop; i++)
	    {
	      int loopid = *((int *) LoopData);
	      LoopData += 4;
	      int line_no = *((int *) LoopData);
	      if (line_no < 1) // compiler has trouble on this
		line_no = 1;
	      LoopData += 4;
	      //	    int nest = *((int *) LoopData);
	      LoopData += 4;
	      int parallel = *((int *) LoopData);
	      LoopData += 4;
	      unsigned hints = *((unsigned *) LoopData);
	      LoopData += 4;
	      //	    int count = *((int *) LoopData);
	      LoopData += 4;
	      LoopSize -= 24;
	      if (!get_src || (loopid >= n_loop))
		continue;
	      ComC *citem = new ComC;
	      citem->sec = LOOP_SEC + i;
	      citem->type = hints;
	      citem->visible = CCMV_ALL;
	      citem->line = line_no;
	      citem->com_str = get_lp_com (hints, parallel, dep_str[loopid]);
	      comComs->append (citem);
	    }
	  if (dep_str)
	    {
	      for (i = 0; i < n_loop; i++)
		free (dep_str[i]);
	      delete[] dep_str;
	      dep_str = NULL;
	    }
	}
    }
}

static char *
get_lp_com (unsigned hints, int parallel, char *dep)
{
  StringBuilder sb;
  if (parallel == -1)
    sb.append (GTXT ("Loop below is serial, but parallelizable: "));
  else if (parallel == 0)
    sb.append (GTXT ("Loop below is not parallelized: "));
  else
    sb.append (GTXT ("Loop below is parallelized: "));
  switch (hints)
    {
    case 0:
      // No loop mesg will print
      // strcat(com, GTXT("no hint available"));
      break;
    case 1:
      sb.append (GTXT ("loop contains procedure call"));
      break;
    case 2:
      sb.append (GTXT ("compiler generated two versions of this loop"));
      break;
    case 3:
      {
	StringBuilder sb_tmp;
	sb_tmp.sprintf (GTXT ("the variable(s) \"%s\" cause a data dependency in this loop"),
			dep ? dep : GTXT ("<Unknown>"));
	sb.append (&sb_tmp);
      }
      break;
    case 4:
      sb.append (GTXT ("loop was significantly transformed during optimization"));
      break;
    case 5:
      sb.append (GTXT ("loop may or may not hold enough work to be profitably parallelized"));
      break;
    case 6:
      sb.append (GTXT ("loop was marked by user-inserted pragma"));
      break;
    case 7:
      sb.append (GTXT ("loop contains multiple exits"));
      break;
    case 8:
      sb.append (GTXT ("loop contains I/O, or other function calls, that are not MT safe"));
      break;
    case 9:
      sb.append (GTXT ("loop contains backward flow of control"));
      break;
    case 10:
      sb.append (GTXT ("loop may have been distributed"));
      break;
    case 11:
      sb.append (GTXT ("two loops or more may have been fused"));
      break;
    case 12:
      sb.append (GTXT ("two or more loops may have been interchanged"));
      break;
    default:
      break;
    }
  return sb.toString ();
}

StabReader::StabReader (Elf *_elf, Platform_t platform, int StabSec, int StabStrSec)
{
  stabCnt = -1;
  stabNum = 0;
  if (_elf == NULL)
    return;
  elf = _elf;

  // Get ELF data
  Elf_Data *data = elf->elf_getdata (StabSec);
  if (data == NULL)
    return;
  uint64_t stabSize = data->d_size;
  StabData = (char *) data->d_buf;
  Elf_Internal_Shdr *shdr = elf->get_shdr (StabSec);
  if (shdr == NULL)
    return;

  // GCC bug: sh_entsize is 20 for 64 apps on Linux
  StabEntSize = (platform == Amd64 || platform == Sparcv9) ? 12 : (unsigned) shdr->sh_entsize;
  if (stabSize == 0 || StabEntSize == 0)
    return;
  data = elf->elf_getdata (StabStrSec);
  if (data == NULL)
    return;
  shdr = elf->get_shdr (StabStrSec);
  if (shdr == NULL)
    return;
  StabStrtab = (char *) data->d_buf;
  StabStrtabEnd = StabStrtab + shdr->sh_size;
  StrTabSize = 0;
  stabCnt = (int) (stabSize / StabEntSize);
}

char *
StabReader::get_stab (struct stab *np, bool comdat)
{
  struct stab *stbp = (struct stab *) (StabData + stabNum * StabEntSize);
  stabNum++;
  *np = *stbp;
  np->n_desc = elf->decode (stbp->n_desc);
  np->n_strx = elf->decode (stbp->n_strx);
  np->n_value = elf->decode (stbp->n_value);
  switch (np->n_type)
    {
    case N_UNDF:
    case N_ILDPAD:
      // Start of new stab section (or padding)
      StabStrtab += StrTabSize;
      StrTabSize = np->n_value;
    }

  char *str = NULL;
  if (np->n_strx)
    {
      if (comdat && np->n_type == N_FUN && np->n_other == 1)
	{
	  if (np->n_strx == 1)
	    StrTabSize++;
	  str = StabStrtab + StrTabSize;
	  // Each COMDAT string must be sized to find the next string:
	  StrTabSize += strlen (str) + 1;
	}
      else
	str = StabStrtab + np->n_strx;
      if (str >= StabStrtabEnd)
	str = NULL;
    }
  if (DEBUG_STABS)
    {
      char buf[128];
      char *s = get_type_name (np->n_type);
      if (s == NULL)
	{
	  snprintf (buf, sizeof (buf), NTXT ("n_type=%d"), np->n_type);
	  s = buf;
	}
      if (str)
	{
	  Dprintf (DEBUG_STABS, NTXT ("%4d:  .stabs \"%s\",%s,0x%x,0x%x,0x%x\n"),
		   stabNum - 1, str, s, (int) np->n_other, (int) np->n_desc,
		   (int) np->n_value);
	}
      else
	Dprintf (DEBUG_STABS, NTXT ("%4d:  .stabn %s,0x%x,0x%x,0x%x\n"),
		 stabNum - 1, s, (int) np->n_other, (int) np->n_desc,
		 (int) np->n_value);
    }
  return str;
}

void
StabReader::parse_N_OPT (Module *mod, char *str)
{
  if (mod == NULL || str == NULL)
      return;
  for (char *s = str; 1; s++)
    {
      switch (*s)
	{
	case 'd':
	  if (s[1] == 'i' && s[2] == ';')
	    {
	      delete mod->dot_o_file;
	      mod->dot_o_file = NULL;
	    }
	  break;
	case 's':
	  if ((s[1] == 'i' || s[1] == 'n') && s[2] == ';')
	    {
	      delete mod->dot_o_file;
	      mod->dot_o_file = NULL;
	    }
	  break;
	}
      s = strchr (s, ';');
      if (s == NULL)
	break;
    }
}

Stabs::Stab_status
Stabs::srcline_Stabs (Module *module, unsigned int StabSec,
		      unsigned int StabStrSec, bool comdat)
{
  StabReader *stabReader = new StabReader (openElf (true), platform, StabSec, StabStrSec);
  int tot = stabReader->stabCnt;
  if (tot < 0)
    {
      delete stabReader;
      return DBGD_ERR_NO_STABS;
    }
  int n, lineno;
  char *sbase, *n_so = NTXT (""), curr_src[2 * MAXPATHLEN];
  Function *newFunc;
  Sp_lang_code _lang_code = module->lang_code;
  Vector<Function*> *functions = module->functions;
  bool no_stabs = true;
  *curr_src = '\0';
  Function *func = NULL;
  int phase = 0;
  int stabs_level = 0;
  int xline = 0;

  // Find module
  for (n = 0; n < tot; n++)
    {
      struct stab stb;
      char *str = stabReader->get_stab (&stb, comdat);
      if (stb.n_type == N_UNDF)
	phase = 0;
      else if (stb.n_type == N_SO)
	{
	  if (str == NULL || *str == '\0')
	    continue;
	  if (phase == 0)
	    {
	      phase = 1;
	      n_so = str;
	      continue;
	    }
	  phase = 0;
	  sbase = str;
	  if (*str == '/')
	    {
	      if (streq (sbase, module->file_name))
		break;
	    }
	  else
	    {
	      size_t last = strlen (n_so);
	      if (n_so[last - 1] == '/')
		last--;
	      if (strncmp (n_so, module->file_name, last) == 0 &&
		  module->file_name[last] == '/' &&
		  streq (sbase, module->file_name + last + 1))
		break;
	    }
	}
    }
  if (n >= tot)
    {
      delete stabReader;
      return DBGD_ERR_NO_STABS;
    }

  Include *includes = new Include;
  includes->new_src_file (module->getMainSrc (), 0, NULL);
  module->hasStabs = true;
  *curr_src = '\0';
  phase = 0;
  for (n++; n < tot; n++)
    {
      struct stab stb;
      char *str = stabReader->get_stab (&stb, comdat);
      int n_desc = (int) ((unsigned short) stb.n_desc);
      switch (stb.n_type)
	{
	case N_UNDF:
	case N_SO:
	case N_ENDM:
	  n = tot;
	  break;
	case N_ALIAS:
	  if (str == NULL)
	    break;
	  if (is_fortran (_lang_code))
	    {
	      char *p = strchr (str, ':');
	      if (p && streq (p + 1, NTXT ("FMAIN")))
		{
		  Function *afunc = find_func (NTXT ("MAIN"), functions, true);
		  if (afunc)
		    afunc->set_match_name (dbe_strndup (str, p - str));
		  break;
		}
	    }
	case N_FUN:
	case N_OUTL:
	  if (str == NULL)
	    break;
	  if (*str == '@')
	    {
	      str++;
	      if (*str == '>' || *str == '<')
		str++;
	    }
	  if (stabs_level != 0)
	    break;

	  // find address of the enclosed function
	  newFunc = find_func (str, functions, is_fortran (_lang_code));
	  if (newFunc == NULL)
	    break;
	  if (func)
	    while (func->popSrcFile ())
	      ;
	  func = newFunc;

	  // First line info to cover function from the beginning
	  lineno = xline + n_desc;
	  if (lineno > 0)
	    {
	      // Set the chain of includes for the new function
	      includes->push_src_files (func);
	      func->add_PC_info (0, lineno);
	      no_stabs = false;
	    }
	  break;
	case N_ENTRY:
	  break;
	case N_CMDLINE:
	  if (str && !module->comp_flags)
	    {
	      char *comp_flags = strchr (str, ';');
	      if (comp_flags)
		{
		  module->comp_flags = dbe_strdup (comp_flags + 1);
		  module->comp_dir = dbe_strndup (str, comp_flags - str);
		}
	    }
	  break;
	case N_LBRAC:
	  stabs_level++;
	  break;
	case N_RBRAC:
	  stabs_level--;
	  break;
	case N_XLINE:
	  xline = n_desc << 16;
	  break;
	case N_SLINE:
	  if (func == NULL)
	    break;
	  no_stabs = false;
	  lineno = xline + n_desc;
	  if (func->line_first <= 0)
	    {
	      // Set the chain of includes for the new function
	      includes->push_src_files (func);
	      func->add_PC_info (0, lineno);
	      break;
	    }
	  if (func->curr_srcfile == NULL)
	    includes->push_src_files (func);
	  if (func->line_first != lineno ||
	      !streq (curr_src, func->getDefSrc ()->get_name ()))
	    func->add_PC_info (stb.n_value, lineno);
	  break;
	case N_OPT:
	  if ((str != NULL) && streq (str, NTXT ("gcc2_compiled.")))
	    _lang_code = Sp_lang_gcc;
	  switch (elfDbg->elf_getehdr ()->e_type)
	    {
	    case ET_EXEC:
	    case ET_DYN:
	      // set the real object timestamp from the executable's N_OPT stab
	      // due to bug #4796329
	      module->real_timestamp = stb.n_value;
	      break;
	    default:
	      module->curr_timestamp = stb.n_value;
	      break;
	    }
	  break;
	case N_GSYM:
	  if ((str == NULL) || strncmp (str, NTXT ("__KAI_K"), 7))
	    break;
	  str += 7;
	  if (!strncmp (str, NTXT ("CC_"), 3))
	    _lang_code = Sp_lang_KAI_KCC;
	  else if (!strncmp (str, NTXT ("cc_"), 3))
	    _lang_code = Sp_lang_KAI_Kcc;
	  else if (!strncmp (str, NTXT ("PTS_"), 4) &&
		   (_lang_code != Sp_lang_KAI_KCC) &&
		   (_lang_code != Sp_lang_KAI_Kcc))
	    _lang_code = Sp_lang_KAI_KPTS;
	  break;
	case N_BINCL:
	  includes->new_include_file (module->setIncludeFile (str), func);
	  break;
	case N_EINCL:
	  includes->end_include_file (func);
	  break;
	case N_SOL:
	  if (str == NULL)
	    break;
	  lineno = xline + n_desc;
	  if (lineno > 0 && func && func->line_first <= 0)
	    {
	      includes->push_src_files (func);
	      func->add_PC_info (0, lineno);
	      no_stabs = false;
	    }
	  if (streq (sbase, str))
	    {
	      module->setIncludeFile (NULL);
	      snprintf (curr_src, sizeof (curr_src), NTXT ("%s"), module->file_name);
	      includes->new_src_file (module->getMainSrc (), lineno, func);
	    }
	  else
	    {
	      if (streq (sbase, get_basename (str)))
		{
		  module->setIncludeFile (NULL);
		  snprintf (curr_src, sizeof (curr_src), NTXT ("%s"), module->file_name);
		  includes->new_src_file (module->setIncludeFile (curr_src), lineno, func);
		}
	      else
		{
		  if (*str == '/')
		    snprintf (curr_src, sizeof (curr_src), NTXT ("%s"), str);
		  else
		    {
		      size_t last = strlen (n_so);
		      if (last == 0 || n_so[last - 1] != '/')
			snprintf (curr_src, sizeof (curr_src), NTXT ("%s/%s"), n_so, str);
		      else
			snprintf (curr_src, sizeof (curr_src), NTXT ("%s%s"), n_so, str);
		    }
		  includes->new_src_file (module->setIncludeFile (curr_src), lineno, func);
		}
	    }
	  break;
	}
    }
  delete includes;
  delete stabReader;
  return no_stabs ? DBGD_ERR_NO_STABS : DBGD_ERR_NONE;
}//srcline_Stabs

static bool
cmp_func_name (char *fname, size_t len, char *name, bool fortran)
{
  return (strncmp (name, fname, len) == 0
	  && (name[len] == 0
	      || (fortran && name[len] == '_' && name[len + 1] == 0)));
}

Function *
Stabs::find_func (char *fname, Vector<Function*> *functions, bool fortran, bool inner_names)
{
  char *arg, *name;
  Function *item;
  int index;
  size_t len;

  len = strlen (fname);
  arg = strchr (fname, ':');
  if (arg != NULL)
    {
      if (arg[1] == 'P') // Prototype for function
	return NULL;
      len -= strlen (arg);
    }

  Vec_loop (Function*, functions, index, item)
  {
    name = item->get_mangled_name ();
    if (cmp_func_name (fname, len, name, fortran))
      return item->cardinal ();
  }

  if (inner_names)
    {
      // Dwarf subprograms may only have plain (non-linker) names
      // Retry with inner names only

      Vec_loop (Function*, functions, index, item)
      {
	name = strrchr (item->get_mangled_name (), '.');
	if (!name) continue;
	name++;
	if (cmp_func_name (fname, len, name, fortran))
	  return item->cardinal ();
      }
    }
  return NULL;
}

Map<const char*, Symbol*> *
Stabs::get_elf_symbols ()
{
  Elf *elf = openElf (false);
  if (elf->elfSymbols == NULL)
    {
      Map<const char*, Symbol*> *elfSymbols = new StringMap<Symbol*>(128, 128);
      elf->elfSymbols = elfSymbols;
      for (int i = 0, sz = SymLst ? SymLst->size () : 0; i < sz; i++)
	{
	  Symbol *sym = SymLst->fetch (i);
	  elfSymbols->put (sym->name, sym);
	}
    }
  return elf->elfSymbols;
}

void
Stabs::read_dwarf_from_dot_o (Module *mod)
{
  Dprintf (DEBUG_STABS, NTXT ("stabsModules: %s\n"), STR (mod->get_name ()));
  Vector<Module*> *mods = mod->dot_o_file->seg_modules;
  char *bname = get_basename (mod->get_name ());
  for (int i1 = 0, sz1 = mods ? mods->size () : 0; i1 < sz1; i1++)
    {
      Module *m = mods->fetch (i1);
      Dprintf (DEBUG_STABS, NTXT ("  MOD: %s\n"), STR (m->get_name ()));
      if (dbe_strcmp (bname, get_basename (m->get_name ())) == 0)
	{
	  mod->indexStabsLink = m;
	  m->indexStabsLink = mod;
	  break;
	}
    }
  if (mod->indexStabsLink)
    {
      mod->dot_o_file->objStabs->openDwarf ()->srcline_Dwarf (mod->indexStabsLink);
      Map<const char*, Symbol*> *elfSymbols = get_elf_symbols ();
      Vector<Function*> *funcs = mod->indexStabsLink->functions;
      for (int i1 = 0, sz1 = funcs ? funcs->size () : 0; i1 < sz1; i1++)
	{
	  Function *f1 = funcs->fetch (i1);
	  Symbol *sym = elfSymbols->get (f1->get_mangled_name ());
	  if (sym == NULL)
	    continue;
	  Dprintf (DEBUG_STABS, NTXT ("  Symbol: %s func=%p\n"), STR (sym->name), sym->func);
	  Function *f = sym->func;
	  if (f->indexStabsLink)
	    continue;
	  f->indexStabsLink = f1;
	  f1->indexStabsLink = f;
	  f->copy_PCInfo (f1);
	}
    }
}

Stabs::Stab_status
Stabs::read_archive (LoadObject *lo)
{
  if (openElf (true) == NULL)
    return status;
  check_Symtab ();
  if (elfDbg->dwarf)
    openDwarf ()->archive_Dwarf (lo);

  // get Module/Function lists from stabs info
  Stab_status statusStabs = DBGD_ERR_NO_STABS;
#define ARCHIVE_STABS(sec, secStr, comdat) \
    if ((elfDbg->sec) != 0  && (elfDbg->secStr) != 0 && \
	archive_Stabs(lo, elfDbg->sec, elfDbg->secStr, comdat) == DBGD_ERR_NONE) \
	statusStabs = DBGD_ERR_NONE

  // prefer index stabs (where they exist) since they're most appropriate
  // for loadobjects and might have N_CPROF stabs for ABS/CPF
  ARCHIVE_STABS (stabIndex, stabIndexStr, true);
  ARCHIVE_STABS (stabExcl, stabExclStr, false);
  ARCHIVE_STABS (stab, stabStr, false);

  // Add all unassigned functions to the <unknown> module
  Symbol *sitem, *alias;
  int index;
  Vec_loop (Symbol*, SymLst, index, sitem)
  {
    if (sitem->func || (sitem->size == 0) || (sitem->flags & SYM_UNDEF))
      continue;
    alias = sitem->alias;
    if (alias)
      {
	if (alias->func == NULL)
	  {
	    alias->func = createFunction (lo, lo->noname, alias);
	    alias->func->alias = alias->func;
	  }
	if (alias != sitem)
	  {
	    sitem->func = createFunction (lo, alias->func->module, sitem);
	    sitem->func->alias = alias->func;
	  }
      }
    else
      sitem->func = createFunction (lo, lo->noname, sitem);
  }
  if (pltSym)
    {
      pltSym->func = createFunction (lo, lo->noname, pltSym);
      pltSym->func->flags |= FUNC_FLAG_PLT;
    }

  // need Module association, so this must be done after handling Modules
  check_AnalyzerInfo ();

  if (dwarf && dwarf->status == DBGD_ERR_NONE)
    return DBGD_ERR_NONE;
  return statusStabs;
}//read_archive

Function *
Stabs::createFunction (LoadObject *lo, Module *module, Symbol *sym)
{
  Function *func = dbeSession->createFunction ();
  func->module = module;
  func->img_fname = path;
  func->img_offset = (off_t) sym->img_offset;
  func->save_addr = sym->save;
  func->size = (uint32_t) sym->size;
  func->set_name (sym->name);
  func->elfSym = sym;
  module->functions->append (func);
  lo->functions->append (func);
  return func;
}

void
Stabs::fixSymtabAlias ()
{
  int ind, i, k;
  Symbol *sym, *bestAlias;
  SymLst->sort (SymImgOffsetCmp);
  ind = SymLst->size () - 1;
  for (i = 0; i < ind; i++)
    {
      bestAlias = SymLst->fetch (i);
      if (bestAlias->img_offset == 0) // Ignore this bad symbol
	continue;
      sym = SymLst->fetch (i + 1);
      if (bestAlias->img_offset != sym->img_offset)
	{
	  if ((bestAlias->size == 0) ||
	      (sym->img_offset < bestAlias->img_offset + bestAlias->size))
	    bestAlias->size = sym->img_offset - bestAlias->img_offset;
	  continue;
	}

      // Find a "best" alias
      size_t bestLen = strlen (bestAlias->name);
      int64_t maxSize = bestAlias->size;
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
	  size_t len = strlen (sym->name);
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
Stabs::check_Symtab ()
{
  if (st_check_symtab)
    return;
  st_check_symtab = true;

  Elf *elf = openElf (true);
  if (elf == NULL)
    return;
  if (elfDis->plt != 0)
    {
      Elf_Internal_Shdr *shdr = elfDis->get_shdr (elfDis->plt);
      if (shdr)
	{
	  pltSym = new Symbol ();
	  pltSym->value = shdr->sh_addr;
	  pltSym->size = shdr->sh_size;
	  pltSym->img_offset = shdr->sh_offset;
	  pltSym->name = dbe_strdup (NTXT ("@plt"));
	  pltSym->flags |= SYM_PLT;
	}
    }
  if (elf->symtab)
    readSymSec (elf->symtab, elf);
  else
    {
      readSymSec (elf->SUNW_ldynsym, elf);
      readSymSec (elf->dynsym, elf);
    }
}

void
Stabs::readSymSec (unsigned int sec, Elf *elf)
{
  Symbol *sitem;
  Sp_lang_code local_lcode;
  if (sec == 0)
    return;
  // Get ELF data
  Elf_Data *data = elf->elf_getdata (sec);
  if (data == NULL)
    return;
  uint64_t SymtabSize = data->d_size;
  Elf_Internal_Shdr *shdr = elf->get_shdr (sec);

  if ((SymtabSize == 0) || (shdr->sh_entsize == 0))
    return;
  Elf_Data *data_str = elf->elf_getdata (shdr->sh_link);
  if (data_str == NULL)
    return;
  char *Strtab = (char *) data_str->d_buf;

  // read func symbolic table
  for (unsigned int n = 0, tot = SymtabSize / shdr->sh_entsize; n < tot; n++)
    {
      Elf_Internal_Sym Sym;
      elf->elf_getsym (data, n, &Sym);
      const char *st_name = Sym.st_name < data_str->d_size ?
	  (Strtab + Sym.st_name) : NTXT ("no_name");
      switch (GELF_ST_TYPE (Sym.st_info))
	{
	case STT_FUNC:
	  // Skip UNDEF symbols (bug 4817083)
	  if (Sym.st_shndx == 0)
	    {
	      if (Sym.st_value == 0)
		break;
	      sitem = new Symbol (SymLst);
	      sitem->flags |= SYM_UNDEF;
	      if (pltSym)
		sitem->img_offset = (uint32_t) (pltSym->img_offset +
						Sym.st_value - pltSym->value);
	    }
	  else
	    {
	      Elf_Internal_Shdr *shdrp = elfDis->get_shdr (Sym.st_shndx);
	      if (shdrp == NULL)
		break;
	      sitem = new Symbol (SymLst);
	      sitem->img_offset = (uint32_t) (shdrp->sh_offset +
					      Sym.st_value - shdrp->sh_addr);
	    }
	  sitem->size = Sym.st_size;
	  sitem->name = dbe_strdup (st_name);
	  sitem->value = is_relocatable () ? sitem->img_offset : Sym.st_value;
	  if (GELF_ST_BIND (Sym.st_info) == STB_LOCAL)
	    {
	      sitem->local_ind = LocalFile->size () - 1;
	      LocalLst->append (sitem);
	    }
	  break;
	case STT_NOTYPE:
	  if (streq (st_name, NTXT ("gcc2_compiled.")))
	    {
	      sitem = new Symbol (SymLst);
	      sitem->lang_code = Sp_lang_gcc;
	      sitem->name = dbe_strdup (st_name);
	      sitem->local_ind = LocalFile->size () - 1;
	      LocalLst->append (sitem);
	    }
	  break;
	case STT_OBJECT:
	  if (!strncmp (st_name, NTXT ("__KAI_KPTS_"), 11))
	    local_lcode = Sp_lang_KAI_KPTS;
	  else if (!strncmp (st_name, NTXT ("__KAI_KCC_"), 10))
	    local_lcode = Sp_lang_KAI_KCC;
	  else if (!strncmp (st_name, NTXT ("__KAI_Kcc_"), 10))
	    local_lcode = Sp_lang_KAI_Kcc;
	  else
	    break;
	  sitem = new Symbol (LocalLst);
	  sitem->lang_code = local_lcode;
	  sitem->name = dbe_strdup (st_name);
	  break;
	case STT_FILE:
	  {
	    int last = LocalFile->size () - 1;
	    if (last >= 0 && LocalFileIdx->fetch (last) == LocalLst->size ())
	      {
		// There were no local functions in the latest file.
		free (LocalFile->get (last));
		LocalFile->store (last, dbe_strdup (st_name));
	      }
	    else
	      {
		LocalFile->append (dbe_strdup (st_name));
		LocalFileIdx->append (LocalLst->size ());
	      }
	    break;
	  }
	}
    }
  fixSymtabAlias ();
  SymLst->sort (SymValueCmp);
  get_save_addr (elf->need_swap_endian);
  dump ();
}//check_Symtab

void
Stabs::check_Relocs ()
{
  // We may have many relocation tables to process: .rela.text%foo,
  // rela.text%bar, etc. On Intel, compilers generate .rel.text sections
  // which have to be processed as well. A lot of rework is needed here.
  Symbol *sptr = NULL;
  if (st_check_relocs)
    return;
  st_check_relocs = true;

  Elf *elf = openElf (false);
  if (elf == NULL)
    return;
  for (unsigned int sec = 1; sec < elf->elf_getehdr ()->e_shnum; sec++)
    {
      bool use_rela, use_PLT;
      char *name = elf->get_sec_name (sec);
      if (name == NULL)
	continue;
      if (strncmp (name, NTXT (".rela.text"), 10) == 0)
	{
	  use_rela = true;
	  use_PLT = false;
	}
      else if (streq (name, NTXT (".rela.plt")))
	{
	  use_rela = true;
	  use_PLT = true;
	}
      else if (strncmp (name, NTXT (".rel.text"), 9) == 0)
	{
	  use_rela = false;
	  use_PLT = false;
	}
      else if (streq (name, NTXT (".rel.plt")))
	{
	  use_rela = false;
	  use_PLT = true;
	}
      else
	continue;

      Elf_Internal_Shdr *shdr = elf->get_shdr (sec);
      if (shdr == NULL)
	continue;

      // Get ELF data
      Elf_Data *data = elf->elf_getdata (sec);
      if (data == NULL)
	continue;
      uint64_t ScnSize = data->d_size;
      uint64_t EntSize = shdr->sh_entsize;
      if ((ScnSize == 0) || (EntSize == 0))
	continue;
      int tot = (int) (ScnSize / EntSize);

      // Get corresponding text section
      Elf_Internal_Shdr *shdr_txt = elf->get_shdr (shdr->sh_info);
      if (shdr_txt == NULL)
	continue;
      if (!(shdr_txt->sh_flags & SHF_EXECINSTR))
	continue;

      // Get corresponding symbol table section
      Elf_Internal_Shdr *shdr_sym = elf->get_shdr (shdr->sh_link);
      if (shdr_sym == NULL)
	continue;
      Elf_Data *data_sym = elf->elf_getdata (shdr->sh_link);

      // Get corresponding string table section
      Elf_Data *data_str = elf->elf_getdata (shdr_sym->sh_link);
      if (data_str == NULL)
	continue;
      char *Strtab = (char*) data_str->d_buf;
      for (int n = 0; n < tot; n++)
	{
	  Elf_Internal_Sym sym;
	  Elf_Internal_Rela rela;
	  char *symName;
	  if (use_rela)
	    elf->elf_getrela (data, n, &rela);
	  else
	    {
	      // GElf_Rela is extended GElf_Rel
	      elf->elf_getrel (data, n, &rela);
	      rela.r_addend = 0;
	    }

	  int ndx = (int) GELF_R_SYM (rela.r_info);
	  elf->elf_getsym (data_sym, ndx, &sym);
	  switch (GELF_ST_TYPE (sym.st_info))
	    {
	    case STT_FUNC:
	    case STT_OBJECT:
	    case STT_NOTYPE:
	      if (sym.st_name == 0 || sym.st_name >= data_str->d_size)
		continue;
	      symName = Strtab + sym.st_name;
	      break;
	    case STT_SECTION:
	      {
		Elf_Internal_Shdr *secHdr = elf->get_shdr (sym.st_shndx);
		if (secHdr == NULL)
		  continue;
		if (sptr == NULL)
		  sptr = new Symbol;
		sptr->value = secHdr->sh_offset + rela.r_addend;
		long index = SymLst->bisearch (0, -1, &sptr, SymFindCmp);
		if (index == -1)
		  continue;
		Symbol *sp = SymLst->fetch (index);
		if (sptr->value != sp->value)
		  continue;
		symName = sp->name;
		break;
	      }
	    default:
	      continue;
	    }
	  Reloc *reloc = new Reloc;
	  reloc->name = dbe_strdup (symName);
	  reloc->type = GELF_R_TYPE (rela.r_info);
	  reloc->value = use_PLT ? rela.r_offset
		  : rela.r_offset + shdr_txt->sh_offset;
	  reloc->addend = rela.r_addend;
	  if (use_PLT)
	    RelPLTLst->append (reloc);
	  else
	    RelLst->append (reloc);
	}
    }
  delete sptr;
  RelLst->sort (RelValueCmp);
} //check_Relocs

void
Stabs::get_save_addr (bool need_swap_endian)
{
  if (elfDis->is_Intel ())
    {
      for (int j = 0, sz = SymLst ? SymLst->size () : 0; j < sz; j++)
	{
	  Symbol *sitem = SymLst->fetch (j);
	  sitem->save = 0;
	}
      return;
    }
  for (int j = 0, sz = SymLst ? SymLst->size () : 0; j < sz; j++)
    {
      Symbol *sitem = SymLst->fetch (j);
      sitem->save = FUNC_NO_SAVE;

      // If an image offset is not known skip it.
      // Works for artificial symbols like '@plt' as well.
      if (sitem->img_offset == 0)
	continue;

      bool is_o7_moved = false;
      int64_t off = sitem->img_offset;
      for (int i = 0; i < sitem->size; i += 4)
	{
	  unsigned int cmd;
	  if (elfDis->get_data (off, sizeof (cmd), &cmd) == NULL)
	    break;
	  if (need_swap_endian)
	    SWAP_ENDIAN (cmd);
	  off += sizeof (cmd);
	  if ((cmd & 0xffffc000) == 0x9de38000)
	    { // save %sp, ??, %sp
	      sitem->save = i;
	      break;
	    }
	  else if ((cmd & 0xc0000000) == 0x40000000 || // call ??
	 (cmd & 0xfff80000) == 0xbfc00000)
	    { // jmpl ??, %o7
	      if (!is_o7_moved)
		{
		  sitem->save = FUNC_ROOT;
		  break;
		}
	    }
	  else if ((cmd & 0xc1ffe01f) == 0x8010000f)    // or %g0,%o7,??
	    is_o7_moved = true;
	}
    }
}

uint64_t
Stabs::mapOffsetToAddress (uint64_t img_offset)
{
  Elf *elf = openElf (false);
  if (elf == NULL)
    return 0;
  if (is_relocatable ())
    return img_offset;
  for (unsigned int sec = 1; sec < elf->elf_getehdr ()->e_shnum; sec++)
    {
      Elf_Internal_Shdr *shdr = elf->get_shdr (sec);
      if (shdr == NULL)
	continue;
      if (img_offset >= (uint64_t) shdr->sh_offset
	  && img_offset < (uint64_t) (shdr->sh_offset + shdr->sh_size))
	return shdr->sh_addr + (img_offset - shdr->sh_offset);
    }
  return 0;
}

Stabs::Stab_status
Stabs::archive_Stabs (LoadObject *lo, unsigned int StabSec,
		      unsigned int StabStrSec, bool comdat)
{
  StabReader *stabReader = new StabReader (openElf (true), platform, StabSec, StabStrSec);
  int tot = stabReader->stabCnt;
  if (tot < 0)
    {
      delete stabReader;
      return DBGD_ERR_NO_STABS;
    }

  char *sbase = NTXT (""), *arg, *fname, sname[2 * MAXPATHLEN];
  int lastMod, phase, stabs_level, modCnt = 0;
  Function *func = NULL;
  Module *mod;
#define INIT_MOD    phase = 0; stabs_level = 0; *sname = '\0'; mod = NULL

  bool updateStabsMod = false;
  if (comdat && ((elfDbg->elf_getehdr ()->e_type == ET_EXEC) || (elfDbg->elf_getehdr ()->e_type == ET_DYN)))
    {
      if (stabsModules == NULL)
	stabsModules = new Vector<Module*>();
      updateStabsMod = true;
    }
  INIT_MOD;
  lastMod = lo->seg_modules->size ();

  for (int n = 0; n < tot; n++)
    {
      struct stab stb;
      char *str = stabReader->get_stab (&stb, comdat);
      switch (stb.n_type)
	{
	case N_FUN:
	  // Ignore a COMDAT function, if there are two or more modules in 'lo'
	  if (comdat && stb.n_other == 1 && modCnt > 1)
	    break;
	case N_OUTL:
	case N_ALIAS:
	case N_ENTRY:
	  if (mod == NULL || str == NULL
	      || (stb.n_type != N_ENTRY && stabs_level != 0))
	    break;
	  if (*str == '@')
	    {
	      str++;
	      if (*str == '>' || *str == '<')
		str++;
	    }

	  fname = dbe_strdup (str);
	  arg = strchr (fname, ':');
	  if (arg != NULL)
	    {
	      if (!strncmp (arg, NTXT (":P"), 2))
		{ // just prototype
		  free (fname);
		  break;
		}
	      *arg = '\0';
	    }

	  func = append_Function (mod, fname);
	  free (fname);
	  break;
	case N_CMDLINE:
	  if (str && mod)
	    {
	      char *comp_flags = strchr (str, ';');
	      if (comp_flags)
		{
		  mod->comp_flags = dbe_strdup (comp_flags + 1);
		  mod->comp_dir = dbe_strndup (str, comp_flags - str);
		}
	    }
	  break;
	case N_LBRAC:
	  stabs_level++;
	  break;
	case N_RBRAC:
	  stabs_level--;
	  break;
	case N_UNDF:
	  INIT_MOD;
	  break;
	case N_ENDM:
	  INIT_MOD;
	  break;
	case N_OPT:
	  stabReader->parse_N_OPT (mod, str);
	  if (mod && (str != NULL) && streq (str, NTXT ("gcc2_compiled.")))
	    // Is it anachronism ?
	    mod->lang_code = Sp_lang_gcc;
	  break;
	case N_GSYM:
	  if (mod && (str != NULL))
	    {
	      if (strncmp (str, NTXT ("__KAI_K"), 7))
		break;
	      str += 7;
	      if (!strncmp (str, NTXT ("CC_"), 3))
		mod->lang_code = Sp_lang_KAI_KCC;
	      else if (!strncmp (str, NTXT ("cc_"), 3))
		mod->lang_code = Sp_lang_KAI_Kcc;
	      else if (!strncmp (str, NTXT ("PTS_"), 4) &&
		       (mod->lang_code != Sp_lang_KAI_KCC) &&
		       (mod->lang_code != Sp_lang_KAI_Kcc))
		mod->lang_code = Sp_lang_KAI_KPTS;
	    }
	  break;
	case N_SO:
	  if (str == NULL || *str == '\0')
	    {
	      INIT_MOD;
	      break;
	    }
	  if (phase == 0)
	    {
	      phase = 1;
	      sbase = str;
	    }
	  else
	    {
	      if (*str == '/')
		sbase = str;
	      else
		{
		  size_t last = strlen (sbase);
		  if (last == 0 || sbase[last - 1] != '/')
		    snprintf (sname, sizeof (sname), NTXT ("%s/%s"), sbase, str);
		  else
		    snprintf (sname, sizeof (sname), NTXT ("%s%s"), sbase, str);
		  sbase = sname;
		}
	      mod = append_Module (lo, sbase, lastMod);
	      if (updateStabsMod)
		stabsModules->append (mod);
	      mod->hasStabs = true;
	      modCnt++;
	      if ((mod->lang_code != Sp_lang_gcc) &&
		  (mod->lang_code != Sp_lang_KAI_KPTS) &&
		  (mod->lang_code != Sp_lang_KAI_KCC) &&
		  (mod->lang_code != Sp_lang_KAI_Kcc))
		mod->lang_code = (Sp_lang_code) stb.n_desc;
	      *sname = '\0';
	      phase = 0;
	    }
	  break;
	case N_OBJ:
	  if (str == NULL)
	    break;
	  if (phase == 0)
	    {
	      phase = 1;
	      sbase = str;
	    }
	  else
	    {
	      if (*str == '/')
		sbase = str;
	      else
		{
		  size_t last = strlen (sbase);
		  if (last == 0 || sbase[last - 1] != '/')
		    snprintf (sname, sizeof (sname), NTXT ("%s/%s"), sbase, str);
		  else
		    snprintf (sname, sizeof (sname), NTXT ("%s%s"), sbase, str);
		  sbase = sname;
		}
	      if (mod && (mod->dot_o_file == NULL))
		{
		  if (strcmp (sbase, NTXT ("/")) == 0)
		    mod->set_name (dbe_strdup (path));
		  else
		    {
		      mod->set_name (dbe_strdup (sbase));
		      mod->dot_o_file = mod->createLoadObject (sbase);
		    }
		}
	      *sname = '\0';
	      phase = 0;
	    }
	  break;
	case N_CPROF:
	  cpf_stabs_t map;
	  Dprintf (DEBUG_STABS, NTXT ("N_CPROF n_desc=%x n_value=0x%04x mod=%s\n"),
		   stb.n_desc, stb.n_value, (mod == NULL) ? NTXT ("???") : mod->get_name ());
	  map.type = stb.n_desc;
	  map.offset = stb.n_value;
	  map.module = mod;
	  analyzerInfoMap.append (map);
	  break;
	}
    }
  delete stabReader;
  return func ? DBGD_ERR_NONE : DBGD_ERR_NO_STABS;
}

Module *
Stabs::append_Module (LoadObject *lo, char *name, int lastMod)
{
  Module *module;
  int size;
  Symbol *sitem;

  if (lo->seg_modules != NULL)
    {
      size = lo->seg_modules->size ();
      if (size < lastMod)
	lastMod = size;
      for (int i = 0; i < lastMod; i++)
	{
	  module = lo->seg_modules->fetch (i);
	  if (module->linkerStabName && streq (module->linkerStabName, name))
	    return module;
	}
    }
  module = dbeSession->createModule (lo, NULL);
  module->set_file_name (dbe_strdup (name));
  module->linkerStabName = dbe_strdup (module->file_name);

  // Append all functions with 'local_ind == -1' to the module.
  if (LocalLst->size () > 0)
    {
      sitem = LocalLst->fetch (0);
      if (!sitem->defined && sitem->local_ind == -1)
	// Append all functions with 'local_ind == -1' to the module.
	append_local_funcs (module, 0);
    }

  // Append local func
  char *basename = get_basename (name);
  size = LocalFile->size ();
  for (int i = 0; i < size; i++)
    {
      if (streq (basename, LocalFile->fetch (i)))
	{
	  int local_ind = LocalFileIdx->fetch (i);
	  if (local_ind >= LocalLst->size ())
	    break;
	  sitem = LocalLst->fetch (local_ind);
	  if (!sitem->defined)
	    {
	      append_local_funcs (module, local_ind);
	      break;
	    }
	}
    }
  return module;
}

void
Stabs::append_local_funcs (Module *module, int first_ind)
{
  Symbol *sitem = LocalLst->fetch (first_ind);
  int local_ind = sitem->local_ind;
  int size = LocalLst->size ();
  for (int i = first_ind; i < size; i++)
    {
      sitem = LocalLst->fetch (i);
      if (sitem->local_ind != local_ind)
	break;
      sitem->defined = true;

      // 3rd party compiled. e.g., Gcc or KAI compiled
      if (sitem->lang_code != Sp_lang_unknown)
	{
	  if (module->lang_code == Sp_lang_unknown)
	    module->lang_code = sitem->lang_code;
	  continue;
	}
      if (sitem->func)
	continue;
      Function *func = dbeSession->createFunction ();
      sitem->func = func;
      func->img_fname = path;
      func->img_offset = (off_t) sitem->img_offset;
      func->save_addr = (uint32_t) sitem->save;
      func->size = (uint32_t) sitem->size;
      func->module = module;
      func->set_name (sitem->name);
      module->functions->append (func);
      module->loadobject->functions->append (func);
    }
}

Function *
Stabs::append_Function (Module *module, char *fname)
{
  Symbol *sitem, *sptr;
  Function *func;
  long sid, index;
  char *name;
  if (SymLstByName == NULL)
    {
      SymLstByName = SymLst->copy ();
      SymLstByName->sort (SymNameCmp);
    }
  sptr = new Symbol;
  if (module->lang_code == N_SO_FORTRAN || module->lang_code == N_SO_FORTRAN90)
    {
      char *fortran = dbe_sprintf (NTXT ("%s_"), fname); // FORTRAN name
      sptr->name = fortran;
      sid = SymLstByName->bisearch (0, -1, &sptr, SymNameCmp);
      if (sid == -1)
	{
	  free (fortran);
	  sptr->name = fname;
	  sid = SymLstByName->bisearch (0, -1, &sptr, SymNameCmp);
	}
      else
	fname = fortran;
    }
  else
    {
      sptr->name = fname;
      sid = SymLstByName->bisearch (0, -1, &sptr, SymNameCmp);
    }
  sptr->name = NULL;
  delete sptr;

  if (sid == -1)
    {
      Vec_loop (Symbol*, SymLstByName, index, sitem)
      {
	if (strncmp (sitem->name, NTXT ("$X"), 2) == 0
	    || strncmp (sitem->name, NTXT (".X"), 2) == 0)
	  {
	    char *n = strchr (((sitem->name) + 2), (int) '.');
	    if (n != NULL)
	      name = n + 1;
	    else
	      name = sitem->name;
	  }
	else
	  name = sitem->name;
	if (name != NULL && fname != NULL && (strcmp (name, fname) == 0))
	  {
	    sid = index;
	    break;
	  }
      }
    }
  if (sid != -1)
    {
      sitem = SymLstByName->fetch (sid);
      if (sitem->alias)
	sitem = sitem->alias;
      if (sitem->func)
	return sitem->func;
      sitem->func = func = dbeSession->createFunction ();
      func->img_fname = path;
      func->img_offset = (off_t) sitem->img_offset;
      func->save_addr = (uint32_t) sitem->save;
      func->size = (uint32_t) sitem->size;
    }
  else
    func = dbeSession->createFunction ();

  func->module = module;
  func->set_name (fname);
  module->functions->append (func);
  module->loadobject->functions->append (func);
  return func;
}

Function *
Stabs::append_Function (Module *module, char *linkerName, uint64_t pc)
{
  Dprintf (DEBUG_STABS, NTXT ("Stabs::append_Function: module=%s linkerName=%s pc=0x%llx\n"),
	   STR (module->get_name ()), STR (linkerName), (unsigned long long) pc);
  long i;
  Symbol *sitem = NULL, *sp;
  Function *func;
  sp = new Symbol;
  if (pc)
    {
      sp->value = pc;
      i = SymLst->bisearch (0, -1, &sp, SymFindCmp);
      if (i != -1)
	sitem = SymLst->fetch (i);
    }

  if (!sitem && linkerName)
    {
      if (SymLstByName == NULL)
	{
	  SymLstByName = SymLst->copy ();
	  SymLstByName->sort (SymNameCmp);
	}
      sp->name = linkerName;
      i = SymLstByName->bisearch (0, -1, &sp, SymNameCmp);
      sp->name = NULL;
      if (i != -1)
	sitem = SymLstByName->fetch (i);
    }
  delete sp;

  if (!sitem)
    return NULL;
  if (sitem->alias)
    sitem = sitem->alias;
  if (sitem->func)
    return sitem->func;

  sitem->func = func = dbeSession->createFunction ();
  func->img_fname = path;
  func->img_offset = (off_t) sitem->img_offset;
  func->save_addr = (uint32_t) sitem->save;
  func->size = (uint32_t) sitem->size;
  func->module = module;
  func->set_name (sitem->name); //XXXX ?? Now call it to set obj->name
  module->functions->append (func);
  module->loadobject->functions->append (func);
  return func;
}// Stabs::append_Function

Dwarf *
Stabs::openDwarf ()
{
  if (dwarf == NULL)
    {
      dwarf = new Dwarf (this);
      check_Symtab ();
    }
  return dwarf;
}

void
Stabs::read_hwcprof_info (Module *module)
{
  openDwarf ()->read_hwcprof_info (module);
}

void
Stabs::dump ()
{
  if (!DUMP_ELF_SYM)
    return;
  printf (NTXT ("\n======= Stabs::dump: %s =========\n"), path ? path : NTXT ("NULL"));
  int i, sz;
  if (LocalFile)
    {
      sz = LocalFile->size ();
      for (i = 0; i < sz; i++)
	printf ("  %3d: %5d '%s'\n", i, LocalFileIdx->fetch (i),
		LocalFile->fetch (i));
    }
  Symbol::dump (SymLst, NTXT ("SymLst"));
  Symbol::dump (LocalLst, NTXT ("LocalLst"));
  printf (NTXT ("\n===== END of Stabs::dump: %s =========\n\n"),
  path ? path : NTXT ("NULL"));
}

///////////////////////////////////////////////////////////////////////////////
//  Class Include
Include::Include ()
{
  stack = new Vector<SrcFileInfo*>;
}

Include::~Include ()
{
  Destroy (stack);
}

void
Include::new_src_file (SourceFile *source, int lineno, Function *func)
{
  for (int index = stack->size () - 1; index >= 0; index--)
    {
      if (source == stack->fetch (index)->srcfile)
	{
	  for (int i = stack->size () - 1; i > index; i--)
	    {
	      delete stack->remove (i);
	      if (func && func->line_first > 0)
		func->popSrcFile ();
	    }
	  return;
	}
    }
  if (func && func->line_first > 0)
    func->pushSrcFile (source, lineno);

  SrcFileInfo *sfinfo = new SrcFileInfo;
  sfinfo->srcfile = source;
  sfinfo->lineno = lineno;
  stack->append (sfinfo);
}

void
Include::push_src_files (Function *func)
{
  int index;
  SrcFileInfo *sfinfo;

  if (func->line_first <= 0 && stack->size () > 0)
    {
      sfinfo = stack->fetch (stack->size () - 1);
      func->setDefSrc (sfinfo->srcfile);
    }
  Vec_loop (SrcFileInfo*, stack, index, sfinfo)
  {
    func->pushSrcFile (sfinfo->srcfile, sfinfo->lineno);
  }
}

void
Include::new_include_file (SourceFile *source, Function *func)
{
  if (stack->size () == 1 && stack->fetch (0)->srcfile == source)
    // workaroud for gcc; gcc creates 'N_BINCL' stab for main source
    return;
  if (func && func->line_first > 0)
    func->pushSrcFile (source, 0);

  SrcFileInfo *sfinfo = new SrcFileInfo;
  sfinfo->srcfile = source;
  sfinfo->lineno = 0;
  stack->append (sfinfo);
}

void
Include::end_include_file (Function *func)
{
  int index = stack->size () - 1;
  if (index > 0)
    {
      delete stack->remove (index);
      if (func && func->line_first > 0)
	func->popSrcFile ();
    }
}

#define RET_S(x)   if (t == x) return (char *) #x
char *
StabReader::get_type_name (int t)
{
  RET_S (N_UNDF);
  RET_S (N_ABS);
  RET_S (N_TEXT);
  RET_S (N_DATA);
  RET_S (N_BSS);
  RET_S (N_COMM);
  RET_S (N_FN);
  RET_S (N_EXT);
  RET_S (N_TYPE);
  RET_S (N_GSYM);
  RET_S (N_FNAME);
  RET_S (N_FUN);
  RET_S (N_OUTL);
  RET_S (N_STSYM);
  RET_S (N_TSTSYM);
  RET_S (N_LCSYM);
  RET_S (N_TLCSYM);
  RET_S (N_MAIN);
  RET_S (N_ROSYM);
  RET_S (N_FLSYM);
  RET_S (N_TFLSYM);
  RET_S (N_PC);
  RET_S (N_CMDLINE);
  RET_S (N_OBJ);
  RET_S (N_OPT);
  RET_S (N_RSYM);
  RET_S (N_SLINE);
  RET_S (N_XLINE);
  RET_S (N_ILDPAD);
  RET_S (N_SSYM);
  RET_S (N_ENDM);
  RET_S (N_SO);
  RET_S (N_MOD);
  RET_S (N_EMOD);
  RET_S (N_READ_MOD);
  RET_S (N_ALIAS);
  RET_S (N_LSYM);
  RET_S (N_BINCL);
  RET_S (N_SOL);
  RET_S (N_PSYM);
  RET_S (N_EINCL);
  RET_S (N_ENTRY);
  RET_S (N_SINCL);
  RET_S (N_LBRAC);
  RET_S (N_EXCL);
  RET_S (N_USING);
  RET_S (N_ISYM);
  RET_S (N_ESYM);
  RET_S (N_PATCH);
  RET_S (N_CONSTRUCT);
  RET_S (N_DESTRUCT);
  RET_S (N_CODETAG);
  RET_S (N_FUN_CHILD);
  RET_S (N_RBRAC);
  RET_S (N_BCOMM);
  RET_S (N_TCOMM);
  RET_S (N_ECOMM);
  RET_S (N_XCOMM);
  RET_S (N_ECOML);
  RET_S (N_WITH);
  RET_S (N_LENG);
  RET_S (N_CPROF);
  RET_S (N_BROWS);
  RET_S (N_FUN_PURE);
  RET_S (N_FUN_ELEMENTAL);
  RET_S (N_FUN_RECURSIVE);
  RET_S (N_FUN_AMD64_PARMDUMP);
  RET_S (N_SYM_OMP_TLS);
  RET_S (N_SO_AS);
  RET_S (N_SO_C);
  RET_S (N_SO_ANSI_C);
  RET_S (N_SO_CC);
  RET_S (N_SO_FORTRAN);
  RET_S (N_SO_FORTRAN77);
  RET_S (N_SO_PASCAL);
  RET_S (N_SO_FORTRAN90);
  RET_S (N_SO_JAVA);
  RET_S (N_SO_C99);
  return NULL;
}
