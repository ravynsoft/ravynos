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
#include <unistd.h>
#include <ar.h>
#include <ctype.h>
#include <sys/param.h>

#include "util.h"
#include "DbeSession.h"
#include "Experiment.h"
#include "DataObject.h"
#include "Function.h"
#include "DbeView.h"
#include "MetricList.h"
#include "Module.h"
#include "ClassFile.h"
#include "LoadObject.h"
#include "Disasm.h"
#include "CompCom.h"
#include "Dwarf.h"
#include "DbeFile.h"
#include "PathTree.h"
#include "Elf.h"

Module::Module ()
{
  lang_code = Sp_lang_unknown;
  flags = 0;
  status = AE_NOTREAD;
  openSourceFlag = AE_NOTREAD;
  hexVisible = false;
  disPath = NULL;
  stabsPath = NULL;
  stabsTmp = NULL;
  disName = NULL;
  stabsName = NULL;
  indexStabsLink = NULL;
  file_name = NULL;
  functions = new Vector<Function*>;
  loadobject = NULL;
  dot_o_file = NULL;
  main_source = dbeSession->get_Unknown_Source ();
  srcContext = main_source;
  includes = new Vector<SourceFile*>;
  includes->append (main_source);
  curr_inc = NULL;
  fragmented = 0;
  hwcprof = 0;
  hdrOffset = 0;
  hasDwarf = false;
  hasStabs = false;
  readStabs = false;
  comComs = NULL;
  infoList = NULL;
  datatypes = NULL;
  objStabs = NULL;
  disasm = NULL;
  comp_flags = NULL;
  comp_dir = NULL;
  linkerStabName = NULL;
  disMTime = (time_t) 0;
  stabsMTime = (time_t) 0;
  real_timestamp = 0;
  curr_timestamp = 0;
  src_items = NULL;
  dis_items = NULL;
  data_items = NULL;
  cur_dbev = NULL;
  maximum = NULL;
  maximum_inc = NULL;
  empty = NULL;
  inlinedSubr = NULL;
}

Module::~Module ()
{
  removeStabsTmp ();
  delete includes;
  if (comComs != NULL)
    {
      comComs->destroy ();
      delete comComs;
    }
  free (comp_flags);
  free (comp_dir);
  free (linkerStabName);
  free (disPath);
  free (stabsPath);
  free (disName);
  free (stabsName);
  delete functions;
  free (file_name);
  if (indexStabsLink)
    // Remove a link to the current module
    indexStabsLink->indexStabsLink = NULL;

  if (dot_o_file)
    {
      delete dot_o_file->dbeFile;
      delete dot_o_file;
    }
  delete src_items;
  delete dis_items;
  delete disasm;
  free (inlinedSubr);
  if (lang_code != Sp_lang_java)
    delete dbeFile;

}

Stabs *
Module::openDebugInfo ()
{
  setFile ();
  objStabs = loadobject->openDebugInfo (disPath);
  return objStabs;
}

void
Module::removeStabsTmp ()
{
  // Remove temporary *.o (got from *.a) after reading Stabs
  if (stabsTmp != NULL)
    {
      unlink (stabsTmp);
      free (stabsTmp);
      stabsTmp = NULL;
    }
}

int64_t
Module::get_size ()
{
  Function *fp;
  int index;
  int64_t result = 0;
  Vec_loop (Function*, functions, index, fp)
  {
    result += fp->size;
  }
  return result;
}

bool
Module::is_fortran ()
{
  return Stabs::is_fortran (lang_code);
}

SourceFile *
Module::findSource (const char *fname, bool create)
{
  SourceFile *sf = NULL;
  if (loadobject && loadobject->firstExp)
    sf = loadobject->firstExp->get_source (fname);
  if (sf == NULL)
    sf = dbeSession->createSourceFile (fname);
  for (int i = 0, sz = includes ? includes->size () : 0; i < sz; i++)
    {
      SourceFile *sf1 = includes->fetch (i);
      if (sf == sf1)
	return sf;
    }
  if (create)
    {
      if (includes == NULL)
	includes = new Vector<SourceFile*>;
      includes->append (sf);
      return sf;
    }
  return NULL;
}

SourceFile *
Module::setIncludeFile (char *includeFile)
{
  curr_inc = NULL;
  if (includeFile)
    curr_inc = findSource (includeFile, true);
  return curr_inc;
}

char *
Module::anno_str (char *fnm)
{
  char timebuf1[26], timebuf2[26];
  const time_t real_time = (time_t) (unsigned int) real_timestamp;
  const time_t curr_time = (time_t) (unsigned int) curr_timestamp;

  switch (status)
    {
    case AE_OK:
    case AE_NOTREAD:
      return NULL;
    case AE_NOSRC:
      return dbe_sprintf (GTXT ("Source file `%s' not readable"),
			  fnm ? fnm : file_name);
    case AE_NOOBJ:
      if (lang_code == Sp_lang_java)
	{
	  Emsg *emsg = get_error ();
	  if (emsg)
	    {
	      char *s = dbe_strdup (emsg->get_msg ());
	      remove_msg (emsg);
	      return s;
	    }
	  return dbe_sprintf (GTXT ("Object file `%s.class' not readable"),
			      name);
	}
      return dbe_sprintf (GTXT ("Object file `%s' not readable"), get_name ());
    case AE_NOLOBJ:
      if (lang_code == Sp_lang_java)
	return dbe_sprintf (GTXT ("Object file `%s' not readable"),
			    dbeFile ? dbeFile->get_name () : name);
      return dbe_sprintf (GTXT ("Object file `%s' not readable"), loadobject->get_pathname ());
    case AE_NOSTABS:
      return dbe_sprintf (GTXT ("Error reading line-number information in object `%s'; source annotation not available"),
			  stabsPath ? stabsPath : NTXT (""));
    case AE_NOSYMTAB:
      return dbe_sprintf (GTXT ("Error reading symbol table in object `%s'; disassembly annotation not available"),
			  disPath ? disPath : NTXT (""));
    case AE_TIMESRC:
      return dbe_sprintf (GTXT ("Warning! Source file `%s' is newer than the experiment data"),
			  main_source->dbeFile->getResolvedPath ());
    case AE_TIMEDIS:
      return dbe_sprintf (GTXT ("Warning! Object file `%s' is newer than the experiment data"),
			  disName ? disName : NTXT (""));
    case AE_TIMESTABS:
      return dbe_sprintf (GTXT ("Warning! Object file `%s' is newer than the experiment data"),
			  stabsName ? stabsName : NTXT (""));
    case AE_TIMESTABS_DIFF:
      snprintf (timebuf1, sizeof (timebuf1), NTXT ("%s"), ctime (&curr_time));
      snprintf (timebuf2, sizeof (timebuf2), NTXT ("%s"), ctime (&real_time));
      timebuf1[24] = timebuf2[24] = '\0';
      return dbe_sprintf (GTXT ("Warning! Object file `%s' is not the same one that was linked into executable.\n"
				"\tObject file: `%s'\n\tcompiled on: %s\n"
				"\tExecutable contains object file compiled on: %s"),
			  getResolvedObjectPath (), getResolvedObjectPath (),
			  timebuf1, timebuf2);
    case AE_OTHER:
    default:
      return dbe_strdup (GTXT ("Annotation computation error"));
    }
}//anno_str

