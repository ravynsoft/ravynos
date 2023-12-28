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
#include <stdarg.h>

#include "util.h"
#include "Emsg.h"
#include "StringBuilder.h"

// The Emsg, experiment message, has as objects I18N'd messages
//	in a structure suitable for attaching to and fetching
//	from a queue of such messages.  It is intended to
//	be used for collector errors, collector warnings, parser
//	errors, and er_archive errors that are encountered when
//	reading an experiment

// ----------------------- Message --------------------------

Emsg::Emsg (Cmsg_warn w, const char *i18n_text)
{
  warn = w;
  flavor = 0;
  par = NULL;
  text = strdup (i18n_text);
  next = NULL;
}

Emsg::Emsg (Cmsg_warn w, StringBuilder& sb)
{
  warn = w;
  flavor = 0;
  par = NULL;
  text = sb.toString ();
  next = NULL;
}

Emsg::Emsg (Cmsg_warn w, int f, const char *param)
{
  char *type;
  warn = w;
  flavor = f;
  if (param != NULL)
    par = dbe_strdup (param);
  else
    par = dbe_strdup ("");
  next = NULL;

  // determine type
  switch (warn)
    {
    case CMSG_WARN:
      type = GTXT ("*** Collector Warning");
      break;
    case CMSG_ERROR:
      type = GTXT ("*** Collector Error");
      break;
    case CMSG_FATAL:
      type = GTXT ("*** Collector Fatal Error");
      break;
    case CMSG_COMMENT:
      type = GTXT ("Comment");
      break;
    case CMSG_PARSER:
      type = GTXT ("*** Log Error");
      break;
    case CMSG_ARCHIVE:
      type = GTXT ("*** Archive Error");
      break;
    default:
      type = GTXT ("*** Internal Error");
      break;
    };

  // now convert the message to its I18N'd string
  switch (flavor)
    {
    case COL_ERROR_NONE:
      text = dbe_sprintf (GTXT ("%s: No error"), type);
      break;
    case COL_ERROR_ARGS2BIG:
      text = dbe_sprintf (GTXT ("%s: Data argument too long"), type);
      break;
    case COL_ERROR_BADDIR:
      text = dbe_sprintf (GTXT ("%s: Bad experiment directory name"), type);
      break;
    case COL_ERROR_ARGS:
      text = dbe_sprintf (GTXT ("%s: Data argument format error `%s'"), type, par);
      break;
    case COL_ERROR_PROFARGS:
      text = dbe_sprintf (GTXT ("%s: [UNUSED] Bad clock-profiling argument"), type);
      break;
    case COL_ERROR_SYNCARGS:
      text = dbe_sprintf (GTXT ("%s: [UNUSED] Bad synchronization tracing argument"), type);
      break;
    case COL_ERROR_HWCARGS:
      text = dbe_sprintf (GTXT ("%s: Bad hardware counter profiling argument"), type);
      break;
    case COL_ERROR_DIRPERM:
      text = dbe_sprintf (GTXT ("%s: Experiment directory is not writeable; check umask and permissions"), type);
      break;
    case COL_ERROR_NOMSACCT:
      text = dbe_sprintf (GTXT ("%s: Turning on microstate accounting failed"), type);
      break;
    case COL_ERROR_PROFINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing clock-profiling failed"), type);
      break;
    case COL_ERROR_SYNCINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing synchronization tracing failed"), type);
      break;
    case COL_ERROR_HWCINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing hardware counter profiling failed -- %s"), type, par);
      break;
    case COL_ERROR_HWCFAIL:
      text = dbe_sprintf (GTXT ("%s: HW counter data collection failed; likely cause is that another process preempted the counters"), type);
      break;
    case COL_ERROR_EXPOPEN:
      text = dbe_sprintf (GTXT ("%s: Experiment initialization failed, %s"), type, par);
      break;
    case COL_ERROR_SIZELIM:
      text = dbe_sprintf (GTXT ("%s: Experiment size limit exceeded, writing %s"), type, par);
      break;
    case COL_ERROR_SYSINFO:
      text = dbe_sprintf (GTXT ("%s: system name can not be determined"), type);
      break;
    case COL_ERROR_OVWOPEN:
      text = dbe_sprintf (GTXT ("%s: Can't open overview %s"), type, par);
      break;
    case COL_ERROR_OVWWRITE:
      text = dbe_sprintf (GTXT ("%s: Can't write overview %s"), type, par);
      break;
    case COL_ERROR_OVWREAD:
      text = dbe_sprintf (GTXT ("%s: Can't read overview data for %s"), type, par);
      break;
    case COL_ERROR_NOZMEM:
      text = dbe_sprintf (GTXT ("%s: Open of /dev/zero failed: %s"), type, par);
      break;
    case COL_ERROR_NOZMEMMAP:
      text = dbe_sprintf (GTXT ("%s: Mmap of /dev/zero failed: %s"), type, par);
      break;
    case COL_ERROR_NOHNDL:
      text = dbe_sprintf (GTXT ("%s: Out of data handles for %s"), type, par);
      break;
    case COL_ERROR_FILEOPN:
      text = dbe_sprintf (GTXT ("%s: Open failed %s"), type, par);
      break;
    case COL_ERROR_FILETRNC:
      text = dbe_sprintf (GTXT ("%s: Truncate failed for file %s"), type, par);
      break;
    case COL_ERROR_FILEMAP:
      text = dbe_sprintf (GTXT ("%s: Mmap failed %s"), type, par);
      break;
    case COL_ERROR_HEAPINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing heap tracing failed"), type);
      break;
    case COL_ERROR_DISPINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing SIGPROF dispatcher failed"), type);
      break;
    case COL_ERROR_ITMRINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing interval timer failed; %s"), type, par);
      break;
    case COL_ERROR_SMPLINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing periodic sampling failed"), type);
      break;
    case COL_ERROR_MPIINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing MPI tracing failed"), type);
      break;
    case COL_ERROR_JAVAINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing Java profiling failed"), type);
      break;
    case COL_ERROR_LINEINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing descendant process lineage failed"), type);
      break;
    case COL_ERROR_NOSPACE:
      text = dbe_sprintf (GTXT ("%s: Out of disk space writing `%s'"), type, par);
      break;
    case COL_ERROR_ITMRRST:
      text = dbe_sprintf (GTXT ("%s: Resetting interval timer failed: %s"), type, par);
      break;
    case COL_ERROR_MKDIR:
      text = dbe_sprintf (GTXT ("%s: Unable to create directory `%s'"), type, par);
      break;
    case COL_ERROR_JVM2NEW:
      text = dbe_sprintf (GTXT ("%s: JVM version with JVMTI requires more recent release of the performance tools; please upgrade"), type);
      break;
    case COL_ERROR_JVMNOTSUPP:
      text = dbe_sprintf (GTXT ("%s: JVM version does not support JVMTI; no java profiling is available"), type);
      break;
    case COL_ERROR_JVMNOJSTACK:
      text = dbe_sprintf (GTXT ("%s: JVM version does not support java callstacks; java mode data will not be recorded"), type);
      break;
    case COL_ERROR_DYNOPEN:
      text = dbe_sprintf (GTXT ("%s: Can't open dyntext file `%s'"), type, par);
      break;
    case COL_ERROR_DYNWRITE:
      text = dbe_sprintf (GTXT ("%s: Can't write dyntext file `%s'"), type, par);
      break;
    case COL_ERROR_MAPOPEN:
      text = dbe_sprintf (GTXT ("%s: Can't open map file `%s'"), type, par);
      break;
    case COL_ERROR_MAPREAD:
      text = dbe_sprintf (GTXT ("%s: Can't read map file `%s'"), type, par);
      break;
    case COL_ERROR_MAPWRITE:
      text = dbe_sprintf (GTXT ("%s: Can't write map file"), type);
      break;
    case COL_ERROR_RESOLVE:
      text = dbe_sprintf (GTXT ("%s: Can't resolve map file `%s'"), type, par);
      break;
    case COL_ERROR_OMPINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing OpenMP tracing failed"), type);
      break;
    case COL_ERROR_DURATION_INIT:
      text = dbe_sprintf (GTXT ("%s: Initializing experiment-duration setting to `%s' failed"), type, par);
      break;
    case COL_ERROR_RDTINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing RDT failed"), type);
      break;
    case COL_ERROR_GENERAL:
      if (strlen (par))
	text = dbe_sprintf (GTXT ("%s: %s"), type, par);
      else
	text = dbe_sprintf (GTXT ("%s: General error"), type);
      break;
    case COL_ERROR_EXEC_FAIL:
      text = dbe_sprintf (GTXT ("%s: Exec of process failed"), type);
      break;
    case COL_ERROR_THR_MAX:
      text = dbe_sprintf (GTXT ("%s: Thread count exceeds maximum (%s); set SP_COLLECTOR_NUMTHREADS for higher value"), type, par);
      break;
    case COL_ERROR_IOINIT:
      text = dbe_sprintf (GTXT ("%s: Initializing IO tracing failed"), type);
      break;
    case COL_ERROR_NODATA:
      text = dbe_sprintf (GTXT ("%s: No data was recorded in the experiment"), type);
      break;
    case COL_ERROR_DTRACE_FATAL:
      text = dbe_sprintf (GTXT ("%s: Fatal error reported from DTrace -- %s"), type, par);
      break;
    case COL_ERROR_MAPSEEK:
      text = dbe_sprintf (GTXT ("%s: Seek error on map file `%s'"), type, par);
      break;
    case COL_ERROR_UNEXP_FOUNDER:
      text = dbe_sprintf (GTXT ("%s: Unexpected value for founder `%s'"), type, par);
      break;
    case COL_ERROR_LOG_OPEN:
      text = dbe_sprintf (GTXT ("%s: Failure to open log file"), type);
      break;
    case COL_ERROR_TSD_INIT:
      text = dbe_sprintf (GTXT ("%s: TSD could not be initialized"), type);
      break;
    case COL_ERROR_UTIL_INIT:
      text = dbe_sprintf (GTXT ("%s: libcol_util.c initialization failed"), type);
      break;
    case COL_ERROR_MAPCACHE:
      text = dbe_sprintf (GTXT ("%s: Unable to cache mappings;  internal error (`%s')"), type, par);
      break;
    case COL_WARN_NONE:
      text = dbe_sprintf (GTXT ("%s: No warning"), type);
      break;
    case COL_WARN_FSTYPE:
      text = dbe_sprintf (GTXT ("%s: Experiment was written to a filesystem of type `%s'; data may be distorted"), type, par);
      break;
    case COL_WARN_PROFRND:
      text = dbe_sprintf (GTXT ("%s: Profiling interval was changed from requested %s (microsecs.) used"), type, par);
      break;
    case COL_WARN_SIZELIM:
      text = dbe_sprintf (GTXT ("%s: Experiment size limit exceeded"), type);
      break;
    case COL_WARN_SIGPROF:
      text = dbe_sprintf (GTXT ("%s: SIGPROF handler was changed (%s) during the run; profile data may be unreliable"), type, par);
      break;
    case COL_WARN_SMPLADJ:
      text = dbe_sprintf (GTXT ("%s: Periodic sampling rate adjusted %s microseconds"), type, par);
      break;
    case COL_WARN_ITMROVR:
      text = dbe_sprintf (GTXT ("%s: Application's attempt to set interval timer period to %s was ignored; its behavior may be changed"), type, par);
      break;
    case COL_WARN_ITMRREP:
      text = dbe_sprintf (GTXT ("%s: Collection interval timer period was changed (%s); profile data may be unreliable"), type, par);
      break;
    case COL_WARN_SIGEMT:
      text = dbe_sprintf (GTXT ("%s: SIGEMT handler was changed during the run; profile data may be unreliable"), type);
      break;
    case COL_WARN_CPCBLK:
      text = dbe_sprintf (GTXT ("%s: libcpc access blocked for hardware counter profiling"), type);
      break;
    case COL_WARN_VFORK:
      text = dbe_sprintf (GTXT ("%s: vfork(2) replaced by %s; execution may be affected"), type, par);
      break;
    case COL_WARN_EXECENV:
      text = dbe_sprintf (GTXT ("%s: exec environment augmented with %s missing collection variables"), type, par);
      break;
    case COL_WARN_SAMPSIGUSED:
      text = dbe_sprintf (GTXT ("%s: target installed handler for sample signal %s; samples may be lost"), type, par);
      break;
    case COL_WARN_PAUSESIGUSED:
      text = dbe_sprintf (GTXT ("%s: target installed handler for pause/resume signal %s; data may be lost or unexpected"),
			  type, par);
      break;
    case COL_WARN_CPCNOTRESERVED:
      text = dbe_sprintf (GTXT ("%s: unable to reserve HW counters; data may be distorted by other users of the counters"), type);
      break;
    case COL_WARN_LIBTHREAD_T1: /* par contains the aslwpid... do we want to report it? */
      text = dbe_sprintf (GTXT ("%s: application ran with a libthread version that may distort data; see collect(1) man page"), type);
      break;
    case COL_WARN_SIGMASK:
      text = dbe_sprintf (GTXT ("%s: Blocking %s ignored while in use for collection"), type, par);
      break;
    case COL_WARN_NOFOLLOW:
      text = dbe_sprintf (GTXT ("%s: Following disabled for uncollectable target (%s)"), type, par);
      break;
    case COL_WARN_RISKYFOLLOW:
      text = dbe_sprintf (GTXT ("%s: Following unqualified target may be unreliable (%s)"), type, par);
      break;
    case COL_WARN_IDCHNG:
      text = dbe_sprintf (GTXT ("%s: Imminent process ID change (%s) may result in an inconsistent experiment"), type, par);
      break;
    case COL_WARN_OLDJAVA:
      text = dbe_sprintf (GTXT ("%s: Java profiling requires JVM version 1.4.2_02 or later"), type);
      break;
    case COL_WARN_ITMRPOVR:
      text = dbe_sprintf (GTXT ("%s: Collector reset application's profile timer %s; application behavior may be changed"), type, par);
      break;
    case COL_WARN_NO_JAVA_HEAP:
      text = dbe_sprintf (GTXT ("%s: Java heap profiling is not supported by JVMTI; disabled "), type);
      break;
    case COL_WARN_RDT_PAUSE_NOMEM:
      text = dbe_sprintf (GTXT ("%s: Data race detection paused at %s because of running out of internal memory"), type, par);
      break;
    case COL_WARN_RDT_RESUME:
      text = dbe_sprintf (GTXT ("%s: Data race detection resumed"), type);
      break;
    case COL_WARN_RDT_THROVER:
      text = dbe_sprintf (GTXT ("%s: Too many concurrent/created threads;  accesses with thread IDs above limit are not checked"), type);
      break;
    case COL_WARN_THR_PAUSE_RESUME:
      text = dbe_sprintf (GTXT ("%s: The collector_thread_pause/collector_thread_resume APIs are deprecated, and will be removed in a future release"), type);
      break;
    case COL_WARN_NOPROF_DATA:
      text = dbe_sprintf (GTXT ("%s: No profile data recorded in experiment"), type);
      break;
    case COL_WARN_LONG_FSTAT:
      text = dbe_sprintf (GTXT ("%s: Long fstat call -- %s"), type, par);
      break;
    case COL_WARN_LONG_READ:
      text = dbe_sprintf (GTXT ("%s: Long read call -- %s"), type, par);
      break;
    case COL_WARN_LINUX_X86_APICID:
      text = dbe_sprintf (GTXT ("%s: Linux libc sched_getcpu() not found; using x86 %s IDs rather than CPU IDs"), type, par);
      break;

    case COL_COMMENT_NONE:
      text = dbe_sprintf (GTXT ("%s"), par);
      break;
    case COL_COMMENT_CWD:
      text = dbe_sprintf (GTXT ("Initial execution directory `%s'"), par);
      break;
    case COL_COMMENT_ARGV:
      text = dbe_sprintf (GTXT ("Argument list `%s'"), par);
      break;
    case COL_COMMENT_MAYASSNAP:
      text = dbe_sprintf (GTXT ("Mayas snap file `%s'"), par);
      break;

    case COL_COMMENT_LINEFORK:
      text = dbe_sprintf (GTXT ("Target fork: %s"), par);
      break;
    case COL_COMMENT_LINEEXEC:
      text = dbe_sprintf (GTXT ("Target exec: %s"), par);
      break;
    case COL_COMMENT_LINECOMBO:
      text = dbe_sprintf (GTXT ("Target fork/exec: %s"), par);
      break;
    case COL_COMMENT_FOXSNAP:
      text = dbe_sprintf (GTXT ("Fox snap file `%s'"), par);
      break;
    case COL_COMMENT_ROCKSNAP:
      text = dbe_sprintf (GTXT ("Rock simulator snap file `%s'"), par);
      break;
    case COL_COMMENT_BITINSTRDATA:
      text = dbe_sprintf (GTXT ("Bit instrument data file `%s'"), par);
      break;
    case COL_COMMENT_BITSNAP:
      text = dbe_sprintf (GTXT ("Bit snap file `%s'"), par);
      break;
    case COL_COMMENT_SIMDSPSNAP:
      text = dbe_sprintf (GTXT ("Simulator dataspace profiling snap file `%s'"), par);
      break;
    case COL_COMMENT_HWCADJ:
      text = dbe_sprintf (GTXT ("%s: HWC overflow interval adjusted: %s"), type, par);
      break;
    case COL_WARN_APP_NOT_READY:
      text = dbe_sprintf (GTXT ("*** Collect: %s"), par);
      break;
    case COL_WARN_RDT_DL_TERMINATE:
      text = dbe_sprintf (GTXT ("%s: Actual deadlock detected; process terminated"), type);
      break;
    case COL_WARN_RDT_DL_TERMINATE_CORE:
      text = dbe_sprintf (GTXT ("%s: Actual deadlock detected; process terminated and core dumped"), type);
      break;
    case COL_WARN_RDT_DL_CONTINUE:
      text = dbe_sprintf (GTXT ("%s: Actual deadlock detected; process allowed to continue"), type);
      break;
    default:
      text = dbe_sprintf (GTXT ("%s: Number %d (\"%s\")"), type, flavor, par);
      break;
    };
}

