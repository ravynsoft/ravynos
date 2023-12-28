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
#include "util.h"
#include "DbeSession.h"
#include "Elf.h"
#include "Stabs.h"
#include "Dwarf.h"
#include "DataObject.h"
#include "Function.h"
#include "LoadObject.h"
#include "Module.h"
#include "DefaultMap.h"

static int
datatypeCmp (const void *a, const void *b)
{
  uint32_t o1 = ((datatype_t *) a)->datatype_id;
  uint32_t o2 = ((datatype_t *) b)->datatype_id;
  return o1 == o2 ? 0 : (o1 < o2 ? -1 : 1);
}

static int
targetOffsetCmp (const void *a, const void *b)
{
  uint32_t o1 = ((target_info_t *) a)->offset;
  uint32_t o2 = ((target_info_t *) b)->offset;
  return o1 == o2 ? 0 : (o1 < o2 ? -1 : 1);
}


//////////////////////////////////////////////////////////
//  class Dwr_type
class Dwr_type
{
public:

  Dwr_type (int64_t _cu_die_offset, int _tag)
  {
    cu_die_offset = _cu_die_offset;
    tag = _tag;
    name = NULL;
    dobj_name = NULL;
    dtype = NULL;
    extent = 0;
    parent = 0;
    child = 0;
    next = 0;
    ref_type = 0;
    size = 0;
    elems = 0;
    offset = -1;
    bit_size = 0;
  };

  char *name, *dobj_name;
  int64_t cu_die_offset, ref_type, extent, parent, child, next;
  int64_t size, elems, offset;
  int tag, bit_size;

  DataObject *get_dobj (Dwarf_cnt *ctx);
  char *get_dobjname (Dwarf_cnt *ctx);
  char *dump ();

private:
  datatype_t *dtype;
  datatype_t *get_datatype (Dwarf_cnt *ctx);
  void get_dobj_for_members (Dwarf_cnt *ctx);
  void set_dobjname (char *spec, char *nm);
};


//////////////////////////////////////////////////////////
//  class Dwarf_cnt
Dwarf_cnt::Dwarf_cnt ()
{
  cu_offset = 0;
  parent = 0;
  module = NULL;
  name = NULL;
  func = NULL;
  fortranMAIN = NULL;
  dwr_types = NULL;
  inlinedSubr = NULL;
  level = 0;
}

Dwr_type *
Dwarf_cnt::get_dwr_type (int64_t cu_die_offset)
{
  Dwr_type *t = dwr_types->get (cu_die_offset);
  if (t == NULL)
    {
      Dprintf (DUMP_DWARFLIB, "DWARF_ERROR: %s:%d wrong cu_die_offset=%lld in Dwarf_cnt::get_dwr_type\n",
	       get_basename (__FILE__), (int) __LINE__,
	       (long long) cu_die_offset);
      t = put_dwr_type (cu_die_offset, 0); // DOBJ_UNSPECIFIED
    }
  return t;
}

Dwr_type *
Dwarf_cnt::put_dwr_type (int64_t cu_die_offset, int tag)
{
  Dwr_type *t = new Dwr_type (cu_die_offset, tag);
  dwr_types->put (cu_die_offset, t);
  return t;
}

Dwr_type *
Dwarf_cnt::put_dwr_type (Dwr_Tag *dwrTag)
{
  Dwr_type *t = new Dwr_type (dwrTag->die, dwrTag->tag);
  dwr_types->put (dwrTag->die, t);
  return t;
}

//////////////////////////////////////////////////////////
//  class Dwr_type
char *
Dwr_type::dump ()
{
  char *s = dbe_sprintf ("%lld %-15s name='%s' parent=%lld next=%lld child=%lld dtype=%llx",
			 (long long) cu_die_offset, DwrCU::tag2str (tag),
			 STR (name), (long long) parent, (long long) next,
			 (long long) child, (long long) dtype);
  return s;
}

void
Dwr_type::set_dobjname (char *spec, char *nm)
{
  if (spec)
    {
      if (nm)
	dobj_name = dbe_sprintf ("%s%s", spec, nm);
      else
	dobj_name = dbe_sprintf ("%s<ANON=%lld>", spec,
				 (long long) cu_die_offset);
    }
  else
    {
      if (nm)
	dobj_name = dbe_sprintf ("%s", nm);
      else
	dobj_name = dbe_sprintf ("<ANON=%lld>", (long long) cu_die_offset);
    }
}