LoadObject *
Module::createLoadObject (const char *lo_name)
{
  LoadObject *lo = new LoadObject (lo_name);
  lo->dbeFile->filetype |= DbeFile::F_DOT_O;
  return lo;
}

static bool
tsIsNewer (time_t t1, time_t t2)
{
  return t1 != 0 && t2 != 0 && t1 < t2;
}

Module::Anno_Errors
Module::checkTimeStamp (bool chkDis)
{
  /* Check the linked and the real object timestamps due to bug #4796329 */
  if (real_timestamp && curr_timestamp && real_timestamp != curr_timestamp)
    return AE_TIMESTABS_DIFF;

  time_t srctime = main_source->getMTime ();
  for (int index = 0; index < dbeSession->nexps (); index++)
    {
      time_t mtime = dbeSession->get_exp (index)->get_mtime ();
      if (tsIsNewer (mtime, srctime))
	return AE_TIMESRC;
      if (tsIsNewer (mtime, stabsMTime))
	return AE_TIMESTABS;
      if (chkDis && tsIsNewer (mtime, disMTime))
	return AE_TIMEDIS;
    }
  return AE_OK;
}//checkTimeStamp

static size_t
get_ar_size (char *s, size_t len)
{
  size_t sz = 0;
  for (size_t i = 0; i < len; i++)
    {
      if (s[i] < '0' || s[i] > '9')
	break;
      sz = sz * 10 + (s[i] - '0');
    }
  return sz;
}

static void
dump_hdr_field (char *nm, char *s, size_t len)
{
  Dprintf (DEBUG_READ_AR, NTXT ("  %s "), nm);
  for (size_t i = 0; i < len; i++)
    Dprintf (DEBUG_READ_AR, "%c", isprint (s[i]) ? s[i] : '?');
  Dprintf (DEBUG_READ_AR, NTXT ("  "));
  for (size_t i = 0; i < len; i++)
    Dprintf (DEBUG_READ_AR, NTXT (" %d"), s[i]);
  Dprintf (DEBUG_READ_AR, NTXT (" \n"));
}

static void
dump_ar_hdr (int lineNum, struct ar_hdr *hdr)
{
  if (DEBUG_READ_AR)
    {
      Dprintf (DEBUG_READ_AR, NTXT ("Module::read_ar %d\n"), lineNum);
      dump_hdr_field (NTXT ("ar_name"), hdr->ar_name, sizeof (hdr->ar_name));
      dump_hdr_field (NTXT ("ar_date"), hdr->ar_date, sizeof (hdr->ar_date));
      dump_hdr_field (NTXT ("ar_uid"), hdr->ar_uid, sizeof (hdr->ar_uid));
      dump_hdr_field (NTXT ("ar_gid"), hdr->ar_gid, sizeof (hdr->ar_gid));
      dump_hdr_field (NTXT ("ar_mode"), hdr->ar_mode, sizeof (hdr->ar_mode));
      dump_hdr_field (NTXT ("ar_size"), hdr->ar_size, sizeof (hdr->ar_size));
      dump_hdr_field (NTXT ("ar_fmag"), hdr->ar_fmag, sizeof (hdr->ar_fmag));
    }
}

bool
Module::read_ar (int ar, int obj, char *obj_base)
{
  struct ar_hdr hdr; // Archive header
  char magic[SARMAG]; // Magic string from archive
  Dprintf (DEBUG_READ_AR, "Module::read_ar %d %p %s %s \n", __LINE__,
	   this, STR (obj_base), STR (get_name ()));
  // Check the magic string
  if ((read_from_file (ar, magic, SARMAG) != SARMAG)
       || strncmp (magic, ARMAG, SARMAG))
    return false;

  // Read and skip the first file in the archive (index file to ld)
  if (read_from_file (ar, &hdr, sizeof (hdr)) != sizeof (hdr))
    return false;
  DEBUG_CODE dump_ar_hdr (__LINE__, &hdr);
  if (lseek (ar, get_ar_size (hdr.ar_size, sizeof (hdr.ar_size)), SEEK_CUR)
	     == -1)
    return false;

  // Read the string file where it keeps long file names (if exist)
  if (read_from_file (ar, &hdr, sizeof (hdr)) != sizeof (hdr))
    return false;
  DEBUG_CODE dump_ar_hdr (__LINE__, &hdr);
  char *longnames = NULL; // Area with names longer than ~13
  size_t longnames_size = 0;
  if (!strncmp (hdr.ar_name, NTXT ("//"), 2))
    {
      longnames_size = get_ar_size (hdr.ar_size, sizeof (hdr.ar_size));
      longnames = (char *) malloc (longnames_size + 1);
      int64_t cnt = read_from_file (ar, longnames, longnames_size);
      if (cnt != (int64_t) longnames_size)
	{
	  free (longnames);
	  return false;
	}
      longnames[longnames_size] = 0;
    }
  else
    // back out, no long file names
    lseek (ar, -(sizeof (hdr)), SEEK_CUR);

  // Search the ar for the object file name
  char ar_buf[sizeof (hdr.ar_name) + 1];
  ar_buf[sizeof (hdr.ar_name)] = 0;
  while (1)
    {
      if (read_from_file (ar, &hdr, sizeof (hdr)) != sizeof (hdr))
	break;
      DEBUG_CODE dump_ar_hdr (__LINE__, &hdr);
      char *ar_name;
      if (hdr.ar_name[0] != '/')
	{ // Name is in the header
	  for (size_t i = 0; i < sizeof (hdr.ar_name); i++)
	    {
	      if (hdr.ar_name[i] == '/')
		{
		  ar_buf[i] = 0;
		  break;
		}
	      ar_buf[i] = hdr.ar_name[i];
	    }
	  ar_name = ar_buf;
	}
      else if (hdr.ar_name[1] == ' ')
	{ // Name is blank
	  ar_buf[0] = 0;
	  ar_name = ar_buf;
	}
      else
	{ // Name is in the string table
	  if (longnames == NULL)
	    break;
	  size_t offset = get_ar_size (hdr.ar_name + 1,
				       sizeof (hdr.ar_name) - 1);
	  if (offset >= longnames_size)
	    break;
	  for (size_t i = offset; i < longnames_size; i++)
	    {
	      if (longnames[i] == '/')
		{
		  longnames[i] = 0;
		  break;
		}
	    }
	  ar_name = longnames + offset;
	}
      Dprintf (DEBUG_READ_AR, "Module::read_ar %d ar_name=%s\n", __LINE__,
	       ar_name);

      if (streq (ar_name, obj_base))
	{ // create object file
	  free (longnames);
	  for (size_t objsize = get_ar_size (hdr.ar_size, sizeof (hdr.ar_size));
		  objsize > 0;)
	    {
	      char buf[MAXPATHLEN];
	      size_t n = objsize < sizeof (buf) ? objsize : sizeof (buf);
	      int64_t cnt = read_from_file (ar, buf, n);
	      if (cnt != (int64_t) n)
		return false;
	      cnt = write (obj, buf, n);
	      if (cnt != (int64_t) n)
		return false;
	      objsize -= n;
	    }
	  return true;
	}
      if (lseek (ar, get_ar_size (hdr.ar_size, sizeof (hdr.ar_size)),
		 SEEK_CUR) == -1)
	break;
    }
  free (longnames);
  return false;
}

static char *
get_obj_name_from_lib (char *nm)
{
  char *base = strrchr (nm, '(');
  if (base)
    {
      size_t last = strlen (base) - 1;
      if (base[last] == ')')
	return base;
    }
  return NULL;
}