Emsg::~Emsg ()
{
  free (par);
  free (text);
}

// ----------------------- Message Queue --------------------
Emsgqueue::Emsgqueue (char *_qname)
{
  first = NULL;
  last = NULL;
  qname = strdup (_qname);
}

Emsgqueue::~Emsgqueue ()
{
  free (qname);
  clear ();
}

Emsg *
Emsgqueue::find_msg (Cmsg_warn w, char *msg)
{
  for (Emsg *m = first; m; m = m->next)
    if (m->get_warn () == w && strcmp (m->get_msg (), msg) == 0)
      return m;
  return NULL;
}

Emsg *
Emsgqueue::append (Cmsg_warn w, char *msg)
{
  Emsg *m = find_msg (w, msg);
  if (m)
    return m;
  m = new Emsg (w, msg);
  append (m);
  return m;
}

// Append a single message to a queue
void
Emsgqueue::append (Emsg* m)
{
  m->next = NULL;
  if (last == NULL)
    {
      first = m;
      last = m;
    }
  else
    {
      last->next = m;
      last = m;
    }
}

// Append a queue of messages to a queue
void
Emsgqueue::appendqueue (Emsgqueue* mq)
{
  Emsg *m = mq->first;
  if (m == NULL)
    return;
  if (last == NULL)
    first = m;
  else
    last->next = m;
  // now find the new last
  while (m->next != NULL)
    m = m->next;
  last = m;
}