char *
Dwr_type::get_dobjname (Dwarf_cnt *ctx)
{
  if (dobj_name)
    return dobj_name;
  switch (tag)
    {
    case DW_TAG_base_type:
      set_dobjname (NULL, name);
      for (int i = 0, len = (int) strlen (dobj_name); i < len; i++)
	{
	  if (dobj_name[i] == ' ')
	    dobj_name[i] = '_';
	}
      break;
    case DW_TAG_constant:
    case DW_TAG_formal_parameter:
    case DW_TAG_variable:
      {
	Dwr_type *t = ctx->get_dwr_type (ref_type);
	set_dobjname (NULL, t->get_dobjname (ctx));
	break;
      }
    case DW_TAG_unspecified_type:
      set_dobjname (NTXT ("unspecified:"), name);
      break;
    case DW_TAG_enumeration_type:
      set_dobjname (NTXT ("enumeration:"), name);
      break;
    case DW_TAG_typedef:
      {
	Dwr_type *t = ctx->get_dwr_type (ref_type);
	dobj_name = dbe_sprintf ("%s=%s", name, t->get_dobjname (ctx));
	break;
      }
    case DW_TAG_const_type:
      set_dobjname (NTXT ("const+"), name);
      break;
    case DW_TAG_volatile_type:
      set_dobjname (NTXT ("volatile+"), name);
      break;
    case DW_TAG_pointer_type:
      {
	Dwr_type *t = ctx->get_dwr_type (ref_type);
	set_dobjname (NTXT ("pointer+"), t->get_dobjname (ctx));
	break;
      }
    case DW_TAG_reference_type:
      {
	Dwr_type *t = ctx->get_dwr_type (ref_type);
	set_dobjname (NTXT ("reference+"), t->get_dobjname (ctx));
	break;
      }
    case DW_TAG_array_type:
      {
	Dwr_type *t = ctx->get_dwr_type (ref_type);
	if (elems > 0)
	  dobj_name = dbe_sprintf ("array[%lld]:%s",
				   (long long) elems, t->get_dobjname (ctx));
	else
	  dobj_name = dbe_sprintf ("array[]:%s", t->get_dobjname (ctx));
	break;
      }
    case DW_TAG_structure_type:
      set_dobjname (NTXT ("structure:"), name);
      break;
    case DW_TAG_union_type:
      set_dobjname (NTXT ("union:"), name);
      break;
    case DW_TAG_class_type:
      set_dobjname (NTXT ("class:"), name);
      break;
    case DW_TAG_member:
      {
	Dwr_type *t = ctx->get_dwr_type (ref_type);
	if (bit_size > 0)
	  dobj_name = dbe_sprintf (NTXT ("%s:%lld"), t->get_dobjname (ctx),
				   (long long) bit_size);
	else
	  dobj_name = dbe_sprintf (NTXT ("%s"), t->get_dobjname (ctx));
	break;
      }
    default:
      Dprintf (DUMP_DWARFLIB, NTXT ("DWARF_ERROR: %s:%d No case for %s cu_die_offset=%lld\n"),
	       get_basename (__FILE__), (int) __LINE__,
	       DwrCU::tag2str (tag), (long long) cu_die_offset);
      set_dobjname (NTXT ("Undefined:"), NULL);
      break;
    }
  return dobj_name;
}

datatype_t*
Dwr_type::get_datatype (Dwarf_cnt *ctx)
{
  if (dtype)
    return dtype;
  dtype = new datatype_t;
  dtype->datatype_id = (unsigned) cu_die_offset;
  dtype->memop_refs = 0;
  dtype->event_data = 0;
  dtype->dobj = NULL;
  ctx->module->datatypes->incorporate (dtype, datatypeCmp);
  return dtype;
}