bool
Module::setFile ()
{
  if ((loadobject->flags & SEG_FLAG_DYNAMIC) != 0)
    return true;
  if ((loadobject->dbeFile->filetype & DbeFile::F_FICTION) != 0)
    return false;
  if ((flags & MOD_FLAG_UNKNOWN) != 0)
    return true;

  if (lang_code == Sp_lang_java)
    {
      if (dbeFile->get_need_refind ())
	{
	  char *fnm = dbeFile->get_location ();
	  stabsPath = dbe_strdup (fnm);
	  stabsName = dbe_strdup (fnm);
	  disPath = dbe_strdup (fnm);
	  disName = dbe_strdup (fnm);
	  stabsMTime = dbeFile->sbuf.st_mtime;
	}
      return dbeFile->get_location () != NULL;
    }

  if (dbeFile == NULL)
    {
      char *objname = get_obj_name_from_lib (name);
      if (objname)
	{
	  // in the format of libpath(obj)
	  objname = dbe_strdup (objname + 1);
	  size_t last = strlen (objname) - 1;
	  objname[last] = '\0';
	}
      dbeFile = new DbeFile (objname ? objname : name);
      free (objname);
      dbeFile->filetype |= DbeFile::F_DOT_O;
    }
  if (dbeFile->get_need_refind ())
    {
      disMTime = (time_t) 0;
      stabsMTime = (time_t) 0;
      free (disName);
      free (stabsName);
      disName = NULL;
      stabsName = NULL;

      // Find the Executable/Shared-Object file of module
      char *path = loadobject->dbeFile->get_location ();
      if (path)
	{
	  disPath = strdup (path);
	  disName = strdup (path);
	  disMTime = loadobject->dbeFile->sbuf.st_mtime;
	}

      char *objname = get_obj_name_from_lib (name);
      if (objname)
	{
	  // in the format of libpath(obj)
	  char *namebuf = dbe_strdup (name);
	  char *base = namebuf + (objname - name);
	  *base = '\0';
	  base++;
	  size_t last = strlen (base) - 1;
	  base[last] = '\0';
	  stabsTmp = dbeSession->get_tmp_file_name (base, false);
	  dbeSession->tmp_files->append (strdup (stabsTmp));

	  DbeFile *dbf = dbeSession->getDbeFile (namebuf,
					DbeFile::F_DOT_A_LIB | DbeFile::F_FILE);
	  path = dbf->get_location ();
	  int ar = -1, obj = -1;
	  if (path != NULL)
	    {
	      ar = open64 (path, O_RDONLY | O_LARGEFILE);
	      if (ar != -1)
		obj = open64 (stabsTmp, O_CREAT | O_WRONLY | O_LARGEFILE, 0600);
	    }
	  if (ar != -1 && obj != -1 && read_ar (ar, obj, base))
	    {
	      dbeFile->set_location (stabsTmp);
	      dbeFile->check_access (stabsTmp); // init 'sbuf'
	      dbeFile->sbuf.st_mtime = 0; // Don't check timestamps
	      dbeFile->container = dbf;
	      stabsPath = strdup (stabsTmp);
	      stabsName = strdup (path);
	      stabsMTime = dbeFile->sbuf.st_mtime;
	    }
	  else
	    {
	      removeStabsTmp ();
	      objname = NULL;
	    }
	  if (ar != -1)
	    close (ar);
	  if (obj != -1)
	    close (obj);
	  free (namebuf);
	}
      if (objname == NULL)
	{
	  path = dbeFile->get_location ();
	  if (path != NULL)
	    {
	      stabsPath = strdup (path);
	      stabsName = strdup (path);
	      stabsMTime = hasDwarf ? 0 : dbeFile->sbuf.st_mtime;
	    }
	}

      // First, try to access the symbol table of the module itself
      // If failed, access the symbol table of the executable
      if (stabsPath == NULL)
	{
	  if (disPath == NULL)
	    return false;
	  stabsPath = strdup (disPath);
	  stabsName = strdup (disName);
	  stabsMTime = disMTime;
	}
      else if (disPath == NULL)
	{
	  disPath = strdup (stabsPath);
	  disName = strdup (stabsName);
	  disMTime = stabsMTime;
	}
    }
  return stabsPath != NULL;
}

// openStabs -- open mappings from PCs to source lines
bool
Module::openStabs (bool all)
{
  if ((loadobject->flags & SEG_FLAG_DYNAMIC) != 0
      || (flags & MOD_FLAG_UNKNOWN) != 0)
    return true;
  if (loadobject->platform == Java)
    {
      setIncludeFile (NULL);
      readFile ();
      return ( status == AE_OK);
    }
  if (readStabs)
    return true;

  // Read Stabs info.
  int64_t Inode = main_source->getInode ();
  char *fname = strrchr (file_name, (int) '/');
  char *mname = strrchr (main_source->get_name (), (int) '/');
  if (fname && mname && !streq (fname, mname))
    {
      SourceFile *sf = findSource (file_name, false);
      if (sf != NULL)
	Inode = sf->getInode ();
    }

  comComs = new Vector<ComC*>;
  Stabs *stabs = openDebugInfo ();
  if (stabs == NULL)
    return false;
  int st = stabs->read_stabs (Inode, this, comComs, true);
  if (!hasDwarf && hasStabs && !streq (stabsPath, disPath))
    {
      // Read stabs from .o file
      if (dot_o_file == NULL)
	{
	  if (dbeFile->get_location ())
	    {
	      dot_o_file = createLoadObject (dbeFile->get_name ());
	      dot_o_file->dbeFile->set_location (dbeFile->get_location ());
	      dot_o_file->dbeFile->sbuf = dbeFile->sbuf;
	      dot_o_file->dbeFile->container = dbeFile->container;
	    }
	}
      if (dot_o_file
	  && dot_o_file->sync_read_stabs () == LoadObject::ARCHIVE_SUCCESS)
	{
	  Stabs *stabs_o = dot_o_file->objStabs;
	  if (stabs_o)
	    {
	      st = stabs_o->read_stabs (Inode, this,
					comComs->size () > 0 ? NULL : comComs);
	      Elf *elf_o = stabs_o->openElf (false);
	      if (elf_o->dwarf)
		stabs->read_dwarf_from_dot_o (this);
	    }
	}
    }
  if (all)
    read_hwcprof_info ();

  readStabs = true;
  return st == Stabs::DBGD_ERR_NONE;
}

char *
Module::get_disasm (uint64_t inst_address, uint64_t end_address,
		   uint64_t start_address, uint64_t address, int64_t &inst_size)
{
  return disasm->get_disasm (inst_address, end_address, start_address,
			     address, inst_size);
}

void
Module::read_stabs (bool all)
{
  if (openSourceFlag == AE_NOTREAD)
    {
      openSourceFlag = AE_OK;
      if (lang_code == Sp_lang_java)
	{
	  char *clpath = file_name;
	  if (clpath == NULL || strcmp (clpath, "<Unknown>") == 0)
	    clpath = ClassFile::get_java_file_name (name, false);
	  main_source = findSource (clpath, true);
	  main_source->dbeFile->filetype |= DbeFile::F_JAVA_SOURCE;
	  if (clpath != file_name)
	    free (clpath);
	}
      else
	main_source = findSource (file_name, true);
      if (setFile ())
	openStabs (all);
    }
}

