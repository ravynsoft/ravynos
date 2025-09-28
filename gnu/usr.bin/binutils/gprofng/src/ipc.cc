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
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>      // creat
#include <unistd.h>     // sleep
#include <pthread.h>    // pthread_exit
#include <sys/wait.h>   // wait
#include <locale.h>

#include "DbeApplication.h"
#include "Histable.h"
#include "ipcio.h"
#include "Dbe.h"
#include "DbeSession.h"
#include "DbeThread.h"
#include "DbeView.h"

int ipc_flags = 0;
IPCTraceLevel ipc_trace_level = TRACE_LVL_0;
int ipc_single_threaded_mode = 0;
const char *IPC_PROTOCOL_UNKNOWN = "IPC_PROTOCOL_UNKNOWN";
const char *IPC_PROTOCOL_CURR = IPC_PROTOCOL_STR;
char const *ipc_protocol = NULL;

DbeThreadPool *ipcThreadPool;

extern int currentRequestID;
extern int currentChannelID;
extern BufferPool *responseBufferPool;
extern bool cancelNeeded (int);
extern void reexec ();

/* Simple implementation of support for cancel of open experiment. Since we have only one cancellable
   operation supported at this moment, we are using just a global variable.
   As we support more and more cancellable ops we need a more sophisticated data struture such
   as a mt-safe array to keep track of all cancellable requests/channels and update the supporting
   routines - setCancellableChannel, cancelNeeded (in ipcio.cc) setCancelRequestedCh */
int cancellableChannelID = 0xFFFFFFFF;
int cancelRequestedChannelID;

static const char *table_name (int);

#define VSIZE(v)        ((long long) ((v) ? (v)->size() : 0))

inline const char*
bool2str (bool v)
{
  return v ? "true" : "false";
}

inline const char*
str2str (const char* v)
{
  return v ? v : "NULL";
}

inline const char*
str2s (const char* v)
{
  return v ? v : "";
}

inline DbeView *
getView (int index)
{
  return dbeSession->getView (index);
}

extern "C"
{
  typedef void (*SignalHandler)(int);
}

/*
 * Fatal error handlers
 */
static int fatalErrorCode = 1;
static int fatalErrorCounter = 0;
static void *fatalErrorContext = 0;
static siginfo_t *fatalErrorInfo = 0;
static char *fatalErrorDynamicMemory = NULL;

extern "C" void
fatalErrorHadler (int sig, siginfo_t *info, void *context)
{
  if (fatalErrorCounter > 0)
    { // Need synchronization here
      //sleep(10); // Wait 10 seconds to make sure previous processing is done
      return; // exit(fatalErrorCode); // Already in processing
    }
  fatalErrorCounter = 1;
  fatalErrorCode = sig;
  fatalErrorContext = context;
  fatalErrorInfo = info;
  // Free reserved memory
  if (fatalErrorDynamicMemory != NULL)
    {
      free (fatalErrorDynamicMemory);
      fatalErrorDynamicMemory = NULL;
    }
  // Get process ID
  pid_t pid = getpid ();
  // Create dump file
  char fname[128];
  snprintf (fname, sizeof (fname), "/tmp/gprofng.%lld", (long long) pid);
  mkdir (fname, 0700);
  snprintf (fname, sizeof (fname), "/tmp/gprofng.%lld/crash.sig%d.%lld",
	    (long long) pid, sig, (long long) pid);
  // Dump stack trace in background using pstack
  char buf[256];
  snprintf (buf, sizeof (buf), "/usr/bin/pstack %lld > %s.pstack",
	    (long long) pid, fname);
  system (buf);
  int fd = creat (fname, 0600);
  if (fd >= 0)
    {
      // Write error message
      dbe_write (fd, "A fatal error has been detected by er_print: Signal %d\n",
		 sig);
      dbe_write (fd, "Protocol Version: %d\n", IPC_VERSION_NUMBER);
      close (fd);
    }
  wait (0); // wait for pstack
  //sleep(10); // Wait 10 seconds to make sure processing of fatal error is done
  // Exit with correct status
  exit (fatalErrorCode);
}

// SIGABRT Handler
extern "C" void
sigABRT_handler (int sig, siginfo_t *info, void *context)
{
  fatalErrorHadler (sig, info, context);
  pthread_exit (&fatalErrorCode);
}

// SIGSEGV Handler
extern "C" void
sigSEGV_handler (int sig, siginfo_t *info, void *context)
{
  //if (fatalErrorCounter > 0) sleep(1); // Wait 1 second
  fatalErrorHadler (sig, info, context);
  pthread_exit (&fatalErrorCode);
}

// SIGTERM Handler
extern "C" void sigterm_handler (int sig, siginfo_t *info, void *context);
struct sigaction old_sigterm_handler;

volatile int term_flag;
int error_flag;

extern "C" void
sigterm_handler (int, siginfo_t *, void *)
{
  if (fatalErrorCounter > 0)
    {
      //sleep(10); // Wait 10 seconds to make sure processing of fatal error is done
      //return; // Fatal error processing will exit it
      pthread_exit (&fatalErrorCode);
    }
  term_flag = 1;
}

#define ipc_log ipc_default_log
#define ipc_request_trace ipc_request_log
#define ipc_response_trace ipc_response_log
static const char *ipc_log_name = NULL;
static const char *ipc_request_log_name = NULL;
static const char *ipc_response_log_name = NULL;
static FILE *requestLogFileP = stderr;
static FILE *responseLogFileP = stderr;
static hrtime_t begin_time;
static long long delta_time = 0;

void
ipc_default_log (const char *fmt, ...)
{
  if (!ipc_log_name || !ipc_flags)
    return;
  if (ipc_trace_level >= TRACE_LVL_3)
    {
      hrtime_t cur_time = gethrtime ();
      unsigned long long time_stamp = (cur_time - begin_time) / 1000000 + delta_time;
      fprintf (stderr, "%7llu: ", time_stamp);
    }
  va_list vp;
  va_start (vp, fmt);
  vfprintf (stderr, fmt, vp);
  va_end (vp);
  fflush (stderr);
}

extern "C" void sigint_handler (int sig, siginfo_t *info, void *context);
struct sigaction old_sigint_handler;

extern "C" void
sigint_handler (int, siginfo_t *, void *)
{
  ipc_log ("SIGINT signal happens\n");
}

void
ipc_request_log (IPCTraceLevel trace_level, const char *fmt, ...)
{
  if (!ipc_request_log_name || !ipc_flags || trace_level > ipc_trace_level)
    return;
  fprintf (responseLogFileP, "thr: %llu ", (unsigned long long) pthread_self ());
  if (ipc_trace_level >= TRACE_LVL_3)
    {
      hrtime_t cur_time = gethrtime ();
      unsigned long long time_stamp = (cur_time - begin_time) / 1000000 + delta_time;
      fprintf (requestLogFileP, "%7llu: ", time_stamp);
    }
  va_list vp;
  va_start (vp, fmt);
  vfprintf (requestLogFileP, fmt, vp);
  va_end (vp);
  fflush (requestLogFileP);
}

void
ipc_response_log (IPCTraceLevel trace_level, const char *fmt, ...)
{
  if (!ipc_response_log_name || !ipc_flags || trace_level > ipc_trace_level)
    return;
  fprintf (responseLogFileP, "thr: %llu ", (unsigned long long) pthread_self ());
  if (ipc_trace_level >= TRACE_LVL_3)
    {
      hrtime_t cur_time = gethrtime ();
      unsigned long long time_stamp = (cur_time - begin_time) / 1000000 + delta_time;
      fprintf (responseLogFileP, "%7llu: ", time_stamp);
    }
  va_list vp;
  va_start (vp, fmt);
  vfprintf (responseLogFileP, fmt, vp);
  va_end (vp);
  fflush (responseLogFileP);
}

#ifdef IPC_LOG
void
ipc_dump (char *s, Vector<bool> *v)
{
  if (v == NULL)
    {
      ipc_log ("    Vector<bool> %s is NULL\n", str2s (s));
      return;
    }
  ipc_log ("    Vector<bool> %s size=%lld\n", str2s (s), VSIZE (v));
  for (int i = 0; i < v->size (); i++)
    ipc_log ("      [%d]: %s\n", i, bool2str (v->fetch (i)));
}

void
ipc_dump (char *s, Vector<String> *v)
{
  if (v == NULL)
    {
      ipc_log ("    Vector<bool> %s is NULL\n", str2s (s));
      return;
    }
  ipc_log ("    Vector<String> %s size=%lld\n", str2s (s), VSIZE (v));
  for (int i = 0; i < v->size (); i++)
    {
      String str = v->fetch (i);
      ipc_log ("      [%d]: '%s'\n", i, str2str (str));
    }
}

void
ipc_dump (char *s, Vector<Obj> *v)
{
  if (v == NULL)
    {
      ipc_log ("    Vector<Obj> %s is NULL\n", str2s (s));
      return;
    }
  ipc_log ("    Vector<void *> %s size=%lld\n", str2s (s), VSIZE (v));
  for (int i = 0; i < v->size (); i++)
    ipc_log ("      [%d]: 0x%08llx\n", i, (long long) (v->fetch (i)));
}

#else
#define ipc_dump(s, v)
#endif

static MetricList *
readMetricListV2 (int dbevindex, IPCrequest* req)
{
  MetricType mtype = (MetricType) readInt (req);
  Vector<int> *type = (Vector<int>*)readArray (req);
  Vector<int> *subtype = (Vector<int>*)readArray (req);
  Vector<bool> *sort = (Vector<bool>*)readArray (req);
  Vector<int> *vis = (Vector<int>*)readArray (req);
  Vector<char*> *cmd = (Vector<char*>*)readArray (req);
  Vector<char*> *expr_spec = (Vector<char*>*)readArray (req);
  Vector<char*> *legends = (Vector<char*>*)readArray (req);
  MetricList *mlist = dbeGetMetricListV2 (dbevindex, mtype, type, subtype, sort,
					  vis, cmd, expr_spec, legends);
  return mlist;
}

static void
setCancellableChannel (int chID)
{
  cancellableChannelID = chID;
}

/* Add more entries here for other cancellable operations */
static void
checkCancellableOp (char *inp, IPCrequest* req)
{
  if (!strcmp (inp, "setFilterStr"))
    setCancellableChannel (currentChannelID);
  else if (!strcmp (inp, "openExperimentList"))
    setCancellableChannel (currentChannelID);
  else if (!strcmp (inp, "getFiles") || !strcmp (inp, "getFileAttributes"))
    {
      setCancellableChannel (currentChannelID);
      req->setCancelImmediate ();
    }
}

/* This is what used to be the core of ipc_mainLoop before asynch ipc.
   Read the details of the request from the request buffer: name, args etc
   do the work by calling the appropriate dbe routine(s) and write the
   response to a response buffer and queue it up in the response queue */