DataObject *
Dwr_type::get_dobj (Dwarf_cnt *ctx)
{
  if (dtype == NULL)
    dtype = get_datatype (ctx);
  dtype->memop_refs++;
  DataObject *dobj = dtype->dobj;
  if (dobj)
    return dobj;

  if (tag == 0)
    dobj = dbeSession->find_dobj_by_name (PTXT (DOBJ_UNSPECIFIED));
  else
    {
      dobj = dbeSession->createDataObject ();
      dobj->size = size;
      dobj->offset = offset;
      dobj->scope = ctx->func ? (Histable*) ctx->func : (Histable*) ctx->module;
    }
  dtype->dobj = dobj;
  if (parent)
    {
      Dwr_type *t = ctx->get_dwr_type (parent);
      dobj->parent = t->get_dobj (ctx);
    }

  if (ref_type)
    {
      Dwr_type *t = ctx->get_dwr_type (ref_type);
      t->get_dobj (ctx);
      if (size == 0)
	{
	  size = t->size;
	  dobj->size = size;
	}
    }

  switch (tag)
    {
    case 0:
      break;
    case DW_TAG_array_type:
    case DW_TAG_base_type:
    case DW_TAG_unspecified_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_typedef:
    case DW_TAG_const_type:
    case DW_TAG_volatile_type:
    case DW_TAG_pointer_type:
    case DW_TAG_reference_type:
      dobj->set_dobjname (get_dobjname (ctx), NULL);
      break;
    case DW_TAG_structure_type:
    case DW_TAG_union_type:
    case DW_TAG_class_type:
      dobj->set_dobjname (get_dobjname (ctx), NULL);
      dobj->master = dbeSession->find_dobj_by_name (dobj_name);
      get_dobj_for_members (ctx);
      break;
    case DW_TAG_constant:
    case DW_TAG_formal_parameter:
    case DW_TAG_member:
    case DW_TAG_variable:
      if (dobj->parent == NULL)
	dobj->parent = dbeSession->get_Scalars_DataObject ();
      dobj->set_dobjname (get_dobjname (ctx), name);
      break;
    default:
      Dprintf (DUMP_DWARFLIB, NTXT ("DWARF_ERROR: %s:%d No case for %s cu_die_offset=%lld\n"),
	       get_basename (__FILE__), (int) __LINE__,
	       DwrCU::tag2str (tag), (long long) cu_die_offset);
      break;
    }
  return dobj;
}

void
Dwr_type::get_dobj_for_members (Dwarf_cnt *ctx)
{
  for (int64_t i = child; i != 0;)
    {
      Dwr_type *t = ctx->get_dwr_type (i);
      t->get_dobj (ctx);
      i = t->next;
    }
}

//////////////////////////////////////////////////////////
//  class Dwarf
Dwarf::Dwarf (Stabs *_stabs)
{
  stabs = _stabs;
  status = Stabs::DBGD_ERR_NONE;
  dwrCUs = 0;
  debug_infoSec = NULL;
  debug_abbrevSec = NULL;
  debug_strSec = NULL;
  debug_lineSec = NULL;
  debug_line_strSec = NULL;
  debug_rangesSec = NULL;
  elf = stabs->openElf (true);
  if (elf == NULL)
    {
      status = Stabs::DBGD_ERR_BAD_ELF_FORMAT;
      return;
    }
  debug_infoSec = dwrGetSec (NTXT (".debug_info"));
  if (debug_infoSec)
    {
      debug_infoSec->reloc = ElfReloc::get_elf_reloc (elf, NTXT (".rela.debug_info"), NULL);
      debug_infoSec->reloc = ElfReloc::get_elf_reloc (elf, NTXT (".rel.debug_info"), debug_infoSec->reloc);
      if (debug_infoSec->reloc)
	debug_infoSec->reloc->dump ();
    }
  debug_abbrevSec = dwrGetSec (NTXT (".debug_abbrev"));
  debug_strSec = dwrGetSec (NTXT (".debug_str"));
  debug_lineSec = dwrGetSec (NTXT (".debug_line"));
  debug_rangesSec = dwrGetSec (NTXT (".debug_ranges"));
  debug_line_strSec = dwrGetSec (".debug_line_str");

  if ((debug_infoSec == NULL) || (debug_abbrevSec == NULL) || (debug_lineSec == NULL))
    {
      status = Stabs::DBGD_ERR_NO_DWARF;
      return;
    }
}

Dwarf::~Dwarf ()
{
  delete debug_infoSec;
  delete debug_abbrevSec;
  delete debug_strSec;
  delete debug_lineSec;
  delete debug_rangesSec;
  Destroy (dwrCUs);
}

DwrSec *
Dwarf::dwrGetSec (const char *sec_name)
{
  int secN = elf->elf_get_sec_num (sec_name);
  if (secN > 0)
    {
      Elf_Data *elfData = elf->elf_getdata (secN);
      if (elfData)
	return new DwrSec ((unsigned char *) elfData->d_buf, elfData->d_size,
			   elf->need_swap_endian,
			   elf->elf_getclass () == ELFCLASS32);
    }
  return NULL;
}

uint64_t
DwrCU::get_low_pc ()
{
  uint64_t pc = Dwarf_addr (DW_AT_low_pc);
  if (pc)
    return pc;
  return pc;
}