bool
Module::openDisPC ()
{
  if (disasm == NULL)
    {
      if (!(loadobject->flags & SEG_FLAG_DYNAMIC) && loadobject->platform != Java)
	{
	  // Read Stabs & Symbol tables
	  if (openDebugInfo () == NULL)
	    return false;
	  if (!objStabs->read_symbols (functions))
	    return false;
	}
      disasm = new Disasm (loadobject->platform, objStabs);
    }
  return true;
}

static SourceFile *cmpSrcContext; // Use only for func_cmp

static int
func_cmp (const void *a, const void *b)
{
  Function *fp1 = *((Function **) a);
  Function *fp2 = *((Function **) b);
  return fp1->func_cmp (fp2, cmpSrcContext);
}

bool
Module::computeMetrics (DbeView *dbev, Function *func, MetricList *metrics,
			Histable::Type type, bool src_metric,
			bool func_scope, SourceFile *source)
{
  name_idx = metrics->get_listorder (NTXT ("name"), Metric::STATIC);
  if (name_idx < 0)
    {
      metrics->print_metric_list (stderr,
				  GTXT ("Fatal: no name metric in Module::computeMetrics mlist:\n"),
				  1);
      abort ();
    }

  // Now find the metrics for size and address, if present
  size_index = metrics->get_listorder (NTXT ("size"), Metric::STATIC);
  addr_index = metrics->get_listorder (NTXT ("address"), Metric::STATIC);

  // free the old cached data for both src and disassembly
  //   If it's disassembly with visible source metrics, we use both
  if (dis_items)
    {
      delete dis_items;
      dis_items = NULL;
    }
  if (src_items)
    {
      delete src_items;
      src_items = NULL;
    }

  // ask the DbeView to generate new data to be cached
  if (src_metric || type == Histable::LINE)
    {
      Histable *obj = (func_scope) ? (Histable*) func : (Histable*)this;
      if (lang_code == Sp_lang_java)
	obj = func_scope ? (Histable *) func :
	      (source && source->get_type () == Histable::SOURCEFILE ?
	       (Histable *) source : (Histable *) this);
      src_items = dbev->get_hist_data (metrics, Histable::LINE, 0,
				       Hist_data::MODL, obj, source);
    }
  if (type == Histable::INSTR)
    dis_items = dbev->get_hist_data (metrics, Histable::INSTR, 0,
			       Hist_data::MODL,
			       func_scope ? (Histable*) func : (Histable*) this,
			       source);

  Hist_data *cur_hist_data;
  if (type == Histable::INSTR)
    cur_hist_data = dis_items;
  else
    cur_hist_data = src_items;

  Vector<Metric*> *items = cur_hist_data->get_metric_list ()->get_items ();
  long sz = items->size ();
  empty = new TValue[sz];
  memset (empty, 0, sizeof (TValue) * sz);
  for (long i = 0; i < sz; i++)
    empty[i].tag = items->get (i)->get_vtype ();
  return true;
}