int
ipc_doWork (void *arg)
{
  IPCrequest *req = (IPCrequest *) arg;
  currentRequestID = req->getRequestID ();
  currentChannelID = req->getChannelID ();
  req->setStatus (IN_PROGRESS);
  String inp = readString (req);
  if (inp == NULL)
    {
      ipc_log ("NULL ipc command received, exiting\n");
      return 0;
    }
  ipc_log ("ipc: %s Req %x Ch %x\n", inp, req->getRequestID (), req->getChannelID ());
  checkCancellableOp (inp, req);
  if (!strcmp (inp, "initApplication"))
    {
      bool nbm = readBoolean (req);
      String arg1 = readString (req);
      String arg2 = readString (req);
      Vector<String> *arg3 = (Vector<String>*)readArray (req);
      ipc_log ("  nbm: %s, arg1: '%s', arg2: '%s'\n", bool2str (nbm), str2str (arg1), str2str (arg2));
      ipc_dump ("arg3", arg3);
      // set the session to be interactive
      dbeSession->set_interactive (true);
      if (nbm)
	theApplication->set_name ("analyzer-NBM");
      else
	theApplication->set_name ("analyzer");

	  // XXX Why does it reset the install directory????  Or a licensing directory???
      // Vector<String> *res = theDbeApplication->initApplication (arg1, arg2, &setProgress);
      Vector<String> *res = theDbeApplication->initApplication (NULL, NULL, &setProgress);
      writeArray (res, req);
      free (arg1);
      free (arg2);
      destroy (arg3);
      destroy (res);
    }
  else if (!strcmp (inp, "syncTime"))
    {
      long long anl_time = readLong (req);
      hrtime_t cur_time = gethrtime ();
      long long time_stamp = (cur_time - begin_time) / 1000000;
      delta_time = anl_time - time_stamp;
      ipc_log (" syncTime %llu %llu \n", anl_time, delta_time);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "getInitMessages"))
    {
      Vector<String> *res = dbeGetInitMessages ();
      ipc_log ("  returned = %lld msgs\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "reExec"))
    {
      ipc_log ("  started reexec()\n");
      reexec ();
    }
  else if (!strcmp (inp, "dbeCreateDirectories"))
    {
      String arg1 = readString (req); // path
      ipc_log ("  arg = %s\n", arg1);
      String res = dbeCreateDirectories (arg1);
      writeString (res, req);
      free (arg1);
      free (res);
    }
  else if (!strcmp (inp, "dbeDeleteFile"))
    {
      String arg1 = readString (req); // path
      ipc_log ("  arg = %s\n", arg1);
      String res = dbeDeleteFile (arg1);
      writeString (res, req);
      free (arg1);
      free (res);
    }
  else if (!strcmp (inp, "dbeReadFile"))
    {
      String arg1 = readString (req);
      ipc_log ("  arg = %s\n", arg1); // path
      Vector<String> *res = dbeReadFile (arg1);
      writeArray (res, req);
      free (arg1);
      destroy (res);
    }
  else if (!strcmp (inp, "dbeWriteFile"))
    {
      String arg1 = readString (req); // path
      String arg2 = readString (req); // contents
      ipc_log ("  arg1 = %s  arg2 = %s\n", arg1, arg2);
      int res = dbeWriteFile (arg1, arg2);
      writeInt (res, req);
      free (arg1);
    }
  else if (!strcmp (inp, "getExpPreview"))
    {
      // XXX add another argument == DbeView index
      String arg1 = readString (req);
      ipc_log ("  arg = %s\n", arg1);
      Vector<String> *res = dbeGetExpPreview (0, arg1);
      writeArray (res, req);
      free (arg1);
      destroy (res);
    }
  else if (!strcmp (inp, "getFileAttributes"))
    {
      String arg1 = readString (req); // filename
      String arg2 = readString (req); // format
      ipc_log ("  arg1 = %s  arg2 = %s\n", arg1, arg2);
      String res = dbeGetFileAttributes (arg1, arg2);
      writeString (res, req);
      free (arg1);
      free (arg2);
      free (res);
    }
  else if (!strcmp (inp, "getFiles"))
    {
      String arg1 = readString (req); // dirname
      String arg2 = readString (req); // format
      ipc_log ("  arg1 = %s  arg2 = %s\n", arg1, arg2);
      String res = dbeGetFiles (arg1, arg2);
      writeString (res, req);
      free (arg1);
      free (arg2);
      free (res);
    }
  else if (!strcmp (inp, "getOSFamily"))
    writeString ("Linux", req);
  else if (!strcmp (inp, "getRunningProcesses"))
    {
      String arg1 = readString (req); // format
      ipc_log ("  arg = %s\n", arg1);
      String res = dbeGetRunningProcesses (arg1);
      writeString (res, req);
      free (arg1);
      free (res);
    }
  else if (!strcmp (inp, "getCurrentDirectory"))
    {
      char buf [2048];
      String res = getcwd (buf, (size_t) sizeof (buf)); // Get current directory
      writeString (res, req);
    }
  else if (!strcmp (inp, "getHomeDirectory"))
    {
      String res = getenv ("HOME"); // Get HOME directory
      writeString (res, req);
    }
  else if (!strcmp (inp, "setCurrentDirectory"))
    {
      String arg1 = readString (req); // dirname
      ipc_log ("  arg = %s\n", arg1);
      int res = chdir (arg1); // Change directory
      writeInt (res, req);
      free (arg1);
    }
  else if (!strcmp (inp, "getLocale"))
    {
      String res = setlocale (LC_ALL, ""); // Get locale
      writeString (res, req);
    }
  else if (!strcmp (inp, "setLocale"))
    {
      String arg1 = readString (req); // locale
      ipc_log ("  arg = %s\n", arg1);
      String res = setlocale (LC_ALL, arg1); // Set locale
      writeString (res, req);
      free (arg1);
    }
  else if (!strcmp (inp, "readRCFile"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req); // file name
      ipc_log ("  arg1=%d, arg2=%s\n", arg1, arg2);
      String res = dbeReadRCFile (arg1, arg2); // Read RC File
      writeString (res, req);
      free (res);
    }
  else if (!strcmp (inp, "dbeGetExpParams"))
    {
      // XXX add another argument == DbeView index
      String arg1 = readString (req);
      ipc_log ("  arg = %s\n", arg1);
      String res = dbeGetExpParams (0, arg1);
      writeString (res, req);
      free (arg1);
      free (res);
    }
  else if (!strcmp (inp, "getExperimentsGroups"))
    {
      Vector<Vector<char*>*> *groups = dbeGetExperimensGroups ();
      writeArray (groups, req);
      destroy (groups);
    }
  else if (!strcmp (inp, "setExperimentsGroups"))
    {
      Vector<Vector<char*>*> *groups = (Vector<Vector<char*>*> *)readArray (req);
      ipc_log ("  groups.size = %lld\n", VSIZE (groups));
      char *msg_str = dbeSetExperimentsGroups (groups);
      writeString (msg_str, req);
      free (msg_str);
      destroy (groups);
    }
  else if (!strcmp (inp, "dropExperiment"))
    {
      int arg1 = readInt (req);
      Vector<int> *arg2 = (Vector<int>*)readArray (req);
      ipc_log ("  arg = %d, exps = %lld\n", arg1, VSIZE (arg2));
      char *res = dbeDropExperiment (arg1, arg2);
      writeString (res, req);
      free (res);
      delete arg2;
    }
  else if (!strcmp (inp, "getUserExpId"))
    {
      Vector<int> *arg = (Vector<int>*)readArray (req);
      ipc_log ("  expIds = %lld\n", VSIZE (arg));
      Vector<int> *res = dbeGetUserExpId (arg);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getFounderExpId"))
    {
      Vector<int> *arg = (Vector<int>*)readArray (req);
      ipc_log ("  expIds = %lld\n", VSIZE (arg));
      Vector<int> *res = dbeGetFounderExpId (arg);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getExpGroupId"))
    {
      Vector<int> *arg = (Vector<int>*)readArray (req);
      ipc_log ("  expIds = %lld\n", VSIZE (arg));
      Vector<int> *res = dbeGetExpGroupId (arg);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getExpsProperty"))
    {
      String arg = readString (req);
      Vector<String> *res = dbeGetExpsProperty (arg);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getExpName"))
    {
      // XXX add argument == DbeView index
      Vector<String> *res = dbeGetExpName (0);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getExpState"))
    {
      // XXX add argument == DbeView index
      Vector<int> *res = dbeGetExpState (0);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getExpEnable"))
    {
      int arg1 = readInt (req);
      ipc_log ("  arg1 = %d\n", arg1);
      Vector<bool> *res = dbeGetExpEnable (arg1);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "setExpEnable"))
    {
      int arg1 = readInt (req);
      Vector<bool> *arg2 = (Vector<bool>*)readArray (req);
      ipc_log ("  arg1=%d\n", arg1);
      ipc_dump ("arg2", arg2);
      bool b = dbeSetExpEnable (arg1, arg2);
      writeBoolean (b, req);
      ipc_log ("  dbeSetExpEnable returns %s\n", bool2str (b));
      delete arg2;
    }
  else if (!strcmp (inp, "getExpInfo"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<String> *res = dbeGetExpInfo (arg1);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getViewModeEnable"))
    {
      bool res = dbeGetViewModeEnable ();
      writeBoolean (res, req);
    }
  else if (!strcmp (inp, "getJavaEnable"))
    {
      bool res = dbeGetJavaEnable ();
      writeBoolean (res, req);
    }
  else if (!strcmp (inp, "updateNotes"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      String arg4 = readString (req);
      bool arg5 = readBoolean (req);
      ipc_log ("  args = %d, %d\n", arg1, arg2);
      int i = dbeUpdateNotes (arg1, arg2, arg3, arg4, arg5);
      writeInt (i, req);
    }
  else if (!strcmp (inp, "getLoadObjectList"))
    {
      int arg1 = readInt (req);
      ipc_log ("  arg = %d\n", arg1);
      Vector<void *> *res = dbeGetLoadObjectList (arg1);
      if (res == NULL)
	ipc_log ("  returning = NULL for LoadObjectList\n");
      else
	{
	  Vector<char*> *s = (Vector<char*> *) res->fetch (0);
	  ipc_log ("  returning = %lld vectors for %lld LoadObjects\n",
		   VSIZE (res), VSIZE (s));
	}
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getLoadObjectName"))
    {
      int arg1 = readInt (req);
      ipc_log ("  arg = %d\n", arg1);
      Vector<String> *res = dbeGetLoadObjectName (arg1);
      ipc_log ("  returning = %lld strings\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getTabListInfo"))
    {
      int arg1 = readInt (req);
      ipc_log ("  arg = %d\n", arg1);
      Vector<void*> *res = dbeGetTabListInfo (arg1);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getSearchPath"))
    {
      // XXX add argument == DbeView index
      ipc_log ("  no args\n");
      Vector<String> *res = dbeGetSearchPath (0);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setSearchPath"))
    {
      // XXX add another argument == DbeView index
      Vector<String> *res = (Vector<String>*)readArray (req);
      ipc_log ("  %lld strings\n", VSIZE (res));
      dbeSetSearchPath (0, res);
      writeString (NULL, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getPathmaps"))
    {
      Vector<void*> *res = dbeGetPathmaps (0);
      ipc_log ("  returns = %lld objects, number of pathmaps = %lld\n",
	       VSIZE (res), VSIZE ((Vector<int>*)res->fetch (0)));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setPathmaps"))
    {
      Vector<String> *from = (Vector<String>*)readArray (req);
      Vector<String> *to = (Vector<String>*)readArray (req);
      char *res = dbeSetPathmaps (from, to);
      writeString (res, req);
      free (res);
      if (from)
	{
	  from->destroy ();
	  delete from;
	}
      if (to)
	{
	  to->destroy ();
	  delete to;
	}
    }
  else if (!strcmp (inp, "addPathmap"))
    {
      // XXX add another argument == DbeView index
      String arg1 = readString (req);
      String arg2 = readString (req);
      ipc_log ("  args = '%s', '%s'\n", arg1 ? arg1 : "NULL", arg2 ? arg2 : "NULL");
      char *res = dbeAddPathmap (0, arg1, arg2);
      ipc_log ("  returns = '%s'\n", (res != NULL ? res : "NULL"));
      writeString (res, req);
      free (arg1);
      free (arg2);
    }
  else if (!strcmp (inp, "getMsg"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  args = %d, %d\n", arg1, arg2);
      String res = dbeGetMsg (arg1, arg2);
      ipc_log ("  returns = '%s'\n", (res != NULL ? res : "<NULL>"));
      writeString (res, req);
      free (res);
    }
  else if (!strcmp (inp, "initView"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  new view = %d; clone of view %d\n", arg1, arg2);
      dbeInitView (arg1, arg2);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "disposeWindow"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      dbeDeleteView (arg1);
      writeString (NULL, req);
    }
#if 0
  else if (!strcmp (inp, "createMapfile"))
    {
      int arg1 = readInt ();
      String arg2 = readString ();
      int arg3 = readInt ();
      ipc_log ("  args = %d, %s, %d\n", arg1, arg2, arg3);
      String res = dbeCreateMapfile (arg1, arg2, arg3);
      writeString (res);
      free (arg2);
      free (res);
    }
#endif
  else if (!strcmp (inp, "setCompareModeV2"))
    {
      int dbevindex = readInt (req);
      int cmp_mode = readInt (req);
      getView (dbevindex)->set_compare_mode (cmp_mode);
      writeResponseGeneric (RESPONSE_STATUS_SUCCESS, req->getRequestID (), req->getChannelID ());
    }
  else if (!strcmp (inp, "getCompareModeV2"))
    {
      int dbevindex = readInt (req);
      int res = CMP_DISABLE;
      if (dbeSession->expGroups && dbeSession->expGroups->size () > 1)
	res = getView (dbevindex)->get_compare_mode ();
      ipc_log ("  %s: %d returns %d\n", inp, dbevindex, res);
      writeInt (res, req);
    }
  else if (!strcmp (inp, "getRefMetricsV2"))
    {
      Vector<void*> *res = dbeGetRefMetricsV2 ();
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setCurMetricsV2"))
    {
      int dbevindex = readInt (req);
      int cmp_mode = readInt (req);
      MetricList *mlist = readMetricListV2 (dbevindex, req);
      getView (dbevindex)->reset_metric_list (mlist, cmp_mode);
      writeResponseGeneric (RESPONSE_STATUS_SUCCESS, req->getRequestID (), req->getChannelID ());
    }
  else if (!strcmp (inp, "getCurMetricsV2"))
    {
      int arg1 = readInt (req);
      MetricType arg2 = (MetricType) readInt (req);
      ipc_log ("  args = %d, %d\n", arg1, arg2);
      Vector<void*> *res = dbeGetCurMetricsV2 (arg1, arg2);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getRefMetricTree"))
    {
      int dbevindex = readInt (req);
      bool include_unregistered = readBoolean (req);
      ipc_log ("  args = %d, %d\n", dbevindex, include_unregistered);
      Vector<void*> *res = dbeGetRefMetricTree (dbevindex, include_unregistered);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getRefMetricTreeValues"))
    {
      int dbevindex = readInt (req);
      Vector<String> *metcmds = (Vector<String>*)readArray (req);
      Vector<String> *nonmetcmds = (Vector<String>*)readArray (req);
      ipc_log ("  args = %d, metcmds->size()=%lld, nonmetcmds->size()=%lld\n",
	       dbevindex, VSIZE (metcmds), VSIZE (nonmetcmds));
      ipc_dump ("metcmds", metcmds);
      ipc_dump ("nonmetcmds", nonmetcmds);
      Vector<void*> *res = dbeGetRefMetricTreeValues (dbevindex, metcmds, nonmetcmds);
#ifdef IPC_LOG
      if (res != NULL)
	ipc_log ("  returns = %lld objects, length = %lld\n",
		 VSIZE (res), VSIZE (((Vector<int>*)res->fetch (0))));
      else
	ipc_log ("  returns NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getOverviewText"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<char*> *res = dbeGetOverviewText (arg1);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setSort"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      MetricType arg3 = (MetricType) readInt (req);
      bool arg4 = readBoolean (req);
      ipc_log ("  args = %d, %d, %d, %c\n", arg1, arg2, arg3, (arg4 ? 'T' : 'F'));
      dbeSetSort (arg1, arg2, arg3, arg4);
      writeString (NULL, req);
    }

  else if (!strcmp (inp, "getAnoValue"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<int> *res = dbeGetAnoValue (arg1);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "setAnoValue"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d, array\n", arg1);
      Vector<int> *arg2 = (Vector<int>*)readArray (req);
      dbeSetAnoValue (arg1, arg2);
      writeString (NULL, req);
      delete arg2;
    }
  else if (!strcmp (inp, "getNameFormat"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      int b = dbeGetNameFormat (arg1);
      writeInt (b, req);
    }
  else if (!strcmp (inp, "getSoName"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      bool b = dbeGetSoName (arg1);
      writeBoolean (b, req);
    }
  else if (!strcmp (inp, "setNameFormat"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      bool arg3 = readBoolean (req);
      ipc_log ("  args = %d, %d, %d\n", arg1, arg2, arg3);
      dbeSetNameFormat (arg1, arg2, arg3);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "getViewMode"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      int i = dbeGetViewMode (arg1);
      ipc_log ("  returns = %d\n", i);
      writeInt (i, req);
    }
  else if (!strcmp (inp, "setViewMode"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  args = %d, %d\n", arg1, arg2);
      dbeSetViewMode (arg1, arg2);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "getTLValue"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetTLValue (arg1);
      ipc_log ("  returns = %lld void*'s\n", VSIZE (res));
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "setTLValue"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      String tldata_cmd = readString (req);
      int entity_prop_id = readInt (req);
      int align = readInt (req);
      int depth = readInt (req);
      dbeSetTLValue (arg1, tldata_cmd, entity_prop_id, align, depth);
      writeString (NULL, req);
      free (tldata_cmd);
    }
  else if (!strcmp (inp, "getExpFounderDescendants"))
    {
      Vector<void*> *res = dbeGetExpFounderDescendants ();
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getExpSelection"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetExpSelection (arg1);
      writeArray (res, req);
      destroy (res);
    }

  else if (!strcmp (inp, "setFilterStr"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      ipc_log ("  args = %d, %s\n", arg1, arg2);
      String res = dbeSetFilterStr (arg1, arg2);
      ipc_log ("  returns = '%s'\n", res ? res : "NULL");
      writeString (res, req);
      free (arg2);
      free (res);
    }

  else if (!strcmp (inp, "getFilterStr"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      String res = dbeGetFilterStr (arg1);
      ipc_log ("  returns = '%s'\n", res ? res : "NULL");
      writeString (res, req);
      free (res);
    }

  else if (!strcmp (inp, "validateFilterExpression"))
    {
      String arg1 = readString (req);
      int res = dbeValidateFilterExpression (arg1);
      ipc_log ("  validateFilterExpression('%s') returned %d\n", str2str (arg1), res);
      free (arg1);
      writeInt (res, req);
    }

  else if (!strcmp (inp, "getFilterKeywords"))
    {
      int dbevindex = readInt (req);
      Vector<void*>*res = dbeGetFilterKeywords (dbevindex);
      writeArray (res, req);
      destroy (res);
    }

  else if (!strcmp (inp, "getFilters"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  args: view = %d, experiment = %d\n", arg1, arg2);
      Vector<void*>*res = dbeGetFilters (arg1, arg2);
      ipc_log ("  -- returned %lld Filters\n", VSIZE (res));
      writeArray (res, req);
      delete res;
    }

  else if (!strcmp (inp, "updateFilters"))
    {
      int arg1 = readInt (req);
      Vector<bool> *arg2 = (Vector<bool>*)readArray (req);
      Vector<String> *arg3 = (Vector<String>*)readArray (req);
      ipc_log ("arg1=%d arg2->size()=%lld  arg3->size()=%lld\n",
	       arg1, VSIZE (arg2), VSIZE (arg3));
      ipc_dump ("arg2", arg2);
      ipc_dump ("arg3", arg3);
      bool b = dbeUpdateFilters (arg1, arg2, arg3);
      writeBoolean (b, req);
      ipc_log ("  returns %s\n", (b == true ? "true" : "false"));
      delete arg2;
      delete arg3;
    }
  else if (!strcmp (inp, "getLoadObjectState"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d \n", arg1);
      Vector<int> *res = dbeGetLoadObjectState (arg1);
      ipc_log ("  returning = %lld int's\n", VSIZE (res));
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "setLoadObjectState"))
    {
      int arg1 = readInt (req);
      Vector<int> *arg2 = (Vector<int>*)readArray (req);
      ipc_log ("  args = %d, %lld objects\n", arg1, VSIZE (arg2));
      dbeSetLoadObjectState (arg1, arg2);
      writeString (NULL, req);
      delete arg2;
    }
  else if (!strcmp (inp, "setLoadObjectDefaults"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      dbeSetLoadObjectDefaults (arg1);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "getMemTabSelectionState"))
    {
      int arg1 = readInt (req);
      ipc_log ("  arg = %d\n", arg1);
      Vector<bool> *res = dbeGetMemTabSelectionState (arg1);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setMemTabSelectionState"))
    {
      int arg1 = readInt (req);
      Vector<bool> *arg2 = (Vector<bool> *)readArray (req);
      ipc_log ("  args = %d\n  arg2 = %lld objects\n", arg1, VSIZE (arg2));
      dbeSetMemTabSelectionState (arg1, arg2);
      writeString (NULL, req);
      destroy (arg2);
    }
  else if (!strcmp (inp, "getIndxTabSelectionState"))
    {
      int arg1 = readInt (req);
      ipc_log ("  arg = %d\n", arg1);
      Vector<bool> *res = dbeGetIndxTabSelectionState (arg1);
      ipc_log ("  -- returned %lld-vector [bool]\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setIndxTabSelectionState"))
    {
      int arg1 = readInt (req);
      Vector<bool> *arg2 = (Vector<bool> *)readArray (req);
      ipc_log ("  args = %d\n  arg2 = %lld objects\n", arg1, VSIZE (arg2));
      dbeSetIndxTabSelectionState (arg1, arg2);
      writeString (NULL, req);
      destroy (arg2);
    }
  else if (!strcmp (inp, "getTabSelectionState"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<bool> *res = dbeGetTabSelectionState (arg1);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "setTabSelectionState"))
    {
      int arg1 = readInt (req);
      Vector<bool> *arg2 = (Vector<bool>*)readArray (req);
      ipc_log ("  args = %d\n  arg2 = %lld objects\n", arg1, VSIZE (arg2));
      dbeSetTabSelectionState (arg1, arg2);
      writeString (NULL, req);
      delete arg2;
    }
  else if (!strcmp (inp, "getMemObjects"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetMemObjects (arg1);

#ifdef IPC_LOG
      if (res == NULL)
	ipc_log ("  -- returned NULL\n");
      else
	{
	  Vector<int> *mo_types = (Vector<int> *)res->fetch (0);
	  ipc_log ("  -- returned %lld-vector [ %lld-vectors]\n",
		   VSIZE (res), VSIZE (mo_types));
	}
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "loadMachineModel"))
    {
      String arg1 = readString (req);
#ifdef IPC_LOG
      ipc_log ("  arg = `%s'\n", arg1);
#endif
      String sts = dbeLoadMachineModel (arg1);
#ifdef IPC_LOG
      ipc_log ("  returns '%s'\n", sts ? sts : "NULL");
#endif
      writeString (sts, req);
      free (arg1);
    }
  else if (!strcmp (inp, "getMachineModel"))
    {
      String sts = dbeGetMachineModel ();
#ifdef IPC_LOG
      ipc_log ("  returns '%s'\n", sts ? sts : "NULL");
#endif
      writeString (sts, req);
    }
  else if (!strcmp (inp, "getCPUVerMachineModel"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<char*> *res = dbeGetCPUVerMachineModel (arg1);
      writeArray (res, req);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "listMachineModels"))
    {
      Vector<String> *res = dbeListMachineModels ();
#ifdef IPC_LOG
      if (res != NULL)
	ipc_log ("  returns = %lld strings\n", VSIZE (res));
      else
	ipc_log ("  returns NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "defineMemObj"))
    {
      String arg1 = readString (req);
      String arg2 = readString (req);
      String arg3 = readString (req);
      String arg4 = readString (req);
#ifdef IPC_LOG
      ipc_log ("  args = %s, %s, %s, %s\n", arg1, arg2, arg3 == NULL ? "NULL" : arg3, arg4 == NULL ? "NULL" : arg4);
#endif
      String sts = dbeDefineMemObj (arg1, arg2, NULL, arg3, arg4);
#ifdef IPC_LOG
      ipc_log ("  returns '%s'\n", sts ? sts : "NULL");
#endif
      writeString (sts, req);
      free (arg1);
      free (arg2);
      free (arg3);
      free (arg4);
    }
  else if (!strcmp (inp, "deleteMemObj"))
    {
      String arg1 = readString (req);
#ifdef IPC_LOG
      ipc_log ("  args = %s\n", arg1);
#endif
      String sts = dbeDeleteMemObj (arg1);
#ifdef IPC_LOG
      ipc_log ("  returns '%s'\n", sts ? sts : "NULL");
#endif
      writeString (sts, req);
      free (arg1);
    }
  else if (!strcmp (inp, "getIndxObjDescriptions"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetIndxObjDescriptions (arg1);
#ifdef IPC_LOG
      if (res == NULL)
	ipc_log ("  -- returned NULL\n");
      else
	{
	  Vector<int> *indxo_types = (Vector<int> *)res->fetch (0);
	  ipc_log ("  -- returned %lld-vector [ %lld-vectors]\n",
		   VSIZE (res), VSIZE (indxo_types));
	}
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getCustomIndxObjects"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetCustomIndxObjects (arg1);
#ifdef IPC_LOG
      if (res == NULL)
	ipc_log ("  -- returned NULL\n");
      else
	{
	  Vector<char *> *indxo_names = (Vector<char *> *)res->fetch (0);
	  ipc_log ("  -- returned %lld-vector [ %lld-vectors]\n",
		   VSIZE (res), VSIZE (indxo_names));
	}
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "defineIndxObj"))
    {
      String arg1 = readString (req);
      String arg2 = readString (req);
      String arg3 = readString (req);
      String arg4 = readString (req);
      ipc_log ("  args = %s, %s, %s, %s\n", arg1, arg2, arg3 == NULL ? "NULL" : arg3, arg4 == NULL ? "NULL" : arg4);
      String sts = dbeDefineIndxObj (arg1, arg2, arg3, arg4);
      ipc_log ("  returns '%s'\n", sts ? sts : "NULL");
      writeString (sts, req);
      free (arg1);
      free (arg2);
      free (arg3);
      free (arg4);
    }
  else if (!strcmp (inp, "setSelObj"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      int arg3 = readInt (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, %ld, %s, %d\n", arg1, (long) arg2, table_name (arg3), arg4);
      dbeSetSelObj (arg1, arg2, arg3, arg4);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "setSelObjV2"))
    {
      int arg1 = readInt (req);
      uint64_t arg2 = readLong (req);
      ipc_log ("  args = %d, %lld\n", arg1, (long long) arg2);
      dbeSetSelObjV2 (arg1, arg2);
      writeString (NULL, req);
    }
  else if (!strcmp (inp, "getSelObj"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %s, %d\n", arg1, table_name (arg2), arg3);
      Obj i = dbeGetSelObj (arg1, arg2, arg3);
      ipc_log ("  returns = %ld (0x%08lx)\n", (long) i, (long) i);
      writeObject (i, req);
    }
  else if (!strcmp (inp, "getSelObjV2"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      ipc_log ("  arg1 = %d  agr2 = %s\n", arg1, arg2 ? arg2 : "NULL");
      Obj res = dbeGetSelObjV2 (arg1, arg2);
      ipc_log ("  returns = %lld\n", (long long) res);
      writeObject (res, req);
      free (arg2);
    }
  else if (!strcmp (inp, "getSelObjIO"))
    {
      int arg1 = readInt (req);
      uint64_t arg2 = readLong (req);
      int arg3 = readInt (req);
      ipc_log ("  arg1 = %d, arg2 = %lld, arg3 = %d\n", arg1, (long long) arg2, arg3);
      Vector<uint64_t> *res = dbeGetSelObjIO (arg1, arg2, arg3);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getSelObjsIO"))
    {
      int arg1 = readInt (req);
      Vector<uint64_t> *arg2 = (Vector<uint64_t>*)readArray (req);
      int arg3 = readInt (req);
      ipc_log ("  arg1 = %d, arg2 size = %lld, arg3 = %d\n",
	       arg1, VSIZE (arg2), arg3);
      Vector<uint64_t> *res = dbeGetSelObjsIO (arg1, arg2, arg3);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getSelObjHeapTimestamp"))
    {
      int arg1 = readInt (req);
      uint64_t arg2 = readLong (req);
      ipc_log ("  arg1 = %d, arg2 = %llu\n", arg1, (unsigned long long) arg2);
      uint64_t st = dbeGetSelObjHeapTimestamp (arg1, arg2);
      ipc_log ("  returns = %llu\n", (unsigned long long) st);
      writeLong (st, req);
    }
  else if (!strcmp (inp, "getSelObjHeapUserExpId"))
    {
      int arg1 = readInt (req);
      uint64_t arg2 = readLong (req);
      ipc_log ("  arg1 = %d, arg2 = %llu\n", arg1, (unsigned long long) arg2);
      int userExpId = dbeGetSelObjHeapUserExpId (arg1, arg2);
      ipc_log ("  returns = %d\n", userExpId);
      writeInt (userExpId, req);
    }
  else if (!strcmp (inp, "getSelIndex"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      int arg3 = readInt (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, 0x%08lx, %s, %d\n", arg1, (long) arg2, table_name (arg3), arg4);
      int i = dbeGetSelIndex (arg1, arg2, arg3, arg4);
      ipc_log ("  returns = %d\n", i);
      writeInt (i, req);
    }
  else if (!strcmp (inp, "printData"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      String arg4 = readString (req);
      String arg5 = readString (req);
      ipc_log ("  args = %d, %s, %d, `%s', `%s'\n",
	       arg1, table_name (arg2), arg3,
	       (arg4 == NULL ? "NULL" : arg4),
	       (arg5 == NULL ? "NULL" : arg5));
      String res = dbePrintData (arg1, arg2, arg3, arg4, arg5, NULL);
      writeString (res, req);
      free (arg4);
      free (arg5);
      free (res);
    }
  else if (!strcmp (inp, "getPrintLimit"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      int i = dbeGetPrintLimit (arg1);
      ipc_log ("  returns = %d\n", i);
      writeInt (i, req);
    }
  else if (!strcmp (inp, "setPrintLimit"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  args = %d, %d\n", arg1, arg2);
      String res = dbeSetPrintLimit (arg1, arg2);
      writeString (res, req);
      free (res);
    }
  else if (!strcmp (inp, "getPrintMode"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      int i = dbeGetPrintMode (arg1);
      ipc_log ("  returns = %d\n", i);
      writeInt (i, req);
    }
  else if (!strcmp (inp, "setPrintMode"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      ipc_log ("  args = %d, %s\n", arg1, arg2);
      String res = dbeSetPrintMode (arg1, arg2);
      writeString (res, req);
      free (arg2);
      free (res);
    }
  else if (!strcmp (inp, "getPrintDelim"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      char i = dbeGetPrintDelim (arg1);
      ipc_log ("  returns = %c\n", i);
      writeInt ((int) i, req);
    }
  else if (!strcmp (inp, "getHotMarks"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  args = %d, %s (%d) \n", arg1, table_name (arg2), arg2);
      Vector<void*> *res = dbeGetHotMarks (arg1, arg2);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getHotMarksInc"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      ipc_log ("  args = %d, %s (%d) \n", arg1, table_name (arg2), arg2);
      Vector<void*> *res = dbeGetHotMarksInc (arg1, arg2);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getSummaryHotMarks"))
    {
      int arg1 = readInt (req);
      Vector<Obj> *arg2 = (Vector<Obj>*)readArray (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, 0x%llx, %s (%d)\n", arg1, (long long) arg2, table_name (arg3), arg3);
      Vector<void*> *res = dbeGetSummaryHotMarks (arg1, arg2, arg3);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncId"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, %s, %d, %d\n", arg1, table_name (arg2), arg3, arg4);
      Vector<uint64_t> *res = dbeGetFuncId (arg1, arg2, arg3, arg4);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getFuncCalleeInfo"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      Vector<int> *arg3 = (Vector<int>*)readArray (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, %s, %lld, %d\n", arg1, table_name (arg2), VSIZE (arg3), arg4);
      Vector<void*> *res = dbeGetFuncCalleeInfo (arg1, arg2, arg3, arg4);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncCallerInfo"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      Vector<int> *arg3 = (Vector<int>*)readArray (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, %s, %lld, %d\n", arg1, table_name (arg2), VSIZE (arg3), arg4);
      Vector<void*> *res = dbeGetFuncCallerInfo (arg1, arg2, arg3, arg4);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setFuncData"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      int arg3 = readInt (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, %ld, %s, %d\n", arg1, (long) arg2, table_name (arg3), arg4);
      int i = dbeSetFuncData (arg1, arg2, arg3, arg4);
      ipc_log ("  returns = %d\n", i);
      writeInt (i, req);
    }
  else if (!strcmp (inp, "setFuncDataV2"))
    {
      int dbevindex = readInt (req);
      Obj sel_obj = readObject (req);
      int type = readInt (req);
      int subtype = readInt (req);
      Vector<long long> *longs = new Vector<long long>(2);
      Vector<char *> *strings = new Vector<char *>(2);

      longs->append (dbeSetFuncData (dbevindex, sel_obj, type, subtype));
      strings->append (dbeGetMsg (dbevindex, ERROR_MSG));
      String sf_name = NULL;
      long long sf_id = 0;
      switch (type)
	{
	case DSP_SOURCE:
	case DSP_DISASM:
	  {
	    Histable *obj = (Histable *) sel_obj;
	    if (obj)
	      {
		Histable *sf = obj->convertto (Histable::SOURCEFILE);
		if (sf)
		  {
		    sf_id = sf->id;
		    sf_name = dbe_strdup (sf->get_name ());
		  }
	      }
	    break;
	  }
	}
      longs->append (sf_id);
      strings->append (sf_name);
      ipc_log ("  setFuncData(%d, %ld, %s, %d)  returns (%lld, %lld)\n   (%s, %s)\n",
	       dbevindex, (long) sel_obj, table_name (type), subtype, longs->get (0), longs->get (1),
	       STR (strings->get (0)), STR (strings->get (1)));

      Vector<void *> *res = new Vector<void *>(2);
      res->append (longs);
      res->append (strings);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncList"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %s, %d\n", arg1, table_name (arg2), arg3);
      Vector<void*> *res = dbeGetFuncList (arg1, arg2, arg3);
#ifdef IPC_LOG
      if (res != NULL)
	ipc_log ("  returns = %lld objects, length = %lld\n",
		 VSIZE (res), VSIZE ((Vector<int>*)res->fetch (0)));
      else
	ipc_log ("  returns NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncListV2"))
    {
      int dbevindex = readInt (req);
      int mtype = readInt (req);
      Obj sel_obj = readObject (req);
      int type = readInt (req);
      int subtype = readInt (req);
      Vector<void*> *res = dbeGetFuncListV2 (dbevindex, mtype, sel_obj, type, subtype);
      ipc_log ("  args = %d 0x%x %ld, %s, %d returns = %d objects, length = %d\n",
	       dbevindex, mtype, (long) sel_obj, table_name (type), subtype,
	       (int) (res ? res->size () : 0),
	       (int) (res ? ((Vector<int>*)res->fetch (0))->size () : 0));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncListMini"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %s, %d\n", arg1, table_name (arg2), arg3);
      Vector<void*> *res = dbeGetFuncListMini (arg1, arg2, arg3);
#ifdef IPC_LOG
      if (res != NULL)
	ipc_log ("  returns = %lld objects, length = %lld\n",
		 VSIZE (res), VSIZE ((Vector<int>*)res->fetch (0)));
      else
	ipc_log ("  returns NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "dbeGetTotals"))
    {
      int dbevindex = readInt (req);
      int dsptype = readInt (req);
      int subtype = readInt (req);
      Vector<void *> *res = dbeGetTotals (dbevindex, dsptype, subtype);
      ipc_log ("  dbeGetTotals(%d, %d, %d) returns %lld objects\n",
	       dbevindex, dsptype, subtype, VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getComparableObjsV2"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      int arg3 = readInt (req);
      Vector<Obj> *res = dbeGetComparableObjsV2 (arg1, arg2, arg3);
      ipc_log ("  args = %d 0x%lx %d\n", arg1, (long) arg2, arg3);
      ipc_dump ("getComparableObjsV2:res", res);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "dbeConvertSelObj"))
    {
      Obj obj = readObject (req);
      int type = readInt (req);
      Obj res = dbeConvertSelObj (obj, type);
      ipc_log ("  args = %lld %d res=%lld \n", (long long) obj, type,
	       (long long) res);
      writeObject (res, req);
    }
  else if (!strcmp (inp, "getTableDataV2"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      String arg3 = readString (req);
      String arg4 = readString (req);
      String arg5 = readString (req);
      Vector<uint64_t> *arg6 = (Vector<uint64_t>*)readArray (req);
      ipc_log ("  args = %d, %s, %s, %s, %s, %lld\n", arg1, STR (arg2),
	       STR (arg3), STR (arg4), STR (arg5), VSIZE (arg6));
      Vector<void*> *res = dbeGetTableDataV2 (arg1, arg2, arg3, arg4, arg5, arg6);
#ifdef IPC_LOG
      if (res != NULL)
	ipc_log ("  returns = %lld objects, length = %lld\n",
		 VSIZE (res), VSIZE ((Vector<int>*)res->fetch (0)));
      else
	ipc_log ("  returns NULL\n");
#endif
      writeArray (res, req);
      //destroy( arg6 );
      destroy (res);
    }
  else if (!strcmp (inp, "getCallTreeNumLevels"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      int res = dbeGetCallTreeNumLevels (arg1);
#ifdef IPC_LOG
      ipc_log ("  returns = %d\n", res);
#endif
      writeInt (res, req);
    }
  else if (!strcmp (inp, "getCallTreeLevel"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %s, %d\n", arg1, arg2, arg3);
      Vector<void*> *res = dbeGetCallTreeLevel (arg1, arg2, arg3);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getCallTreeChildren"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      Vector<int> *arg3 = (Vector<int> *) readArray (req); /*NodeIdx array*/
      ipc_log ("  args = %d, %s, vec_size=%lld\n", arg1, arg2, (long long) (arg3 ? arg3->size () : 0));
      Vector<void*> *res = dbeGetCallTreeChildren (arg1, arg2, arg3);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getCallTreeLevels"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      ipc_log ("  args = %d, %s\n", arg1, arg2);
      Vector<void*> *res = dbeGetCallTreeLevels (arg1, arg2);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getCallTreeLevelFuncs"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %d, %d\n", arg1, arg2, arg3);
      Vector<void*> *res = dbeGetCallTreeLevelFuncs (arg1, arg2, arg3);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getCallTreeFuncs"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetCallTreeFuncs (arg1);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getGroupIds"))
    {
      int arg1 = readInt (req);
      Vector<int> *res = dbeGetGroupIds (arg1);
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getNames"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      Obj arg3 = readObject (req);
#ifdef IPC_LOG
      ipc_log ("  args = %d, %s 0x%lx\n", arg1, table_name (arg2), (long) arg3);
#endif
      Vector<String> *res = dbeGetNames (arg1, arg2, arg3);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getTotalMax"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %s, %d\n", arg1, table_name (arg2), arg3);
      Vector<void*> *res = dbeGetTotalMax (arg1, arg2, arg3);
#ifdef IPC_LOG
      if (res != NULL)
	ipc_log ("  returns = %lld vectors, length %lld\n",
		 VSIZE (res), VSIZE ((Vector<void*>*)res->fetch (0)));
      else
	ipc_log ("  returns NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "composeFilterClause"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      Vector<int> *arg4 = (Vector<int>*)readArray (req);
      ipc_log ("  args = %d, %s, %d, %lld selections\n",
	       arg1, table_name (arg2), arg3, VSIZE (arg4));
      String s = dbeComposeFilterClause (arg1, arg2, arg3, arg4);
      ipc_log ("  returns %s\n", (s == NULL ? "<NULL>" : s));
      writeString (s, req);
    }
  else if (!strcmp (inp, "getStatisOverviewList"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<Object> *res = dbeGetStatisOverviewList (arg1);
      ipc_log ("  dbeStatisGetOverviewList returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getStatisList"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<Object> *res = dbeGetStatisList (arg1);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getSummary"))
    {
      int arg1 = readInt (req);
      Vector<Obj> *arg2 = (Vector<Obj>*)readArray (req);
      int arg3 = readInt (req);
      int arg4 = readInt (req);
      ipc_log ("  args = %d, 0x%llx, %s (%d), %d\n", arg1, (long long) arg2, table_name (arg3), arg3, arg4);
      Vector<Object> *res = dbeGetSummary (arg1, arg2, arg3, arg4);
      ipc_log ("  dbeGetSummary returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getSummaryV2"))
    {
      int dbevindex = readInt (req);
      Vector<Obj> *sel_objs = (Vector<Obj>*)readArray (req);
      int type = readInt (req);
      int subtype = readInt (req);
      Vector<void*> *res = dbeGetSummaryV2 (dbevindex, sel_objs, type, subtype);
      ipc_log ("  args = %d, [%lld], %s (%d), %d res=[%lld] 0x%llx \n",
	       dbevindex, VSIZE (sel_objs), table_name (type), type, subtype,
	       VSIZE (res), (unsigned long long) res);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getExpName1"))
    {
      // XXX add an argument = DbeView index
      String arg1 = readString (req);
      ipc_log ("  arg = `%s'\n", arg1 ? arg1 : "NULL");
      String res = dbeGetExpName (0, arg1);
      writeString (res, req);
      ipc_log ("  returns `%s'\n", res ? res : "NULL");
      free (arg1);
      free (res);
    }
  else if (!strcmp (inp, "getHwcHelp"))
    {
      // XXX add an argument = DbeView index
      bool forKernel = readBoolean (req);
      Vector<String> *res = dbeGetHwcHelp (0, forKernel);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getHwcSets"))
    {
      // XXX add an argument = DbeView index
      bool forKernel = readBoolean (req);
      Vector<Vector<char*>*> *res = dbeGetHwcSets (0, forKernel);
      writeArray (res, req);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getHwcsAll"))
    {
      // XXX add an argument = DbeView index
      bool forKernel = readBoolean (req);
      Vector<void*> *res = dbeGetHwcsAll (0, forKernel);
      writeArray (res, req);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getHwcAttrList"))
    {
      // XXX add an argument = DbeView index
      bool forKernel = readBoolean (req);
      Vector<char*> *res = dbeGetHwcAttrList (0, forKernel);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getHwcMaxConcurrent"))
    {
      // XXX add an argument = DbeView index
      bool forKernel = readBoolean (req);
      int res = dbeGetHwcMaxConcurrent (0, forKernel);
      writeInt (res, req);
    }
  else if (!strcmp (inp, "getIfreqData"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<char*> *res = dbeGetIfreqData (arg1);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getNewLeakListInfo"))
    {
      int arg1 = readInt (req);
      bool arg2 = readBoolean (req);
      ipc_log ("  args = %d, %d\n", arg1, arg2);
      Vector<void*> *res = dbeGetLeakListInfo (arg1, arg2);
      ipc_log ("  returns %lld void*'s\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getObject"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      Obj arg3 = readObject (req);
      Obj i = dbeGetObject (arg1, arg2, arg3);
      writeObject (i, req);
    }
  else if (!strcmp (inp, "getExpVerboseName"))
    {
      Vector<int> *arg = (Vector<int>*)readArray (req);
      ipc_log ("  expIds = %lld\n", VSIZE (arg));
      Vector<String> *res = dbeGetExpVerboseName (arg);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getName"))
    {
      // XXX add an argument = DbeView index
      int arg1 = readInt (req);
      String res = dbeGetName (0, arg1);
      writeString (res, req);
      free (res);
    }
  else if (!strcmp (inp, "getStartTime"))
    {
      // XXX add an argument = DbeView index
      int arg1 = readInt (req);
      long long l = dbeGetStartTime (0, arg1);
      ipc_log ("  returns = %llu\n", l);
      writeLong (l, req);
    }
  else if (!strcmp (inp, "getRelativeStartTime"))
    {
      // XXX add an argument = DbeView index
      int arg1 = readInt (req);
      long long l = dbeGetRelativeStartTime (0, arg1);
      ipc_log ("  returns = %llu\n", l);
      writeLong (l, req);
    }
  else if (!strcmp (inp, "getEndTime"))
    {
      // XXX add an argument = DbeView index
      int arg1 = readInt (req);
      long long l = dbeGetEndTime (0, arg1);
      ipc_log ("  returns = %llu\n", l);
      writeLong (l, req);
    }
  else if (!strcmp (inp, "getClock"))
    {
      // XXX add an argument = DbeView index
      int arg1 = readInt (req);
      int i = dbeGetClock (0, arg1);
      writeInt (i, req);
    }
    /*
	    else if ( !strcmp( inp, "getFounderExpId" ) ) {
		    // XXX add an argument = DbeView index
		int       arg1 = readInt(req);
		int i = dbeGetFounderExpId(0, arg1 );
		writeInt( i, req );
	    }
     */
  else if (!strcmp (inp, "getEntityProps"))
    {
      int arg1 = readInt (req);
      ipc_log ("  args = %d\n", arg1);
      Vector<void*> *res = dbeGetEntityProps (arg1);
      writeArray (res, req);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getEntities"))
    {
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %d, %d\n", arg1, arg2, arg3);
      Vector<void*> *res = dbeGetEntities (arg1, arg2, arg3);
      writeArray (res, req);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getEntitiesV2"))
    {
      int arg1 = readInt (req);
      Vector<int> *arg2 = (Vector<int>*)readArray (req);
      int arg3 = readInt (req);
      ipc_log ("  args = %d, %lld, %d\n", arg1, VSIZE (arg2), arg3);
      Vector<void*> *res = dbeGetEntitiesV2 (arg1, arg2, arg3);
      writeArray (res, req);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getTLDetails"))
    {//TBR
      int arg1 = readInt (req);
      int arg2 = readInt (req);
      int arg3 = readInt (req);
      int arg4 = readInt (req);
      long long arg5 = readLong (req);
      ipc_log (" dbevindex= %d, exp_id = %d, data_id = %d, "
	       "entity_prop_id = %d, event_id = %lld\n",
	       arg1, arg2, arg3, arg4, arg5);
      Vector<void*> *res = dbeGetTLDetails (arg1, arg2, arg3, arg4, arg5);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getStackNames"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      ipc_log ("  args = %d, %ld\n", arg1, (long) arg2);
      Vector<String> *res = dbeGetStackNames (arg1, arg2);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getStackFunctions"))
    {
      // XXX add an argument = DbeView index
      Obj arg1 = readObject (req);
      ipc_log ("  args = %ld\n", (long) arg1);
      Vector<Obj> *res = dbeGetStackFunctions (0, arg1);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getStacksFunctions"))
    {
      // XXX add an argument = DbeView index
      Vector<Obj> *arg1 = (Vector<Obj>*)readArray (req);
      ipc_log ("  argc = %ld\n", (long) arg1->size ());
      Vector<void*> *res = dbeGetStacksFunctions (0, arg1);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getStackPCs"))
    {
      // XXX add an argument = DbeView index
      Obj arg1 = readObject (req);
      ipc_log ("  args = %ld\n", (long) arg1);
      Vector<Obj> *res = dbeGetStackPCs (0, arg1);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      delete res;
    }
  else if (!strcmp (inp, "getIOStatistics"))
    {
      int dbevindex = readInt (req);
      Vector<Vector<char*>*> *res = dbeGetIOStatistics (dbevindex);
      writeArray (res, req);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getHeapStatistics"))
    {
      int dbevindex = readInt (req);
      Vector<Vector<char*>*> *res = dbeGetHeapStatistics (dbevindex);
      writeArray (res, req);
      ipc_log ("  returns %lld char*'s\n", VSIZE (res));
      destroy (res);
    }
  else if (!strcmp (inp, "getSamples"))
    {
      int dbev_id = readInt (req);
      int exp_id = readInt (req);
      int64_t lo = readLong (req);
      int64_t hi = readLong (req);
      ipc_log ("  dbevindex= %d, exp_id = %d, lo_idx:%lld, hi_idx:%lld\n",
	       dbev_id, exp_id, (long long) lo, (long long) hi);
      Vector<void*> *res = dbeGetSamples (dbev_id, exp_id, lo, hi);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getGCEvents"))
    {
      int dbev_id = readInt (req);
      int exp_id = readInt (req);
      int64_t lo = readLong (req);
      int64_t hi = readLong (req);
      ipc_log ("  dbevindex= %d, exp_id = %d, lo_idx:%lld, hi_idx:%lld\n",
	       dbev_id, exp_id, (long long) lo, (long long) hi);
      Vector<void*> *res = dbeGetGCEvents (dbev_id, exp_id, lo, hi);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncNames"))
    {
      int arg1 = readInt (req);
      Vector<Obj> *arg2 = (Vector<Obj>*)readArray (req);
      ipc_log ("  arg1 = %d, arg2 absent, size = %lld\n", arg1, VSIZE (arg2));
      Vector<String> *res = dbeGetFuncNames (arg1, arg2);
      writeArray (res, req);
      delete arg2;
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncIds"))
    {
      int arg1 = readInt (req);
      Vector<Obj> *arg2 = (Vector<Obj>*)readArray (req);
      ipc_log ("  arg1 = %d, arg2 absent, size = %lld\n", arg1, VSIZE (arg2));
      Vector<uint64_t> *res = dbeGetFuncIds (arg1, arg2);
      writeArray (res, req);
      delete arg2;
      destroy (res);
    }
  else if (!strcmp (inp, "getObjNamesV2"))
    {
      int arg1 = readInt (req);
      Vector<uint64_t> *arg2 = (Vector<uint64_t>*)readArray (req);
      ipc_log ("  arg1 = %d, arg2 absent, size = %lld\n", arg1, VSIZE (arg2));
      Vector<String> *res = dbeGetObjNamesV2 (arg1, arg2);
      writeArray (res, req);
      delete arg2;
      destroy (res);
    }
  else if (!strcmp (inp, "getFuncName"))
    {
      int arg1 = readInt (req);
      Obj arg2 = readObject (req);
      ipc_log ("  arg1 = %d, arg2 = %lld\n", arg1, (long long) arg2);
      String res = dbeGetFuncName (arg1, arg2);
      ipc_log ("  returning = %s\n", res ? res : "NULL");
      writeString (res, req);
      free (res);
    }
  else if (!strcmp (inp, "getObjNameV2"))
    {
      int arg1 = readInt (req);
      uint64_t arg2 = readLong (req);
      ipc_log ("  arg1 = %d, arg2 = %llu\n", arg1, (unsigned long long) arg2);
      String res = dbeGetObjNameV2 (arg1, arg2);
      ipc_log ("  returning = %s\n", res ? res : "NULL");
      writeString (res, req);
      free (res);
    }
  else if (!strcmp (inp, "getDataspaceTypeDesc"))
    {
      // XXX add an argument = DbeView index
      Obj arg1 = readObject (req);
      ipc_log ("  arg1 absent, index = %ld\n", (long) arg1);
      String res = dbeGetDataspaceTypeDesc (0, arg1);
      ipc_log ("  returning = %s\n", res ? res : "NULL");
      writeString (res, req);
      free (res);
    }
    /*
     *   New Interface with Timeline
     */
#if 0 //YXXX TBR
  else if (!strcmp (inp, "dbeInit"))
    dbeInit ();
  else if (!strcmp (inp, "getDefaultExperimentName"))
    {
      String res = dbeGetDefaultExperimentName ();
      ipc_log ("  returning = %s\n", res);
      writeString (res);
      free (res);
    }
  else if (!strcmp (inp, "getExperimentState"))
    {
      String res = dbeGetExperimentState ();
      ipc_log ("  returning = %s\n", res);
      writeString (res);
      free (res);
    }
  else if (!strcmp (inp, "getExpStartTime"))
    {
      long long l = dbeGetExpStartTime ();
      ipc_log ("  returns = %llu\n", l);
      writeLong (l);
    }
  else if (!strcmp (inp, "getExpEndTime"))
    {
      long long l = dbeGetExpEndTime ();
      ipc_log ("  returns = %llu\n", l);
      writeLong (l);
    }
#endif
  else if (!strcmp (inp, "getDataDescriptorsV2"))
    {//TBR? TBD
      int exp_id = readInt (req);
      ipc_log (" exp_id = %d\n", exp_id);
      Vector<void*> *res = dbeGetDataDescriptorsV2 (exp_id);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getDataPropertiesV2"))
    {//TBR? TBD
      int exp_id = readInt (req);
      int arg2 = readInt (req);
      ipc_log (" exp_id = %d, data_idx = %d\n", exp_id, arg2);
      Vector<void*> *res = dbeGetDataPropertiesV2 (exp_id, arg2);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getExperimentTimeInfo"))
    {
      Vector<int> *exp_ids = (Vector<int>*)readArray (req);
      ipc_log ("  cnt = %lld\n", VSIZE (exp_ids));
      Vector<void*> *res = dbeGetExperimentTimeInfo (exp_ids);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getExperimentDataDescriptors"))
    {
      Vector<int> *exp_ids = (Vector<int>*)readArray (req);
      ipc_log ("  cnt = %lld\n", VSIZE (exp_ids));
      Vector<void*> *res = dbeGetExperimentDataDescriptors (exp_ids);
      ipc_log ("  returns = %lld objects\n", VSIZE (res));
      writeArray (res, req);
      destroy (res);
    }
#if 0 //YXXX TBR?
  else if (!strcmp (inp, "getExprValues"))
    {//TBR? TBD
      int arg1 = readInt ();
      String arg2 = readString ();
      ipc_log ("  data_idx = %d expr = %s\n", arg1, arg2 ? arg2 : "NULL");
      Vector<long long> *res = dbeGetExprValues (arg1, arg2);
      ipc_log ("  returns = %d objects\n", res ? res->size () : 0);
      writeArray (res);
      delete res;
      free (arg2);
    }
#endif
  else if (!strcmp (inp, "hasTLData"))
    {
      int dbevindex = readInt (req);
      Vector<int> *exp_ids = (Vector<int>*)readArray (req);
      Vector<int> *data_ids = (Vector<int>*)readArray (req);
      Vector<int> *eprop_ids = (Vector<int>*)readArray (req);
      Vector<int> *eprop_vals = (Vector<int>*)readArray (req);
      Vector<int> *auxs = (Vector<int>*)readArray (req);
      ipc_log ("  dbev_id = %d, cnt = %lld\n", dbevindex, VSIZE (exp_ids));
      Vector<bool> *res = dbeHasTLData (dbevindex,
					exp_ids, data_ids, eprop_ids, eprop_vals, auxs);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getTLData"))
    {
      int dbevindex = readInt (req);
      int exp_id = readInt (req);
      int tldata_type = readInt (req);
      int entity_prop_id = readInt (req);
      int entity_prop_val = readInt (req);
      int aux = readInt (req);
      long long arg5 = readLong (req);
      long long arg6 = readLong (req);
      int arg7 = readInt (req);
      bool getReps = readBoolean (req);
      Vector<String> *secondaryProps = (Vector<String>*)readArray (req);

      ipc_log ("  args = %d:%d; tldata_type=%d entity_prop_id=%d ent=%d aux=%d"
	       "\n    tstart=%lld delta=%lld ndeltas=%d getReps=%d nProps=%lld\n",
	       dbevindex, exp_id,
	       tldata_type, entity_prop_id, entity_prop_val, aux,
	       arg5, arg6, arg7, (int) getReps, VSIZE (secondaryProps));
      Vector<void*> *res = dbeGetTLData (dbevindex, exp_id,
					 tldata_type, entity_prop_id, entity_prop_val, aux,
					 arg5, arg6, arg7, getReps, secondaryProps);
#ifdef IPC_LOG
      if (res)
	{
	  Vector<Obj> *reps = (Vector<Obj>*)res->fetch (0);
	  Vector<Obj> *props = (Vector<Obj>*)res->fetch (1);
	  if (reps)
	    {
	      Vector <long long> *fids = (Vector <long long> *)reps->fetch (2);
	      int sz = fids ? fids->size () : 0;
	      ipc_log ("  returning TL reps (dDscrs); nreps=%d:", sz);
	      int i;
	      for (i = 0; i < sz && i < 7; i++)
		ipc_log (" %lld", fids->fetch (i));
	      if (i < sz)
		ipc_log (" ... %lld", fids->fetch (sz - 1));
	      ipc_log ("\n");
	    }
	  if (props)
	    {
	      int nprops = props->size ();
	      ipc_log ("  returning values for %d properties:\n", nprops);
	      assert (secondaryProps->size () == nprops);
	    }
	}
      else
	ipc_log ("  returning NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
      destroy (secondaryProps);
    }
  else if (!strcmp (inp, "getTLEventCenterTime"))
    {
      int dbevindex = readInt (req);
      int exp_id = readInt (req);
      int tldata_type = readInt (req);
      int entity_prop_id = readInt (req);
      int entity_prop_val = readInt (req);
      int aux = readInt (req);
      long long event_id = readLong (req);
      long long move_count = readLong (req);
      ipc_log ("  args = %d:%d; tldata_type = %d entity_prop_id = %d "
	       "ent = %d aux = %d idx = %lld move=%lld\n",
	       dbevindex, exp_id,
	       tldata_type, entity_prop_id, entity_prop_val, aux, event_id, move_count);
      Vector<long long> * res = dbeGetTLEventCenterTime (dbevindex, exp_id,
							 tldata_type, entity_prop_id, entity_prop_val, aux, event_id, move_count);
      ipc_log ("  returning  idx = %lld, time = %lld\n",
	       res ? res->fetch (0) : -1, res ? res->fetch (1) : -1);
      writeArray (res, req);
    }
  else if (!strcmp (inp, "getTLEventIdxNearTime"))
    {
      int dbevindex = readInt (req);
      int exp_id = readInt (req);
      int tldata_type = readInt (req);
      int entity_prop_id = readInt (req);
      int entity_prop_val = readInt (req);
      int aux = readInt (req);
      int searchDirection = readInt (req);
      long long value = readLong (req);
      ipc_log ("  args = %d:%d; tldata_type = %d entity_prop_id = %d "
	       "ent = %d aux = %d direction = %d value = %lld(0x%llx)\n",
	       dbevindex, exp_id,
	       tldata_type, entity_prop_id, entity_prop_val, aux,
	       searchDirection, value, value);
      long long res = dbeGetTLEventIdxNearTime (dbevindex, exp_id,
						tldata_type, entity_prop_id, entity_prop_val, aux,
						searchDirection, value);
      ipc_log ("  returning = %lld\n", res);
      writeLong (res, req);
    }
  else if (!strcmp (inp, "getAggregatedValue"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      String arg3 = readString (req);
      String arg4 = readString (req);
      long long arg5 = readLong (req);
      long long arg6 = readLong (req);
      int arg7 = readInt (req);
      String arg8 = readString (req);
      String arg9 = readString (req);
      ipc_log ("  data_idx = %d lfilter = \"%s\" fexpr = \"%s\" "
	       "time = \"%s\" tstart = %lld delta = %lld "
	       "num = %d key = \"%s\" aggr = \"%s\"\n",
	       arg1, arg2 ? arg2 : "NULL", arg3 ? arg3 : "NULL",
	       arg4 ? arg4 : "NULL", arg5, arg6,
	       arg7, arg8 ? arg8 : "NULL", arg9 ? arg9 : "NULL");
      Vector<long long> *res = dbeGetAggregatedValue (arg1, arg2, arg3,
						      arg4, arg5, arg6, arg7, arg8, arg9);
#ifdef IPC_LOG
      if (res)
	{
	  int sz = res->size ();
	  ipc_log ("  returning = %d values:", sz);
	  if (sz > 10)
	    sz = 10;
	  for (int i = 0; i < sz; i++)
	    ipc_log (" %lld", res->fetch (i));
	  ipc_log ("\n");
	}
      else
	ipc_log ("  returning NULL\n");
#endif
      writeArray (res, req);
      delete res;
      free (arg2);
      free (arg3);
      free (arg4);
      free (arg8);
      free (arg9);
    }
#if 0//YXXX TBR
  else if (!strcmp (inp, "getExprValue"))
    {
      int exp_id = readInt ();
      int arg1 = readInt ();
      int arg2 = readInt ();
      String arg3 = readString ();
      ipc_log ("  exp_id %d, data_id = %d, event_id = %d, expr = %s\n",
	       exp_id, arg1, arg2, arg3 ? arg3 : "NULL");
      String res = dbeGetExprValue (exp_id, arg1, arg2, arg3);
      ipc_log ("  returning = %s\n", res ? res : "");
      writeString (res);
      free (res);
      free (arg3);
    }
  else if (!strcmp (inp, "getListValues"))
    {
      Obj arg1 = readObject ();
      ipc_log ("  stack = %lu\n", (long) arg1);
      Vector<Obj> *res = dbeGetListValues (arg1);
      ipc_log ("  returns = %d objects\n", res ? res->size () : 0);
      writeArray (res);
      destroy (res);
    }
  else if (!strcmp (inp, "getListNames"))
    {
      Obj arg1 = readObject ();
      ipc_log ("  stack = %lu\n", (long) arg1);
      Vector<String> *res = dbeGetListNames (arg1);
      ipc_log ("  returns = %d objects\n", res ? res->size () : 0);
      writeArray (res);
      destroy (res);
    }
#endif
  else if (!strcmp (inp, "getLineInfo"))
    {
      Obj arg1 = readObject (req);
      ipc_log ("  pc = %lu\n", (long) arg1);
      Vector<String> *res = dbeGetLineInfo (arg1);
      ipc_log ("  returning File name: '%s'\n", res ? res->fetch (0) : "");
      ipc_log ("  returning Lineno:    '%s'\n", res ? res->fetch (1) : "");
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "setAlias"))
    {
      String arg1 = readString (req);
      String arg2 = readString (req);
      String arg3 = readString (req);
      ipc_log ("  name=\"%s\" uname=\"%s\" expr=\"%s\"\n",
	       arg1 ? arg1 : "", arg2 ? arg2 : "", arg3 ? arg3 : "");
      int res = dbeSetAlias (arg1, arg2, arg3);
      ipc_log ("  returning = %d\n", res);
      writeInt (res, req);
    }
  else if (!strcmp (inp, "getAlias"))
    {
      String arg1 = readString (req);
      ipc_log ("  name=\"%s\"\n", arg1 ? arg1 : "");
      Vector<char*> *res = dbeGetAlias (arg1);
      ipc_log ("  returning uname: '%s'\n", res && res->fetch (0) ? res->fetch (0) : "");
      ipc_log ("  returning expr:  '%s'\n", res && res->fetch (1) ? res->fetch (0) : "");
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getXYPlotData"))
    {
      int arg1 = readInt (req);
      String arg2 = readString (req);
      String arg3 = readString (req);
      String arg4 = readString (req);
      String arg5 = readString (req);
      String arg6 = readString (req);
      String arg7 = readString (req);
      String arg8 = readString (req);
      String arg9 = readString (req);
      ipc_log ("  data_idx = %d lfilter = \"%s\" arg = \"%s\" "
	       "func1 = \"%s\" aggr1 = \"%s\" "
	       "func2 = \"%s\" aggr2 = \"%s\" "
	       "func3 = \"%s\" aggr3 = \"%s\" \n",
	       arg1, arg2 ? arg2 : "NULL", arg3 ? arg3 : "NULL",
	       arg4 ? arg4 : "NULL", arg5 ? arg5 : "NULL", arg6 ? arg6 : "NULL",
	       arg7 ? arg7 : "NULL", arg8 ? arg8 : "NULL", arg9 ? arg9 : "NULL");
      Vector<Vector<long long>*> *res = dbeGetXYPlotData (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);

#ifdef IPC_LOG
      if (res)
	{
	  long nvals = res->size ();
	  for (long i = 0; i < nvals; ++i)
	    {
	      Vector<long long> *vals = res->fetch (i);
	      long long sz = VSIZE (vals);
	      ipc_log ("  returning = %lld values:", sz);
	      if (sz > 10)
		sz = 10;
	      for (long j = 0; j < sz; j++)
		ipc_log (" %lld", vals->fetch (j));
	      ipc_log ("\n");
	    }
	}
      else
	ipc_log ("  returning NULL\n");
#endif
      writeArray (res, req);
      destroy (res);
    }
  else if (strcmp (inp, "dbe_archive") == 0)
    {
      Vector<long long> *ids = (Vector<long long> *) readArray (req);
      Vector<const char*> *locations = (Vector<const char*> *) readArray (req);
      dbe_archive (ids, locations);
      delete ids;
      destroy (locations);
      writeResponseGeneric (RESPONSE_STATUS_SUCCESS, req->getRequestID (), req->getChannelID ());
    }
  else if (strcmp (inp, "dbeSetLocations") == 0)
    {
      Vector<const char*> *fnames = (Vector<const char*> *) readArray (req);
      Vector<const char*> *locations = (Vector<const char*> *) readArray (req);
      dbeSetLocations (fnames, locations);
      destroy (fnames);
      destroy (locations);
      writeResponseGeneric (RESPONSE_STATUS_SUCCESS, req->getRequestID (), req->getChannelID ());
    }
  else if (strcmp (inp, "dbeResolvedWith_setpath") == 0)
    {
      char *path = readString (req);
      Vector<void *> *res = dbeResolvedWith_setpath (path);
      free (path);
      writeArray (res, req);
      destroy (res);
    }
  else if (strcmp (inp, "dbeResolvedWith_pathmap") == 0)
    {
      char *old_prefix = readString (req);
      char *new_prefix = readString (req);
      Vector<void *> *res = dbeResolvedWith_pathmap (old_prefix, new_prefix);
      free (old_prefix);
      free (new_prefix);
      writeArray (res, req);
      destroy (res);
    }
  else if (!strcmp (inp, "getCollectorControlValue"))
    {
      /* int dbevindex =*/ readInt (req);
      char *control = readString (req);
      ipc_log ("  args = %s\n", control);
      char *ret = dbeGetCollectorControlValue (control);
      ipc_log ("  returning %s\n", STR (ret));
      writeString (ret, req);
    }
  else if (!strcmp (inp, "setCollectorControlValue"))
    {
      /* int dbevindex =*/ readInt (req);
      char *control = readString (req);
      char *value = readString (req);
#ifdef IPC_LOG
      ipc_log ("  args = %s %s\n", control, value);
#endif
      char *ret = dbeSetCollectorControlValue (control, value);
#ifdef IPC_LOG
      if (ret)
	ipc_log ("  returning %s\n", ret);
      else
	ipc_log ("  returning NULL\n");
#endif
      writeString (ret, req);
    }
  else if (!strcmp (inp, "unsetCollectorControlValue"))
    {
      /* int dbevindex =*/ readInt (req);
      char *control = readString (req);
      ipc_log ("  args = %s\n", control);
      char *ret = dbeUnsetCollectorControlValue (control);
      ipc_log ("  returning %s\n", STR (ret));
      writeString (ret, req);
    }
  else if (!strcmp (inp, "getSignalValue"))
    {
      String arg1 = readString (req);
      ipc_log ("  arg1=\"%s\"\n", arg1 ? arg1 : "");
      int res = dbeGetSignalValue (arg1);
      ipc_log ("  returning = %d\n", res);
      writeInt (res, req);
    }
  else if (!strcmp (inp, "sendSignal"))
    {
      long long p = readLong (req);
      int signum = readInt (req);
      ipc_log ("  args = %llu, %d\n", (long long) p, signum);
      char * ret = dbeSendSignal ((pid_t) p, signum);
#ifdef IPC_LOG
      if (ret)
	ipc_log ("  returning %s\n", ret);
      else
	ipc_log ("  returning NULL\n");
#endif
      writeString (ret, req);
    }
  else if (!strcmp (inp, "checkConnection"))
    {
      String arg1 = readString (req);
      ipc_log ("  arg = `%s'\n", arg1 ? arg1 : "NULL");
      String res = dbeCheckConnection (arg1);
      writeString (res, req);
      ipc_log ("  returns `%s'\n", res ? res : "NULL");
      free (arg1);
      free (res);
    }
  else if (!strcmp (inp, "QUIT"))
    {
#ifdef IPC_LOG
      ipc_log ("  %s\n", inp);
#endif
      exit (0);
    }
  else
    {
      ipc_log ("Unrecognized input cmd \"%s\"; Aborting.\n", inp);
      return 1;
    }
  ipc_log ("  processing IPC command %s complete\n", inp);
  free (inp);
  fflush (stdout);
  if (req->getStatus () != CANCELLED_IMMEDIATE)
    // wake up the main working thread, let it take care of delete
    req->setStatus (COMPLETED);
  delete req;
  return 0;
}

void
check_env_args (int argc, char *argv[])
{
  int indx = 2; // Skip "-IPC"
  const char *MINUS_E = "-E";
  const char *SP_ER_PRINT_TRACE_LEVEL = "SP_ER_PRINT_TRACE_LEVEL";
  const char *SP_IPC_PROTOCOL = "SP_IPC_PROTOCOL";
  const char SEPARATOR = '=';
  char *cmd_env_var = NULL;
  while (argc - indx >= 2)
    {
      char *option = argv[indx++];
      if (!streq (option, MINUS_E))
	continue;
      cmd_env_var = argv[indx++];
      char *separator = strchr (cmd_env_var, SEPARATOR);
      if (!separator)
	// Unrecognized option. Fatal error?
	continue;
      char *cmd_env_var_val = separator + 1;
      if (!strncmp (cmd_env_var, SP_ER_PRINT_TRACE_LEVEL,
		    strlen (SP_ER_PRINT_TRACE_LEVEL)))
	{
	  if (streq (cmd_env_var_val, "1"))
	    ipc_trace_level = TRACE_LVL_1;
	  else if (streq (cmd_env_var_val, "2"))
	    ipc_trace_level = TRACE_LVL_2;
	  else if (streq (cmd_env_var_val, "3"))
	    ipc_trace_level = TRACE_LVL_3;
	  else if (streq (cmd_env_var_val, "4"))
	    ipc_trace_level = TRACE_LVL_4;
	  continue;
	}
      if (!strncmp (cmd_env_var, SP_IPC_PROTOCOL, strlen (SP_IPC_PROTOCOL)))
	{
	  if (streq (cmd_env_var_val, IPC_PROTOCOL_CURR))
	    // Only one protocol is currently supported
	    ipc_protocol = IPC_PROTOCOL_CURR;
	  else
	    ipc_protocol = IPC_PROTOCOL_UNKNOWN;
	  continue;
	}
      // Unrecognized option. Fatal error?
    }
}

void
print_ipc_protocol_confirmation ()
{
  if (NULL != ipc_protocol)
    {
      fprintf (stdout, "ER_IPC: %s\n", ipc_protocol);
      fflush (stdout);
    }
}

void
ipc_mainLoop (int argc, char *argv[])
{
  if (getenv ("GPROFNG_DBE_DELAY"))
    sleep (20);
#ifdef IPC_LOG
  ipc_flags = 1;
#endif
  // check_env_args(argc, argv);

  char *er_print_trace_level = getenv ("SP_ER_PRINT_TRACE_LEVEL");
  if (er_print_trace_level != NULL)
    {
      if (streq (er_print_trace_level, "1"))
	ipc_trace_level = TRACE_LVL_1;
      else if (streq (er_print_trace_level, "2"))
	ipc_trace_level = TRACE_LVL_2;
      else if (streq (er_print_trace_level, "3"))
	ipc_trace_level = TRACE_LVL_3;
      else if (streq (er_print_trace_level, "4"))
	ipc_trace_level = TRACE_LVL_4;
    }
  check_env_args (argc, argv);
  print_ipc_protocol_confirmation ();

  if (ipc_flags || getenv ("SP_ER_PRINT_IPC_FLAG") || ipc_trace_level > TRACE_LVL_0)
    {
      ipc_flags = 1;
      if (ipc_trace_level == TRACE_LVL_0)
	ipc_trace_level = TRACE_LVL_1;
      // reopen stderr as file "ipc_log"
      ipc_log_name = getenv ("SP_ER_PRINT_IPC_LOG");
      if (ipc_log_name == NULL)
	ipc_log_name = "ipc_log";
      freopen (ipc_log_name, "w", stderr);
      if (ipc_trace_level >= TRACE_LVL_2)
	{
	  ipc_request_log_name = "ipc_request_log";
	  ipc_response_log_name = "ipc_response_log";
	  requestLogFileP = fopen (ipc_request_log_name, "w");
	  responseLogFileP = fopen (ipc_response_log_name, "w");
	}
      else
	{
	  ipc_request_log_name = "ipc_log";
	  ipc_response_log_name = "ipc_log";
	}
      begin_time = gethrtime ();
    }
  else
    // Reopen stderr as /dev/null
    freopen ("/dev/null", "w", stderr);

  struct sigaction act;
  memset (&act, 0, sizeof (struct sigaction));
  term_flag = 0;
  /* install a handler for TERM */
  ipc_request_trace (TRACE_LVL_1, "Installing SIGTERM handler to abort on error\n");
  sigemptyset (&act.sa_mask);
  act.sa_handler = (SignalHandler) sigterm_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (SIGTERM, &act, &old_sigterm_handler) == -1)
    {
      ipc_request_trace (TRACE_LVL_1, "Unable to install SIGTERM handler\n");
      abort ();
    }
  /* install a handler for INT */
  ipc_request_trace (TRACE_LVL_1, "Installing SIGINT handler to send message to analyzer\n");
  sigemptyset (&act.sa_mask);
  act.sa_handler = (SignalHandler) sigint_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO;
  if (sigaction (SIGINT, &act, &old_sigint_handler) == -1)
    {
      ipc_request_trace (TRACE_LVL_1, "Unable to install SIGINT handler\n");
      abort ();
    }
  ipc_log ("Installed SIGINT handler to handle Ctrl-C properly\n");
  int er_print_catch_crash = 1; // Default: catch fatal signals
  char *s = getenv ("GPROFNG_ALLOW_CORE_DUMP");
  if (s && (strcasecmp (s, "no") == 0 || strcmp (s, "0") == 0))
    er_print_catch_crash = 0;
  if (er_print_catch_crash)
    {
      /* reserve memory for fatal error processing */
      fatalErrorDynamicMemory = (char *) malloc (4 * 1024 * 1024); // reserve 4 MB
      /* install a handler for SIGABRT */
      ipc_request_trace (TRACE_LVL_1, "Installing SIGABRT handler to send message to analyzer\n");
      sigemptyset (&act.sa_mask);
      act.sa_handler = (SignalHandler) sigABRT_handler;
      act.sa_flags = SA_RESTART | SA_SIGINFO;
      if (sigaction (SIGABRT, &act, NULL) == -1)
	{
	  ipc_request_trace (TRACE_LVL_1, "Unable to install SIGABRT handler\n");
	  // abort();
	}
      else
	ipc_log ("Installed SIGABRT handler to handle crash properly\n");
      /* install a handler for SIGSEGV */
      ipc_request_trace (TRACE_LVL_1, "Installing SIGABRT handler to send message to analyzer\n");
      sigemptyset (&act.sa_mask);
      act.sa_handler = (SignalHandler) sigSEGV_handler;
      act.sa_flags = SA_RESTART | SA_SIGINFO;
      if (sigaction (SIGSEGV, &act, NULL) == -1)
	{
	  ipc_request_trace (TRACE_LVL_1, "Unable to install SIGSEGV handler\n");
	  // abort();
	}
      else
	ipc_log ("Installed SIGSEGV handler to handle crash properly\n");
    }
  ipc_request_trace (TRACE_LVL_1, "Entering ipc_mainLoop; run dir `%s'\n",
		     theApplication->get_run_dir ());
  cancelRequestedChannelID = 0xFFFFFFFF;
  ipcThreadPool = new DbeThreadPool (0); // DbeThreadPool (-1);
  responseBufferPool = new BufferPool ();
  ipc_log (ipc_single_threaded_mode ?
	   "RUNNING er_print -IPC IN SINGLE THREADED MODE\n" :
	   "RUNNING er_print -IPC IN MULTITHREAD MODE\n");

  /* Send "Ready" signal to the GUI */
  setProgress (100, "Restart engine");

  /* start listening for requests */
  error_flag = 0;
  for (;;)
    {
      readRequestHeader ();
      if (term_flag == 1 || error_flag == 1)
	{
	  ipc_request_trace (TRACE_LVL_1, "SIGTERM received, exiting\n");
	  return;
	}
    }
}

static const char *
table_name (int flavor)
{
  static char def_name[64];

  switch ((FuncListDisp_type) flavor)
    {
    case DSP_FUNCTION:
      return ("FUNCTION");
    case DSP_LINE:
      return ("LINE");
    case DSP_PC:
      return ("PC");
    case DSP_SOURCE:
      return ("SOURCE");
    case DSP_DISASM:
      return ("DISASM");
    case DSP_SELF:
      return ("SELF");
    case DSP_CALLER:
      return ("CALLER");
    case DSP_CALLEE:
      return ("CALLEE");
    case DSP_CALLTREE:
      return ("CALLTREE");
    case DSP_TIMELINE:
      return ("TIMELINE");
    case DSP_STATIS:
      return ("STATIS");
    case DSP_EXP:
      return ("EXP");
    case DSP_LEAKLIST:
      return ("LEAKLIST");
    case DSP_HEAPCALLSTACK:
      return ("HEAP");
    case DSP_MEMOBJ:
      return ("MEMOBJ");
    case DSP_DATAOBJ:
      return ("DATAOBJ");
    case DSP_DLAYOUT:
      return ("DLAYOUT");
    case DSP_SRC_FILE:
      return ("SRC_FILE");
    case DSP_IFREQ:
      return ("IFREQ");
    case DSP_RACES:
      return ("RACES");
    case DSP_INDXOBJ:
      return ("INDXOBJ");
    case DSP_DUALSOURCE:
      return ("DUALSOURCE");
    case DSP_SOURCE_DISASM:
      return ("SOURCE_DISASM");
    case DSP_DEADLOCKS:
      return ("DEADLOCKS");
    case DSP_SOURCE_V2:
      return ("SOURCE_V2");
    case DSP_DISASM_V2:
      return ("DISASM_V2");
    case DSP_IOACTIVITY:
      return ("IOACTIVITY");
    case DSP_OVERVIEW:
      return ("OVERVIEW");
    case DSP_SAMPLE:
      return ("SAMPLE -- UNEXPECTED");
    default:
      snprintf (def_name, sizeof (def_name), "table number %d", flavor);
      return (def_name);
    }
}