char *
DwrCU::get_linkage_name ()
{
  char *nm = Dwarf_string (DW_AT_linkage_name);
  if (nm != NULL)
    return nm;
  nm = Dwarf_string (DW_AT_SUN_link_name);
  if (nm != NULL)
    return nm;
  return Dwarf_string (DW_AT_MIPS_linkage_name);
}

void
DwrCU::parseChild (Dwarf_cnt *ctx)
{
  if (!dwrTag.hasChild)
    return;
  uint64_t old_size = debug_infoSec->size;
  uint64_t next_die_offset = 0;
  Dwarf_Die next_die;
  if (read_ref_attr (DW_AT_sibling, &next_die) == DW_DLV_OK)
    {
      next_die_offset = next_die + cu_offset;
      if (next_die_offset <= debug_infoSec->offset)
	{
	  Dprintf (DEBUG_ERR_MSG, NTXT ("DwrCU::parseChild: next_die(0x%llx) <= debug_infoSec->offset(%llx)\n"),
		   (long long) next_die, (long long) debug_infoSec->offset);
	  next_die_offset = 0;
	}
      else if (debug_infoSec->size > next_die_offset)
	debug_infoSec->size = next_die_offset;
    }
  dwrTag.level++;
  ctx->level++;
  for (;;)
    {
      if (set_die (0) != DW_DLV_OK)
	break;
      Function *func;
      char *old_name;
      int hasChild = dwrTag.hasChild;
      switch (dwrTag.tag)
	{
	case DW_TAG_imported_declaration:
	  if (Stabs::is_fortran (ctx->module->lang_code))
	    {
	      char *link_name = Dwarf_string (DW_AT_name);
	      ctx->fortranMAIN = NULL;
	      parseChild (ctx);
	      hasChild = 0;
	      if (ctx->fortranMAIN)
		{
		  ctx->fortranMAIN->set_match_name (link_name);
		  ctx->fortranMAIN = NULL;
		}
	    }
	  break;
	case DW_TAG_subprogram:
	  if (dwrTag.get_attr (DW_AT_abstract_origin))
	    break;
	  if (dwrTag.get_attr (DW_AT_declaration))
	    {
	      // Only declaration
	      if (Stabs::is_fortran (ctx->module->lang_code))
		{
		  char *link_name = Dwarf_string (DW_AT_name);
		  if (link_name && streq (link_name, NTXT ("MAIN")))
		    ctx->fortranMAIN = Stabs::find_func (NTXT ("MAIN"), ctx->module->functions, true, true);
		}
	      break;
	    }
	  func = append_Function (ctx);
	  if (func)
	    {
	      if (Stabs::is_fortran (ctx->module->lang_code) &&
		  streq (func->get_match_name (), NTXT ("MAIN")))
		ctx->fortranMAIN = func;
	      old_name = ctx->name;
	      Function *old_func = ctx->func;
	      ctx->name = func->get_match_name ();
	      ctx->func = func;
	      parseChild (ctx);
	      hasChild = 0;
	      ctx->name = old_name;
	      ctx->func = old_func;
	    }
	  break;
	case DW_TAG_module:
	  old_name = ctx->name;
	  ctx->name = Dwarf_string (DW_AT_SUN_link_name);
	  parseChild (ctx);
	  hasChild = 0;
	  ctx->name = old_name;
	  break;
	case DW_TAG_class_type:
	  old_name = ctx->name;
	  ctx->name = Dwarf_string (DW_AT_name);
	  parseChild (ctx);
	  hasChild = 0;
	  ctx->name = old_name;
	  break;
	case DW_TAG_structure_type:
	  old_name = ctx->name;
	  ctx->name = NULL;
	  parseChild (ctx);
	  hasChild = 0;
	  ctx->name = old_name;
	  break;
	case DW_TAG_namespace:
	  old_name = ctx->name;
	  ctx->name = Dwarf_string (DW_AT_name);
	  parseChild (ctx);
	  hasChild = 0;
	  ctx->name = old_name;
	  break;
	case DW_TAG_lexical_block:
	  old_name = ctx->name;
	  ctx->name = NULL;
	  parseChild (ctx);
	  hasChild = 0;
	  ctx->name = old_name;
	  break;
	case DW_TAG_SUN_memop_info:
	  isMemop = true;
	  break;
	case DW_TAG_inlined_subroutine:
	  if (ctx->module)
	    {
	      parse_inlined_subroutine (ctx);
	      hasChild = 0;
	    }
	  break;
	default: // No more special cases
	  break;
	}
      if (hasChild)
	parseChild (ctx);
    }
  ctx->level--;
  dwrTag.level--;
  if (next_die_offset != 0)
    debug_infoSec->offset = next_die_offset;
  debug_infoSec->size = old_size;
}