Emsg *
Emsgqueue::fetch (void)
{
  return first;
}

// Empty the queue, deleting all messages
void
Emsgqueue::clear (void)
{
  Emsg *pp;
  Emsg *p = first;
  while (p != NULL)
    {
      pp = p;
      p = p->next;
      delete pp;
    }
  first = NULL;
  last = NULL;
}

// Mark the queue empty, without deleting the messages --
//	used when the messages have been requeued somewhere else
void
Emsgqueue::mark_clear (void)
{
  first = NULL;
  last = NULL;
}

DbeMessages::DbeMessages ()
{
  msgs = NULL;
}

DbeMessages::~DbeMessages ()
{
  if (msgs)
    {
      msgs->destroy ();
      delete msgs;
    }
}

Emsg *
DbeMessages::get_error ()
{
  for (int i = msgs ? msgs->size () - 1 : -1; i >= 0; i--)
    {
      Emsg *msg = msgs->get (i);
      if (msg->get_warn () == CMSG_ERROR)
	return msg;
    }
  return NULL;
}

void
DbeMessages::remove_msg (Emsg *msg)
{
  for (int i = 0, sz = msgs ? msgs->size () : 0; i < sz; i++)
    if (msg == msgs->get (i))
      {
	msgs->remove (i);
	delete msg;
	return;
      }
}