// Method to get annotated source or disassembly for the module
//	or a function within it
Hist_data *
Module::get_data (DbeView *dbev, MetricList *mlist, Histable::Type type,
		  TValue *ftotal, SourceFile *srcFile, Function *func,
		  Vector<int> *marks, int threshold, int vis_bits,
		  int src_visible, bool hex_vis, bool func_scope,
		  bool /*src_only*/, Vector<int_pair_t> *marks2d,
		  Vector<int_pair_t> *marks2d_inc)
{
  cur_dbev = dbev;
  srcContext = srcFile ? srcFile : main_source;
  read_stabs ();
  status = AE_OK;
  dbev->warning_msg = NULL;
  dbev->error_msg = NULL;
  if (type == Histable::LINE)
    {
      if (!srcContext->readSource ())
	{
	  status = AE_NOSRC;
	  dbev->error_msg = anno_str (srcContext->get_name ());
	  return NULL;
	}
      if (!computeMetrics (dbev, func, mlist, type, false, func_scope, srcContext))
	{
	  status = AE_OTHER;
	  dbev->error_msg = anno_str ();
	  return NULL;
	}
      status = checkTimeStamp (false);
    }
  else
    { // Histable::INSTR
      Anno_Errors src_status = AE_OK;
      if (!srcContext->readSource ())
	{
	  src_status = AE_NOSRC;
	  dbev->error_msg = anno_str (srcContext->get_name ());
	}
      if (!setFile ())
	status = AE_NOLOBJ;
      else
	{
	  if (!openStabs ())
	    src_status = AE_NOSTABS;
	  if (!openDisPC ())
	    status = AE_NOSYMTAB;
	}
      if (status != AE_OK)
	{
	  dbev->error_msg = anno_str ();
	  return NULL;
	}
      if (src_status != AE_OK && func != NULL)
	{
	  if (loadobject->platform == Java && (func->flags & FUNC_FLAG_NATIVE) != 0)
	    {
	      append_msg (CMSG_ERROR,
			  GTXT ("`%s' is a native method; byte code not available\n"),
			  func->get_name ());
	      status = AE_NOOBJ;
	      dbev->error_msg = anno_str ();
	      return NULL;
	    }
	  func_scope = true;
	}
      // get the disassembly-line metric data
      if (!computeMetrics (dbev, func, mlist, type,
			   (src_visible & SRC_METRIC) != 0,
			   func_scope, srcContext))
	{
	  status = AE_OTHER;
	  dbev->error_msg = anno_str ();
	  return NULL;
	}
      status = checkTimeStamp (true);
    }
  total = ftotal;

  // initialize line number
  init_line ();

  // initialize data -- get duplicate metric list for the line texts
  // pick up the metric list from the computed data
  MetricList *nmlist = NULL;
  if (type == Histable::INSTR)
    {
      mlist = dis_items->get_metric_list ();
      nmlist = new MetricList (mlist);
      data_items = new Hist_data (nmlist, Histable::INSTR, Hist_data::MODL);
      data_items->set_status (dis_items->get_status ());
      set_dis_data (func, vis_bits, dbev->get_cmpline_visible (),
		    src_visible, hex_vis, func_scope,
		    dbev->get_funcline_visible ());
    }
  else
    {
      mlist = src_items->get_metric_list ();
      nmlist = new MetricList (mlist);
      data_items = new Hist_data (nmlist, Histable::LINE, Hist_data::MODL);
      data_items->set_status (src_items->get_status ());
      set_src_data (func_scope ? func : NULL, vis_bits,
		    dbev->get_cmpline_visible (),
		    dbev->get_funcline_visible ());
    }
  data_items->compute_minmax ();

  Metric *mitem;
  int index;
  Hist_data::HistItem *max_item;
  TValue *value;
  Hist_data::HistItem *max_item_inc;
  TValue *value_inc;
  double dthreshold = threshold / 100.0;

  int sz = data_items->get_metric_list ()->get_items ()->size ();
  maximum = new TValue[sz];
  maximum_inc = new TValue[sz];
  memset (maximum, 0, sizeof (TValue) * sz);
  memset (maximum_inc, 0, sizeof (TValue) * sz);
  max_item = data_items->get_maximums ();
  max_item_inc = data_items->get_maximums_inc ();

  Vec_loop (Metric*, data_items->get_metric_list ()->get_items (), index, mitem)
  {
    maximum_inc[index].tag = maximum[index].tag = mitem->get_vtype ();

    if (mitem->get_subtype () == Metric::STATIC)
      continue;
    if (!mitem->is_visible () && !mitem->is_tvisible ()
	&& !mitem->is_pvisible ())
      continue;

    value = &max_item->value[index];
    value_inc = &max_item_inc->value[index];

    double dthresh;
    if (mitem->is_zeroThreshold () == true)
      dthresh = 0;
    else
      dthresh = dthreshold;
    switch (value->tag)
      {
      case VT_INT:
	maximum[index].i = (int) (dthresh * (double) value->i);
	maximum_inc[index].i = (int) (dthresh * (double) value_inc->i);
	break;
      case VT_DOUBLE:
	maximum[index].d = dthresh * value->d;
	maximum_inc[index].d = dthresh * value_inc->d;
	break;
      case VT_LLONG:
	maximum[index].ll = (unsigned long long) (dthresh * (double) value->ll);
	maximum_inc[index].ll = (unsigned long long)
		(dthresh * (double) value_inc->ll);
	break;
      case VT_ULLONG:
	maximum[index].ull = (unsigned long long)
		(dthresh * (double) value->ull);
	maximum_inc[index].ull = (unsigned long long)
		(dthresh * (double) value_inc->ull);
	break;
      default:
	// not needed for non-numerical metrics
	break;
      }
  }

  // mark all high values
  for (int index1 = 0; index1 < data_items->size (); index1++)
    {
      Hist_data::HistItem *hi = data_items->fetch (index1);
      int index2;
      Vec_loop (Metric*, nmlist->get_items (), index2, mitem)
      {
	bool mark = false;
	if (mitem->get_subtype () == Metric::STATIC)
	  continue;
	if (!mitem->is_visible () && !mitem->is_tvisible ()
	    && !mitem->is_pvisible ())
	  continue;

	switch (hi->value[index2].tag)
	  {
	  case VT_DOUBLE:
	    if (nmlist->get_type () == MET_SRCDIS
		&& data_items->get_callsite_mark ()->get (hi->obj))
	      {
		if (hi->value[index2].d > maximum_inc[index2].d)
		  mark = true;
		break;
	      }
	    if (hi->value[index2].d > maximum[index2].d)
	      mark = true;
	    break;
	  case VT_INT:
	    if (nmlist->get_type () == MET_SRCDIS
		&& data_items->get_callsite_mark ()->get (hi->obj))
	      {
		if (hi->value[index2].i > maximum_inc[index2].i)
		  mark = true;
		break;
	      }
	    if (hi->value[index2].i > maximum[index2].i)
	      mark = true;
	    break;
	  case VT_LLONG:
	    if (nmlist->get_type () == MET_SRCDIS
		&& data_items->get_callsite_mark ()->get (hi->obj))
	      {
		if (hi->value[index2].ll > maximum_inc[index2].ll)
		  mark = true;
		break;
	      }
	    if (hi->value[index2].ll > maximum[index2].ll)
	      mark = true;
	    break;
	  case VT_ULLONG:
	    if (nmlist->get_type () == MET_SRCDIS
		&& data_items->get_callsite_mark ()->get (hi->obj))
	      {
		if (hi->value[index2].ull > maximum_inc[index2].ull)
		  mark = true;
		break;
	      }
	    if (hi->value[index2].ull > maximum[index2].ull)
	      mark = true;
	    break;
	    // ignoring the following cases (why?)
	  case VT_SHORT:
	  case VT_FLOAT:
	  case VT_HRTIME:
	  case VT_LABEL:
	  case VT_ADDRESS:
	  case VT_OFFSET:
	    break;
	  }
	if (mark)
	  {
	    marks->append (index1);
	    break;
	  }
      }
    }

  // mark all high values to marks2d
  if (marks2d != NULL && marks2d_inc != NULL)
    {
      for (int index1 = 0; index1 < data_items->size (); index1++)
	{
	  Hist_data::HistItem *hi = data_items->fetch (index1);
	  int index2;
	  Vec_loop (Metric*, nmlist->get_items (), index2, mitem)
	  {
	    Metric::SubType subType = mitem->get_subtype ();
	    if (subType == Metric::STATIC)
	      continue;
	    if (!mitem->is_visible () && !mitem->is_tvisible ()
		&& !mitem->is_pvisible ())
	      continue;
	    switch (hi->value[index2].tag)
	      {
	      case VT_DOUBLE:
		if (nmlist->get_type () == MET_SRCDIS
		    && data_items->get_callsite_mark ()->get (hi->obj))
		  {
		    if (hi->value[index2].d > maximum_inc[index2].d)
		      {
			int_pair_t pair = {index1, index2};
			marks2d_inc->append (pair);
		      }
		    break;
		  }
		if (hi->value[index2].d > maximum[index2].d)
		  {
		    int_pair_t pair = {index1, index2};
		    marks2d->append (pair);
		  }
		break;
	      case VT_INT:
		if (nmlist->get_type () == MET_SRCDIS
		    && data_items->get_callsite_mark ()->get (hi->obj))
		  {
		    if (hi->value[index2].i > maximum_inc[index2].i)
		      {
			int_pair_t pair = {index1, index2};
			marks2d_inc->append (pair);
		      }
		    break;
		  }
		if (hi->value[index2].i > maximum[index2].i)
		  {
		    int_pair_t pair = {index1, index2};
		    marks2d->append (pair);
		  }
		break;
	      case VT_LLONG:
		if (nmlist->get_type () == MET_SRCDIS
		    && data_items->get_callsite_mark ()->get (hi->obj))
		  {
		    if (hi->value[index2].ll > maximum_inc[index2].ll)
		      {
			int_pair_t pair = {index1, index2};
			marks2d_inc->append (pair);
		      }
		    break;
		  }
		if (hi->value[index2].ll > maximum[index2].ll)
		  {
		    int_pair_t pair = {index1, index2};
		    marks2d->append (pair);
		  }
		break;
	      case VT_ULLONG:
		if (nmlist->get_type () == MET_SRCDIS
		    && data_items->get_callsite_mark ()->get (hi->obj))
		  {
		    if (hi->value[index2].ull > maximum_inc[index2].ull)
		      {
			int_pair_t pair = {index1, index2};
			marks2d_inc->append (pair);
		      }
		    break;
		  }
		if (hi->value[index2].ull > maximum[index2].ull)
		  {
		    int_pair_t pair = {index1, index2};
		    marks2d->append (pair);
		  }
		break;
	      case VT_SHORT:
	      case VT_FLOAT:
	      case VT_HRTIME:
	      case VT_LABEL:
	      case VT_ADDRESS:
	      case VT_OFFSET:
		break;
	      }
	  }
	}
    }

  // free memory used by Computing & Printing metrics
  delete[] maximum;
  delete[] maximum_inc;
  delete[] empty;
  maximum = NULL;
  maximum_inc = NULL;
  empty = NULL;
  dbev->warning_msg = anno_str ();
  return data_items;
}

Vector<uint64_t> *
Module::getAddrs (Function *func)
{
  uint64_t start_address = func->img_offset;
  uint64_t end_address = start_address + func->size;
  int64_t inst_size = 0;

  // initialize "disasm" if necessary
  if (!openDisPC ())
    return NULL;

  Vector<uint64_t> *addrs = new Vector<uint64_t>;
  for (uint64_t inst_address = start_address; inst_address < end_address;)
    {
      char *s = disasm->get_disasm (inst_address, end_address, start_address,
				    func->img_offset, inst_size);
      free (s);
      addrs->append (inst_address - start_address);
      inst_address += inst_size;
      if (inst_size == 0)
	break;
    }
  return addrs;
}