bool
Dwarf::archive_Dwarf (LoadObject *lo)
{
  if (debug_infoSec == NULL)
    return false;
  if (dwrCUs)
    return true;
  dwrCUs = new Vector<DwrCU *>;

  debug_infoSec->offset = 0;
  while (debug_infoSec->offset < debug_infoSec->sizeSec)
    {
      DwrCU *dwrCU = new DwrCU (this);
      dwrCUs->append (dwrCU);
      debug_infoSec->size = debug_infoSec->sizeSec;
      debug_infoSec->offset = dwrCU->next_cu_offset;

      if (dwrCU->set_die (dwrCU->cu_header_offset) != DW_DLV_OK)
	{
	  Dprintf (1, "DwrCU::archive_Dwarf: CU=%lld  (offset=0x%llx); set_die(0x%llx) failed\n",
		   (long long) dwrCUs->size (), (long long) dwrCU->cu_offset,
		   (long long) dwrCU->cu_header_offset);
	  continue;
	}

      Module *mod = dwrCU->parse_cu_header (lo);
      if (mod)
	{
	  mod->hdrOffset = dwrCUs->size ();
	  DwrLineRegs *lineReg = dwrCU->get_dwrLineReg ();
	  if (lineReg != NULL)
	    {
	      dwrCU->srcFiles = new Vector<SourceFile *> (VecSize (lineReg->file_names));
	      for (long i = 0, sz = VecSize (lineReg->file_names); i < sz; i++)
		{
		  char *fname = lineReg->getPath (i);
		  if (fname)
		    dwrCU->srcFiles->append (mod->findSource (fname, true));
		}
	    }

	  Dwarf_cnt ctx;
	  ctx.module = mod;
	  dwrCU->parseChild (&ctx);
	  if (dwrCU->dwrInlinedSubrs && DUMP_DWARFLIB)
	    {
	      char msg[128];
	      char *lo_name = mod->loadobject ? mod->loadobject->get_name ()
		      : NULL;
	      snprintf (msg, sizeof (msg), NTXT ("\ndwrCUs[%lld]: %s:%s\n"),
			(long long) dwrCUs->size (),
			STR (lo_name), STR (mod->get_name ()));
	      dwrCU->dwrInlinedSubrs->dump (msg);
	    }
	}
    }
  return true;
}

void
Dwarf::srcline_Dwarf (Module *module)
{
  if (module == NULL || module->hdrOffset == 0)
    return;
  DwrCU *dwrCU = dwrCUs->get (module->hdrOffset - 1);
  dwrCU->map_dwarf_lines (module);
}


// parse hwcprof info for given module in loadobject