Emsg *
DbeMessages::append_msg (Cmsg_warn w, const char *fmt, ...)
{
  char buffer[256];
  size_t buf_size;
  Emsg *msg;
  va_list vp;

  va_start (vp, fmt);
  buf_size = vsnprintf (buffer, sizeof (buffer), fmt, vp) + 1;
  va_end (vp);
  if (buf_size < sizeof (buffer))
    msg = new Emsg (w, buffer);
  else
    {
      va_start (vp, fmt);
      char *buf = (char *) malloc (buf_size);
      vsnprintf (buf, buf_size, fmt, vp);
      va_end (vp);
      msg = new Emsg (w, buf);
      free (buf);
    }

  if (msgs == NULL)
    msgs = new Vector<Emsg*>();
  msgs->append (msg);
  Dprintf (DEBUG_ERR_MSG, NTXT ("Warning: %s\n"), msg->get_msg ());
  return msg;
}

void
DbeMessages::append_msgs (Vector<Emsg*> *lst)
{
  if (lst && (lst->size () != 0))
    {
      if (msgs == NULL)
	msgs = new Vector<Emsg*>();
      for (int i = 0, sz = lst->size (); i < sz; i++)
	{
	  Emsg *m = lst->fetch (i);
	  msgs->append (new Emsg (m->get_warn (), m->get_msg ()));
	}
    }
}