void
Module::init_line ()
{
  // initialize the compiler commentary data
  cindex = 0;
  if (comComs != NULL && comComs->size () > 0)
    cline = comComs->fetch (cindex)->line;
  else
    cline = -1;

  sindex = 0;
  if (src_items && src_items->size () > 0)
    sline = ((DbeLine*) src_items->fetch (0)->obj)->lineno;
  else
    sline = -1;

  dindex = 0;
  mindex = 0;
  mline = -1;
  if (dis_items && dis_items->size () > 0)
    {
      daddr = (DbeInstr*) dis_items->fetch (0)->obj;

      // After sorting all HistItems with PCLineFlag appear
      // at the end of the list. Find the first one.
      for (mindex = dis_items->size () - 1; mindex >= 0; mindex--)
	{
	  Hist_data::HistItem *item = dis_items->fetch (mindex);
	  if (!(((DbeInstr*) item->obj)->flags & PCLineFlag))
	    break;
	  mline = (unsigned) (((DbeInstr*) item->obj)->addr);
	}
      mindex++;
    }
  else
    daddr = NULL;
}

void
Module::set_src_data (Function *func, int vis_bits, int cmpline_visible,
		      int funcline_visible)
{
  Function *curr_func = NULL;

  // start at the top of the file, and loop over all lines in the file (source context)
  for (curline = 1; curline <= srcContext->getLineCount (); curline++)
    {
      // Before writing the line, see if there's compiler commentary to insert
      if (cline == curline)
	set_ComCom (vis_bits);

      // Find out if we need to print zero metrics with the line
      DbeLine *dbeline = srcContext->find_dbeline (NULL, curline);
      Anno_Types type = AT_SRC_ONLY;
      if (dbeline->dbeline_func_next)
	{
	  if (func)
	    for (DbeLine *dl = dbeline->dbeline_func_next; dl; dl = dl->dbeline_func_next)
	      {
		if (dl->func == func)
		  {
		    type = AT_SRC;
		    break;
		  }
	      }
	  else
	    type = AT_SRC;
	}

      if (funcline_visible)
	{ // show red lines
	  // is there a function index line to insert?
	  Function *func_next = NULL;
	  for (DbeLine *dl = dbeline; dl; dl = dl->dbeline_func_next)
	    {
	      Function *f = dl->func;
	      if (f && f->line_first == curline
		  && f->getDefSrc () == srcContext)
		{
		  if (lang_code == Sp_lang_java
		      && (f->flags & FUNC_FLAG_DYNAMIC))
		    continue;
		  if (cur_dbev && cur_dbev->get_path_tree ()->get_func_nodeidx (f))
		    {
		      func_next = f;
		      break;
		    }
		  else if (func_next == NULL)
		    func_next = f;
		}
	    }
	  if (func_next && curr_func != func_next)
	    {
	      curr_func = func_next;
	      char *func_name = curr_func->get_name ();
	      if (is_fortran () && streq (func_name, NTXT ("MAIN_")))
		func_name = curr_func->get_match_name ();
	      Hist_data::HistItem *item =
		      src_items->new_hist_item (curr_func, AT_FUNC, empty);
	      item->value[name_idx].l = dbe_sprintf (GTXT ("<Function: %s>"),
						     func_name);
	      data_items->append_hist_item (item);
	    }
	} // end of red line
      set_src (type, dbeline); // add the source line
    } //  end of loop over source lines

  // See if compiler flags are set; if so, append them
  if (cmpline_visible && comp_flags)
    {
      Hist_data::HistItem *item = src_items->new_hist_item (NULL, AT_EMPTY,
							    empty);
      item->value[name_idx].l = strdup (NTXT (""));
      data_items->append_hist_item (item);
      item = src_items->new_hist_item (NULL, AT_COM, empty);
      item->value[name_idx].l = dbe_sprintf (GTXT ("Compile flags: %s"),
					     comp_flags);
      data_items->append_hist_item (item);
    }
}