void
Dwarf::read_hwcprof_info (Module *module)
{
  if (module->datatypes || (module->hdrOffset == 0))
    return;
  DwrCU *dwrCU = dwrCUs->get (module->hdrOffset - 1);
  if (!dwrCU->isMemop)
    return;
  module->datatypes = new Vector<datatype_t*>;
  if (dwrCU->set_die (dwrCU->cu_header_offset) != DW_DLV_OK)
    {
      Dprintf (1, "Dwarf::read_hwcprof_info: CU=%lld  (offset=0x%llx); set_die(0x%llx) failed\n",
	       (long long) module->hdrOffset, (long long) dwrCU->cu_offset,
	       (long long) dwrCU->cu_header_offset);
      return;
    }
  Dwarf_cnt ctx;
  ctx.module = module;
  ctx.cu_offset = dwrCU->cu_offset; // CU header offset;
  ctx.dwr_types = new DefaultMap<int64_t, Dwr_type*>;
  ctx.put_dwr_type (0, 0); // for DOBJ_UNSPECIFIED
  dwrCU->read_hwcprof_info (&ctx);

  Vector<inst_info_t*> *infoList = module->infoList;
  Dprintf (DUMP_DWARFLIB,
     "\n\n ### Dwarf::read_hwcprof_info: module: '%s'  infoList->size()=%lld\n",
     STR (module->get_name ()),
     (long long) (infoList ? infoList->size () : -1));
  for (int i = 0, sz = infoList ? infoList->size () : -1; i < sz; i++)
    {
      inst_info_t *ip = infoList->fetch (i);
      memop_info_t *mp = ip->memop;
      Dwr_type *t = ctx.get_dwr_type (mp->datatype_id);
      t->get_dobj (&ctx);
    }

#ifdef DEBUG
  Dprintf (DUMP_DWARFLIB,
	   "\n\n ### Dwarf::read_hwcprof_info: '%s'  infoList->size()=%lld\n",
	   STR (module->get_name ()),
	   (long long) (infoList ? infoList->size () : 1));
  for (int i = 0, sz = infoList ? infoList->size () : -1; i < sz; i++)
    {
      inst_info_t *ip = infoList->fetch (i);
      memop_info_t *mp = ip->memop;
      Dprintf (DUMP_DWARFLIB,
	       "  %d id=%lld  offset=%lld  signature=%lld  datatype_id=%lld \n",
	       i, (long long) mp->id, (long long) mp->offset,
	       (long long) mp->signature, (long long) mp->datatype_id);
    }

  Vector<int64_t> *keys = ctx.dwr_types->keySet ();
  Dprintf (DUMP_DWARFLIB,
	   "\n\n ### Dwarf::read_hwcprof_info: '%s'  keys->size()=%lld\n",
	   STR (module->get_name ()), (long long) (keys ? keys->size () : -1));
  for (int i = 0, sz = keys->size (); i < sz; i++)
    {
      int64_t ind = keys->fetch (i);
      Dwr_type *t = ctx.get_dwr_type (ind);
      Dprintf (DUMP_DWARFLIB, NTXT ("  %d %lld %s\n"), i,
	       (long long) ind, t->dump ());
    }
#endif
}

