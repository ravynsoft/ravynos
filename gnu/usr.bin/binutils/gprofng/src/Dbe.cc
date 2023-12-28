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
#include <sys/types.h> // open, chmod
#include <signal.h>
#include <fcntl.h>     // open
#include <strings.h>
#include <unistd.h>

#include "util.h"
#include "Histable.h"
#include "DbeSession.h"
#include "DbeView.h"
#include "BaseMetric.h"
#include "CallStack.h"
#include "collctrl.h"
#include "Command.h"
#include "Dbe.h"
#include "DbeApplication.h"
#include "DefaultMap.h"
#include "LoadObject.h"
#include "Experiment.h"
#include "IndexObject.h"
#include "IOActivity.h"
#include "PreviewExp.h"
#include "Function.h"
#include "Hist_data.h"
#include "MetricList.h"
#include "Module.h"
#include "DataSpace.h"
#include "MemorySpace.h"
#include "DataObject.h"
#include "MemObject.h"
#include "Filter.h"
#include "FilterSet.h"
#include "FilterExp.h"
#include "Sample.h"
#include "Print.h"
#include "StringBuilder.h"
#include "dbe_types.h"
#include "ExpGroup.h"
#include "vec.h"
#include "UserLabel.h"
#include "DbeFile.h"
#include "PathTree.h"

// Data structures for managing the collector control info for Collection GUI
static Coll_Ctrl *col_ctr = NULL;

template<> VecType Vector<int>::type ()
{
  return VEC_INTEGER;
}

template<> VecType Vector<unsigned>::type ()
{
  return VEC_INTEGER;
}

template<> VecType Vector<char>::type ()
{
  return VEC_CHAR;
}

template<> VecType Vector<bool>::type ()
{
  return VEC_BOOL;
}

template<> VecType Vector<double>::type ()
{
  return VEC_DOUBLE;
}

template<> VecType Vector<long long>::type ()
{
  return VEC_LLONG;
}

template<> VecType Vector<uint64_t>::type ()
{
  return VEC_LLONG;
}

template<> VecType Vector<void*>::type ()
{
  return VEC_VOIDARR;
}

template<> VecType Vector<char*>::type ()
{
  return VEC_STRING;
}

template<> VecType Vector<Vector<int>*>::type ()
{
  return VEC_INTARR;
}

template<> VecType Vector<Vector<char*>*>::type ()
{
  return VEC_STRINGARR;
}

template<> VecType Vector<Vector<long long>*>::type ()
{
  return VEC_LLONGARR;
}

// gcc won't instantiate Vector<unsigned>::type() without it
Vector<unsigned> __dummy_unsigned_vector;

#define CASE_S(x)   case x: return #x
static const char *
dsp_type_to_string (int t)
{
  switch (t)
    {
      CASE_S (DSP_FUNCTION);
      CASE_S (DSP_LINE);
      CASE_S (DSP_PC);
      CASE_S (DSP_SOURCE);
      CASE_S (DSP_DISASM);
      CASE_S (DSP_SELF);
      CASE_S (DSP_CALLER);
      CASE_S (DSP_CALLEE);
      CASE_S (DSP_CALLTREE);
      CASE_S (DSP_TIMELINE);
      CASE_S (DSP_STATIS);
      CASE_S (DSP_EXP);
      CASE_S (DSP_LEAKLIST);
      CASE_S (DSP_MEMOBJ);
      CASE_S (DSP_DATAOBJ);
      CASE_S (DSP_DLAYOUT);
      CASE_S (DSP_SRC_FILE);
      CASE_S (DSP_IFREQ);
      CASE_S (DSP_RACES);
      CASE_S (DSP_INDXOBJ);
      CASE_S (DSP_DUALSOURCE);
      CASE_S (DSP_SOURCE_DISASM);
      CASE_S (DSP_DEADLOCKS);
      CASE_S (DSP_SOURCE_V2);
      CASE_S (DSP_DISASM_V2);
      CASE_S (DSP_IOACTIVITY);
      CASE_S (DSP_OVERVIEW);
      CASE_S (DSP_IOCALLSTACK);
      CASE_S (DSP_HEAPCALLSTACK);
      CASE_S (DSP_SAMPLE);
    default:
      break;
    }
  return NTXT ("ERROR");
}

enum
{
  COMPARE_BIT       = 1 << 8,
  MTYPE_MASK        = (1 << 8) - 1,
  GROUP_ID_SHIFT    = 16
};

static DbeView *
getDbeView (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  return dbev;
}


Vector<char*> *
dbeGetInitMessages ()
{
  // If any comments from the .rc files, send them to the GUI
  Emsg *msg = theDbeApplication->fetch_comments ();
  int size = 0;
  while (msg != NULL)
    {
      size++;
      msg = msg->next;
    }

  // Initialize Java String array
  Vector<char*> *list = new Vector<char*>(size);
  msg = theDbeApplication->fetch_comments ();
  size = 0;
  int i = 0;
  while (msg != NULL)
    {
      char *str = msg->get_msg ();
      list->store (i, dbe_strdup (str));
      i++;
      msg = msg->next;
    }

  // now delete the comments
  theDbeApplication->delete_comments ();
  return list;
}

Vector<char*> *
dbeGetExpPreview (int /*dbevindex*/, char *exp_name)
{
  PreviewExp *preview = new PreviewExp ();
  preview->experiment_open (exp_name);
  preview->open_epilogue ();

  // Initialize Java String array
  Vector<char*> *info = preview->preview_info ();
  int size = info->size ();
  Vector<char*> *list = new Vector<char*>(size);

  // Get experiment names
  for (int i = 0; i < size; i++)
    {
      char *str = info->fetch (i);
      if (str == NULL)
	str = GTXT ("N/A");
      list->store (i, dbe_strdup (str));
    }
  delete info;
  delete preview;
  return list;
}

char *
dbeGetExpParams (int /*dbevindex*/, char *exp_name)
{
  PreviewExp *preview = new PreviewExp ();
  preview->experiment_open (exp_name);

  // Initialize Java String array
  char *arg_list = dbe_strdup (preview->getArgList ());
  delete preview;
  return arg_list;
}

/**
 * Gets File Attributes according to the specified format
 * Supported formats:
 * "/bin/ls -dl " - see 'man ls' for details
 * @param filename
 * @param format
 * @return char * attributes
 */
char *
dbeGetFileAttributes (const char *filename, const char *format)
{
  if (format != NULL)
    {
      if (!strcmp (format, NTXT ("/bin/ls -dl ")))
	{
	  // A kind of "/bin/ls -dl " simulation
	  struct stat64 sbuf;
	  sbuf.st_mode = 0;
	  dbe_stat (filename, &sbuf);
	  if (S_IREAD & sbuf.st_mode)
	    { // Readable
	      if (S_ISDIR (sbuf.st_mode) != 0)
		return dbe_sprintf (NTXT ("%s %s\n"), NTXT ("drwxrwxr-x"), filename);
	      else if (S_ISREG (sbuf.st_mode) != 0)
		return dbe_sprintf (NTXT ("%s %s\n"), NTXT ("-rwxrwxr-x"), filename);
	    }
	}
    }
  return dbe_strdup (NTXT (""));
}

/**
 * Gets list of files for specified directory according to the specified format
 * Supported formats:
 * "/bin/ls -a" - see 'man ls' for details
 * "/bin/ls -aF" - see 'man ls' for details
 * @param dirname
 * @param format
 * @return char * files
 */
char *
dbeGetFiles (const char *dirname, const char *format)
{
  if (format != NULL)
    return dbe_read_dir (dirname, format);
  return dbe_strdup (NTXT (""));
}

/**
 * Creates the directory named by this full path name, including any
 * necessary but nonexistent parent directories.
 * @param dirname
 * @return result
 */
char *
dbeCreateDirectories (const char *dirname)
{
  if (dirname != NULL)
    {
      char *res = dbe_create_directories (dirname);
      if (res != NULL)
	return res;
    }
  return dbe_strdup (NTXT (""));
}

/**
 * Deletes the file or the directory named by the specified path name.
 * If this pathname denotes a directory, then the directory must be empty in order to be deleted.
 * @param const char *pathname
 * @return int result
 */
char *
dbeDeleteFile (const char *pathname)
{
  // return unlink(pathname);
  if (pathname != NULL)
    {
      char *res = dbe_delete_file (pathname);
      if (res != NULL)
	return res;
    }
  return dbe_strdup (NTXT (""));
}

/**
 * Reads the file named by the specified path name.
 * Temporary limitation: file should be "text only" and its size should be less than the 1 MB limit.
 * If the operation was successful, the contents is in the first element, and second element is NULL.
 * If the operation failed, then first element is NULL, and second element contains the error message.
 * @param const char *pathname
 * @return Vector<char*> *result
 */
Vector<char*> *
dbeReadFile (const char *pathname)
{
  Vector<char*> *result = new Vector<char*>(2);
  int limit = 1024 * 1024; // Temporary limit: 1 MB
  char * contents = (char *) malloc (limit);
  StringBuilder sb;
  if (NULL == contents)
    {
      sb.sprintf (NTXT ("\nError: Cannot allocate %d bytes\n"), limit);
      result->store (0, NULL);
      result->store (1, sb.toString ()); // failure
      return result;
    }
  int fd = open (pathname, O_RDONLY);
  if (fd >= 0)
    {
      int64_t bytes = read_from_file (fd, contents, limit);
      close (fd);
      if (bytes >= limit)
	{
	  sb.sprintf (NTXT ("\nError: file size is greater than the limit (%d bytes)\n"), limit);
	  result->store (0, NULL);
	  result->store (1, sb.toString ()); // failure
	}
      else
	{
	  contents[bytes] = '\0'; // add string terminator
	  result->store (0, contents);
	  result->store (1, NULL); // success
	}
    }
  else
    {
      sb.sprintf (NTXT ("\nError: Cannot open file %s\n"), pathname);
      result->store (0, NULL);
      result->store (1, sb.toString ()); // failure
      free (contents);
    }
  return result;
}

/**
 * Writes the file named by the specified path name.
 * Temporary limitation: file should be "text only" and its size should be less than the 1 MB limit.
 * If the operation failed, then -1 is returned.
 * @param const char *pathname
 * @return int result  (written bytes)
 */
int
dbeWriteFile (const char *pathname, const char *contents)
{
  int result = -1; // error
  size_t len = 0;
  if (NULL != contents)
    len = strlen (contents);
  size_t limit = 1024 * 1024; // Temporary limit: 1 MB
  if (len > limit) return result;
  unlink (pathname);
  mode_t mode = S_IRUSR | S_IWUSR;
  int fd = open (pathname, O_WRONLY | O_CREAT | O_TRUNC, mode);
  if (fd >= 0)
    {  // replace file contents
      chmod (pathname, /*S_IRUSR || S_IWUSR*/ 0600); // rw for owner only
      ssize_t bytes = 0;
      if (len > 0)
	bytes = write (fd, contents, len);
      close (fd);
      result = (int) bytes;
    }
  return result;
}

/**
 * Gets list of running processes according to the specified format
 * Supported formats:
 * "/bin/ps -ef" - see 'man ps' for details
 * @param format
 * @return char * processes
 */
char *
dbeGetRunningProcesses (const char *format)
{
  if (format != NULL)
    return dbe_get_processes (format);
  return dbe_strdup (NTXT (""));
}

//
// Open experiment
//
char *
dbeOpenExperimentList (int /* dbevindex */, Vector<Vector<char*>*> *groups,
		       bool sessionRestart)
{
  if (sessionRestart)
    dbeSession->reset ();
  char *errstr;
  // Open experiments
  try
    {
      errstr = dbeSession->setExperimentsGroups (groups);
    }
  catch (ExperimentLoadCancelException *)
    {
      errstr = dbe_strdup (NTXT ("Experiment Load Cancelled"));
    }
  return errstr;
}

//
// Drop experiments
//
char *
dbeDropExperiment (int /* dbevindex */, Vector<int> *drop_index)
{
  for (int i = drop_index->size () - 1; i >= 0; i--)
    {
      char *ret = dbeSession->drop_experiment (drop_index->fetch (i));
      if (ret != NULL)
	  return ret;
    }
  return NULL;
}

/**
 * Read .er.rc file from the specified location
 * @param path
 * @return
 */
char *
dbeReadRCFile (int dbevindex, char* path)
{
  DbeView *dbev = getDbeView (dbevindex);
  char *err_msg = dbev->get_settings ()->read_rc (path);
  return err_msg;
}

char *
dbeSetExperimentsGroups (Vector<Vector<char*>*> *groups)
{
  int cmp_mode = dbeSession->get_settings ()->get_compare_mode ();
  if (groups->size () < 2)
    cmp_mode = CMP_DISABLE;
  else if (cmp_mode == CMP_DISABLE)
    cmp_mode = CMP_ENABLE;
  for (int i = 0;; i++)
    {
      DbeView *dbev = dbeSession->getView (i);
      if (dbev == NULL)
	break;
      dbev->get_settings ()->set_compare_mode (cmp_mode);
    }
  char *err_msg = dbeSession->setExperimentsGroups (groups);

  // automatically load machine model if applicable
  dbeDetectLoadMachineModel (0);
  return err_msg;
}

Vector<Vector<char*>*> *
dbeGetExperimensGroups ()
{
  Vector<Vector<char*>*> *grops = dbeSession->getExperimensGroups ();
  return grops;
}

Vector<int> *
dbeGetFounderExpId (Vector<int> *expIds)
{
  Vector<int> *ret = new Vector<int>(expIds->size ());
  for (int i = 0; i < expIds->size (); i++)
    {
      int expId = expIds->fetch (i);
      Experiment *exp = dbeSession->get_exp (expId);
      if (exp != NULL)
	{
	  int founderExpId = exp->getBaseFounder ()->getExpIdx ();
	  ret->store (i, founderExpId);
	}
      else
	ret->store (i, -1);
    }
  return ret;
}

Vector<int> *
dbeGetUserExpId (Vector<int> *expIds)
{
  // returns "User Visible" ids used for EXPID filters and timeline processes
  Vector<int> *ret = new Vector<int>(expIds->size ());
  for (int i = 0; i < expIds->size (); i++)
    {
      int expId = expIds->fetch (i);
      Experiment *exp = dbeSession->get_exp (expId);
      if (exp != NULL)
	{
	  int userExpId = exp->getUserExpId ();
	  ret->store (i, userExpId);
	}
      else
	ret->store (i, -1);
    }
  return ret;
}

//
// Get experiment groupid
//
Vector<int> *
dbeGetExpGroupId (Vector<int> *expIds)
{
  Vector<int> *ret = new Vector<int>(expIds->size ());
  for (int i = 0; i < expIds->size (); i++)
    {
      int expId = expIds->fetch (i);
      Experiment *exp = dbeSession->get_exp (expId);
      if (exp != NULL)
	{
	  int gId = exp->groupId;
	  ret->store (i, gId);
	}
      else
	ret->store (i, -1);
    }
  return ret;
}

Vector<char*> *
dbeGetExpsProperty (const char *prop_name)
{
  long nexps = dbeSession->nexps ();
  if (prop_name == NULL || nexps == 0)
    return NULL;
  Vector<char*> *list = new Vector<char*>(nexps);
  StringBuilder sb;
  int empty = 1;
  int prop = 99;
  if (strcasecmp (prop_name, NTXT ("ERRORS")) == 0)
    prop = 1;
  else if (strcasecmp (prop_name, NTXT ("WARNINGS")) == 0)
    prop = 2;
  if (prop < 3)
    {
      for (long i = 0; i < nexps; i++)
	{
	  Experiment *exp = dbeSession->get_exp (i);
	  char *nm = exp->get_expt_name ();
	  sb.setLength (0);
	  for (Emsg *emsg = (prop == 1) ? exp->fetch_errors () : exp->fetch_warnings ();
		  emsg; emsg = emsg->next)
	    sb.appendf (NTXT ("%s: %s\n"), STR (nm), STR (emsg->get_msg ()));
	  char *s = NULL;
	  if (sb.length () > 0)
	    {
	      s = sb.toString ();
	      empty = 0;
	    }
	  list->append (s);
	}
    }
  if (empty)
    {
      delete list;
      list = NULL;
    }
  return list;
}

//
// Get experiment names
//
Vector<char*> *
dbeGetExpName (int /*dbevindex*/)
{
  int size = dbeSession->nexps ();
  if (size == 0)
    return NULL;
  // Initialize Java String array
  Vector<char*> *list = new Vector<char*>(size);

  // Get experiment names
  for (int i = 0; i < size; i++)
    {
      Experiment *texp = dbeSession->get_exp (i);
      char *buf = dbe_sprintf (NTXT ("%s [%s]"), texp->get_expt_name (),
			       texp->utargname != NULL ? texp->utargname : GTXT ("(unknown)"));
      list->store (i, buf);
    }
  return list;
}

//
// Get experiment state
//
Vector<int> *
dbeGetExpState (int /* dbevindex */)
{
  int size = dbeSession->nexps ();
  if (size == 0)
    return NULL;
  // Initialize Java array
  Vector<int> *state = new Vector<int>(size);

  // Get experiment state
  for (int i = 0; i < size; i++)
    {
      Experiment *exp = dbeSession->get_exp (i);
      int set = EXP_SUCCESS;
      if (exp->get_status () == Experiment::FAILURE)
	set |= EXP_FAILURE;
      if (exp->get_status () == Experiment::INCOMPLETE)
	set |= EXP_INCOMPLETE;
      if (exp->broken)
	set |= EXP_BROKEN;
      if (exp->obsolete)
	set |= EXP_OBSOLETE;
      state->store (i, set);
    }
  return state;
}

//
// Get enabled experiment indices
//
Vector<bool> *
dbeGetExpEnable (int dbevindex)
{
  DbeView *dbev = getDbeView (dbevindex);
  int size = dbeSession->nexps ();
  if (dbev == NULL || size == 0)
    return NULL;

  // Get enabled experiment
  Vector<bool> *enable = new Vector<bool>(size);
  for (int i = 0; i < size; i++)
    {
      bool val = dbev->get_exp_enable (i) && !dbeSession->get_exp (i)->broken;
      enable->store (i, val);
    }
  return enable;
}

//
// Get enabled experiment indices
//
bool
dbeSetExpEnable (int dbevindex, Vector<bool> *enable)
{
  DbeView *dbev = getDbeView (dbevindex);
  bool ret = false;
  int size = dbeSession->nexps ();
  if (dbev == NULL || size == 0)
    return false;

  // set enable, as per input vector
  for (int i = 0; i < size; i++)
    if (!dbeSession->get_exp (i)->broken
	&& dbev->get_exp_enable (i) != enable->fetch (i))
      {
	dbev->set_exp_enable (i, enable->fetch (i));
	ret = true;
      }
  return ret;
}

//
// Get experiment info
//
Vector<char*> *
dbeGetExpInfo (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  int size = dbeSession->nexps ();
  if (size == 0)
    return NULL;

  // Initialize Java String array
  Vector<char*> *list = new Vector<char*>(size * 2 + 1);

  // Get experiment names
  Vector<LoadObject*> *text_segments = dbeSession->get_text_segments ();
  char *msg = pr_load_objects (text_segments, NTXT (""));
  delete text_segments;
  list->store (0, msg);
  int k = 1;
  for (int i = 0; i < size; i++)
    {
      Experiment *exp = dbeSession->get_exp (i);
      char *msg0 = pr_mesgs (exp->fetch_notes (), NTXT (""), NTXT (""));
      char *msg1 = pr_mesgs (exp->fetch_errors (), GTXT ("No errors\n"), NTXT (""));
      char *msg2 = pr_mesgs (exp->fetch_warnings (), GTXT ("No warnings\n"), NTXT (""));
      char *msg3 = pr_mesgs (exp->fetch_comments (), NTXT (""), NTXT (""));
      char *msg4 = pr_mesgs (exp->fetch_pprocq (), NTXT (""), NTXT (""));
      msg = dbe_sprintf (NTXT ("%s%s%s%s"), msg1, msg2, msg3, msg4);
      list->store (k++, msg0);
      list->store (k++, msg);
      free (msg1);
      free (msg2);
      free (msg3);
      free (msg4);
    }
  return list;
}

bool
dbeGetViewModeEnable ()
{
  return dbeSession->has_ompavail () || dbeSession->has_java ();
}

bool
dbeGetJavaEnable ()
{
  return dbeSession->has_java ();
}

int
dbeUpdateNotes (int dbevindex, int exp_id, int type, char* text, bool handle_file)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  int size = dbeSession->nexps ();
  if (size == 0)
    return -1;
  Experiment *exp = dbeSession->get_exp (exp_id);
  return (type == 0) ? exp->save_notes (text, handle_file) : exp->delete_notes (handle_file);
}

//
// Get load object names
//
Vector<char*> *
dbeGetLoadObjectName (int /* dbevindex */)
{
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  int size = lobjs->size ();

  // Initialize Java String array
  Vector<char*> *list = new Vector<char*>(size);

  // Get load object names
  LoadObject *lo;
  int index;
  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    list->store (index, dbe_strdup (lo->get_name ()));
  }
  delete lobjs;
  return list;
}

// XXX Will use later when order has to be passed too,
// Get complete List of tabs
//
Vector<void*> *
dbeGetTabList (int /* dbevindex */)
{
  //DbeView *dbev = getDbeView (dbevindex);
  //Vector<void*> *tabs = dbeSession->get_TabList();
  //return tabs;
  return NULL;
}

//
// Returns list of available tabs
//
Vector<void*> *
dbeGetTabListInfo (int dbevindex)
{
  int index;
  DispTab *dsptab;
  DbeView *dbev = getDbeView (dbevindex);

  // make sure the tabs are initialized properly
  dbev->get_settings ()->proc_tabs (theDbeApplication->rdtMode);
  Vector<DispTab*> *tabs = dbev->get_TabList ();

  // Get number of available tabs
  int size = 0;
  Vec_loop (DispTab*, tabs, index, dsptab)
  {
    if (!dsptab->available)
      continue;
    size++;
  }
  Vector<void*> *data = new Vector<void*>(2);
  Vector<int> *typelist = new Vector<int>(size);
  Vector<char*> *cmdlist = new Vector<char*>(size);
  Vector<int> *ordlist = new Vector<int>(size);

  // Build list of avaliable tabs
  int i = 0;

  Vec_loop (DispTab*, tabs, index, dsptab)
  {
    if (!dsptab->available)
      continue;
    typelist->store (i, dsptab->type);
    cmdlist->store (i, dbe_strdup (Command::get_cmd_str (dsptab->cmdtoken)));
    ordlist->store (i, dsptab->order);
    i++;
  }
  data->store (0, typelist);
  data->store (1, cmdlist);
  data->store (2, ordlist);
  return data;
}

// Return visibility state for all available tabs
//
Vector<bool> *
dbeGetTabSelectionState (int dbevindex)
{
  int index;
  DispTab *dsptab;
  DbeView *dbev = getDbeView (dbevindex);
  Vector<DispTab*> *tabs = dbev->get_TabList ();

  // Get number of available tabs
  int size = 0;
  Vec_loop (DispTab*, tabs, index, dsptab)
  {
    if (!dsptab->available)
      continue;
    size++;
  }
  Vector<bool> *states = new Vector<bool>(size);

  // Get visibility bit for all available tabs
  int i = 0;
  Vec_loop (DispTab*, tabs, index, dsptab)
  {
    if (!dsptab->available)
      continue;
    states->store (i++, dsptab->visible);
  }
  return states;
}

// Set visibility bit for a tab
void
dbeSetTabSelectionState (int dbevindex, Vector<bool> *selected)
{
  int index;
  DispTab *dsptab;
  DbeView *dbev = getDbeView (dbevindex);
  Vector<DispTab*> *tabs = dbev->get_TabList ();
  int i = 0;
  Vec_loop (DispTab*, tabs, index, dsptab)
  {
    if (!dsptab->available)
      continue;
    dsptab->visible = selected->fetch (i++);
  }
}

// Return visibility state for all available MemObj tabs
Vector<bool> *
dbeGetMemTabSelectionState (int dbevindex)
{
  int index;
  bool dsptab;
  DbeView *dbev = getDbeView (dbevindex);
  Vector<bool> *memtabs = dbev->get_MemTabState ();

  // set the output vector
  int size = memtabs->size ();
  Vector<bool> *states = new Vector<bool>(size);

  // Get visibility bit for all available tabs
  int i = 0;
  Vec_loop (bool, memtabs, index, dsptab)
  {
    states->store (i++, dsptab);
  }
  return states;
}

// Set visibility bit for a memory tab
//
void
dbeSetMemTabSelectionState (int dbevindex, Vector<bool> *selected)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_MemTabState (selected);
}

// Return visibility state for all available index tabs
Vector<bool> *
dbeGetIndxTabSelectionState (int dbevindex)
{
  int index;
  bool dsptab;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<bool> *indxtabs = dbev->get_IndxTabState ();

  // set the output vector
  int size = indxtabs->size ();
  Vector<bool> *states = new Vector<bool>(size);

  // Get visibility bit for all available tabs
  int i = 0;
  Vec_loop (bool, indxtabs, index, dsptab)
  {
    states->store (i++, dsptab);
  }
  return states;
}

// Set visibility bit for a index tab
void
dbeSetIndxTabSelectionState (int dbevindex, Vector<bool> *selected)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_IndxTabState (selected);
}

//
// Get search path
//
Vector<char*> *
dbeGetSearchPath (int /*dbevindex*/)
{
  Vector<char*> *path = dbeSession->get_search_path ();
  int size = path->size ();
  Vector<char*> *list = new Vector<char*>(size);
  int index;
  char *name;
  Vec_loop (char*, path, index, name)
  {
    list->store (index, dbe_strdup (name));
  }
  return list;
}

//
// Set search path
//
void
dbeSetSearchPath (int /*dbevindex*/, Vector<char*> *path)
{
  dbeSession->set_search_path (path, true);
  return;
}

//
// Get pathmaps
//
Vector<void*> *
dbeGetPathmaps (int /*dbevindex*/)
{
  int index;
  pathmap_t *pthmap;
  Vector<pathmap_t*> *path = dbeSession->get_pathmaps ();
  int size = path->size ();
  Vector<void*> *data = new Vector<void*>(2);
  Vector<char*> *oldlist = new Vector<char*>(size);
  Vector<char*> *newlist = new Vector<char*>(size);

  int i = 0;
  Vec_loop (pathmap_t*, path, index, pthmap)
  {
    oldlist->store (i, dbe_strdup (pthmap->old_prefix));
    newlist->store (i, dbe_strdup (pthmap->new_prefix));
    i++;
  }
  data->store (0, oldlist);
  data->store (1, newlist);
  return data;
} // dbeGetPathmaps

char *
dbeSetPathmaps (Vector<char*> *from, Vector<char*> *to)
{
  if (from == NULL || to == NULL || from->size () != to->size ())
    return dbe_strdup ("dbeSetPathmaps: size of 'from' does not match for size of 'to'\n");
  Vector<pathmap_t*> *newPath = new Vector<pathmap_t*>(from->size ());
  for (int i = 0, sz = from->size (); i < sz; i++)
    {
      char *err = Settings::add_pathmap (newPath, from->get (i), to->get (i));
      if (err)
	{
	  newPath->destroy ();
	  delete newPath;
	  return err;
	}
    }
  dbeSession->set_pathmaps (newPath);
  return NULL;
}

//
// Add pathmap
char *
dbeAddPathmap (int /* dbevindex */, char *from, char *to)
{
  Vector<pathmap_t*> *pmp = dbeSession->get_pathmaps ();
  char *err = Settings::add_pathmap (pmp, from, to);
  return err;
}

//
// Get error/warning string of data
char *
dbeGetMsg (int dbevindex, int type)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  char *msgstr = NULL;
  if (type == ERROR_MSG)
    msgstr = dbev->get_error_msg ();
  else if (type == WARNING_MSG)
    msgstr = dbev->get_warning_msg ();
  else if (type == PSTAT_MSG)
    msgstr = dbev->get_processor_msg (PSTAT_MSG);
  else if (type == PWARN_MSG)
    msgstr = dbev->get_processor_msg (PWARN_MSG);
  return msgstr ? dbe_strdup (msgstr) : NULL;
}

// Create a DbeView, given new index, and index of view to clone
int
dbeInitView (int id, int cloneid)
{
  return dbeSession->createView (id, cloneid);
}


// Delete a DbeView
void
dbeDeleteView (int dbevindex)
{
  dbeSession->dropView (dbevindex);
  return;
} // dbeDeleteView

MetricList *
dbeGetMetricListV2 (int dbevindex, MetricType mtype,
		    Vector<int> *type, Vector<int> *subtype, Vector<bool> *sort,
		    Vector<int> *vis, Vector<char*> *cmd,
		    Vector<char*> *expr_spec, Vector<char*> *legends)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  MetricList *mlist = new MetricList (mtype);
  for (int i = 0, msize = type->size (); i < msize; i++)
    {
      BaseMetric *bm = dbev->register_metric_expr ((BaseMetric::Type) type->fetch (i),
						   cmd->fetch (i),
						   expr_spec->fetch (i));
      Metric *m = new Metric (bm, (Metric::SubType) subtype->fetch (i));
      m->set_raw_visbits (vis->fetch (i));
      if (m->legend == NULL)
	m->legend = dbe_strdup (legends->fetch (i));
      mlist->append (m);
      if (sort->fetch (i))
	{
	  mlist->set_sort_ref_index (i);
	}
    }
  return mlist;
}

static Vector<void*> *
dbeGetMetricList (MetricList *mlist)
{
  int clock_val = dbeSession->get_clock (-1);
  Vector<Metric*> *items = mlist->get_items ();
  int size = items->size ();

  Vector<int> *type = new Vector<int>(size);
  Vector<int> *subtype = new Vector<int>(size);
  Vector<int> *clock = new Vector<int>(size);
  Vector<int> *flavors = new Vector<int>(size);
  Vector<int> *vis = new Vector<int>(size);
  Vector<bool> *sorted = new Vector<bool>(size);
  Vector<int> *value_styles = new Vector<int>(size);
  Vector<char*> *aux = new Vector<char*>(size);
  Vector<char*> *name = new Vector<char*>(size);
  Vector<char*> *abbr = new Vector<char*>(size);
  Vector<char*> *comd = new Vector<char*>(size);
  Vector<char*> *unit = new Vector<char*>(size);
  Vector<char*> *user_name = new Vector<char*>(size);
  Vector<char*> *expr_spec = new Vector<char*>(size);
  Vector<char*> *legend = new Vector<char*>(size);
  Vector<int> *valtype = new Vector<int>(size);
  Vector<char*> *data_type_name = new Vector<char*>(size);
  Vector<char*> *data_type_uname = new Vector<char*>(size);
  Vector<char*> *short_desc = new Vector<char*>(size);

  int sort_index = mlist->get_sort_ref_index ();
  // Fill metric elements
  for (int i = 0; i < size; i++)
    {
      Metric *m = items->fetch (i);
      type->append (m->get_type ());
      subtype->append (m->get_subtype ());
      flavors->append (m->get_flavors ());
      abbr->append (dbe_strdup (m->get_abbr ()));
      char *s = m->get_abbr_unit ();
      if ((m->get_visbits () & VAL_RATIO) != 0)
	s = NULL;
      unit->append (dbe_strdup (s ? s : NTXT ("")));
      value_styles->append (m->get_value_styles ());
      vis->append (m->get_visbits ());
      sorted->append (i == sort_index);
      clock->append (m->get_type () == Metric::HWCNTR ? clock_val
		     : m->get_clock_unit ());
      aux->append (dbe_strdup (m->get_aux ()));
      name->append (dbe_strdup (m->get_name ()));
      comd->append (dbe_strdup (m->get_cmd ()));
      user_name->append (dbe_strdup (m->get_username ()));
      expr_spec->append (dbe_strdup (m->get_expr_spec ()));
      legend->append (dbe_strdup (m->legend));
      valtype->append (m->get_vtype2 ());

      char* _data_type_name = NULL;
      char* _data_type_uname = NULL;
      int data_type = m->get_packet_type ();
      if (data_type >= 0 && data_type < DATA_LAST)
	{
	  _data_type_name = dbe_strdup (get_prof_data_type_name (data_type));
	  _data_type_uname = dbe_strdup (get_prof_data_type_uname (data_type));
	}
      data_type_name->append (_data_type_name);
      data_type_uname->append (_data_type_uname);

      char* _short_desc = NULL;
      if (m->get_type () == Metric::HWCNTR)
	{
	  Hwcentry * hwctr = m->get_hw_ctr ();
	  if (hwctr)
	    _short_desc = dbe_strdup (hwctr->short_desc);
	}
      short_desc->append (_short_desc);
    }

  // Set Java array
  Vector<void*> *data = new Vector<void*>(16);
  data->append (type);
  data->append (subtype);
  data->append (clock);
  data->append (flavors);
  data->append (value_styles);
  data->append (user_name);
  data->append (expr_spec);
  data->append (aux);
  data->append (name);
  data->append (abbr);
  data->append (comd);
  data->append (unit);
  data->append (vis);
  data->append (sorted);
  data->append (legend);
  data->append (valtype);
  data->append (data_type_name);
  data->append (data_type_uname);
  data->append (short_desc);
  return data;
}

Vector<void*> *
dbeGetRefMetricsV2 ()
{
  MetricList *mlist = new MetricList (MET_NORMAL);
  Vector<BaseMetric*> *base_metrics = dbeSession->get_base_reg_metrics ();
  for (long i = 0, sz = base_metrics->size (); i < sz; i++)
    {
      BaseMetric *bm = base_metrics->fetch (i);
      Metric *m;
      if (bm->get_flavors () & Metric::EXCLUSIVE)
	{
	  m = new Metric (bm, Metric::EXCLUSIVE);
	  m->enable_all_visbits ();
	  mlist->append (m);
	}
      else if (bm->get_flavors () & BaseMetric::STATIC)
	{
	  m = new Metric (bm, BaseMetric::STATIC);
	  m->enable_all_visbits ();
	  mlist->append (m);
	}
    }
  Vector<void*> *data = dbeGetMetricList (mlist);
  delete mlist;
  return data;
}

Vector<void*> *
dbeGetCurMetricsV2 (int dbevindex, MetricType mtype)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  MetricList *mlist = dbev->get_metric_list (mtype);
  Vector<void*> *data = dbeGetMetricList (mlist);
  return data;
}

// YXXX we should refactor Metrics/BaseMetrics so that it no longer uses VAL_VALUE to enable time.
static int
convert_visbits_to_gui_checkbox_bits (BaseMetric *bm, const int visbits)
{
  // The purpose of this function is to handle the following case:
  //    When bm->get_value_styles() supports VAL_TIMEVAL but not VAL_VALUE
  //        Metric and BaseMetric use (visbits&VAL_VALUE) to enable time.
  //    However, the Overview expects the VAL_TIMEVAL bit to enable time.
  // Inputs: visbits as returned by BaseMetric->get_default_visbits();
  // Returns: valuebits, as used for checks in GUI checkboxes
  int valuebits = visbits;
  const int value_styles = bm->get_value_styles ();
  if ((value_styles & VAL_TIMEVAL) && // supports time
      !(value_styles & VAL_VALUE))
    { // but not value
      unsigned mask = ~(VAL_VALUE | VAL_TIMEVAL);
      valuebits = (unsigned) valuebits & mask; // clear bits
      if (visbits & VAL_VALUE)
	valuebits |= VAL_TIMEVAL; // set VAL_TIMEVAL
      if (visbits & VAL_TIMEVAL)
	valuebits |= VAL_TIMEVAL; // weird, this should never happen.
    }
  return valuebits;
}

static Vector<void*> *
dbeGetMetricTreeNode (BaseMetricTreeNode* curr, MetricList *mlist,
		      bool include_unregistered, bool has_clock_profiling_data)
{
  Vector<void*> *data = new Vector<void*>(2);

  // ----- fields
  Vector<void*> *fields = new Vector<void*>();
  Vector<char*> *name = new Vector<char*>(1);
  Vector<char*> *username = new Vector<char*>(1);
  Vector<char*> *description = new Vector<char*>(1);
  Vector<int> * flavors = new Vector<int>(1);
  Vector<int> * vtype = new Vector<int>(1);
  Vector<int> * vstyles_capable = new Vector<int>(1);

  // Specifies which default styles should be enabled when a metric is enabled.
  // Also, specifies if metric should start enabled
  Vector<int> *vstyles_e_defaults = new Vector<int>(1);
  Vector<int> *vstyles_i_defaults = new Vector<int>(1);
  Vector<bool> *registered = new Vector<bool>(1);
  Vector<bool> *aggregation = new Vector<bool>(1);
  Vector<bool> *has_value = new Vector<bool>(1);
  Vector<char*> *unit = new Vector<char*>(1);
  Vector<char*> *unit_uname = new Vector<char*>(1);

  char *_name = NULL;
  char *_username = NULL;
  char *_description = dbe_strdup (curr->get_description ());

  // BaseMetric fields
  int _flavors = 0; // SubType bitmask: (e.g. EXCLUSIVE)
  int _vtype = 0; // ValueTag: e.g. VT_INT, VT_FLOAT, ...
  int _vstyles_capable = 0; // ValueType bitmask, e.g. VAL_TIMEVAL
  int _vstyles_e_default_values = 0; // default visibility settings, exclusive/static
  int _vstyles_i_derault_values = 0; // default visibility settings, inclusive
  bool _registered = curr->is_registered ()
	  || curr->get_num_registered_descendents () > 0;
  bool _aggregation = curr->is_composite_metric ()
	  && curr->get_num_registered_descendents () > 0;
  bool _has_value = false; //not used yet; for nodes that don't have metrics
  char *_unit = NULL;
  char *_unit_uname = NULL;

  BaseMetric *bm = curr->get_BaseMetric ();
  if (bm)
    {
      _name = dbe_strdup (bm->get_cmd ());
      _username = dbe_strdup (bm->get_username ());
      if (!include_unregistered && !curr->is_registered ())
	abort ();
      _flavors = bm->get_flavors ();
      _vtype = bm->get_vtype ();
      _vstyles_capable = bm->get_value_styles ();
      int e_visbits = bm->get_default_visbits (BaseMetric::EXCLUSIVE);
      int i_visbits = bm->get_default_visbits (BaseMetric::INCLUSIVE);
      _vstyles_e_default_values = convert_visbits_to_gui_checkbox_bits (bm, e_visbits);
      _vstyles_i_derault_values = convert_visbits_to_gui_checkbox_bits (bm, i_visbits);
      // not all metrics shown in er_print cmd line should be selected in the GUI at startup:
      if (has_clock_profiling_data && bm->get_hw_ctr ())
	{
	  bool hide = true; // by default, hide HWCs
	  if (dbe_strcmp (bm->get_hw_ctr ()->name, NTXT ("c_stalls")) == 0 ||
	      dbe_strcmp (bm->get_hw_ctr ()->name, NTXT ("K_c_stalls")) == 0)
	    {
	      bool is_time = (bm->get_value_styles () & VAL_TIMEVAL) != 0;
	      if (is_time)
		// By default, show time variant of c_stalls
		hide = false;
	    }
	  if (hide)
	    {
	      _vstyles_e_default_values |= VAL_HIDE_ALL;
	      _vstyles_i_derault_values |= VAL_HIDE_ALL;
	    }
	}
    }
  else
    {
      // not a base metric
      _name = dbe_strdup (curr->get_name ());
      _username = dbe_strdup (curr->get_user_name ());
     if (curr->get_unit ())
	{ // represents a value
	  _has_value = true;
	  _unit = dbe_strdup (curr->get_unit ());
	  _unit_uname = dbe_strdup (curr->get_unit_uname ());
	}
    }
  name->append (_name); // unique id string (dmetrics cmd)
  username->append (_username); // user-visible name
  description->append (_description);
  flavors->append (_flavors); // SubType bitmask: (e.g. EXCLUSIVE)
  vtype->append (_vtype); // ValueTag: e.g. VT_INT, VT_FLOAT, ...
  vstyles_capable->append (_vstyles_capable); // ValueType bitmask, e.g. VAL_TIMEVAL
  vstyles_e_defaults->append (_vstyles_e_default_values);
  vstyles_i_defaults->append (_vstyles_i_derault_values);
  registered->append (_registered); // is a "live" metric
  aggregation->append (_aggregation); // value derived from children nodes
  has_value->append (_has_value); // value generated from other source
  unit->append (_unit); // See BaseMetric.h, e.g. UNIT_SECONDS
  unit_uname->append (_unit_uname); //See BaseMetric.h,

  fields->append (name);
  fields->append (username);
  fields->append (description);
  fields->append (flavors);
  fields->append (vtype);
  fields->append (vstyles_capable);
  fields->append (vstyles_e_defaults);
  fields->append (vstyles_i_defaults);
  fields->append (registered);
  fields->append (aggregation);
  fields->append (has_value);
  fields->append (unit);
  fields->append (unit_uname);
  data->append (fields);

  // ----- children
  Vector<BaseMetricTreeNode*> *children = curr->get_children ();
  int num_children = children->size ();
  Vector<void*> *children_list = new Vector<void*>(num_children);
  BaseMetricTreeNode *child_node;
  int index;

  Vec_loop (BaseMetricTreeNode*, children, index, child_node)
  {
    if (include_unregistered /* fetch everything */
	|| child_node->is_registered ()
	|| child_node->get_num_registered_descendents () > 0)
      {
	//Special case for metrics that aren't registered
	// but have registered children
	// Linux example: Total Time is unregistered, CPU Time is registered
	if (!include_unregistered && /* not fetching everything */
	    !child_node->is_registered () &&
	    (child_node->get_BaseMetric () != NULL ||
	     child_node->is_composite_metric ()))
	  {
	    Vector<BaseMetricTreeNode*> *registered_descendents =
		    new Vector<BaseMetricTreeNode*>();
	    child_node->get_nearest_registered_descendents (registered_descendents);
	    int idx2;
	    BaseMetricTreeNode*desc_node;
	    Vec_loop (BaseMetricTreeNode*, registered_descendents, idx2, desc_node)
	    {
	      Vector<void*> *desc_data;
	      desc_data = dbeGetMetricTreeNode (desc_node, mlist,
				include_unregistered, has_clock_profiling_data);
	      children_list->append (desc_data);
	    }
	    delete registered_descendents;
	    continue;
	  }
	Vector<void*> *child_data;
	child_data = dbeGetMetricTreeNode (child_node, mlist,
				include_unregistered, has_clock_profiling_data);
	children_list->append (child_data);
      }
  }
  data->append (children_list);
  return data;
}

Vector<void*> *
dbeGetRefMetricTree (int dbevindex, bool include_unregistered)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  MetricList *mlist = dbev->get_metric_list (MET_NORMAL);
  bool has_clock_profiling_data = false;
  for (long i = 0, sz = mlist->get_items ()->size (); i < sz; i++)
    {
      Metric *m = mlist->get_items ()->fetch (i);
      if (m->get_packet_type () == DATA_CLOCK)
	{
	  has_clock_profiling_data = true;
	  break;
	}
    }
  BaseMetricTreeNode *curr = dbeSession->get_reg_metrics_tree ();
  return dbeGetMetricTreeNode (curr, mlist, include_unregistered, has_clock_profiling_data);
}

static Vector<void*> *
dbeGetTableDataV2Data (DbeView *dbev, Hist_data *data);

static Vector<void*> *dbeGetTableDataOneColumn (Hist_data *data, int met_ind);
static Vector<void*> *
dbeGetTableDataOneColumn (DbeView *dbev, Vector<Hist_data::HistItem*> *data,
			  ValueTag vtype, int metricColumnNumber);

static hrtime_t
dbeCalcGroupDuration (int grInd)
{
  int thisGroupSize = 1;
  hrtime_t max_time = 0;
  Experiment *exp;
  if (dbeSession->expGroups->size () > 0)
    {
      ExpGroup *grp = dbeSession->expGroups->fetch (grInd);
      thisGroupSize = grp->exps->size ();
      for (int ii = 0; ii < thisGroupSize; ii++)
	{
	  exp = grp->exps->fetch (ii);
	  Vector<DataDescriptor*> *ddscr = exp->getDataDescriptors ();
	  delete ddscr;// getDataDescriptors() forces reading of experiment data
	  if (exp != NULL)
	    {
	      hrtime_t tot_time = exp->getLastEvent () - exp->getStartTime ()
		      + exp->getRelativeStartTime ();
	      if (max_time < tot_time)
		max_time = tot_time;
	    }
	}
    }
  else
    {
      exp = dbeSession->get_exp (0);
      if (exp != NULL)
	max_time = exp->getLastEvent () - exp->getStartTime ();
    }
  return max_time; //nanoseconds
}

static hrtime_t
dbeCalcGroupGCDuration (int grInd)
{
  int thisGroupSize = 1;
  hrtime_t tot_time = 0;
  Experiment *exp;
  if (dbeSession->expGroups->size () > 0)
    {
      ExpGroup *grp = dbeSession->expGroups->fetch (grInd);
      thisGroupSize = grp->exps->size ();
      for (int ii = 0; ii < thisGroupSize; ii++)
	{
	  exp = grp->exps->fetch (ii);
	  Vector<DataDescriptor*> *ddscr = exp->getDataDescriptors ();
	  delete ddscr; // getDataDescriptors() forces reading of experiment data
	  if (exp != NULL)
	    tot_time += exp->getGCDuration ();
	}
    }
  else
    {
      exp = dbeSession->get_exp (0);
      if (exp != NULL)
	tot_time = exp->getGCDuration ();
    }
  return tot_time; //nanoseconds
}

Vector<void*> *
dbeGetRefMetricTreeValues (int dbevindex, Vector<char *> *metric_cmds,
			   Vector<char *> *non_metric_cmds)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  // valueTable will have N "columns" of values, where N is the number of
  //   requested metrics and non-metrics.
  // Each column will be a vector with M "rows", where M is the number of
  //   compare groups.
  // highlightTable mirrors the structure of valueTable.  Each cell indicates
  //   if the corresponding valueTable cell is "hot" (interesting)
  int numMetrics = metric_cmds->size ();
  int numNonMetrics = non_metric_cmds->size ();
  int totalColumns = numMetrics + numNonMetrics; // Columns
  Vector<void*> *valueTable = new Vector<void*>(totalColumns);
  Vector<void*> *highlightTable = new Vector<void*>(totalColumns);

  // the return value consists of the two tables discussed above.
  Vector<void*> *rc = new Vector<void*>(2);
  rc->append (valueTable);
  rc->append (highlightTable);
  if (dbeSession->nexps () == 0)
    { // no experiments are loaded
      for (int jj = 0; jj < totalColumns; jj++)
	{
	  Vector<void *> *columnData = new Vector<void *>();
	  valueTable->append (columnData);
	  highlightTable->append (columnData);
	}
      return rc;
    }

  int ngroups = dbeSession->expGroups->size (); // Rows (one per compare group)
  if (ngroups == 0 || !dbev->comparingExperiments ())
    ngroups = 1;

  Vector<double> *groupTotalTime = new Vector<double>(ngroups);
  Vector<double> *groupCpuTime = new Vector<double>(ngroups);
  // initialize highlight table
  for (int ii = 0; ii < totalColumns; ii++)
    { // metrics
      Vector<bool> *columnData = new Vector<bool>(ngroups);
      highlightTable->append (columnData);
      for (int grInd = 0; grInd < ngroups; grInd++)
	columnData->store (grInd, false); // non-highlight
    }

  if (numMetrics > 0)
    {
      MetricList *bmlist;
      // set bmlist to list of requested base metrics
      BaseMetricTreeNode *root = dbeSession->get_reg_metrics_tree ();
      int index;
      char *mcmd;
      Vector<BaseMetric*> *base_metrics = new Vector<BaseMetric*>();
      Vec_loop (char *, metric_cmds, index, mcmd)
      {
	BaseMetricTreeNode *bmt_node = root->find (mcmd);
	if (!bmt_node)
	  abort (); //YXXX weird
	BaseMetric * baseNetric = bmt_node->get_BaseMetric ();
	if (!baseNetric)
	  abort ();
	base_metrics->append (baseNetric);
      }

      // MET_INDX will create MetricList of Exclusive metrics
      bmlist = new MetricList (base_metrics, MET_SRCDIS);

      // Use the Function List to fetch <Total> values
      // A temporary table, v_totals, stores <total> by group
      Vector<Hist_data::HistItem *> *v_totals = new Vector<Hist_data::HistItem *>(ngroups);
      for (int grInd = 0; grInd < ngroups; grInd++)
	{
	  MetricList *mlist;
	  if (ngroups > 1)
	    mlist = dbev->get_compare_mlist (bmlist, grInd);
	  else
	    mlist = bmlist;
	  if (mlist->size () != numMetrics)
	    abort ();

	  Hist_data *data;
	  data = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
				      Hist_data::ALL);
	  Hist_data::HistItem * totals = data->get_totals ();
	  v_totals->append (totals);
	}

      // store the Hist_data totals in valueTable
      {
	Metric *mitem;
	int index;
	Vec_loop (Metric*, bmlist->get_items (), index, mitem)
	{
	  Vector<void*> * columnData = dbeGetTableDataOneColumn (dbev,
					  v_totals, mitem->get_vtype (), index);
	  valueTable->append (columnData);
	}
      }

      // 7207285: hack for hwc profiling cycles conversion:
      {
	Metric *mitem;
	int index;
	Vec_loop (Metric*, bmlist->get_items (), index, mitem)
	{
	  if (mitem->is_time_val ()
	      && mitem->get_vtype () == VT_ULLONG)
	    {
	      Vector<long long> *cycleValues = (Vector<long long> *)valueTable->fetch (index);
	      Vector<double> *timeValues = new Vector<double>(ngroups);
	      assert (cycleValues->size () == ngroups);
	      for (int grInd = 0; grInd < ngroups; grInd++)
		{
		  long long cycles = cycleValues->fetch (grInd);
		  int expId;
		  if (dbeSession->expGroups->size () > 0)
		    {
		      ExpGroup *gr = dbeSession->expGroups->fetch (grInd);
		      Experiment *exp = gr->exps->fetch (0);
		      expId = exp->getExpIdx ();
		    }
		  else
		    expId = -1;
		  int clock = dbeSession->get_clock (expId);
		  double time;
		  if (clock)
		    time = cycles / (1.e+6 * clock);
		  else
		    time = cycles; //weird
		  timeValues->store (grInd, time);
		}
	      delete cycleValues;
	      valueTable->store (index, timeValues);
	    }
	}
      }

      // Scan metrics for best measure of CPU time
      int bestCpuTimeIndx = -1;
      {
	Metric *mitem;
	int index;
	Vec_loop (Metric*, bmlist->get_items (), index, mitem)
	{
	  BaseMetric::Type type = mitem->get_type ();
	  if (type == BaseMetric::CP_KERNEL_CPU)
	    {
	      bestCpuTimeIndx = index;
	      break; // CP_KERNEL_CPU trumps other measures
	    }
	  if (type == BaseMetric::CP_TOTAL_CPU)
	    {
	      // clock profiling CPU time
	      bestCpuTimeIndx = index;
	      // keep looking in case CP_KERNEL_CPU also exists
	      continue;
	    }

	  bool isTime = ((mitem->get_value_styles () & VAL_TIMEVAL) != 0);
	  bool isHwcCycles = (type == BaseMetric::HWCNTR
			      && (dbe_strcmp (mitem->get_aux (), "cycles") == 0)
			      && isTime);
	  if (isHwcCycles)
	    if (bestCpuTimeIndx < 0)
	      bestCpuTimeIndx = index;
	}
	if (bestCpuTimeIndx >= 0)
	  {
	    Vector<double> *timeValues = (Vector<double> *)valueTable->fetch (bestCpuTimeIndx);
	    if (timeValues->type () == VEC_DOUBLE)
	      for (int grInd = 0; grInd < ngroups; grInd++)
		{
		  double time = timeValues->fetch (grInd);
		  groupCpuTime->append (time);
		}
	  }
      }

      // Scan metrics for Total Thread time
      {
	Metric *mitem;
	int index;
	Vec_loop (Metric*, bmlist->get_items (), index, mitem)
	{
	  BaseMetric::Type type = mitem->get_type ();
	  if (type == BaseMetric::CP_TOTAL)
	    {
	      Vector<double> *timeValues = (Vector<double> *)valueTable->fetch (index);
	      if (timeValues->type () != VEC_DOUBLE)
		continue; // weird
	      for (int grInd = 0; grInd < ngroups; grInd++)
		{
		  double time = timeValues->fetch (grInd);
		  groupTotalTime->append (time);
		}
	      break;
	    }
	}
      }

      // highlight metrics based on cpu time
#define CPUSEC_PERCENT_THRESHOLD            10.0
#define HWC_OVERFLOWS_PER_CPUSEC_THRESHOLD  15
      {
	Metric *mitem;
	int index;
	Vec_loop (Metric*, bmlist->get_items (), index, mitem)
	{
	  BaseMetric::Type type = mitem->get_type ();
	  Vector<bool> * columnHilites = (Vector<bool> *)highlightTable->fetch (index);

	  // always highlight the following
	  if (index == bestCpuTimeIndx)
	    {
	      for (int grInd = 0; grInd < ngroups; grInd++)
		columnHilites->store (grInd, true);
	      continue;
	    }

	  // skip certain types
	  bool typeIsCycles = (type == BaseMetric::HWCNTR
		       && dbe_strcmp (mitem->get_aux (), NTXT ("cycles")) == 0);
	  bool typeIsInsts = (type == BaseMetric::HWCNTR
			&& dbe_strcmp (mitem->get_aux (), NTXT ("insts")) == 0);
	  if (type == BaseMetric::CP_TOTAL
	      || type == BaseMetric::CP_TOTAL_CPU
	      || type == BaseMetric::CP_LMS_USER
	      || type == BaseMetric::CP_LMS_SYSTEM
	      || type == BaseMetric::CP_LMS_TRAP
	      || type == BaseMetric::CP_LMS_USER_LOCK
	      || type == BaseMetric::CP_LMS_SLEEP
	      || type == BaseMetric::CP_KERNEL_CPU
	      || type == BaseMetric::OMP_WORK
	      || typeIsCycles
	      || typeIsInsts
	      // || type == BaseMetric::CP_TOTAL_WAIT
	      )
	    continue; // types we never highlight

	  // for time values, compare against CPUSEC_PERCENT_THRESHOLD
	  bool isTime = ((mitem->get_value_styles () & VAL_TIMEVAL) != 0);
	  if (isTime)
	    {
	      if (groupCpuTime->size () == 0)
		continue; // no time to use as reference
	      Vector<double> *timeValues = (Vector<double> *)valueTable->fetch (index);
	      if (timeValues->type () != VEC_DOUBLE)
		continue; // weird
	      for (int grInd = 0; grInd < ngroups; grInd++)
		{
		  double thistime = timeValues->fetch (grInd);
		  double usertime = groupCpuTime->fetch (grInd);
		  if (thistime / (CPUSEC_PERCENT_THRESHOLD / 100) > usertime)
		    columnHilites->store (grInd, true);
		}
	      continue;
	    }

	  // for HWC event counts, look at rate of events
	  if (type == BaseMetric::HWCNTR)
	    {
	      Hwcentry *hwctr = mitem->get_hw_ctr ();
	      if (!hwctr)
		continue; // weird
	      if (!hwctr->metric)
		continue; // raw counter
	      if (groupCpuTime->size () == 0)
		continue; // no time to use as reference
	      if (mitem->get_base_metric ()->get_dependent_bm ())
		continue; // has a derived time metric, only flag time version
	      Vector<long long> *llValues = (Vector<long long> *)valueTable->fetch (index);
	      if (llValues->type () != VEC_LLONG)
		continue; // weird
	      int overflowVal = hwctr->val; //overflow count
	      if (!overflowVal)
		continue; // weird
	      if (overflowVal > (4000000))
		// cut off events that are very frequent like loads/stores
		// 4Ghz * (0.01 seconds/event) / (4000000 events/overflow) = 10 cycles
		continue;
	      // for HWCs we could base it on the overflow rate
	      for (int grInd = 0; grInd < ngroups; grInd++)
		{
		  double thisVal = llValues->fetch (grInd);
		  thisVal /= overflowVal;
		  double usertime = groupCpuTime->fetch (grInd);
		  if (thisVal > usertime * HWC_OVERFLOWS_PER_CPUSEC_THRESHOLD)
		    columnHilites->store (grInd, true);
		}
	      continue;
	    }

	  // check for non-zero counts of the following
	  if (type == BaseMetric::DEADLOCKS ||
	      type == BaseMetric::RACCESS ||
	      type == BaseMetric::HEAP_ALLOC_BYTES ||
	      type == BaseMetric::HEAP_LEAK_BYTES)
	    {
	      Vector<long long> *llValues = (Vector<long long> *)valueTable->fetch (index);
	      if (llValues->type () != VEC_LLONG)
		continue; // weird
	      for (int grInd = 0; grInd < ngroups; grInd++)
		{
		  long long thisVal = llValues->fetch (grInd);
		  if (thisVal)
		    columnHilites->store (grInd, true);
		}
	      continue;
	    }
	  // continue adding cases as needed
	}
      }
    }

  if (numNonMetrics > 0)
    {
      int index;
      char *mcmd;
      Vec_loop (char *, non_metric_cmds, index, mcmd)
      {
	if (dbe_strcmp (mcmd, NTXT ("YXXX_TOTAL_TIME_PLUS_THREADS")) == 0
	    && groupCpuTime->size () == ngroups)
	  {
	    Vector<char *> *columnData = new Vector<char *>(ngroups);
	    for (int grInd = 0; grInd < ngroups; grInd++)
	      {
		double totaltime = groupTotalTime->fetch (grInd);
		columnData->append (dbe_sprintf (NTXT ("%0.3f %s"), totaltime, GTXT ("Seconds")));
	      }
	    valueTable->append (columnData);
	  }
	else if (dbe_strcmp (mcmd, L1_DURATION) == 0)
	  {
	    Vector<double> *columnData = new Vector<double>(ngroups);
	    for (int grInd = 0; grInd < ngroups; grInd++)
	      {
		hrtime_t duration = dbeCalcGroupDuration (grInd);
		double seconds = duration * 1.e-9;
		columnData->append (seconds);
	      }
	    valueTable->append (columnData);
	  }
	else if (dbe_strcmp (mcmd, L1_GCDURATION) == 0)
	  {
	    Vector<double> *columnData = new Vector<double>(ngroups);
	    for (int grInd = 0; grInd < ngroups; grInd++)
	      {
		hrtime_t duration = dbeCalcGroupGCDuration (grInd);
		double seconds = duration * 1.e-9;
		columnData->append (seconds);
	      }
	    valueTable->append (columnData);
	  }
	else
	  {
	    Vector<char *> *columnData = new Vector<char *>(ngroups);
	    char * valueString = NTXT ("<unknown>");
	    for (int grInd = 0; grInd < ngroups; grInd++)
	      columnData->append (dbe_strdup (valueString));
	    valueTable->append (columnData);
	  }
      }
    }
  return rc;
}

Vector<char*> *
dbeGetOverviewText (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  Vector<char*> *info = new Vector<char*>;
  char *field;
  int ngroups = dbeSession->expGroups->size (); // Rows (one per compare group)
  if (ngroups == 0 || !dbev->comparingExperiments ())
    ngroups = 1;
  for (int grInd = 0; grInd < ngroups; grInd++)
    {
      int thisGroupSize = 1;
      Experiment *exp;
      if (dbeSession->expGroups->size () > 0)
	{
	  ExpGroup *gr = dbeSession->expGroups->fetch (grInd);
	  exp = gr->exps->fetch (0);
	  thisGroupSize = gr->exps->size ();
	}
      else
	{
	  if (dbeSession->nexps () == 0)
	    return info;
	  exp = dbeSession->get_exp (0);
	}
      char * expHeader;
      if (ngroups == 1)
	expHeader = dbe_strdup (GTXT ("Experiment      :"));
      else if (grInd == 0)
	expHeader = dbe_strdup (GTXT ("Base Group      : "));
      else if (ngroups == 2)
	expHeader = dbe_strdup (GTXT ("Compare Group   : "));
      else
	expHeader = dbe_sprintf (GTXT ("Compare Group %d : "), grInd);
      if (thisGroupSize == 1)
	info->append (dbe_sprintf ("%s%s", expHeader, exp->get_expt_name ()));
      else
	info->append (dbe_sprintf ("%s%s (plus %d more)",
			  expHeader, exp->get_expt_name (), thisGroupSize - 1));
      free (expHeader);
      field = exp->uarglist;
      if (field && field[0])
	info->append (dbe_sprintf (GTXT ("  Target        : '%s'"), field));
      field = exp->hostname;
      if (field && field[0])
	info->append (dbe_sprintf (GTXT ("  Host          : %s (%s, %s)"),
				   field,
				   exp->architecture ? exp->architecture
				   : GTXT ("<CPU architecture not recorded>"),
				   exp->os_version ? exp->os_version
				   : GTXT ("<OS version not recorded>")));
      time_t start_sec = (time_t) exp->start_sec;
      char *p = ctime (&start_sec);
      hrtime_t tot_time = dbeCalcGroupDuration (grInd);
      double seconds = tot_time * 1.e-9;
      info->append (dbe_sprintf (
		GTXT ("  Start Time    : %s  Duration      : %0.3f Seconds"),
		p, seconds));
      // Number of descendants/processes would be nice
      info->append (dbe_strdup (NTXT ("")));
    }
  return info;
}

//--------------------------------------------------------------------------
// Set Sort by index
//
void
dbeSetSort (int dbevindex, int sort_index, MetricType mtype, bool reverse)
{
  DbeView *dbev;

  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->setSort (sort_index, mtype, reverse);
  return;
}

//
// Get annotation setting
//
Vector<int> *
dbeGetAnoValue (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<int> *set = new Vector<int>(9);
  set->store (0, dbev->get_src_compcom ());
  set->store (1, dbev->get_dis_compcom ());
  set->store (2, dbev->get_thresh_src ());
  set->store (3, dbev->get_thresh_src ());
  set->store (4, dbev->get_src_visible ());
  set->store (5, (int) dbev->get_srcmetric_visible ());
  set->store (6, (int) dbev->get_hex_visible ());
  set->store (7, (int) dbev->get_cmpline_visible ());
  set->store (8, (int) dbev->get_func_scope ());
  return set;
}

//
// Set annotation setting
//
void
dbeSetAnoValue (int dbevindex, Vector<int> *set)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (set->size () != 10)
    return;
  dbev->set_src_compcom (set->fetch (0));
  dbev->set_dis_compcom (set->fetch (1));
  dbev->set_thresh_src (set->fetch (2));
  dbev->set_thresh_dis (set->fetch (3));
  dbev->set_src_visible (set->fetch (4));
  dbev->set_srcmetric_visible ((bool)set->fetch (5));
  dbev->set_hex_visible ((bool)set->fetch (6));
  dbev->set_cmpline_visible ((bool)set->fetch (7));
  dbev->set_func_scope (set->fetch (8));
  dbev->set_funcline_visible ((bool)set->fetch (9));
  return;
}

//
// Get name formats
//
int
dbeGetNameFormat (int dbevindex)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable::NameFormat fmt = dbev->get_name_format ();
  return Histable::fname_fmt (fmt);
}

bool
dbeGetSoName (int dbevindex)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable::NameFormat fmt = dbev->get_name_format ();
  return Histable::soname_fmt (fmt);
}

//
// Set name formats
//
void
dbeSetNameFormat (int dbevindex, int nformat, bool soname)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_name_format (nformat, soname);
}

//
// Get View mode
//
int
dbeGetViewMode (int dbevindex)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  return (int) dbev->get_view_mode ();
}

// Set View mode
void
dbeSetViewMode (int dbevindex, int nmode)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_view_mode ((VMode) nmode);
  return;
}

// Get timeline setting
//
Vector<void*> *
dbeGetTLValue (int dbevindex)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<char *> *strings = new Vector<char *>();
  char *tldata_cmd = dbev->get_tldata ();
  strings->store (0, tldata_cmd);

  Vector<int> *ints = new Vector<int>(3);
  int val;
  val = dbev->get_tlmode ();
  ints->store (0, val);
  val = dbev->get_stack_align ();
  ints->store (1, val);
  val = dbev->get_stack_depth ();
  ints->store (2, val);

  Vector<void*> *objs = new Vector<void*>(2);
  objs->store (0, strings);
  objs->store (1, ints);
  return objs;
}

//
// Set timeline setting
//
void
dbeSetTLValue (int dbevindex, const char *tldata_cmd,
	       int entitiy_prop_id, int stackalign, int stackdepth)
{
  DbeView *dbev;
  dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_tldata (tldata_cmd);
  dbev->set_tlmode (entitiy_prop_id);
  dbev->set_stack_align (stackalign);
  dbev->set_stack_depth (stackdepth);
  return;
}

//
// Get founder experiments and their descendants
//
Vector<void*> *
dbeGetExpFounderDescendants ()
{
  int size = dbeSession->nexps ();
  if (size == 0)
    return NULL;
  Vector<void*> *table = new Vector<void*>(2);
  Vector<int> *founderExpIds = new Vector<int>();
  Vector<Vector<int> *> *subExpIds = new Vector<Vector<int>*>();
  for (int index = 0; index < size; index++)
    {
      Experiment *exp = dbeSession->get_exp (index);
      if (exp->founder_exp == NULL)
	{
	  founderExpIds->append (exp->getExpIdx ());
	  Vector<int> *subExps = new Vector<int>();
	  for (int i = 0; i < exp->children_exps->size (); i++)
	    {
	      Experiment * subExp = exp->children_exps->fetch (i);
	      subExps->append (subExp->getExpIdx ());
	    }
	  subExpIds->append (subExps);
	}
    }
  table->store (0, founderExpIds);
  table->store (1, subExpIds);
  return table;
}

//
// Get experiment selection
//
Vector<void*> *
dbeGetExpSelection (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  int size = dbeSession->nexps ();
  if (size == 0)
    return NULL;
  Vector<void*> *table = new Vector<void*>(3);
  Vector<char*> *names = new Vector<char*>(size);
  Vector<bool> *enable = new Vector<bool>(size);
  Vector<int> *userExpIds = new Vector<int>(size);

  // Get experiment names
  for (int index = 0; index < size; index++)
    {
      Experiment *exp = dbeSession->get_exp (index);
      char *buf = dbeGetName (dbevindex, index);
      names->store (index, buf);
      bool val;
      val = dbev->get_exp_enable (index);
      enable->store (index, val);
      userExpIds->store (index, exp->getUserExpId ());
    }
  table->store (0, names);
  table->store (1, enable);
  table->store (2, userExpIds);
  return table;
}

int
dbeValidateFilterExpression (char *str_expr)
{
  if (str_expr == NULL)
    return 0;
  Expression *expr = dbeSession->ql_parse (str_expr);
  if (expr == NULL)
    return 0;
  delete expr;
  return 1;
}

Vector<void*> *
dbeGetFilterKeywords (int /* dbevindex */)
{
  Vector <char*> *kwCategory = new Vector<char *>();
  Vector <char*> *kwCategoryI18N = new Vector<char *>();
  Vector <char*> *kwDataType = new Vector<char *>();
  Vector <char*> *kwKeyword = new Vector<char *>();
  Vector <char*> *kwFormula = new Vector<char *>();
  Vector <char*> *kwDescription = new Vector<char *>();
  Vector <void*> *kwEnumDescs = new Vector<void *>();

  Vector<void*> *res = new Vector<void*>(7);
  res->append (kwCategory);
  res->append (kwCategoryI18N);
  res->append (kwDataType);
  res->append (kwKeyword);
  res->append (kwFormula);
  res->append (kwDescription);
  res->append (kwEnumDescs);

  char *vtypeNames[] = VTYPE_TYPE_NAMES;
  // section header for global definitions
  kwCategory->append (dbe_strdup (NTXT ("FK_SECTION")));
  kwCategoryI18N->append (dbe_strdup (GTXT ("Global Definitions")));
  kwDataType->append (NULL);
  kwKeyword->append (NULL);
  kwFormula->append (NULL);
  kwDescription->append (NULL);
  kwEnumDescs->append (NULL);
  dbeSession->get_filter_keywords (res);
  MemorySpace::get_filter_keywords (res);

  // loop thru all founder experiments
  int nexp = dbeSession->nexps ();
  for (int ii = 0; ii < nexp; ++ii)
    {
      Experiment* fexp = dbeSession->get_exp (ii);
      if (fexp->founder_exp != NULL)
	continue; // is a child; should be covered when we get to founder

      // section header for each founder
      // section header for founder experiment
      kwCategory->append (dbe_strdup (NTXT ("FK_SECTION")));
      kwCategoryI18N->append (dbe_sprintf (NTXT ("%s [EXPGRID==%d]"),
					   fexp->get_expt_name (),
					   fexp->groupId));
      kwDataType->append (NULL);
      kwKeyword->append (NULL);
      kwFormula->append (NULL);
      kwDescription->append (NULL);
      kwEnumDescs->append (NULL);

      int nchildren = fexp->children_exps->size ();
      Experiment *exp;
      // category header: Experiments
      {
	char *propUName = dbeSession->getPropUName (PROP_EXPID);

	// store list of subexperiments in kwEnumDescs
	Vector <char*> *enumDescs = new Vector<char *>();
	int jj = 0;
	exp = fexp;
	while (1)
	  {
	    char * expBasename = get_basename (exp->get_expt_name ());
	    char * targetName = exp->utargname ? exp->utargname
		    : (char *) GTXT ("(unknown)");
	    enumDescs->append (dbe_sprintf (NTXT ("(%d) -> %s [%s, PID %d]"),
					    exp->getUserExpId (), expBasename,
					    targetName, exp->getPID ()));
	    if (jj >= nchildren)
	      break;
	    exp = fexp->children_exps->fetch (jj);
	    jj++;
	  }
	kwCategory->append (dbe_strdup (NTXT ("FK_EXPLIST")));
	kwCategoryI18N->append (dbe_strdup (GTXT ("Experiments")));
	kwDataType->append (dbe_strdup (vtypeNames[TYPE_INT32]));
	kwKeyword->append (dbe_strdup (NTXT ("EXPID")));
	kwFormula->append (NULL);
	kwDescription->append (propUName);
	kwEnumDescs->append (enumDescs);
      }

      // select representative experiment
      if (nchildren == 0)
	exp = fexp; // founder
      else
	exp = fexp->children_exps->fetch (0); // first child
      int expIdx = exp->getExpIdx ();
      Vector<void*> *data = dbeGetDataDescriptorsV2 (expIdx);
      if (data == NULL)
	continue;
      Vector<int> *dataId = (Vector<int>*)data->fetch (0);
      Vector<char*> *dataName = (Vector<char*>*)data->fetch (1);
      Vector<char*> *dataUName = (Vector<char*>*)data->fetch (2);
      if (dataId == NULL || dataName == NULL)
	{
	  destroy (data);
	  continue;
	}
      // loop thru data descriptors
      int ndata = dataId->size ();
      for (int j = 0; j < ndata; ++j)
	{
	  // category: data name (e.g. Clock Profiling)
	  char * catName = dataName->fetch (j);
	  char * dUname = dataUName ? dataUName->fetch (j) : catName;
	  char * catUname = dUname ? dUname : catName;

	  Vector<void*> *props = dbeGetDataPropertiesV2 (expIdx, dataId->fetch (j));
	  if (props == NULL)
	    continue;
	  Vector<char*> *propUName = (Vector<char*>*)props->fetch (1);
	  Vector<int> *propTypeId = (Vector<int> *)props->fetch (2);
	  Vector<char*> *propType = (Vector<char*>*)props->fetch (3);
	  Vector<char*> *propName = (Vector<char*>*)props->fetch (5);
	  Vector<Vector<char*>*> *propStateNames =
		  (Vector<Vector<char*>*> *)props->fetch (6);
	  Vector<Vector<char*>*> *propStateUNames =
		  (Vector<Vector<char*>*> *)props->fetch (7);
	  if (propName == NULL || propUName == NULL || propType == NULL
	      || propName->size () <= 0)
	    {
	      destroy (props);
	      continue;
	    }
	  int nprop = propName->size ();
	  for (int k = 0; k < nprop; ++k)
	    {
	      if (propTypeId->fetch (k) == TYPE_OBJ)
		continue;
	      if (dbe_strcmp (propName->fetch (k), NTXT ("FRINFO")) == 0)
		continue;

	      // store list of states in kwEnumDescs
	      Vector<char*> *enumDescs = new Vector<char *>();
	      Vector<char*>* stateNames = propStateNames->fetch (k);
	      Vector<char*>* stateUNames = propStateUNames->fetch (k);
	      int nStates = stateNames ? stateNames->size () : 0;
	      for (int kk = 0; kk < nStates; ++kk)
		{
		  const char *stateName = stateNames->fetch (kk);
		  if (stateName == NULL || strlen (stateName) == 0)
		    continue;
		  const char *stateUName = stateUNames->fetch (kk);
		  if (stateUName == NULL || strlen (stateUName) == 0)
		    stateUName = stateName;
		  enumDescs->append (dbe_sprintf (NTXT ("(%d) -> %s"), kk, stateUName));
		}
	      kwCategory->append (dbe_strdup (catName));
	      kwCategoryI18N->append (dbe_strdup (catUname));
	      kwDataType->append (dbe_strdup (propType->fetch (k)));
	      kwKeyword->append (dbe_strdup (propName->fetch (k)));
	      kwFormula->append (NULL);
	      kwDescription->append (dbe_strdup (propUName->fetch (k)));
	      kwEnumDescs->append (enumDescs);
	    }
	  destroy (props);
	}
      destroy (data);
    }
  return (res);
}

// GetFilters -- returns the list of filters for the indexed experiment
//	returns false if there's a problem; true otherwise
//
Vector<void*> *
dbeGetFilters (int dbevindex, int nexp)
{
  FilterNumeric *filt;
  int index;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<FilterNumeric *>*filters = dbev->get_all_filters (nexp);
  if (filters == NULL)
    return NULL;

  // return an array of filter data for that experiment
  Vector <int> *findex = new Vector<int>(); // index of the filters
  Vector <char*> *shortname = new Vector<char *>();
  // short name of filter
  Vector <char*> *i18n_name = new Vector<char *>();
  // External I18N'd name of filter
  Vector <char*> *pattern = new Vector<char *>();
  // current setting string
  Vector <char*> *status = new Vector<char *>();
  // current status of filter (%, range, etc.)

  Vec_loop (FilterNumeric *, filters, index, filt)
  {
    findex->append (index);
    shortname->append (dbe_strdup (filt->get_cmd ()));
    i18n_name->append (dbe_strdup (filt->get_name ()));
    pattern->append (dbe_strdup (filt->get_pattern ()));
    status->append (dbe_strdup (filt->get_status ()));
  }
  Vector<void*> *res = new Vector<void*>(5);
  res->store (0, findex);
  res->store (1, shortname);
  res->store (2, i18n_name);
  res->store (3, pattern);
  res->store (4, status);
  return (res);
}

// Set a filter string for a view
//	Returns NULL if OK, error message if not

char *
dbeSetFilterStr (int dbevindex, char *filter_str)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->clear_error_msg ();
  dbev->clear_warning_msg ();
  char *ret = dbev->set_filter (filter_str);
  return ret;
}

// Get the current filter setting for the view
char *
dbeGetFilterStr (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  char *ret = dbev->get_filter ();
  return ret;
}

// Update a filters for a single experiment
// Returns true if any filter->set_pattern() returns true,
//	implying rereading the data is needed (i.e., a filter changed)
//
bool
dbeUpdateFilters (int dbevindex, Vector<bool> *selected, Vector<char *> *pattern_str)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->clear_error_msg ();
  dbev->clear_warning_msg ();

  // Get index of first selected experiment
  int size = selected->size ();
  int nselexp = -1;
  for (int index = 0; index < size; index++)
    {
      if (selected->fetch (index) == true)
	{
	  nselexp = index;
	  break;
	}
    }
  if (nselexp == -1) // No experiment selected
    return false;

  bool ret = false;
  for (int j = 0; j < size; j++)
    {
      if (selected->fetch (j) == false)
	continue;
      bool error;
      if (dbev->set_pattern (j, pattern_str, &error))
	ret = true;
    }
  dbev->update_advanced_filter ();
  return ret;
}

char *
dbeComposeFilterClause (int dbevindex, int type, int subtype, Vector<int> *selections)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  // ask the cached data to generate the string
  Hist_data *data;
  switch (type)
    {
    case DSP_FUNCTION:
      data = dbev->func_data;
      break;
    case DSP_DLAYOUT:
      data = dbev->dlay_data;
      break;
    case DSP_DATAOBJ:
      data = dbev->dobj_data;
      break;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      data = dbev->get_indxobj_data (subtype);
      break;
    case DSP_LINE:
      data = dbev->line_data;
      break;
    case DSP_PC:
      data = dbev->pc_data;
      break;
    case DSP_SOURCE:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
      data = dbev->dis_data;
      break;
    case DSP_IOACTIVITY:
      data = dbev->iofile_data;
      break;
    case DSP_IOVFD:
      data = dbev->iovfd_data;
      break;
    case DSP_IOCALLSTACK:
      data = dbev->iocs_data;
      break;
    case DSP_HEAPCALLSTACK:
      data = dbev->heapcs_data;
      break;
    default:
      return NULL;
    }
  if (data == NULL)
    return NULL;

  // Get array of object indices, and compose filter string
  Vector<uint64_t> *obj_ids = data->get_object_indices (selections);
  if (obj_ids == NULL || obj_ids->size () == 0)
    return NULL;

  uint64_t sel;
  int index;
  int found = 0;
  char buf[128];
  StringBuilder sb;
  sb.append ('(');
  switch (type)
    {
    case DSP_LINE:
    case DSP_PC:
    case DSP_SOURCE:
    case DSP_DISASM:
    case DSP_FUNCTION:
      sb.append (NTXT ("LEAF IN "));
      break;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      sb.append (dbeSession->getIndexSpaceName (subtype));
      sb.append (NTXT (" IN "));
      break;
    }
  Vec_loop (uint64_t, obj_ids, index, sel)
  {
    if (found == 0)
      {
	found = 1;
	sb.append ('(');
      }
    else
      sb.append (NTXT (", "));
    snprintf (buf, sizeof (buf), NTXT ("%llu"), (long long) sel);
    sb.append (buf);
  }
  if (found == 1)
    sb.append (')');

  switch (type)
    {
    case DSP_DLAYOUT:
    case DSP_DATAOBJ:
      sb.append (NTXT (" SOME IN DOBJ"));
      break;
    }
  sb.append (')');
  return sb.toString ();
}

//
// Get load object states
//
Vector<void *> *
dbeGetLoadObjectList (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  int size = lobjs->size ();

  // Initialize Java boolean array
  Vector<char *> *names = new Vector<char *>(size);
  Vector<int> *states = new Vector<int>(size);
  Vector<int> *indices = new Vector<int>(size);
  Vector<char *> *paths = new Vector<char *>(size);
  Vector<int> *isJava = new Vector<int>(size);

  // Get load object states
  int index;
  LoadObject *lo;
  char *lo_name;

  // lobjectsNoJava is a trimmed list of indices provided to front-end skipping the Java
  // classes. lobjectsNoJava preserves the mapping of the index into the complete lobjs
  // vector. What front-end sees as lobj[i] is really lobj[lobjectsNoJava[i]];

  // This list is constructed every time GetLoadObjectList() or GetLoadObjectState() is
  // called. Possibility of further optimization by making it more persistent.
  // Only consumer of this list is dbeSetLoadObjectState
  int new_index = 0;
  if (dbev->lobjectsNoJava == NULL)
    dbev->lobjectsNoJava = new Vector<int>(1);
  else
    dbev->lobjectsNoJava->reset ();

  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    // Set 0, 1, or 2 for show/hide/api
    enum LibExpand expand = dbev->get_lo_expand (lo->seg_idx);

    lo_name = lo->get_name ();
    if (lo_name != NULL)
      {
	size_t len = strlen (lo_name);
	if (len > 7 && streq (lo_name + len - 7, NTXT (".class>")))
	  isJava->store (new_index, 1);
	else
	  isJava->store (new_index, 0);
      }
    else
      isJava->store (new_index, 0);
    dbev->lobjectsNoJava->append (index);

    names->store (new_index, dbe_sprintf (NTXT ("%s"), lo_name));
    states->store (new_index, (int) expand);
    indices->store (new_index, (int) lo->seg_idx);
    paths->store (new_index, dbe_sprintf (NTXT ("%s"), lo->get_pathname ()));
    new_index++;
  }
  Vector<void*> *res = new Vector<void*>(5);
  res->store (0, names);
  res->store (1, states);
  res->store (2, indices);
  res->store (3, paths);
  res->store (4, isJava);
  delete lobjs;
  return res;
}

Vector<int> *
dbeGetLoadObjectState (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();
  int size = lobjs->size ();

  // Initialize Java boolean array
  Vector<int> *states = new Vector<int>(size);
  char *lo_name;

  // lobjectsNoJava is a trimmed list of indices provided to front-end skipping the Java
  // classes. lobjectsNoJava preserves the mapping of the index into the complete lobjs
  // vector. What front-end sees as lobj[i] is really lobj[lobjectsNoJava[i]];

  // This list is constructed every time GetLoadObjectList() or GetLoadObjectState() is
  // called. Possibility of further optimization by making it more persistent.
  // Only consumer of this list is dbeSetLoadObjectState
  int new_index = 0;
  if (dbev->lobjectsNoJava == NULL)
    dbev->lobjectsNoJava = new Vector<int>(1);
  else
    dbev->lobjectsNoJava->reset ();

  // Get load object states
  int index;
  LoadObject *lo;

  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    // Set 0, 1, or 2 for show/hide/api
    lo_name = lo->get_name ();
    if (lo_name != NULL)
      {
	size_t len = strlen (lo_name);
	if (len > 7 && streq (lo_name + len - 7, NTXT (".class>")))
	  continue;
      }
    else
      dbev->lobjectsNoJava->append (index);

    enum LibExpand expand = dbev->get_lo_expand (lo->seg_idx);
    states->store (new_index, (int) expand);
    new_index++;
  }
  delete lobjs;
  return states;
}

// Set load object states
void
dbeSetLoadObjectState (int dbevindex, Vector<int> *selected)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<LoadObject*> *lobjs = dbeSession->get_text_segments ();

  int index;
  bool changed = false;

  LoadObject *lo;
  int new_index = 0;
  dbev->setShowAll ();
  Vec_loop (LoadObject*, lobjs, index, lo)
  {
    if (dbev->lobjectsNoJava != NULL)
      {
	// This loadobject is a java class and was skipped
	if (dbev->lobjectsNoJava->fetch (new_index) != index)
	  continue;
      }
    // Get array of settings
    enum LibExpand expand = (enum LibExpand) selected->fetch (new_index);
    if (expand == LIBEX_HIDE)
      {
	dbev->resetShowAll ();
	dbeSession->set_lib_visibility_used ();
      }
    changed = changed | dbev->set_libexpand (lo->get_pathname (), expand);
    new_index++;
  }
  delete lobjs;
  if (changed == true)
    {
      dbev->setShowHideChanged ();
      dbev->update_lo_expands ();
    }

  return;
}

// Reset load object states
void
dbeSetLoadObjectDefaults (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_libdefaults ();
}

// Get  Machine model
Vector<char*>*
dbeGetCPUVerMachineModel (int dbevindex)
{
  Vector<char*>* table = new Vector<char*>();
  DbeView *dbev = dbeSession->getView (dbevindex);
  char * mach_model = dbev->get_settings ()->get_machinemodel ();
  if (mach_model != NULL)
    {
      table->append (mach_model);
      return table;
    }
  int grsize = dbeSession->expGroups->size ();
  for (int j = 0; j < grsize; j++)
    {
      ExpGroup *gr = dbeSession->expGroups->fetch (j);
      Vector<Experiment*> *exps = gr->exps;
      for (int i = 0, sz = exps->size (); i < sz; i++)
	{
	  Experiment *exp = exps->fetch (i);
	  char *model = exp->machinemodel;
	  if (model != NULL)
	    table->append (dbe_strdup (model));
	}
    }
  return table;
}

// automatically load machine model if applicable
void
dbeDetectLoadMachineModel (int dbevindex)
{
  if (dbeSession->is_datamode_available ())
    {
      char *model = dbeGetMachineModel ();
      if (model == NULL)
	{
	  Vector<char*>* models = dbeGetCPUVerMachineModel (dbevindex);
	  char * machineModel = NTXT ("generic");
	  if (models->size () > 0)
	    {
	      machineModel = models->get (0);
	      for (int i = 1; i < models->size (); i++)
		{
		  if (strncmp (models->get (i), machineModel, strlen (machineModel)) == 0)
		    {
		      machineModel = NTXT ("generic");
		      break;
		    }
		}
	      dbeLoadMachineModel (machineModel);
	    }
	  delete models;
	}
    }
}

// Managing Memory Objects
char *
dbeDefineMemObj (char *name, char *index_expr, char *machinemodel,
		 char *sdesc, char *ldesc)
{
  return MemorySpace::mobj_define (name, index_expr, machinemodel, sdesc, ldesc);
}

char *
dbeDeleteMemObj (char *name)
{
  return MemorySpace::mobj_delete (name);
}

Vector<void*> *
dbeGetMemObjects (int /*dbevindex*/)
{
  Vector<void*> *res = MemorySpace::getMemObjects ();
  return res;
}

// Managing machine model
char *
dbeLoadMachineModel (char *name)
{
  return dbeSession->load_mach_model (name);
}

char *
dbeGetMachineModel ()
{
  return dbeSession->get_mach_model ();
}

Vector <char *> *
dbeListMachineModels ()
{
  return dbeSession->list_mach_models ();
}

// Managing Index Objects
char *
dbeDefineIndxObj (char *name, char *index_expr, char *sdesc, char *ldesc)
{
  return dbeSession->indxobj_define (name, NULL, index_expr, sdesc, ldesc);
}

Vector<void*> *
dbeGetIndxObjDescriptions (int /*dbevindex*/)
{
  Vector<void*> *res = dbeSession->getIndxObjDescriptions ();
  return res;
}

Vector<void*> *
dbeGetCustomIndxObjects (int /*dbevindex*/)
{
  Vector<void*> *res = dbeSession->getCustomIndxObjects ();
  return res;
}

void
dbeSetSelObj (int dbevindex, Obj sel_obj_or_ind, int type, int subtype)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable *sel_obj;
  Hist_data *data;
  int sel_ind = (int) sel_obj_or_ind;

  switch (type)
    {
    case DSP_FUNCTION:
      data = dbev->func_data;
      break;
    case DSP_LINE:
      data = dbev->line_data;
      break;
    case DSP_PC:
      data = dbev->pc_data;
      break;
    case DSP_CALLER:
      data = dbev->callers;
      break;
    case DSP_CALLEE:
      data = dbev->callees;
      break;
    case DSP_SOURCE:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
      data = dbev->dis_data;
      break;
    case DSP_DLAYOUT:
      data = dbev->dlay_data;
      if (data == NULL)
	{
	  dbev->sel_binctx = NULL;
	  return;
	}
      if (sel_ind >= 0 && sel_ind < dbev->dlay_data->size ())
	dbev->sel_dobj = dbev->dlay_data->fetch (sel_ind)->obj;
      return;
    case DSP_DATAOBJ:
      data = dbev->dobj_data;
      if (data == NULL)
	{
	  dbev->sel_binctx = NULL;
	  return;
	}
      if (sel_ind >= 0 && sel_ind < dbev->dobj_data->size ())
	dbev->sel_dobj = dbev->dobj_data->fetch (sel_ind)->obj;
      return;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      dbev->set_indxobj_sel (subtype, sel_ind);
      sel_obj = dbev->get_indxobj_sel (subtype);
      if (sel_obj && sel_obj->get_type () == Histable::INDEXOBJ)
	dbev->set_sel_obj (((IndexObject*) sel_obj)->get_obj ());
      return;
    case DSP_SOURCE_V2:
    case DSP_DISASM_V2:
    case DSP_TIMELINE:
    case DSP_LEAKLIST:
    case DSP_RACES:
    case DSP_DEADLOCKS:
    case DSP_DUALSOURCE:
    case DSP_SOURCE_DISASM:
    case DSP_IOACTIVITY:
    case DSP_IOVFD:
    case DSP_IOCALLSTACK:
    case DSP_HEAPCALLSTACK:
    case DSP_MINICALLER:
      dbev->set_sel_obj ((Histable *) sel_obj_or_ind);
      return;
    default:
      // abort();
      return;
    }
  if (type != DSP_SOURCE && type != DSP_DISASM && type != DSP_SOURCE_V2
      && type != DSP_DISASM_V2)
    dbev->sel_binctx = NULL;

  if (data == NULL || data->get_status () != Hist_data::SUCCESS
      || sel_ind >= data->size ())
    return;

  if (sel_ind >= 0 && sel_ind < data->size ())
    dbev->set_sel_obj (data->fetch (sel_ind)->obj);
}

void
dbeSetSelObjV2 (int dbevindex, uint64_t id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->set_sel_obj (dbeSession->findObjectById (id));
}

Obj
dbeGetSelObj (int dbevindex, int type, int subtype)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  Histable *sel_obj = NULL;
  switch (type)
    {
    case DSP_FUNCTION:
      sel_obj = dbev->get_sel_obj (Histable::FUNCTION);
      break;
    case DSP_LINE:
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      sel_obj = dbev->get_sel_obj (Histable::LINE);
      break;
    case DSP_PC:
    case DSP_DISASM:
    case DSP_DISASM_V2:
      sel_obj = dbev->get_sel_obj (Histable::INSTR);
      break;
    case DSP_SRC_FILE:
      sel_obj = dbev->get_sel_obj (Histable::SOURCEFILE);
      break;
    case DSP_DATAOBJ:
    case DSP_DLAYOUT:
      if (dbev->sel_dobj)
	sel_obj = dbev->sel_dobj->convertto (Histable::DOBJECT);
      break;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      sel_obj = dbev->get_indxobj_sel (subtype);
      break;
    default:
      abort ();
    }
  Dprintf (DEBUG_DBE, NTXT ("### dbeGetSelObj: Dbe.cc:%d %s (%d) returns %s\n"),
	   __LINE__, dsp_type_to_string (type), type, sel_obj ? sel_obj->dump () : "NULL");
  return (Obj) sel_obj;
}

Obj
dbeConvertSelObj (Obj obj, int type)
{
  Histable *sel_obj = (Histable *) obj;
  Dprintf (DEBUG_DBE, NTXT ("### dbeConvertSelObj: Dbe.cc:%d %s (%d) sel_obj=%s\n"),
	   __LINE__, dsp_type_to_string (type), type, sel_obj ? sel_obj->dump ()
	  : "NULL");
  if (sel_obj == NULL)
    return (Obj) NULL;
  switch (type)
    {
    case DSP_FUNCTION:
      return (Obj) sel_obj->convertto (Histable::FUNCTION);
    case DSP_LINE:
      return (Obj) sel_obj->convertto (Histable::LINE);
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      {
	SourceFile* srcCtx = NULL;
	if (sel_obj->get_type () == Histable::INSTR)
	  {
	    DbeInstr* dbei = (DbeInstr *) sel_obj;
	    srcCtx = (SourceFile*) dbei->convertto (Histable::SOURCEFILE);
	  }
	else if (sel_obj->get_type () == Histable::LINE)
	  {
	    DbeLine * dbel = (DbeLine *) sel_obj;
	    srcCtx = dbel->sourceFile;
	  }
	sel_obj = sel_obj->convertto (Histable::LINE, srcCtx);
	Dprintf (DEBUG_DBE, NTXT ("### dbeConvertSelObj: Dbe.cc:%d %s (%d) returns %s\n"),
		 __LINE__, dsp_type_to_string (type), type, sel_obj ? sel_obj->dump () : "NULL");
	if (sel_obj && sel_obj->get_type () == Histable::LINE)
	  {
	    DbeLine * dbel = (DbeLine *) sel_obj;
	    return (Obj) dbel->dbeline_base;
	  }
	return (Obj) sel_obj->convertto (Histable::LINE, srcCtx);
      }
    case DSP_PC:
    case DSP_DISASM:
    case DSP_DISASM_V2:
      return (Obj) sel_obj->convertto (Histable::INSTR);
    case DSP_SRC_FILE:
      return (Obj) sel_obj->convertto (Histable::SOURCEFILE);
    default:
      abort ();
    }
  return (Obj) NULL;
}

uint64_t
dbeGetSelObjV2 (int dbevindex, char *typeStr)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable *obj = NULL;
  if (typeStr != NULL)
    {
      if (streq (typeStr, NTXT ("FUNCTION")))
	obj = dbev->get_sel_obj (Histable::FUNCTION);
      else if (streq (typeStr, NTXT ("INSTRUCTION")))
	obj = dbev->get_sel_obj (Histable::INSTR);
      else if (streq (typeStr, NTXT ("SOURCELINE")))
	obj = dbev->get_sel_obj (Histable::LINE);
      else if (streq (typeStr, NTXT ("SOURCEFILE")))
	obj = dbev->get_sel_obj (Histable::SOURCEFILE);
    }
  Dprintf (DEBUG_DBE, NTXT ("### dbeGetSelObjV2: Dbe.cc:%d %s returns %s\n"),
	   __LINE__, STR (typeStr), obj ? obj->dump () : "NULL");
  return obj != NULL ? obj->id : (uint64_t) - 1;
}

Vector<uint64_t> *
dbeGetSelObjsIO (int dbevindex, Vector<uint64_t> *ids, int type)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<uint64_t> *res = NULL;
  Vector<uint64_t> *result = new Vector<uint64_t>();
  for (int i = 0; i < ids->size (); i++)
    {
      res = dbeGetSelObjIO (dbevindex, ids->fetch (i), type);
      if (res != NULL)
	{
	  result->addAll (res);
	  delete res;
	}
    }
  return result;
}

Vector<uint64_t> *
dbeGetSelObjIO (int dbevindex, uint64_t id, int type)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable *obj = NULL;
  Vector<uint64_t> *res = NULL;
  int size = 0;
  switch (type)
    {
    case DSP_IOACTIVITY:
      obj = dbev->get_sel_obj_io (id, Histable::IOACTFILE);
      size = obj != NULL ? ((FileData*) obj)->getVirtualFds ()->size () : 0;
      if (size)
	{
	  res = new Vector<uint64_t>();
	  Vector<int64_t> *vfds = ((FileData*) obj)->getVirtualFds ();
	  for (int i = 0; i < size; i++)
	    res->append (vfds->fetch (i));
	}
      break;
    case DSP_IOVFD:
      obj = dbev->get_sel_obj_io (id, Histable::IOACTVFD);
      if (obj)
	{
	  res = new Vector<uint64_t>();
	  res->append (obj->id);
	}
      break;
    case DSP_IOCALLSTACK:
      obj = dbev->get_sel_obj_io (id, Histable::IOCALLSTACK);
      if (obj)
	{
	  Vector<Obj> *instrs = dbeGetStackPCs (dbevindex, obj->id);
	  if (instrs == NULL)
	    return NULL;
	  int stsize = instrs->size ();
	  res = new Vector<uint64_t>(stsize);
	  for (int i = 0; i < stsize; i++)
	    {
	      Histable *objFunc = (DbeInstr*) (instrs->fetch (i));
	      if (objFunc->get_type () != Histable::LINE)
		{
		  objFunc = objFunc->convertto (Histable::FUNCTION);
		  res->insert (0, objFunc->id);
		}
	    }
	  delete instrs;
	}
      break;
    default:
      break;
    }
  return res;
}

uint64_t
dbeGetSelObjHeapTimestamp (int dbevindex, uint64_t id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable *obj = NULL;
  uint64_t res = 0;
  Vector<uint64_t> *peakStackIds;
  Vector<hrtime_t> *peakTimestamps;

  // Find and return the timestamp for the peak
  bool foundPeakId = false;
  if (id > 0)
    {
      obj = dbev->get_sel_obj_heap (0);
      if (obj != NULL)
	{
	  peakStackIds = ((HeapData*) obj)->getPeakStackIds ();
	  peakTimestamps = ((HeapData*) obj)->getPeakTimestamps ();
	  for (int i = 0; i < peakStackIds->size (); i++)
	    {
	      if (id == peakStackIds->fetch (i))
		{
		  res = peakTimestamps->fetch (i);
		  foundPeakId = true;
		  break;
		}
	    }
	}
    }

  // Return the first timestamp for the peak
  // if the callstack id is zero or it
  // doesn't match with the peak stack id
  if (id == 0 || !foundPeakId)
    {
      obj = dbev->get_sel_obj_heap (0);
      res = obj != NULL ? ((HeapData*) obj)->getPeakTimestamps ()->fetch (0) : 0;
    }
  return res;
}

int
dbeGetSelObjHeapUserExpId (int dbevindex, uint64_t id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable *obj = NULL;
  int res = 0;
  obj = dbev->get_sel_obj_heap (id);
  res = obj != NULL ? ((HeapData*) obj)->getUserExpId () : 0;
  return res;
}

//
// Get index of selected function/object
//
int
dbeGetSelIndex (int dbevindex, Obj sel_obj, int type, int subtype)
{
  Hist_data *data;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  switch (type)
    {
    case DSP_FUNCTION:
      data = dbev->func_data;
      break;
    case DSP_LINE:
      data = dbev->line_data;
      break;
    case DSP_PC:
      data = dbev->pc_data;
      break;
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dbev->dis_data;
      break;
    case DSP_DLAYOUT:
      data = dbev->dlay_data;
      break;
    case DSP_DATAOBJ:
      data = dbev->dobj_data;
      break;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      data = dbev->get_indxobj_data (subtype);
      break;
    default:
      data = NULL;
      break;
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return -1;

  Histable *chk_obj = (Histable *) sel_obj;
  Vector<Hist_data::HistItem*> *histItems = data->get_hist_items ();
  if (histItems == NULL || chk_obj == NULL)
    return -1;
  for (int i = 0, sz = histItems->size (); i < sz; i++)
    {
      if (histItems->get (i)->obj == chk_obj)
	return i;
      if (histItems->get (i)->obj == NULL)
	continue;
      if (histItems->get (i)->obj->get_type () == Histable::LINE
	  && chk_obj->get_type () == Histable::LINE)
	{
	  if (((DbeLine*) histItems->get (i)->obj)->convertto (Histable::FUNCTION)
	      == ((DbeLine*) chk_obj)->convertto (Histable::FUNCTION)
	      && ((DbeLine*) histItems->get (i)->obj)->lineno
	      == ((DbeLine*) chk_obj)->lineno)
	    return i;
	}
      else if (histItems->get (i)->obj->get_type () == Histable::INSTR
	 && chk_obj->get_type () == Histable::INSTR)
	if (((DbeInstr*) histItems->get (i)->obj)->convertto (Histable::FUNCTION)
	    == ((DbeInstr*) chk_obj)->convertto (Histable::FUNCTION)
	    && ((DbeInstr*) histItems->get (i)->obj)->addr
	    == ((DbeInstr*) chk_obj)->addr)
	  return i;
    }

  Histable *chk_obj1 = NULL;
  switch (type)
    {
    case DSP_FUNCTION:
      chk_obj1 = chk_obj->convertto (Histable::FUNCTION);
      break;
    case DSP_LINE:
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      chk_obj1 = chk_obj->convertto (Histable::LINE);
      break;
    case DSP_PC:
    case DSP_DISASM:
    case DSP_DISASM_V2:
      chk_obj1 = chk_obj->convertto (Histable::INSTR);
      break;
    }
  if (chk_obj1 && chk_obj != chk_obj1)
    for (int i = 0, sz = histItems->size (); i < sz; i++)
      if (histItems->get (i)->obj == chk_obj1)
	return i;

  if (type == DSP_LINE)
    {
      for (int i = 0, sz = histItems->size (); i < sz; i++)
	if (histItems->get (i)->obj != NULL
	    && chk_obj->convertto (Histable::FUNCTION)
	    == histItems->get (i)->obj->convertto (Histable::FUNCTION))
	  return i;
    }
  else if (type == DSP_PC)
    {
      for (int i = 0, sz = histItems->size (); i < sz; i++)
	if (histItems->get (i)->obj != NULL
	    && (histItems->get (i)->obj)->convertto (Histable::FUNCTION)
	    == (chk_obj)->convertto (Histable::FUNCTION)
	    && ((DbeLine*) histItems->get (i)->obj->convertto (Histable::LINE))->lineno
	    == ((DbeLine*) chk_obj->convertto (Histable::LINE))->lineno)
	  return i;
      for (int i = 0, sz = histItems->size (); i < sz; i++)
	if (histItems->get (i)->obj != NULL
	    && (histItems->get (i)->obj)->convertto (Histable::FUNCTION)
	    == (chk_obj)->convertto (Histable::FUNCTION))
	  return i;
    }

  // If we clicked on an mfunction line in the called-by call mini in user mode for omp
  // we might not find that function in func data
  if (dbev->isOmpDisMode () && type == DSP_FUNCTION)
    {
      int p = dbeGetSelIndex (dbevindex, sel_obj, DSP_DISASM, subtype);
      if (p != -1)
	return p;
    }
  return -1;
}

// Print data
//
char *
dbePrintData (int dbevindex, int type, int subtype, char *printer,
	      char *fname, FILE *outfile)
{
  Histable *current_obj;
  Function *func;
  Module *module;
  MetricList *mlist_orig;
  bool header;
  Print_params params;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();

  // Set print parameters
  if (printer != NULL)
    {
      params.dest = DEST_PRINTER;
      params.name = printer;
    }
  else if (outfile != NULL)
    {
      params.dest = DEST_OPEN_FILE;
      params.openfile = outfile;
      params.name = NULL;
    }
  else
    {
      params.dest = DEST_FILE;
      params.name = fname;
      if (*(params.name) == '\0')
	{
	  free (params.name);
	  return dbe_strdup (GTXT ("Please enter the name of the file to which to print"));
	}
    }
  params.ncopies = 1;
  if (outfile != NULL)
    header = false;
  else
    header = !(type == DSP_SOURCE || type == DSP_SOURCE_V2 || type == DSP_DISASM_V2);

  params.header = header;

  // figure out what kind of metrics to use
  if (type == DSP_SELF || type == DSP_CALLER || type == DSP_CALLEE
      || type == DSP_CALLTREE)
    mlist_orig = dbev->get_metric_list (MET_CALL);
  else if (type == DSP_DATAOBJ || type == DSP_DLAYOUT || type == DSP_MEMOBJ)
    mlist_orig = dbev->get_metric_list (MET_DATA);
  else if (type == DSP_INDXOBJ)
    mlist_orig = dbev->get_metric_list (MET_INDX);
  else if (type == DSP_IOACTIVITY || type == DSP_IOVFD
	   || type == DSP_IOCALLSTACK)
    mlist_orig = dbev->get_metric_list (MET_IO);
  else if (type == DSP_HEAPCALLSTACK)
    mlist_orig = dbev->get_metric_list (MET_HEAP);
  else
    mlist_orig = dbev->get_metric_list (MET_NORMAL);

  // make a compacted version of the input list
  // the list will either be moved to the generated data,
  //   or freed below if it wasn't needed
  MetricList *mlist = new MetricList (mlist_orig);
  Hist_data *data = NULL;
  er_print_common_display *cd = NULL;
  int ix;
  // Set data
  switch (type)
    {
    case DSP_FUNCTION:
    case DSP_LINE:
    case DSP_PC:
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
    case DSP_DATAOBJ:
      data = dbev->get_hist_data (mlist,
				  ((type == DSP_FUNCTION) ? Histable::FUNCTION :
				   (type == DSP_LINE) ? Histable::LINE :
				   (type == DSP_PC) ? Histable::INSTR :
				   (type == DSP_INDXOBJ) ? Histable::INDEXOBJ :
				   (type == DSP_MEMOBJ) ? Histable::MEMOBJ
				   : Histable::DOBJECT),
				  subtype, Hist_data::ALL);
      if (data->get_status () != Hist_data::SUCCESS)
	return DbeView::status_str (DbeView::DBEVIEW_NO_DATA); // no strdup()

      cd = new er_print_histogram (dbev, data, mlist, MODE_LIST,
				   dbev->get_limit (),
				   mlist->get_sort_name (), NULL, true, true);
      break;
    case DSP_DLAYOUT:
      {
	data = dbev->get_hist_data (mlist, Histable::DOBJECT, 0, Hist_data::LAYOUT);
	if (data->get_status () != Hist_data::SUCCESS)
	  return DbeView::status_str (DbeView::DBEVIEW_NO_DATA); // no strdup()
	cd = new er_print_histogram (dbev, data, mlist, MODE_ANNOTATED,
				     dbev->get_thresh_dis (),
				     mlist->get_sort_name (), NULL, true, true);
	break;
      }

      // source and disassembly
    case DSP_SOURCE:
    case DSP_DISASM:
    case DSP_SOURCE_V2:
    case DSP_DISASM_V2:
      if (dbev->sel_obj == NULL)
	return NULL;
      current_obj = dbev->sel_obj->convertto (Histable::FUNCTION);
      if (current_obj->get_type () != Histable::FUNCTION)
	return dbe_strdup (GTXT ("Not a real function; no source or disassembly available."));
      func = (Function*) current_obj->convertto (Histable::FUNCTION);
      if (func->flags & FUNC_FLAG_SIMULATED)
	return dbe_strdup (GTXT ("Not a real function; no source or disassembly available."));
      if (func->get_name () == NULL)
	return dbe_strdup (GTXT ("Source location not recorded in experiment"));
      module = func->module;
      if (module == NULL || module->get_name () == NULL)
	return dbe_strdup (GTXT ("Object name not recorded in experiment"));
      ix = module->loadobject->seg_idx;
      if (dbev->get_lo_expand (ix) == LIBEX_HIDE)
	return dbe_strdup (GTXT ("No source or disassembly available for hidden object"));
      cd = new er_print_histogram (dbev, dbev->func_data, mlist, MODE_ANNOTATED,
				   type == DSP_DISASM || type == DSP_DISASM_V2,
				   mlist->get_sort_name (),
				   func, false, false);
      break;

      // callers-callees
    case DSP_SELF:
    case DSP_CALLER:
    case DSP_CALLEE:
      if (dbev->sel_obj == NULL)
	return NULL;
      current_obj = dbev->sel_obj->convertto (Histable::FUNCTION);
      cd = new er_print_histogram (dbev, dbev->func_data, mlist, MODE_GPROF, 1,
				   mlist->get_sort_name (), current_obj,
				   false, false);
      break;

      // statistics; this won't use the metric list copied above, so delete it
    case DSP_STATIS:
      cd = new er_print_experiment (dbev, 0, dbeSession->nexps () - 1,
				    true, true, true, true, false);
      delete mlist;
      break;
    case DSP_EXP:
      cd = new er_print_experiment (dbev, 0, dbeSession->nexps () - 1,
				    true, true, false, false, false);
      delete mlist;
      break;
    case DSP_LEAKLIST:
      cd = new er_print_leaklist (dbev, true, true, dbev->get_limit ());
      delete mlist;
      break;
    case DSP_HEAPCALLSTACK:
      cd = new er_print_heapactivity (dbev, Histable::HEAPCALLSTACK, false,
				      dbev->get_limit ());
      delete mlist;
      break;
    case DSP_IOACTIVITY:
      cd = new er_print_ioactivity (dbev, Histable::IOACTFILE, false,
				    dbev->get_limit ());
      delete mlist;
      break;
    case DSP_IOVFD:
      cd = new er_print_ioactivity (dbev, Histable::IOACTVFD, false,
				    dbev->get_limit ());
      delete mlist;
      break;

      // the io call stack
    case DSP_IOCALLSTACK:
      cd = new er_print_ioactivity (dbev, Histable::IOCALLSTACK, false,
				    dbev->get_limit ());
      delete mlist;
      break;

      // some unknown panel -- return an error string
    default:
      delete mlist;
      return dbe_strdup (GTXT ("Print not available"));
    }

  // Start printing
  char *buf = NULL;

  // first open the file/device/whatever
  if (cd->open (&params) == 0)
    {
      // now call the actual print routine
      cd->data_dump ();
      if (params.dest == DEST_PRINTER)
	{
	  if (streq ((char *) params.name, NTXT ("-")))
	    {
	      // Special case - return report to the GUI
	      int maxbytes = 2 * 1024 * 1024; // IPC large buffer limit
	      char *report = cd->get_output (maxbytes);
	      delete data;
	      delete cd;
	      return report; // TEMPORARY
	    }
	}
      if (cd->print_output () == false)
	buf = dbe_sprintf (NTXT ("%s: %s"),
			   GTXT ("Unable to submit print request to"),
			   params.name);
    }
  else
    // if unable to set up the print, return an error
    buf = dbe_sprintf (NTXT ("%s: %s"),
		       GTXT ("Unable to open file"),
		       params.name);

  // dbe_free((void *) params.name); XXX when should this happen?
  if (data)
    if (data->isViewOwned () == false)
      delete data;
  delete cd;
  return buf;
}

// Set limit for print data
//
char *
dbeSetPrintLimit (int dbevindex, int limit)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  return (dbev->set_limit (limit));
}

// get limit for print data
int
dbeGetPrintLimit (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  int limit = dbev->get_limit ();
  return limit;
}

// set printmode for data
char *
dbeSetPrintMode (int dbevindex, char * pmode)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  char *r = dbev->set_printmode (pmode);
  return r;
}

// get printmode for data
int
dbeGetPrintMode (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  return (dbev->get_printmode ());
}

// get printmode for data
char *
dbeGetPrintModeString (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  return ( dbev->get_printmode_str ());
}

// get print delimiter for csv data
char
dbeGetPrintDelim (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  return (dbev->get_printdelimiter ());
}

// Set Source/Object/Load-Object file names
static void
set_file_names (Function *func, char *names[3])
{
  Module *module = func->module;
  LoadObject *loadobject = module->loadobject;
  if (loadobject == NULL)
    loadobject = dbeSession->get_Unknown_LoadObject ();
  free (names[0]);
  free (names[1]);
  free (names[2]);
  SourceFile *sf = func->getDefSrc ();
  char *src_name = sf->dbeFile->get_location_info ();
  DbeFile *df = module->dbeFile;
  if (df == NULL || (df->filetype & DbeFile::F_JAVACLASS) == 0)
    df = module->loadobject->dbeFile;
  char *lo_name = df->get_location_info ();
  char *dot_o_name = lo_name;
  if (module->dot_o_file)
    dot_o_name = module->dot_o_file->dbeFile->get_location_info ();
  names[0] = dbe_sprintf (NTXT ("%s: %s"), GTXT ("Source File"), src_name);
  names[1] = dbe_sprintf (NTXT ("%s: %s"), GTXT ("Object File"), dot_o_name);
  names[2] = dbe_sprintf (NTXT ("%s: %s"), GTXT ("Load Object"), lo_name);
}

// dbeSetFuncData
//	Master function to generate all Tab data for the analyzer
//	Returns the index of the selected item in the specified list
//
// After calling it to set up, the Analyzer calls dbeGetFuncList
//	to format the generated data and return the table
//	Most of the data is destined for a JTable
//
int
dbeSetFuncData (int dbevindex, Obj sel_obj, int type, int subtype)
{
  MetricList *_mlist;
  Histable *org_obj;
  Hist_data *data = NULL;
  int index, sel_index;
  Function *func;
  char *name;
  int ix;

  DbeView *dbev = dbeSession->getView (dbevindex);
  sel_index = -1;
  dbev->resetOmpDisMode ();
  dbev->error_msg = dbev->warning_msg = NULL;

  // get metric list, make a compact duplicate
  _mlist = dbev->get_metric_list (MET_NORMAL);
  MetricList *mlist = new MetricList (_mlist);

  // Remove old function/obj list data & Get new function/obj list data
  org_obj = (Histable *) sel_obj;

  // Figure out which "function" data is being asked for, i.e.,
  //	which of the analyzer displays is asking for data
  switch (type)
    {
      // the various tables: functions, lines, PCs, DataObjects, IndexObjects
    case DSP_FUNCTION:
    case DSP_LINE:
    case DSP_PC:
    case DSP_DATAOBJ:
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      switch (type)
	{
	case DSP_FUNCTION:
	  if (dbev->func_data)
	    delete dbev->func_data;
	  dbev->func_data = data = dbev->get_hist_data (mlist,
				   Histable::FUNCTION, subtype, Hist_data::ALL);
	  break;
	case DSP_LINE:
	  if (dbev->line_data)
	    delete dbev->line_data;
	  dbev->line_data = data = dbev->get_hist_data (mlist,
				       Histable::LINE, subtype, Hist_data::ALL);
	  break;
	case DSP_PC:
	  if (dbev->pc_data)
	    delete dbev->pc_data;
	  dbev->pc_data = data = dbev->get_hist_data (mlist,
				      Histable::INSTR, subtype, Hist_data::ALL);
	  break;
	case DSP_DATAOBJ:
	  if (dbev->dobj_data)
	    delete dbev->dobj_data;
	  mlist = dbev->get_metric_list (MET_DATA);
	  dbev->dobj_data = data = dbev->get_hist_data (mlist,
				    Histable::DOBJECT, subtype, Hist_data::ALL);
	  break;
	case DSP_MEMOBJ:
	  mlist = dbev->get_metric_list (MET_DATA);
	  data = dbev->get_hist_data (mlist, Histable::MEMOBJ, subtype,
				      Hist_data::ALL);
	  dbev->indx_data->store (subtype, data);
	  break;
	case DSP_INDXOBJ:
	  mlist = dbev->get_metric_list (MET_INDX);
	  data = dbev->get_hist_data (mlist, Histable::INDEXOBJ, subtype,
				      Hist_data::ALL);
	  dbev->indx_data->store (subtype, data);
	  break;
	default:
	  break;
	}

      // Set the selection of row item
      if (data->get_status () == Hist_data::SUCCESS)
	{
	  // otherwise, look for it
	  sel_index = -1;
	  if (org_obj)
	    {
	      Hist_data::HistItem *hi;
	      Vec_loop (Hist_data::HistItem*, data->get_hist_items (), index, hi)
	      {
		if (hi->obj == org_obj)
		  {
		    sel_index = index;
		    break;
		  }
	      }
	      if (sel_index == -1 && (type == DSP_LINE || type == DSP_PC))
		{
		  Vec_loop (Hist_data::HistItem*, data->get_hist_items (), index, hi)
		  {
		    name = hi->obj->get_name ();
		    if (strcmp (name, NTXT ("<Total>")) &&
			strcmp (name, GTXT ("<Unknown>")))
		      {
			int done = 0;
			switch (type)
			  {
			  case DSP_LINE:
			    if (org_obj->convertto (Histable::FUNCTION)
				     == hi->obj->convertto (Histable::FUNCTION))
			      {
				sel_index = index;
				done = 1;
			      }
			    break;
			  case DSP_PC:
			    if (hi->obj->convertto (Histable::FUNCTION)
				== org_obj->convertto (Histable::FUNCTION)
				&& ((DbeLine*) hi->obj->convertto (Histable::LINE))->lineno
				== ((DbeLine*) org_obj->convertto (Histable::LINE))->lineno)
			      {
				sel_index = index;
				done = 1;
			      }
			    break;
			  }
			if (done)
			  break;
		      }
		  }
		}
	      if (sel_index == -1 && type == DSP_PC)
		{
		  Vec_loop (Hist_data::HistItem*, data->get_hist_items (), index, hi)
		  {
		    name = hi->obj->get_name ();
		    if (strcmp (name, NTXT ("<Total>")) &&
			strcmp (name, GTXT ("<Unknown>")))
		      {
			int done = 0;
			if (hi->obj->convertto (Histable::FUNCTION) ==
			    org_obj->convertto (Histable::FUNCTION))
			  {
			    sel_index = index;
			    done = 1;
			  }
			if (done)
			  break;
		      }
		  }
		}
	    }
	  if (sel_index == -1)
	    {
	      Hist_data::HistItem *hi;
	      Vec_loop (Hist_data::HistItem*, data->get_hist_items (), index, hi)
	      {
		name = hi->obj->get_name ();
		if (strcmp (name, NTXT ("<Total>")) &&
		    strcmp (name, GTXT ("<Unknown>")))
		  {
		    sel_index = index;
		    break;
		  }
	      }
	    }
	}
      else
	dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
      return sel_index;
      // the end of the code for the regular tables

      // Data Layout
    case DSP_DLAYOUT:
      if (dbev->dlay_data)
	delete dbev->dlay_data;
      dbev->marks->reset ();
      mlist = dbev->get_metric_list (MET_DATA);

      // initial dobj list ...
      data = dbev->get_hist_data (mlist, Histable::DOBJECT, subtype,
				  Hist_data::LAYOUT);
      // .. provides metric data for layout
      dbev->dlay_data = data = dbev->get_data_space ()->get_layout_data (data,
					  dbev->marks, dbev->get_thresh_dis ());
      if (data->get_status () != Hist_data::SUCCESS)
	dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
      return sel_index;

      // Source or disassembly
    case DSP_SOURCE_V2:
    case DSP_DISASM_V2:
    case DSP_SOURCE:
    case DSP_DISASM:
      {
	if (org_obj == NULL)
	  {
	    dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_SEL_OBJ);
	    return sel_index;
	  }
	if (org_obj->get_type () != Histable::FUNCTION)
	  {
	    dbev->error_msg = dbe_strdup (
	     GTXT ("Not a real function; no source or disassembly available."));
	    return sel_index;
	  }
	func = (Function *) org_obj;
	if (func->flags & FUNC_FLAG_SIMULATED)
	  {
	    dbev->error_msg = dbe_strdup (
	     GTXT ("Not a real function; no source or disassembly available."));
	    return sel_index;
	  }
	if (func->get_name () == NULL)
	  {
	    dbev->error_msg = dbe_strdup (
			   GTXT ("Source location not recorded in experiment"));
	    return sel_index;
	  }
	Module *module = func->module;
	if ((module == NULL) || (module->get_name () == NULL))
	  {
	    dbev->error_msg = dbe_strdup (
			       GTXT ("Object name not recorded in experiment"));
	    return sel_index;
	  }
	ix = module->loadobject->seg_idx;
	if (dbev->get_lo_expand (ix) == LIBEX_HIDE)
	  {
	    dbev->error_msg = dbe_strdup (
		 GTXT ("No source or disassembly available for hidden object"));
	    return sel_index;
	  }

	if ((type == DSP_DISASM || type == DSP_DISASM_V2)
	     && dbev->get_view_mode () == VMODE_USER
	    && dbeSession->is_omp_available ())
	  dbev->setOmpDisMode ();

	dbev->marks->reset ();
	SourceFile *srcContext = NULL;
	switch (dbev->sel_obj->get_type ())
	  {
	  case Histable::FUNCTION:
	    {
	      Function *f = (Function *) dbev->sel_obj;
	      srcContext = f->getDefSrc ();
	      dbev->sel_binctx = f->module;
	      break;
	    }
	  case Histable::LINE:
	    {
	      DbeLine *dl = (DbeLine *) dbev->sel_obj;
	      srcContext = dl->sourceFile;
	      Function *f = dl->func;
	      if (f)
		dbev->sel_binctx = f;
	      break;
	    }
	  case Histable::INSTR:
	    {
	      Function *f = (Function *) dbev->sel_obj->convertto (Histable::FUNCTION);
	      if (f)
		{
		  dbev->sel_binctx = f;
		  srcContext = f->getDefSrc ();
		}
	      break;
	    }
	  default:
	    break;
	  }
	mlist = dbev->get_metric_list (MET_SRCDIS);

	// for source and disassembly the name needs to be invisible,
	//	but that's handled in the module code
	if (type == DSP_SOURCE)
	  {
	    if (dbev->src_data)
	      delete dbev->src_data;

	    // func_data computation needed for get_totals
	    if (dbev->func_data == NULL)
	      dbev->func_data = data = dbev->get_hist_data (mlist,
				   Histable::FUNCTION, subtype, Hist_data::ALL);
	    dbev->marks2dsrc->reset ();
	    dbev->marks2dsrc_inc->reset ();
	    data = dbev->src_data = module->get_data (dbev, mlist,
			  Histable::LINE, dbev->func_data->get_totals ()->value,
			  srcContext, func, dbev->marks,
			  dbev->get_thresh_src (), dbev->get_src_compcom (),
			  dbev->get_src_visible (), dbev->get_hex_visible (),
			  false, false, dbev->marks2dsrc, dbev->marks2dsrc_inc);
	    set_file_names (func, dbev->names_src);
	    if (srcContext)
	      {
		free (dbev->names_src[0]);
		dbev->names_src[0] = dbe_sprintf (GTXT ("Source File: %s"),
				     srcContext->dbeFile->get_location_info ());
	      }
	    Obj obj = (Obj) func->convertto (Histable::LINE, srcContext);
	    sel_index = dbeGetSelIndex (dbevindex, obj, type, subtype);
	  }
	else
	  { /* type == DSP_DISASM */
	    if (dbev->dis_data)
	      delete dbev->dis_data;

	    // func_data computation needed for get_totals
	    if (dbev->func_data == NULL)
	      dbev->func_data = data = dbev->get_hist_data (mlist,
				  Histable::FUNCTION, subtype, Hist_data::ALL);
	    dbev->marks2ddis->reset ();
	    dbev->marks2ddis_inc->reset ();
	    data = dbev->dis_data = module->get_data (dbev, mlist,
			 Histable::INSTR, dbev->func_data->get_totals ()->value,
			 srcContext, func, dbev->marks, dbev->get_thresh_dis (),
			 dbev->get_dis_compcom (), dbev->get_src_visible (),
			 dbev->get_hex_visible (), dbev->get_func_scope (),
			 false, dbev->marks2ddis, dbev->marks2ddis_inc);
	    set_file_names (func, dbev->names_dis);
	    if (srcContext)
	      {
		free (dbev->names_dis[0]);
		dbev->names_dis[0] = dbe_sprintf (GTXT ("Source File: %s"),
				    srcContext->dbeFile->get_location_info ());
	      }
	    Obj obj = (Obj) func->convertto (Histable::INSTR);
	    sel_index = dbeGetSelIndex (dbevindex, obj, type, subtype);
	  }
	return sel_index;
      }

      // the three cases for caller-callee
    case DSP_SELF:
    case DSP_CALLER:
    case DSP_CALLEE:
      if (org_obj == NULL)
	{
	  dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_SEL_OBJ);
	  return sel_index;
	}

      // Caller data
      if (dbev->callers)
	delete dbev->callers;
      mlist = dbev->get_metric_list (MET_CALL);
      dbev->callers = dbev->get_hist_data (mlist, Histable::FUNCTION, subtype,
					   Hist_data::CALLERS, org_obj);
      if (dbev->callers->get_status () != Hist_data::SUCCESS)
	{
	  dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
	  return sel_index;
	}

      // Callee data
      if (dbev->callees)
	delete dbev->callees;
      dbev->callees = dbev->get_hist_data (mlist, Histable::FUNCTION, subtype,
					   Hist_data::CALLEES, org_obj);
      if (dbev->callees->get_status () != Hist_data::SUCCESS)
	{
	  dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
	  return sel_index;
	}

      // Center Function item
      if (dbev->fitem_data)
	delete dbev->fitem_data;
      dbev->fitem_data = dbev->get_hist_data (mlist, Histable::FUNCTION, subtype,
					      Hist_data::SELF, org_obj);
      if (dbev->fitem_data->get_status () != Hist_data::SUCCESS)
	{
	  dbev->error_msg = DbeView::status_str (DbeView::DBEVIEW_NO_DATA);
	  return sel_index;
	}
      return sel_index;
    default:
      abort ();
    }
  return sel_index;
}

Vector<void*>*
dbeGetTotals (int dbevindex, int dsptype, int subtype)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  MetricList *mlist = dbev->get_metric_list (dsptype, subtype);
  Hist_data *data = dbev->get_hist_data (mlist, Histable::FUNCTION, 0,
					 Hist_data::ALL);
  Hist_data::HistItem *totals = data->get_totals ();
  Vector<void*> *tbl = new Vector<void*>(mlist->size ());
  for (long i = 0, sz = mlist->size (); i < sz; i++)
    {
      Metric *m = mlist->get (i);
      switch (m->get_vtype ())
	{
	case VT_DOUBLE:
	  {
	    Vector<double> *lst = new Vector<double>(1);
	    lst->append (totals->value[i].d);
	    tbl->append (lst);
	    break;
	  }
	case VT_INT:
	  {
	    Vector<int> *lst = new Vector<int>(1);
	    lst->append (totals->value[i].i);
	    tbl->append (lst);
	    break;
	  }
	case VT_LLONG:
	case VT_ULLONG:
	case VT_ADDRESS:
	  {
	    Vector<long long> *lst = new Vector<long long>(1);
	    lst->append (totals->value[i].ll);
	    tbl->append (lst);
	    break;
	  }
	case VT_LABEL:
	  {
	    Vector<char *> *lst = new Vector<char *>(1);
	    Histable::NameFormat nfmt = dbev->get_name_format ();
	    lst->append (dbe_strdup (totals->obj->get_name (nfmt)));
	    tbl->append (lst);
	    break;
	  }
	default:
	  abort ();
	}
    }
  Vector<void*> *res = new Vector<void*>(2);
  res->append (dbeGetMetricList (mlist));
  res->append (tbl);
  return res;
}

Vector<void*>*
dbeGetHotMarks (int dbevindex, int type)
{
  Vector<void*>* table = new Vector<void*>(2);
  Vector<int>* table0 = new Vector<int> ();
  Vector<int>* table1 = new Vector<int> ();
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    return NULL;

  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      for (int i = 0; i < dbev->marks2dsrc->size (); i++)
	{
	  table0->append (dbev->marks2dsrc->fetch (i).index1);
	  table1->append (dbev->marks2dsrc->fetch (i).index2);
	}
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      for (int i = 0; i < dbev->marks2ddis->size (); i++)
	{
	  table0->append (dbev->marks2ddis->fetch (i).index1);
	  table1->append (dbev->marks2ddis->fetch (i).index2);
	}
      break;
    default:
      break;
    }
  table->store (0, table0);
  table->store (1, table1);
  return table;
}

Vector<void*>*
dbeGetHotMarksInc (int dbevindex, int type)
{
  Vector<void*>* table = new Vector<void*>(2);
  Vector<int>* table0 = new Vector<int> ();
  Vector<int>* table1 = new Vector<int> ();
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    return NULL;

  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      for (int i = 0; i < dbev->marks2dsrc_inc->size (); i++)
	{
	  table0->append (dbev->marks2dsrc_inc->fetch (i).index1);
	  table1->append (dbev->marks2dsrc_inc->fetch (i).index2);
	}
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      for (int i = 0; i < dbev->marks2ddis_inc->size (); i++)
	{
	  table0->append (dbev->marks2ddis_inc->fetch (i).index1);
	  table1->append (dbev->marks2ddis_inc->fetch (i).index2);
	}
      break;
    default:
      break;
    }
  table->store (0, table0);
  table->store (1, table1);
  return table;
}

Vector<void*>*
dbeGetSummaryHotMarks (int dbevindex, Vector<Obj> *sel_objs, int type)
{
  Vector<void*>* table = new Vector<void*>(2);
  Vector<int>* table0 = new Vector<int> ();
  Vector<int>* table1 = new Vector<int> ();
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    return NULL;
  if (sel_objs == NULL || sel_objs->size () == 0)
    return NULL;

  Hist_data *data;
  Vector<int_pair_t> *marks2d;
  Vector<int_pair_t>* marks2d_inc;
  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = dbev->src_data;
      marks2d = dbev->marks2dsrc;
      marks2d_inc = dbev->marks2dsrc_inc;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dbev->dis_data;
      marks2d = dbev->marks2ddis;
      marks2d_inc = dbev->marks2ddis_inc;
      break;
    default:
      data = NULL;
      marks2d = NULL;
      marks2d_inc = NULL;
      break;
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS
      || marks2d_inc == NULL || marks2d == NULL)
    return NULL;

  MetricList *orig_mlist = data->get_metric_list ();
  MetricList *prop_mlist = new MetricList (dbev->get_metric_ref (MET_NORMAL));
  if (prop_mlist && dbev->comparingExperiments ())
    prop_mlist = dbev->get_compare_mlist (prop_mlist, 0);
  Metric *mitem;
  int index, index2;
  index2 = 0;
  Vec_loop (Metric*, prop_mlist->get_items (), index, mitem)
  {
    if (mitem->get_subtype () == Metric::STATIC)
      continue;

    for (int i = 0; i < marks2d_inc->size (); i++)
      {
	int found = 0;
	for (int j = 0; j < sel_objs->size (); j++)
	  {
	    int sel_index = (int) sel_objs->fetch (j);
	    int marked_index = marks2d_inc->fetch (i).index1;
	    if (sel_index == marked_index)
	      {
		found = 1;
		break;
	      }
	  }
	if (!found)
	  continue;
	int mindex = marks2d_inc->fetch (i).index2;
	Metric *orig_metric = orig_mlist->get_items ()->fetch (mindex);
	if (orig_metric->get_id () == mitem->get_id ()
	    && mitem->get_subtype () == Metric::INCLUSIVE)
	  {
	    table0->append (index2);
	    table1->append (1);
	  }
      }

    for (int i = 0; i < marks2d->size (); i++)
      {
	int found = 0;
	for (int j = 0; j < sel_objs->size (); j++)
	  {
	    int sel_index = (int) sel_objs->fetch (j);
	    int marked_index = marks2d->fetch (i).index1;
	    if (sel_index == marked_index)
	      {
		found = 1;
		break;
	      }
	  }
	if (!found)
	  continue;
	int mindex = marks2d->fetch (i).index2;
	Metric *orig_metric = orig_mlist->get_items ()->fetch (mindex);
	if (orig_metric->get_id () == mitem->get_id ()
	    && mitem->get_subtype () == Metric::EXCLUSIVE)
	  {
	    table0->append (index2);
	    table1->append (0);
	  }
      }
    if (!(mitem->get_subtype () == Metric::EXCLUSIVE
	  || mitem->get_subtype () == Metric::DATASPACE))
      index2++;
  }
  table->store (0, table0);
  table->store (1, table1);
  return table;
}

// Get a vector of function ids of data(begin, begin + length - 1)
// Currently only support source/disassembly view
Vector<uint64_t>*
dbeGetFuncId (int dbevindex, int type, int begin, int length)
{
  Vector<uint64_t>* table = new Vector<uint64_t > ();
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();

  Hist_data *data;
  Function* given_func = NULL;
  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dbev->dis_data;
      break;
    default:
      data = NULL;
      abort ();
    }

  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;

  if (begin < 0 || begin + length > data->size ())
    return NULL;

  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
    case DSP_DISASM:
    case DSP_DISASM_V2:
      {
	for (int i = begin; i < begin + length; i++)
	  {
	    given_func = NULL;
	    Histable * sel_obj = data->fetch (i)->obj;
	    if (sel_obj != NULL)
	      given_func = (Function*) (sel_obj)->convertto (Histable::FUNCTION, (Histable*) dbev);
	    if (given_func == NULL)
	      table->append (0);
	    else
	      table->append (given_func->id);
	  }
      }
      break;
    default:
      abort ();
    }
  return table;
}

Vector<void*>*
dbeGetFuncCallerInfo (int dbevindex, int type, Vector<int>* idxs, int groupId)
{
  Vector<void*>* data = new Vector<void*>();
  if (type == DSP_SOURCE_V2 || type == DSP_DISASM_V2)
    {
      Obj sel_func = dbeGetSelObj (dbevindex, DSP_FUNCTION, 0);
      if (sel_func == 0)
	return data;
      Vector<Obj> * cmpObjs = dbeGetComparableObjsV2 (dbevindex, sel_func, type);
      if (cmpObjs == NULL)
	return data;
      DbeView *dbev = dbeSession->getView (dbevindex);
      int mtype = MET_COMMON | COMPARE_BIT | ((groupId + 1) << GROUP_ID_SHIFT);
      MetricList *mlist = dbev->get_metric_list ((MetricType) (mtype & MTYPE_MASK),
						 (mtype & COMPARE_BIT) != 0,
						 mtype >> GROUP_ID_SHIFT);
      Histable *selObj = (Histable *) cmpObjs->fetch (groupId);
      int subtype = 0;
      Hist_data *hist_data = dbev->get_data (mlist, selObj, type, subtype);
      if (hist_data == NULL)
	return data;
    }
  for (int i = 0; i < idxs->size (); i++)
    data->append (dbeGetFuncCallerInfoById (dbevindex, type, idxs->fetch (i)));
  return data;
}

//
// Get Table of Caller info:
// param: idx -- selected AT_FUNC row
// return: callsite_id, callsite_name (function: file: line)
Vector<void*>*
dbeGetFuncCallerInfoById (int dbevindex, int type, int idx)
{
  Vector<void*>* table = new Vector<void*>(3);
  Vector<uint64_t>* table0 = new Vector<uint64_t> ();
  Vector<int>* table1 = new Vector<int> ();
  Vector<char*>* table2 = new Vector<char*>();

  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Hist_data *data;
  Function* given_func = NULL;
  Vector<Histable*> *instr_info = NULL;
  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dbev->dis_data;
      break;
    default:
      data = NULL;
      abort ();
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;

  if (idx < 0 || idx >= data->size ())
    return NULL;
  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
    case DSP_DISASM:
    case DSP_DISASM_V2:
      {
	Histable * sel_obj = data->fetch (idx)->obj;
	if (sel_obj == NULL)
	  return NULL;
	given_func = (Function*) (sel_obj)->convertto (Histable::FUNCTION, (Histable*) dbev);
	if (given_func == NULL)
	  return NULL;
	PathTree * ptree = dbev->get_path_tree ();
	if (ptree == NULL)
	  return NULL;
	instr_info = ptree->get_clr_instr (given_func);
	DefaultMap<uint64_t, int> * line_seen = new DefaultMap<uint64_t, int>();
	for (int j = 0; j < ((Vector<Histable*>*)instr_info)->size (); j++)
	  {
	    Histable *instr = ((Vector<Histable*>*)instr_info)->fetch (j);
	    Function *cur_func = NULL;
	    if (instr->get_type () == Histable::INSTR)
	      cur_func = ((DbeInstr*) instr)->func;
	    else if (instr->get_type () == Histable::LINE)
	      cur_func = ((DbeLine*) instr)->func;
	    if (cur_func == NULL || (cur_func->flags & FUNC_FLAG_SIMULATED))
		continue; // skip functions like <Total>
	    Histable* line;
	    switch (type)
	      {
	      case DSP_SOURCE:
	      case DSP_SOURCE_V2:
		if (cur_func != NULL)
		  {
		    SourceFile *sourceFile = cur_func->getDefSrc ();
		    if (sourceFile == NULL ||
			(sourceFile->flags & SOURCE_FLAG_UNKNOWN) != 0)
		      continue; // skip functions like <Function: %s, instructions without line numbers>
		  }
		line = instr->convertto (Histable::LINE, NULL);
		break;
	      case DSP_DISASM:
	      case DSP_DISASM_V2:
		line = instr->convertto (Histable::INSTR, NULL);
		break;
	      default:
		abort ();
	      }
	    uint64_t func_id = cur_func->id;
	    uint64_t line_id = instr->id;
	    int is_null = 0;
	    int line_no = -1;
	    switch (type)
	      {
	      case DSP_SOURCE:
	      case DSP_SOURCE_V2:
		is_null = (((DbeLine*) line)->func == NULL) ? 1 : 0;
		if (is_null)
		  ((DbeLine*) line)->func = cur_func;
		line_no = ((DbeLine*) line)->lineno;
		if (line_seen->get (line_id) == 0)
		  {
		    line_seen->put (line_id, 1);
		    table0->append (func_id);
		    table1->append (line_no);
		    Histable::NameFormat nfmt = dbev->get_name_format ();
		    table2->append (dbe_strdup (line->get_name (nfmt)));
		  }
		if (is_null)
		  ((DbeLine*) line)->func = NULL;
		break;
	      case DSP_DISASM:
	      case DSP_DISASM_V2:
		is_null = (((DbeInstr*) line)->func == NULL) ? 1 : 0;
		if (is_null)
		  ((DbeInstr*) line)->func = cur_func;
		line_no = ((DbeInstr*) line)->addr;
		if (line_seen->get (line_id) == 0)
		  {
		    line_seen->put (line_id, 1);
		    table0->append (func_id);
		    table1->append (line_no);
		    Histable::NameFormat nfmt = dbev->get_name_format ();
		    table2->append (dbe_strdup (line->get_name (nfmt)));
		  }
		if (is_null)
		  ((DbeInstr*) line)->func = NULL;
		break;
	      default:
		abort ();
	      }
	 }
	delete line_seen;
	delete instr_info;
      }
      break;
    default:
      abort ();
    }
  table->store (0, table0);
  table->store (1, table1);
  table->store (2, table2);
  return table;
}

Vector<void*>*
dbeGetFuncCalleeInfo (int dbevindex, int type, Vector<int>* idxs, int groupId)
{
  Vector<void*>* data = new Vector<void*>();
  if (type == DSP_SOURCE_V2 || type == DSP_DISASM_V2)
    {
      Obj sel_func = dbeGetSelObj (dbevindex, DSP_FUNCTION, 0);
      if (sel_func == 0)
	return data;
      Vector<Obj> * cmpObjs = dbeGetComparableObjsV2 (dbevindex, sel_func, type);
      if (cmpObjs == NULL)
	return data;
      DbeView *dbev = dbeSession->getView (dbevindex);
      int mtype = MET_COMMON | COMPARE_BIT | ((groupId + 1) << GROUP_ID_SHIFT);
      MetricList *mlist = dbev->get_metric_list ((MetricType) (mtype & MTYPE_MASK),
						 (mtype & COMPARE_BIT) != 0,
						 mtype >> GROUP_ID_SHIFT);
      Histable *selObj = (Histable *) cmpObjs->fetch (groupId);
      int subtype = 0;
      Hist_data *hist_data = dbev->get_data (mlist, selObj, type, subtype);
      if (hist_data == NULL)
	return data;
    }

  for (int i = 0; i < idxs->size (); i++)
    data->append (dbeGetFuncCalleeInfoById (dbevindex, type, idxs->fetch (i)));
  return data;
}

//
// Get Table of Callee info:
// param: idx -- selected AT_FUNC row
// return: callsite_row, callee_id, callee_name
//
Vector<void*>*
dbeGetFuncCalleeInfoById (int dbevindex, int type, int idx)
{
  Vector<void*>* table = new Vector<void*>(3);
  Vector<int>* table0 = new Vector<int>();
  Vector<uint64_t>* table1 = new Vector<uint64_t > ();
  Vector<char*>* table2 = new Vector<char*>();
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Hist_data *data;
  Function* given_func = NULL;
  Vector<Histable*> *instr_info = NULL;
  Vector<void*> *func_info = NULL;

  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dbev->dis_data;
      break;
    default:
      data = NULL;
      abort ();
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;
  if (idx < 0 || idx >= data->size ())
    return NULL;
  switch (type)
    {
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
    case DSP_DISASM:
    case DSP_DISASM_V2:
      {
	Histable * sel_obj = data->fetch (idx)->obj;
	if (sel_obj == NULL)
	  return NULL;
	given_func = (Function*) (sel_obj)->convertto (Histable::FUNCTION, (Histable*) dbev);
	if (given_func == NULL)
	  return NULL;
	PathTree * ptree = dbev->get_path_tree ();
	if (ptree == NULL)
	  return NULL;
	Vector<Histable*> *instrs = NULL;
	Vector<void*> *callee_instrs = ptree->get_cle_instr (given_func, instrs);
	func_info = new Vector<void*>();
	instr_info = new Vector<Histable*>();
	for (long a = 0, sz_a = callee_instrs ? callee_instrs->size () : 0; a < sz_a; a++)
	  {
	    Vector<Histable*> *temp = ((Vector<Vector<Histable*>*>*)callee_instrs)->get (a);
	    DefaultMap<Function*, int> * func_seen = new DefaultMap<Function*, int>();
	    Histable* instr0 = (Histable*) instrs->fetch (a);
	    for (long b = 0, sz_b = temp ? temp->size () : 0; b < sz_b; b++)
	      {
		Histable *instr = temp->get (b);
		if (instr->get_type () == Histable::INSTR)
		  {
		    Function* func1 = ((DbeInstr *) instr)->func;
		    func_seen->put (func1, 1);
		  }
		else if (instr->get_type () == Histable::LINE)
		  {
		    Function* func1 = ((DbeLine *) instr)->func;
		    func_seen->put (func1, 1);
		  }
	      }
	    Vector<Function*> *funcs = func_seen->keySet ();
	    delete func_seen;
	    if (funcs->size () > 0)
	      {
		instr_info->append (instr0);
		func_info->append (funcs);
	      }
	  }
	delete instrs;
	destroy (callee_instrs);

	DefaultMap<uint64_t, Vector<int>* > * instr_idxs = new DefaultMap<uint64_t, Vector<int>* >();
	DefaultMap<uint64_t, int> * func_idxs = new DefaultMap<uint64_t, int>();
	for (long j = 0, sz_j = instr_info ? instr_info->size () : 0; j < sz_j; j++)
	  {
	    Histable *instr = instr_info->get (j);
	    Function *cur_func = NULL;
	    if (instr->get_type () == Histable::INSTR)
	      cur_func = ((DbeInstr*) instr)->func;
	    else if (instr->get_type () == Histable::LINE)
	      cur_func = ((DbeLine*) instr)->func;
	    if (cur_func != NULL && (cur_func->flags & FUNC_FLAG_SIMULATED))
	      continue; // skip functions like <Total>
	    Histable* line;
	    switch (type)
	      {
	      case DSP_SOURCE:
	      case DSP_SOURCE_V2:
		if (cur_func != NULL)
		  {
		    SourceFile *sourceFile = cur_func->getDefSrc ();
		    if (sourceFile == NULL ||
			(sourceFile->flags & SOURCE_FLAG_UNKNOWN) != 0)
		      // skip functions like <Function: %s, instructions without line numbers>
		      continue;
		  }
		line = instr->convertto (Histable::LINE, NULL);
		if (type == DSP_SOURCE_V2)
		  line = dbev->get_compare_obj (line);
		break;
	      case DSP_DISASM:
	      case DSP_DISASM_V2:
		line = instr;
		if (type == DSP_DISASM_V2)
		  line = dbev->get_compare_obj (line);
		break;
	      default:
		abort ();
	      }
	    if (func_idxs->get (line->id) == 0)
	      {
		func_idxs->put (line->id, 1);
		Vector<int> *temp_idx = new Vector<int>();
		temp_idx->append (j);
		instr_idxs->put (line->id, temp_idx);
	      }
	    else
	      {
		Vector<int> *temp_idx = instr_idxs->get (line->id);
		temp_idx->append (j);
	      }
	  }
	for (long i = 0; i < data->size (); i++)
	  {
	    Histable* line = data->fetch (i)->obj;
	    if (line == NULL)
	      continue;
	    Vector<int> * instr_idx = instr_idxs->get (line->id);
	    if (instr_idx == NULL)
	      continue;
	    for (long j = 0; j < instr_idx->size (); j++)
	      {
		Vector<void*>* callee_funcs_vec = (Vector<void*>*)func_info;
		if (callee_funcs_vec->size () == 0)
		  continue;
		Vector<Function*>* callee_funcs_value = (Vector<Function*>*)callee_funcs_vec->fetch (instr_idx->fetch (j));
		for (int k = 0; callee_funcs_value != NULL && k < callee_funcs_value->size (); k++)
		  {
		    uint64_t funcobj_id = ((Function*) callee_funcs_value->fetch (k))->id;
		    int old_size = table0->size ();
		    if (old_size > 0 && i == table0->fetch (old_size - 1)
			&& funcobj_id == table1->fetch (old_size - 1))
		      continue;
		    table0->append (i);
		    table1->append (funcobj_id);
		    table2->append (dbe_strdup (((Function*) callee_funcs_value->fetch (k))->get_name ()));
		  }
	      }
	  }
	delete instr_idxs;
	delete func_idxs;
	destroy (func_info);
	delete instr_info;
      }
      break;
    default:
      abort ();
    }
  table->store (0, table0);
  table->store (1, table1);
  table->store (2, table2);
  return table;
}

//
// Get Table of Function List data with only total values
//
Vector<void*> *
dbeGetFuncListMini (int dbevindex, int type, int /*subtype*/)
{
  Hist_data *data;
  DbeView *dbev = dbeSession->getView (dbevindex);
  switch (type)
    {
    case DSP_FUNCTION:
      data = dbev->func_data;
      break;
    default:
      data = NULL;
      break;
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;

  MetricList *mlist = data->get_metric_list ();

  // Get table size: count visible metrics
  int nvisible = 0;
  for (long i = 0, sz = mlist->size (); i < sz; i++)
    {
      Metric *m = mlist->get (i);
      if (m->is_visible () || m->is_tvisible () || m->is_pvisible ())
	nvisible++;
    }
  Vector<void*> *table = new Vector<void*>(nvisible + 1);

  // Fill function list elements
  Hist_data::HistItem *totals = data->get_totals ();
  for (long i = 0, sz = mlist->size (); i < sz; i++)
    {
      Metric *m = mlist->get (i);
      if (!m->is_visible () && !m->is_tvisible () && !m->is_pvisible ())
	continue;
      TValue res;
      TValue *v = data->get_value (&res, i, totals);
      if ((m->get_visbits () & VAL_RATIO) != 0)
	{
	  Vector<double> *col = new Vector<double>(1);
	  double d = (v->tag != VT_LABEL) ? v->to_double () : 100.; // NaN
	  col->append (d);
	  table->append (col);
	  continue;
	}
      switch (m->get_vtype ())
	{
	case VT_INT:
	  {
	    Vector<int> *col = new Vector<int>(1);
	    col->append (v->i);
	    table->append (col);
	    break;
	  }
	case VT_ADDRESS:
	case VT_ULLONG:
	case VT_LLONG:
	  {
	    Vector<long long> *col = new Vector<long long>(1);
	    col->append (v->ll);
	    table->append (col);
	    break;
	  }
	case VT_LABEL:
	  {
	    Vector<char *> *col = new Vector<char *>(1);
	    col->append (dbe_strdup (v->l));
	    table->append (col);
	    break;
	  }
	case VT_DOUBLE:
	default:
	  {
	    Vector<double> *col = new Vector<double>(1);
	    col->append (v->d);
	    table->append (col);
	    break;
	  }
	}
    }
  table->append (NULL);
  return table;
}

// Get Table of Function List data
Vector<void*> *
dbeGetFuncList (int dbevindex, int type, int subtype)
{
  MetricList *mlist;
  Metric *mitem;
  int nitems, nvisible;
  int index, index2, nv;
  char *cell;
  Vector<int> *ji_list;
  Hist_data *data;
  Hist_data::HistItem *item;

  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();

  // fprintf(stderr, NTXT("XXX dbeGetFuncList, FuncListDisp_type = %d\n"), type);
  switch (type)
    {
    case DSP_FUNCTION:
      data = dbev->func_data;
      break;
    case DSP_LINE:
      data = dbev->line_data;
      break;
    case DSP_PC:
      data = dbev->pc_data;
      break;
    case DSP_SOURCE:
    case DSP_SOURCE_V2:
      data = dbev->src_data;
      break;
    case DSP_DISASM:
    case DSP_DISASM_V2:
      data = dbev->dis_data;
      break;
    case DSP_SELF:
      data = dbev->fitem_data;
      break;
    case DSP_CALLER:
      data = dbev->callers;
      break;
    case DSP_CALLEE:
      data = dbev->callees;
      break;
    case DSP_DLAYOUT:
      data = dbev->dlay_data;
      break;
    case DSP_DATAOBJ:
      data = dbev->dobj_data;
      break;
    case DSP_MEMOBJ:
    case DSP_INDXOBJ:
      data = dbev->get_indxobj_data (subtype);
      break;
    default:
      data = NULL;
      break;
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;
  mlist = data->get_metric_list ();

  // Get table size: count visible metrics
  nitems = data->size ();
  nvisible = 0;
  Vec_loop (Metric*, mlist->get_items (), index, mitem)
  {
    if (mitem->is_visible () || mitem->is_tvisible () || mitem->is_pvisible ())
      nvisible++;
  }

  // Initialize Java String array
  Vector<void*> *table = new Vector<void*>(nvisible + 1);

  // Mark Hi-value metric items for annotated src/dis/layout
  if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_DLAYOUT
      || type == DSP_SOURCE_V2 || type == DSP_DISASM_V2)
    {
      ji_list = new Vector<int>(nitems);

      if (dbev->marks->size () > 0)
	index = dbev->marks->fetch (0);
      else
	index = -1;
      int mindex = 0;
      for (index2 = 0; index2 < nitems; index2++)
	{
	  item = data->fetch (index2);
	  if (index2 == index)
	    {
	      ji_list->store (index2, -item->type);
	      if (++mindex < dbev->marks->size ())
		index = dbev->marks->fetch (mindex);
	      else
		index = -1;
	    }
	  else
	    ji_list->store (index2, item->type);
	}
      table->store (nvisible, ji_list);
    }
  else
    table->store (nvisible, NULL);

  // Fill function list elements
  nv = 0;

  Vec_loop (Metric*, mlist->get_items (), index, mitem)
  {
    if (!mitem->is_visible () && !mitem->is_tvisible () &&
	!mitem->is_pvisible ())
      continue;

    // Fill values
    switch (mitem->get_vtype ())
      {
      case VT_LABEL:
	{
	  Vector<char*> *jobjects = new Vector<char*>(nitems);
	  char *buf = NULL;
	  size_t bufsz = 0;
	  int lspace = 0;
	  if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_SOURCE_V2
	      || type == DSP_DISASM_V2)
	    {
	      // if this is source or disassembly, where we'll insert
	      //	a preface into the output line, figure out how wide
	      //	it needs to be
	      // first, scan all the lines, to get the maximum line number
	      bufsz = 1024;
	      buf = (char *) malloc (bufsz);
	      int max_lineno = 0;
	      int hidx;
	      Hist_data::HistItem *hitem;
	      Vec_loop (Hist_data::HistItem*, data, hidx, hitem)
	      {
		if (!hitem->obj)
		  continue;
		if (hitem->obj->get_type () == Histable::LINE &&
		    ((DbeLine*) hitem->obj)->lineno > max_lineno)
		  max_lineno = ((DbeLine*) hitem->obj)->lineno;
		else if (hitem->obj->get_type () == Histable::INSTR
			 && ((DbeInstr*) hitem->obj)->lineno > max_lineno)
		  max_lineno = ((DbeInstr*) hitem->obj)->lineno;
	      }

	      // we have the maximum integer over all linenumbers in the file
	      // 	figure out how many digits are needed
	      lspace = snprintf (buf, bufsz, NTXT ("%d"), max_lineno);
	    }
	  for (index2 = 0; index2 < nitems; index2++)
	    {
	      item = data->fetch (index2);
	      if (type == DSP_DLAYOUT)
		cell = dbe_strdup (((DataObject*) (item->obj))->get_offset_name ());
	      else if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_SOURCE_V2 || type == DSP_DISASM_V2)
		{
		  // This code is duplicated in output.cc, yet it's
		  // intended for presentation purpose and thus is
		  // potentially different for er_print and analyzer.
		  switch (item->type)
		    {
		    case Module::AT_SRC_ONLY:
		    case Module::AT_SRC:
		      if (item->obj == NULL)
			snprintf (buf, bufsz, NTXT (" %*c. "), lspace, ' ');
		      else
			snprintf (buf, bufsz, NTXT (" %*d. "), lspace, ((DbeLine*) item->obj)->lineno);
		      break;
		    case Module::AT_FUNC:
		    case Module::AT_QUOTE:
		      snprintf (buf, bufsz, NTXT ("%*c"), lspace + 3, ' ');
		      break;
		    case Module::AT_DIS:
		    case Module::AT_DIS_ONLY:
		      if (item->obj == NULL || ((DbeInstr*) item->obj)->lineno == -1)
			snprintf (buf, bufsz, NTXT ("%*c[%*s] "), lspace + 3, ' ', lspace, NTXT ("?"));
		      else
			snprintf (buf, bufsz, NTXT ("%*c[%*d] "), lspace + 3, ' ', lspace,
				  ((DbeInstr*) item->obj)->lineno);
		      break;
		    case Module::AT_COM:
		    case Module::AT_EMPTY:
		      *buf = (char) 0;
		      break;
		    }
		  // get the line's text
		  char *s = item->value[index].l;
		  if (s != NULL)
		    {
		      // copy the string expanding all tabulations
		      // (JTable doesn't render them)
		      char *d = buf + strlen (buf);
		      char c;
		      size_t column = 0;
		      do
			{
			  c = *s++;
			  if (c == '\t')
			    {
			      do
				{
				  *d++ = ' ';
				  column++;
				}
			      while (column & 07);
			    }
			  else
			    {
			      *d++ = c;
			      column++;
			    }
			  if (column + 32 > bufsz)
			    {
			      // Reallocate the buffer
			      size_t curlen = d - buf;
			      bufsz += 1024;
			      char *buf_new = (char *) malloc (bufsz);
			      strncpy (buf_new, buf, curlen);
			      buf_new[curlen] = '\0';
			      free (buf);
			      buf = buf_new;
			      d = buf + curlen;
			    }
			}
		      while (c != (char) 0);
		    }
		  cell = dbe_strdup (buf);
		  free (item->value[index].l);
		  item->value[index].l = NULL; //YXXX missing from dbeGetFuncListV2
		}
	      else
		{
		  // omazur: why don't we have it as metric value
		  Histable::NameFormat nfmt = dbev->get_name_format ();
		  cell = dbe_strdup (item->obj->get_name (nfmt));
		}
	      jobjects->store (index2, cell);
	    }
	  if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_SOURCE_V2
	      || type == DSP_DISASM_V2)
	    free (buf);
	  table->store (nv++, jobjects);
	  break;
	}
      default:
	table->store (nv++, dbeGetTableDataOneColumn (data, index));
	break;
      }
  }
  return table;
}

Vector<Obj> *
dbeGetComparableObjsV2 (int /* dbevindex */, Obj sel_obj, int type)
{
  long grsize = dbeSession->expGroups->size ();
  Vector<Obj> *res = new Vector<Obj> (grsize + 1);
  for (long j = 0; j < grsize; j++)
    res->append ((Obj) NULL);
  res->append (sel_obj);
  Histable *obj = (Histable *) sel_obj;
  if (obj == NULL)
    return res;
  Function *func = (Function *) obj->convertto (Histable::FUNCTION);
  if (func == NULL)
    return res;
  Vector<Histable *> *cmpObjs = func->get_comparable_objs ();
  if (cmpObjs == NULL || cmpObjs->size () != grsize)
    return res;

  Histable::Type conv_type = (type == DSP_SOURCE || type == DSP_SOURCE_V2) ?
	  Histable::LINE : Histable::INSTR;
  switch (obj->get_type ())
    {
    case Histable::FUNCTION:
      for (long j = 0; j < grsize; j++)
	res->store (j, (Obj) cmpObjs->get (j));
      return res;
    case Histable::INSTR:
    case Histable::LINE:
      {
	SourceFile *srcContext = (SourceFile *) obj->convertto (Histable::SOURCEFILE);
	char *bname = get_basename (srcContext->get_name ());
	for (long j = 0; j < grsize; j++)
	  {
	    Function *func1 = (Function *) cmpObjs->get (j);
	    if (func == func1)
	      {
		if (conv_type == Histable::LINE)
		  res->store (j, (Obj) obj);
		else
		  res->store (j, (Obj) obj->convertto (conv_type, srcContext));
		continue;
	      }
	    if (func1 == NULL)
	      continue;
	    Vector<SourceFile*> *sources = func1->get_sources ();
	    SourceFile *sf = NULL;
	    for (long j1 = 0, sz1 = sources ? sources->size () : 0; j1 < sz1; j1++)
	      {
		SourceFile *sf1 = sources->get (j1);
		if (sf1 == srcContext)
		  { // the same file
		    sf = srcContext;
		    break;
		  }
		else if (sf == NULL)
		  {
		    char *bname1 = get_basename (sf1->get_name ());
		    if (dbe_strcmp (bname, bname1) == 0)
		      sf = sf1;
		  }
	      }
	    res->store (j, (Obj) func1->convertto (conv_type, srcContext));
	  }
	break;
      }
    default:
      break;
    }
  return res;
}

// Get Table of Function List data
Vector<void *> *
dbeGetFuncListV2 (int dbevindex, int mtype, Obj sel_obj, int type, int subtype)
{
  Metric *mitem;
  int nitems, nvisible;
  int index, index2, nv;
  char *cell;
  Hist_data::HistItem *item;
  DbeView *dbev = dbeSession->getView (dbevindex);
  dbev->error_msg = dbev->warning_msg = NULL;
  MetricList *mlist = dbev->get_metric_list ((MetricType) (mtype & MTYPE_MASK),
					     (mtype & COMPARE_BIT) != 0,
					     mtype >> GROUP_ID_SHIFT);
  Histable *selObj = (Histable *) sel_obj;
  int old_compare_mode = dbev->get_compare_mode ();
  if ((mtype & COMPARE_BIT) != 0)
    dbev->reset_compare_mode (CMP_DISABLE);
  Hist_data *data = dbev->get_data (mlist, selObj, type, subtype);
  dbev->reset_compare_mode (old_compare_mode);
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;
  nitems = data->size ();
  nvisible = mlist->get_items ()->size ();

  // Initialize Java String array
  Vector<void*> *table = new Vector<void*>(nvisible + 3);
  // Mark Hi-value metric items for annotated src/dis/layout
  if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_DLAYOUT
      || type == DSP_SOURCE_V2 || type == DSP_DISASM_V2)
    {
      Vector<int> *types = new Vector<int>(nitems);
      Vector<Obj> *ids = new Vector<Obj > (nitems);
      if (dbev->marks->size () > 0)
	index = dbev->marks->fetch (0);
      else
	index = -1;
      int mindex = 0;
      for (int i = 0; i < nitems; i++)
	{
	  item = data->fetch (i);
	  ids->store (i, (Obj) item->obj);
	  if (i == index)
	    {
	      types->store (i, -item->type);
	      if (++mindex < dbev->marks->size ())
		index = dbev->marks->fetch (mindex);
	      else
		index = -1;
	    }
	  else
	    types->store (i, item->type);
	}
      table->store (nvisible, types);
      table->store (nvisible + 1, ids);
    }
  else
    {
      table->store (nvisible, NULL);
      table->store (nvisible + 1, NULL);
    }

  // Fill function list elements
  nv = 0;
  Vec_loop (Metric*, mlist->get_items (), index, mitem)
  {
    if (!mitem->is_visible () && !mitem->is_tvisible () &&
	!mitem->is_pvisible ())
      continue;

    // Fill values
    switch (mitem->get_vtype ())
      {
      default:
	table->store (nv++, dbeGetTableDataOneColumn (data, index));
	break;
      case VT_LABEL:
	Vector<char*> *jobjects = new Vector<char*>(nitems);
	char *buf = NULL;
	size_t bufsz = 0;
	int lspace = 0;
	if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_SOURCE_V2
	    || type == DSP_DISASM_V2)
	  {
	    // if this is source or disassembly, where we'll insert
	    //	a preface into the output line, figure out how wide
	    //	it needs to be
	    // first, scan all the lines, to get the maximum line number
	    bufsz = 1024;
	    buf = (char *) malloc (bufsz);
	    int max_lineno = 0;
	    int hidx;
	    Hist_data::HistItem *hitem;
	    Vec_loop (Hist_data::HistItem*, data, hidx, hitem)
	    {
	      if (!hitem->obj)
		continue;
	      if (hitem->obj->get_type () == Histable::LINE &&
		  ((DbeLine*) hitem->obj)->lineno > max_lineno)
		max_lineno = ((DbeLine*) hitem->obj)->lineno;
	      else if (hitem->obj->get_type () == Histable::INSTR
		       && ((DbeInstr*) hitem->obj)->lineno > max_lineno)
		max_lineno = ((DbeInstr*) hitem->obj)->lineno;
	    }

	    // we have the maximum integer over all linenumbers in the file
	    // 	figure out how many digits are needed
	    lspace = snprintf (buf, bufsz, NTXT ("%d"), max_lineno);
	  }

	for (index2 = 0; index2 < nitems; index2++)
	  {
	    item = data->fetch (index2);
	    if (type == DSP_DLAYOUT)
	      cell = dbe_strdup (((DataObject*) (item->obj))->get_offset_name ());
	    else if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_SOURCE_V2 || type == DSP_DISASM_V2)
	      {
		// This code is duplicated in output.cc, yet it's
		// intended for presentation purpose and thus is
		// potentially different for er_print and analyzer.
		switch (item->type)
		  {
		  case Module::AT_SRC_ONLY:
		  case Module::AT_SRC:
		    if (item->obj == NULL)
		      snprintf (buf, bufsz, NTXT (" %*c. "), lspace, ' ');
		    else
		      snprintf (buf, bufsz, NTXT (" %*d. "), lspace,
				((DbeLine*) item->obj)->lineno);
		    break;
		  case Module::AT_FUNC:
		  case Module::AT_QUOTE:
		    snprintf (buf, bufsz, NTXT ("%*c"), lspace + 3, ' ');
		    break;
		  case Module::AT_DIS:
		  case Module::AT_DIS_ONLY:
		    if (item->obj == NULL || ((DbeInstr*) item->obj)->lineno == -1)
		      snprintf (buf, bufsz, NTXT ("%*c[%*s] "), lspace + 3, ' ',
				lspace, NTXT ("?"));
		    else
		      snprintf (buf, bufsz, NTXT ("%*c[%*d] "), lspace + 3, ' ',
				lspace,
				((DbeInstr*) item->obj)->lineno);
		    break;
		  case Module::AT_COM:
		  case Module::AT_EMPTY:
		    *buf = (char) 0;
		    break;
		  }
		// get the line's text
		char *s = item->value[index].l;
		if (s != NULL)
		  {
		    // copy the string expanding all tabulations
		    // (JTable doesn't render them)
		    char *d = buf + strlen (buf);
		    char c;
		    size_t column = 0;
		    do
		      {
			c = *s++;
			if (c == '\t')
			  {
			    do
			      {
				*d++ = ' ';
				column++;
			      }
			    while (column & 07);
			  }
			else
			  {
			    *d++ = c;
			    column++;
			  }
			if (column + 32 > bufsz)
			  {
			    // Reallocate the buffer
			    size_t curlen = d - buf;
			    bufsz += 1024;
			    char *buf_new = (char *) malloc (bufsz);
			    strncpy (buf_new, buf, curlen);
			    buf_new[curlen] = '\0';
			    free (buf);
			    buf = buf_new;
			    d = buf + curlen;
			  }
		      }
		    while (c != (char) 0);
		  }
		cell = dbe_strdup (buf);
	      }
	    else
	      {
		Histable::NameFormat nfmt = dbev->get_name_format ();
		cell = dbe_strdup (item->obj->get_name (nfmt));
	      }
	    jobjects->store (index2, cell);
	  }

	if (type == DSP_SOURCE || type == DSP_DISASM || type == DSP_SOURCE_V2
	    || type == DSP_DISASM_V2)
	  free (buf);
	table->store (nv++, jobjects);
	break;
      }
  }
  table->append (dbeGetMetricList (mlist));
  return table;
} // dbeGetFuncListV2

//
// Get Table DataV2
//
Vector<void*> *
dbeGetTableDataV2 (int dbevindex, char *mlistStr, char *modeStr, char *typeStr,
		   char *subtypeStr, Vector<uint64_t> *ids)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();

  // Process metric list specification
  if (mlistStr == NULL)
    return NULL;
  bool met_call = false;
  MetricList *mlist = NULL;
  if (streq (mlistStr, NTXT ("MET_NORMAL")))
    mlist = dbev->get_metric_list (MET_NORMAL);
  else if (streq (mlistStr, NTXT ("MET_CALL")))
    {
      met_call = true;
      mlist = dbev->get_metric_list (MET_CALL);
    }
  else if (streq (mlistStr, NTXT ("MET_CALL_AGR")))
    mlist = dbev->get_metric_list (MET_CALL_AGR);
  else if (streq (mlistStr, NTXT ("MET_DATA")))
    mlist = dbev->get_metric_list (MET_DATA);
  else if (streq (mlistStr, NTXT ("MET_INDX")))
    mlist = dbev->get_metric_list (MET_INDX);
  else if (streq (mlistStr, NTXT ("MET_IO")))
    mlist = dbev->get_metric_list (MET_IO);
  else if (streq (mlistStr, NTXT ("MET_HEAP")))
    mlist = dbev->get_metric_list (MET_HEAP);
  else
    return NULL;

  // Process mode specification
  if (modeStr == NULL)
    return NULL;
  Hist_data::Mode mode = (Hist_data::Mode)0;
  if (streq (modeStr, NTXT ("CALLERS")))
    mode = Hist_data::CALLERS;
  else if (streq (modeStr, NTXT ("CALLEES")))
    mode = Hist_data::CALLEES;
  else if (streq (modeStr, NTXT ("SELF")))
    mode = Hist_data::SELF;
  else if (streq (modeStr, NTXT ("ALL")))
    mode = Hist_data::ALL;
  else
    return NULL;

  // Process type specification
  if (typeStr == NULL)
    return NULL;
  Histable::Type type = Histable::OTHER;
  if (streq (typeStr, NTXT ("FUNCTION")))
    type = Histable::FUNCTION;
  else if (streq (typeStr, NTXT ("INDEXOBJ")))
    type = Histable::INDEXOBJ;
  else if (streq (typeStr, NTXT ("IOACTFILE")))
    type = Histable::IOACTFILE;
  else if (streq (typeStr, NTXT ("IOACTVFD")))
    type = Histable::IOACTVFD;
  else if (streq (typeStr, NTXT ("IOCALLSTACK")))
    type = Histable::IOCALLSTACK;
  else if (streq (typeStr, NTXT ("HEAPCALLSTACK")))
    type = Histable::HEAPCALLSTACK;
  else if (streq (typeStr, NTXT ("LINE")))
    type = Histable::LINE;
  else if (streq (typeStr, NTXT ("INSTR")))
    type = Histable::INSTR;
  else
    // XXX Accepting objects other than above may require a different
    // implementation of the id -> Histable mapping below
    return NULL;

  // Process subtype specification
  int subtype = 0;
  if (subtypeStr != NULL)
    subtype = atoi (subtypeStr);
  Vector<Histable*> *hobjs = NULL;
  if (ids != NULL)
    {
      hobjs = new Vector<Histable*>();
      for (int i = 0; i < ids->size (); ++i)
	{
	  Histable::Type obj_type = type;
	  if ((obj_type == Histable::LINE || obj_type == Histable::INSTR)
	      && subtype == 0)
	    obj_type = Histable::FUNCTION;
	  Histable *hobj = dbeSession->findObjectById (obj_type, subtype, ids->fetch (i));
	  if ((obj_type == Histable::LINE || obj_type == Histable::INSTR)
	      && subtype == 0 && hobj == NULL)
	    return NULL;
	  hobjs->append (hobj);
	}
    }

  PathTree::PtreeComputeOption flag = PathTree::COMPUTEOPT_NONE;
  if (dbev->isOmpDisMode () && type == Histable::FUNCTION
      && mode == Hist_data::CALLEES && met_call)
    flag = PathTree::COMPUTEOPT_OMP_CALLEE;

  Hist_data *data = dbev->get_hist_data (mlist, type, subtype, mode, hobjs, NULL, NULL, flag);
  return dbeGetTableDataV2Data (dbev, data);
}

static Vector<void*> *
dbeGetTableDataV2Data (DbeView * /*dbev*/, Hist_data *data)
{
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;
  MetricList *mlist;
  mlist = data->get_metric_list ();
  int nitems = data->size ();

  // Initialize Java String array
  Vector<void*> *table = new Vector<void*>(mlist->size () + 1);

  // Fill function list elements
  for (long i = 0, sz = mlist->size (); i < sz; i++)
    {
      Metric *mitem = mlist->get (i);
      if (!mitem->is_visible () && !mitem->is_tvisible () &&
	  !mitem->is_pvisible ())
	continue;
      table->append (dbeGetTableDataOneColumn (data, i));
    }

  // Add an array of Histable IDs
  Vector<uint64_t> *idList = new Vector<uint64_t>(nitems);
  for (int i = 0; i < nitems; ++i)
    {
      Hist_data::HistItem *item = data->fetch (i);
      if (item->obj->get_type () == Histable::LINE
	  || item->obj->get_type () == Histable::INSTR)
	idList->store (i, (uint64_t) (item->obj));
      else
	idList->store (i, item->obj->id);
    }
  table->append (idList);
  return table;
} // dbeGetTableData

//YXXX try to use the following to consolidate similar cut/paste code

static Vector<void*> *
dbeGetTableDataOneColumn (Hist_data *data, int met_ind)
{
  // Allocates a vector and fills it with metric values for one column
  TValue res;
  Metric *m = data->get_metric_list ()->get (met_ind);
  if ((m->get_visbits () & VAL_RATIO) != 0)
    {
      Vector<double> *col = new Vector<double>(data->size ());
      for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	{
	  TValue *v = data->get_value (&res, met_ind, row);
	  double d = (v->tag != VT_LABEL) ? v->to_double () : 100.; // NaN
	  col->append (d);
	}
      return (Vector<void*> *) col;
    }

  switch (m->get_vtype ())
    {
    case VT_DOUBLE:
      {
	Vector<double> *col = new Vector<double>(data->size ());
	for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	  {
	    TValue *v = data->get_value (&res, met_ind, row);
	    col->append (v->d);
	  }
	return (Vector<void*> *) col;
      }
    case VT_INT:
      {
	Vector<int> *col = new Vector<int>(data->size ());
	for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	  {
	    TValue *v = data->get_value (&res, met_ind, row);
	    col->append (v->i);
	  }
	return (Vector<void*> *) col;
      }
    case VT_ULLONG:
    case VT_LLONG:
      {
	Vector<long long> *col = new Vector<long long>(data->size ());
	for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	  {
	    TValue *v = data->get_value (&res, met_ind, row);
	    col->append (v->ll);
	  }
	return (Vector<void*> *) col;
      }
    case VT_ADDRESS:
      {
	Vector<long long> *col = new Vector<long long>(data->size ());
	for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	  {
	    TValue *v = data->get_value (&res, met_ind, row);
	    // set the highest bit to mark this jlong as
	    // a VT_ADDRESS (rather than a regular VT_LLONG)
	    col->append (v->ll | 0x8000000000000000ULL);
	  }
	return (Vector<void*> *) col;
      }
    case VT_LABEL:
      {
	Vector<char *> *col = new Vector<char *>(data->size ());
	for (long row = 0, sz_row = data->size (); row < sz_row; row++)
	  {
	    TValue *v = data->get_value (&res, met_ind, row);
	    col->append (dbe_strdup (v->l));
	  }
	return (Vector<void*> *) col;
      }
    default:
      return NULL;
    }
}

static Vector<void*> *
dbeGetTableDataOneColumn (DbeView *dbev, Vector<Hist_data::HistItem*> *data,
			  ValueTag vtype, int metricColumnNumber)
// Allocates a vector and fills it with metric values for one column
{
  Vector<void*> *column_data = NULL;
  int nitems = data->size (); // number of rows
  int index = metricColumnNumber;
  switch (vtype)
    {
    case VT_DOUBLE:
      {
	Vector<double> *jd_list = new Vector<double>(nitems);
	for (int index2 = 0; index2 < nitems; index2++)
	  {
	    Hist_data::HistItem *item = data->fetch (index2);
	    jd_list->store (index2, item->value[index].d);
	  }
	column_data = (Vector<void*> *)jd_list;
	break;
      }
    case VT_INT:
      {
	Vector<int> *ji_list = new Vector<int>(nitems);
	for (int index2 = 0; index2 < nitems; index2++)
	  {
	    Hist_data::HistItem *item = data->fetch (index2);
	    ji_list->store (index2, item->value[index].i);
	  }
	column_data = (Vector<void*> *)ji_list;
	break;
      }
    case VT_ULLONG:
    case VT_LLONG:
      {
	Vector<long long> *jl_list = new Vector<long long>(nitems);
	for (int index2 = 0; index2 < nitems; index2++)
	  {
	    Hist_data::HistItem *item = data->fetch (index2);
	    jl_list->store (index2, item->value[index].ll);
	  }
	column_data = (Vector<void*> *)jl_list;
	break;
      }
    case VT_ADDRESS:
      {
	Vector<long long> *jl_list = new Vector<long long>(nitems);
	for (int index2 = 0; index2 < nitems; index2++)
	  {
	    Hist_data::HistItem *item = data->fetch (index2);

	    // set the highest bit to mark this jlong as
	    // a VT_ADDRESS (rather than a regular VT_LLONG)
	    uint64_t addr = item->value[index].ll;
	    addr |= 0x8000000000000000ULL;
	    jl_list->store (index2, addr);
	  }
	column_data = (Vector<void*> *)jl_list;
	break;
      }
    case VT_LABEL:
      {
	Vector<char*> *jobjects = new Vector<char*>(nitems);
	for (int index2 = 0; index2 < nitems; index2++)
	  {
	    Hist_data::HistItem *item = data->fetch (index2);

	    // omazur: why don't we have it as metric value
	    Histable::NameFormat nfmt = dbev->get_name_format ();
	    char *str = dbe_strdup (item->obj->get_name (nfmt));
	    jobjects->store (index2, str);
	  }
	column_data = (Vector<void*> *)jobjects;
	break;
      }
    default:
      abort ();
    }
  return column_data;
}

int
dbeGetCallTreeNumLevels (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  PathTree * ptree = dbev->get_path_tree ();
  if (ptree == NULL)
    return 0;
  return ptree->get_ftree_depth ();
}

Vector<void*>*
dbeGetCallTreeLevel (int dbevindex, char *mcmd, int level)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  PathTree * ptree = dbev->get_path_tree ();
  if (ptree == NULL)
    return NULL;
  if (mcmd == NULL)
    return NULL;
  BaseMetric *bm = dbeSession->find_base_reg_metric (mcmd);
  if (bm == NULL)
    return NULL;
  return ptree->get_ftree_level (bm, level);
}

Vector<void*>*
dbeGetCallTreeLevels (int dbevindex, char *mcmd)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  PathTree * ptree = dbev->get_path_tree ();
  if (ptree == NULL)
    return NULL;
  if (mcmd == NULL)
    return NULL;
  BaseMetric *bm = dbeSession->find_base_reg_metric (mcmd);
  if (bm == NULL)
    return NULL;

  int depth = ptree->get_ftree_depth ();
  Vector<void*> *results = new Vector<void*>(depth);
  for (int ii = 0; ii < depth; ii++)
    results->append (ptree->get_ftree_level (bm, ii));
  return results;
}

Vector<void*>*
dbeGetCallTreeLevelFuncs (int dbevindex, int start_level, int end_level)
{ // (0,-1) -> all levels
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  PathTree * ptree = dbev->get_path_tree ();
  if (ptree == NULL)
    return NULL;

  int depth = ptree->get_ftree_depth ();
  if (start_level < 0)
    start_level = 0;
  if (end_level < 0 || end_level >= depth)
    end_level = depth - 1;

  Histable::NameFormat nfmt = dbev->get_name_format (); //YXXX or fixed format?
  Vector<char*> *funcNames = new Vector<char*>();
  Vector<long long> *funcIds = new Vector<long long>();
  Vector<Obj> *funcObjs = new Vector<Obj>();

  if (start_level == 0 && end_level == depth - 1)
    return dbeGetCallTreeFuncs (dbevindex);
  else
    {
      for (int ii = start_level; ii <= end_level; ii++)
	{
	  Vector<void*> *info = ptree->get_ftree_level (NULL, ii); /*no metric*/
	  if (!info)
	    continue;
	  Vector<long long> *fids = (Vector<long long> *)info->get (2);
	  if (!fids)
	    continue;
	  int index;
	  long long fid;
	  Vec_loop (long long, fids, index, fid)
	  {
	    funcIds->append (fid);
	    Histable *obj = dbeSession->findObjectById (fid);
	    char * fname = obj ? dbe_strdup (obj->get_name (nfmt)) : NULL;
	    funcNames->append (fname);
	    funcObjs->append ((unsigned long) obj); // avoiding sign extension
	  }
	  destroy (info);
	}
    }
  Vector<void*> *results = new Vector<void*>(3);
  results->append (funcIds);
  results->append (funcNames);
  results->append (funcObjs);
  return results;
}

Vector<void*> *
dbeGetCallTreeFuncs (int dbevindex)
{ // does not require ptree->get_ftree_level() to be computed
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  PathTree * ptree = dbev->get_path_tree ();
  if (ptree == NULL)
    return 0;
  Vector<Function*>* funcs = ptree->get_funcs (); // Unique functions in tree
  if (funcs == NULL)
    return NULL;

  long sz = funcs->size ();
  Vector<void*> *results = new Vector<void*>(3);
  Vector<long long> *funcIds = new Vector<long long>(sz);
  Vector<char*> *funcNames = new Vector<char*>(sz);
  Vector<Obj> *funcObjs = new Vector<Obj>(sz);

  int index;
  Function * func;
  Histable::NameFormat nfmt = dbev->get_name_format (); //YXXX or fixed format?
  Vec_loop (Function *, funcs, index, func)
  {
    funcIds->append (func->id); // do we need IDs?
    char *fname = dbe_strdup (func->get_name (nfmt));
    funcNames->append (fname);
    funcObjs->append ((unsigned long) func); // avoiding sign extension
  }
  results->put (0, funcIds);
  results->put (1, funcNames);
  results->put (2, funcObjs);
  destroy (funcs);
  return results;
}

Vector<void*>*
dbeGetCallTreeChildren (int dbevindex, char *mcmd, Vector<int /*NodeIdx*/>*node_idxs)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (node_idxs == NULL || node_idxs->size () == 0)
    return NULL;
  long sz = node_idxs->size ();
  PathTree * ptree = dbev->get_path_tree ();
  if (ptree == NULL)
    return NULL;
  if (mcmd == NULL)
    return NULL;
  BaseMetric *bm = dbeSession->find_base_reg_metric (mcmd);
  if (bm == NULL)
    return NULL;

  Vector<void*> *results = new Vector<void*>(sz);
  for (long ii = 0; ii < sz; ii++)
    {
      PathTree::NodeIdx nodeIdx = node_idxs->get (ii); // upcasted from int
      results->append (ptree->get_ftree_node_children (bm, nodeIdx));
    }
  return results;
}

Vector<int> *
dbeGetGroupIds (int /*dbevindex*/)
{
  Vector<ExpGroup*> *groups = dbeSession->expGroups;
  int sz = groups->size ();
  Vector<int> *grIds = new Vector<int>(sz);
  for (int i = 0; i < sz; i++)
    grIds->store (i, groups->fetch (i)->groupId);
  return grIds;
}

//
// Get label for name column
//
Vector<char*> *
dbeGetNames (int dbevindex, int type, Obj sel_obj)
{
  char *s0, *s1, *s2;
  bool need_strdup = true;
  switch (type)
    {
    case DSP_SOURCE_V2:
    case DSP_DISASM_V2:
    case DSP_SOURCE:
    case DSP_DISASM:
      {
	if (sel_obj)
	  {
	    Histable *selObj = (Histable*) sel_obj;
	    Function *func = (Function *) selObj->convertto (Histable::FUNCTION);
	    if (func)
	      {
		char *names[3] = {NULL, NULL, NULL};
		set_file_names (func, names);
		s0 = names[0];
		s1 = names[1];
		s2 = names[2];
		need_strdup = false;
		break;
	      }
	  }
	DbeView *dbev = dbeSession->getView (dbevindex);
	char **names = type == DSP_SOURCE || type == DSP_SOURCE_V2 ? dbev->names_src : dbev->names_dis;
	s0 = names[0];
	s1 = names[1];
	s2 = names[2];
	break;
      }
    case DSP_LINE:
      s0 = GTXT ("Lines");
      s1 = GTXT ("Function, line # in \"sourcefile\"");
      s2 = NTXT ("");
      break;
    case DSP_PC:
      s0 = GTXT ("PCs");
      s1 = GTXT ("Function + offset");
      s2 = NTXT ("");
      break;
    case DSP_DLAYOUT:
      s0 = GTXT ("Name");
      s1 = GTXT ("* +offset .element");
      s2 = NTXT ("");
      break;
    default:
      s0 = GTXT ("Name");
      s1 = s2 = NTXT ("");
      break;
    }
  if (need_strdup)
    {
      s0 = dbe_strdup (s0);
      s1 = dbe_strdup (s1);
      s2 = dbe_strdup (s2);
    }
  Vector<char*> *table = new Vector<char*>(3);
  table->store (0, s0);
  table->store (1, s1);
  table->store (2, s2);
  return table;
}

//
// Get Total/Maximum element of Function List
//
Vector<void*> *
dbeGetTotalMax (int dbevindex, int type, int subtype)
{
  Hist_data *data;
  int index;
  Hist_data::HistItem *total_item, *maximum_item;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();

  switch (type)
    {
    case DSP_LINE:
      data = dbev->line_data;
      break;
    case DSP_PC:
      data = dbev->pc_data;
      break;
    case DSP_CALLER:
      data = dbev->callers;
      break;
    case DSP_SELF:
    case DSP_CALLEE:
      data = dbev->callees;
      break;
    case DSP_DLAYOUT:
      data = dbev->dlay_data;
      break;
    case DSP_DATAOBJ:
      data = dbev->dobj_data;
      break;
    case DSP_MEMOBJ:
      data = dbev->get_indxobj_data (subtype);
      break;
    case DSP_INDXOBJ:
      data = dbev->get_indxobj_data (subtype);
      break;
    case DSP_FUNCTION: // annotated src/dis use func total/max
    case DSP_SOURCE:
    case DSP_DISASM:
    case DSP_SOURCE_V2:
    case DSP_DISASM_V2:
      data = dbev->func_data;
      break;
    default:
      abort ();
    }
  if (data == NULL || data->get_status () != Hist_data::SUCCESS)
    return NULL;

  // Get list size
  // XXX -- the original list has all items, visible or not;
  // XXX -- the one from Hist_data has only visible items,
  // XXX --    and should be the only ones computed
  // XXX --    Analyzer got confused (yesterday), when we used the shorter list
  // XXX -- Why can we fetch total/max for metrics never
  // XXX --    computed without core dumping?
  MetricList *mlist2 = data->get_metric_list ();
  int size = mlist2->get_items ()->size ();

  // Initialize Java array
  Vector<void*> *total_max = new Vector<void*>(2);
  Vector<double> *total = new Vector<double>(size);
  Vector<double> *maximum = new Vector<double>(size);

  // Fill total/maximum element
  total_item = data->get_totals ();
  maximum_item = data->get_maximums ();

  for (index = 0; index < size; index++)
    {
      total->store (index, total_item->value[index].to_double ());
      maximum->store (index, maximum_item->value[index].to_double ());
    }
  total_max->store (0, total);
  total_max->store (1, maximum);
  return total_max;
}

//
// Get Table of Overview List
Vector<void*> *
dbeGetStatisOverviewList (int dbevindex)
{
  int size;
  Ovw_data **data;
  Ovw_data::Ovw_item labels, *totals;
  int nitems;
  int index, index2;

  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->error_msg = dbev->warning_msg = NULL;

  size = dbeSession->nexps ();
  totals = new Ovw_data::Ovw_item[size + 1];
  data = new Ovw_data*[size + 1];
  data[0] = new Ovw_data ();

  for (index = 1; index <= size; index++)
    {
      data[index] = dbev->get_ovw_data (index - 1);
      if (data[index] == NULL)
	{
	  Ovw_data::reset_item (&totals[index]); // set contents to zeros
	  continue;
	}
      data[0]->sum (data[index]);
      totals[index] = data[index]->get_totals (); //shallow copy!
    }
  totals[0] = data[0]->get_totals ();

  // Get table size
  labels = data[0]->get_labels ();
  nitems = labels.size + 4;

  // Initialize Java String array
  Vector<void*> *table = new Vector<void*>(size + 4);
  Vector<char*> *jobjects = new Vector<char*>(nitems);

  // Set the label
  jobjects->store (0, dbe_strdup (GTXT ("Start Time (sec.)")));
  jobjects->store (1, dbe_strdup (GTXT ("End Time (sec.)")));
  jobjects->store (2, dbe_strdup (GTXT ("Duration (sec.)")));
  jobjects->store (3, dbe_strdup (GTXT ("Total Thread Time (sec.)")));
  jobjects->store (4, dbe_strdup (GTXT ("Average number of Threads")));

  for (index2 = 5; index2 < nitems; index2++)
    jobjects->store (index2, dbe_strdup (labels.values[index2 - 4].l));
  table->store (0, jobjects);

  // Set the data
  for (index = 0; index <= size; index++)
    {
      Vector<double> *jd_list = new Vector<double>(nitems);
      jd_list->store (0, tstodouble (totals[index].start));
      jd_list->store (1, tstodouble (totals[index].end));
      jd_list->store (2, tstodouble (totals[index].duration));
      jd_list->store (3, tstodouble (totals[index].tlwp));
      jd_list->store (4, totals[index].nlwp);
      for (index2 = 5; index2 < nitems; index2++)
	jd_list->store (index2, tstodouble (totals[index].values[index2 - 4].t));
      table->store (index + 1, jd_list);
    }
  for (index = 0; index <= size; index++)
    delete data[index];
  delete[] data;
  delete[] totals;
  return table;
}

// Get Table of Statistics List
Vector<void*> *
dbeGetStatisList (int dbevindex)
{
  int size;
  Stats_data **data;
  int nitems;
  int index, index2;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  dbev->error_msg = dbev->warning_msg = NULL;
  if ((size = dbeSession->nexps ()) == 0)
    return NULL;

  // Get statistics data
  data = (Stats_data **) malloc ((size + 1) * sizeof (Stats_data *));
  data[0] = new Stats_data ();
  for (index = 1; index <= size; index++)
    {
      data[index] = dbev->get_stats_data (index - 1);
      if (data[index] == NULL)
	continue;
      data[0]->sum (data[index]);
    }

  // Get table size
  nitems = data[0]->size ();

  // Initialize Java String array
  Vector<void*> *table = new Vector<void*>(size + 2);
  Vector<char*> *jobjects = new Vector<char*>(nitems);

  // Set the label
  for (index2 = 0; index2 < nitems; index2++)
    jobjects->store (index2, dbe_strdup (data[0]->fetch (index2).label));
  table->store (0, jobjects);

  // Set the data
  for (index = 0; index <= size; index++)
    {
      Vector<double> *jd_list = new Vector<double>(nitems);
      for (index2 = 0; index2 < nitems; index2++)
	{
	  double val = 0;
	  if (data[index])
	    val = data[index]->fetch (index2).value.to_double ();
	  jd_list->store (index2, val);
	}
      table->store (index + 1, jd_list);
    }
  if (data)
    {
      for (index = 0; index <= size; index++)
	delete data[index];
      free (data);
    }
  return table;
}


//
// Set summary list
//
static void
setSummary (Vector<Histable*> *objs, Vector<int> *saligns,
	    Vector<char> *mnemonic, Vector<char*> *jlabels, Vector<char*> *jvalues)
{
  char *sname = NULL, *oname = NULL, *lname = NULL, *alias = NULL,
	  *mangle = NULL, *address = NULL, *size = NULL,
	  *name_0 = NULL, *sname_0 = NULL, *oname_0 = NULL, *lname_0 = NULL,
	  *alias_0 = NULL, *mangle_0 = NULL;
  Function *func, *last_func = NULL;
  int one_func = 1;

  // Get the source/object/load-object files & aliases
  long long ll_size = 0;
  for (long i = 0; i < objs->size (); i++)
    {
      Histable *current_obj = objs->fetch (i);
      Histable::Type htype = current_obj->get_type ();
      if (htype == Histable::LOADOBJECT)
	lname = ((LoadObject *) current_obj)->dbeFile->get_location_info ();
      else if ((func = (Function*) current_obj->convertto (Histable::FUNCTION)) != NULL)
	{
	  if (one_func && last_func != NULL && last_func != func)
	    one_func = 0;
	  last_func = func;
	  sname = NULL;
	  DbeLine *dbeline = (DbeLine*) current_obj->convertto (Histable::LINE);
	  if (dbeline)
	    {
	      SourceFile *sf;
	      if (dbeline->lineno == 0 && dbeline->include != NULL)
		sf = dbeline->include;
	      else if (dbeline->sourceFile != NULL)
		sf = dbeline->sourceFile;
	      else
		sf = func->getDefSrc ();
	      if (sf)
		sname = sf->dbeFile->get_location_info ();
	    }
	  char *func_name = func->get_name ();
	  mangle = func->get_mangled_name ();
	  if (mangle && streq (func_name, mangle))
	    mangle = NULL;
	  Module *module = func->module;
	  if (module != NULL)
	    {
	      module->read_stabs ();
	      if (sname == NULL || strlen (sname) == 0)
		{
		  SourceFile *sf = module->getMainSrc ();
		  sname = sf->dbeFile->get_location_info ();
		}
	      DbeFile *df = module->dbeFile;
	      if (df == NULL || (df->filetype & DbeFile::F_JAVACLASS) == 0)
		df = module->loadobject->dbeFile;
	      lname = df->get_location_info ();
	      oname = lname;
	      if (module->dot_o_file)
		oname = module->dot_o_file->dbeFile->get_location_info ();
	    }

	  if (htype == Histable::INSTR && dbeSession->is_datamode_available ())
	    alias = ((DbeInstr*) current_obj)->get_descriptor ();
	}

      char *name = current_obj->get_name ();
      if (i == 0)
	{
	  name_0 = name;
	  lname_0 = lname;
	  sname_0 = sname;
	  oname_0 = oname;
	  mangle_0 = mangle;
	  alias_0 = alias;
	  if (objs->size () == 1)
	    {
	      uint64_t addr = current_obj->get_addr ();
	      address = dbe_sprintf (NTXT ("%lld:0x%08llX"),
				     (long long) ADDRESS_SEG (addr),
				     (long long) ADDRESS_OFF (addr));
	    }
	}
      else
	{
	  if (name_0 != name)
	    name_0 = NULL;
	  if (lname_0 != lname)
	    lname_0 = NULL;
	  if (sname_0 != sname)
	    sname_0 = NULL;
	  if (oname_0 != oname)
	    oname_0 = NULL;
	  if (mangle_0 != mangle)
	    mangle_0 = NULL;
	  if (alias_0 != alias)
	    alias_0 = NULL;
	}
      if (current_obj->get_size () == -1)
	{
	  if (size == NULL)
	    size = dbe_strdup (GTXT ("(Unknown)"));
	}
      else
	ll_size += current_obj->get_size ();
    }
  if (size == NULL)
    size = dbe_sprintf (NTXT ("%lld"), ll_size);
  if (name_0 == NULL)
    {
      if (objs->size () > 1)
	{
	  char *func_name = last_func == NULL ? NULL :
		  (one_func == 0 ? NULL : last_func->get_name ());
	  name_0 = dbe_sprintf (NTXT ("%s%s%s (%lld %s)"),
				func_name == NULL ? "" : func_name,
				func_name == NULL ? "" : ": ",
				GTXT ("Multiple Selection"),
				(long long) objs->size (),
				GTXT ("objects"));
	}
    }
  else
    name_0 = dbe_strdup (name_0);

  // Set the name area
  saligns->store (0, TEXT_LEFT);
  mnemonic->store (0, 'N');
  jlabels->store (0, dbe_strdup (GTXT ("Name")));
  jvalues->store (0, name_0);

  saligns->store (1, TEXT_LEFT);
  mnemonic->store (1, 'P');
  jlabels->store (1, dbe_strdup (GTXT ("PC Address")));
  jvalues->store (1, address);

  saligns->store (2, TEXT_LEFT);
  mnemonic->store (2, 'z');
  jlabels->store (2, dbe_strdup (GTXT ("Size")));
  jvalues->store (2, size);

  saligns->store (3, TEXT_RIGHT);
  mnemonic->store (3, 'r');
  jlabels->store (3, dbe_strdup (GTXT ("Source File")));
  jvalues->store (3, dbe_strdup (sname_0));

  saligns->store (4, TEXT_RIGHT);
  mnemonic->store (4, 'b');
  jlabels->store (4, dbe_strdup (GTXT ("Object File")));
  jvalues->store (4, dbe_strdup (oname_0));

  saligns->store (5, TEXT_LEFT);
  mnemonic->store (5, 'j');
  jlabels->store (5, dbe_strdup (GTXT ("Load Object")));
  jvalues->store (5, dbe_strdup (lname_0));

  saligns->store (6, TEXT_LEFT);
  mnemonic->store (6, 'm');
  jlabels->store (6, dbe_strdup (GTXT ("Mangled Name")));
  jvalues->store (6, dbe_strdup (mangle_0));

  saligns->store (7, TEXT_LEFT);
  mnemonic->store (7, 'A');
  jlabels->store (7, dbe_strdup (GTXT ("Aliases")));
  jvalues->store (7, dbe_strdup (alias_0));
}

// Set memory-object summary list
//
static void
setMemSummary (Vector<Histable*> *objs, Vector<int> *saligns,
	       Vector<char> *mnemonic, Vector<char*> *jlabels,
	       Vector<char*> *jvalues)
{
  saligns->store (0, TEXT_LEFT);
  mnemonic->store (0, 'M');
  jlabels->store (0, dbe_strdup (GTXT ("Memory Object")));
  if (objs->size () == 1)
    {
      Histable *current_obj = objs->fetch (0);
      jvalues->store (0, dbe_strdup (current_obj->get_name ()));
    }
  else
    {
      char *name = dbe_sprintf (NTXT ("%s (%lld %s)"),
				GTXT ("Multiple Selection"),
				(long long) objs->size (), GTXT ("objects"));
      jvalues->store (0, name);
    }
}

// Set index-object summary list
//
static void
setIndxSummary (Vector<Histable*> *objs, Vector<int> *saligns,
		Vector<char> *mnemonic, Vector<char*> *jlabels,
		Vector<char*> *jvalues)
{
  saligns->store (0, TEXT_LEFT);
  mnemonic->store (0, 'I');
  jlabels->store (0, dbe_strdup (GTXT ("Index Object")));

  if (objs->size () == 1)
    {
      Histable *current_obj = objs->fetch (0);
      jvalues->store (0, dbe_strdup (current_obj->get_name ()));
    }
  else
    {
      char *name = dbe_sprintf (NTXT ("%s (%lld %s)"), GTXT ("Multiple Selection"),
				(long long) objs->size (), GTXT ("objects"));
      jvalues->store (0, name);
    }
}

// Set I/O activity summary list
//
static void
setIOActivitySummary (Vector<Histable*> *objs, Vector<int> *saligns,
		      Vector<char> *mnemonic, Vector<char*> *jlabels,
		      Vector<char*> *jvalues)
{
  saligns->store (0, TEXT_LEFT);
  mnemonic->store (0, 'O');
  jlabels->store (0, dbe_strdup (GTXT ("I/O Activity")));
  if (objs->size () == 1)
    {
      Histable *current_obj = objs->fetch (0);
      jvalues->store (0, dbe_strdup (current_obj->get_name ()));
    }
  else
    {
      char *name = dbe_sprintf (NTXT ("%s (%lld %s)"), GTXT ("Multiple Selection"),
				(long long) objs->size (), GTXT ("objects"));
      jvalues->store (0, name);
    }
}

// Set heap activity summary list
//
static void
setHeapActivitySummary (Vector<Histable*> *objs, Vector<int> *saligns,
			Vector<char> *mnemonic, Vector<char*> *jlabels,
			Vector<char*> *jvalues)
{
  saligns->store (0, TEXT_LEFT);
  mnemonic->store (0, 'O');
  jlabels->store (0, dbe_strdup (GTXT ("Heap Activity")));

  if (objs->size () == 1)
    {
      Histable *current_obj = objs->fetch (0);
      jvalues->store (0, dbe_strdup (current_obj->get_name ()));
    }
  else
    {
      char *name = dbe_sprintf (NTXT ("%s (%lld %s)"), GTXT ("Multiple Selection"),
				(long long) objs->size (), GTXT ("objects"));
      jvalues->store (0, name);
    }
}

//
// Set data-object summary list
//
static void
setDataSummary (Vector<Histable*> *objs, Vector<int> *saligns,
		Vector<char> *mnemonic, Vector<char*> *jlabels,
		Vector<char*> *jvalues)
{
  char *name, *type, *member, *elist;
  DataObject *dobj;
  Vector<DataObject *> *delem;
  Histable *scope;
  int index;
  char *size, *offset, *elements, *scopename;

  // Get the data object elements
  member = elist = type = size = offset = elements = scopename = NULL;

  if (objs->size () == 1)
    {
      Histable *current_obj = objs->fetch (0);
      name = dbe_strdup (current_obj->get_name ());
      dobj = (DataObject *) current_obj;
      type = dobj->get_typename ();
      scope = dobj->get_scope ();
      delem = dbeSession->get_dobj_elements (dobj);
      if (type == NULL)
	type = GTXT ("(Synthetic)");
      if (!scope)
	scopename = dbe_strdup (GTXT ("(Global)"));
      else
	{
	  switch (scope->get_type ())
	    {
	    case Histable::FUNCTION:
	      scopename = dbe_sprintf (NTXT ("%s(%s)"),
				       ((Function*) scope)->module->get_name (),
				       scope->get_name ());
	      break;
	    case Histable::LOADOBJECT:
	    case Histable::MODULE:
	    default:
	      scopename = dbe_strdup (scope->get_name ());
	      break;
	    }
	}

      if (dobj->get_offset () != -1)
	{
	  if (dobj->get_parent ())
	    member = dbe_strdup (dobj->get_parent ()->get_name ());
	  offset = dbe_sprintf (NTXT ("%lld"), (long long) dobj->get_offset ());
	}
      size = dbe_sprintf ("%lld", (long long) dobj->get_size ());

      if (delem->size () > 0)
	{
	  elements = dbe_sprintf (NTXT ("%lld"), (long long) delem->size ());
	  StringBuilder sb_tmp, sb;
	  sb.append (GTXT ("Offset Size  Name\n"));
	  for (index = 0; index < delem->size (); index++)
	    {
	      DataObject *ditem = delem->fetch (index);
	      sb_tmp.sprintf (NTXT ("%6lld %5lld  %s\n"),
			      (long long) ditem->get_offset (),
			      (long long) ditem->get_size (), ditem->get_name ());
	      sb.append (&sb_tmp);
	    }
	  if (sb.charAt (sb.length () - 1) == '\n')
	    sb.setLength (sb.length () - 1);
	  elist = sb.toString ();
	}
    }
  else
    name = dbe_sprintf (NTXT ("%s (%lld %s)"), GTXT ("Multiple Selection"),
			(long long) objs->size (), GTXT ("objects"));

  saligns->store (0, TEXT_LEFT);
  mnemonic->store (0, 'D');
  jlabels->store (0, dbe_strdup (GTXT ("Data Object")));
  jvalues->store (0, name);

  saligns->store (1, TEXT_LEFT);
  mnemonic->store (1, 'S');
  jlabels->store (1, dbe_strdup (GTXT ("Scope")));
  jvalues->store (1, scopename);

  saligns->store (2, TEXT_LEFT);
  mnemonic->store (2, 'T');
  jlabels->store (2, dbe_strdup (GTXT ("Type")));
  jvalues->store (2, dbe_strdup (type));

  saligns->store (3, TEXT_LEFT);
  mnemonic->store (3, 'M');
  jlabels->store (3, dbe_strdup (GTXT ("Member of")));
  jvalues->store (3, member);

  saligns->store (4, TEXT_LEFT);
  mnemonic->store (4, 'O');
  jlabels->store (4, dbe_strdup (GTXT ("Offset")));
  jvalues->store (4, offset);

  saligns->store (5, TEXT_LEFT);
  mnemonic->store (5, 'z');
  jlabels->store (5, dbe_strdup (GTXT ("Size")));
  jvalues->store (5, size);

  saligns->store (6, TEXT_LEFT);
  mnemonic->store (6, 'E');
  jlabels->store (6, dbe_strdup (GTXT ("Elements")));
  jvalues->store (6, elements);

  saligns->store (7, TEXT_LEFT);
  mnemonic->store (7, 'L');
  jlabels->store (7, dbe_strdup (GTXT ("List")));
  jvalues->store (7, elist);
}

#define SUMMARY_NAME 8
#define DSUMMARY_NAME 8
#define LSUMMARY_NAME   7
#define IMSUMMARY_NAME   1

Vector<void*> *
dbeGetSummaryV2 (int dbevindex, Vector<Obj> *sel_objs, int type, int subtype)
{
  if (sel_objs == NULL || sel_objs->size () == 0)
    return NULL;
  DbeView *dbev = dbeSession->getView (dbevindex);
  Vector<Histable*>*objs = new Vector<Histable*>(sel_objs->size ());
  for (int i = 0; i < sel_objs->size (); i++)
    {
      Histable *obj = (Histable *) sel_objs->fetch (i);
      if (obj == NULL)
	continue;
      char *nm = obj->get_name ();
      if (streq (nm, NTXT ("<Total>")))
	{
	  // Special case for 'Total'.
	  // Multi selection which includes 'Total' is only 'Total'
	  objs->reset ();
	  objs->append (obj);
	  break;
	}
      objs->append (obj);
    }
  if (objs->size () == 0)
    return NULL;

  // Set name area
  int nname = SUMMARY_NAME;
  Vector<int> *saligns = new Vector<int>(nname);
  Vector<char>*mnemonic = new Vector<char>(nname);
  Vector<char*> *jlabels = new Vector<char*>(nname);
  Vector<char*> *jvalues = new Vector<char*>(nname);
  Vector<void*> *name_objs = new Vector<void*>(4);
  name_objs->store (0, saligns);
  name_objs->store (1, mnemonic);
  name_objs->store (2, jlabels);
  name_objs->store (3, jvalues);
  setSummary (objs, saligns, mnemonic, jlabels, jvalues);

  MetricList *prop_mlist = new MetricList (dbev->get_metric_ref (MET_NORMAL));
  if (prop_mlist && dbev->comparingExperiments ())
    prop_mlist = dbev->get_compare_mlist (prop_mlist, 0);

  int nitems = prop_mlist->get_items ()->size ();

  // Set the metrics area
  jlabels = new Vector<char*>(nitems);
  Vector<double> *clock_list = new Vector<double>(nitems);
  Vector<double> *excl_list = new Vector<double>(nitems);
  Vector<double> *ep_list = new Vector<double>(nitems);
  Vector<double> *incl_list = new Vector<double>(nitems);
  Vector<double> *ip_list = new Vector<double>(nitems);
  Vector<int> *vtype = new Vector<int>(nitems);

  // Initialize Java String array
  Vector<void*> *metric_objs = new Vector<void*>(8);
  metric_objs->store (0, jlabels);
  metric_objs->store (1, clock_list);
  metric_objs->store (2, excl_list);
  metric_objs->store (3, ep_list);
  metric_objs->store (4, incl_list);
  metric_objs->store (5, ip_list);
  metric_objs->store (6, vtype);

  int last_init = -1;
  for (int i = 0; i < objs->size (); i++)
    {
      Histable *obj = objs->fetch (i);
      // Get the data to be displayed
      Hist_data *data = dbev->get_hist_data (prop_mlist, obj->get_type (), subtype,
					     Hist_data::SELF, obj, dbev->sel_binctx, objs);

      if (data->get_status () != Hist_data::SUCCESS)
	{
	  if (type != DSP_DLAYOUT)
	    { // For data_layout, rows with zero metrics are OK
	      delete data;
	      continue;
	    }
	}
      TValue *values = NULL;
      if (data->get_status () == Hist_data::SUCCESS)
	{
	  Hist_data::HistItem *hi = data->fetch (0);
	  if (hi)
	    values = hi->value;
	}
      Hist_data::HistItem *total = data->get_totals ();
      int index2 = 0;
      char *tstr = GTXT (" Time");
      char *estr = GTXT ("Exclusive ");
      size_t len = strlen (estr);

      // get the metric list from the data
      MetricList *mlist = data->get_metric_list ();
      int index;
      Metric *mitem;
      double clock;
      Vec_loop (Metric*, mlist->get_items (), index, mitem)
      {
	if (mitem->get_subtype () == Metric::STATIC)
	  continue;
	if (last_init < index2)
	  {
	    last_init = index2;
	    jlabels->store (index2, NULL);
	    clock_list->store (index2, 0.0);
	    excl_list->store (index2, 0.0);
	    ep_list->store (index2, 0.0);
	    incl_list->store (index2, 0.0);
	    ip_list->store (index2, 0.0);
	    vtype->store (index2, 0);
	  }
	double dvalue = (values != NULL) ? values[index].to_double () : 0.0;
	double dtotal = total->value[index].to_double ();
	if (mitem->is_time_val ())
	  clock = 1.e+6 * dbeSession->get_clock (-1);
	else
	  clock = 0.0;

	clock_list->store (index2, clock);
	if ((mitem->get_subtype () == Metric::EXCLUSIVE) ||
	    (mitem->get_subtype () == Metric::DATASPACE))
	  {
	    if (i == 0)
	      {
		char *sstr = mitem->get_name ();
		if (!strncmp (sstr, estr, len))
		  sstr += len;
		char *buf, *lstr = strstr (sstr, tstr);
		if (lstr)
		  buf = dbe_strndup (sstr, lstr - sstr);
		else
		  buf = dbe_strdup (sstr);
		jlabels->store (index2, buf);
		vtype->store (index2, mitem->get_vtype ());
	      }
	    dvalue += excl_list->fetch (index2);
	    double percent = dtotal == 0.0 ? dtotal : (dvalue / dtotal) * 100;
	    excl_list->store (index2, dvalue);
	    ep_list->store (index2, percent);
	  }
	else
	  {
	    dvalue += incl_list->fetch (index2);
	    if (dvalue > dtotal)
	      dvalue = dtotal; // temporary correction
	    double percent = dtotal == 0.0 ? dtotal : (dvalue / dtotal) * 100;
	    incl_list->store (index2, dvalue);
	    ip_list->store (index2, percent);
	    index2++;
	  }
      }
      delete data;
    }
  delete prop_mlist;
  Vector<void*> *summary = new Vector<void*>(2);
  summary->store (0, name_objs);
  summary->store (1, metric_objs);
  return summary;
}

// Get Summary List
Vector<void*> *
dbeGetSummary (int dbevindex, Vector<Obj> *sel_objs, int type, int subtype)
{
  bool is_data, is_mem, is_indx, is_iodata, is_heapdata;
  Hist_data::HistItem *total;
  MetricList *prop_mlist; // as passed to get_hist_data
  MetricList *mlist; // as stored in the data
  Metric *mitem;
  int i, nname, nitems, index, index2;
  TValue *values;
  double dvalue, clock;
  Hist_data *data;
  Vector<double> *percent_scale;

  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (sel_objs == NULL || sel_objs->size () == 0)
    return NULL;

  is_mem = false;
  is_data = false;
  is_indx = false;
  is_iodata = false;
  is_heapdata = false;
  nname = SUMMARY_NAME;
  Vector<Histable*>*objs = new Vector<Histable*>(sel_objs->size ());
  if (type == DSP_TIMELINE)
    objs->append ((Histable *) sel_objs->fetch (0));
  else
    {
      switch (type)
	{
	case DSP_FUNCTION:
	  data = dbev->func_data;
	  break;
	case DSP_LINE:
	  data = dbev->line_data;
	  break;
	case DSP_PC:
	  data = dbev->pc_data;
	  break;
	case DSP_SELF:
	  data = dbev->fitem_data;
	  break;
	case DSP_SOURCE:
	case DSP_SOURCE_V2:
	  data = dbev->src_data;
	  break;
	case DSP_DISASM:
	case DSP_DISASM_V2:
	  data = dbev->dis_data;
	  break;
	case DSP_DLAYOUT:
	  is_data = true;
	  nname = LSUMMARY_NAME;
	  data = dbev->dlay_data;
	  break;
	case DSP_DATAOBJ:
	  is_data = true;
	  nname = DSUMMARY_NAME;
	  data = dbev->dobj_data;
	  break;
	case DSP_MEMOBJ:
	  is_data = true;
	  is_mem = true;
	  nname = IMSUMMARY_NAME;
	  data = dbev->get_indxobj_data (subtype);
	  break;
	case DSP_INDXOBJ:
	  is_indx = true;
	  nname = IMSUMMARY_NAME;
	  data = dbev->get_indxobj_data (subtype);
	  break;
	case DSP_IOACTIVITY:
	  is_iodata = true;
	  nname = IMSUMMARY_NAME;
	  data = dbev->iofile_data;
	  break;
	case DSP_IOVFD:
	  is_iodata = true;
	  nname = IMSUMMARY_NAME;
	  data = dbev->iovfd_data;
	  break;
	case DSP_IOCALLSTACK:
	  is_iodata = true;
	  nname = IMSUMMARY_NAME;
	  data = dbev->iocs_data;
	  break;
	case DSP_HEAPCALLSTACK:
	  is_heapdata = true;
	  nname = IMSUMMARY_NAME;
	  data = dbev->heapcs_data;
	  break;
	default:
	  data = NULL;
	  break;
	}
      if (data == NULL || data->get_status () != Hist_data::SUCCESS)
	return NULL;

      Hist_data::HistItem *current_item;
      for (i = 0; i < sel_objs->size (); i++)
	{
	  int sel_index = (int) sel_objs->fetch (i);
	  if (type != DSP_IOACTIVITY && type != DSP_IOVFD &&
	      type != DSP_IOCALLSTACK && type != DSP_HEAPCALLSTACK)
	    {
	      if (sel_index < 0 || sel_index >= data->size ())
		continue;
	      current_item = data->fetch (sel_index);
	      if (current_item->obj == NULL)
		continue;
	    }
	  else
	    {
	      if (sel_index < 0)
		continue;
	      bool found = false;
	      for (int j = 0; j < data->size (); j++)
		{
		  current_item = data->fetch (j);
		  if ((current_item->obj != NULL) && (current_item->obj->id == sel_index))
		    {
		      found = true;
		      break;
		    }
		}
	      if (!found)
		continue;
	    }
	  char *nm = current_item->obj->get_name ();
	  if (streq (nm, NTXT ("<Total>")))
	    {
	      // Special case for 'Total'.
	      // Multi selection which includes 'Total' is only 'Total'
	      objs->reset ();
	      objs->append (current_item->obj);
	      break;
	    }
	  objs->append (current_item->obj);
	}
    }
  if (objs->size () == 0)
    return NULL;

  // Set name area
  Vector<int> *saligns = new Vector<int>(nname);
  Vector<char>*mnemonic = new Vector<char>(nname);
  Vector<char*> *jlabels = new Vector<char*>(nname);
  Vector<char*> *jvalues = new Vector<char*>(nname);
  Vector<void*> *name_objs = new Vector<void*>(4);
  name_objs->store (0, saligns);
  name_objs->store (1, mnemonic);
  name_objs->store (2, jlabels);
  name_objs->store (3, jvalues);
  if (is_mem)
    setMemSummary (objs, saligns, mnemonic, jlabels, jvalues);
  else if (is_indx)
    setIndxSummary (objs, saligns, mnemonic, jlabels, jvalues);
  else if (is_data)
    setDataSummary (objs, saligns, mnemonic, jlabels, jvalues);
  else if (is_iodata)
    setIOActivitySummary (objs, saligns, mnemonic, jlabels, jvalues);
  else if (is_heapdata)
    setHeapActivitySummary (objs, saligns, mnemonic, jlabels, jvalues);
  else
    setSummary (objs, saligns, mnemonic, jlabels, jvalues);

  // Get the reference metrics
  if (is_data)
    prop_mlist = new MetricList (dbev->get_metric_ref (MET_DATA));
  else if (is_indx)
    prop_mlist = new MetricList (dbev->get_metric_ref (MET_INDX));
  else if (is_iodata)
    prop_mlist = new MetricList (dbev->get_metric_ref (MET_IO));
  else if (is_heapdata)
    prop_mlist = new MetricList (dbev->get_metric_ref (MET_HEAP));
  else
    prop_mlist = new MetricList (dbev->get_metric_ref (MET_NORMAL));

  // XXXX a workaround to avoid aggregated data for compare mode, only show base experiment data
  if (prop_mlist && dbev->comparingExperiments ())
    prop_mlist = dbev->get_compare_mlist (prop_mlist, 0);
  nitems = prop_mlist->get_items ()->size ();

  // Set the metrics area
  jlabels = new Vector<char*>(nitems);
  Vector<double> *clock_list = new Vector<double>(nitems);
  Vector<double> *excl_list = new Vector<double>(nitems);
  Vector<double> *ep_list = new Vector<double>(nitems);
  Vector<double> *incl_list = new Vector<double>(nitems);
  Vector<double> *ip_list = new Vector<double>(nitems);
  Vector<int> *vtype = new Vector<int>(nitems);

  // Initialize Java String array
  Vector<void*> *metric_objs = new Vector<void*>(8);
  metric_objs->store (0, jlabels);
  metric_objs->store (1, clock_list);
  metric_objs->store (2, excl_list);
  metric_objs->store (3, ep_list);
  metric_objs->store (4, incl_list);
  metric_objs->store (5, ip_list);
  metric_objs->store (6, vtype);
  percent_scale = new Vector<double>();
  int last_init = -1;
  for (i = 0; i < objs->size (); i++)
    {
      Histable *current_obj = objs->fetch (i);
      // Get the data to be displayed
      data = dbev->get_hist_data (prop_mlist, current_obj->get_type (), subtype,
				  Hist_data::SELF, current_obj, dbev->sel_binctx, objs);
      if (data->get_status () != Hist_data::SUCCESS)
	{
	  if (type != DSP_DLAYOUT)
	    { // For data_layout, rows with zero metrics are OK
	      delete data;
	      continue;
	    }
	}
      Hist_data::HistItem *hi = data->fetch (0);
      values = hi ? hi->value : NULL;
      total = data->get_totals ();
      index2 = 0;

      // get the metric list from the data
      mlist = data->get_metric_list ();

      // We loop over the metrics in mlist.
      // We construct index2, which tells us
      // the corresponding entry in the metric_objs lists.
      // We need this mapping multiple times.
      // So, if you change the looping in any way here,
      // do so as well in other similar loops.
      // All such loops are marked so:
      // See discussion on "mlist-to-index2 mapping".

      Vec_loop (Metric*, mlist->get_items (), index, mitem)
      {
	if (mitem->get_subtype () == Metric::STATIC)
	  continue;
	if (last_init < index2)
	  {
	    last_init = index2;
	    jlabels->store (index2, NULL);
	    clock_list->store (index2, 0.0);
	    excl_list->store (index2, 0.0);
	    ep_list->store (index2, 0.0);
	    incl_list->store (index2, 0.0);
	    ip_list->store (index2, 0.0);
	    vtype->store (index2, 0);
	  }
	dvalue = (values != NULL) ? values[index].to_double () : 0.0;
	double dtotal = total->value[index].to_double ();
	percent_scale->store (index, dtotal == 0. ? 0. : 100. / dtotal);
	if (mitem->is_time_val ())
	  clock = 1.e+6 * dbeSession->get_clock (-1);
	else
	  clock = 0.0;

	clock_list->store (index2, clock);
	if (mitem->get_subtype () == Metric::EXCLUSIVE ||
	    mitem->get_subtype () == Metric::DATASPACE)
	  {
	    if (i == 0)
	      {
		char *sstr = mitem->get_username ();
		char *buf = dbe_strdup (sstr);
		jlabels->store (index2, buf);
		vtype->store (index2, mitem->get_vtype ());
	      }
	    dvalue += excl_list->fetch (index2);
	    double percent = dvalue * percent_scale->fetch (index);
	    excl_list->store (index2, dvalue);
	    ep_list->store (index2, percent);
	    if (is_data || is_indx || is_iodata || is_heapdata)
	      // move to next row, except if there's inclusive data, too
	      index2++;
	  }
	else
	  {
	    dvalue += incl_list->fetch (index2);
	    if (dvalue > dtotal && mitem->get_type () != BaseMetric::DERIVED)
	      dvalue = dtotal; // temporary correction
	    double percent = dvalue * percent_scale->fetch (index);
	    incl_list->store (index2, dvalue);
	    ip_list->store (index2, percent);
	    index2++;
	  }
      }
      delete data;
    }

  // for multi-selection, we have to recompute the derived metrics
  if (objs->size () > 1 &&
      dbev->get_derived_metrics () != NULL &&
      dbev->get_derived_metrics ()->get_items () != NULL &&
      dbev->get_derived_metrics ()->get_items ()->size () > 0)
    {
      // See discussion on "mlist-to-index2 mapping".
      Vector<Metric*> *mvec = new Vector<Metric*>(nitems);
      index2 = 0;
      Vec_loop (Metric*, prop_mlist->get_items (), index, mitem)
      {
	if (mitem->get_subtype () == Metric::STATIC)
	  continue;
	if (mitem->get_subtype () == Metric::EXCLUSIVE ||
	    mitem->get_subtype () == Metric::DATASPACE)
	  {
	    mvec->store (index2, mitem);
	    if (is_data || is_indx || is_iodata || is_heapdata)
	      index2++;
	  }
	else
	  {
	    assert (strcmp (mvec->fetch (index2)->get_cmd (), mitem->get_cmd ()) == 0);
	    index2++;
	  }
      }
      int *map = dbev->get_derived_metrics ()->construct_map (mvec, BaseMetric::EXCLUSIVE, mvec->fetch (0)->get_expr_spec ());
      if (map != NULL)
	{
	  int nmetrics = mvec->size ();
	  double *evalues = (double *) malloc (nmetrics * sizeof (double));
	  double *ivalues = (double *) malloc (nmetrics * sizeof (double));
	  for (index2 = 0; index2 < nmetrics; index2++)
	    {
	      evalues[index2] = excl_list->fetch (index2);
	      ivalues[index2] = incl_list->fetch (index2);
	    }

	  // evaluate derived metrics
	  dbev->get_derived_metrics ()->eval (map, evalues);
	  dbev->get_derived_metrics ()->eval (map, ivalues);
	  for (index2 = 0; index2 < nmetrics; index2++)
	    {
	      excl_list->store (index2, evalues[index2]);
	      incl_list->store (index2, ivalues[index2]);
	    }

	  // recompute percentages for derived metrics    EUGENE maybe all percentage computations should be moved here
	  // See discussion on "mlist-to-index2 mapping".
	  index2 = 0;
	  Vec_loop (Metric*, prop_mlist->get_items (), index, mitem)
	  {
	    if (mitem->get_subtype () == Metric::STATIC)
	      continue;
	    if (mitem->get_subtype () == Metric::EXCLUSIVE ||
		mitem->get_subtype () == Metric::DATASPACE)
	      {
		if (mitem->get_type () == BaseMetric::DERIVED)
		  ep_list->store (index2, excl_list->fetch (index2) * percent_scale->fetch (index));
		if (is_data || is_indx || is_iodata || is_heapdata)
		  index2++;
	      }
	    else
	      {
		if (mitem->get_type () == BaseMetric::DERIVED)
		  ip_list->store (index2, incl_list->fetch (index2) * percent_scale->fetch (index));
		index2++;
	      }
	  }
	  free (evalues);
	  free (ivalues);
	  free (map);
	}
      delete mvec;
    }
  delete prop_mlist;
  Vector<void*> *summary = new Vector<void*>(2);
  summary->store (0, name_objs);
  summary->store (1, metric_objs);
  delete objs;
  delete percent_scale;
  return summary;
}

char *
dbeGetExpName (int /*dbevindex*/, char *dir_name)
{
  char *ret;
  char *warn;
  if (col_ctr == NULL)
    col_ctr = new Coll_Ctrl (1); // Potential race condition?
  if (dir_name != NULL)
    {
      ret = col_ctr->set_directory (dir_name, &warn);
      // note that the warning and error msgs are written to stderr, not returned to caller
      if (warn != NULL)
	fprintf (stderr, NTXT ("%s"), warn);
      if (ret != NULL)
	fprintf (stderr, NTXT ("%s"), ret);
    }
  return dbe_strdup (col_ctr->get_expt ());
}

// === CollectDialog HWC info ===

Vector<Vector<char*>*> *
dbeGetHwcSets (int /*dbevindex*/, bool forKernel)
{
  Vector<Vector<char*>*> *list = new Vector<Vector<char*>*>(2);
  char * defctrs = hwc_get_default_cntrs2 (forKernel, 1);
  Vector<char*> *i18n = new Vector<char*>(1); // User name
  Vector<char*> *name = new Vector<char*>(1); // Internal name
  if (NULL != defctrs)
    {
      i18n->store (0, strdup (defctrs));
      name->store (0, strdup (NTXT ("default")));
    }
  list->store (0, i18n);
  list->store (1, name);
  return list;
}

static Vector<void*> *
dbeGetHwcs (Hwcentry **hwcs)
{
  int sz;
  for (sz = 0; hwcs && hwcs[sz]; sz++)
    ;
  Vector<void*> *list = new Vector<void*>(9);
  Vector<char*> *i18n = new Vector<char*>(sz);
  Vector<char*> *name = new Vector<char*>(sz);
  Vector<char*> *int_name = new Vector<char*>(sz);
  Vector<char*> *metric = new Vector<char*>(sz);
  Vector<long long> *val = new Vector<long long>(sz);
  Vector<int> *timecvt = new Vector<int>(sz);
  Vector<int> *memop = new Vector<int>(sz);
  Vector<char*> *short_desc = new Vector<char*>(sz);
  Vector<Vector<int>*> *reglist_v = new Vector<Vector<int>*>(sz);
  Vector<bool> *supportsAttrs = new Vector<bool>(sz);
  Vector<bool> *supportsMemspace = new Vector<bool>(sz);

  for (int i = 0; i < sz; i++)
    {
      Hwcentry *ctr = hwcs[i];
      Vector<int> *registers = new Vector<int>(MAX_PICS);
      regno_t *reglist = ctr->reg_list;
      for (int k = 0; !REG_LIST_EOL (reglist[k]) && k < MAX_PICS; k++)
	registers->store (k, reglist[k]);

      i18n->store (i, dbe_strdup (hwc_i18n_metric (ctr)));
      name->store (i, dbe_strdup (ctr->name));
      int_name->store (i, dbe_strdup (ctr->int_name));
      metric->store (i, dbe_strdup (ctr->metric));
      val->store (i, ctr->val); // signed promotion from int
      timecvt->store (i, ctr->timecvt);
      memop->store (i, ctr->memop);
      reglist_v->store (i, registers);
      short_desc->store (i, dbe_strdup (ctr->short_desc));
      supportsAttrs->store (i, true);
      supportsMemspace->store (i, ABST_MEMSPACE_ENABLED (ctr->memop));
    }
  list->store (0, i18n);
  list->store (1, name);
  list->store (2, int_name);
  list->store (3, metric);
  list->store (4, val);
  list->store (5, timecvt);
  list->store (6, memop);
  list->store (7, short_desc);
  list->store (8, reglist_v);
  list->store (9, supportsAttrs);
  list->store (10, supportsMemspace);
  return list;
}

Vector<void *> *
dbeGetHwcsAll (int /*dbevindex*/, bool forKernel)
{
  Vector<void*> *list = new Vector<void*>(2);
  list->store (0, dbeGetHwcs (hwc_get_std_ctrs (forKernel)));
  list->store (1, dbeGetHwcs (hwc_get_raw_ctrs (forKernel)));
  return list;
}

Vector<char*> *
dbeGetHwcHelp (int /*dbevindex*/, bool forKernel)
{
  Vector<char*> *strings = new Vector<char*>(32);
  FILE *f = tmpfile ();
  hwc_usage_f (forKernel, f, "", 0, 0, 1); // writes to f
  fflush (f);
  fseek (f, 0, SEEK_SET);
#define MAX_LINE_LEN 2048
  char buff[MAX_LINE_LEN];
  int ii = 0;
  while (fgets (buff, MAX_LINE_LEN, f))
    strings->store (ii++, dbe_strdup (buff));
  fclose (f);
  return strings;
}

Vector<char*> *
dbeGetHwcAttrList (int /*dbevindex*/, bool forKernel)
{
  char ** attr_list = hwc_get_attrs (forKernel); // Get Attribute list
  int size;
  for (size = 0; attr_list && attr_list[size]; size++)
    ;

  Vector<char*> *name = new Vector<char*>(size);
  for (int i = 0; i < size; i++)
    name->store (i, dbe_strdup (attr_list[i]));
  return name;
}

//Get maximum number of simultaneous counters
int
dbeGetHwcMaxConcurrent (int /*dbevindex*/, bool forKernel)
{
  return hwc_get_max_concurrent (forKernel);
}

// === End CollectDialog HWC info ===


//  Instruction-frequency data
Vector<char*> *
dbeGetIfreqData (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (!dbeSession->is_ifreq_available ())
    return NULL;
  int size = dbeSession->nexps ();
  if (size == 0)
    return NULL;

  // Initialize Java String array
  Vector<char*> *list = new Vector<char*>();
  for (int i = 0; i < size; i++)
    {
      Experiment *exp = dbeSession->get_exp (i);
      if (exp->broken || !dbev->get_exp_enable (i) || !exp->ifreqavail)
	continue;
      // write a header for the experiment
      list->append (dbe_sprintf (GTXT ("Instruction frequency data from experiment %s\n\n"),
				 exp->get_expt_name ()));
      // add its instruction frequency messages
      char *ifreq = pr_mesgs (exp->fetch_ifreq (), NTXT (""), NTXT (""));
      list->append (ifreq);
    }
  return list;
}

//   LeakList related methods
//
Vector<void*> *
dbeGetLeakListInfo (int dbevindex, bool leakflag)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  MetricList *origmlist = dbev->get_metric_list (MET_NORMAL);
  MetricList *nmlist = new MetricList (origmlist);
  if (leakflag)
    nmlist->set_metrics (NTXT ("e.heapleakbytes:e.heapleakcnt:name"), true,
			 dbev->get_derived_metrics ());
  else
    nmlist->set_metrics (NTXT ("e.heapallocbytes:e.heapalloccnt:name"), true,
			 dbev->get_derived_metrics ());
  MetricList *mlist = new MetricList (nmlist);
  delete nmlist;

  CStack_data *lam = dbev->get_cstack_data (mlist);
  if (lam == NULL || lam->size () == 0)
    {
      delete lam;
      delete mlist;
      return NULL;
    }
  Vector<Vector<Obj>*> *evalue = new Vector<Vector<Obj>*>(lam->size ());
  Vector<Vector<Obj>*> *pcstack = new Vector<Vector<Obj>*>(lam->size ());
  Vector<Vector<Obj>*> *offstack = new Vector<Vector<Obj>*>(lam->size ());
  Vector<Vector<Obj>*> *fpcstack = new Vector<Vector<Obj>*>(lam->size ());
  Vector<Vector<Obj>*> *sumval = new Vector<Vector<Obj>*>(lam->size ());

  int index;
  CStack_data::CStack_item *lae;
  Vec_loop (CStack_data::CStack_item*, lam->cstack_items, index, lae)
  {
    Vector<Obj> *jivals = NULL;
    if (lae != NULL)
      {
	jivals = new Vector<Obj>(4);
	jivals->store (0, (Obj) (index + 1));
	jivals->store (1, (Obj) lae->value[1].ll);
	jivals->store (2, (Obj) lae->value[0].ll);
	jivals->store (3, (Obj) (leakflag ? 1 : 2));
      }
    evalue->store (index, jivals);
    int snum = lae->stack->size ();
    Vector<Obj> *jivals1 = new Vector<Obj>(snum);
    Vector<Obj> *jivals2 = new Vector<Obj>(snum);
    Vector<Obj> *jivals3 = new Vector<Obj>(snum);
    if (lae->stack != NULL)
      {
	for (int i = lae->stack->size () - 1; i >= 0; i--)
	  {
	    DbeInstr *instr = lae->stack->fetch (i);
	    jivals1->store (i, (Obj) instr);
	    jivals2->store (i, (Obj) instr->func);
	    jivals3->store (i, (Obj) instr->addr);
	  }
      }
    fpcstack->store (index, jivals1);
    pcstack->store (index, jivals2);
    offstack->store (index, jivals3);
    lae++;
  }
  Vector<Obj> *jivals4 = new Vector<Obj>(3);
  jivals4->store (0, (Obj) lam->size ());
  jivals4->store (1, (Obj) lam->total->value[1].ll);
  jivals4->store (2, (Obj) lam->total->value[0].ll);
  sumval->store (0, jivals4);
  delete lam;
  delete mlist;
  Vector<void*> *earray = new Vector<void*>(5);
  earray->store (0, evalue);
  earray->store (1, pcstack);
  earray->store (2, offstack);
  earray->store (3, fpcstack);
  earray->store (4, sumval);
  return earray;
}

// Map timeline address to function instr
//
Obj
dbeGetObject (int dbevindex, Obj sel_func, Obj sel_pc)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
   abort ();
  if (sel_pc)
    return sel_pc;
  return sel_func;
}

char *
dbeGetName (int /*dbevindex*/, int exp_id)
// This function's name is not descriptive enough - it returns a string
//   containing the full experiment name with path, process name, and PID.
// There are various dbe functions that provide experiment name and experiment
// details, and they should probably be consolidated/refactored. (TBR)
// For another example of similar output formatting, see dbeGetExpName().
{
  int id = (exp_id < 0) ? 0 : exp_id;
  Experiment *exp = dbeSession->get_exp (id);
  if (exp == NULL)
    return NULL;
  char *buf =
	  dbe_sprintf (NTXT ("%s [%s, PID %d]"),
		       exp->get_expt_name (),
		       exp->utargname != NULL ? exp->utargname : GTXT ("(unknown)"),
		       exp->getPID ());
  return buf;
}

Vector<char*> *
dbeGetExpVerboseName (Vector<int> *exp_ids)
{
  int len = exp_ids->size ();
  Vector<char*> *list = new Vector<char*>(len);
  for (int i = 0; i < len; i++)
    {
      char * verboseName = dbeGetName (0, exp_ids->fetch (i)); // no strdup()
      list->store (i, verboseName);
    }
  return list;
}

long long
dbeGetStartTime (int /*dbevindex*/, int exp_id)
{
  int id = (exp_id < 0) ? 0 : exp_id;
  Experiment *exp = dbeSession->get_exp (id);
  return exp ? exp->getStartTime () : (long long) 0;
}

long long
dbeGetRelativeStartTime (int /*dbevindex*/, int exp_id)
{
  int id = (exp_id < 0) ? 0 : exp_id;
  Experiment *exp = dbeSession->get_exp (id);
  return exp ? exp->getRelativeStartTime () : (long long) 0;
}

long long
dbeGetEndTime (int /*dbevindex*/, int exp_id)
{
  int id = (exp_id < 0) ? 0 : exp_id;
  Experiment *exp = dbeSession->get_exp (id);

  // Experiment::getEndTime was initially implemented as
  // returning exp->last_event. To preserve the semantics
  // new Experiment::getLastEvent() is used here.
  return exp ? exp->getLastEvent () : (long long) 0;
}

int
dbeGetClock (int /*dbevindex*/, int exp_id)
{
  return dbeSession->get_clock (exp_id);
}

long long
dbeGetWallStartSec (int /*dbevindex*/, int exp_id)
{
  int id = (exp_id < 0) ? 0 : exp_id;
  Experiment *exp = dbeSession->get_exp (id);
  return exp ? exp->getWallStartSec () : 0ll;
}

char *
dbeGetHostname (int /*dbevindex*/, int exp_id)
{
  int id = (exp_id < 0) ? 0 : exp_id;
  Experiment *exp = dbeSession->get_exp (id);
  return exp ? dbe_strdup (exp->hostname) : NULL;
}

static DataView *
getTimelinePackets (int dbevindex, int exp_id, int data_id, int entity_prop_id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  const int sortprop_count = 3;
  const int sortprops[sortprop_count] = {
    PROP_HWCTAG, // aux
    entity_prop_id,
    PROP_TSTAMP
  };
  DataView *packets = dbev->get_filtered_events (exp_id, data_id,
						 sortprops, sortprop_count);
  return packets;
}

static long
getIdxByVals (DataView * packets, int aux, int entity_prop_val,
	      uint64_t time, DataView::Relation rel)
{
  const int sortprop_count = 3;
  Datum tval[sortprop_count];
  tval[0].setUINT32 (aux);
  tval[1].setUINT32 (entity_prop_val); //CPUID, LWPID, THRID are downsized to 32
  tval[2].setUINT64 (time);
  long idx = packets->getIdxByVals (tval, rel);
  return idx;
}

static bool
isValidIdx (DataView * packets, int entity_prop_id,
	    int aux, int entity_prop_val, long idx)
{
  if (idx < 0 || idx >= packets->getSize ())
    return false;
  int pkt_aux = packets->getIntValue (PROP_HWCTAG, idx);
  if (pkt_aux != aux)
    return false;
  if (entity_prop_id == PROP_EXPID)
    return true; // not a packet property; we know the packet is in this experiment
  if (entity_prop_id == PROP_NONE)
    return true; // not a packet property; we know the packet is in this experiment
  int pkt_ent = packets->getIntValue (entity_prop_id, idx);
  if (pkt_ent != entity_prop_val)
    return false;
  return true;
}

static bool
hasInvisbleTLEvents (Experiment *exp, VMode view_mode)
{
  if (exp->has_java && view_mode == VMODE_USER)
    return true;
  return false;
}

static bool
isVisibleTLEvent (Experiment *exp, VMode view_mode, DataView* packets, long idx)
{
  if (hasInvisbleTLEvents (exp, view_mode))
    {
      JThread *jthread = (JThread*) packets->getObjValue (PROP_JTHREAD, idx);
      if (jthread == JTHREAD_NONE || (jthread != NULL && jthread->is_system ()))
	return false;
    }
  return true;
}

static long
getTLVisibleIdxByStepping (Experiment *exp, VMode view_mode, int entity_prop_id,
			   DataView * packets, int aux, int entity_prop_val,
			   long idx, long move_count, int direction)
{
  assert (move_count >= 0);
  assert (direction == 1 || direction == -1 || direction == 0);
  if (direction == 0 /* precise hit required */)
    move_count = 0;
  do
    {
      if (!isValidIdx (packets, entity_prop_id, aux, entity_prop_val, idx))
	return -1;
      if (isVisibleTLEvent (exp, view_mode, packets, idx))
	{
	  if (move_count <= 0)
	    break;
	  move_count--;
	}
      if (direction == 0)
	return -1;
      idx += direction;
    }
  while (1);
  return idx;
}

static long
getTLVisibleIdxByVals (Experiment *exp, VMode view_mode, int entity_prop_id,
		       DataView * packets,
		       int aux, int entity_prop_val, uint64_t time, DataView::Relation rel)
{
  long idx = getIdxByVals (packets, aux, entity_prop_val, time, rel);
  if (!hasInvisbleTLEvents (exp, view_mode))
    return idx;
  if (idx < 0)
    return idx;
  if (rel == DataView::REL_EQ)
    return -1; // would require bi-directional search... not supported for now
  int direction = (rel == DataView::REL_LT || rel == DataView::REL_LTEQ) ? -1 : 1;
  idx = getTLVisibleIdxByStepping (exp, view_mode, entity_prop_id, packets,
				   aux, entity_prop_val,
				   idx, 0 /* first match */, direction);
  return idx;
}

// In thread mode, the entity name for non Java thread should be the 1st func
// from the current thread's stack. See #4961315
static char*
getThreadRootFuncName (int, int, int, int, VMode)
{
  return NULL; // until we figure out what we want to show... YXXX
}

Vector<void*> *
dbeGetEntityProps (int dbevindex) //YXXX TBD, should this be exp-specific?
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Vector<int> *prop_id = new Vector<int>();
  Vector<char*> *prop_name = new Vector<char*>();
  Vector<char*> *prop_uname = new Vector<char*>();
  Vector<char*> *prop_cname = new Vector<char*>(); //must match TLModeCmd vals!

  prop_id->append (PROP_NONE);
  prop_name->append (dbe_strdup (GTXT ("NONE")));
  prop_uname->append (dbe_strdup (GTXT ("Unknown")));
  prop_cname->append (dbe_strdup (NTXT ("unknown")));

  prop_id->append (PROP_LWPID);
  prop_name->append (dbe_strdup (GTXT ("LWPID")));
  prop_uname->append (dbe_strdup (GTXT ("LWP")));
  prop_cname->append (dbe_strdup (NTXT ("lwp")));

  prop_id->append (PROP_THRID);
  prop_name->append (dbe_strdup (GTXT ("THRID")));
  prop_uname->append (dbe_strdup (GTXT ("Thread")));
  prop_cname->append (dbe_strdup (NTXT ("thread")));

  prop_id->append (PROP_CPUID);
  prop_name->append (dbe_strdup (GTXT ("CPUID")));
  prop_uname->append (dbe_strdup (GTXT ("CPU")));
  prop_cname->append (dbe_strdup (NTXT ("cpu")));

  prop_id->append (PROP_EXPID);
  prop_name->append (dbe_strdup (GTXT ("EXPID")));
  prop_uname->append (dbe_strdup (GTXT ("Process"))); // placeholder...
  // ...until we finalize how to expose user-level Experiments, descendents
  prop_cname->append (dbe_strdup (NTXT ("experiment")));
  Vector<void*> *darray = new Vector<void*>();
  darray->store (0, prop_id);
  darray->store (1, prop_name);
  darray->store (2, prop_uname);
  darray->store (3, prop_cname);
  return darray;
}

Vector<void*> *
dbeGetEntities (int dbevindex, int exp_id, int entity_prop_id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Experiment *exp = dbeSession->get_exp (exp_id);
  if (exp == NULL)
    return NULL;

  // Recognize and skip faketime experiments
  if (exp->timelineavail == false)
    return NULL;
  Vector<Histable*> *tagObjs = exp->getTagObjs ((Prop_type) entity_prop_id);
  int total_nelem;
  if (tagObjs)
    total_nelem = (int) tagObjs->size ();
  else
    total_nelem = 0;
  const VMode view_mode = dbev->get_view_mode ();
  bool show_java_threadnames = (entity_prop_id == PROP_THRID &&
				view_mode != VMODE_MACHINE);
  // allocate the structures for the return
  Vector<int> *entity_prop_vals = new Vector<int>();
  Vector<char*> *jthr_names = new Vector<char*>();
  Vector<char*> *jthr_g_names = new Vector<char*>();
  Vector<char*> *jthr_p_names = new Vector<char*>();

  // now walk the tagObjs from the experiment, and check for filtering
  for (int tagObjsIdx = 0; tagObjsIdx < total_nelem; tagObjsIdx++)
    {
      int entity_prop_val = (int) ((Other *) tagObjs->fetch (tagObjsIdx))->tag;
      entity_prop_vals->append (entity_prop_val);
      char *jname, *jgname, *jpname;
      JThread *jthread = NULL;
      bool has_java_threadnames = false;
      if (show_java_threadnames)
	{
	  jthread = exp->get_jthread (entity_prop_val);
	  has_java_threadnames = (jthread != JTHREAD_DEFAULT
				  && jthread != JTHREAD_NONE);
	}
      if (!has_java_threadnames)
	{
	  jname = jgname = jpname = NULL;
	  if (entity_prop_id == PROP_THRID || entity_prop_id == PROP_LWPID)
	    // if non Java thread, set thread name to the 1st func
	    // from the current thread's stack. see #4961315
	    jname = getThreadRootFuncName (dbevindex, exp_id, entity_prop_id,
					   entity_prop_val, view_mode);
	}
      else
	{
	  jname = dbe_strdup (jthread->name);
	  jgname = dbe_strdup (jthread->group_name);
	  jpname = dbe_strdup (jthread->parent_name);
	}
      jthr_names->append (jname);
      jthr_g_names->append (jgname);
      jthr_p_names->append (jpname);
    }
  Vector<char*> *entity_prop_name_v = new Vector<char*>();
  char* entity_prop_name = dbeSession->getPropName (entity_prop_id);
  entity_prop_name_v->append (entity_prop_name);
  Vector<void*> *darray = new Vector<void*>(5);
  darray->store (0, entity_prop_vals);
  darray->store (1, jthr_names);
  darray->store (2, jthr_g_names);
  darray->store (3, jthr_p_names);
  darray->store (4, entity_prop_name_v); // vector only has 1 element
  return darray;
}

// TBR: dbeGetEntities() can be set to private now that we have dbeGetEntitiesV2()
Vector<void*> *
dbeGetEntitiesV2 (int dbevindex, Vector<int> *exp_ids, int entity_prop_id)
{
  int sz = exp_ids->size ();
  Vector<void*> *res = new Vector<void*>(sz);
  for (int ii = 0; ii < sz; ii++)
    {
      int expIdx = exp_ids->fetch (ii);
      Vector<void*>* ents = dbeGetEntities (dbevindex, expIdx, entity_prop_id);
      res->store (ii, ents);
    }
  return res;
}

//YXXX old-tl packets still used for details
static Vector<void*> *
getTLDetailValues (int dbevindex, Experiment * exp, int data_id,
		   VMode view_mode, DataView *packets, long idx)
{
  Vector<long long> *value = new Vector<long long>(15);
  long i = idx;
  if (data_id == DATA_SAMPLE || data_id == DATA_GCEVENT)
    {
      //YXXX DATA_SAMPLE not handled but could be.
    }
  Obj stack = (unsigned long) getStack (view_mode, packets, i);
  Vector<Obj> *funcs = stack ? dbeGetStackFunctions (dbevindex, stack) : NULL;
  Function *func = (Function*)
	  getStackPC (0, view_mode, packets, i)->convertto (Histable::FUNCTION);
  // Fill common data
  value->store (0, packets->getIntValue (PROP_LWPID, i));
  value->store (1, packets->getIntValue (PROP_THRID, i));
  value->store (2, packets->getIntValue (PROP_CPUID, i));
  value->store (3, packets->getLongValue (PROP_TSTAMP, i));
  value->store (4, (unsigned long) stack);
  value->store (5, (unsigned long) func);

  // Fill specific data
  switch (data_id)
    {
    case DATA_CLOCK:
      value->store (6, packets->getIntValue (PROP_MSTATE, i));
      {
	hrtime_t interval = exp->get_params ()->ptimer_usec * 1000LL // nanoseconds
		* packets->getLongValue (PROP_NTICK, i);
	value->store (7, interval);
      }
      value->store (8, packets->getIntValue (PROP_OMPSTATE, i));
      value->store (9, packets->getLongValue (PROP_EVT_TIME, i)); // visual duration
      break;
    case DATA_SYNCH:
      value->store (6, packets->getLongValue (PROP_EVT_TIME, i));
      value->store (7, packets->getLongValue (PROP_SOBJ, i));
      break;
    case DATA_HWC:
      value->store (6, packets->getLongValue (PROP_HWCINT, i));
      value->store (7, packets->getLongValue (PROP_VADDR, i)); // data vaddr
      value->store (8, packets->getLongValue (PROP_PADDR, i)); // data paddr
      value->store (9, packets->getLongValue (PROP_VIRTPC, i)); // pc paddr
      value->store (10, packets->getLongValue (PROP_PHYSPC, i)); // pc vaddr
      break;
    case DATA_RACE:
      value->store (6, packets->getIntValue (PROP_RTYPE, i));
      value->store (7, packets->getIntValue (PROP_RID, i));
      value->store (8, packets->getLongValue (PROP_RVADDR, i));
      break;
    case DATA_DLCK:
      value->store (6, packets->getIntValue (PROP_DTYPE, i));
      value->store (7, packets->getIntValue (PROP_DLTYPE, i));
      value->store (8, packets->getIntValue (PROP_DID, i));
      value->store (9, packets->getLongValue (PROP_DVADDR, i));
      break;
    case DATA_HEAP:
    case DATA_HEAPSZ:
      value->store (6, packets->getIntValue (PROP_HTYPE, i));
      value->store (7, packets->getLongValue (PROP_HSIZE, i));
      value->store (8, packets->getLongValue (PROP_HVADDR, i));
      value->store (9, packets->getLongValue (PROP_HOVADDR, i));
      value->store (10, packets->getLongValue (PROP_HLEAKED, i));
      value->store (11, packets->getLongValue (PROP_HFREED, i));
      value->store (12, packets->getLongValue (PROP_HCUR_ALLOCS, i)); // signed int64_t
      value->store (13, packets->getLongValue (PROP_HCUR_LEAKS, i));
      break;
    case DATA_IOTRACE:
      value->store (6, packets->getIntValue (PROP_IOTYPE, i));
      value->store (7, packets->getIntValue (PROP_IOFD, i));
      value->store (8, packets->getLongValue (PROP_IONBYTE, i));
      value->store (9, packets->getLongValue (PROP_EVT_TIME, i));
      value->store (10, packets->getIntValue (PROP_IOVFD, i));
      break;
    }
  Vector<void*> *result = new Vector<void*>(5);
  result->store (0, value);
  result->store (1, funcs); // Histable::Function*
  result->store (2, funcs ? dbeGetFuncNames (dbevindex, funcs) : 0); // formatted func names
  result->store (3, stack ? dbeGetStackPCs (dbevindex, stack) : 0); // Histable::DbeInstr*
  result->store (4, stack ? dbeGetStackNames (dbevindex, stack) : 0); // formatted pc names
  return result;
}

Vector<void*> *
dbeGetTLDetails (int dbevindex, int exp_id, int data_id,
		 int entity_prop_id, Obj event_id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Experiment *exp = dbeSession->get_exp (exp_id < 0 ? 0 : exp_id);
  if (exp == NULL)
    return NULL;
  DataView *packets =
	  getTimelinePackets (dbevindex, exp_id, data_id, entity_prop_id);
  if (!packets)
    return NULL;

  VMode view_mode = dbev->get_view_mode ();
  long idx = (long) event_id;
  Vector<void*> *values = getTLDetailValues (dbevindex, exp, data_id, view_mode, packets, idx);
  return values;
}

Vector<Obj> *
dbeGetStackFunctions (int dbevindex, Obj stack)
{
  Vector<Obj> *instrs = dbeGetStackPCs (dbevindex, stack);
  if (instrs == NULL)
    return NULL;
  int stsize = instrs->size ();
  Vector<Obj> *jivals = new Vector<Obj>(stsize);
  for (int i = 0; i < stsize; i++)
    {
      Histable *obj = (Histable*) instrs->fetch (i);
      // if ( obj->get_type() != Histable::LINE ) {//YXXX what is this?
      // Remove the above check: why not do this conversion for lines -
      // otherwise filtering in timeline by function stack in omp user mode is broken
      obj = obj->convertto (Histable::FUNCTION);
      jivals->store (i, (Obj) obj);
    }
  delete instrs;
  return jivals;
}

Vector<void*> *
dbeGetStacksFunctions (int dbevindex, Vector<Obj> *stacks)
{
  long sz = stacks->size ();
  Vector<void*> *res = new Vector<void*>(sz);
  for (int ii = 0; ii < sz; ii++)
    {
      Obj stack = stacks->fetch (ii);
      Vector<Obj> *jivals = dbeGetStackFunctions (dbevindex, stack);
      res->store (ii, jivals);
    }
  return res;
}

Vector<Obj> *
dbeGetStackPCs (int dbevindex, Obj stack)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (stack == 0)
    return NULL;

  bool show_all = dbev->isShowAll ();
  Vector<Histable*> *instrs = CallStack::getStackPCs ((void *) stack, !show_all);
  int stsize = instrs->size ();
  int istart = 0;
  bool showAll = dbev->isShowAll ();
  for (int i = 0; i < stsize - 1; i++)
    {
      Function *func = (Function*) instrs->fetch (i)->convertto (Histable::FUNCTION);
      int ix = func->module->loadobject->seg_idx;
      if (showAll && dbev->get_lo_expand (ix) == LIBEX_API)
	// truncate stack here:  LIBRARY_VISIBILITY if we are using API only but no hide
	istart = i;
    }
  stsize = stsize - istart;
  Vector<Obj> *jlvals = new Vector<Obj>(stsize);
  for (int i = 0; i < stsize; i++)
    {
      Histable *instr = instrs->fetch (i + istart);
      jlvals->store (i, (Obj) instr);
    }
  delete instrs;
  return jlvals;
}

Vector<char*> *
dbeGetStackNames (int dbevindex, Obj stack)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  Vector<Obj> *instrs = dbeGetStackPCs (dbevindex, stack);
  if (instrs == NULL)
    return NULL;
  int stsize = instrs->size ();
  Vector<char*> *list = new Vector<char*>(stsize);
  bool showAll = dbev->isShowAll ();
  for (int i = 0; i < stsize; i++)
    {
      Histable* instr = (Histable*) instrs->fetch (i);
      if (!showAll)
	{
	  // LIBRARY_VISIBILITY
	  Function *func = (Function*) instr->convertto (Histable::FUNCTION);
	  LoadObject *lo = ((Function*) func)->module->loadobject;
	  if (dbev->get_lo_expand (lo->seg_idx) == LIBEX_HIDE)
	    {
	      list->store (i, dbe_strdup (lo->get_name ()));
	      continue;
	    }
	}
      list->store (i, dbe_strdup (instr->get_name (dbev->get_name_format ())));
    }
  delete instrs;
  return list;
}

Vector<void*> *
dbeGetSamples (int dbevindex, int exp_id, int64_t lo_idx, int64_t hi_idx)
{
  DataView * packets =
	  getTimelinePackets (dbevindex, exp_id, DATA_SAMPLE, PROP_EXPID);
  if (packets == NULL || packets->getSize () == 0)
    return NULL;
  long lo;
  if (lo_idx < 0)
    lo = 0;
  else
    lo = (long) lo_idx;

  long long max = packets->getSize () - 1;
  long hi;
  if (hi_idx < 0 || hi_idx > max)
    hi = (long) max;
  else
    hi = (long) hi_idx;

  Vector<Vector<long long>*> *sarray = new Vector<Vector<long long>*>;
  Vector<long long>* starts = new Vector<long long>;
  Vector<long long>* ends = new Vector<long long>;
  Vector<long long>* rtimes = new Vector<long long>;
  Vector<char*> *startNames = new Vector<char*>;
  Vector<char*> *endNames = new Vector<char*>;
  Vector<int> *sampId = new Vector<int>;

  for (long index = lo; index <= hi; index++)
    {
      Sample *sample = (Sample*) packets->getObjValue (PROP_SMPLOBJ, index);
      PrUsage *prusage = sample->get_usage ();
      if (prusage == NULL)
	prusage = new PrUsage;
      Vector<long long> *states = prusage->getMstateValues ();
      sarray->append (states);
      starts->append (sample->get_start_time ());
      ends->append (sample->get_end_time ());
      rtimes->append (prusage->pr_rtime);
      startNames->append (dbe_strdup (sample->get_start_label ()));
      endNames->append (dbe_strdup (sample->get_end_label ()));
      sampId->append (sample->get_number ());
    }
  Vector<void *> *res = new Vector<void*>(6);
  res->store (0, sarray);
  res->store (1, starts);
  res->store (2, ends);
  res->store (3, rtimes);
  res->store (4, startNames);
  res->store (5, endNames);
  res->store (6, sampId);
  return res;
}

Vector<void*> *
dbeGetGCEvents (int dbevindex, int exp_id, int64_t lo_idx, int64_t hi_idx)
{
  DataView *packets =
	  getTimelinePackets (dbevindex, exp_id, DATA_GCEVENT, PROP_EXPID);
  if (packets == NULL || packets->getSize () == 0)
    return NULL;

  long lo;
  if (lo_idx < 0)
    lo = 0;
  else
    lo = (long) lo_idx;
  long long max = packets->getSize () - 1;
  long hi;
  if (hi_idx < 0 || hi_idx > max)
    hi = (long) max;
  else
    hi = (long) hi_idx;

  Vector<long long>* starts = new Vector<long long>;
  Vector<long long>* ends = new Vector<long long>;
  Vector<int> *eventId = new Vector<int>;
  for (long index = lo; index <= hi; index++)
    {
      GCEvent *gcevent = (GCEvent*) packets->getObjValue (PROP_GCEVENTOBJ, index);
      if (gcevent)
	{
	  starts->append (gcevent->start);
	  ends->append (gcevent->end);
	  eventId->append (gcevent->id);
	}
    }
  Vector<void *> *res = new Vector<void*>(3);
  res->store (0, starts);
  res->store (1, ends);
  res->store (2, eventId);
  return res;
}

Vector<Vector<char*>*>*
dbeGetIOStatistics (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  Hist_data *hist_data;
  Hist_data::HistItem *hi;
  FileData *fDataTotal;

  hist_data = dbev->iofile_data;
  if (hist_data == NULL)
    return NULL;
  hi = hist_data->fetch (0);
  fDataTotal = (FileData*) hi->obj;

  Vector<char*> *writeStat = new Vector<char*>;
  Vector<char*> *readStat = new Vector<char*>;
  Vector<char*> *otherStat = new Vector<char*>;
  Vector<char*> *errorStat = new Vector<char*>;

  writeStat->append (dbe_strdup (GTXT ("Write Statistics")));
  readStat->append (dbe_strdup (GTXT ("Read Statistics")));
  otherStat->append (dbe_strdup (GTXT ("Other I/O Statistics")));
  errorStat->append (dbe_strdup (GTXT ("I/O Error Statistics")));

  StringBuilder sb;
  if (fDataTotal->getWriteCnt () > 0)
    {
      if (fDataTotal->getW0KB1KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("0KB - 1KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW0KB1KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW1KB8KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1KB - 8KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW1KB8KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW8KB32KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("8KB - 32KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW8KB32KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW32KB128KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("32KB - 128KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW32KB128KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW128KB256KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("128KB - 256KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW128KB256KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW256KB512KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("256KB - 512KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW256KB512KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW512KB1000KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("512KB - 1000KB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW512KB1000KBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW1000KB10MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1000KB - 10MB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW1000KB10MBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW10MB100MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10MB - 100MB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW10MB100MBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW100MB1GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100MB - 1GB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW100MB1GBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW1GB10GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1GB - 10GB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW1GB10GBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW10GB100GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10GB - 100GB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW10GB100GBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW100GB1TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100GB - 1TB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW100GB1TBCnt ());
	  writeStat->append (sb.toString ());
	}
      if (fDataTotal->getW1TB10TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1TB - 10TB"));
	  writeStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getW1TB10TBCnt ());
	  writeStat->append (sb.toString ());
	}

      sb.sprintf (GTXT ("Longest write"));
      writeStat->append (sb.toString ());
      sb.sprintf (NTXT ("%.6f (secs.)"),
		  (double) (fDataTotal->getWSlowestBytes () / (double) NANOSEC));
      writeStat->append (sb.toString ());

      sb.sprintf (GTXT ("Smallest write bytes"));
      writeStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getWSmallestBytes ()));
      writeStat->append (sb.toString ());

      sb.sprintf (GTXT ("Largest write bytes"));
      writeStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getWLargestBytes ()));
      writeStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total time"));
      writeStat->append (sb.toString ());
      sb.sprintf (NTXT ("%.6f (secs.)"),
		  (double) (fDataTotal->getWriteTime () / (double) NANOSEC));
      writeStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total calls"));
      writeStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getWriteCnt ()));
      writeStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total bytes"));
      writeStat->append (sb.toString ());
      sb.sprintf (NTXT ("%lld"), (long long) (fDataTotal->getWriteBytes ()));
      writeStat->append (sb.toString ());
    }

  if (fDataTotal->getReadCnt () > 0)
    {
      if (fDataTotal->getR0KB1KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("0KB - 1KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR0KB1KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR1KB8KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1KB - 8KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR1KB8KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR8KB32KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("8KB - 32KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR8KB32KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR32KB128KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("32KB - 128KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR32KB128KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR128KB256KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("128KB - 256KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR128KB256KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR256KB512KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("256KB - 512KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR256KB512KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR512KB1000KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("512KB - 1000KB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR512KB1000KBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR1000KB10MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1000KB - 10MB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR1000KB10MBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR10MB100MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10MB - 100MB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR10MB100MBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR100MB1GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100MB - 1GB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR100MB1GBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR1GB10GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1GB - 10GB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR1GB10GBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR10GB100GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10GB - 100GB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR10GB100GBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR100GB1TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100GB - 1TB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR100GB1TBCnt ());
	  readStat->append (sb.toString ());
	}
      if (fDataTotal->getR1TB10TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1TB - 10TB"));
	  readStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), fDataTotal->getR1TB10TBCnt ());
	  readStat->append (sb.toString ());
	}

      sb.sprintf (GTXT ("Longest read"));
      readStat->append (sb.toString ());
      sb.sprintf (NTXT ("%.6f (secs.)"),
		  (double) (fDataTotal->getRSlowestBytes () / (double) NANOSEC));
      readStat->append (sb.toString ());

      sb.sprintf (GTXT ("Smallest read bytes"));
      readStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getRSmallestBytes ()));
      readStat->append (sb.toString ());

      sb.sprintf (GTXT ("Largest read bytes"));
      readStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getRLargestBytes ()));
      readStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total time"));
      readStat->append (sb.toString ());
      sb.sprintf (NTXT ("%.6f (secs.)"),
		  (double) (fDataTotal->getReadTime () / (double) NANOSEC));
      readStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total calls"));
      readStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getReadCnt ()));
      readStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total bytes"));
      readStat->append (sb.toString ());
      sb.sprintf (NTXT ("%lld"), (long long) (fDataTotal->getReadBytes ()));
      readStat->append (sb.toString ());
    }

  if (fDataTotal->getOtherCnt () > 0)
    {
      sb.sprintf (GTXT ("Total time"));
      otherStat->append (sb.toString ());
      sb.sprintf (NTXT ("%.6f (secs.)"),
		  (double) (fDataTotal->getOtherTime () / (double) NANOSEC));
      otherStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total calls"));
      otherStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getOtherCnt ()));
      otherStat->append (sb.toString ());
    }

  if (fDataTotal->getErrorCnt () > 0)
    {
      sb.sprintf (GTXT ("Total time"));
      errorStat->append (sb.toString ());
      sb.sprintf (NTXT ("%.6f (secs.)"),
		  (double) (fDataTotal->getErrorTime () / (double) NANOSEC));
      errorStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total calls"));
      errorStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (fDataTotal->getErrorCnt ()));
      errorStat->append (sb.toString ());
    }
  Vector<Vector<char*>*>* statisticsData = new Vector<Vector<char*>*>(4);
  statisticsData->store (0, writeStat);
  statisticsData->store (1, readStat);
  statisticsData->store (2, otherStat);
  statisticsData->store (3, errorStat);
  return statisticsData;
}

Vector<Vector<char*>*>*
dbeGetHeapStatistics (int dbevindex)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  Hist_data *hist_data;
  Hist_data::HistItem *hi;
  HeapData *hDataTotal;
  hist_data = dbev->heapcs_data;
  if (hist_data == NULL)
    return NULL;

  hi = hist_data->fetch (0);
  hDataTotal = (HeapData*) hi->obj;
  Vector<char*> *memoryUsage = new Vector<char*>;
  Vector<char*> *allocStat = new Vector<char*>;
  Vector<char*> *leakStat = new Vector<char*>;

  memoryUsage->append (dbe_strdup (GTXT ("Process With Highest Peak Memory Usage")));
  allocStat->append (dbe_strdup (GTXT ("Memory Allocations Statistics")));
  leakStat->append (dbe_strdup (GTXT ("Memory Leaks Statistics")));
  StringBuilder sb;
  if (hDataTotal->getPeakMemUsage () > 0)
    {
      sb.sprintf (GTXT ("Heap size bytes"));
      memoryUsage->append (sb.toString ());
      sb.sprintf (NTXT ("%lld"), (long long) (hDataTotal->getPeakMemUsage ()));
      memoryUsage->append (sb.toString ());

      sb.sprintf (GTXT ("Experiment Id"));
      memoryUsage->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getUserExpId ()));
      memoryUsage->append (sb.toString ());

      sb.sprintf (GTXT ("Process Id"));
      memoryUsage->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getPid ()));
      memoryUsage->append (sb.toString ());

      Vector<hrtime_t> *pTimestamps;
      pTimestamps = hDataTotal->getPeakTimestamps ();
      if (pTimestamps != NULL)
	{
	  for (int i = 0; i < pTimestamps->size (); i++)
	    {
	      sb.sprintf (GTXT ("Time of peak"));
	      memoryUsage->append (sb.toString ());
	      sb.sprintf (NTXT ("%.3f (secs.)"), (double) (pTimestamps->fetch (i) / (double) NANOSEC));
	      memoryUsage->append (sb.toString ());
	    }
	}
    }

  if (hDataTotal->getAllocCnt () > 0)
    {
      if (hDataTotal->getA0KB1KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("0KB - 1KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA0KB1KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA1KB8KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1KB - 8KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA1KB8KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA8KB32KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("8KB - 32KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA8KB32KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA32KB128KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("32KB - 128KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA32KB128KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA128KB256KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("128KB - 256KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA128KB256KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA256KB512KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("256KB - 512KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA256KB512KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA512KB1000KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("512KB - 1000KB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA512KB1000KBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA1000KB10MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1000KB - 10MB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA1000KB10MBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA10MB100MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10MB - 100MB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA10MB100MBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA100MB1GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100MB - 1GB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA100MB1GBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA1GB10GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1GB - 10GB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA1GB10GBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA10GB100GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10GB - 100GB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA10GB100GBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA100GB1TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100GB - 1TB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA100GB1TBCnt ());
	  allocStat->append (sb.toString ());
	}
      if (hDataTotal->getA1TB10TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1TB - 10TB"));
	  allocStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getA1TB10TBCnt ());
	  allocStat->append (sb.toString ());
	}

      sb.sprintf (GTXT ("Smallest allocation bytes"));
      allocStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getASmallestBytes ()));
      allocStat->append (sb.toString ());

      sb.sprintf (GTXT ("Largest allocation bytes"));
      allocStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getALargestBytes ()));
      allocStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total allocations"));
      allocStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getAllocCnt ()));
      allocStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total bytes"));
      allocStat->append (sb.toString ());
      sb.sprintf (NTXT ("%lld"), (long long) (hDataTotal->getAllocBytes ()));
      allocStat->append (sb.toString ());
    }

  if (hDataTotal->getLeakCnt () > 0)
    {
      if (hDataTotal->getL0KB1KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("0KB - 1KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL0KB1KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL1KB8KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1KB - 8KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL1KB8KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL8KB32KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("8KB - 32KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL8KB32KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL32KB128KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("32KB - 128KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL32KB128KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL128KB256KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("128KB - 256KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL128KB256KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL256KB512KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("256KB - 512KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL256KB512KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL512KB1000KBCnt () > 0)
	{
	  sb.sprintf (GTXT ("512KB - 1000KB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL512KB1000KBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL1000KB10MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1000KB - 10MB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL1000KB10MBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL10MB100MBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10MB - 100MB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL10MB100MBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL100MB1GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100MB - 1GB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL100MB1GBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL1GB10GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1GB - 10GB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL1GB10GBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL10GB100GBCnt () > 0)
	{
	  sb.sprintf (GTXT ("10GB - 100GB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL10GB100GBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL100GB1TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("100GB - 1TB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL100GB1TBCnt ());
	  leakStat->append (sb.toString ());
	}
      if (hDataTotal->getL1TB10TBCnt () > 0)
	{
	  sb.sprintf (GTXT ("1TB - 10TB"));
	  leakStat->append (sb.toString ());
	  sb.sprintf (NTXT ("%d"), hDataTotal->getL1TB10TBCnt ());
	  leakStat->append (sb.toString ());
	}

      sb.sprintf (GTXT ("Smallest leaked bytes"));
      leakStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getLSmallestBytes ()));
      leakStat->append (sb.toString ());

      sb.sprintf (GTXT ("Largest leaked bytes"));
      leakStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getLLargestBytes ()));
      leakStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total leaked"));
      leakStat->append (sb.toString ());
      sb.sprintf (NTXT ("%d"), (int) (hDataTotal->getLeakCnt ()));
      leakStat->append (sb.toString ());

      sb.sprintf (GTXT ("Total bytes"));
      leakStat->append (sb.toString ());
      sb.sprintf (NTXT ("%lld"), (long long) (hDataTotal->getLeakBytes ()));
      leakStat->append (sb.toString ());
    }
  Vector<Vector<char*>*>* statisticsData = new Vector<Vector<char*>*>(3);
  statisticsData->store (0, memoryUsage);
  statisticsData->store (1, allocStat);
  statisticsData->store (2, leakStat);
  return statisticsData;
}

Vector<char*> *
dbeGetFuncNames (int dbevindex, Vector<Obj> *funcs)
{
  int len = funcs->size ();
  Vector<char*> *list = new Vector<char*>(len);
  for (int i = 0; i < len; i++)
    list->store (i, dbeGetFuncName (dbevindex, funcs->fetch (i))); // no strdup()
  return list;
}

Vector<char*> *
dbeGetObjNamesV2 (int dbevindex, Vector<uint64_t> *ids)
{
  int len = ids->size ();
  Vector<char*> *list = new Vector<char*>(len);
  for (int i = 0; i < len; i++)
    list->store (i, dbeGetObjNameV2 (dbevindex, ids->fetch (i))); // no strdup()
  return list;
}

char *
dbeGetFuncName (int dbevindex, Obj func)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (func == 0)
    return NULL;
  char *fname;
  fname = ((Histable *) func)->get_name (dbev->get_name_format ());
  return fname ? dbe_strdup (fname) : NULL;
}

Vector<uint64_t> *
dbeGetFuncIds (int dbevindex, Vector<Obj> *funcs)
{
  int len = funcs->size ();
  Vector<uint64_t> *list = new Vector<uint64_t>(len);
  for (int i = 0; i < len; i++)
    list->store (i, dbeGetFuncId (dbevindex, funcs->fetch (i)));
  return list;
}

uint64_t
dbeGetFuncId (int dbevindex, Obj func)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  if (func == 0)
    return 0;
  uint64_t id = ((Histable *) func)->id;
  return id;
}

char *
dbeGetObjNameV2 (int dbevindex, uint64_t id)
{
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Histable *obj = dbeSession->findObjectById (id);
  if (obj == NULL)
    return NULL;
  char *fname = obj->get_name (dbev->get_name_format ());
  return fname ? dbe_strdup (fname) : NULL;
}

char *
dbeGetDataspaceTypeDesc (int /*dbevindex*/, Obj stack)
{
  if (stack == 0)
    return NULL;
  Histable *hist = CallStack::getStackPC ((void *) stack, 0);
  DbeInstr *instr;
  Histable::Type type = hist->get_type ();
  if (type != Histable::INSTR)
    return NULL;
  else
    instr = (DbeInstr *) hist;
  char *descriptor = instr->get_descriptor ();
  return descriptor ? dbe_strdup (descriptor) : NULL;
}

Vector<void*> *
dbeGetDataDescriptorsV2 (int exp_id)
{
  Experiment *exp = dbeSession->get_exp (exp_id);
  if (exp == NULL)
    return NULL;
  Vector<int> *dataId = new Vector<int>;
  Vector<char*> *dataName = new Vector<char*>;
  Vector<char*> *dataUName = new Vector<char*>;
  Vector<int> *auxProp = new Vector<int>;
  Vector<DataDescriptor*> *ddscr = exp->getDataDescriptors ();
  for (int i = 0; i < ddscr->size (); i++)
    {
      DataDescriptor *dataDscr = ddscr->fetch (i);
      if (dataDscr->getFlags () & DDFLAG_NOSHOW)
	continue;
      int data_id = dataDscr->getId ();
      int aux_prop_id = (data_id == DATA_HWC) ? PROP_HWCTAG : PROP_NONE;
      dataId->append (data_id);
      dataName->append (strdup (dataDscr->getName ()));
      dataUName->append (strdup (dataDscr->getUName ()));
      auxProp->append (aux_prop_id);
    }
  delete ddscr;
  Vector<void*> *res = new Vector<void*>(3);
  res->store (0, dataId);
  res->store (1, dataName);
  res->store (2, dataUName);
  res->store (3, auxProp);
  return res;
}

Vector<void*> *
dbeGetDataPropertiesV2 (int exp_id, int data_id)
{
  Experiment *exp = dbeSession->get_exp (exp_id);
  if (exp == NULL)
    return NULL;
  DataDescriptor *dataDscr = exp->get_raw_events (data_id);
  if (dataDscr == NULL)
    return NULL;
  Vector<PropDescr*> *props = dataDscr->getProps ();
  Vector<int> *propId = new Vector<int>(props->size ());
  Vector<char*> *propUName = new Vector<char*>(props->size ());
  Vector<int> *propTypeId = new Vector<int>(props->size ());
  Vector<char*> *propTypeName = new Vector<char*>(props->size ());
  Vector<int> *propFlags = new Vector<int>(props->size ());
  Vector<char*> *propName = new Vector<char*>(props->size ());
  Vector<void*> *propStateNames = new Vector<void*>(props->size ());
  Vector<void*> *propStateUNames = new Vector<void*>(props->size ());

  for (int i = 0; i < props->size (); i++)
    {
      PropDescr *prop = props->fetch (i);
      char *pname = prop->name;
      if (pname == NULL)
	pname = NTXT ("");
      char *uname = prop->uname;
      if (uname == NULL)
	uname = pname;
      int vtypeNum = prop->vtype;
      if (vtypeNum < 0 || vtypeNum >= TYPE_LAST)
	vtypeNum = TYPE_NONE;
      const char * vtypeNames[] = VTYPE_TYPE_NAMES;
      const char *vtype = vtypeNames[prop->vtype];
      Vector<char*> *stateNames = NULL;
      Vector<char*> *stateUNames = NULL;
      int nStates = prop->getMaxState ();
      if (nStates > 0)
	{
	  stateNames = new Vector<char*>(nStates);
	  stateUNames = new Vector<char*>(nStates);
	  for (int kk = 0; kk < nStates; kk++)
	    {
	      const char * stateName = prop->getStateName (kk);
	      stateNames->store (kk, dbe_strdup (stateName));
	      const char * Uname = prop->getStateUName (kk);
	      stateUNames->store (kk, dbe_strdup (Uname));
	    }
	}
      propId->store (i, prop->propID);
      propUName->store (i, dbe_strdup (uname));
      propTypeId->store (i, prop->vtype);
      propTypeName->store (i, dbe_strdup (vtype));
      propFlags->store (i, prop->flags);
      propName->store (i, dbe_strdup (pname));
      propStateNames->store (i, stateNames);
      propStateUNames->store (i, stateUNames);
    }
  Vector<void*> *res = new Vector<void*>(7);
  res->store (0, propId);
  res->store (1, propUName);
  res->store (2, propTypeId);
  res->store (3, propTypeName);
  res->store (4, propFlags);
  res->store (5, propName);
  res->store (6, propStateNames);
  res->store (7, propStateUNames);
  return res;
}

Vector<void *> *
dbeGetExperimentTimeInfo (Vector<int> *exp_ids)
{
  int sz = exp_ids->size ();
  Vector<long long> *offset_time = new Vector<long long> (sz);
  Vector<long long> *start_time = new Vector<long long> (sz);
  Vector<long long> *end_time = new Vector<long long> (sz);
  Vector<long long> *start_wall_sec = new Vector<long long> (sz);
  Vector<char* > *hostname = new Vector<char*> (sz);
  Vector<int> *cpu_freq = new Vector<int> (sz);
  for (int ii = 0; ii < sz; ii++)
    {
      int expIdx = exp_ids->fetch (ii);
      { // update end_time by forcing fetch of experiment data
	// workaround until dbeGetEndTime() is more robust
	int id = (expIdx < 0) ? 0 : expIdx;
	Experiment *exp = dbeSession->get_exp (id);
	if (exp)
	  {
	    Vector<DataDescriptor*> *ddscr = exp->getDataDescriptors ();
	    delete ddscr;
	  }
      }
      offset_time->store (ii, dbeGetRelativeStartTime (0, expIdx));
      start_time->store (ii, dbeGetStartTime (0, expIdx));
      end_time->store (ii, dbeGetEndTime (0, expIdx));
      start_wall_sec->store (ii, dbeGetWallStartSec (0, expIdx));
      hostname->store (ii, dbeGetHostname (0, expIdx));
      cpu_freq->store (ii, dbeGetClock (0, expIdx));
    }
  Vector<void*> *res = new Vector<void*>(4);
  res->store (0, offset_time);
  res->store (1, start_time);
  res->store (2, end_time);
  res->store (3, start_wall_sec);
  res->store (4, hostname);
  res->store (5, cpu_freq);
  return res;
}

Vector<void *> *
dbeGetExperimentDataDescriptors (Vector<int> *exp_ids)
{
  int sz = exp_ids->size ();
  Vector<void*> *exp_dscr_info = new Vector<void*> (sz);
  Vector<void*> *exp_dscr_props = new Vector<void*> (sz);

  for (int ii = 0; ii < sz; ii++)
    {
      int expIdx = exp_ids->fetch (ii);
      Vector<void*> *ddscrInfo = dbeGetDataDescriptorsV2 (expIdx);
      Vector<void*> *ddscrProps = new Vector<void*> (); // one entry per ddscrInfo
      if (ddscrInfo)
	{
	  Vector<int> *dataId = (Vector<int>*)ddscrInfo->fetch (0);
	  if (dataId)
	    {
	      // loop thru data descriptors
	      int ndata = dataId->size ();
	      for (int j = 0; j < ndata; ++j)
		{
		  Vector<void*> *props = dbeGetDataPropertiesV2 (expIdx, dataId->fetch (j));
		  ddscrProps->store (j, props);
		}
	    }
	}
      exp_dscr_info->store (ii, ddscrInfo);
      exp_dscr_props->store (ii, ddscrProps);
    }
  Vector<void*> *res = new Vector<void*>(2);
  res->store (0, exp_dscr_info);
  res->store (1, exp_dscr_props);
  return res;
}

static Vector<void *> *
dbeGetTLDataRepVals (VMode view_mode, hrtime_t start_ts, hrtime_t delta,
		     int numDeltas, DataView*packets,
		     Vector<long> *representativeEvents, bool showDuration);

static bool
dbeHasTLData (int dbevindex, int exp_id, int data_id, int entity_prop_id,
	      int entity_prop_value, int aux)
{
  DataView *packets =
	  getTimelinePackets (dbevindex, exp_id, data_id, entity_prop_id);
  if (!packets || packets->getSize () == 0)
    return false;
  long start_ind = getIdxByVals (packets, aux, entity_prop_value,
				 0, DataView::REL_GTEQ); // time >= 0
  if (start_ind < 0)
    return false;

  DbeView *dbev = dbeSession->getView (dbevindex);
  VMode view_mode = dbev->get_view_mode ();
  Experiment *exp = dbeSession->get_exp (exp_id);
  if (!hasInvisbleTLEvents (exp, view_mode))
    return true; // all events are visible, no further checking required
  long end_ind = getIdxByVals (packets, aux, entity_prop_value,
			       MAX_TIME, DataView::REL_LTEQ);
  for (long ii = start_ind; ii <= end_ind; ii++)
    {
      if (!isVisibleTLEvent (exp, view_mode, packets, ii))
	continue;
      return true; // first visible packet => has data
    }
  return false;
}

Vector<bool> *
dbeHasTLData (int dbev_index, Vector<int> *exp_ids, Vector<int> *data_ids,
	      Vector<int> *entity_prop_ids, // LWP,CPU,THR, etc
	      Vector<int> *entity_prop_values, Vector<int> *auxs)
{
  DbeView *dbev = dbeSession->getView (dbev_index);
  if (!dbev->isShowAll () && (dbev->isShowHideChanged ()
			      || dbev->isNewViewMode ()))
    {
      // LIBRARY_VISIBILITY
      dbev->resetAndConstructShowHideStacks ();
      if (dbev->isNewViewMode ())
	dbev->resetNewViewMode ();
      if (dbev->isShowHideChanged ())
	dbev->resetShowHideChanged ();
    }

  int sz = exp_ids->size ();
  Vector<bool> *hasVec = new Vector<bool>(sz);
  for (int ii = 0; ii < sz; ii++)
    {
      bool hasData = dbeHasTLData (dbev_index, exp_ids->fetch (ii),
				   data_ids->fetch (ii),
				   entity_prop_ids->fetch (ii),
				   entity_prop_values->fetch (ii),
				   auxs->fetch (ii));
      hasVec->store (ii, hasData);
    }
  return hasVec;
}

/*
 *   dbeGetTLData implements:
 *   FROM data_id
 *     DURATION >= delta AND ( start_ts <= TSTAMP < start_ts+num*delta OR
 *                             start_ts <= TSTAMP-DURATION < start_ts+num*delta )
 *     OR
 *     FAIR( DURATION < delta AND ( start_ts <= TSTAMP < start_ts+num*delta ) )
 *     WHERE lfilter
 */

Vector<void *> *
dbeGetTLData (
	      int dbevindex,
	      int exp_id,
	      int data_id, // DATA_*
	      int entity_prop_id, // Show PROP_LWPID, PROP_CPUID, PROP_THRID, PROP_EXPID, or N/A
	      int entity_prop_value, // which LWPID, CPUID, THRID, EXPID for this request
	      int aux,
	      hrtime_t param_start_ts,
	      hrtime_t param_delta,
	      int param_numDeltas,
	      bool getRepresentatives, // fetch TL representatives
	      Vector<char *> *chartProps) // calculate sums for these property vals
{
  const hrtime_t start_ts = param_start_ts;
  const hrtime_t delta = param_delta;
  const int numDeltas = param_numDeltas;
  DbeView *dbev = dbeSession->getView (dbevindex);
  if (dbev == NULL)
    abort ();
  Experiment *exp = dbeSession->get_exp (exp_id);
  if (exp == NULL)
    return NULL;
  if (getRepresentatives == false && chartProps == NULL)
    return NULL;
  if (delta <= 0)
    return NULL;

  hrtime_t tmp_ts = start_ts + delta * numDeltas;
  if (tmp_ts < start_ts)
    tmp_ts = MAX_TIME;
  const hrtime_t end_ts = tmp_ts;
  if (exp->get_status () == Experiment::INCOMPLETE &&
      exp->getLastEvent () < end_ts)
    exp->update ();
  DataView *packets =
	  getTimelinePackets (dbevindex, exp_id, data_id, entity_prop_id);
  if (packets == NULL)
    return NULL; // strange, no data view?

  VMode view_mode = dbev->get_view_mode (); // user, expert, machine //YXXX yuck

  // storage for calculating timeline representative events
  Vector<long> *representativeEvents = NULL;
  // list of representative events to be displayed on TL
  Vector<int> *binRepIdx = NULL;
  // for each bin, index    of current "best" representativeEvent
  Vector<void*> *representativeVals = NULL;
  // TL representative packets' values

  // storage for calculating charts
  Vector<int> *propIds = NULL; // [propIdx], which prop to measure
  Vector<void*> *propVals = NULL; // [propIdx][bin], prop vals
  Vector<int> *propNumStates = NULL; // [propIdx], how many states for prop?
  Vector<bool> *propCumulativeChart = NULL; // [propIdx], data represents cumulative totals
  Vector<long long> *propCumulativeRecentBinLastVal = NULL; // [propIdx], most recent value
  Vector<long long> *propCumulativeRecentBinHighVal = NULL; // [propIdx], highest value for propCumulativeRecentBin
  Vector<int> *propCumulativeRecentBin = NULL; // [propIdx], most recent bin

  // determine when to show duration of events
  bool tmp_repsShowDuration = false;
  bool tmp_statesUseDuration = false;
  bool tmp_extendMicrostates = false;
  const hrtime_t ptimerTickDuration = exp->get_params ()->ptimer_usec * 1000LL; // nanoseconds per tick
  const bool hasDuration = packets->getProp (PROP_EVT_TIME) ? true : false;
  if (hasDuration)
    {
      switch (entity_prop_id)
	{
	case PROP_CPUID:
	  tmp_repsShowDuration = false;
	  tmp_statesUseDuration = false;
	  break;
	case PROP_THRID:
	case PROP_LWPID:
	  tmp_repsShowDuration = true;
	  tmp_statesUseDuration = true;
	  tmp_extendMicrostates = (DATA_CLOCK == data_id) && (ptimerTickDuration < param_delta);
	  break;
	case PROP_EXPID:
	case PROP_NONE: // experiment summary row uses this
	default:
	  if (DATA_SAMPLE == data_id)
	    {
	      tmp_repsShowDuration = true;
	      tmp_statesUseDuration = true;
	    }
	  else if (DATA_GCEVENT == data_id)
	    {
	      tmp_repsShowDuration = true;
	      tmp_statesUseDuration = true;
	    }
	  else if (DATA_CLOCK == data_id)
	    {
	      tmp_repsShowDuration = false;
	      tmp_statesUseDuration = true;
	      tmp_extendMicrostates = true;
	    }
	  else
	    {
	      tmp_repsShowDuration = false;
	      tmp_statesUseDuration = true;
	    }
	  break;
	}
    }
  const bool repsShowDuration = tmp_repsShowDuration; // show stretched callstacks
  const bool statesUseDuration = tmp_statesUseDuration; // use duration to calculate state charts
  const bool extendMicrostates = tmp_extendMicrostates; // we show discrete profiling microstates with
  // width=(tick-1), but for computing
  // zoomed-out graphs we need to extend to
  // account for all ticks, width=(ntick)
  const bool reverseScan = repsShowDuration || extendMicrostates; // scan packets in reverse

  // determine range of packet indices (lo_pkt_idx, hi_pkt_idx)
  long lo_pkt_idx, hi_pkt_idx;
  if (extendMicrostates && !(entity_prop_id == PROP_THRID || entity_prop_id == PROP_LWPID))
    {
      // merging data from multiple threads, need to scan all packets with timestamp [start_ts, exp end]
      hrtime_t exp_end_time = exp->getLastEvent () + 1;
      hi_pkt_idx = getIdxByVals (packets, aux, entity_prop_value,
				 exp_end_time, DataView::REL_LT); // last item
    }
  else
    hi_pkt_idx = getIdxByVals (packets, aux, entity_prop_value,
			       end_ts, DataView::REL_LT);
  if (repsShowDuration)
    {
      // There are two issues to deal with
      // 1. events that end "off screen" to the right
      // 2. overlapping events

      // 1. events that end "off screen" to the right
      // For now, we only consistently handle the case where events don't overlap.
      // Note that packet timestamps mark end of duration, not start.
      // This means that the rightmost event won't be within hi_pkt_idx.
      // Solution: Check if end+1 packet _started_ in-range
      // Caveat: because we only look ahead by one packet, if there are
      // overlapping duration events (e.g. EXPID aggregation)), zoom level
      // and panning combo may cause events with TSTAMP>end_ts
      // to appear/disappear.  A complete solution would involve
      // a solution to 2.

      // 2. overlapping events
      // For now, we have a simplistic solution that makes "wide" events win.  However,
      // a future solution for deterministically dealing with overlap might look like this:
      // - find all packets that touch the visible time range
      //   - possibly use two DataViews: one with TSTAMP_HI sort and one with TSTAMP_LO
      //     sort to allow efficient determination of packets with HI and LO endpoints in-range
      // - create buckets to  capture "winning" event for each bin (each pixel, that is)
      // - sort the new list of packets by TSTAMP_HI (for example)
      // - looping thru the packets that are in-range, update every bin it touches with it's id
      // - if there is overlap, earlier packets will be kicked out of bins
      // - On the GUI side, paint one event at a time, as normal.
      // - However, for selections, recognize that duration of event may span many bins
      //
      long idx;
      if (hi_pkt_idx >= 0)
	// a packet was found to the left of the end time
	idx = hi_pkt_idx + 1; // attempt to go one packet right
      else
	idx = getIdxByVals (packets, aux, entity_prop_value,
			    end_ts, DataView::REL_GTEQ);
      if (isValidIdx (packets, entity_prop_id, aux, entity_prop_value, idx))
	{
	  int64_t pkt_ts = packets->getLongValue (PROP_TSTAMP, idx);
	  int64_t duration = packets->getLongValue (PROP_EVT_TIME, idx);
	  pkt_ts -= duration;
	  if (pkt_ts < end_ts)
	    hi_pkt_idx = idx;
	}
    }
  lo_pkt_idx = getIdxByVals (packets, aux, entity_prop_value,
			     start_ts, DataView::REL_GTEQ);

  // allocate structs that return chart data
  bool hasCumulativeCharts = false;
  if (chartProps && chartProps->size () > 0)
    {
      int nprops = chartProps->size ();
      // pre-allocate storage
      propIds = new Vector<int> (nprops);
      propVals = new Vector<void*>(nprops);
      propNumStates = new Vector<int> (nprops);
      propCumulativeChart = new Vector<bool>(nprops);
      propCumulativeRecentBinLastVal = new Vector<long long>(nprops);
      propCumulativeRecentBinHighVal = new Vector<long long>(nprops);
      propCumulativeRecentBin = new Vector<int>(nprops);
      for (int propNum = 0; propNum < nprops; propNum++)
	{
	  const char* propStr = chartProps->fetch (propNum);
	  int items_per_prop = 0;
	  int prop_id = PROP_NONE;
	  if (!strcmp (propStr, "EVT_COUNT"))
	    items_per_prop = 1; // use PROP_NONE for counting packets
	  else
	    {
	      int lookup_prop_id = dbeSession->getPropIdByName (propStr);
	      PropDescr *propDscr = packets->getProp (lookup_prop_id);
	      if (propDscr != NULL)
		{
		  switch (propDscr->vtype)
		    {
		    case TYPE_INT32:
		    case TYPE_UINT32:
		    case TYPE_INT64:
		    case TYPE_UINT64:
		      items_per_prop = propDscr->getMaxState () + 1;
		      // add extra slot to store values with out-of-range idx
		      prop_id = lookup_prop_id;
		      break;
		    case TYPE_DOUBLE:
		      break; // not implemented yet
		    case TYPE_STRING:
		    case TYPE_OBJ:
		    case TYPE_DATE:
		    default:
		      break;
		    }
		}
	    }
	  void *vals;
	  if (!items_per_prop)
	    vals = NULL;
	  else if (items_per_prop == 1)
	    {
	      Vector<long long> *longVals = new Vector<long long> ();
	      longVals->store (numDeltas - 1, 0); // initialize all elements
	      vals = longVals;
	    }
	  else
	    {
	      Vector<Vector<long long>*> *stateVals =
		      new Vector<Vector<long long>*> ();
	      vals = stateVals;
	      // initialize only on-demand, some may not be needed
	    }

	  bool isCumulativeChart;
#define YXXX_HEAP_VS_TIME 1 // YXXX add data meaning to properties?
#if YXXX_HEAP_VS_TIME
	  isCumulativeChart = (prop_id == PROP_HCUR_LEAKS || prop_id == PROP_HCUR_ALLOCS);
#endif
	  if (isCumulativeChart)
	    hasCumulativeCharts = true;
	  propIds->store (propNum, prop_id);
	  propVals->store (propNum, vals);
	  propNumStates->store (propNum, items_per_prop);
	  propCumulativeRecentBinLastVal->store (propNum, 0);
	  propCumulativeRecentBinHighVal->store (propNum, 0);
	  propCumulativeRecentBin->store (propNum, 0);
	  propCumulativeChart->store (propNum, isCumulativeChart);
	}
    }

  // Adjust idx range for calculating 'cumulative charts' e.g. heap size
  if (hasCumulativeCharts)
    {
      // set initial values if earlier packet exists
      long lo_idx;
      if (lo_pkt_idx >= 0)
	// packet was found to the right of start
	lo_idx = lo_pkt_idx - 1; // attempt to go left by one event
      else
	// no packet was to the right of start, look left of start
	lo_idx = getIdxByVals (packets, aux, entity_prop_value,
			       start_ts, DataView::REL_LT);
      if (isValidIdx (packets, entity_prop_id, aux, entity_prop_value, lo_idx))
	{
	  // preceding packet found
	  // update initial values
	  int nprops = propCumulativeChart->size ();
	  for (int propNum = 0; propNum < nprops; propNum++)
	    {
	      if (!propCumulativeChart->fetch (propNum))
		continue;
	      int propId = propIds->fetch (propNum);
	      long long value = packets->getLongValue (propId, lo_idx);
	      propCumulativeRecentBinLastVal->store (propNum, value);
	      propCumulativeRecentBinHighVal->store (propNum, value);
	    }
	  // update indices used for iterating
	  lo_pkt_idx = lo_idx;
	  if (hi_pkt_idx < lo_pkt_idx)
	    hi_pkt_idx = lo_pkt_idx;
	}
    }
  if (lo_pkt_idx < 0 || hi_pkt_idx < 0)
    goto dbeGetTLData_done; // no data; return empty vectors, not null

  // representative events (subset of callstacks to represent on TL)
  if (getRepresentatives)
    {
      representativeEvents = new Vector<long>(numDeltas);
      // per-bin, longest event's index
      binRepIdx = new Vector<int>(numDeltas);
      for (int ii = 0; ii < numDeltas; ++ii)
	binRepIdx->append (-1);
    }
  // While packets are sorted by _end_ timestamp (TSTAMP),
  // after calculating start times for non-zero durations,
  // start times are not guaranteed be monotonically increasing.
  // For packets with duration, we'll scan them in reverse order to
  // take advantage of the monotonically decreasing _end_ timestamps.
  long start_idx, idx_inc;
  if (!reverseScan)
    {
      start_idx = lo_pkt_idx;
      idx_inc = 1;
    }
  else
    {
      start_idx = hi_pkt_idx;
      idx_inc = -1;
    }
  for (long ii = start_idx; ii >= lo_pkt_idx && ii <= hi_pkt_idx; ii += idx_inc)
    {
      if (!isVisibleTLEvent (exp, view_mode, packets, ii) && !hasCumulativeCharts)
	continue;

      // determine packet time duration and start bin
      int tmp_start_bin; // packet start bin
      int tmp_end_bin; // packet end bin (inclusive)
      const hrtime_t pkt_end_ts = packets->getLongValue (PROP_TSTAMP, ii);
      const hrtime_t pkt_dur = packets->getLongValue (PROP_EVT_TIME, ii);
      const hrtime_t pkt_start_ts = pkt_end_ts - pkt_dur;
      if (pkt_end_ts < start_ts && !hasCumulativeCharts)
	continue; // weird, should not happen
      if (pkt_start_ts >= end_ts)
	continue; // could happen
      hrtime_t bin_end_ts = pkt_end_ts;
      if (bin_end_ts >= end_ts)
	bin_end_ts = end_ts - 1;
      tmp_end_bin = (int) ((bin_end_ts - start_ts) / delta);
      hrtime_t bin_start_ts = pkt_start_ts;
      if (bin_start_ts < start_ts)
	bin_start_ts = start_ts; // event truncated to left.
      tmp_start_bin = (int) ((bin_start_ts - start_ts) / delta);
      // By definition
      //   (end_ts - start_ts) == delta * numDeltas
      // and we know
      //   pkt_start < end_ts
      // therefore
      //   (pkt_start - start_ts) < delta * numDeltas
      //   (pkt_start - start_ts) / delta < numDeltas
      //   bin < numDeltas
      assert (tmp_end_bin < numDeltas);
      assert (tmp_start_bin < numDeltas);
      const bool is_offscreen = tmp_end_bin < 0 ? true : false;
      if (tmp_end_bin < 0)
	tmp_end_bin = 0;
      const int pkt_end_bin = tmp_end_bin; // packet end bin (inclusive)
      const int pkt_start_bin = tmp_start_bin;
      if (getRepresentatives && !is_offscreen)
	{ // find best representative
	  // Note: for events with duration, we're scanning packets in order
	  // of decreasing end-timestamp.  This means that the first packet
	  // that hits a particular _start_ bin will have the longest duration
	  // of any later packet that might hit that start bin.  The
	  // the first packet will be the best (longest) packet.
	  const int bin = reverseScan ? pkt_start_bin : pkt_end_bin;
	  int eventIdx = binRepIdx->fetch (bin);
	  if (eventIdx == -1)
	    {
	      eventIdx = representativeEvents->size (); // append to end
	      representativeEvents->append (ii);
	      binRepIdx->store (bin, eventIdx);
	    }
	}
      if (propIds)
	{ // per-bin chart: sum across filtered packets
	  for (int propNum = 0; propNum < propIds->size (); propNum++)
	    {
	      void *thisProp = propVals->fetch (propNum);
	      if (thisProp == NULL)
		continue; // no valid data
	      if (is_offscreen && !propCumulativeChart->fetch (propNum))
		continue; // offscreen events are only processed for cumulative charts
	      int propId = propIds->fetch (propNum);
	      long long val;
	      if (propId == PROP_NONE)
		val = 1; // count
	      else
		val = packets->getLongValue (propId, ii);
	      long nitems = propNumStates->fetch (propNum);
	      if (nitems < 1)
		continue;
	      else if (nitems == 1)
		{
		  // chart is not based on not multiple states
		  Vector<long long>* thisPropVals =
			  (Vector<long long>*)thisProp;
		  if (thisPropVals->size () == 0)
		    thisPropVals->store (numDeltas - 1, 0);
		  const int bin = statesUseDuration ? pkt_start_bin : pkt_end_bin;
		  if (!propCumulativeChart->fetch (propNum))
		    {
		      val += thisPropVals->fetch (bin);
		      thisPropVals->store (bin, val);
		    }
		  else
		    {
		      // propCumulativeChart
		      long long high_value = propCumulativeRecentBinHighVal->fetch (propNum);
		      int last_bin = propCumulativeRecentBin->fetch (propNum);
		      if (last_bin < bin)
			{
			  // backfill from previous event
			  // last_bin: store largest value (in case of multiple events)
			  thisPropVals->store (last_bin, high_value);
			  // propagate forward the bin's last value
			  long long last_value = propCumulativeRecentBinLastVal->fetch (propNum);
			  for (int kk = last_bin + 1; kk < bin; kk++)
			    thisPropVals->store (kk, last_value);
			  // prepare new bin for current event
			  high_value = 0; // high value of next bin is 0.
			  propCumulativeRecentBinHighVal->store (propNum, high_value);
			  propCumulativeRecentBin->store (propNum, bin);
			}
		      long long this_value = packets->getLongValue (propId, ii);
		      propCumulativeRecentBinLastVal->store (propNum, this_value);
		      if (high_value < this_value)
			{
			  // record the max
			  high_value = this_value;
			  propCumulativeRecentBinHighVal->store (propNum, high_value);
			}
		      if (ii == hi_pkt_idx)
			{
			  // bin: show largest value (in case of multiple events
			  thisPropVals->store (bin, high_value);
			  //forward fill remaining bins
			  for (int kk = bin + 1; kk < numDeltas; kk++)
			    thisPropVals->store (kk, this_value);
			}
		    }
		}
	      else
		{
		  // means val is actually a state #
		  Vector<Vector<long long>*>* thisPropStateVals =
			  (Vector<Vector<long long>*>*)thisProp;
		  if (thisPropStateVals->size () == 0)
		    thisPropStateVals->store (numDeltas - 1, 0);
		  long stateNum;
		  if (val >= 0 && val < nitems)
		    stateNum = (long) val;
		  else
		    stateNum = nitems - 1; // out of range, use last slot
		  hrtime_t graph_pkt_dur = pkt_dur;
		  hrtime_t graph_pkt_start_ts = pkt_start_ts;
		  int tmp2_start_bin = pkt_start_bin;
		  if (propId == PROP_MSTATE)
		    {
		      if (statesUseDuration && extendMicrostates)
			{
			  // microstate stacks are shown and filtered with width=NTICK-1
			  // but for microstate graph calcs use width=NTICK.
			  graph_pkt_dur += ptimerTickDuration;
			  graph_pkt_start_ts -= ptimerTickDuration;
			  hrtime_t bin_start_ts = graph_pkt_start_ts;
			  if (bin_start_ts < start_ts)
			    bin_start_ts = start_ts; // event truncated to left.
			  tmp2_start_bin = (int) ((bin_start_ts - start_ts) / delta);
			}
		    }
		  const int graph_pkt_start_bin = statesUseDuration ? tmp2_start_bin : pkt_end_bin;

		  // We will distribute the state's presence evenly over duration of the event.
		  // When only a 'partial bin' is touched by an event, adjust accordingly.
		  long long value_per_bin; // weight to be applied to each bin
		  {
		    long long weight;
		    if (propId == PROP_MSTATE)  // ticks to nanoseconds
		      weight = packets->getLongValue (PROP_NTICK, ii) * ptimerTickDuration;
		    else if (graph_pkt_dur)
		      weight = graph_pkt_dur; // nanoseconds
		    else
		      weight = 1; // no duration; indicate presence
		    if (graph_pkt_start_bin != pkt_end_bin)
		      {
			// spans multiple bins
			double nbins = (double) graph_pkt_dur / delta;
			value_per_bin = weight / nbins;
		      }
		    else
		      value_per_bin = weight;
		  }
		  for (int evtbin = graph_pkt_start_bin; evtbin <= pkt_end_bin; evtbin++)
		    {
		      Vector<long long>* stateValues =
			      (Vector<long long>*) thisPropStateVals->fetch (evtbin);
		      if (stateValues == NULL)
			{
			  // on-demand storage
			  stateValues = new Vector<long long>(nitems);
			  stateValues->store (nitems - 1, 0); // force memset of full vector
			  thisPropStateVals->store (evtbin, stateValues);
			}
		      long long new_val = stateValues->fetch (stateNum);
		      if (graph_pkt_start_bin == pkt_end_bin ||
			  (evtbin > graph_pkt_start_bin && evtbin < pkt_end_bin))
			{
			  new_val += value_per_bin;
			}
		      else
			{
			  // partial bin
			  const hrtime_t bin_start = start_ts + evtbin * delta;
			  const hrtime_t bin_end = start_ts + (evtbin + 1) * delta - 1;
			  if (evtbin == graph_pkt_start_bin)
			    {
			      // leftmost bin
			      if (graph_pkt_start_ts < bin_start)
				new_val += value_per_bin;
			      else
				{
				  double percent = (double) (bin_end - graph_pkt_start_ts) / delta;
				  new_val += value_per_bin*percent;
				}
			    }
			  else
			    {
			      // rightmost bin
			      if (pkt_end_ts > bin_end)
				new_val += value_per_bin;
			      else
				{
				  double percent = (double) (pkt_end_ts - bin_start) / delta;
				  new_val += value_per_bin*percent;
				}
			    }
			}
		      stateValues->store (stateNum, new_val);
		    }
		}
	    }
	}
    }
  delete binRepIdx;
  delete propIds;
  delete propCumulativeChart;
  delete propCumulativeRecentBinLastVal;
  delete propCumulativeRecentBinHighVal;
  delete propCumulativeRecentBin;
  if (representativeEvents != NULL && reverseScan)
    {
      if (repsShowDuration)
	{
	  //YXXX for now prune here, but in the future, let gui decide what to show
	  // Prune events that are completely obscured long duration events.
	  // Note: representativeEvents is sorted by decreasing _end_ timestamps.
	  Vector<long> *prunedEvents = new Vector<long>(numDeltas);
	  hrtime_t prev_start_ts = MAX_TIME;
	  long repCnt = representativeEvents->size ();
	  for (long kk = 0; kk < repCnt; kk++)
	    {
	      long ii = representativeEvents->fetch (kk);
	      hrtime_t tmp_end_ts = packets->getLongValue (PROP_TSTAMP, ii);
	      hrtime_t tmp_dur = packets->getLongValue (PROP_EVT_TIME, ii);
	      hrtime_t tmp_start_ts = tmp_end_ts - tmp_dur;
	      if (tmp_start_ts >= prev_start_ts)
		// this event would be completely hidden
		// (because of sorting, we know tmp_end_ts <= prev_end_ts)
		continue;
	      prev_start_ts = tmp_start_ts;
	      prunedEvents->append (ii);
	    }
	  // invert order to to get increasing _end_ timestamps
	  representativeEvents->reset ();
	  for (long kk = prunedEvents->size () - 1; kk >= 0; kk--)
	    {
	      long packet_idx = prunedEvents->fetch (kk);
	      representativeEvents->append (packet_idx);
	    }
	  delete prunedEvents;
	}
      else
	{ // !repsShowDuration
	  // Note: representativeEvents is sorted by decreasing _end_ timestamps.
	  // Reverse the order:
	  long hi_idx = representativeEvents->size () - 1;
	  long lo_idx = 0;
	  while (hi_idx > lo_idx)
	    {
	      // swap
	      long lo = representativeEvents->fetch (lo_idx);
	      long hi = representativeEvents->fetch (hi_idx);
	      representativeEvents->store (lo_idx, hi);
	      representativeEvents->store (hi_idx, lo);
	      hi_idx--;
	      lo_idx++;
	    }
	}
    }

dbeGetTLData_done:
  if (getRepresentatives)
    {
      representativeVals = dbeGetTLDataRepVals (view_mode, start_ts, delta,
		    numDeltas, packets, representativeEvents, repsShowDuration);
      delete representativeEvents;
    }
  Vector<void*> *results = new Vector<void*> (2);
  results->store (0, representativeVals);
  results->store (1, propVals);
  return results;
}

// add representative events to return buffer

static Vector<void *> *
dbeGetTLDataRepVals (VMode view_mode, hrtime_t start_ts, hrtime_t delta,
		     int numDeltas, DataView*packets,
		     Vector<long> *representativeEvents, bool showDuration)
{
  int numrecs = representativeEvents ? representativeEvents->size () : 0;
  // allocate storage for results
  Vector<int> *startBins = new Vector<int>(numrecs);
  Vector<int> *numBins = new Vector<int>(numrecs);
  Vector<Obj> *eventIdxs = new Vector<Obj>(numrecs);
  Vector<Obj> *stackIds = NULL;
  if (packets->getProp (PROP_FRINFO))
    stackIds = new Vector<Obj>(numrecs);
  Vector<int> *mstates = NULL;
  if (packets->getProp (PROP_MSTATE))
    mstates = new Vector<int>(numrecs);
  Vector<Vector<long long>*> *sampleVals = NULL;
  if (packets->getProp (PROP_SMPLOBJ))
    sampleVals = new Vector<Vector<long long>*>(numrecs);
  Vector<long long> *timeStart = new Vector<long long>(numrecs);
  Vector<long long> *timeEnd = new Vector<long long>(numrecs);
  int prevEndBin = -1; // make sure we don't overlap bins
  for (int eventIdx = 0; eventIdx < numrecs; eventIdx++)
    {
      long packetIdx = representativeEvents->fetch (eventIdx);
      // long eventId = packets->getIdByIdx( packetIdx );
      const hrtime_t pkt_tstamp = packets->getLongValue (PROP_TSTAMP, packetIdx);
      const hrtime_t pkt_dur = showDuration ? packets->getLongValue (PROP_EVT_TIME, packetIdx) : 0;
      timeStart->store (eventIdx, pkt_tstamp - pkt_dur);
      timeEnd->store (eventIdx, pkt_tstamp);

      // calc startBin
      int startBin = (int) ((pkt_tstamp - pkt_dur - start_ts) / delta);
      if (startBin <= prevEndBin)
	startBin = prevEndBin + 1;
      // calc binCnt
      int endBin = (int) ((pkt_tstamp - start_ts) / delta);
      if (endBin >= numDeltas)
	endBin = numDeltas - 1;
      int binCnt = endBin - startBin + 1;
      prevEndBin = endBin;
      startBins->store (eventIdx, startBin);
      numBins->store (eventIdx, binCnt);
      eventIdxs->store (eventIdx, packetIdx); // store packet's idx
      if (stackIds != NULL)
	{
	  void* stackId = getStack (view_mode, packets, packetIdx);
	  stackIds->store (eventIdx, (Obj) (unsigned long) stackId);
	}
      if (mstates != NULL)
	{
	  int mstate = packets->getIntValue (PROP_MSTATE, packetIdx);
	  mstates->store (eventIdx, mstate);
	}
      if (sampleVals != NULL)
	{
	  Sample* sample = (Sample*) packets->getObjValue (PROP_SMPLOBJ, packetIdx);
	  if (!sample || !sample->get_usage ())
	    sample = sample;
	  else
	    {
	      PrUsage* prusage = sample->get_usage ();
	      Vector<long long> *mstateVals = prusage->getMstateValues ();
	      sampleVals->store (eventIdx, mstateVals);
	    }
	}
    }
  // caller responsible for: delete representativeEvents;
  Vector<void*> *results = new Vector<void*> (8);
  results->store (0, startBins);
  results->store (1, numBins);
  results->store (2, eventIdxs);
  results->store (3, stackIds);
  results->store (4, mstates);
  results->store (5, sampleVals);
  results->store (6, timeStart);
  results->store (7, timeEnd);
  return results;
}

// starting from <event_id> packet idx, step <move_count> visible events
// return the resulting idx and that packet's center time, or null if no event.
Vector<long long> *
dbeGetTLEventCenterTime (int dbevindex, int exp_id, int data_id,
			 int entity_prop_id, int entity_prop_val, int aux,
			 long long event_id, long long move_count)
{
  DataView *packets = getTimelinePackets (dbevindex, exp_id, data_id,
					  entity_prop_id);
  if (packets == NULL)
    return NULL;
  long idx = (long) event_id;

  DbeView *dbev = dbeSession->getView (dbevindex);
  VMode view_mode = dbev->get_view_mode ();
  Experiment *exp = dbeSession->get_exp (exp_id);
  int direction;
  if (move_count == 0)
    direction = 0;
  else if (move_count < 0)
    {
      move_count = -move_count;
      direction = -1;
    }
  else
    direction = 1;
  idx = getTLVisibleIdxByStepping (exp, view_mode, entity_prop_id, packets, aux,
				   entity_prop_val, idx, move_count, direction);
  if (idx >= 0)
    {
      long long ts = packets->getLongValue (PROP_TSTAMP, idx);
      long long dur = packets->getLongValue (PROP_EVT_TIME, idx);
      long long center = ts - dur / 2;
      Vector<long long> *results = new Vector<long long> (2);
      results->store (0, idx); // result idx
      results->store (1, center); // result timestamp
      return results;
    }
  return NULL;
}

long long
dbeGetTLEventIdxNearTime (int dbevindex, int exp_id, int data_id,
			  int entity_prop_id, int entity_prop_val, int aux,
			  int searchDirection, long long tstamp)
{
  DataView *packets = getTimelinePackets (dbevindex, exp_id, data_id,
					  entity_prop_id);
  if (packets == NULL)
    return -1;
  DbeView *dbev = dbeSession->getView (dbevindex);
  VMode view_mode = dbev->get_view_mode ();
  Experiment *exp = dbeSession->get_exp (exp_id);
  if (searchDirection < 0)
    {
      int idx = getTLVisibleIdxByVals (exp, view_mode, entity_prop_id,
				       packets, aux, entity_prop_val, tstamp,
				       DataView::REL_LTEQ);
      if (idx != -1)
	return idx;
      searchDirection = 1; // couldn't find to left, try to right
    }
  if (searchDirection > 0)
    {
      int idx = getTLVisibleIdxByVals (exp, view_mode, entity_prop_id,
				       packets, aux, entity_prop_val, tstamp,
				       DataView::REL_GTEQ);
      if (idx != -1)
	return idx;
      // couldn't find to right, fall through to generic
    }
  // search left and right of timestamp
  long idx1, idx2;
  idx1 = getTLVisibleIdxByVals (exp, view_mode, entity_prop_id,
				packets, aux, entity_prop_val, tstamp,
				DataView::REL_LT);
  idx2 = getTLVisibleIdxByVals (exp, view_mode, entity_prop_id,
				packets, aux, entity_prop_val, tstamp,
				DataView::REL_GTEQ);
  if (idx1 == -1)
    return idx2;
  else if (idx2 == -1)
    return idx1;

  // both valid, so need to compare to see which is closer
  long long t1 = packets->getLongValue (PROP_TSTAMP, idx1);
  long long t2 = packets->getLongValue (PROP_TSTAMP, idx2);
  long long t2dur = packets->getLongValue (PROP_EVT_TIME, idx2);
  long long delta1 = tstamp - t1; // should always be positive
  long long delta2 = (t2 - t2dur) - tstamp; // if negative, overlaps idx1
  if (delta1 > delta2)
    return idx2;
  else
    return idx1;
}

enum Aggr_type
{
  AGGR_NONE,
  AGGR_FAIR,
  AGGR_MAX,
  AGGR_MIN,
  AGGR_CNT,
  AGGR_SUM,
  AGGR_AVG
};

static Aggr_type
getAggrFunc (char *aname)
{
  Aggr_type agrfn = AGGR_NONE;
  if (aname == NULL)
    return agrfn;
  if (strcmp (aname, NTXT ("FAIR")) == 0)
    agrfn = AGGR_FAIR;
  else if (strcmp (aname, NTXT ("MAX")) == 0)
    agrfn = AGGR_MAX;
  else if (strcmp (aname, NTXT ("MIN")) == 0)
    agrfn = AGGR_MIN;
  else if (strcmp (aname, NTXT ("CNT")) == 0)
    agrfn = AGGR_CNT;
  else if (strcmp (aname, NTXT ("SUM")) == 0)
    agrfn = AGGR_SUM;
  else if (strcmp (aname, NTXT ("AVG")) == 0)
    agrfn = AGGR_AVG;
  return agrfn;
}

static long long
computeAggrVal (DefaultMap<long long, long long> *fval_map, Aggr_type agrfn)
{
  long long aval = 0;
  long cnt = 0;
  Vector<long long> *fvals = fval_map->values ();
  long nvals = fvals->size ();
  for (int i = 0; i < nvals; ++i)
    {
      long long val = fvals->fetch (i);
      switch (agrfn)
	{
	case AGGR_FAIR:
	  aval = val;
	  break;
	case AGGR_MAX:
	  if (aval < val || cnt == 0)
	    aval = val;
	  break;
	case AGGR_MIN:
	  if (aval > val || cnt == 0)
	    aval = val;
	  break;
	case AGGR_CNT:
	  aval = cnt + 1;
	  break;
	case AGGR_SUM:
	case AGGR_AVG:
	  aval += val;
	  break;
	case AGGR_NONE:
	  break;
	}
      if (agrfn == AGGR_FAIR)
	break;
      cnt += 1;
    }

  // Finalize aggregation
  if (agrfn == AGGR_AVG)
    if (cnt > 0)
      aval = (aval + cnt / 2) / cnt;
  delete fvals;
  return aval;
}

Vector<long long> *
dbeGetAggregatedValue (int data_id, // data table id
		       char *lfilter, // local filter
		       char *fexpr, // function expression
		       char *pname_ts, // property name for timestamp
		       hrtime_t start_ts, // start of the first time interval
		       hrtime_t delta, // time interval length
		       int num, // number of time intervals
		       char *pname_key, // property name for aggregation key
		       char *aggr_func) // aggregation function
{
  Vector<long long> *res = new Vector<long long>;
  Experiment *exp = dbeSession->get_exp (0);
  if (exp == NULL)
    return res;
  hrtime_t end_ts = start_ts + delta * num;
  if (end_ts < start_ts)    // check overflow
    end_ts = MAX_TIME;

  if (exp->get_status () == Experiment::INCOMPLETE
      && exp->getLastEvent () < end_ts)
    exp->update ();

  DataDescriptor *dataDscr = exp->get_raw_events (data_id);
  if (dataDscr == NULL)
    return res;

  // Process timestamp argument
  int prop_ts = dbeSession->getPropIdByName (pname_ts);
  if (prop_ts == PROP_NONE)
    return res;
  assert (prop_ts == -1);

  // Parse all expressions
  Expression *flt_expr = NULL;
  if (lfilter != NULL)
    flt_expr = dbeSession->ql_parse (lfilter);
  Expression *func_expr = NULL;
  if (fexpr != NULL)
    func_expr = dbeSession->ql_parse (fexpr);
  if (func_expr == NULL)   // Not specified or malformed
    return res;

  // Process aggregation key argument
  int prop_key = PROP_NONE;
  Data *data_key = NULL;
  if (pname_key != NULL)
    {
      prop_key = dbeSession->getPropIdByName (pname_key);
      data_key = dataDscr->getData (prop_key);
      if (data_key == NULL)   // Specified but not found
	return res;
    }

  // Process aggregation function argument
  Aggr_type agrfn = AGGR_FAIR;
  if (aggr_func != NULL)
    {
      agrfn = getAggrFunc (aggr_func);
      if (agrfn == AGGR_NONE) // Specified but not recognized
	return res;
    }
  DefaultMap<long long, long long> *
	fval_map = new DefaultMap<long long, long long>; // key_val -> func_val
  Vector<long long> *key_set = NULL;
  assert (key_set != NULL);
  if (key_set == NULL)
    {
      key_set = new Vector<long long>;
      key_set->append (0L);
    }
  DefaultMap<long long, int> *key_seen = new DefaultMap<long long, int>;
  long idx_prev = -1;
  for (int tidx = 0; tidx < num; ++tidx)
    {
      long idx_cur = -1;
      assert (idx_cur != -1);
      int left = key_set->size ();
      key_seen->clear ();
      for (long idx = idx_cur; idx > idx_prev; --idx)
	{
	  long id = 0;
	  assert (id != 0);

	  // Pre-create expression context
	  Expression::Context ctx (dbeSession->getView (0), exp, NULL, id);
	  // First use the filter
	  if (flt_expr != NULL)
	    if (flt_expr->eval (&ctx) == 0)
	      continue;

	  // Calculate the key
	  // keys are limited to integral values
	  long long key = 0;
	  if (data_key != NULL)
	    key = data_key->fetchLong (id);

	  // Check if already seen
	  if (key_seen->get (key) == 1)
	    continue;
	  key_seen->put (key, 1);
	  left -= 1;

	  // Calculate function value
	  // function values are limited to integral values
	  long long fval = func_expr->eval (&ctx);
	  fval_map->put (key, fval);
	  if (left == 0)
	    break;
	}
      idx_prev = idx_cur;
      long long aval = computeAggrVal (fval_map, agrfn);
      res->store (tidx, aval);
    }
  delete key_seen;
  delete fval_map;
  delete flt_expr;
  delete func_expr;
  return res;
}

Vector<char*> *
dbeGetLineInfo (Obj pc)
{
  DbeInstr *instr = (DbeInstr*) pc;
  if (instr == NULL || instr->get_type () != Histable::INSTR)
    return NULL;
  DbeLine *dbeline = (DbeLine*) instr->convertto (Histable::LINE);
  const char *fname = dbeline ? dbeline->sourceFile->get_name () : NTXT ("");
  char lineno[16];
  *lineno = '\0';
  if (dbeline != NULL)
    snprintf (lineno, sizeof (lineno), NTXT ("%d"), dbeline->lineno);
  Vector<char*> *res = new Vector<char*>(2);
  res->store (0, strdup (fname));
  res->store (1, strdup (lineno));
  return res;
}

int
dbeSetAlias (char *name, char *uname, char *expr)
{
  char *res = dbeSession->indxobj_define (name, uname, expr, NULL, NULL);
  return res == NULL ? 0 : 1;
}

Vector<char*> *
dbeGetAlias (char *name)
{
  Vector<char*> *res = new Vector<char*>;
  int idx = dbeSession->findIndexSpaceByName (name);
  if (idx >= 0)
    {
      char *str = dbeSession->getIndexSpaceDescr (idx);
      res->append (dbe_strdup (str));
      str = dbeSession->getIndexSpaceExprStr (idx);
      res->append (dbe_strdup (str));
    }
  return res;
}

static int
key_cmp (const void *p1, const void *p2)
{
  long long ll1 = *(long long*) p1;
  long long ll2 = *(long long*) p2;
  return ll1 < ll2 ? -1 : ll1 > ll2 ? 1 : 0;
}

Vector<Vector<long long>*> *
dbeGetXYPlotData (
		  int data_id, // data table id
		  char *lfilter, // local filter expression
		  char *arg, // name for the argument
		  char *func1, // expression for the first axis (x)
		  char *aggr1, // aggregation function for func1: "SUM","CNT",...
		  char *func2, // expression for the second axis (y)
		  char *aggr2, // aggregation function for func2
		  char *func3, // expression for the third axis (color)
		  char *aggr3) // aggregation function for func3
{
  Vector<Vector<long long>*> *res = new Vector<Vector<long long>*>;
  Experiment *exp = dbeSession->get_exp (0);
  if (exp == NULL)
    return res;
  if (exp->get_status () == Experiment::INCOMPLETE)
    exp->update ();

  DataDescriptor *dataDscr = exp->get_raw_events (data_id);
  if (dataDscr == NULL)
    return res;

  // Parse all expressions
  Vector<Expression*> *funcs = new Vector<Expression*>;
  Vector<Aggr_type> *aggrs = new Vector<Aggr_type>;
  Vector<DefaultMap<long long, long long>*> *fval_maps =
	  new Vector<DefaultMap<long long, long long>*>;
  Vector<DefaultMap<long long, long>*> *cnt_maps =
	  new Vector<DefaultMap<long long, long>*>;
  if (func1 != NULL)
    {
      Expression *expr = dbeSession->ql_parse (func1);
      funcs->append (expr);
      aggrs->append (getAggrFunc (aggr1));
      fval_maps->append (new DefaultMap<long long, long long>);
      cnt_maps->append (new DefaultMap<long long, long>);
      res->append (new Vector<long long>);
      if (func2 != NULL)
	{
	  expr = dbeSession->ql_parse (func2);
	  funcs->append (expr);
	  aggrs->append (getAggrFunc (aggr2));
	  fval_maps->append (new DefaultMap<long long, long long>);
	  cnt_maps->append (new DefaultMap<long long, long>);
	  res->append (new Vector<long long>);
	  if (func3 != NULL)
	    {
	      expr = dbeSession->ql_parse (func3);
	      funcs->append (expr);
	      aggrs->append (getAggrFunc (aggr3));
	      fval_maps->append (new DefaultMap<long long, long long>);
	      cnt_maps->append (new DefaultMap<long long, long>);
	      res->append (new Vector<long long>);
	    }
	}
    }
  if (funcs->size () == 0)
    {
      funcs->destroy ();
      delete funcs;
      fval_maps->destroy ();
      delete fval_maps;
      cnt_maps->destroy ();
      delete cnt_maps;
      delete aggrs;
      return res;
    }
  Expression *arg_expr = NULL;
  if (arg != NULL)
    arg_expr = dbeSession->ql_parse (arg);
  if (arg_expr == NULL)
    {
      funcs->destroy ();
      delete funcs;
      fval_maps->destroy ();
      delete fval_maps;
      cnt_maps->destroy ();
      delete cnt_maps;
      delete aggrs;
      return res;
    }
  Expression *flt_expr = NULL;
  if (lfilter != NULL)
    flt_expr = dbeSession->ql_parse (lfilter);
  Vector<long long> *kidx_map = new Vector<long long>(); // key_idx -> key_val
  for (long i = 0; i < dataDscr->getSize (); i++)
    {
      Expression::Context ctx (dbeSession->getView (0), exp, NULL, i);
      // First use the filter
      if (flt_expr != NULL)
	if (flt_expr->eval (&ctx) == 0)
	  continue;

      // Compute the argument
      long long key = arg_expr->eval (&ctx);
      if (kidx_map->find (key) == -1)
	kidx_map->append (key);
      for (long j = 0; j < funcs->size (); ++j)
	{
	  Expression *func = funcs->fetch (j);
	  Aggr_type aggr = aggrs->fetch (j);
	  DefaultMap<long long, long long> *fval_map = fval_maps->fetch (j);
	  DefaultMap<long long, long> *cnt_map = cnt_maps->fetch (j);
	  long long fval = func->eval (&ctx);
	  long long aval = fval_map->get (key);
	  long cnt = cnt_map->get (key);
	  switch (aggr)
	    {
	    case AGGR_NONE:
	    case AGGR_FAIR:
	      if (cnt == 0)
		aval = fval;
	      break;
	    case AGGR_MAX:
	      if (aval < fval || cnt == 0)
		aval = fval;
	      break;
	    case AGGR_MIN:
	      if (aval > fval || cnt == 0)
		aval = fval;
	      break;
	    case AGGR_CNT:
	      aval = cnt + 1;
	      break;
	    case AGGR_SUM:
	    case AGGR_AVG:
	      aval += fval;
	      break;
	    }
	  cnt_map->put (key, cnt + 1);
	  fval_map->put (key, aval);
	}
    }
  kidx_map->sort (key_cmp);

  // Finalize aggregation, prepare result
  for (long j = 0; j < funcs->size (); ++j)
    {
      Aggr_type aggr = aggrs->fetch (j);
      Vector<long long> *resj = res->fetch (j);
      DefaultMap<long long, long long> *
	      fval_map = fval_maps->fetch (j);
      DefaultMap<long long, long> *
	      cnt_map = cnt_maps->fetch (j);
      for (int kidx = 0; kidx < kidx_map->size (); ++kidx)
	{
	  long long key = kidx_map->fetch (kidx);
	  long long aval = fval_map->get (key);
	  if (aggr == AGGR_AVG)
	    {
	      long cnt = cnt_map->get (key);
	      if (cnt > 0)
		aval = (aval + cnt / 2) / cnt;
	    }
	  resj->append (aval);
	}
    }
  delete flt_expr;
  funcs->destroy ();
  delete funcs;
  delete aggrs;
  delete arg_expr;
  delete kidx_map;
  fval_maps->destroy ();
  delete fval_maps;
  cnt_maps->destroy ();
  delete cnt_maps;
  return res;
}

/* ********************************************************************* */
/*  Routines for use by Collector GUI */
/**
 * Returns signal value for provided name. Example of name: "SIGUSR1"
 * @param signal
 * @return value
 */
int
dbeGetSignalValue (char *signal)
{
  int ret = -1;
  if (signal == NULL)
    return ret;
  if (strcmp (signal, "SIGUSR1") == 0)
    return (SIGUSR1);
  if (strcmp (signal, "SIGUSR2") == 0)
    return (SIGUSR2);
  if (strcmp (signal, "SIGPROF") == 0)
    return (SIGPROF);
  return ret;
}

char *
dbeSendSignal (pid_t p, int signum)
{
  int ret = kill (p, signum);
  if (p == 0 || p == -1)
    return (dbe_sprintf (GTXT ("kill of process %d not supported\n"), p));
  if (ret == 0)
    return NULL;
  char *msg = dbe_sprintf (GTXT ("kill(%d, %d) failed: %s\n"), p, signum,
			   strerror (errno));
  return msg;
}

char *
dbeGetCollectorControlValue (char *control)
{
  if (control == NULL)
    return NULL;
  if (col_ctr == NULL)
    col_ctr = new Coll_Ctrl (1);
  char *msg = col_ctr->get (control);
  return msg;
}

char *
dbeSetCollectorControlValue (char *control, char * value)
{
  if (control == NULL)
    return NULL;
  if (col_ctr == NULL)
    col_ctr = new Coll_Ctrl (1);
  char *msg = col_ctr->set (control, value);
  return msg;
}

char *
dbeUnsetCollectorControlValue (char *control)
{
  if (control == NULL)
    return NULL;
  if (col_ctr == NULL)
    col_ctr = new Coll_Ctrl (1);
  char *msg = col_ctr->unset (control);
  return msg;
}

void
dbeSetLocation (const char *fname, const char *location)
{
  Vector<SourceFile*> *sources = dbeSession->get_sources ();
  for (long i = 0, sz = sources ? sources->size () : 0; i < sz; i++)
    {
      SourceFile *src = sources->get (i);
      DbeFile *df = src->dbeFile;
      if (df && (strcmp (fname, df->get_name ()) == 0))
	{
	  df->find_file ((char *) location);
	  break;
	}
    }
}

void
dbeSetLocations (Vector<const char *> *fnames, Vector<const char *> *locations)
{
  if (fnames == NULL || locations == NULL
      || fnames->size () != locations->size ())
    return;
  for (long i = 0, sz = fnames->size (); i < sz; i++)
    dbeSetLocation (fnames->get (i), locations->get (i));
}

Vector<void*> *
dbeResolvedWith_setpath (const char *path)
{
  Vector<char*> *names = new Vector<char*>();
  Vector<char*> *pathes = new Vector<char*>();
  Vector<long long> *ids = new Vector<long long>();
  Vector<SourceFile*> *sources = dbeSession->get_sources ();
  for (long i = 0, sz = sources ? sources->size () : 0; i < sz; i++)
    {
      SourceFile *src = sources->get (i);
      DbeFile *df = src->dbeFile;
      if (df == NULL || (df->filetype & DbeFile::F_FICTION) != 0)
	continue;
      char *fnm = df->get_name ();
      if ((df->filetype & (DbeFile::F_JAVACLASS | DbeFile::F_JAVA_SOURCE)) != 0)
	{
	  char *jnm = dbe_sprintf (NTXT ("%s/%s"), path, fnm);
	  if (df->check_access (jnm) == DbeFile::F_FILE)
	    {
	      names->append (dbe_strdup (fnm));
	      pathes->append (jnm);
	      ids->append (src->id);
	      continue;
	    }
	  free (jnm);
	}
      char *nm = dbe_sprintf (NTXT ("%s/%s"), path, get_basename (fnm));
      if (df->check_access (nm) == DbeFile::F_FILE)
	{
	  names->append (dbe_strdup (fnm));
	  pathes->append (nm);
	  ids->append (src->id);
	  continue;
	}
      free (nm);
    }
  if (names->size () != 0)
    {
      Vector<void*> *data = new Vector<void*>(3);
      data->append (names);
      data->append (pathes);
      data->append (ids);
      return data;
    }
  return NULL;
}

Vector<void*> *
dbeResolvedWith_pathmap (const char *old_prefix, const char *new_prefix)
{
  size_t len = strlen (old_prefix);
  Vector<char*> *names = new Vector<char*>();
  Vector<char*> *pathes = new Vector<char*>();
  Vector<long long> *ids = new Vector<long long>();
  Vector<SourceFile*> *sources = dbeSession->get_sources ();
  for (long i = 0, sz = sources ? sources->size () : 0; i < sz; i++)
    {
      SourceFile *src = sources->get (i);
      DbeFile *df = src->dbeFile;
      if (df == NULL || (df->filetype & DbeFile::F_FICTION) != 0)
	continue;
      char *fnm = df->get_name ();
      if (strncmp (old_prefix, fnm, len) == 0
	  && (fnm[len] == '/' || fnm[len] == '\0'))
	{
	  char *nm = dbe_sprintf (NTXT ("%s/%s"), new_prefix, fnm + len);
	  if (df->check_access (nm) == DbeFile::F_FILE)
	    {
	      names->append (dbe_strdup (fnm));
	      pathes->append (nm);
	      ids->append (src->id);
	      continue;
	    }
	  if ((df->filetype & DbeFile::F_JAVA_SOURCE) != 0)
	    {
	      free (nm);
	      nm = dbe_sprintf (NTXT ("%s/%s"), new_prefix, fnm);
	      if (df->check_access (nm) == DbeFile::F_FILE)
		{
		  names->append (dbe_strdup (fnm));
		  pathes->append (nm);
		  ids->append (src->id);
		  continue;
		}
	    }
	  free (nm);
	}
    }
  if (names->size () != 0)
    {
      Vector<void*> *data = new Vector<void*>(3);
      data->append (names);
      data->append (pathes);
      data->append (ids);
      return data;
    }
  return NULL;
}

void
dbe_archive (Vector<long long> *ids, Vector<const char *> *locations)
{
  if (ids == NULL || locations == NULL || ids->size () != locations->size ())
    return;
  Experiment *exp = dbeSession->get_exp (0);
  if (exp == NULL)
    return;
  Vector<SourceFile*> *sources = dbeSession->get_sources ();
  for (long i1 = 0, sz1 = ids->size (); i1 < sz1; i1++)
    {
      long long id = ids->get (i1);
      for (long i = 0, sz = sources ? sources->size () : 0; i < sz; i++)
	{
	  SourceFile *src = sources->get (i);
	  if (src->id == id)
	    {
	      DbeFile *df = src->dbeFile;
	      if (df)
		{
		  char *fnm = df->find_file ((char *) locations->get (i1));
		  if (fnm)
		    {
		      char *nm = df->get_name ();
		      char *anm = exp->getNameInArchive (nm, false);
		      exp->copy_file (fnm, anm, true);
		      free (anm);
		    }
		}
	    }
	}
    }
}

/* ************************************************************************ */

/* Routines to check connection between Remote Analyzer Client and er_print */
char *
dbeCheckConnection (char *str)
{
  return dbe_strdup (str);
}