void
Module::set_dis_data (Function *func, int vis_bits, int cmpline_visible,
		      int src_visible, bool hex_vis, bool func_scope,
		      int funcline_visible)
{
  bool nextFile = false;

  // initialize the source output, if any
  curline = (srcContext->getLineCount () > 0) ? 1 : -1;
  if (func)
    nextFile = srcContext != func->getDefSrc ();
  curr_inc = srcContext;

  bool src_code = (src_visible & SRC_CODE);
  Anno_Types src_type = (src_visible & SRC_METRIC) ? AT_SRC : AT_SRC_ONLY;

  char *img_fname = func ? func->img_fname : NULL;

  // Build a new Function list
  Vector<Function*> *FuncLst = new Vector<Function*>;
  if (func_scope)
    {
      if (func)
	FuncLst->append (func);
    }
  else
    {
      for (int i = 0, sz = functions ? functions->size () : 0; i < sz; i++)
	{
	  Function *fitem = functions->fetch (i);
	  if (fitem != fitem->cardinal ())
	    continue;
	  if (img_fname == NULL)
	    img_fname = fitem->img_fname;
	  if (fitem->img_fname == NULL || strcmp (fitem->img_fname, img_fname))
	    continue;
	  FuncLst->append (fitem);
	}
    }
  if (FuncLst->size () == 0)
    { // no function is good
      delete FuncLst;
      return;
    }
  cmpSrcContext = srcContext;
  FuncLst->sort (func_cmp);

  disasm->set_hex_visible (hex_vis);
  for (int index = 0, sz = FuncLst->size (); index < sz; index++)
    {
      Function *fitem = FuncLst->fetch (index);
      uint64_t start_address, end_address;
      int64_t inst_size;
      if (fitem->getDefSrc () != srcContext && curline > 0)
	{
	  // now flush the left source line, if available
	  for (; curline <= srcContext->getLineCount (); curline++)
	    {
	      // see if there's a compiler comment line to dump
	      if (cline == curline)
		set_ComCom (vis_bits);
	      if (src_code)
		set_src (src_type, srcContext->find_dbeline (curline));
	    }
	  curline = -1;
	}

      curr_inc = NULL;
      // disassemble one function
      start_address = objStabs ?
	      objStabs->mapOffsetToAddress (fitem->img_offset) : 0;
      end_address = start_address + fitem->size;
      inst_size = 0;

      disasm->set_addr_end (end_address);
      if ((loadobject->flags & SEG_FLAG_DYNAMIC)
	   && loadobject->platform != Java)
	disasm->set_img_name (img_fname);

      for (uint64_t inst_address = start_address; inst_address < end_address;)
	{
	  uint64_t address = inst_address - start_address;
	  DbeInstr *instr = fitem->find_dbeinstr (0, address);
	  DbeLine *dbeline = (DbeLine *) (instr->convertto (Histable::LINE));
	  if (instr->lineno == -1 && dbeline && dbeline->lineno > 0)
	    instr->lineno = dbeline->lineno;

	  // now write the unannotated source line, if available
	  if (curline > 0)
	    { // source is present
	      int lineno = curline - 1;
	      if (instr->lineno != -1)
		{
		  if (dbeline && streq (dbeline->sourceFile->get_name (),
					srcContext->get_name ()))
		    lineno = instr->lineno;
		}
	      else if (curr_inc == NULL && srcContext == fitem->def_source
		       && fitem->line_first > 0)
		lineno = fitem->line_first;

	      for (; curline <= lineno; curline++)
		{
		  // see if there's a compiler comment line to dump
		  if (cline == curline)
		    set_ComCom (vis_bits);
		  if (mline == curline)
		    set_MPSlave ();
		  if (src_code)
		    set_src (src_type, srcContext->find_dbeline (curline));
		  if (curline >= srcContext->getLineCount ())
		    {
		      curline = -1;
		      break;
		    }
		}
	    }

	  if (funcline_visible)
	    { // show red lines
	      if (!curr_inc || (dbeline && curr_inc != dbeline->sourceFile))
		{
		  Hist_data::HistItem *item = dis_items->new_hist_item (dbeline, AT_FUNC, empty);
		  curr_inc = dbeline ? dbeline->sourceFile : srcContext;
		  char *str;
		  if (curr_inc != srcContext)
		    {
		      char *fileName = curr_inc->dbeFile->getResolvedPath ();
		      str = dbe_sprintf (GTXT ("<Function: %s, instructions from source file %s>"),
					 fitem->get_name (), fileName);
		    }
		  else
		    str = dbe_sprintf (GTXT ("<Function: %s>"),
				       fitem->get_name ());
		  item->value[name_idx].l = str;
		  data_items->append_hist_item (item);
		}
	    }

	  char *dis_str = get_disasm (inst_address, end_address, start_address,
				      fitem->img_offset, inst_size);
	  if (inst_size == 0)
	    break;
	  else if (instr->size == 0)
	    instr->size = (unsigned int) inst_size;
	  inst_address += inst_size;

	  // stomp out control characters
	  for (size_t i = 0, len = strlen (dis_str); i < len; i++)
	    {
	      if (dis_str[i] == '\t')
		dis_str[i] = ' ';
	    }

	  for (int i = 0; i < bTargets.size (); i++)
	    {
	      target_info_t *bTarget = bTargets.fetch (i);
	      if (bTarget->offset == fitem->img_offset + address)
		{
		  // insert a new line for the bTarget
		  size_t colon = strcspn (dis_str, NTXT (":"));
		  char *msg = GTXT ("*  <branch target>");
		  size_t len = colon + strlen (msg);
		  len = (len < 50) ? (50 - len) : 1;
		  char *new_dis_str = dbe_sprintf ("%.*s%s%*c  <===----<<<",
						   (int) colon, dis_str, msg,
						   (int) len, ' ');
		  DbeInstr *bt = fitem->find_dbeinstr (PCTrgtFlag, address);
		  bt->lineno = instr->lineno;
		  bt->size = 0;
		  set_dis (bt, AT_DIS, nextFile, new_dis_str);
		  break;
		}
	    }

	  // AnalyzerInfo/Datatype annotations
	  if (infoList != NULL)
	    {
	      inst_info_t *info = NULL;
	      int pinfo;
	      Vec_loop (inst_info_t*, infoList, pinfo, info)
	      {
		if (info->offset == fitem->img_offset + address) break;
	      }
	      if (info != NULL)
		{ // got a matching memop
		  char typetag[400];
		  typetag[0] = '\0';
		  long t;
		  datatype_t *dtype = NULL;
		  Vec_loop (datatype_t*, datatypes, t, dtype)
		  {
		    if (dtype->datatype_id == info->memop->datatype_id)
		      break;
		  }
		  if (datatypes != NULL)
		    {
		      size_t len = strlen (typetag);
		      if (dtype == NULL || t == datatypes->size ())
			snprintf (typetag + len, sizeof (typetag) - len, "%s",
				  PTXT (DOBJ_UNSPECIFIED));
		      else if (dtype->dobj == NULL)
			snprintf (typetag + len, sizeof (typetag) - len, "%s",
				  PTXT (DOBJ_UNDETERMINED));
		      else
			snprintf (typetag + len, sizeof (typetag) - len, "%s",
				  dtype->dobj->get_name ());
		    }
		  if (strlen (typetag) > 1)
		    {
		      char *new_dis_str;
		      new_dis_str = dbe_sprintf ("%-50s  %s", dis_str, typetag);
		      free (dis_str);
		      dis_str = new_dis_str;
		    }
		}
	    }
	  set_dis (instr, AT_DIS, nextFile, dis_str);
	}
    }

  // now flush the left source line, if available
  if (curline > 0)
    { // source is present
      for (; curline <= srcContext->getLineCount (); curline++)
	{
	  // see if there's a compiler comment line to dump
	  if (cline == curline)
	    set_ComCom (vis_bits);

	  if (src_code)
	    set_src (src_type, srcContext->find_dbeline (curline));
	}
    }

  // See if compiler flags are set; if so, append them
  if (cmpline_visible && comp_flags)
    {
      Hist_data::HistItem *item = dis_items->new_hist_item (NULL, AT_EMPTY,
							    empty);
      item->value[name_idx].l = dbe_strdup (NTXT (""));
      data_items->append_hist_item (item);
      item = dis_items->new_hist_item (NULL, AT_COM, empty);
      item->value[name_idx].l = dbe_sprintf (GTXT ("Compile flags: %s"),
					     comp_flags);
      data_items->append_hist_item (item);
    }
  delete FuncLst;
}

// set_src -- inserts one or more lines into the growing data list
void
Module::set_src (Anno_Types type, DbeLine *dbeline)
{
  Hist_data::HistItem *item;

  // Flush items that are not represented in source
  while (sline >= 0 && sline < curline)
    {
      item = src_items->fetch (sindex);
      if (((DbeLine*) item->obj)->lineno > 0)
	set_one (item, AT_QUOTE, item->obj->get_name ());

      if (++sindex < src_items->size ()) // get next line with metrics
	sline = ((DbeLine*) src_items->fetch (sindex)->obj)->lineno;
      else
	sline = -1;
    }

  //  write values in the metric fields for the given source line
  if (curline == sline)
    { // got metrics for this line
      item = src_items->fetch (sindex);
      if (((DbeLine*) item->obj)->lineno > 0)
	set_one (item, AT_SRC, srcContext->getLine (curline));

      if (++sindex < src_items->size ()) // get next line metric index
	sline = ((DbeLine*) src_items->fetch (sindex)->obj)->lineno;
      else
	sline = -1;
    }
  else
    {
      item = data_items->new_hist_item (dbeline, type, empty);
      if (size_index != -1)
	item->value[size_index].ll = dbeline->get_size ();
      if (addr_index != -1)
	item->value[addr_index].ll = dbeline->get_addr ();
      item->value[name_idx].l = dbe_strdup (srcContext->getLine (curline));
      data_items->append_hist_item (item);
    }
}