void
DwrCU::read_hwcprof_info (Dwarf_cnt *ctx)
{
  if (!dwrTag.hasChild)
    return;
  uint64_t old_size = debug_infoSec->size;
  uint64_t next_die_offset = 0;
  Dwarf_Die next_die;
  if (read_ref_attr (DW_AT_sibling, &next_die) == DW_DLV_OK)
    {
      next_die_offset = next_die + cu_offset;
      if (next_die_offset <= debug_infoSec->offset)
	next_die_offset = 0;
      else if (debug_infoSec->size > next_die_offset)
	debug_infoSec->size = next_die_offset;
    }
  dwrTag.level++;
  ctx->level++;
  for (;;)
    {
      if (set_die (0) != DW_DLV_OK)
	break;
      Dprintf (DUMP_DWARFLIB, NTXT ("%s:%d <%lld:%lld> cu_die=%lld %-15s\n"),
	       get_basename (__FILE__), (int) __LINE__, (long long) ctx->level,
	       (long long) dwrTag.die, (long long) dwrTag.offset,
	       DwrCU::tag2str (dwrTag.tag));
      switch (dwrTag.tag)
	{
	case DW_TAG_SUN_memop_info:
	  {
	    if (ctx->func == NULL)
	      break;
	    Dwarf_Unsigned mid = Dwarf_data (DW_AT_SUN_profile_id);
	    Dwarf_Unsigned off = Dwarf_data (DW_AT_SUN_func_offset);
	    Dwarf_Unsigned sig = Dwarf_data (DW_AT_SUN_memop_signature);
	    Dwarf_Off ref = Dwarf_ref (DW_AT_SUN_memop_type_ref);

	    // define memop entry
	    memop_info_t *memop = new memop_info_t;
	    memop->id = (unsigned) mid;
	    memop->signature = (unsigned) sig;
	    memop->datatype_id = ref ? (unsigned) ref : 0;
	    memop->offset = (unsigned) (ctx->func->img_offset + off);

	    // define instop entry
	    inst_info_t *instop = new inst_info_t;
	    instop->type = CPF_INSTR_TYPE_PREFETCH; // XXXX UNKNOWN
	    instop->offset = memop->offset;
	    instop->memop = memop;
	    if (ctx->module->infoList == NULL)
	      ctx->module->infoList = new Vector<inst_info_t*>;
	    ctx->module->infoList->append (instop);
	    break;
	  }
	case DW_TAG_SUN_codeflags:
	  {
	    if (ctx->func == NULL)
	      break;
	    Dwarf_Unsigned kind = Dwarf_data (DW_AT_SUN_cf_kind);
	    if (kind == DW_ATCF_SUN_branch_target)
	      {
		DwrSec *secp = Dwarf_block (DW_AT_SUN_func_offsets);
		if (secp)
		  {
		    int foffset = 0;
		    for (int i = 0; secp->offset < secp->size; i++)
		      {
			int val = (int) secp->GetSLEB128 ();
			if (i == 0 || val != 0)
			  {
			    foffset += val;
			    target_info_t *t = new target_info_t;
			    t->offset = (unsigned) (ctx->func->img_offset + foffset);
			    ctx->module->bTargets.incorporate (t, targetOffsetCmp);
			  }
		      }
		    delete(secp);
		  }
	      }
	    break;
	  }
	case DW_TAG_subprogram:
	  {
	    Function *old_func = ctx->func;
	    if (dwrTag.get_attr (DW_AT_abstract_origin)
				 || dwrTag.get_attr (DW_AT_declaration))
	      ctx->func = NULL;
	    else
	      ctx->func = append_Function (ctx);
	    read_hwcprof_info (ctx);
	    ctx->func = old_func;
	    break;
	  }
	case DW_TAG_base_type:
	  {
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->size = Dwarf_data (DW_AT_byte_size);
	    break;
	  }
	case DW_TAG_unspecified_type:
	  ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	  break;
	case DW_TAG_enumeration_type:
	  {
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->size = Dwarf_data (DW_AT_byte_size);
	    break;
	  }
	case DW_TAG_constant:
	case DW_TAG_formal_parameter:
	case DW_TAG_variable:
	case DW_TAG_typedef:
	case DW_TAG_const_type:
	case DW_TAG_volatile_type:
	  {
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->ref_type = Dwarf_ref (DW_AT_type);
	    break;
	  }
	case DW_TAG_pointer_type:
	case DW_TAG_reference_type:
	  {
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->ref_type = Dwarf_ref (DW_AT_type);
	    t->size = (dwarf->stabs->get_class () == W64) ? 8 : 4;
	    break;
	  }
	case DW_TAG_array_type:
	  {
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->ref_type = Dwarf_ref (DW_AT_type);
	    t->size = Dwarf_data (DW_AT_byte_size);
	    ctx->size = -1;
	    read_hwcprof_info (ctx);
	    t->elems = ctx->size;
	    break;
	  }
	case DW_TAG_subrange_type:
	  {
	    int64_t ref_type = Dwarf_ref (DW_AT_type);
	    int64_t hi = Dwarf_data (DW_AT_upper_bound);
	    int64_t lo = Dwarf_data (DW_AT_lower_bound);
	    int64_t ss = Dwarf_data (DW_AT_stride_size);
	    ctx->size = 1 + hi - lo;
	    if (ss != 0)
	      ctx->size /= ss;
	    Dprintf (DUMP_DWARFLIB,
		    "Got subrange [%lld:%lld:%lld] indexed <%lld>: size=%lld\n",
		     (long long) lo, (long long) hi, (long long) ss,
		     (long long) ref_type, (long long) ctx->size);
	    break;
	  }
	case DW_TAG_structure_type:
	case DW_TAG_union_type:
	case DW_TAG_class_type:
	  {
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->size = Dwarf_data (DW_AT_byte_size);
	    t->extent = Dwarf_ref (DW_AT_sibling);
	    int64_t old_parent = ctx->parent;
	    ctx->parent = t->cu_die_offset;

	    char *old_name = ctx->name;
	    ctx->name = (dwrTag.tag == DW_TAG_class_type) ? Dwarf_string (DW_AT_name) : NULL;
	    read_hwcprof_info (ctx);
	    ctx->name = old_name;
	    ctx->parent = old_parent;
	    // Reverse children
	    for (int64_t i = t->child, last = 0; i != 0;)
	      {
		Dwr_type *t1 = ctx->get_dwr_type (i);
		t->child = i;
		i = t1->next;
		t1->next = last;
		last = t->child;
	      }
	    break;
	  }
	case DW_TAG_member:
	  {
	    if (ctx->parent == 0)
	      {
		Dprintf (DUMP_DWARFLIB, NTXT ("DWARF_ERROR: %s:%d %s cu_die_offset=%lld\n"),
			 get_basename (__FILE__), (int) __LINE__,
			 DwrCU::tag2str (dwrTag.tag), (long long) dwrTag.die);
		break;
	      }
	    Dwr_type *t = ctx->put_dwr_type (dwrTag.die, dwrTag.tag);
	    t->name = Dwarf_string (DW_AT_name);
	    t->ref_type = Dwarf_ref (DW_AT_type);
	    t->offset = Dwarf_location (DW_AT_data_member_location);
	    Dwr_type *parent = ctx->get_dwr_type (ctx->parent);
	    t->parent = ctx->parent;
	    t->next = parent->child; // a reverse order of members
	    parent->child = t->cu_die_offset;
	    t->bit_size = (uint32_t) Dwarf_data (DW_AT_bit_size);
	    break;
	  }
	case DW_TAG_module:
	  {
	    char *old_name = ctx->name;
	    ctx->name = Dwarf_string (DW_AT_SUN_link_name);
	    read_hwcprof_info (ctx);
	    ctx->name = old_name;
	    break;
	  }
	case DW_TAG_namespace:
	  {
	    char *old_name = ctx->name;
	    ctx->name = Dwarf_string (DW_AT_name);
	    read_hwcprof_info (ctx);
	    ctx->name = old_name;
	    break;
	  }
	case DW_TAG_lexical_block:
	  {
	    char *old_name = ctx->name;
	    ctx->name = NULL;
	    read_hwcprof_info (ctx);
	    ctx->name = old_name;
	    break;
	  }
	default: // No more special cases
	  read_hwcprof_info (ctx);
	  break;
	}
    }
  ctx->level--;
  dwrTag.level--;
  if (next_die_offset != 0)
    debug_infoSec->offset = next_die_offset;
  debug_infoSec->size = old_size;
}

// Append function to module
Function *
DwrCU::append_Function (Dwarf_cnt *ctx)
{
  char *outerName = ctx->name, *name, tmpname[2048];
  Function *func;
  char *fname = Dwarf_string (DW_AT_name);
  if (fname && outerName && !strchr (fname, '.'))
    {
      size_t outerlen = strlen (outerName);
      if (outerlen > 0 && outerName[outerlen - 1] == '_')
	{
	  outerlen--;
	  snprintf (tmpname, sizeof (tmpname), NTXT ("%s"), outerName);
	  snprintf (tmpname + outerlen, sizeof (tmpname) - outerlen, NTXT (".%s_"), fname);
	}
      else
	snprintf (tmpname, sizeof (tmpname), NTXT ("%s.%s"), outerName, fname);
      name = tmpname;
      Dprintf (DUMP_DWARFLIB, NTXT ("Generated innerfunc name %s\n"), name);
    }
  else
    name = fname;

  char *link_name = get_linkage_name ();
  if (link_name == NULL)
    link_name = name;

  uint64_t pc = get_low_pc ();
  func = dwarf->stabs->append_Function (module, link_name, pc);
  if (func != NULL)
    {
      int lineno = (int) Dwarf_data (DW_AT_decl_line);
      func->set_match_name (name);
      if (lineno > 0)
	{
	  func->setLineFirst (lineno);
	  int fileno = ((int) Dwarf_data (DW_AT_decl_file));
	  SourceFile *sf = ((fileno >= 0) && (fileno < VecSize (srcFiles))) ? srcFiles->get (fileno)
		  : module->getMainSrc ();
	  func->setDefSrc (sf);
	  func->pushSrcFile (func->def_source, 0);
	  func->popSrcFile ();
	}
    }
  return func;
}

// Get language code
Sp_lang_code
DwrCU::Dwarf_lang ()
{
  char *str = Dwarf_string (DW_AT_producer);
  if (str && strncmp (str, NTXT ("GNU"), 3) == 0)
    isGNU = true;
  int64_t lang = Dwarf_data (DW_AT_language);
  switch (lang)
    {
    case DW_LANG_C89:
    case DW_LANG_C:
      return Sp_lang_c; // Sp_lang_ansic?
    case DW_LANG_C99:
      return Sp_lang_c99;
    case DW_LANG_C_plus_plus:
      return isGNU ? Sp_lang_gcc : Sp_lang_cplusplus;
    case DW_LANG_Fortran90:
      return Sp_lang_fortran90;
    case DW_LANG_Fortran77:
      return Sp_lang_fortran;
    case DW_LANG_Java:
      return Sp_lang_java;
    case DW_LANG_Mips_Assembler:
    case DW_LANG_SUN_Assembler:
      return Sp_lang_asm;
    case DW_LANG_Pascal83:
      return Sp_lang_pascal;
    default:
    case DW_LANG_Ada83:
    case DW_LANG_Cobol74:
    case DW_LANG_Cobol85:
    case DW_LANG_Modula2:
    case DW_LANG_Ada95:
    case DW_LANG_Fortran95:
    case DW_LANG_lo_user:
      return Sp_lang_unknown;
    }
}