void
Module::set_dis (DbeInstr *instr, Anno_Types type, bool nextFile, char *dis_str)
{
  // Flush items that are not represented in disassembly
  while (daddr && daddr->pc_cmp (instr) < 0)
    {
      if (!nextFile)
	set_one (dis_items->fetch (dindex), AT_QUOTE, daddr->get_name ());
      if (++dindex < dis_items->size ()) // get next line metric index
	daddr = (DbeInstr*) dis_items->fetch (dindex)->obj;
      else
	daddr = NULL;
    }

  // Write values in the metric fields for the given pc index value
  if (instr->inlinedInd >= 0)
    {
      StringBuilder sb;
      sb.append (dis_str);
      instr->add_inlined_info (&sb);
      free (dis_str);
      dis_str = sb.toString ();
    }
  if (daddr && daddr->pc_cmp (instr) == 0)
    {
      Hist_data::HistItem *item = data_items->new_hist_item (instr, type,
					      dis_items->fetch (dindex)->value);
      item->value[name_idx].tag = VT_LABEL;
      item->value[name_idx].l = dis_str;
      data_items->append_hist_item (item);
      if (dis_items->get_callsite_mark ()->get (dis_items->fetch (dindex)->obj))
	data_items->get_callsite_mark ()->put (item->obj, 1);

      if (++dindex < dis_items->size ()) // get next line metric index
	daddr = (DbeInstr*) dis_items->fetch (dindex)->obj;
      else
	daddr = NULL;
    }
  else
    {
      // create a new item for this PC
      Hist_data::HistItem *item = dis_items->new_hist_item (instr, type, empty);
      if (size_index != -1)
	item->value[size_index].ll = instr->size;
      if (addr_index != -1)
	item->value[addr_index].ll = instr->get_addr ();
      item->value[name_idx].tag = VT_LABEL;
      item->value[name_idx].l = dis_str;
      data_items->append_hist_item (item);
    }
}

void
Module::set_MPSlave ()
{
  Hist_data::HistItem *item;
  Function *fp;
  int index;

  // write the inclusive metrics for slave threads
  while (mline == curline)
    {
      item = dis_items->fetch (mindex);
      DbeInstr *instr = (DbeInstr *) item->obj;
      Vec_loop (Function*, functions, index, fp)
      {
	if (fp->derivedNode == instr)
	  {
	    set_one (item, AT_QUOTE, (fp->isOutlineFunction) ?
		     GTXT ("<inclusive metrics for outlined functions>") :
		     GTXT ("<inclusive metrics for slave threads>"));
	    break;
	  }
      }

      mindex++;
      if (mindex < dis_items->size ())
	mline = (unsigned) ((DbeInstr*) (dis_items->fetch (mindex)->obj))->addr;
      else
	mline = -1;
    }
}//set_MPSlave

void
Module::set_one (Hist_data::HistItem *org_item, Anno_Types type,
		 const char *text)
{
  if (org_item == NULL)
    return;
  Hist_data::HistItem *item = data_items->new_hist_item (org_item->obj, type,
							 org_item->value);
  item->value[name_idx].tag = VT_LABEL;
  item->value[name_idx].l = dbe_strdup (text);
  data_items->append_hist_item (item);
  if (org_item != NULL && src_items != NULL
      && src_items->get_callsite_mark ()->get (org_item->obj))
    data_items->get_callsite_mark ()->put (item->obj, 1);
}//set_one

void
Module::set_ComCom (int vis_bits)
{
  Hist_data::HistItem *item;
  Function *func = dbeSession->get_Unknown_Function ();

  if (vis_bits)
    {
      // precede the compiler commentary with a blank line
      item = data_items->new_hist_item (func, AT_EMPTY, empty);
      item->value[name_idx].l = dbe_strdup (NTXT (""));
      data_items->append_hist_item (item);
    }
  while (cline == curline)
    {
      ComC *comm = comComs->fetch (cindex);
      if (comm->visible & vis_bits)
	{
	  // write the compiler commentary
	  item = data_items->new_hist_item (func, AT_COM, empty);
	  item->value[name_idx].l = dbe_strdup (comm->com_str);
	  data_items->append_hist_item (item);
	}
      if (++cindex < comComs->size ())
	cline = comComs->fetch (cindex)->line;
      else
	cline = -1;
    }
}

void
Module::dump_dataobjects (FILE *out)
{
  int index;
  datatype_t *dtype;
  Vec_loop (datatype_t*, datatypes, index, dtype)
  {
    fprintf (out, NTXT ("[0x%08X,%6lld] %4d %6d %s "), dtype->datatype_id,
	     dtype->dobj ? dtype->dobj->id : 0LL,
	     dtype->memop_refs, dtype->event_data,
	     (dtype->dobj != NULL ? (dtype->dobj->get_name () ?
		 dtype->dobj->get_name () : "<NULL>") : "<no object>"));
#if DEBUG
    Histable* scope = dtype->dobj ? dtype->dobj->get_scope () : NULL;
    if (scope != NULL)
      {
	switch (scope->get_type ())
	  {
	  case Histable::LOADOBJECT:
	  case Histable::FUNCTION:
	    fprintf (out, NTXT ("%s"), scope->get_name ());
	    break;
	  case Histable::MODULE:
	    {
	      char *filename = get_basename (scope->get_name ());
	      fprintf (out, NTXT ("%s"), filename);
	      break;
	    }
	  default:
	    fprintf (out, NTXT ("\tUnexpected scope %d:%s"),
		     scope->get_type (), scope->get_name ());
	  }
      }
#endif
    fprintf (out, NTXT ("\n"));
  }
}

void
Module::set_name (char *str)
{
  free (name);
  name = str;
}

void
Module::read_hwcprof_info ()
{
  if (hwcprof == 0)
    {
      hwcprof = 1;
      Stabs *stabs = openDebugInfo ();
      if (stabs)
	stabs->read_hwcprof_info (this);
    }
}

void
Module::reset_datatypes ()
{
  for (int i = 0, sz = datatypes ? datatypes->size () : -1; i < sz; i++)
    {
      datatype_t *t = datatypes->fetch (i);
      t->event_data = 0;
    }
}

DataObject *
Module::get_dobj (uint32_t dtype_id)
{
  read_hwcprof_info ();
  for (int i = 0, sz = datatypes ? datatypes->size () : -1; i < sz; i++)
    {
      datatype_t *t = datatypes->fetch (i);
      if (t->datatype_id == dtype_id)
	{
	  t->event_data++;
	  return t->dobj;
	}
    }
  return NULL;
}

int
Module::readFile ()
{
  return AE_OK;
}

Vector<Histable*> *
Module::get_comparable_objs ()
{
  update_comparable_objs ();
  if (comparable_objs || dbeSession->expGroups->size () <= 1 || loadobject == NULL)
    return comparable_objs;
  Vector<Histable*> *comparableLoadObjs = loadobject->get_comparable_objs ();
  if (comparableLoadObjs == NULL)
    return NULL;
  comparable_objs = new Vector<Histable*>(comparableLoadObjs->size ());
  for (int i = 0, sz = comparableLoadObjs->size (); i < sz; i++)
    {
      Module *mod = NULL;
      LoadObject *lo = (LoadObject*) comparableLoadObjs->fetch (i);
      if (lo)
	{
	  mod = lo->get_comparable_Module (this);
	  if (mod)
	    mod->comparable_objs = comparable_objs;
	}
      comparable_objs->store (i, mod);
    }
  dump_comparable_objs ();
  return comparable_objs;
}

JMethod *
Module::find_jmethod (const char *nm, const char *sig)
{
  // Vladimir: Probably we should not use linear search
  for (long i = 0, sz = VecSize (functions); i < sz; i++)
    {
      JMethod *jmthd = (JMethod*) functions->get (i);
      char *jmt_name = jmthd->get_name (Histable::SHORT);
      if (strcmp (jmt_name, nm) == 0
	  && strcmp (jmthd->get_signature (), sig) == 0)
	return jmthd;
    }
  return NULL;
}
