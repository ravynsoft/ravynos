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
#include <utime.h>
#include <alloca.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <set>

#include "util.h"
#include "CacheMap.h"
#include "DbeFile.h"
#include "DbeCacheMap.h"
#include "DefaultHandler.h"
#include "DefaultMap2D.h"
#include "Emsg.h"
#include "Elf.h"
#include "SAXParser.h"
#include "SAXParserFactory.h"
#include "StringBuilder.h"
#include "DbeSession.h"
#include "DbeThread.h"
#include "Application.h"
#include "CallStack.h"
#include "Experiment.h"
#include "Exp_Layout.h"
#include "DataStream.h"
#include "Expression.h"
#include "Function.h"
#include "HeapMap.h"
#include "LoadObject.h"
#include "Module.h"
#include "Ovw_data.h"
#include "PRBTree.h"
#include "Sample.h"
#include "SegMem.h"
#include "StringMap.h"
#include "UserLabel.h"
#include "Table.h"
#include "dbe_types.h"
#include "FileData.h"
#include "cc_libcollector.h"
#include "ExpGroup.h"

int nPush;
int nPop;
int pushCnt;
int popCnt;
int pushCnt3;
int popCnt3;

struct Experiment::UIDnode
{
  uint64_t uid;
  uint64_t val;
  UIDnode *next;
};

struct Experiment::RawFramePacket
{
  uint64_t uid;
  UIDnode *uidn;
  UIDnode *uidj;
  UIDnode *omp_uid;
  uint32_t omp_state;
};

static hrtime_t
parseTStamp (const char *s)
{
  hrtime_t ts = (hrtime_t) 0;
  ts = (hrtime_t) atoi (s) * NANOSEC;
  s = strchr (s, '.');
  if (s != NULL)
    ts += (hrtime_t) atoi (s + 1);
  return ts;
}

class Experiment::ExperimentFile
{
public:

  enum
  {
    EF_NOT_OPENED,
    EF_OPENED,
    EF_CLOSED,
    EF_FAILURE
  };

  ExperimentFile (Experiment *_exp, const char *_fname);
  ~ExperimentFile ();

  bool open (bool new_open = false);

  char *
  get_name ()
  {
    return fname;
  }

  inline int
  get_status ()
  {
    return ef_status;
  }

  char *fgets ();
  void close ();

  FILE *fh;

private:
  Experiment *exp;
  char *fname;
  off64_t offset;
  int bufsz, ef_status;
  char *buffer;
};

class Experiment::ExperimentHandler : public DefaultHandler
{
public:

  ExperimentHandler (Experiment *_exp);
  ~ExperimentHandler ();

  void
  startDocument () { }
  void endDocument ();
  void startElement (char *uri, char *localName, char *qName, Attributes *attrs);
  void endElement (char *uri, char *localName, char *qName);
  void characters (char *ch, int start, int length);

  void
  ignorableWhitespace (char*, int, int) { }
  void
  error (SAXParseException *e);

private:

  enum Element
  {
    EL_NONE,
    EL_EXPERIMENT,
    EL_COLLECTOR,
    EL_SETTING,
    EL_PROCESS,
    EL_SYSTEM,
    EL_EVENT,
    EL_PROFILE,
    EL_DATAPTR,
    EL_PROFDATA,
    EL_PROFPCKT,
    EL_FIELD,
    EL_CPU,
    EL_STATE,
    EL_FREQUENCY,
    EL_POWERM,
    EL_DTRACEFATAL
  };

  static int toInt (Attributes *attrs, const char *atr);
  static char*toStr (Attributes *attrs, const char *atr);
  void pushElem (Element);
  void popElem ();

  Experiment *exp;
  Element curElem;
  Vector<Element> *stack;
  Module *dynfuncModule;
  DataDescriptor *dDscr;
  PacketDescriptor *pDscr;
  PropDescr *propDscr;
  char *text;
  Cmsg_warn mkind;
  int mnum;
  int mec;
};


// HTableSize is the size of smemHTable and instHTable
// omazur: both HTableSize and the hash function haven't been tuned;
static const int HTableSize = 8192;

//-------------------------------------------------- Experiment file handler

Experiment::ExperimentFile::ExperimentFile (Experiment *_exp, const char *_fname)
{
  exp = _exp;
  fh = NULL;
  bufsz = 0;
  buffer = NULL;
  ef_status = EF_NOT_OPENED;
  offset = 0;
  fname = dbe_sprintf (NTXT ("%s/%s"), exp->expt_name, _fname);
}

Experiment::ExperimentFile::~ExperimentFile ()
{
  close ();
  free (buffer);
  free (fname);
}

bool
Experiment::ExperimentFile::open (bool new_open)
{
  if (fh == NULL)
    {
      fh = fopen64 (fname, NTXT ("r"));
      if (fh == NULL)
	{
	  ef_status = EF_FAILURE;
	  return false;
	}
      ef_status = EF_OPENED;
      if (new_open)
	offset = 0;
      if (offset != 0)
	fseeko64 (fh, offset, SEEK_SET);
    }
  return true;
}

char *
Experiment::ExperimentFile::fgets ()
{
  if (bufsz == 0)
    {
      bufsz = 1024;
      buffer = (char *) malloc (bufsz);
      if (buffer == NULL)
	return NULL;
      buffer[bufsz - 1] = (char) 1; // sentinel
    }
  char *res = ::fgets (buffer, bufsz, fh);
  if (res == NULL)
    return NULL;
  while (buffer[bufsz - 1] == (char) 0)
    {
      int newsz = bufsz + 1024;
      char *newbuf = (char *) malloc (newsz);
      if (newbuf == NULL)
	return NULL;
      memcpy (newbuf, buffer, bufsz);
      free (buffer);
      buffer = newbuf;
      buffer[newsz - 1] = (char) 1; // sentinel
      // we don't care about fgets result here
      ::fgets (buffer + bufsz - 1, newsz - bufsz + 1, fh);
      bufsz = newsz;
    }
  return buffer;
}

void
Experiment::ExperimentFile::close ()
{
  if (fh)
    {
      offset = ftello64 (fh);
      fclose (fh);
      ef_status = EF_CLOSED;
      fh = NULL;
    }
}


//-------------------------------------------------- Experiment XML parser
int
Experiment::ExperimentHandler::toInt (Attributes *attrs, const char *atr)
{
  const char *str = attrs->getValue (atr);
  return str ? atoi (str) : 0;
}

char *
Experiment::ExperimentHandler::toStr (Attributes *attrs, const char *atr)
{
  const char *str = attrs->getValue (atr);
  return dbe_strdup (str ? str : NTXT (""));
}

Experiment::ExperimentHandler::ExperimentHandler (Experiment *_exp)
{
  exp = _exp;
  stack = new Vector<Element>;
  pushElem (EL_NONE);
  dynfuncModule = NULL;
  dDscr = NULL;
  pDscr = NULL;
  propDscr = NULL;
  text = NULL;
  mkind = (Cmsg_warn) - 1; // CMSG_NONE
  mnum = -1;
  mec = -1;
}

Experiment::ExperimentHandler::~ExperimentHandler ()
{
  delete stack;
  free (text);
}

void
Experiment::ExperimentHandler::endDocument ()
{
  { // SP_TAG_STATE should be used to describe states, but it isn't
    // let's do it here:
    DataDescriptor *dd = exp->getDataDescriptor (DATA_HEAP);
    if (dd != NULL)
      {
	PropDescr *prop = dd->getProp (PROP_HTYPE);
	if (prop != NULL)
	  {
	    char * stateNames [HEAPTYPE_LAST] = HEAPTYPE_STATE_STRINGS;
	    char * stateUNames[HEAPTYPE_LAST] = HEAPTYPE_STATE_USTRINGS;
	    for (int ii = 0; ii < HEAPTYPE_LAST; ii++)
	      prop->addState (ii, stateNames[ii], stateUNames[ii]);
	  }
      }
    dd = exp->getDataDescriptor (DATA_IOTRACE);
    if (dd != NULL)
      {
	PropDescr *prop = dd->getProp (PROP_IOTYPE);
	if (prop != NULL)
	  {
	    char * stateNames [IOTRACETYPE_LAST] = IOTRACETYPE_STATE_STRINGS;
	    char * stateUNames[IOTRACETYPE_LAST] = IOTRACETYPE_STATE_USTRINGS;
	    for (int ii = 0; ii < IOTRACETYPE_LAST; ii++)
	      prop->addState (ii, stateNames[ii], stateUNames[ii]);
	  }
      }
  }
}

void
Experiment::ExperimentHandler::pushElem (Element elem)
{
  curElem = elem;
  stack->append (curElem);
}

void
Experiment::ExperimentHandler::popElem ()
{
  stack->remove (stack->size () - 1);
  curElem = stack->fetch (stack->size () - 1);
}

void
Experiment::ExperimentHandler::startElement (char*, char*, char *qName, Attributes *attrs)
{
  DEBUG_CODE if (DEBUG_SAXPARSER) dump_startElement (qName, attrs);
  if (strcmp (qName, SP_TAG_EXPERIMENT) == 0)
    {
      pushElem (EL_EXPERIMENT);
      const char *str = attrs->getValue (NTXT ("version"));
      if (str != NULL)
	{
	  int major = atoi (str);
	  str = strchr (str, '.');
	  int minor = str ? atoi (str + 1) : 0;
	  exp->exp_maj_version = major;
	  exp->exp_min_version = minor;
	  if (major != SUNPERF_VERNUM || minor != SUNPERF_VERNUM_MINOR)
	    {
	      // not the current version, see if we support some earlier versions
	      if (major < 12)
		{
		  StringBuilder sb;
		  sb.sprintf (GTXT ("*** Error: experiment %s version %d.%d is not supported;\nuse the version of the tools that recorded the experiment to read it"),
			      exp->get_expt_name (), major, minor);
		  // exp->errorq->append( new Emsg(CMSG_FATAL, sb) );
		  exp->status = FAILURE;
		  exp->obsolete = 1;
		  throw new SAXException (sb.toString ());
		}
	    }
	}
    }
  else if (strcmp (qName, SP_TAG_COLLECTOR) == 0)
    pushElem (EL_COLLECTOR);
  else if (strcmp (qName, SP_TAG_SETTING) == 0)
    {
      int found = 0;
      pushElem (EL_SETTING);
      const char *str = attrs->getValue (SP_JCMD_LIMIT);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.limit = atoi (str);
	}
      str = attrs->getValue (SP_JCMD_BLKSZ);
      if (str != NULL)
	{
	  found = 1;
	  exp->blksz = strtol (str, NULL, 0);
	}
      str = attrs->getValue (SP_JCMD_STACKBASE);
      if (str)
	{
	  found = 1;
	  exp->stack_base = strtoull (str, NULL, 0);
	}
      str = attrs->getValue (SP_JCMD_HWC_DEFAULT);
      if (str != NULL)
	{
	  found = 1;
	  exp->hwc_default = true;
	}
      str = attrs->getValue (SP_JCMD_NOIDLE);
      if (str != NULL)
	{
	  found = 1;
	  exp->commentq->append (new Emsg (CMSG_COMMENT,
					   GTXT ("*** Note: experiment does not have events from idle CPUs")));
	}
      str = attrs->getValue (SP_JCMD_FAKETIME);
      if (str != NULL)
	{
	  found = 1;
	  exp->timelineavail = false;
	  exp->commentq->append (new Emsg (CMSG_COMMENT,
					   GTXT ("*** Note: experiment does not have timestamps; timeline unavailable")));
	}
      str = attrs->getValue (SP_JCMD_DELAYSTART);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.start_delay = strdup (str);
	}
      str = attrs->getValue (SP_JCMD_TERMINATE);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.terminate = strdup (str);
	}
      str = attrs->getValue (SP_JCMD_PAUSE_SIG);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.pause_sig = strdup (str);
	}
      str = attrs->getValue (SP_JCMD_SAMPLE_PERIOD);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.sample_periodic = 1;
	  exp->coll_params.sample_timer = atoi (str);
	}
      str = attrs->getValue (SP_JCMD_SAMPLE_SIG);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.sample_sig = str;
	}
      str = attrs->getValue (SP_JCMD_SRCHPATH);
      if (str != NULL)
	{
	  found = 1;
	  StringBuilder sb;
	  sb.sprintf (GTXT ("Search path: %s"), str);
	  exp->runlogq->append (new Emsg (CMSG_COMMENT, sb));
	  dbeSession->add_classpath ((char*) str);
	}
      str = attrs->getValue (SP_JCMD_LINETRACE);
      if (str != NULL)
	{
	  found = 1;
	  exp->coll_params.linetrace = strdup (str);
	}

      str = attrs->getValue (SP_JCMD_COLLENV);
      if (str != NULL)
	{
	  found = 1;
	  StringBuilder sb;
	  sb.sprintf (GTXT ("  Data collection environment variable: %s"), str);
	  exp->runlogq->append (new Emsg (CMSG_COMMENT, sb));
	}
      if (found == 0)
	{
	  int nattr = attrs->getLength ();
	  if (nattr != 0)
	    {
	      fprintf (stderr, "XXX Unexpected setting found; %d attributes:\n",
		       nattr);
	      for (int k = 0; k < nattr; k++)
		{
		  const char *qn = attrs->getQName (k);
		  const char *vl = attrs->getValue (k);
		  fprintf (stderr, "XXX      %s = %s\n", qn, vl);
		}
	    }
	}
      // END OF CODE FOR "setting"
    }
  else if (strcmp (qName, SP_TAG_SYSTEM) == 0)
    {
      pushElem (EL_SYSTEM);
      const char *str = attrs->getValue (NTXT ("hostname"));
      if (str != NULL)
	exp->hostname = strdup (str);
      str = attrs->getValue (NTXT ("os"));
      if (str != NULL)
	{
	  exp->os_version = strdup (str);
	  /* For Linux experiments expect sparse thread ID's */
	  if (strncmp (str, NTXT ("SunOS"), 5) != 0)
	    exp->sparse_threads = true;
	}
      str = attrs->getValue (NTXT ("arch"));
      if (str != NULL)
	{
	  if (strcmp (str, "i86pc") == 0 || strcmp (str, "i686") == 0
	      || strcmp (str, "x86_64") == 0)
	    exp->platform = Intel;
	  else if (strcmp (str, "aarch64") == 0)
	    exp->platform = Aarch64;
	  else
	    exp->platform = Sparc;
	  exp->need_swap_endian = (DbeSession::platform == Sparc) ?
		  (exp->platform != Sparc) : (exp->platform == Sparc);
	  exp->architecture = strdup (str);
	}
      str = attrs->getValue (NTXT ("pagesz"));
      if (str != NULL)
	exp->page_size = atoi (str);
      str = attrs->getValue (NTXT ("npages"));
      if (str != NULL)
	exp->npages = atoi (str);
    }
  else if (strcmp (qName, SP_TAG_POWERM) == 0)
    pushElem (EL_POWERM);
  else if (strcmp (qName, SP_TAG_FREQUENCY) == 0)
    {
      pushElem (EL_FREQUENCY);
      const char *str = attrs->getValue (NTXT ("clk"));
      if (str != NULL)
	exp->set_clock (atoi (str));
      // check for frequency_scaling or turbo_mode recorded from libcollector under dbx
      str = attrs->getValue (NTXT ("frequency_scaling"));
      const char *str2 = attrs->getValue (NTXT ("turbo_mode"));
      if (str != NULL || str2 != NULL)
	exp->varclock = 1;
    }
  else if (strcmp (qName, SP_TAG_CPU) == 0)
    {
      pushElem (EL_CPU);
      exp->ncpus++;
      const char *str = attrs->getValue (NTXT ("clk"));
      if (str != NULL)
	{
	  int clk = atoi (str);
	  if (exp->maxclock == 0)
	    {
	      exp->minclock = clk;
	      exp->maxclock = clk;
	    }
	  else
	    {
	      if (clk < exp->minclock)
		exp->minclock = clk;
	      if (clk > exp->maxclock)
		exp->maxclock = clk;
	    }
	  exp->clock = clk;
	}
      // check for frequency_scaling or turbo_mode
      str = attrs->getValue (NTXT ("frequency_scaling"));
      const char *str2 = attrs->getValue (NTXT ("turbo_mode"));
      if (str != NULL || str2 != NULL)
	exp->varclock = 1;
    }
  else if (strcmp (qName, SP_TAG_PROCESS) == 0)
    {
      pushElem (EL_PROCESS);
      const char *str = attrs->getValue (NTXT ("wsize"));
      if (str != NULL)
	{
	  int wsz = atoi (str);
	  if (wsz == 32)
	    exp->wsize = W32;
	  else if (wsz == 64)
	    exp->wsize = W64;
	}
      str = attrs->getValue (NTXT ("pid"));
      if (str != NULL)
	exp->pid = atoi (str);
      str = attrs->getValue (NTXT ("ppid"));
      if (str != NULL)
	exp->ppid = atoi (str);
      str = attrs->getValue (NTXT ("pgrp"));
      if (str != NULL)
	exp->pgrp = atoi (str);
      str = attrs->getValue (NTXT ("sid"));
      if (str != NULL)
	exp->sid = atoi (str);
      str = attrs->getValue (NTXT ("cwd"));
      if (str != NULL)
	exp->ucwd = strdup (str);
      str = attrs->getValue (NTXT ("pagesz"));
      if (str != NULL)
	exp->page_size = atoi (str);
    }
  else if (strcmp (qName, SP_TAG_EVENT) == 0)
    { // Start code for event
      pushElem (EL_EVENT);
      hrtime_t ts = (hrtime_t) 0;
      const char *str = attrs->getValue (NTXT ("tstamp"));
      if (str != NULL)
	ts = parseTStamp (str);
      str = attrs->getValue (NTXT ("kind"));
      if (str != NULL)
	{
	  if (strcmp (str, SP_JCMD_RUN) == 0)
	    {
	      exp->broken = 0;
	      exp->exp_start_time = ts;
	      str = attrs->getValue (NTXT ("time"));
	      if (str != NULL)
		exp->start_sec = atoll (str);
	      str = attrs->getValue (NTXT ("pid"));
	      if (str != NULL)
		exp->pid = atoi (str);
	      str = attrs->getValue (NTXT ("ppid"));
	      if (str != NULL)
		exp->ppid = atoi (str);
	      str = attrs->getValue (NTXT ("pgrp"));
	      if (str != NULL)
		exp->pgrp = atoi (str);
	      str = attrs->getValue (NTXT ("sid"));
	      if (str != NULL)
		exp->sid = atoi (str);
	      exp->status = Experiment::INCOMPLETE;
	    }
	  else if (strcmp (str, SP_JCMD_ARCHIVE) == 0)
	    {
	      StringBuilder sb;
	      sb.sprintf (GTXT ("er_archive run: XXXXXXX"));
	      exp->pprocq->append (new Emsg (CMSG_WARN, sb));
	    }
	  else if (strcmp (str, SP_JCMD_SAMPLE) == 0)
	    {
	      exp->update_last_event (exp->exp_start_time + ts); // ts is 0-based
	      str = attrs->getValue (NTXT ("id"));
	      int id = str ? atoi (str) : -1;
	      char *label = dbe_strdup (attrs->getValue (NTXT ("label")));
	      exp->process_sample_cmd (NULL, ts, id, label);
	    }
	  else if (strcmp (str, SP_JCMD_EXIT) == 0)
	    {
	      // don't treat EXIT as an event w.r.t. last_event and non_paused_time
	      exp->status = Experiment::SUCCESS;
	    }
	  else if (strcmp (str, SP_JCMD_CERROR) == 0)
	    {
	      mkind = CMSG_ERROR;
	      str = attrs->getValue (NTXT ("id"));
	      if (str != NULL)
		{
		  mnum = atoi (str);
		}
	      str = attrs->getValue (NTXT ("ec"));
	      if (str != NULL)
		{
		  mec = atoi (str);
		}
	    }
	  else if (strcmp (str, SP_JCMD_CWARN) == 0)
	    {
	      mkind = CMSG_WARN;
	      str = attrs->getValue (NTXT ("id"));
	      if (str != NULL)
		mnum = atoi (str);
	    }
	  else if (strcmp (str, SP_JCMD_COMMENT) == 0)
	    {
	      mkind = CMSG_COMMENT;
	      str = attrs->getValue (NTXT ("id"));
	      if (str != NULL)
		mnum = atoi (str);
	      str = attrs->getValue (NTXT ("text"));
	      if (str != NULL)
		{
		  StringBuilder sb;
		  sb.sprintf (GTXT ("*** Note: %s"), str);
		  exp->commentq->append (new Emsg (CMSG_COMMENT, sb));
		}
	    }
	  else if (strcmp (str, SP_JCMD_DESC_START) == 0)
	    {
	      char *variant = toStr (attrs, NTXT ("variant"));
	      char *lineage = toStr (attrs, NTXT ("lineage"));
	      int follow = toInt (attrs, NTXT ("follow"));
	      char *msg = toStr (attrs, NTXT ("msg"));
	      exp->process_desc_start_cmd (NULL, ts, variant, lineage, follow, msg);
	    }
	  else if (strcmp (str, SP_JCMD_DESC_STARTED) == 0)
	    {
	      char *variant = toStr (attrs, NTXT ("variant"));
	      char *lineage = toStr (attrs, NTXT ("lineage"));
	      int follow = toInt (attrs, NTXT ("follow"));
	      char *msg = toStr (attrs, NTXT ("msg"));
	      exp->process_desc_started_cmd (NULL, ts, variant, lineage, follow, msg);
	    }
	  else if (strcmp (str, SP_JCMD_EXEC_START) == 0)
	    {
	      // if successful, acts like experiment termination - no "exit" entry will follow
	      exp->update_last_event (exp->exp_start_time + ts); // ts is 0-based
	      char *variant = toStr (attrs, NTXT ("variant"));
	      char *lineage = toStr (attrs, NTXT ("lineage"));
	      int follow = toInt (attrs, NTXT ("follow"));
	      char *msg = toStr (attrs, NTXT ("msg"));
	      exp->process_desc_start_cmd (NULL, ts, variant, lineage, follow, msg);
	      exp->exec_started = true;
	    }
	  else if (strcmp (str, SP_JCMD_EXEC_ERROR) == 0)
	    {
	      exp->update_last_event (exp->exp_start_time + ts); // ts is 0-based
	      char *variant = toStr (attrs, NTXT ("variant"));
	      char *lineage = toStr (attrs, NTXT ("lineage"));
	      int follow = toInt (attrs, NTXT ("follow"));
	      char *msg = toStr (attrs, NTXT ("msg"));
	      exp->process_desc_started_cmd (NULL, ts, variant, lineage, follow, msg);
	      exp->exec_started = false;
	    }
	  else if (strcmp (str, SP_JCMD_JTHRSTART) == 0)
	    {
	      char *name = dbe_strdup (attrs->getValue (NTXT ("name")));
	      char *grpname = dbe_strdup (attrs->getValue (NTXT ("grpname")));
	      char *prntname = dbe_strdup (attrs->getValue (NTXT ("prntname")));
	      str = attrs->getValue (NTXT ("tid"));
	      uint64_t tid = str ? strtoull (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("jthr"));
	      Vaddr jthr = str ? strtoull (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("jenv"));
	      Vaddr jenv = str ? strtoull (str, NULL, 0) : 0;
	      exp->process_jthr_start_cmd (NULL, name, grpname, prntname, tid, jthr, jenv, ts);
	    }
	  else if (strcmp (str, SP_JCMD_JTHREND) == 0)
	    {
	      str = attrs->getValue (NTXT ("tid"));
	      uint64_t tid = str ? strtoull (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("jthr"));
	      Vaddr jthr = str ? strtoull (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("jenv"));
	      Vaddr jenv = str ? strtoull (str, NULL, 0) : 0;
	      exp->process_jthr_end_cmd (NULL, tid, jthr, jenv, ts);
	    }
	  else if (strcmp (str, SP_JCMD_GCEND) == 0)
	    {
	      if (exp->getDataDescriptor (DATA_GCEVENT) == NULL)
		exp->newDataDescriptor (DATA_GCEVENT);
	      exp->process_gc_end_cmd (ts);
	    }
	  else if (strcmp (str, SP_JCMD_GCSTART) == 0)
	    {
	      if (exp->getDataDescriptor (DATA_GCEVENT) == NULL)
		exp->newDataDescriptor (DATA_GCEVENT);
	      exp->process_gc_start_cmd (ts);
	    }
	  else if (strcmp (str, SP_JCMD_PAUSE) == 0)
	    {
	      if (exp->resume_ts != MAX_TIME)
		{
		  // data collection was active
		  hrtime_t delta = ts - exp->resume_ts;
		  exp->non_paused_time += delta;
		  exp->resume_ts = MAX_TIME; // collection is paused
		}
	      StringBuilder sb;
	      str = attrs->getValue (NTXT ("name"));
	      if (str == NULL)
		sb.sprintf (GTXT ("Pause: %ld.%09ld"), (long) (ts / NANOSEC),
			    (long) (ts % NANOSEC));
	      else
		sb.sprintf (GTXT ("Pause (%s): %ld.%09ld"), str,
			    (long) (ts / NANOSEC), (long) (ts % NANOSEC));
	      exp->runlogq->append (new Emsg (CMSG_COMMENT, sb));
	    }
	  else if (strcmp (str, SP_JCMD_RESUME) == 0)
	    {
	      if (exp->resume_ts == MAX_TIME)
		// data collection was paused
		exp->resume_ts = ts; // remember start time
	      StringBuilder sb;
	      sb.sprintf (GTXT ("Resume: %ld.%09ld"), (long) (ts / NANOSEC), (long) (ts % NANOSEC));
	      exp->runlogq->append (new Emsg (CMSG_COMMENT, sb));
	      if (exp->exp_start_time == ZERO_TIME)
		exp->exp_start_time = ts;
	    }
	  else if (strcmp (str, SP_JCMD_THREAD_PAUSE) == 0)
	    {
	      str = attrs->getValue (NTXT ("tid"));
	      uint64_t tid = str ? strtoull (str, NULL, 0) : 0;
	      StringBuilder sb;
	      sb.sprintf (GTXT ("Thread %llu pause: %ld.%09ld"), (unsigned long long) tid,
			  (long) (ts / NANOSEC), (long) (ts % NANOSEC));
	      exp->runlogq->append (new Emsg (CMSG_COMMENT, sb));
	    }
	  else if (strcmp (str, SP_JCMD_THREAD_RESUME) == 0)
	    {
	      str = attrs->getValue (NTXT ("tid"));
	      uint64_t tid = str ? strtoull (str, NULL, 0) : 0;
	      StringBuilder sb;
	      sb.sprintf (GTXT ("Thread %llu resume: %ld.%09ld"), (unsigned long long) tid,
			  (long) (ts / NANOSEC), (long) (ts % NANOSEC));
	      exp->runlogq->append (new Emsg (CMSG_COMMENT, sb));
	    }
	  else if (strcmp (str, NTXT ("map")) == 0)
	    {
	      ts += exp->exp_start_time;
	      str = attrs->getValue (NTXT ("vaddr"));
	      Vaddr vaddr = str ? strtoull (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("size"));
	      int msize = str ? atoi (str) : 0;
	      str = attrs->getValue (NTXT ("foffset"));
	      int64_t offset = str ? strtoll (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("modes"));
	      int64_t modes = str ? strtoll (str, NULL, 0) : 0;
	      str = attrs->getValue (NTXT ("chksum"));
	      int64_t chksum = 0;
	      if (str)
		chksum = Elf::normalize_checksum (strtoll (str, NULL, 0));
	      char *name = (char *) attrs->getValue (NTXT ("name"));
	      str = attrs->getValue (NTXT ("object"));
	      if (strcmp (str, NTXT ("segment")) == 0)
		{
		  if (strcmp (name, NTXT ("LinuxKernel")) == 0)
		    exp->process_Linux_kernel_cmd (ts);
		  else
		    exp->process_seg_map_cmd (NULL, ts, vaddr, msize, 0,
					      offset, modes, chksum, name);
		}
	      else if (strcmp (str, NTXT ("function")) == 0)
		{
		  exp->process_fn_load_cmd (dynfuncModule, name, vaddr, msize, ts);
		  dynfuncModule = NULL;
		}
	      else if (strcmp (str, NTXT ("dynfunc")) == 0)
		{
		  if (dynfuncModule == NULL)
		    {
		      dynfuncModule = dbeSession->createModule (exp->get_dynfunc_lo (DYNFUNC_SEGMENT), name);
		      dynfuncModule->flags |= MOD_FLAG_UNKNOWN;
		      dynfuncModule->set_file_name (dbe_strdup (dynfuncModule->getMainSrc ()->get_name ()));
		    }
		  (void) exp->create_dynfunc (dynfuncModule,
					      (char*) attrs->getValue (NTXT ("funcname")), vaddr, msize);
		}
	      else if (strcmp (str, NTXT ("jcm")) == 0)
		{
		  str = attrs->getValue (NTXT ("methodId"));
		  Vaddr mid = str ? strtoull (str, NULL, 0) : 0;
		  exp->process_jcm_load_cmd (NULL, mid, vaddr, msize, ts);
		}
	    }
	  else if (strcmp (str, NTXT ("unmap")) == 0)
	    {
	      ts += exp->exp_start_time;
	      str = attrs->getValue (NTXT ("vaddr"));
	      Vaddr vaddr = str ? strtoull (str, NULL, 0) : 0;
	      exp->process_seg_unmap_cmd (NULL, ts, vaddr);
	    }
	}
      // end of code for event
    }
  else if (strcmp (qName, SP_TAG_PROFILE) == 0)
    {
      pushElem (EL_PROFILE);
      const char *str = attrs->getValue (NTXT ("name"));
      if (str == NULL)
	return;
      if (strcmp (str, NTXT ("profile")) == 0)
	{
	  exp->coll_params.profile_mode = 1;
	  str = attrs->getValue (NTXT ("numstates"));
	  if (str != NULL)
	    exp->coll_params.lms_magic_id = atoi (str);
	  str = attrs->getValue (NTXT ("ptimer"));
	  if (str != NULL)
	    exp->coll_params.ptimer_usec = atoi (str); // microseconds

	  PropDescr *mstate_prop = NULL;
	  char * stateNames [/*LMS_NUM_STATES*/] = LMS_STATE_STRINGS;
	  char * stateUNames[/*LMS_NUM_STATES*/] = LMS_STATE_USTRINGS;
	  {
	    dDscr = exp->newDataDescriptor (DATA_CLOCK);
	    PropDescr *prop = new PropDescr (PROP_MSTATE, NTXT ("MSTATE"));
	    prop->uname = dbe_strdup (GTXT ("Thread state"));
	    prop->vtype = TYPE_UINT32;
	    // (states added below)
	    dDscr->addProperty (prop);
	    mstate_prop = prop;

	    prop = new PropDescr (PROP_NTICK, NTXT ("NTICK"));
	    prop->uname = dbe_strdup (GTXT ("Number of Profiling Ticks"));
	    prop->vtype = TYPE_UINT32;
	    dDscr->addProperty (prop);
	  }

	  switch (exp->coll_params.lms_magic_id)
	    {
	    case LMS_MAGIC_ID_SOLARIS:
	      exp->register_metric (Metric::CP_TOTAL);
	      exp->register_metric (Metric::CP_TOTAL_CPU);
	      exp->register_metric (Metric::CP_LMS_USER);
	      exp->register_metric (Metric::CP_LMS_SYSTEM);
	      exp->register_metric (Metric::CP_LMS_TRAP);
	      exp->register_metric (Metric::CP_LMS_DFAULT);
	      exp->register_metric (Metric::CP_LMS_TFAULT);
	      exp->register_metric (Metric::CP_LMS_KFAULT);
	      exp->register_metric (Metric::CP_LMS_STOPPED);
	      exp->register_metric (Metric::CP_LMS_WAIT_CPU);
	      exp->register_metric (Metric::CP_LMS_SLEEP);
	      exp->register_metric (Metric::CP_LMS_USER_LOCK);
	      for (int ii = 0; ii < LMS_NUM_SOLARIS_MSTATES; ii++)
		mstate_prop->addState (ii, stateNames[ii], stateUNames[ii]);
	      break;
	    case LMS_MAGIC_ID_ERKERNEL_KERNEL:
	      exp->register_metric (Metric::CP_KERNEL_CPU);
	      {
		int ii = LMS_KERNEL_CPU;
		mstate_prop->addState (ii, stateNames[ii], stateUNames[ii]);
	      }
	      break;
	    case LMS_MAGIC_ID_ERKERNEL_USER:
	      exp->register_metric (Metric::CP_TOTAL_CPU);
	      exp->register_metric (Metric::CP_LMS_USER);
	      exp->register_metric (Metric::CP_LMS_SYSTEM);
	      {
		int ii = LMS_KERNEL_CPU;
		mstate_prop->addState (ii, stateNames[ii], stateUNames[ii]);
		ii = LMS_USER;
		mstate_prop->addState (ii, stateNames[ii], stateUNames[ii]);
		ii = LMS_SYSTEM;
		mstate_prop->addState (ii, stateNames[ii], stateUNames[ii]);
	      }
	      break;
	    case LMS_MAGIC_ID_LINUX:
	      exp->register_metric (Metric::CP_TOTAL_CPU);
	      {
		int ii = LMS_LINUX_CPU;
		mstate_prop->addState (ii, stateNames[ii], stateUNames[ii]);
	      }
	      break;
	    default:
	      // odd
	      break;
	    }
	}
      else if (strcmp (str, NTXT ("heaptrace")) == 0)
	{
	  exp->coll_params.heap_mode = 1;
	  exp->leaklistavail = true;
	  exp->heapdataavail = true;
	  exp->register_metric (Metric::HEAP_ALLOC_BYTES);
	  exp->register_metric (Metric::HEAP_ALLOC_CNT);
	  exp->register_metric (Metric::HEAP_LEAK_BYTES);
	  exp->register_metric (Metric::HEAP_LEAK_CNT);
	  dDscr = exp->newDataDescriptor (DATA_HEAP);
	}
      else if (strcmp (str, NTXT ("iotrace")) == 0)
	{
	  exp->coll_params.io_mode = 1;
	  exp->iodataavail = true;
	  exp->register_metric (Metric::IO_READ_TIME);
	  exp->register_metric (Metric::IO_READ_BYTES);
	  exp->register_metric (Metric::IO_READ_CNT);
	  exp->register_metric (Metric::IO_WRITE_TIME);
	  exp->register_metric (Metric::IO_WRITE_BYTES);
	  exp->register_metric (Metric::IO_WRITE_CNT);
	  exp->register_metric (Metric::IO_OTHER_TIME);
	  exp->register_metric (Metric::IO_OTHER_CNT);
	  exp->register_metric (Metric::IO_ERROR_TIME);
	  exp->register_metric (Metric::IO_ERROR_CNT);
	  dDscr = exp->newDataDescriptor (DATA_IOTRACE);
	}
      else if (strcmp (str, NTXT ("synctrace")) == 0)
	{
	  exp->coll_params.sync_mode = 1;
	  str = attrs->getValue (NTXT ("threshold"));
	  if (str != NULL)
	    exp->coll_params.sync_threshold = atoi (str);
	  str = attrs->getValue (NTXT ("scope"));
	  if (str != NULL)
	    exp->coll_params.sync_scope = atoi (str);
	  else  // Should only happen with old experiments; use the old default
	    exp->coll_params.sync_scope = SYNCSCOPE_NATIVE | SYNCSCOPE_JAVA;
	  exp->register_metric (Metric::SYNC_WAIT_TIME);
	  exp->register_metric (Metric::SYNC_WAIT_COUNT);
	  dDscr = exp->newDataDescriptor (DATA_SYNCH);
	}
      else if (strcmp (str, NTXT ("omptrace")) == 0)
	{
	  exp->coll_params.omp_mode = 1;
	  dDscr = exp->newDataDescriptor (DATA_OMP, DDFLAG_NOSHOW);
	}
      else if (strcmp (str, NTXT ("hwcounter")) == 0)
	{
	  str = attrs->getValue (NTXT ("cpuver"));
	  int cpuver = str ? atoi (str) : 0;
	  char *counter = dbe_strdup (attrs->getValue (NTXT ("hwcname")));
	  char *int_name = dbe_strdup (attrs->getValue (NTXT ("int_name"))); // may not be present
	  str = attrs->getValue (NTXT ("interval"));
	  int interval = str ? atoi (str) : 0;
	  str = attrs->getValue (NTXT ("tag"));
	  int tag = str ? atoi (str) : 0;
	  str = attrs->getValue (NTXT ("memop"));
	  int i_tpc = str ? atoi (str) : 0;
	  char *modstr = dbe_strdup (attrs->getValue (NTXT ("modstr")));
	  exp->process_hwcounter_cmd (NULL, cpuver, counter, int_name, interval, tag, i_tpc, modstr);
	  dDscr = exp->newDataDescriptor (DATA_HWC);
	}
      else if (strcmp (str, NTXT ("hwsimctr")) == 0)
	{
	  int cpuver = toInt (attrs, NTXT ("cpuver"));
	  char *hwcname = dbe_strdup (attrs->getValue (NTXT ("hwcname")));
	  char *int_name = dbe_strdup (attrs->getValue (NTXT ("int_name")));
	  char *metric = dbe_strdup (attrs->getValue (NTXT ("metric")));
	  int reg = toInt (attrs, NTXT ("reg_num"));
	  int interval = toInt (attrs, NTXT ("interval"));
	  int timecvt = toInt (attrs, NTXT ("timecvt"));
	  int i_tpc = toInt (attrs, NTXT ("memop"));
	  int tag = toInt (attrs, NTXT ("tag"));
	  exp->process_hwsimctr_cmd (NULL, cpuver, hwcname, int_name, metric, reg,
				     interval, timecvt, i_tpc, tag);
	  dDscr = exp->newDataDescriptor (DATA_HWC);
	}
      else if (strcmp (str, NTXT ("dversion")) == 0)
	exp->dversion = dbe_strdup (attrs->getValue (NTXT ("version")));
      else if (strcmp (str, NTXT ("jprofile")) == 0)
	{
	  exp->has_java = true;
	  str = attrs->getValue (NTXT ("jversion"));
	  if (str != NULL)
	    exp->jversion = strdup (str);
	}
      else if (strcmp (str, NTXT ("datarace")) == 0)
	{
	  exp->coll_params.race_mode = 1;
	  exp->racelistavail = true;
	  str = attrs->getValue (NTXT ("scheme"));
	  exp->coll_params.race_stack = str ? atoi (str) : 0;
	  exp->register_metric (Metric::RACCESS);
	  dDscr = exp->newDataDescriptor (DATA_RACE);
	}
      else if (strcmp (str, NTXT ("deadlock")) == 0)
	{
	  exp->coll_params.deadlock_mode = 1;
	  exp->deadlocklistavail = true;
	  exp->register_metric (Metric::DEADLOCKS);
	  dDscr = exp->newDataDescriptor (DATA_DLCK);
	}
    }
    /* XXX -- obsolete tag, but is still written to experiments */
  else if (strcmp (qName, SP_TAG_DATAPTR) == 0)
    {
      pushElem (EL_DATAPTR);
      return;
    }
  else if (strcmp (qName, SP_TAG_PROFDATA) == 0)
    {
      pushElem (EL_PROFDATA);
      // SS12 HWC experiments are not well structured
      const char *fname = attrs->getValue (NTXT ("fname"));
      if (fname && strcmp (fname, SP_HWCNTR_FILE) == 0)
	dDscr = exp->newDataDescriptor (DATA_HWC);
    }
  else if (strcmp (qName, SP_TAG_PROFPCKT) == 0)
    {
      pushElem (EL_PROFPCKT);
      const char *str = attrs->getValue (NTXT ("kind")); // see Pckt_type
      int kind = str ? atoi (str) : -1;
      if (kind < 0)
	return;
      if (exp->coll_params.omp_mode == 1)
	{
	  if (kind == OMP_PCKT)
	    dDscr = exp->newDataDescriptor (DATA_OMP, DDFLAG_NOSHOW);
	  else if (kind == OMP2_PCKT)
	    dDscr = exp->newDataDescriptor (DATA_OMP2, DDFLAG_NOSHOW);
	  else if (kind == OMP3_PCKT)
	    dDscr = exp->newDataDescriptor (DATA_OMP3, DDFLAG_NOSHOW);
	  else if (kind == OMP4_PCKT)
	    dDscr = exp->newDataDescriptor (DATA_OMP4, DDFLAG_NOSHOW);
	  else if (kind == OMP5_PCKT)
	    dDscr = exp->newDataDescriptor (DATA_OMP5, DDFLAG_NOSHOW);
	}
      pDscr = exp->newPacketDescriptor (kind, dDscr);
      return;
    }
  else if (strcmp (qName, SP_TAG_FIELD) == 0)
    {
      pushElem (EL_FIELD);
      if (pDscr != NULL)
	{
	  const char *name = attrs->getValue (NTXT ("name"));
	  if (name == NULL)
	    return;
	  int propID = dbeSession->registerPropertyName (name);
	  propDscr = new PropDescr (propID, name);
	  FieldDescr *fldDscr = new FieldDescr (propID, name);

	  const char *str = attrs->getValue (NTXT ("type"));
	  if (str)
	    {
	      if (strcmp (str, NTXT ("INT32")) == 0)
		fldDscr->vtype = TYPE_INT32;
	      else if (strcmp (str, NTXT ("UINT32")) == 0)
		fldDscr->vtype = TYPE_UINT32;
	      else if (strcmp (str, NTXT ("INT64")) == 0)
		fldDscr->vtype = TYPE_INT64;
	      else if (strcmp (str, NTXT ("UINT64")) == 0)
		fldDscr->vtype = TYPE_UINT64;
	      else if (strcmp (str, NTXT ("STRING")) == 0)
		fldDscr->vtype = TYPE_STRING;
	      else if (strcmp (str, NTXT ("DOUBLE")) == 0)
		fldDscr->vtype = TYPE_DOUBLE;
	      else if (strcmp (str, NTXT ("DATE")) == 0)
		{
		  fldDscr->vtype = TYPE_DATE;
		  const char *fmt = attrs->getValue (NTXT ("format"));
		  fldDscr->format = strdup (fmt ? fmt : "");
		}
	    }
	  propDscr->vtype = fldDscr->vtype;

	  // TYPE_DATE is converted to TYPE_UINT64 in propDscr
	  if (fldDscr->vtype == TYPE_DATE)
	    propDscr->vtype = TYPE_UINT64;

	  // Fix some types until they are fixed in libcollector
	  if (propID == PROP_VIRTPC || propID == PROP_PHYSPC)
	    {
	      if (fldDscr->vtype == TYPE_INT32)
		propDscr->vtype = TYPE_UINT32;
	      else if (fldDscr->vtype == TYPE_INT64)
		propDscr->vtype = TYPE_UINT64;
	    }

	  // The following props get mapped to 32-bit values in readPacket
	  if (propID == PROP_CPUID || propID == PROP_THRID
	      || propID == PROP_LWPID)
	    propDscr->vtype = TYPE_UINT32; // override experiment property

	  str = attrs->getValue (NTXT ("uname"));
	  if (str)
	    propDscr->uname = strdup (PTXT ((char*) str));
	  str = attrs->getValue (NTXT ("noshow"));
	  if (str && atoi (str) != 0)
	    propDscr->flags |= PRFLAG_NOSHOW;

	  if (dDscr == NULL)
	    {
	      StringBuilder sb;
	      sb.sprintf (GTXT ("*** Error: data parsing failed. Log file is corrupted."));
	      exp->warnq->append (new Emsg (CMSG_ERROR, sb));
	      throw new SAXException (sb.toString ());
	    }

	  dDscr->addProperty (propDscr);
	  str = attrs->getValue (NTXT ("offset"));
	  if (str)
	    fldDscr->offset = atoi (str);
	  pDscr->addField (fldDscr);
	}
    }
  else if (strcmp (qName, SP_TAG_STATE) == 0)
    {
      pushElem (EL_STATE);
      if (propDscr != NULL)
	{
	  const char *str = attrs->getValue (NTXT ("value"));
	  int value = str ? atoi (str) : -1;
	  str = attrs->getValue (NTXT ("name"));
	  const char *ustr = attrs->getValue (NTXT ("uname"));
	  propDscr->addState (value, str, ustr);
	}
    }
  else if (strcmp (qName, SP_TAG_DTRACEFATAL) == 0)
    pushElem (EL_DTRACEFATAL);
  else
    {
      StringBuilder sb;
      sb.sprintf (GTXT ("*** Warning: unrecognized element %s"), qName);
      exp->warnq->append (new Emsg (CMSG_WARN, sb));
      pushElem (EL_NONE);
    }
}

void
Experiment::ExperimentHandler::characters (char *ch, int start, int length)
{
  switch (curElem)
    {
    case EL_COLLECTOR:
      exp->cversion = dbe_strndup (ch + start, length);
      break;
    case EL_PROCESS:
      exp->process_arglist_cmd (NULL, dbe_strndup (ch + start, length));
      break;
    case EL_EVENT:
      free (text);
      text = dbe_strndup (ch + start, length);
      break;
    default:
      break;
    }
}

void
Experiment::ExperimentHandler::endElement (char*, char*, char*)
{
  if (curElem == EL_EVENT && mkind >= 0 && mnum >= 0)
    {
      char *str;
      if (mec > 0)
	str = dbe_sprintf ("%s -- %s", text != NULL ? text : "", strerror (mec));
      else
	str = dbe_sprintf ("%s", text != NULL ? text : "");
      Emsg *msg = new Emsg (mkind, mnum, str);
      if (mkind == CMSG_WARN)
	{
	  if (mnum != COL_WARN_FSTYPE
	      || dbeSession->check_ignore_fs_warn () == false)
	    exp->warnq->append (msg);
	  else
	    exp->commentq->append (msg);
	}
      else if (mkind == CMSG_ERROR || mkind == CMSG_FATAL)
	exp->errorq->append (msg);
      else if (mkind == CMSG_COMMENT)
	exp->commentq->append (msg);
      else
	delete msg;
      mkind = (Cmsg_warn) - 1;
      mnum = -1;
      mec = -1;
    }
  else if (curElem == EL_PROFILE)
    dDscr = NULL;
  else if (curElem == EL_PROFPCKT)
    pDscr = NULL;
  else if (curElem == EL_FIELD)
    propDscr = NULL;
  free (text);
  text = NULL;
  popElem ();
}

void
Experiment::ExperimentHandler::error (SAXParseException *e)
{
  StringBuilder sb;
  sb.sprintf (GTXT ("%s at line %d, column %d"),
	      e->getMessage (), e->getLineNumber (), e->getColumnNumber ());
  char *msg = sb.toString ();
  SAXException *e1 = new SAXException (msg);
  free (msg);
  throw ( e1);
}

//-------------------------------------------------- Experiment

Experiment::Experiment ()
{
  groupId = 0;
  userExpId = expIdx = -1;
  founder_exp = NULL;
  baseFounder = NULL;
  children_exps = new Vector<Experiment*>;
  loadObjs = new Vector<LoadObject*>;
  loadObjMap = new StringMap<LoadObject*>(128, 128);
  sourcesMap = NULL;

  // Initialize configuration information.
  status = FAILURE;
  start_sec = 0;
  mtime = 0;
  hostname = NULL;
  username = NULL;
  architecture = NULL;
  os_version = NULL;
  uarglist = NULL;
  utargname = NULL;
  ucwd = NULL;
  cversion = NULL;
  dversion = NULL;
  jversion = NULL;
  exp_maj_version = 0;
  exp_min_version = 0;
  platform = Unknown;
  wsize = Wnone;
  page_size = 4096;
  npages = 0;
  stack_base = 0xf0000000;
  broken = 1;
  obsolete = 0;
  hwc_bogus = 0;
  hwc_lost_int = 0;
  hwc_scanned = 0;
  hwc_default = false;
  invalid_packet = 0;

  // clear HWC event stats
  dsevents = 0;
  dsnoxhwcevents = 0;

  memset (&coll_params, 0, sizeof (coll_params));
  ncpus = 0;
  minclock = 0;
  maxclock = 0;
  clock = 0;
  varclock = 0;
  exec_started = false;
  timelineavail = true;
  leaklistavail = false;
  heapdataavail = false;
  iodataavail = false;
  dataspaceavail = false;
  ifreqavail = false;
  racelistavail = false;
  deadlocklistavail = false;
  ompavail = false;
  tiny_threshold = -1;
  pid = 0;
  ppid = 0;
  pgrp = 0;
  sid = 0;

  gc_duration = ZERO_TIME;
  exp_start_time = ZERO_TIME; // not known.  Wall-clock hrtime (not zero based)
  last_event = ZERO_TIME; // not known.  Wall-clock hrtime (not zero based)
  non_paused_time = 0; // 0 non-paused time (will sum as experiment is processed)
  resume_ts = 0; // by default, collection is "resumed" (not paused) from time=0
  need_swap_endian = false;
  exp_rel_start_time_set = false;
  exp_rel_start_time = ZERO_TIME;
  has_java = false;
  hex_field_width = 8;
  hw_cpuver = CPUVER_UNDEFINED;
  machinemodel = NULL;
  expt_name = NULL;
  arch_name = NULL;
  fndr_arch_name = NULL;
  dyntext_name = NULL;
  logFile = NULL;

  dataDscrs = new Vector<DataDescriptor*>;
  for (int i = 0; i < DATA_LAST; ++i)
    dataDscrs->append (NULL);

  pcktDscrs = new Vector<PacketDescriptor*>;
  blksz = PROFILE_BUFFER_CHUNK;
  jthreads = new Vector<JThread*>;
  jthreads_idx = new Vector<JThread*>;
  gcevents = new Vector<GCEvent*>;
  gcevent_last_used = (GCEvent *) NULL;
  heapUnmapEvents = new Vector<UnmapChunk*>;
  cstack = NULL;
  cstackShowHide = NULL;
  frmpckts = new Vector<RawFramePacket*>;
  typedef DefaultMap2D<uint32_t, hrtime_t, uint64_t> OmpMap0;
  mapPRid = new OmpMap0 (OmpMap0::Interval);
  typedef DefaultMap2D<uint32_t, hrtime_t, void*> OmpMap;
  mapPReg = new OmpMap (OmpMap::Interval);
  mapTask = new OmpMap (OmpMap::Interval);
  openMPdata = NULL;
  archiveMap = NULL;
  nnodes = 0;
  nchunks = 0;
  chunks = 0;
  uidHTable = NULL;
  uidnodes = new Vector<UIDnode*>;
  mrecs = new Vector<MapRecord*>;
  samples = new Vector<Sample*>;
  sample_last_used = (Sample *) NULL;
  first_sample_label = (char*) NULL;
  fDataMap = NULL;
  vFdMap = NULL;
  resolveFrameInfo = true;
  discardTiny = false;
  init ();
}

Experiment::~Experiment ()
{
  fini ();
  free (coll_params.linetrace);
  for (int i = 0; i < MAX_HWCOUNT; i++)
    {
      free (coll_params.hw_aux_name[i]);
      free (coll_params.hw_username[i]);
    }
  free (hostname);
  free (username);
  free (architecture);
  free (os_version);
  free (uarglist);
  free (utargname);
  free (ucwd);
  free (cversion);
  free (dversion);
  free (jversion);
  delete logFile;
  free (expt_name);
  free (arch_name);
  free (fndr_arch_name);
  free (dyntext_name);
  delete jthreads_idx;
  delete cstack;
  delete cstackShowHide;
  delete mapPRid;
  delete mapPReg;
  delete mapTask;
  delete openMPdata;
  destroy_map (DbeFile *, archiveMap);
  delete[] uidHTable;
  delete uidnodes;
  delete mrecs;
  delete children_exps;
  delete loadObjs;
  delete loadObjMap;
  delete sourcesMap;
  free (first_sample_label);
  free (machinemodel);

  dataDscrs->destroy ();
  delete dataDscrs;
  pcktDscrs->destroy ();
  delete pcktDscrs;
  jthreads->destroy ();
  delete jthreads;
  gcevents->destroy ();
  delete gcevents;
  heapUnmapEvents->destroy ();
  delete heapUnmapEvents;
  frmpckts->destroy ();
  delete frmpckts;
  samples->destroy ();
  delete samples;
  delete fDataMap;
  delete vFdMap;

  for (long i = 0; i < nchunks; i++)
    delete[] chunks[i];
  delete[] chunks;
}

void
Experiment::init_cache ()
{
  if (smemHTable)
    return;
  smemHTable = new SegMem*[HTableSize];
  instHTable = new DbeInstr*[HTableSize];
  for (int i = 0; i < HTableSize; i++)
    {
      smemHTable[i] = NULL;
      instHTable[i] = NULL;
    }
  uidHTable = new UIDnode*[HTableSize];
  for (int i = 0; i < HTableSize; i++)
    uidHTable[i] = NULL;

  cstack = CallStack::getInstance (this);
  cstackShowHide = CallStack::getInstance (this);
}

void
Experiment::init ()
{
  userLabels = NULL;
  seg_items = new Vector<SegMem*>;
  maps = new PRBTree ();
  jmaps = NULL; // used by JAVA_CLASSES only
  jmidHTable = NULL;
  smemHTable = NULL;
  instHTable = NULL;
  min_thread = (uint64_t) - 1;
  max_thread = 0;
  thread_cnt = 0;
  min_lwp = (uint64_t) - 1;
  max_lwp = 0;
  lwp_cnt = 0;
  min_cpu = (uint64_t) - 1;
  max_cpu = 0;
  cpu_cnt = 0;

  commentq = new Emsgqueue (NTXT ("commentq"));
  runlogq = new Emsgqueue (NTXT ("runlogq"));
  errorq = new Emsgqueue (NTXT ("errorq"));
  warnq = new Emsgqueue (NTXT ("warnq"));
  notesq = new Emsgqueue (NTXT ("notesq"));
  pprocq = new Emsgqueue (NTXT ("pprocq"));
  ifreqq = NULL;

  metrics = new Vector<BaseMetric*>;
  tagObjs = new Vector<Vector<Histable*>*>;
  tagObjs->store (PROP_THRID, new Vector<Histable*>);
  tagObjs->store (PROP_LWPID, new Vector<Histable*>);
  tagObjs->store (PROP_CPUID, new Vector<Histable*>);
  tagObjs->store (PROP_EXPID, new Vector<Histable*>);
  sparse_threads = false;
}

void
Experiment::fini ()
{
  seg_items->destroy ();
  delete seg_items;
  delete maps;
  delete jmaps;
  delete[] smemHTable;
  delete[] instHTable;
  delete jmidHTable;
  delete commentq;
  delete runlogq;
  delete errorq;
  delete warnq;
  delete notesq;
  delete pprocq;
  if (ifreqq != NULL)
    {
      delete ifreqq;
      ifreqq = NULL;
    }

  int index;
  BaseMetric *mtr;
  Vec_loop (BaseMetric*, metrics, index, mtr)
  {
    dbeSession->drop_metric (mtr);
  }
  delete metrics;
  tagObjs->fetch (PROP_THRID)->destroy ();
  tagObjs->fetch (PROP_LWPID)->destroy ();
  tagObjs->fetch (PROP_CPUID)->destroy ();
  tagObjs->fetch (PROP_EXPID)->destroy ();
  tagObjs->destroy ();
  delete tagObjs;
}

// These are the data files which can be read in parallel
// for multiple sub-experiments.
// Postpone calling resolve_frame_info()
void
Experiment::read_experiment_data (bool read_ahead)
{

  read_frameinfo_file ();
  if (read_ahead)
    {
      resolveFrameInfo = false;
      (void) get_profile_events ();
      resolveFrameInfo = true;
    }
}

Experiment::Exp_status
Experiment::open_epilogue ()
{

  // set up mapping for tagObj(PROP_EXPID)
  (void) mapTagValue (PROP_EXPID, userExpId);

  post_process ();
  if (last_event != ZERO_TIME)
    { // if last_event is known
      StringBuilder sb;
      hrtime_t ts = last_event - exp_start_time;
      sb.sprintf (GTXT ("Experiment Ended: %ld.%09ld\nData Collection Duration: %ld.%09ld"),
		  (long) (ts / NANOSEC), (long) (ts % NANOSEC),
		  (long) (non_paused_time / NANOSEC),
		  (long) (non_paused_time % NANOSEC));
      runlogq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // Check for incomplete experiment, and inform the user
  if (status == INCOMPLETE)
    {
      if (exec_started == true)
	// experiment ended with the exec, not abnormally
	status = SUCCESS;
      else
	{
	  char * cmnt = GTXT ("*** Note: experiment was not closed");
	  commentq->append (new Emsg (CMSG_COMMENT, cmnt));
	  // runlogq->append(new Emsg(CMSG_COMMENT, cmnt));
	}
    }
  // write a descriptive header for the experiment
  write_header ();
  return status;
}

Experiment::Exp_status
Experiment::open (char *path)
{

  // Find experiment directory
  if (find_expdir (path) != SUCCESS)
    // message will have been queued and status set
    return status;

  // Get creation time for experiment
  struct stat64 st;
  if (dbe_stat (path, &st) == 0)
    mtime = st.st_mtime;

  // Read the warnings file
  read_warn_file ();

  // Open the log file
  read_log_file ();
  if (status == SUCCESS && last_event // last event is initialized
      && (last_event - exp_start_time) / 1000000 < tiny_threshold)
    {
      // Process "tiny_threshold" (SP_ANALYZER_DISCARD_TINY_EXPERIMENTS)
      // At this point, we've only processed log.xml.
      // Note: if an experiment terminated abnormally, last_event will not yet
      //   represent events from clock profiling and other metrics.
      //   Other events will often have timestamps after the last log.xml entry.
      discardTiny = true;
      return status;
    }
  if (status == FAILURE)
    {
      if (logFile->get_status () == ExperimentFile::EF_FAILURE)
	{
	  Emsg *m = new Emsg (CMSG_FATAL, GTXT ("*** Error: log file in experiment cannot be read"));
	  errorq->append (m);
	}
      else if (fetch_errors () == NULL)
	{
	  if (broken == 1)
	    {
	      Emsg *m = new Emsg (CMSG_FATAL, GTXT ("*** Error: log does not show target starting"));
	      errorq->append (m);
	    }
	  else
	    {
	      Emsg *m = new Emsg (CMSG_FATAL, GTXT ("*** Error: log file in experiment could not be parsed"));
	      errorq->append (m);
	    }
	}
      return status;
    }
  init_cache ();
  if (varclock != 0)
    {
      StringBuilder sb;
      sb.sprintf (
		  GTXT ("*** Warning: system has variable clock frequency, which may cause variable execution times and inaccurate conversions of cycle counts into time."));
      warnq->append (new Emsg (CMSG_WARN, sb));
    }

  // Read the notes file
  read_notes_file ();
  read_labels_file ();
  read_archives ();

  // The log file shows experiment started
  read_java_classes_file ();

  read_map_file ();

  // Dyntext file has to be processed after loadobjects file
  // as we need to be able to map (vaddr,ts) to dynamic functions.
  read_dyntext_file ();

  // Read the overview file and create samples.
  // Profiling data hasn't been read yet so we may have
  // events after the last recorded sample.
  // We'll create a fake sample to cover all those
  // events later.
  read_overview_file ();

  // Check if instruction frequency data is available
  read_ifreq_file ();

  // Check if OMP data is available
  read_omp_file ();

  return status;
}

/* XXX -- update() is a no-op now, but may be needed for auto-update */
Experiment::Exp_status
Experiment::update ()
{
  return status;
}

void
Experiment::append (LoadObject *lo)
{
  loadObjs->append (lo);
  char *obj_name = lo->get_pathname ();
  char *bname = get_basename (obj_name);
  loadObjMap->put (obj_name, lo);
  loadObjMap->put (bname, lo);
  if (lo->flags & SEG_FLAG_EXE)
    loadObjMap->put (COMP_EXE_NAME, lo);
}

void
Experiment::read_notes_file ()
{
  Emsg *m;

  // Open log file:
  char *fname = dbe_sprintf (NTXT ("%s/%s"), expt_name, SP_NOTES_FILE);
  FILE *f = fopen (fname, NTXT ("r"));
  free (fname);
  if (f == NULL)
    return;
  if (!dbeSession->is_interactive ())
    {
      m = new Emsg (CMSG_COMMENT, NTXT ("Notes:"));
      notesq->append (m);
    }

  while (1)
    {
      char str[MAXPATHLEN];
      char *e = fgets (str, ((int) sizeof (str)) - 1, f);
      if (e == NULL)
	{
	  if (!dbeSession->is_interactive ())
	    {
	      m = new Emsg (CMSG_COMMENT,
			    "============================================================");
	      notesq->append (m);
	    }
	  break;
	}
      size_t i = strlen (str);
      if (i > 0 && str[i - 1] == '\n')
	// remove trailing nl
	str[i - 1] = 0;
      m = new Emsg (CMSG_COMMENT, str);
      notesq->append (m);
    }
  (void) fclose (f);
}

int
Experiment::save_notes (char* text, bool handle_file)
{
  if (handle_file)
    {
      FILE *fnotes;
      char *fname = dbe_sprintf (NTXT ("%s/%s"), expt_name, SP_NOTES_FILE);
      fnotes = fopen (fname, NTXT ("w"));
      free (fname);
      if (fnotes != NULL)
	{
	  (void) fprintf (fnotes, NTXT ("%s"), text);
	  fclose (fnotes);
	}
      else
	return 1; // Cannot write file
    }
  notesq->clear ();
  Emsg *m = new Emsg (CMSG_COMMENT, text);
  notesq->append (m);

  return 0;
}

int
Experiment::delete_notes (bool handle_file)
{
  if (handle_file)
    {
      char *fname = dbe_sprintf (NTXT ("%s/%s"), expt_name, SP_NOTES_FILE);
      if (unlink (fname) != 0)
	{
	  free (fname);
	  return 1; // Cannot delete file
	}
      free (fname);
    }
  notesq->clear ();
  return 0;
}

int
Experiment::read_warn_file ()
{
  int local_status = SUCCESS;

  ExperimentFile *warnFile = new ExperimentFile (this, SP_WARN_FILE);
  if (warnFile == NULL)
    return FAILURE;
  if (!warnFile->open ())
    {
      delete warnFile;
      return FAILURE;
    }
  SAXParserFactory *factory = SAXParserFactory::newInstance ();
  SAXParser *saxParser = factory->newSAXParser ();
  DefaultHandler *dh = new ExperimentHandler (this);
  try
    {
      saxParser->parse ((File*) warnFile->fh, dh);
    }
  catch (SAXException *e)
    {
      // Fatal error in the parser
      StringBuilder sb;
      sb.sprintf (NTXT ("%s: %s"), SP_WARN_FILE, e->getMessage ());
      char *str = sb.toString ();
      Emsg *m = new Emsg (CMSG_FATAL, str);
      errorq->append (m);
      local_status = FAILURE;
      delete e;
    }
  delete warnFile;
  delete dh;
  delete saxParser;
  delete factory;

  return local_status;
}

int
Experiment::read_log_file ()
{
  if (logFile == NULL)
    logFile = new ExperimentFile (this, SP_LOG_FILE);
  if (!logFile->open ())
    {
      status = FAILURE;
      return status;
    }

  SAXParserFactory *factory = SAXParserFactory::newInstance ();
  SAXParser *saxParser = factory->newSAXParser ();
  DefaultHandler *dh = new ExperimentHandler (this);
  try
    {
      saxParser->parse ((File*) logFile->fh, dh);
    }
  catch (SAXException *e)
    {
      // Fatal error in the parser
      StringBuilder sb;
      if (obsolete == 1)
	sb.sprintf (NTXT ("%s"), e->getMessage ());
      else
	sb.sprintf (NTXT ("%s: %s"), SP_LOG_FILE, e->getMessage ());
      char *str = sb.toString ();
      Emsg *m = new Emsg (CMSG_FATAL, str);
      errorq->append (m);
      status = FAILURE;
      delete e;
    }
  logFile->close ();
  dbeSession->register_metric (GTXT ("IPC"), GTXT ("Instructions Per Cycle"),
			       NTXT ("insts/cycles"));
  dbeSession->register_metric (GTXT ("CPI"), GTXT ("Cycles Per Instruction"),
			       NTXT ("cycles/insts"));
  dbeSession->register_metric (GTXT ("K_IPC"),
			       GTXT ("Kernel Instructions Per Cycle"),
			       NTXT ("K_insts/K_cycles"));
  dbeSession->register_metric (GTXT ("K_CPI"),
			       GTXT ("Kernel Cycles Per Instruction"),
			       NTXT ("K_cycles/K_insts"));

  delete dh;
  delete saxParser;
  delete factory;

  return status;
}

////////////////////////////////////////////////////////////////////////////////
//  class Experiment::ExperimentLabelsHandler
//

class Experiment::ExperimentLabelsHandler : public DefaultHandler
{
public:

  ExperimentLabelsHandler (Experiment *_exp)
  {
    exp = _exp;
  }

  ~ExperimentLabelsHandler () { };
  void startDocument () { }
  void endDocument () { }
  void endElement (char * /*uri*/, char * /*localName*/, char * /*qName*/) { }
  void characters (char * /*ch*/, int /*start*/, int /*length*/) { }
  void ignorableWhitespace (char*, int, int) { }
  void error (SAXParseException * /*e*/) { }

  void startElement (char *uri, char *localName, char *qName, Attributes *attrs);

private:

  inline const char *
  s2s (const char *s)
  {
    return s ? s : "NULL";
  }

  Experiment *exp;
  char *hostname;
  hrtime_t time, tstamp;
};

void
Experiment::ExperimentLabelsHandler::startElement (char*, char*, char *qName,
						   Attributes *attrs)
{
  DEBUG_CODE if (DEBUG_SAXPARSER) dump_startElement (qName, attrs);
  if (qName == NULL || strcmp (qName, NTXT ("id")) != 0)
    return;
  char *name = NULL, *all_times = NULL, *comment = NULL, *hostName = NULL;
  long startSec = 0;
  //    long tm_zone = 0;
  hrtime_t startHrtime = (hrtime_t) 0;
  long long lbl_ts = 0;
  int relative = 0;
  timeval start_tv;
  start_tv.tv_usec = start_tv.tv_sec = 0;
  for (int i = 0, sz = attrs ? attrs->getLength () : 0; i < sz; i++)
    {
      const char *qn = attrs->getQName (i);
      const char *vl = attrs->getValue (i);
      if (strcmp (qn, NTXT ("name")) == 0)
	name = dbe_xml2str (vl);
      else if (strcmp (qn, NTXT ("cmd")) == 0)
	all_times = dbe_xml2str (vl);
      else if (strcmp (qn, NTXT ("comment")) == 0)
	comment = dbe_xml2str (vl);
      else if (strcmp (qn, NTXT ("relative")) == 0)
	relative = atoi (vl);
      else if (strcmp (qn, NTXT ("hostname")) == 0)
	hostName = dbe_xml2str (vl);
      else if (strcmp (qn, NTXT ("time")) == 0)
	startSec = atol (vl);
      else if (strcmp (qn, NTXT ("tstamp")) == 0)
	startHrtime = parseTStamp (vl);
      else if (strcmp (qn, NTXT ("lbl_ts")) == 0)
	{
	  if (*vl == '-')
	    lbl_ts = -parseTStamp (vl + 1);
	  else
	    lbl_ts = parseTStamp (vl);
	}
    }
  if (name == NULL || hostName == NULL || (all_times == NULL && comment == NULL))
    {
      free (name);
      free (hostName);
      free (all_times);
      free (comment);
      return;
    }
  UserLabel *lbl = new UserLabel (name);
  lbl->comment = comment;
  lbl->hostname = hostName;
  lbl->start_sec = startSec;
  lbl->start_hrtime = startHrtime;
  exp->userLabels->append (lbl);
  if (all_times)
    {
      lbl->all_times = all_times;
      lbl->start_tv = start_tv;
      lbl->relative = relative;
      if (relative == UserLabel::REL_TIME)
	lbl->atime = lbl_ts;
      else
	{ // relative == UserLabel::CUR_TIME
	  long long delta = 0;
	  if (exp->hostname && strcmp (lbl->hostname, exp->hostname) == 0)
	    delta = lbl_ts + (lbl->start_hrtime - exp->exp_start_time);
	  else
	    for (int i = 0; i < exp->userLabels->size (); i++)
	      {
		UserLabel *firstLbl = exp->userLabels->fetch (i);
		if (strcmp (lbl->hostname, firstLbl->hostname) == 0)
		  {
		    delta = lbl_ts + (lbl->start_hrtime - firstLbl->start_hrtime) +
			    ((long long) (firstLbl->start_sec - exp->start_sec)) * NANOSEC;
		    break;
		  }
	      }
	  lbl->atime = delta > 0 ? delta : 0;
	}
    }
}

static int
sortUserLabels (const void *a, const void *b)
{
  UserLabel *l1 = *((UserLabel **) a);
  UserLabel *l2 = *((UserLabel **) b);
  int v = dbe_strcmp (l1->name, l2->name);
  if (v != 0)
    return v;
  if (l1->atime < l2->atime)
    return -1;
  else if (l1->atime > l2->atime)
    return 1;
  if (l1->id < l2->id)
    return -1;
  else if (l1->id > l2->id)
    return 1;
  return 0;
}

static char *
append_string (char *s, char *str)
{
  if (s == NULL)
    return dbe_strdup (str);
  char *new_s = dbe_sprintf (NTXT ("%s %s"), s, str);
  free (s);
  return new_s;
}

void
Experiment::read_labels_file ()
{
  ExperimentFile *fp = new ExperimentFile (this, SP_LABELS_FILE);
  if (!fp->open ())
    {
      delete fp;
      return;
    }
  userLabels = new Vector<UserLabel*>();
  SAXParserFactory *factory = SAXParserFactory::newInstance ();
  SAXParser *saxParser = factory->newSAXParser ();
  DefaultHandler *dh = new ExperimentLabelsHandler (this);
  try
    {
      saxParser->parse ((File*) fp->fh, dh);
    }
  catch (SAXException *e)
    {
      // Fatal error in the parser
      StringBuilder sb;
      sb.sprintf (NTXT ("%s: %s"), SP_LABELS_FILE, e->getMessage ());
      char *str = sb.toString ();
      Emsg *m = new Emsg (CMSG_FATAL, str);
      errorq->append (m);
      delete e;
    }
  fp->close ();
  delete fp;
  delete dh;
  delete saxParser;
  delete factory;

  userLabels->sort (sortUserLabels);
  UserLabel::dump ("After sortUserLabels:", userLabels);
  UserLabel *ulbl = NULL;
  for (int i = 0, sz = userLabels->size (); i < sz; i++)
    {
      UserLabel *lbl = userLabels->fetch (i);
      if (ulbl == NULL)
	ulbl = new UserLabel (lbl->name);
      else if (dbe_strcmp (lbl->name, ulbl->name) != 0)
	{ // new Label
	  ulbl->register_user_label (groupId);
	  if (ulbl->expr == NULL)
	    delete ulbl;
	  ulbl = new UserLabel (lbl->name);
	}
      if (lbl->all_times)
	{
	  if (strncmp (lbl->all_times, NTXT ("start"), 5) == 0)
	    {
	      if (!ulbl->start_f)
		{
		  ulbl->start_f = true;
		  ulbl->timeStart = lbl->atime;
		}
	    }
	  else
	    { // stop
	      if (!ulbl->start_f)
		continue;
	      ulbl->all_times = append_string (ulbl->all_times, lbl->all_times);
	      ulbl->stop_f = true;
	      ulbl->timeStop = lbl->atime;
	      ulbl->gen_expr ();
	    }
	}
      if (lbl->comment != NULL)
	ulbl->comment = append_string (ulbl->comment, lbl->comment);
    }
  if (ulbl)
    {
      ulbl->register_user_label (groupId);
      if (ulbl->expr == NULL)
	delete ulbl;
    }
  Destroy (userLabels);
}

void
Experiment::read_archives ()
{
  if (founder_exp)
    return;
  char *allocated_str = NULL;
  char *nm = get_arch_name ();
  DIR *exp_dir = opendir (nm);
  if (exp_dir == NULL)
    {
      if (founder_exp == NULL)
	{
	  // Check if the user uses a subexperiment only
	  nm = dbe_sprintf (NTXT ("%s/../%s"), expt_name, SP_ARCHIVES_DIR);
	  exp_dir = opendir (nm);
	  if (exp_dir == NULL)
	    {
	      free (nm);
	      return;
	    }
	  allocated_str = nm;
	}
      else
	return;
    }

  StringBuilder sb;
  sb.append (nm);
  sb.append ('/');
  int dlen = sb.length ();
  free (allocated_str);
  archiveMap = new StringMap<DbeFile *>();

  struct dirent *entry = NULL;
  while ((entry = readdir (exp_dir)) != NULL)
    {
      char *dname = entry->d_name;
      if (dname[0] == '.'
	  && (dname[1] == '\0' || (dname[1] == '.' && dname[2] == '\0')))
	// skip links to ./ or ../
	continue;
      sb.setLength (dlen);
      sb.append (dname);
      char *fnm = sb.toString ();
      DbeFile *df = new DbeFile (fnm);
      df->set_location (fnm);
      df->filetype |= DbeFile::F_FILE;
      df->inArchive = true;
      df->experiment = this;
      archiveMap->put (dname, df);
      free (fnm);
    }
  closedir (exp_dir);
}

static char *
gen_file_name (const char *packet_name, const char *src_name)
{
  char *fnm, *bname = get_basename (packet_name);
  if (bname == packet_name)
    fnm = dbe_strdup (src_name);
  else
    fnm = dbe_sprintf ("%.*s%s", (int) (bname - packet_name),
		       packet_name, src_name);

  // convert "java.lang.Object/Integer.java" => "java/lang/Object/Integer.java"
  bname = get_basename (fnm);
  for (char *s = fnm; s < bname; s++)
    if (*s == '.')
      *s = '/';
  return fnm;
}

static char *
get_jlass_name (const char *nm)
{
  // Convert "Ljava/lang/Object;" => "java/lang/Object.class"
  if (*nm == 'L')
    {
      size_t len = strlen (nm);
      if (nm[len - 1] == ';')
	return dbe_sprintf ("%.*s.class", (int) (len - 2), nm + 1);
    }
  return dbe_strdup (nm);
}

static char *
get_jmodule_name (const char *nm)
{
  // convert "Ljava/lang/Object;" => "java.lang.Object"
  if (*nm == 'L')
    {
      size_t len = strlen (nm);
      if (nm[len - 1] == ';')
	{
	  char *mname = dbe_sprintf (NTXT ("%.*s"), (int) (len - 2), nm + 1);
	  for (char *s = mname; *s; s++)
	    if (*s == '/')
	      *s = '.';
	  return mname;
	}
    }
  return dbe_strdup (nm);
}

LoadObject *
Experiment::get_j_lo (const char *className, const char *fileName)
{
  char *class_name = get_jlass_name (className);
  Dprintf (DUMP_JCLASS_READER,
	"Experiment::get_j_lo: className='%s' class_name='%s' fileName='%s'\n",
	   STR (className), STR (class_name), STR (fileName));
  LoadObject *lo = loadObjMap->get (class_name);
  if (lo == NULL)
    {
      lo = createLoadObject (class_name, fileName);
      lo->type = LoadObject::SEG_TEXT;
      lo->mtime = (time_t) 0;
      lo->size = 0;
      lo->set_platform (Java, wsize);
      lo->dbeFile->filetype |= DbeFile::F_FILE | DbeFile::F_JAVACLASS;
      append (lo);
      Dprintf (DUMP_JCLASS_READER,
	       "Experiment::get_j_lo: creates '%s' location='%s'\n",
	       STR (lo->get_name ()), STR (lo->dbeFile->get_location (false)));
    }
  free (class_name);
  return lo;
}

Module *
Experiment::get_jclass (const char *className, const char *fileName)
{
  LoadObject *lo = get_j_lo (className, NULL);
  char *mod_name = get_jmodule_name (className);
  Module *mod = lo->find_module (mod_name);
  if (mod == NULL)
    {
      mod = dbeSession->createClassFile (mod_name);
      mod->loadobject = lo;
      if (strcmp (fileName, NTXT ("<Unknown>")) != 0)
	mod->set_file_name (gen_file_name (lo->get_pathname (), fileName));
      else
	mod->set_file_name (dbe_strdup (fileName));
      lo->append_module (mod);
      mod_name = NULL;
    }
  else if (mod->file_name && (strcmp (mod->file_name, "<Unknown>") == 0)
	   && strcmp (fileName, "<Unknown>") != 0)
    mod->set_file_name (gen_file_name (lo->get_pathname (), fileName));
  Dprintf (DUMP_JCLASS_READER,
	"Experiment::get_jclass: class_name='%s' mod_name='%s' fileName='%s'\n",
	   mod->loadobject->get_pathname (), mod->get_name (), mod->file_name);
  free (mod_name);
  return mod;
}

#define ARCH_STRLEN(s) ( ( strlen(s) + 4 ) & ~0x3 )

int
Experiment::read_java_classes_file ()
{
  char *data_file_name = dbe_sprintf (NTXT ("%s/%s"), expt_name, SP_JCLASSES_FILE);
  Data_window *dwin = new Data_window (data_file_name);
  free (data_file_name);
  if (dwin->not_opened ())
    {
      delete dwin;
      return INCOMPLETE;
    }
  dwin->need_swap_endian = need_swap_endian;
  jmaps = new PRBTree ();
  jmidHTable = new DbeCacheMap<unsigned long long, JMethod>;

  hrtime_t cur_loaded = 0;
  Module *cur_mod = NULL;
  for (int64_t offset = 0;;)
    {
      CM_Packet *cpkt = (CM_Packet*) dwin->bind (offset, sizeof (CM_Packet));
      if (cpkt == NULL)
	break;
      uint16_t v16 = (uint16_t) cpkt->tsize;
      size_t cpktsize = dwin->decode (v16);
      cpkt = (CM_Packet*) dwin->bind (offset, cpktsize);
      if ((cpkt == NULL) || (cpktsize == 0))
	{
	  char *buf = dbe_sprintf (GTXT ("archive file malformed %s"),
				   arch_name);
	  errorq->append (new Emsg (CMSG_ERROR, buf));
	  free (buf);
	  break;
	}
      v16 = (uint16_t) cpkt->type;
      v16 = dwin->decode (v16);
      switch (v16)
	{
	case ARCH_JCLASS:
	  {
	    ARCH_jclass *ajcl = (ARCH_jclass*) cpkt;
	    uint64_t class_id = dwin->decode (ajcl->class_id);
	    char *className = ((char*) ajcl) + sizeof (*ajcl);
	    char *fileName = className + ARCH_STRLEN (className);
	    Dprintf (DUMP_JCLASS_READER,
		     "read_java_classes_file: ARCH_JCLASS(Ox%x)"
		     "class_id=Ox%llx className='%s' fileName='%s' \n",
		     (int) v16, (long long) class_id, className, fileName);
	    cur_mod = NULL;
	    if (*className == 'L')
	      { // Old libcollector generated '[' (one array dimension).
		cur_mod = get_jclass (className, fileName);
		cur_loaded = dwin->decode (ajcl->tstamp);
		jmaps->insert (class_id, cur_loaded, cur_mod);
	      }
	    break;
	  }
	case ARCH_JCLASS_LOCATION:
	  {
	    ARCH_jclass_location *ajcl = (ARCH_jclass_location *) cpkt;
	    uint64_t class_id = dwin->decode (ajcl->class_id);
	    char *className = ((char*) ajcl) + sizeof (*ajcl);
	    char *fileName = className + ARCH_STRLEN (className);
	    Dprintf (DUMP_JCLASS_READER,
		     "read_java_classes_file: ARCH_JCLASS_LOCATION(Ox%x)"
		     "class_id=Ox%llx className='%s' fileName='%s' \n",
		     (int) v16, (long long) class_id, className, fileName);
	    get_j_lo (className, fileName);
	    break;
	  }
	case ARCH_JMETHOD:
	  {
	    if (cur_mod == NULL)
	      break;
	    ARCH_jmethod *ajmt = (ARCH_jmethod*) cpkt;
	    uint64_t method_id = dwin->decode (ajmt->method_id);
	    char *s_name = ((char*) ajmt) + sizeof (*ajmt);
	    char *s_signature = s_name + ARCH_STRLEN (s_name);
	    char *fullname = dbe_sprintf ("%s.%s", cur_mod->get_name (), s_name);
	    Dprintf (DUMP_JCLASS_READER,
		     "read_java_classes_file: ARCH_JMETHOD(Ox%x) "
		     "method_id=Ox%llx name='%s' signature='%s' fullname='%s'\n",
		     (int) v16, (long long) method_id, s_name,
		     s_signature, fullname);
	    JMethod *jmthd = cur_mod->find_jmethod (fullname, s_signature);
	    if (jmthd == NULL)
	      {
		jmthd = dbeSession->createJMethod ();
		jmthd->size = (unsigned) - 1; // unknown until later (maybe)
		jmthd->module = cur_mod;
		jmthd->set_signature (s_signature);
		jmthd->set_name (fullname);
		cur_mod->functions->append (jmthd);
		cur_mod->loadobject->functions->append (jmthd);
		Dprintf (DUMP_JCLASS_READER,
		    "read_java_classes_file: ARCH_JMETHOD CREATE fullname=%s\n",
			 fullname);
	      }
	    jmaps->insert (method_id, cur_loaded, jmthd);
	    free (fullname);
	    break;
	  }
	default:
	  Dprintf (DUMP_JCLASS_READER,
		   "read_java_classes_file: type=Ox%x (%d) cpktsize=%d\n",
		   (int) v16, (int) v16, (int) cpktsize);
	  break; // ignore unknown packets
	}
      offset += cpktsize;
    }
  delete dwin;
  return SUCCESS;
}

void
Experiment::read_map_file ()
{
  ExperimentFile *mapFile = new ExperimentFile (this, SP_MAP_FILE);
  if (!mapFile->open ())
    {
      delete mapFile;
      return;
    }

  SAXParserFactory *factory = SAXParserFactory::newInstance ();
  SAXParser *saxParser = factory->newSAXParser ();
  DefaultHandler *dh = new ExperimentHandler (this);
  try
    {
      saxParser->parse ((File*) mapFile->fh, dh);
    }
  catch (SAXException *e)
    {
      // Fatal error in the parser
      StringBuilder sb;
      sb.sprintf (NTXT ("%s: %s"), SP_MAP_FILE, e->getMessage ());
      char *str = sb.toString ();
      Emsg *m = new Emsg (CMSG_FATAL, str);
      errorq->append (m);
      status = FAILURE;
      free (str);
      delete e;
    }
  delete mapFile;
  delete dh;
  delete saxParser;
  delete factory;

  for (int i = 0, sz = mrecs ? mrecs->size () : 0; i < sz; i++)
    {
      MapRecord *mrec = mrecs->fetch (i);
      SegMem *smem, *sm_lo, *sm_hi;
      switch (mrec->kind)
	{
	case MapRecord::LOAD:
	  smem = new SegMem;
	  smem->base = mrec->base;
	  smem->size = mrec->size;
	  smem->load_time = mrec->ts;
	  smem->unload_time = MAX_TIME;
	  smem->obj = mrec->obj;
	  smem->set_file_offset (mrec->foff);
	  seg_items->append (smem); // add to the master list

	  // Check if the new segment overlaps other active segments
	  sm_lo = (SegMem*) maps->locate (smem->base, smem->load_time);
	  if (sm_lo && sm_lo->base + sm_lo->size > smem->base)
	    {
	      // check to see if it is a duplicate record: same address and size, and
	      if ((smem->base == sm_lo->base) && (smem->size == sm_lo->size))
		{
		  // addresses and sizes match, check name
		  if (strstr (smem->obj->get_name (), sm_lo->obj->get_name ()) != NULL
		      || strstr (sm_lo->obj->get_name (), smem->obj->get_name ()) != NULL)
		    // this is a duplicate; just move on the the next map record
		    continue;
		  fprintf (stderr,
			   GTXT ("*** Warning: Segment `%s' loaded with same address, size as `%s' [0x%llx-0x%llx]\n"),
			   smem->obj->get_name (), sm_lo->obj->get_name (),
			   sm_lo->base, sm_lo->base + sm_lo->size);
		}

	      // Not a duplicate; implicitly unload the old one
	      //     Note: implicit unloading causes high <Unknown>
	      //           when such overlapping is bogus
	      StringBuilder sb;
	      sb.sprintf (GTXT ("*** Warning: Segment %s [0x%llx-0x%llx] overlaps %s [0x%llx-0x%llx], which has been implicitly unloaded"),
			  smem->obj->get_name (), smem->base, smem->base + smem->size,
			  sm_lo->obj->get_name (), sm_lo->base, sm_lo->base + sm_lo->size);
	      warnq->append (new Emsg (CMSG_WARN, sb));
	    }

	  // now look for other segments with which this might overlap
	  sm_hi = (SegMem*) maps->locate_up (smem->base, smem->load_time);
	  while (sm_hi && sm_hi->base < smem->base + smem->size)
	    {

	      // Note: implicit unloading causes high <Unknown> when such overlapping is bogus
	      // maps->remove( sm_hi->base, smem->load_time );
	      StringBuilder sb;
	      sb.sprintf (GTXT ("*** Warning: Segment %s [0x%llx-0x%llx] overlaps %s [0x%llx-0x%llx], which has been implicitly unloaded"),
			  smem->obj->get_name (), smem->base,
			  smem->base + smem->size, sm_hi->obj->get_name (),
			  sm_hi->base, sm_hi->base + sm_hi->size);
	      warnq->append (new Emsg (CMSG_WARN, sb));
	      sm_hi = (SegMem*) maps->locate_up (sm_hi->base + sm_hi->size,
						 smem->load_time);
	    }

	  maps->insert (smem->base, smem->load_time, smem);
	  break;
	case MapRecord::UNLOAD:
	  smem = (SegMem*) maps->locate (mrec->base, mrec->ts);
	  if (smem && smem->base == mrec->base)
	    {
	      smem->unload_time = mrec->ts;
	      maps->remove (mrec->base, mrec->ts);
	    }
	  break;
	}
    }
  mrecs->destroy ();

  // See if there are comments or warnings for a load object;
  // if so, queue them to Experiment
  for (long i = 0, sz = loadObjs ? loadObjs->size () : 0; i < sz; i++)
    {
      LoadObject *lo = loadObjs->get (i);
      for (Emsg *m = lo->fetch_warnings (); m; m = m->next)
	warnq->append (m->get_warn (), m->get_msg ());
      for (Emsg *m = lo->fetch_comments (); m; m = m->next)
	commentq->append (m->get_warn (), m->get_msg ());
    }
}

void
Experiment::read_frameinfo_file ()
{
  init_cache ();
  char *base_name = get_basename (expt_name);
  char *msg = dbe_sprintf (GTXT ("Loading CallStack Data: %s"), base_name);
  read_data_file ("data." SP_FRINFO_FILE, msg);
  free (msg);
  frmpckts->sort (frUidCmp);
  uidnodes->sort (uidNodeCmp);
}

void
Experiment::read_omp_preg ()
{
  // Parallel region descriptions
  DataDescriptor *pregDdscr = getDataDescriptor (DATA_OMP4);
  if (pregDdscr == NULL)
    return;
  DataView *pregData = pregDdscr->createView ();
  pregData->sort (PROP_CPRID); // omptrace PROP_CPRID

  // OpenMP enter parreg events
  DataDescriptor *dDscr = getDataDescriptor (DATA_OMP2);
  if (dDscr == NULL || dDscr->getSize () == 0)
    {
      delete pregData;
      return;
    }

  char *idxname = NTXT ("OMP_preg");
  delete dbeSession->indxobj_define (idxname, GTXT ("OpenMP Parallel Region"),
				     NTXT ("CPRID"), NULL, NULL);
  int idxtype = dbeSession->findIndexSpaceByName (idxname);
  if (idxtype < 0)
    {
      delete pregData;
      return;
    }
  ompavail = true;

  // Pre-create parallel region with id == 0
  Histable *preg0 = dbeSession->createIndexObject (idxtype, (int64_t) 0);
  preg0->set_name (dbe_strdup (GTXT ("Implicit OpenMP Parallel Region")));

  // Take care of the progress bar
  char *msg = dbe_sprintf (GTXT ("Processing OpenMP Parallel Region Data: %s"),
  get_basename (expt_name));
  theApplication->set_progress (0, msg);
  free (msg);
  long deltaReport = 1000;
  long nextReport = 0;
  long errors_found = 0;
  Vector<Histable*> pregs;

  long size = dDscr->getSize ();
  for (long i = 0; i < size; ++i)
    {
      if (i == nextReport)
	{
	  int percent = (int) (i * 100 / size);
	  if (percent > 0)
	    theApplication->set_progress (percent, NULL);
	  nextReport += deltaReport;
	}

      uint32_t thrid = dDscr->getIntValue (PROP_THRID, i);
      hrtime_t tstamp = dDscr->getLongValue (PROP_TSTAMP, i);
      uint64_t cprid = dDscr->getLongValue (PROP_CPRID, i); // omptrace CPRID
      mapPRid->put (thrid, tstamp, cprid);

      pregs.reset ();
      /*
       * We will use 2 pointers to make sure there is no loop.
       * First pointer "curpreg" goes to the next element,
       * second pointer "curpreg_loop_control" goes to the next->next element.
       * If these pointers have the same value - there is a loop.
       */
      uint64_t curpreg_loop_control = cprid;
      Datum tval_loop_control;
      if (curpreg_loop_control != 0)
	{
	  tval_loop_control.setUINT64 (curpreg_loop_control);
	  long idx = pregData->getIdxByVals (&tval_loop_control, DataView::REL_EQ);
	  if (idx < 0)
	    curpreg_loop_control = 0;
	  else
	    curpreg_loop_control = pregData->getLongValue (PROP_PPRID, idx);
	}
      for (uint64_t curpreg = cprid; curpreg != 0;)
	{
	  Histable *val = NULL;
	  Datum tval;
	  tval.setUINT64 (curpreg);
	  long idx = pregData->getIdxByVals (&tval, DataView::REL_EQ);
	  if (idx < 0)
	    break;
	  /*
	   * Check if there is a loop
	   */
	  if (0 != curpreg_loop_control)
	    {
	      if (curpreg == curpreg_loop_control)
		{
		  errors_found++;
		  if (1 == errors_found)
		    {
		      Emsg *m = new Emsg (CMSG_WARN, GTXT ("*** Warning: circular links in OMP regions; data may not be correct."));
		      warnq->append (m);
		    }
		  break;
		}
	    }
	  uint64_t pragmapc = pregData->getLongValue (PROP_PRPC, idx);
	  DbeInstr *instr = map_Vaddr_to_PC (pragmapc, tstamp);
	  if (instr == NULL)
	    {
	      break;
	    }
	  val = instr;
	  DbeLine *dbeline = (DbeLine*) instr->convertto (Histable::LINE);
	  if (dbeline->lineno > 0)
	    {
	      if (instr->func->usrfunc)
		dbeline = dbeline->sourceFile->find_dbeline
			(instr->func->usrfunc, dbeline->lineno);
	      dbeline->set_flag (DbeLine::OMPPRAGMA);
	      val = dbeline;
	    }
	  val = dbeSession->createIndexObject (idxtype, val);
	  pregs.append (val);

	  curpreg = pregData->getLongValue (PROP_PPRID, idx);
	  /*
	   * Update curpreg_loop_control
	   */
	  if (0 != curpreg_loop_control)
	    {
	      tval_loop_control.setUINT64 (curpreg_loop_control);
	      idx = pregData->getIdxByVals
		      (&tval_loop_control, DataView::REL_EQ);
	      if (idx < 0)
		curpreg_loop_control = 0;
	      else
		{
		  curpreg_loop_control = pregData->getLongValue
			  (PROP_PPRID, idx);
		  tval_loop_control.setUINT64 (curpreg_loop_control);
		  idx = pregData->getIdxByVals
			  (&tval_loop_control, DataView::REL_EQ);
		  if (idx < 0)
		    curpreg_loop_control = 0;
		  else
		    curpreg_loop_control = pregData->getLongValue
			    (PROP_PPRID, idx);
		}
	    }
	}
      pregs.append (preg0);
      void *prstack = cstack->add_stack (&pregs);
      mapPReg->put (thrid, tstamp, prstack);
    }
  theApplication->set_progress (0, NTXT (""));
  delete pregData;
}

void
Experiment::read_omp_task ()
{
  // Task description
  DataDescriptor *taskDataDdscr = getDataDescriptor (DATA_OMP5);
  if (taskDataDdscr == NULL)
    return;

  //7035272: previously, DataView was global; now it's local...is this OK?
  DataView *taskData = taskDataDdscr->createView ();
  taskData->sort (PROP_TSKID); // omptrace PROP_TSKID

  // OpenMP enter task events
  DataDescriptor *dDscr = getDataDescriptor (DATA_OMP3);
  if (dDscr == NULL || dDscr->getSize () == 0)
    {
      delete taskData;
      return;
    }

  char *idxname = NTXT ("OMP_task");
  // delete a possible error message. Ugly.
  delete dbeSession->indxobj_define (idxname, GTXT ("OpenMP Task"), NTXT ("TSKID"), NULL, NULL);
  int idxtype = dbeSession->findIndexSpaceByName (idxname);
  if (idxtype < 0)
    {
      delete taskData;
      return;
    }
  ompavail = true;

  // Pre-create task with id == 0
  Histable *task0 = dbeSession->createIndexObject (idxtype, (int64_t) 0);
  task0->set_name (dbe_strdup (GTXT ("OpenMP Task from Implicit Parallel Region")));

  // Take care of the progress bar
  char *msg = dbe_sprintf (GTXT ("Processing OpenMP Task Data: %s"), get_basename (expt_name));
  theApplication->set_progress (0, msg);
  free (msg);
  long deltaReport = 1000;
  long nextReport = 0;

  Vector<Histable*> tasks;
  long size = dDscr->getSize ();
  long errors_found = 0;
  for (long i = 0; i < size; ++i)
    {
      if (i == nextReport)
	{
	  int percent = (int) (i * 100 / size);
	  if (percent > 0)
	    theApplication->set_progress (percent, NULL);
	  nextReport += deltaReport;
	}

      uint32_t thrid = dDscr->getIntValue (PROP_THRID, i);
      hrtime_t tstamp = dDscr->getLongValue (PROP_TSTAMP, i);
      uint64_t tskid = dDscr->getLongValue (PROP_TSKID, i); //omptrace TSKID
      tasks.reset ();
      /*
       * We will use 2 pointers to make sure there is no loop.
       * First pointer "curtsk" goes to the next element,
       * second pointer "curtsk_loop_control" goes to the next->next element.
       * If these pointers have the same value - there is a loop.
       */
      uint64_t curtsk_loop_control = tskid;
      Datum tval_loop_control;
      if (curtsk_loop_control != 0)
	{
	  tval_loop_control.setUINT64 (curtsk_loop_control);
	  long idx = taskData->getIdxByVals (&tval_loop_control, DataView::REL_EQ);
	  if (idx < 0)
	    curtsk_loop_control = 0;
	  else
	    curtsk_loop_control = taskData->getLongValue (PROP_PTSKID, idx);
	}
      for (uint64_t curtsk = tskid; curtsk != 0;)
	{
	  Histable *val = NULL;

	  Datum tval;
	  tval.setUINT64 (curtsk);
	  long idx = taskData->getIdxByVals (&tval, DataView::REL_EQ);
	  if (idx < 0)
	    break;
	  /*
	   * Check if there is a loop
	   */
	  if (0 != curtsk_loop_control)
	    {
	      if (curtsk == curtsk_loop_control)
		{
		  errors_found++;
		  if (1 == errors_found)
		    {
		      Emsg *m = new Emsg (CMSG_WARN, GTXT ("*** Warning: circular links in OMP tasks; data may not be correct."));
		      warnq->append (m);
		    }
		  break;
		}
	    }
	  uint64_t pragmapc = taskData->getLongValue (PROP_PRPC, idx);
	  DbeInstr *instr = map_Vaddr_to_PC (pragmapc, tstamp);
	  if (instr == NULL)
	    break;
	  val = instr;
	  DbeLine *dbeline = (DbeLine*) instr->convertto (Histable::LINE);
	  if (dbeline->lineno > 0)
	    {
	      if (instr->func->usrfunc)
		dbeline = dbeline->sourceFile->find_dbeline
			(instr->func->usrfunc, dbeline->lineno);
	      dbeline->set_flag (DbeLine::OMPPRAGMA);
	      val = dbeline;
	    }
	  val = dbeSession->createIndexObject (idxtype, val);
	  tasks.append (val);

	  curtsk = taskData->getLongValue (PROP_PTSKID, idx);
	  /*
	   * Update curtsk_loop_control
	   */
	  if (0 != curtsk_loop_control)
	    {
	      tval_loop_control.setUINT64 (curtsk_loop_control);
	      idx = taskData->getIdxByVals (&tval_loop_control, DataView::REL_EQ);
	      if (idx < 0)
		curtsk_loop_control = 0;
	      else
		{
		  curtsk_loop_control = taskData->getLongValue (PROP_PTSKID, idx);
		  tval_loop_control.setUINT64 (curtsk_loop_control);
		  idx = taskData->getIdxByVals (&tval_loop_control,
						DataView::REL_EQ);
		  if (idx < 0)
		    curtsk_loop_control = 0;
		  else
		    curtsk_loop_control = taskData->getLongValue (PROP_PTSKID,
								  idx);
		}
	    }
	}
      tasks.append (task0);
      void *tskstack = cstack->add_stack (&tasks);
      mapTask->put (thrid, tstamp, tskstack);
    }
  theApplication->set_progress (0, NTXT (""));
  delete taskData;
}

void
Experiment::read_omp_file ()
{
  // DATA_OMP2 table is common between OpenMP 2.5 and 3.0 profiling
  DataDescriptor *dDscr = getDataDescriptor (DATA_OMP2);
  if (dDscr == NULL)
    return;
  if (dDscr->getSize () == 0)
    {
      char *base_name = get_basename (expt_name);
      char *msg = dbe_sprintf (GTXT ("Loading OpenMP Data: %s"), base_name);
      read_data_file (SP_OMPTRACE_FILE, msg);
      free (msg);

      // OpenMP fork events
      dDscr = getDataDescriptor (DATA_OMP);
      long sz = dDscr->getSize ();
      if (sz > 0)
	{
	  // progress bar
	  msg = dbe_sprintf (GTXT ("Processing OpenMP Parallel Region Data: %s"),
			     base_name);
	  theApplication->set_progress (0, msg);
	  free (msg);
	  long deltaReport = 5000;
	  long nextReport = 0;
	  for (int i = 0; i < sz; ++i)
	    {
	      if (i == nextReport)
		{
		  int percent = (int) (i * 100 / sz);
		  if (percent > 0)
		    theApplication->set_progress (percent, NULL);
		  nextReport += deltaReport;
		}
	      uint32_t thrid = dDscr->getIntValue (PROP_THRID, i);
	      hrtime_t tstamp = dDscr->getLongValue (PROP_TSTAMP, i);
	      uint64_t cprid = dDscr->getLongValue (PROP_CPRID, i); //omptrace
	      mapPRid->put (thrid, tstamp, cprid);
	    }
	  theApplication->set_progress (0, NTXT (""));

	  ompavail = true;
	  openMPdata = dDscr->createView ();
	  openMPdata->sort (PROP_CPRID); // omptrace PROP_CPRID

	  // thread enters parreg events
	  dDscr = getDataDescriptor (DATA_OMP2);
	  sz = dDscr->getSize ();

	  // progress bar
	  msg = dbe_sprintf (GTXT ("Processing OpenMP Parallel Region Data: %s"),
			     base_name);
	  theApplication->set_progress (0, msg);
	  free (msg);
	  deltaReport = 5000;
	  nextReport = 0;

	  for (int i = 0; i < sz; ++i)
	    {
	      if (i == nextReport)
		{
		  int percent = (int) (i * 100 / sz);
		  if (percent > 0)
		    theApplication->set_progress (percent, NULL);
		  nextReport += deltaReport;
		}
	      uint32_t thrid = dDscr->getIntValue (PROP_THRID, i);
	      hrtime_t tstamp = dDscr->getLongValue (PROP_TSTAMP, i);
	      uint64_t cprid = dDscr->getLongValue (PROP_CPRID, i); //omptrace
	      mapPRid->put (thrid, tstamp, cprid);
	    }
	  theApplication->set_progress (0, NTXT (""));
	}
      else
	{
	  read_omp_preg ();
	  read_omp_task ();
	}
      if (ompavail && coll_params.profile_mode)
	{
	  dbeSession->status_ompavail = 1;
	  register_metric (Metric::OMP_WORK);
	  register_metric (Metric::OMP_WAIT);
	  register_metric (Metric::OMP_OVHD);
	  if (coll_params.lms_magic_id == LMS_MAGIC_ID_SOLARIS)
	    register_metric (Metric::OMP_MASTER_THREAD);
	}
    }
}

void
Experiment::read_ifreq_file ()
{
  char *fname = dbe_sprintf (NTXT ("%s/%s"), expt_name, SP_IFREQ_FILE);
  FILE *f = fopen (fname, NTXT ("r"));
  free (fname);
  if (f == NULL)
    {
      ifreqavail = false;
      return;
    }
  ifreqavail = true;
  ifreqq = new Emsgqueue (NTXT ("ifreqq"));

  while (1)
    {
      Emsg *m;
      char str[MAXPATHLEN];
      char *e = fgets (str, ((int) sizeof (str)) - 1, f);
      if (e == NULL)
	{
	  // end the list from the experiment
	  m = new Emsg (CMSG_COMMENT,
			GTXT ("============================================================"));
	  ifreqq->append (m);
	  break;
	}
      // get the string
      size_t i = strlen (str);
      if (i > 0 && str[i - 1] == '\n')
	// remove trailing nl
	str[i - 1] = 0;
      // and append it
      m = new Emsg (CMSG_COMMENT, str);
      ifreqq->append (m);
    }
  (void) fclose (f);
}

Experiment *
Experiment::getBaseFounder ()
{
  if (baseFounder)
    return baseFounder;
  Experiment *founder = this;
  Experiment *parent = founder->founder_exp;
  while (parent)
    {
      founder = parent;
      parent = founder->founder_exp;
    }
  baseFounder = founder;
  return baseFounder;
}

hrtime_t
Experiment::getRelativeStartTime ()
{
  if (exp_rel_start_time_set)
    return exp_rel_start_time;
  Experiment *founder = getBaseFounder ();
  hrtime_t child_start = this->getStartTime ();
  hrtime_t founder_start = founder->getStartTime ();
  exp_rel_start_time = child_start - founder_start;
  if (child_start == 0 && founder_start)
    exp_rel_start_time = 0;     // when descendents have incomplete log.xml
  exp_rel_start_time_set = true;
  return exp_rel_start_time;
}

DataDescriptor *
Experiment::get_raw_events (int data_id)
{
  DataDescriptor *dDscr;
  switch (data_id)
    {
    case DATA_CLOCK:
      dDscr = get_profile_events ();
      break;
    case DATA_SYNCH:
      dDscr = get_sync_events ();
      break;
    case DATA_HWC:
      dDscr = get_hwc_events ();
      break;
    case DATA_HEAP:
      dDscr = get_heap_events ();
      break;
    case DATA_HEAPSZ:
      dDscr = get_heapsz_events ();
      break;
    case DATA_IOTRACE:
      dDscr = get_iotrace_events ();
      break;
    case DATA_RACE:
      dDscr = get_race_events ();
      break;
    case DATA_DLCK:
      dDscr = get_deadlock_events ();
      break;
    case DATA_SAMPLE:
      dDscr = get_sample_events ();
      break;
    case DATA_GCEVENT:
      dDscr = get_gc_events ();
      break;
    default:
      dDscr = NULL;
      break;
    }
  return dDscr;
}

int
Experiment::base_data_id (int data_id)
{
  switch (data_id)
    {
    case DATA_HEAPSZ:
      return DATA_HEAP; // DATA_HEAPSZ DataView is based on DATA_HEAP's DataView
    default:
      break;
    }
  return data_id;
}

DataView *
Experiment::create_derived_data_view (int data_id, DataView *dview)
{
  // dview contains filtered packets
  switch (data_id)
    {
    case DATA_HEAPSZ:
      return create_heapsz_data_view (dview);
    default:
      break;
    }
  return NULL;
}

DataDescriptor *
Experiment::get_profile_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_CLOCK);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () == 0)
    {
      char *base_name = get_basename (expt_name);
      char *msg = dbe_sprintf (GTXT ("Loading Profile Data: %s"), base_name);
      read_data_file (SP_PROFILE_FILE, msg);
      free (msg);
      add_evt_time_to_profile_events (dDscr);
      resolve_frame_info (dDscr);
    }
  else if (!dDscr->isResolveFrInfoDone ())
    resolve_frame_info (dDscr);
  return dDscr;
}

void
Experiment::add_evt_time_to_profile_events (DataDescriptor *dDscr)
{
  if (coll_params.lms_magic_id != LMS_MAGIC_ID_SOLARIS)
    return;

  DataView *dview = dDscr->createView ();
  dview->sort (PROP_THRID, PROP_TSTAMP);

  // add PROP_EVT_TIME
  PropDescr* tmp_propDscr = new PropDescr (PROP_EVT_TIME, "EVT_TIME");
  tmp_propDscr->uname = dbe_strdup (GTXT ("Event duration"));
  tmp_propDscr->vtype = TYPE_INT64;
  dDscr->addProperty (tmp_propDscr);

  long sz = dview->getSize ();
  long long ptimer_usec = get_params ()->ptimer_usec;
  for (long i = 0; i < sz; i++)
    {
      int next_sample;
      int jj;
      {
	hrtime_t this_tstamp = dview->getLongValue (PROP_TSTAMP, i);
	long this_thrid = dview->getLongValue (PROP_THRID, i);
	for (jj = i + 1; jj < sz; jj++)
	  {
	    hrtime_t tmp_tstamp = dview->getLongValue (PROP_TSTAMP, jj);
	    if (tmp_tstamp != this_tstamp)
	      break;
	    long tmp_thrid = dview->getLongValue (PROP_THRID, jj);
	    if (tmp_thrid != this_thrid)
	      break;
	  }
	next_sample = jj;
      }

      long nticks = 0;
      for (jj = i; jj < next_sample; jj++)
	nticks += dview->getLongValue (PROP_NTICK, jj);
      if (nticks <= 1)
	continue; // no duration

      nticks--;
      hrtime_t duration = ptimer_usec * 1000LL * nticks; // nanoseconds
      for (jj = i; jj < next_sample; jj++)
	dview->setValue (PROP_EVT_TIME, jj, duration);
      i = jj - 1;
    }
  delete dview;
}

DataDescriptor *
Experiment::get_sync_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_SYNCH);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () > 0)
    return dDscr;

  // fetch data
  {
    char *base_name = get_basename (expt_name);
    char *msg = dbe_sprintf (GTXT ("Loading Synctrace Data: %s"), base_name);
    read_data_file (SP_SYNCTRACE_FILE, msg);
    free (msg);
    resolve_frame_info (dDscr);
  }

  // check for PROP_EVT_TIME
  PropDescr *tmp_propDscr = dDscr->getProp (PROP_EVT_TIME);
  if (tmp_propDscr)
    return dDscr;

  // add PROP_EVT_TIME
  tmp_propDscr = new PropDescr (PROP_EVT_TIME, "EVT_TIME");
  tmp_propDscr->uname = dbe_strdup (GTXT ("Event duration"));
  tmp_propDscr->vtype = TYPE_INT64;
  dDscr->addProperty (tmp_propDscr);

  long sz = dDscr->getSize ();
  for (long i = 0; i < sz; i++)
    {
      uint64_t event_duration = dDscr->getLongValue (PROP_TSTAMP, i);
      event_duration -= dDscr->getLongValue (PROP_SRQST, i);
      dDscr->setValue (PROP_EVT_TIME, i, event_duration);
    }
  return dDscr;
}

DataDescriptor *
Experiment::get_hwc_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_HWC);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () == 0)
    {
      char *base_name = get_basename (expt_name);
      char *msg = dbe_sprintf (GTXT ("Loading HW Profile Data: %s"), base_name);

      // clear HWC event stats
      dsevents = 0;
      dsnoxhwcevents = 0;
      read_data_file (SP_HWCNTR_FILE, msg);
      free (msg);
      resolve_frame_info (dDscr);

      // describe the HW counters in PropDescr
      PropDescr *prop = dDscr->getProp (PROP_HWCTAG);
      if (prop)
	{
	  Collection_params *cparam = get_params ();
	  if (cparam->hw_mode != 0)
	    for (int aux = 0; aux < MAX_HWCOUNT; aux++)
	      if (cparam->hw_aux_name[aux])
		{
		  const char* cmdname = cparam->hw_aux_name[aux];
		  const char* uname = cparam->hw_username[aux];
		  prop->addState (aux, cmdname, uname);
		}
	}
      else
	assert (0);

      double dserrrate = 100.0 * ((double) dsnoxhwcevents) / ((double) dsevents);
      if ((dsevents > 0) && (dserrrate > 10.0))
	{
	  // warn the user that rate is high
	  StringBuilder sb;
	  if (dbeSession->check_ignore_no_xhwcprof ())
	    sb.sprintf (
			GTXT ("Warning: experiment %s has %.1f%%%% (%lld of %lld) dataspace events that were accepted\n  without verification; data may be incorrect or misleading\n  recompile with -xhwcprof and rerecord to get better data\n"),
			base_name, dserrrate, (long long) dsnoxhwcevents,
			(long long) dsevents);
	  else
	    sb.sprintf (
			GTXT ("Warning: experiment %s has %.1f%%%% (%lld of %lld) dataspace events that could not be verified\n  recompile with -xhwcprof and rerecord to get better data\n"),
			base_name, dserrrate, (long long) dsnoxhwcevents,
			(long long) dsevents);
	  errorq->append (new Emsg (CMSG_WARN, sb));
	}

      // see if we've scanned the data
      if (hwc_scanned == 0)
	{
	  // no, scan the packets to see how many are bogus, or represent lost interrupts
	  long hwc_cnt = 0;

	  // loop over the packets, counting the bad ones
	  if (hwc_bogus != 0 || hwc_lost_int != 0)
	    {
	      // hwc counter data had bogus packets and/or packets reflecting lost interrupts
	      double bogus_rate = 100. * (double) hwc_bogus / (double) hwc_cnt;
	      if (bogus_rate > 5.)
		{
		  StringBuilder sb;
		  sb.sprintf (
			      GTXT ("WARNING: Too many invalid HW counter profile events (%ld/%ld = %3.2f%%) in experiment %d (`%s'); data may be unreliable"),
			      (long) hwc_bogus, (long) hwc_cnt, bogus_rate,
			      (int) userExpId, base_name);
		  Emsg *m = new Emsg (CMSG_WARN, sb);
		  warnq->append (m);
		}
	      hwc_scanned = 1;
	    }
	}
    }
  return dDscr;
}

DataDescriptor *
Experiment::get_iotrace_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_IOTRACE);
  if (dDscr == NULL)
    return NULL;

  if (dDscr->getSize () > 0)
    return dDscr;

  char *base_name = get_basename (expt_name);
  char *msg = dbe_sprintf (GTXT ("Loading IO Trace Data: %s"), base_name);
  read_data_file (SP_IOTRACE_FILE, msg);
  free (msg);

  if (dDscr->getSize () == 0)
    return dDscr;
  resolve_frame_info (dDscr);

  // check for PROP_EVT_TIME
  PropDescr *tmp_propDscr = dDscr->getProp (PROP_EVT_TIME);
  if (tmp_propDscr)
    return dDscr;

  // add PROP_EVT_TIME
  tmp_propDscr = new PropDescr (PROP_EVT_TIME, "EVT_TIME");
  tmp_propDscr->uname = dbe_strdup (GTXT ("Event duration"));
  tmp_propDscr->vtype = TYPE_INT64;
  dDscr->addProperty (tmp_propDscr);

  // add PROP_IOVFD
  tmp_propDscr = new PropDescr (PROP_IOVFD, "IOVFD");
  tmp_propDscr->uname = dbe_strdup (GTXT ("Virtual File Descriptor"));
  tmp_propDscr->vtype = TYPE_INT64;
  dDscr->addProperty (tmp_propDscr);

  delete fDataMap;
  fDataMap = new DefaultMap<int64_t, FileData*>;

  delete vFdMap;
  vFdMap = new DefaultMap<int, int64_t>;

  static int64_t virtualFd = 0;

  FileData *fData;
  virtualFd += 10;
  fData = fDataMap->get (VIRTUAL_FD_STDIN);
  if (fData == NULL)
    {
      fData = new FileData (STDIN_FILENAME);
      fData->setVirtualFd (VIRTUAL_FD_STDIN);
      fData->id = VIRTUAL_FD_STDIN;
      fData->setFileDes (STDIN_FD);
      fDataMap->put (VIRTUAL_FD_STDIN, fData);
      vFdMap->put (STDIN_FD, VIRTUAL_FD_STDIN);
    }

  fData = fDataMap->get (VIRTUAL_FD_STDOUT);
  if (fData == NULL)
    {
      fData = new FileData (STDOUT_FILENAME);
      fData->setVirtualFd (VIRTUAL_FD_STDOUT);
      fData->id = VIRTUAL_FD_STDOUT;
      fData->setFileDes (STDOUT_FD);
      fDataMap->put (VIRTUAL_FD_STDOUT, fData);
      vFdMap->put (STDOUT_FD, VIRTUAL_FD_STDOUT);
    }

  fData = fDataMap->get (VIRTUAL_FD_STDERR);
  if (fData == NULL)
    {
      fData = new FileData (STDERR_FILENAME);
      fData->setVirtualFd (VIRTUAL_FD_STDERR);
      fData->id = VIRTUAL_FD_STDERR;
      fData->setFileDes (STDERR_FD);
      fDataMap->put (VIRTUAL_FD_STDERR, fData);
      vFdMap->put (STDERR_FD, VIRTUAL_FD_STDERR);
    }

  fData = fDataMap->get (VIRTUAL_FD_OTHERIO);
  if (fData == NULL)
    {
      fData = new FileData (OTHERIO_FILENAME);
      fData->setVirtualFd (VIRTUAL_FD_OTHERIO);
      fData->id = VIRTUAL_FD_OTHERIO;
      fData->setFileDes (OTHERIO_FD);
      fDataMap->put (VIRTUAL_FD_OTHERIO, fData);
    }

  DataView *dview = dDscr->createView ();
  dview->sort (PROP_TSTAMP);
  long sz = dview->getSize ();
  for (long i = 0; i < sz; i++)
    {
      hrtime_t event_duration = dview->getLongValue (PROP_TSTAMP, i);
      hrtime_t event_start = dview->getLongValue (PROP_IORQST, i);
      if (event_start > 0)
	event_duration -= event_start;
      else
	event_duration = 0;
      dview->setValue (PROP_EVT_TIME, i, event_duration);

      int32_t fd = -1;
      int64_t vFd = VIRTUAL_FD_NONE;
      char *fName = NULL;
      int32_t origFd = -1;
      StringBuilder *sb = NULL;
      FileData *fDataOrig = NULL;
      FileSystem_type fsType;

      IOTrace_type ioType = (IOTrace_type) dview->getIntValue (PROP_IOTYPE, i);
      switch (ioType)
	{
	case READ_TRACE:
	case WRITE_TRACE:
	case READ_TRACE_ERROR:
	case WRITE_TRACE_ERROR:
	  fd = dview->getIntValue (PROP_IOFD, i);
	  vFd = vFdMap->get (fd);
	  if (vFd == 0 || vFd == VIRTUAL_FD_NONE
	      || (fData = fDataMap->get (vFd)) == NULL)
	    {
	      fData = new FileData (UNKNOWNFD_FILENAME);
	      fData->setVirtualFd (virtualFd);
	      fData->setFsType ("N/A");
	      fData->setFileDes (fd);
	      fDataMap->put (virtualFd, fData);
	      vFdMap->put (fd, virtualFd);
	      vFd = virtualFd;
	      virtualFd++;
	    }
	  dview->setValue (PROP_IOVFD, i, vFd);
	  break;
	case OPEN_TRACE:
	  fName = NULL;
	  sb = (StringBuilder*) dview->getObjValue (PROP_IOFNAME, i);
	  if (sb != NULL && sb->length () > 0)
	    fName = sb->toString ();
	  fd = dview->getIntValue (PROP_IOFD, i);
	  origFd = dview->getIntValue (PROP_IOOFD, i);
	  fsType = (FileSystem_type) dview->getIntValue (PROP_IOFSTYPE, i);

	  if (fName != NULL)
	    {
	      fData = new FileData (fName);
	      fDataMap->put (virtualFd, fData);
	      vFdMap->put (fd, virtualFd);
	      fData->setFileDes (fd);
	      fData->setFsType (fsType);
	      fData->setVirtualFd (virtualFd);
	      vFd = virtualFd;
	      virtualFd++;
	    }
	  else if (origFd > 0)
	    {
	      vFd = vFdMap->get (origFd);
	      if (vFd == 0 || vFd == VIRTUAL_FD_NONE)
		{
		  Dprintf (DEBUG_IO,
			   "*** Error I/O tracing: (open) cannot get the virtual file descriptor, fd=%d  origFd=%d\n",
			   fd, origFd);
		  continue;
		}
	      else if ((fDataOrig = fDataMap->get (vFd)) == NULL)
		{
		  Dprintf (DEBUG_IO,
			   "*** Error IO tracing: (open) cannot get original FileData object, fd=%d  origFd=%d\n",
			   fd, origFd);
		  continue;
		}
	      else
		{
		  fName = fDataOrig->getFileName ();
		  fData = new FileData (fName);
		  fData->setFileDes (fd);
		  fData->setFsType (fDataOrig->getFsType ());
		  fData->setVirtualFd (virtualFd);
		  fDataMap->put (virtualFd, fData);
		  vFdMap->put (fd, virtualFd);
		  vFd = virtualFd;
		  virtualFd++;
		}
	    }
	  else if (fd >= 0)
	    {
	      vFd = vFdMap->get (fd);
	      if (vFd == 0 || vFd == VIRTUAL_FD_NONE
		  || (fData = fDataMap->get (vFd)) == NULL)
		{
		  fData = new FileData (UNKNOWNFD_FILENAME);
		  fData->setVirtualFd (virtualFd);
		  fData->setFsType ("N/A");
		  fData->setFileDes (fd);
		  fDataMap->put (virtualFd, fData);
		  vFdMap->put (fd, virtualFd);
		  vFd = virtualFd;
		  virtualFd++;
		}
	    }
	  else
	    {
	      Dprintf (DEBUG_IO,
		       NTXT ("*** Error IO tracing: (open) unknown open IO type, fd=%d  origFd=%d\n"), fd, origFd);
	      continue;
	    }

	  dview->setValue (PROP_IOVFD, i, vFd);
	  break;

	case OPEN_TRACE_ERROR:
	  fName = NULL;

	  sb = (StringBuilder*) dview->getObjValue (PROP_IOFNAME, i);
	  if (sb != NULL && sb->length () > 0)
	    fName = sb->toString ();
	  fd = dview->getIntValue (PROP_IOFD, i);
	  origFd = dview->getIntValue (PROP_IOOFD, i);
	  fsType = (FileSystem_type) dview->getIntValue (PROP_IOFSTYPE, i);

	  if (fName != NULL)
	    {
	      fData = new FileData (fName);
	      fDataMap->put (virtualFd, fData);
	      fData->setFileDes (fd);
	      fData->setFsType (fsType);
	      fData->setVirtualFd (virtualFd);
	      vFd = virtualFd;
	      virtualFd++;
	    }
	  else if (origFd > 0)
	    {
	      vFd = vFdMap->get (origFd);
	      if (vFd == 0 || vFd == VIRTUAL_FD_NONE)
		{
		  Dprintf (DEBUG_IO,
			   "*** Error IO tracing: (open error) cannot get the virtual file descriptor, fd=%d  origFd=%d\n",
			   fd, origFd);
		  continue;
		}
	      else if ((fDataOrig = fDataMap->get (vFd)) == NULL)
		{
		  Dprintf (DEBUG_IO,
			   "*** Error IO tracing: (open error) cannot get original FileData object, fd=%d  origFd=%d\n",
			   fd, origFd);
		  continue;
		}
	      else
		{
		  fName = fDataOrig->getFileName ();
		  fData = new FileData (fName);
		  fData->setFileDes (fd);
		  fData->setFsType (fDataOrig->getFsType ());
		  fData->setVirtualFd (virtualFd);
		  fDataMap->put (virtualFd, fData);
		  vFd = virtualFd;
		  virtualFd++;
		}
	    }

	  dview->setValue (PROP_IOVFD, i, vFd);
	  break;

	case CLOSE_TRACE:
	case CLOSE_TRACE_ERROR:
	  fd = dview->getIntValue (PROP_IOFD, i);
	  vFd = vFdMap->get (fd);
	  if (vFd == 0 || vFd == VIRTUAL_FD_NONE)
	    {
	      Dprintf (DEBUG_IO,
		       "*** Error IO tracing: (close) cannot get the virtual file descriptor, fd=%d\n",
		       fd);
	      continue;
	    }
	  fData = fDataMap->get (vFd);
	  if (fData == NULL)
	    {
	      Dprintf (DEBUG_IO,
		       "*** Error IO tracing: (close) cannot get the FileData object, fd=%d\n",
		       fd);
	      continue;
	    }

	  vFdMap->put (fd, VIRTUAL_FD_NONE);
	  dview->setValue (PROP_IOVFD, i, vFd);
	  break;

	case OTHERIO_TRACE:
	case OTHERIO_TRACE_ERROR:
	  vFd = VIRTUAL_FD_OTHERIO;
	  fData = fDataMap->get (vFd);
	  if (fData == NULL)
	    {
	      Dprintf (DEBUG_IO,
		       "*** Error IO tracing: (other IO) cannot get the FileData object\n");
	      continue;
	    }

	  dview->setValue (PROP_IOVFD, i, vFd);
	  break;
	case IOTRACETYPE_LAST:
	  break;
	}
    }

  delete dview;

  return dDscr;
}

DataDescriptor *
Experiment::get_heap_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_HEAP);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () > 0)
    return dDscr;

  char *base_name = get_basename (expt_name);
  char *msg = dbe_sprintf (GTXT ("Loading Heap Trace Data: %s"), base_name);
  read_data_file (SP_HEAPTRACE_FILE, msg);
  free (msg);

  if (dDscr->getSize () == 0)
    return dDscr;
  resolve_frame_info (dDscr);

  // Match FREE to MALLOC
  PropDescr *prop = new PropDescr (PROP_HLEAKED, NTXT ("HLEAKED"));
  prop->uname = dbe_strdup (GTXT ("Bytes Leaked"));
  prop->vtype = TYPE_UINT64;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_HMEM_USAGE, NTXT ("HMEM_USAGE"));
  prop->uname = dbe_strdup (GTXT ("Heap Memory Usage"));
  prop->vtype = TYPE_UINT64;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_HFREED, NTXT ("HFREED"));
  prop->uname = dbe_strdup (GTXT ("Bytes Freed"));
  prop->vtype = TYPE_UINT64;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_HCUR_ALLOCS, NTXT ("HCUR_ALLOCS"));
  prop->uname = dbe_strdup (GTXT ("Net Bytes Allocated"));
  prop->vtype = TYPE_INT64;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_HCUR_LEAKS, NTXT ("HCUR_LEAKS"));
  prop->uname = dbe_strdup (GTXT ("Net Bytes Leaked"));
  prop->vtype = TYPE_UINT64;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_HCUR_NET_ALLOC, NTXT ("HCUR_NET_ALLOC"));
  prop->vtype = TYPE_INT64;
  prop->flags = DDFLAG_NOSHOW;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_DDSCR_LNK, NTXT ("DDSCR_LNK"));
  prop->vtype = TYPE_UINT64;
  prop->flags = DDFLAG_NOSHOW;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_VOIDP_OBJ, NTXT ("VOIDP_OBJ"));
  prop->vtype = TYPE_OBJ;
  prop->flags = DDFLAG_NOSHOW;
  dDscr->addProperty (prop);

  prop = new PropDescr (PROP_TSTAMP2, NTXT ("TSTAMP2"));
  prop->uname = dbe_strdup (GTXT ("End Timestamp (nanoseconds)"));
  prop->vtype = TYPE_UINT64;
  prop->flags = DDFLAG_NOSHOW;
  dDscr->addProperty (prop);

  DataView *dview = dDscr->createView ();
  dview->sort (PROP_TSTAMP);

  // Keep track of memory usage
  Size memoryUsage = 0;

  HeapMap *heapmap = new HeapMap ();
  long sz = dview->getSize ();
  for (long i = 0; i < sz; i++)
    {

      Heap_type mtype = (Heap_type) dview->getIntValue (PROP_HTYPE, i);
      Vaddr vaddr = dview->getULongValue (PROP_HVADDR, i);
      Vaddr ovaddr = dview->getULongValue (PROP_HOVADDR, i);
      Size hsize = dview->getULongValue (PROP_HSIZE, i);
      hrtime_t tstamp = dview->getLongValue (PROP_TSTAMP, i);

      switch (mtype)
	{
	case MALLOC_TRACE:
	  dview->setValue (PROP_TSTAMP2, i, (uint64_t) MAX_TIME);
	  if (vaddr)
	    {
	      dview->setValue (PROP_HLEAKED, i, hsize);
	      heapmap->allocate (vaddr, i + 1);

	      // Increase heap size
	      memoryUsage += hsize;
	      dview->setValue (PROP_HMEM_USAGE, i, memoryUsage);
	    }
	  break;

	case FREE_TRACE:
	  if (vaddr)
	    {
	      long idx = heapmap->deallocate (vaddr) - 1;
	      if (idx >= 0)
		{
		  // Decrease heap size
		  Size leaked = dview->getLongValue (PROP_HLEAKED, idx);
		  memoryUsage -= leaked;
		  dview->setValue (PROP_HMEM_USAGE, i, memoryUsage);

		  Size alloc = dview->getLongValue (PROP_HSIZE, idx);
		  // update allocation
		  dview->setValue (PROP_HLEAKED, idx, (uint64_t) 0);
		  dview->setValue (PROP_TSTAMP2, idx, tstamp);
		  dview->setValue (PROP_DDSCR_LNK, idx, dview->getIdByIdx (i) + 1);
		  // update this event
		  dview->setValue (PROP_HFREED, i, alloc);
		}
	    }
	  break;

	case REALLOC_TRACE:
	  dview->setValue (PROP_TSTAMP2, i, (uint64_t) MAX_TIME);
	  if (ovaddr)
	    {
	      long idx = heapmap->deallocate (ovaddr) - 1;
	      if (idx >= 0)
		{
		  // Decrease heap size
		  Size leaked = dview->getLongValue (PROP_HLEAKED, idx);
		  memoryUsage -= leaked;
		  dview->setValue (PROP_HMEM_USAGE, i, memoryUsage);

		  Size alloc = dview->getLongValue (PROP_HSIZE, idx);
		  // update allocation
		  dview->setValue (PROP_HLEAKED, idx, (uint64_t) 0);
		  dview->setValue (PROP_TSTAMP2, idx, tstamp);
		  dview->setValue (PROP_DDSCR_LNK, idx, dview->getIdByIdx (i) + 1);
		  // update this event
		  dview->setValue (PROP_HFREED, i, alloc);
		}
	    }
	  if (vaddr)
	    {
	      dview->setValue (PROP_HLEAKED, i, hsize);
	      heapmap->allocate (vaddr, i + 1);

	      // Increase heap size
	      memoryUsage += hsize;
	      dview->setValue (PROP_HMEM_USAGE, i, memoryUsage);
	    }
	  break;
	case MMAP_TRACE:
	case MUNMAP_TRACE:
	  // Adjust the size to be multiple of page_size
	  //hsize = (( hsize - 1 ) / page_size + 1 ) * page_size;
	  if (vaddr)
	    {
	      UnmapChunk *list;
	      if (mtype == MMAP_TRACE)
		{
		  dview->setValue (PROP_TSTAMP2, i, (uint64_t) MAX_TIME);
		  dview->setValue (PROP_HLEAKED, i, hsize);
		  list = heapmap->mmap (vaddr, hsize, i);

		  // Increase heap size
		  memoryUsage += hsize;
		  dview->setValue (PROP_HMEM_USAGE, i, memoryUsage);
		}
	      else
		{ // MUNMAP_TRACE
		  list = heapmap->munmap (vaddr, hsize);

		  // Set allocation size to zero
		  // Note: We're currently reusing PROP_HSIZE to mean allocation size
		  // If we ever need to save the original HSIZE, we'll need to
		  // create a new PROP_* to represent event allocation size
		  //
		  //	For now, tuck the original size away as HOVADDR
		  dview->setValue (PROP_HOVADDR, i, (uint64_t) hsize);
		  dview->setValue (PROP_HSIZE, i, (uint64_t) 0);
		}
	      Size total_freed = 0;
	      while (list)
		{
		  long idx = list->val;
		  total_freed += list->size;
		  Size leaked = dview->getLongValue (PROP_HLEAKED, idx);

		  // Decrease heap size
		  memoryUsage -= list->size;
		  dview->setValue (PROP_HMEM_USAGE, i, memoryUsage);

		  Size leak_update = leaked - list->size;
		  // update allocation
		  dview->setValue (PROP_HLEAKED, idx, leak_update);
		  // update allocation's list of frees
		  {
		    UnmapChunk *copy = new UnmapChunk;
		    heapUnmapEvents->append (copy);
		    copy->val = dview->getIdByIdx (i);
		    copy->size = list->size;
		    copy->next = (UnmapChunk *) dview->getObjValue (PROP_VOIDP_OBJ, idx);
		    dview->setObjValue (PROP_VOIDP_OBJ, idx, copy);
		  }
		  if (leak_update <= 0)
		    if (leak_update == 0)
		      dview->setValue (PROP_TSTAMP2, idx, tstamp);
		  UnmapChunk *t = list;
		  list = list->next;
		  delete t;
		}
	      // update this event
	      if (total_freed)
		// only need to write value if it is non-zero
		dview->setValue (PROP_HFREED, i, total_freed);
	    }
	  break;
	  // ignoring HEAPTYPE_LAST, which will never be recorded
	case HEAPTYPE_LAST:
	  break;
	}
    }
  delete heapmap;
  delete dview;

  return dDscr;
}

DataDescriptor *
Experiment::get_heapsz_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_HEAPSZ);
  if (dDscr)
    return dDscr;
  dDscr = get_heap_events (); // derived from DATA_HEAP
  if (dDscr == NULL)
    return NULL;
  dDscr = newDataDescriptor (DATA_HEAPSZ, 0, dDscr);
  return dDscr;
}

static void
update_heapsz_packet (std::set<long> &pkt_id_set, DataView *dview,
		      long alloc_pkt_id, int64_t net_alloc, uint64_t leaks)
{
  // pkt_id_set: set is updated to include packet
  // alloc_pkt_id: data descriptor id (NOT dview idx)
  // net_alloc: adjustment to net allocation for this packet (note: signed value)
  // leaks: leak bytes to attribute to alloc_pkt_id
  std::pair < std::set<long>::iterator, bool> ret;
  ret = pkt_id_set.insert (alloc_pkt_id); // add to set
  bool new_to_set = ret.second; // was not in set
  if (!new_to_set)
    {
      // Has been seen before, update values
      net_alloc += dview->getDataDescriptorValue (PROP_HCUR_NET_ALLOC, alloc_pkt_id);
      if (leaks)
	{
	  uint64_t old = dview->getDataDescriptorValue (PROP_HCUR_LEAKS, alloc_pkt_id);
	  if (old != 0)
	    leaks = old;
	}
    }
  dview->setDataDescriptorValue (PROP_HCUR_NET_ALLOC, alloc_pkt_id, net_alloc);
  dview->setDataDescriptorValue (PROP_HCUR_LEAKS, alloc_pkt_id, leaks);
}

DataView *
Experiment::create_heapsz_data_view (DataView *heap_dview)
{
  // heap_dview has DATA_HEAP _filtered_ packets.
  // This creates, populates, and returns DATA_HEAPSZ DataView
  DataDescriptor *dDscr = get_heapsz_events ();
  if (dDscr == NULL)
    return NULL;
  std::set<long> pkt_id_set;
  DataView *dview = heap_dview;
  long sz = dview->getSize ();
  for (long i = 0; i < sz; i++)
    {
      int64_t hsize = (int64_t) dview->getULongValue (PROP_HSIZE, i);
      uint64_t leaks = dview->getULongValue (PROP_HLEAKED, i);
      long alloc_pkt_id = dview->getIdByIdx (i);
      update_heapsz_packet (pkt_id_set, dview, alloc_pkt_id, hsize, leaks);

      // linked free
      UnmapChunk *mmap_frees = (UnmapChunk *) dview->getObjValue (PROP_VOIDP_OBJ, i); // mmap metadata
      if (mmap_frees)
	{
	  // mmap: all frees associated with this packet
	  while (mmap_frees)
	    {
	      long free_pkt_id = mmap_frees->val;
	      int64_t free_sz = mmap_frees->size;
	      update_heapsz_packet (pkt_id_set, dview, free_pkt_id, -free_sz, 0);
	      mmap_frees = mmap_frees->next;
	    }
	}
      else
	{
	  // malloc: check for associated free
	  long free_pkt_id = dview->getLongValue (PROP_DDSCR_LNK, i) - 1;
	  if (free_pkt_id >= 0)
	    update_heapsz_packet (pkt_id_set, dview, free_pkt_id, -hsize, 0);
	}
    }

  // create a new DataView based on the filtered-in and associated free events
  std::set<long>::iterator it;
  DataView *heapsz_dview = dDscr->createExtManagedView ();
  for (it = pkt_id_set.begin (); it != pkt_id_set.end (); ++it)
    {
      long ddscr_pkt_id = *it;
      heapsz_dview->appendDataDescriptorId (ddscr_pkt_id);
    }
  compute_heapsz_data_view (heapsz_dview);
  return heapsz_dview;
}

void
Experiment::compute_heapsz_data_view (DataView *heapsz_dview)
{
  DataView *dview = heapsz_dview;

  // Keep track of memory usage
  int64_t currentAllocs = 0;
  Size currentLeaks = 0;
  dview->sort (PROP_TSTAMP);
  long sz = dview->getSize ();
  for (long i = 0; i < sz; i++)
    {
      int64_t net_alloc = dview->getLongValue (PROP_HCUR_NET_ALLOC, i);
      currentAllocs += net_alloc;
      dview->setValue (PROP_HCUR_ALLOCS, i, currentAllocs);

      Size leaks = dview->getULongValue (PROP_HCUR_LEAKS, i);
      currentLeaks += leaks;
      dview->setValue (PROP_HCUR_LEAKS, i, currentLeaks);
    }
}

void
Experiment::DBG_memuse (Sample * s)
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_HEAP);
  if (dDscr == NULL || dDscr->getSize () == 0)
    return;

  DataView *dview = dDscr->createView ();
  dview->sort (PROP_TSTAMP);
  hrtime_t ts1 = s->get_start_time ();
  hrtime_t ts2 = s->get_end_time ();

  HeapMap *heapmap = new HeapMap ();
  long sz = dview->getSize ();
  Size maxSize = 0;
  Size curSize = 0;
  hrtime_t maxTime = 0;
  for (long i = 0; i < sz; i++)
    {
      hrtime_t tstamp = dview->getLongValue (PROP_TSTAMP, i);
      if (tstamp < ts1)
	continue;
      if (tstamp >= ts2)
	break;

      Heap_type mtype = (Heap_type) dview->getIntValue (PROP_HTYPE, i);
      Vaddr vaddr = dview->getULongValue (PROP_HVADDR, i);
      Vaddr ovaddr = dview->getULongValue (PROP_HOVADDR, i);
      switch (mtype)
	{
	case REALLOC_TRACE:
	  break;
	case MALLOC_TRACE:
	  ovaddr = 0;
	  break;
	case FREE_TRACE:
	  ovaddr = vaddr;
	  vaddr = 0;
	  break;
	default:
	  vaddr = 0;
	  ovaddr = 0;
	  break;
	}
      if (ovaddr)
	{
	  long idx = heapmap->deallocate (ovaddr) - 1;
	  if (idx >= 0)
	    curSize -= dview->getULongValue (PROP_HSIZE, idx);
	}
      if (vaddr)
	{
	  heapmap->allocate (vaddr, i + 1);
	  curSize += dview->getULongValue (PROP_HSIZE, i);
	  if (curSize > maxSize)
	    {
	      maxSize = curSize;
	      maxTime = tstamp;
	    }
	}
    }
  printf ("SAMPLE=%s (id=%d) MEMUSE=%lld TSTAMP=%lld\n", s->get_start_label (),
	  s->get_number (), maxSize, maxTime - getStartTime ());
  delete dview;
  delete heapmap;
}

void
Experiment::DBG_memuse (const char *sname)
{
  for (int i = 0; i < samples->size (); ++i)
    {
      Sample *sample = samples->fetch (i);
      if (streq (sname, sample->get_start_label ()))
	{
	  DBG_memuse (sample);
	  break;
	}
    }
}

DataDescriptor *
Experiment::get_race_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_RACE);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () == 0)
    {
      char *base_name = get_basename (expt_name);
      char *msg = dbe_sprintf (GTXT ("Loading Race Data: %s"), base_name);
      read_data_file (SP_RACETRACE_FILE, msg);
      free (msg);
      resolve_frame_info (dDscr);
    }
  return dDscr;
}

DataDescriptor *
Experiment::get_deadlock_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_DLCK);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () == 0)
    {
      char *base_name = get_basename (expt_name);
      char *msg = dbe_sprintf (GTXT ("Loading Deadlocks Data: %s"), base_name);
      read_data_file (SP_DEADLOCK_FILE, msg);
      free (msg);
      resolve_frame_info (dDscr);
    }
  return dDscr;
}

DataDescriptor *
Experiment::get_sample_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_SAMPLE);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () > 0)
    return dDscr;

  // read_overview_file(); //YXXX do this here at some point instead of:
  PropDescr *tmp_propDscr;
  tmp_propDscr = new PropDescr (PROP_SMPLOBJ, NTXT ("SMPLOBJ"));
  tmp_propDscr->uname = NULL;
  tmp_propDscr->vtype = TYPE_OBJ;
  dDscr->addProperty (tmp_propDscr);

  tmp_propDscr = new PropDescr (PROP_TSTAMP, NTXT ("TSTAMP"));
  tmp_propDscr->uname = dbe_strdup ("High resolution timestamp");
  tmp_propDscr->vtype = TYPE_UINT64;
  dDscr->addProperty (tmp_propDscr);

  tmp_propDscr = new PropDescr (PROP_SAMPLE, NTXT ("SAMPLE"));
  tmp_propDscr->uname = dbe_strdup ("Sample number");
  tmp_propDscr->vtype = TYPE_UINT64;
  dDscr->addProperty (tmp_propDscr);

  tmp_propDscr = new PropDescr (PROP_EVT_TIME, NTXT ("EVT_TIME"));
  tmp_propDscr->uname = dbe_strdup ("Event duration");
  tmp_propDscr->vtype = TYPE_UINT64;
  dDscr->addProperty (tmp_propDscr);

  long ssize = samples->size ();
  for (long ii = 0; ii < ssize; ii++)
    {
      Sample * sample = samples->fetch (ii);
      long recn = dDscr->addRecord ();
      hrtime_t sduration = sample->get_end_time () - sample->get_start_time ();
      dDscr->setObjValue (PROP_SMPLOBJ, recn, sample);
      dDscr->setValue (PROP_SAMPLE, recn, sample->get_number ());
      dDscr->setValue (PROP_TSTAMP, recn, sample->get_end_time ());
      dDscr->setValue (PROP_EVT_TIME, recn, sduration);
    }
  return dDscr;
}

DataDescriptor *
Experiment::get_gc_events ()
{
  DataDescriptor *dDscr = getDataDescriptor (DATA_GCEVENT);
  if (dDscr == NULL)
    return NULL;
  if (dDscr->getSize () > 0)
    return dDscr;

  // read_overview_file(); //YXXX do this here at some point instead of:
  PropDescr *tmp_propDscr;
  tmp_propDscr = new PropDescr (PROP_GCEVENTOBJ, NTXT ("GCEVENTOBJ"));
  tmp_propDscr->uname = NULL;
  tmp_propDscr->vtype = TYPE_OBJ;
  dDscr->addProperty (tmp_propDscr);

  tmp_propDscr = new PropDescr (PROP_TSTAMP, NTXT ("TSTAMP"));
  tmp_propDscr->uname = dbe_strdup ("High resolution timestamp");
  tmp_propDscr->vtype = TYPE_UINT64;
  dDscr->addProperty (tmp_propDscr);

  tmp_propDscr = new PropDescr (PROP_GCEVENT, NTXT ("GCEVENT"));
  tmp_propDscr->uname = dbe_strdup ("GCEvent number");
  tmp_propDscr->vtype = TYPE_UINT64;
  dDscr->addProperty (tmp_propDscr);

  tmp_propDscr = new PropDescr (PROP_EVT_TIME, NTXT ("EVT_TIME"));
  tmp_propDscr->uname = dbe_strdup ("Event duration");
  tmp_propDscr->vtype = TYPE_UINT64;
  dDscr->addProperty (tmp_propDscr);

  long ssize = gcevents->size ();
  for (long ii = 0; ii < ssize; ii++)
    {
      GCEvent * gcevent = gcevents->fetch (ii);
      long recn = dDscr->addRecord ();
      hrtime_t sduration = gcevent->end - gcevent->start;
      dDscr->setObjValue (PROP_GCEVENTOBJ, recn, gcevent);
      dDscr->setValue (PROP_GCEVENT, recn, gcevent->id);
      dDscr->setValue (PROP_TSTAMP, recn, gcevent->end);
      dDscr->setValue (PROP_EVT_TIME, recn, sduration);
    }
  return dDscr;
}

void
Experiment::update_last_event (hrtime_t ts/*wall_ts*/)
{
  if (last_event == ZERO_TIME)
    {
      // not yet initialized
      last_event = ts;
    }
  if (last_event - exp_start_time < ts - exp_start_time)
    // compare deltas to avoid hrtime_t wrap
    last_event = ts;
}

void
Experiment::write_header ()
{
  StringBuilder sb;

  // write commentary to the experiment, describing the parameters
  if (dbeSession->ipc_mode || dbeSession->rdt_mode)
    {
      // In GUI: print start time at the beginning
      time_t t = (time_t) start_sec;
      char *start_time = ctime (&t);
      if (start_time != NULL)
	{
	  sb.setLength (0);
	  sb.sprintf (GTXT ("Experiment started %s"), start_time);
	  commentq->append (new Emsg (CMSG_COMMENT, sb));
	}
    }
  // write message with target arglist
  if (uarglist != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("\nTarget command (%s): '%s'"),
		  (wsize == W32 ? "32-bit" : "64-bit"), uarglist);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  sb.setLength (0);
  sb.sprintf (GTXT ("Process pid %d, ppid %d, pgrp %d, sid %d"),
	      pid, ppid, pgrp, sid);
  commentq->append (new Emsg (CMSG_COMMENT, sb));

  // add comment for user name, if set
  if (username != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("User: `%s'"), username);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // add comment for current working directory
  if (ucwd != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("Current working directory: %s"), ucwd);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // add comment for collector version string
  if (cversion != NULL)
    {
      char *wstring;
      switch (wsize)
	{
	case Wnone:
	  wstring = NTXT ("?");
	  break;
	case W32:
	  wstring = GTXT ("32-bit");
	  break;
	case W64:
	  wstring = GTXT ("64-bit");
	  break;
	default:
	  wstring = NTXT ("??");
	  break;
	}
      sb.setLength (0);
      sb.sprintf (GTXT ("Collector version: `%s'; experiment version %d.%d (%s)"),
		  cversion, exp_maj_version, exp_min_version, wstring);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // add comment for driver version string (er_kernel)
  if (dversion != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("Kernel driver version: `%s'"), dversion);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  if (jversion != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("JVM version: `%s'"), jversion);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // add comment for hostname, parameters
  if (hostname == NULL)
    hostname = dbe_strdup (GTXT ("unknown"));
  if (os_version == NULL)
    os_version = dbe_strdup (GTXT ("unknown"));
  if (architecture == NULL)
    architecture = dbe_strdup (GTXT ("unknown"));
  sb.setLength (0);
  sb.sprintf (GTXT ("Host `%s', OS `%s', page size %d, architecture `%s'"),
	      hostname, os_version, page_size, architecture);
  commentq->append (new Emsg (CMSG_COMMENT, sb));

  sb.setLength (0);
  if (maxclock != minclock)
    {
      clock = maxclock;
      sb.sprintf (
		  GTXT ("  %d CPUs, with clocks ranging from %d to %d MHz.; max of %d MHz. assumed"),
		  ncpus, minclock, maxclock, clock);
    }
  else
    sb.sprintf (GTXT ("  %d CPU%s, clock speed %d MHz."),
		ncpus, (ncpus == 1 ? NTXT ("") : "s"), clock);
  commentq->append (new Emsg (CMSG_COMMENT, sb));

  // add comment for machine memory size
  if (page_size > 0 && npages > 0)
    {
      long long memsize = ((long long) npages * page_size) / (1024 * 1024);
      sb.setLength (0);
      sb.sprintf (GTXT ("  Memory: %d pages @  %d = %lld MB."),
		  npages, page_size, memsize);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // add comment for machine memory size
  if (machinemodel != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Machine model: %s"), machinemodel);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }

  // add comment for start time
  time_t t = (time_t) start_sec;
  char *p = ctime (&t);
  sb.setLength (0);
  if (p != NULL)
    sb.sprintf (GTXT ("Experiment started %s"), p);
  else
    sb.sprintf (GTXT ("\nExperiment start not recorded"));
  write_coll_params ();
  commentq->append (new Emsg (CMSG_COMMENT, sb));
  commentq->appendqueue (runlogq);
  runlogq->mark_clear ();
}

void
Experiment::write_coll_params ()
{
  StringBuilder sb;

  // now write the various collection parameters as comments
  sb.setLength (0);
  sb.append (GTXT ("Data collection parameters:"));
  commentq->append (new Emsg (CMSG_COMMENT, sb));
  if (coll_params.profile_mode == 1)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Clock-profiling, interval = %d microsecs."),
		  (int) (coll_params.ptimer_usec));
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.sync_mode == 1)
    {
      sb.setLength (0);
      char *scope_str = NTXT ("");
      switch (coll_params.sync_scope)
	{
	case 0:
	  scope_str = GTXT ("Native- and Java-APIs");
	  break;
	case SYNCSCOPE_JAVA:
	  scope_str = GTXT ("JAVA-APIs");
	  break;
	case SYNCSCOPE_NATIVE:
	  scope_str = GTXT ("Native-APIs");
	  break;
	case SYNCSCOPE_JAVA | SYNCSCOPE_NATIVE:
	  scope_str = GTXT ("Native- and Java-APIs");
	  break;
	}
      if (coll_params.sync_threshold < 0)
	sb.sprintf (GTXT ("  Synchronization tracing, threshold = %d microsecs. (calibrated); %s"),
		    -coll_params.sync_threshold, scope_str);
      else
	sb.sprintf (GTXT ("  Synchronization tracing, threshold = %d microsecs.; %s"),
		    coll_params.sync_threshold, scope_str);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.heap_mode == 1)
    {
      sb.setLength (0);
      sb.append (GTXT ("  Heap tracing"));
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.io_mode == 1)
    {
      sb.setLength (0);
      sb.append (GTXT ("  IO tracing"));
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.race_mode == 1)
    {
      sb.setLength (0);
      char *race_stack_name;
      switch (coll_params.race_stack)
	{
	case 0:
	  race_stack_name = GTXT ("dual-stack");
	  break;
	case 1:
	  race_stack_name = GTXT ("single-stack");
	  break;
	case 2:
	  race_stack_name = GTXT ("leaf");
	  break;
	default:
	  abort ();
	}
      sb.sprintf (GTXT ("  Datarace detection, %s"), race_stack_name);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.deadlock_mode == 1)
    {
      sb.setLength (0);
      sb.append (GTXT ("  Deadlock detection"));
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.hw_mode == 1)
    {
      sb.setLength (0);
      if (hwc_default == true)
	sb.append (GTXT ("  HW counter-profiling (default); counters:"));
      else
	sb.append (GTXT ("  HW counter-profiling; counters:"));
      commentq->append (new Emsg (CMSG_COMMENT, sb));
      for (int i = 0; i < MAX_HWCOUNT; i++)
	{
	  if (!coll_params.hw_aux_name[i])
	    continue;
	  sb.setLength (0);
	  sb.sprintf (GTXT ("    %s, tag %d, interval %d, memop %d"),
		      coll_params.hw_aux_name[i], i,
		      coll_params.hw_interval[i], coll_params.hw_tpc[i]);
	  commentq->append (new Emsg (CMSG_COMMENT, sb));
	}
    }
  if (coll_params.sample_periodic == 1)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Periodic sampling, %d secs."),
		  coll_params.sample_timer);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.limit != 0)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Experiment size limit, %d"),
		  coll_params.limit);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.linetrace != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Follow descendant processes from: %s"),
		  coll_params.linetrace);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.pause_sig != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Pause signal %s"), coll_params.pause_sig);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.sample_sig != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Sample signal %s"), coll_params.sample_sig);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.start_delay != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Data collection delay start %s seconds"), coll_params.start_delay);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  if (coll_params.terminate != NULL)
    {
      sb.setLength (0);
      sb.sprintf (GTXT ("  Data collection termination after %s seconds"), coll_params.terminate);
      commentq->append (new Emsg (CMSG_COMMENT, sb));
    }
  // add a blank line after data description
  commentq->append (new Emsg (CMSG_COMMENT, NTXT ("")));
}


/*
 *    Raw packet processing
 */
static int
check_mstate (char *ptr, PacketDescriptor *pDscr, int arg)
{
  switch (arg)
    {
    case PROP_UCPU:
    case PROP_SCPU:
    case PROP_TRAP:
    case PROP_TFLT:
    case PROP_DFLT:
    case PROP_KFLT:
    case PROP_ULCK:
    case PROP_TSLP:
    case PROP_WCPU:
    case PROP_TSTP:
      break;
    default:
      return 0;
    }
  Vector<FieldDescr*> *fields = pDscr->getFields ();
  for (int i = 0, sz = fields->size (); i < sz; i++)
    {
      FieldDescr *fDscr = fields->fetch (i);
      if (fDscr->propID == arg)
	return *((int*) (ptr + fDscr->offset));
    }
  return 0;
}

#define PACKET_ALIGNMENT 4

uint64_t
Experiment::readPacket (Data_window *dwin, Data_window::Span *span)
{
  Common_packet *rcp = (Common_packet *) dwin->bind (span,
						    sizeof (CommonHead_packet));
  uint16_t v16;
  uint64_t size = 0;
  if (rcp)
    {
      if ((((long) rcp) % PACKET_ALIGNMENT) != 0)
	{
	  invalid_packet++;
	  size = PROFILE_BUFFER_CHUNK - span->offset % PROFILE_BUFFER_CHUNK;
	  return size;
	}
      v16 = (uint16_t) rcp->tsize;
      size = dwin->decode (v16);
      if (size == 0)
	{
	  size = PROFILE_BUFFER_CHUNK - span->offset % PROFILE_BUFFER_CHUNK;
	  return size;
	}
      rcp = (Common_packet *) dwin->bind (span, size);
    }
  if (rcp == NULL)
    return 0;

  if ((((long) rcp) % PACKET_ALIGNMENT) != 0)
    {
      invalid_packet++;
      size = PROFILE_BUFFER_CHUNK - span->offset % PROFILE_BUFFER_CHUNK;
      return size;
    }
  v16 = (uint16_t) rcp->type;
  uint32_t rcptype = dwin->decode (v16);
  if (rcptype == EMPTY_PCKT)
    return size;
  if (rcptype == FRAME_PCKT)
    {
      RawFramePacket *fp = new RawFramePacket;
      fp->uid = dwin->decode (((Frame_packet*) rcp)->uid);
      fp->uidn = NULL;
      fp->uidj = NULL;
      fp->omp_uid = NULL;
      fp->omp_state = 0;
      char *ptr = (char*) rcp + dwin->decode (((Frame_packet*) rcp)->hsize);
      if ((((long) ptr) % PACKET_ALIGNMENT) != 0)
	{
	  invalid_packet++;
	  delete fp;
	  return size;
	}
      v16 = (uint16_t) ((Frame_packet*) rcp)->tsize;
      char *end = (char*) rcp + dwin->decode (v16);
      for (; ptr < end;)
	{
	  Common_info *cinfo = (Common_info*) ptr;
	  uint32_t hsize = dwin->decode (cinfo->hsize);
	  if (hsize == 0 || ptr + hsize > end)
	    break;
	  int kind = dwin->decode (cinfo->kind);
	  bool compressed = false;
	  if (kind & COMPRESSED_INFO)
	    {
	      compressed = true;
	      kind &= ~COMPRESSED_INFO;
	    }
	  switch (kind)
	    {
	    case STACK_INFO:
	      {
		char *stack = ptr + sizeof (Stack_info);
		size_t stack_size = hsize - sizeof (Stack_info);
		uint64_t uidn = dwin->decode (((Stack_info*) cinfo)->uid);
		if (stack_size <= 0)
		  {
		    fp->uidn = get_uid_node (uidn);
		    break;
		  }
		uint64_t link_uid = (uint64_t) 0;
		if (compressed)
		  {
		    stack_size -= sizeof (uint64_t);
		    unsigned char *s = (unsigned char*) (stack + stack_size);
		    int shift = 0;
		    for (size_t i = 0; i<sizeof (link_uid); i++)
		      {
			link_uid |= (uint64_t) * s++ << shift;
			shift += 8;
		      }
		  }
		if (wsize == W32)
		  fp->uidn = add_uid (dwin, uidn,
				      (int) (stack_size / sizeof (uint32_t)),
				      (uint32_t*) stack, link_uid);
		else
		  fp->uidn = add_uid (dwin, uidn,
				      (int) (stack_size / sizeof (uint64_t)),
				      (uint64_t*) stack, link_uid);
		break;
	      }
	    case JAVA_INFO:
	      {
		char *stack = ptr + sizeof (Java_info);
		size_t stack_size = hsize - sizeof (Java_info);
		uint64_t uidj = dwin->decode (((Java_info*) cinfo)->uid);
		if (stack_size <= 0)
		  {
		    fp->uidj = get_uid_node (uidj);
		    break;
		  }

		uint64_t link_uid = (uint64_t) 0;
		if (compressed)
		  {
		    stack_size -= sizeof (uint64_t);
		    unsigned char *s = (unsigned char*) (stack + stack_size);
		    int shift = 0;
		    for (size_t i = 0; i<sizeof (link_uid); i++)
		      {
			link_uid |= (uint64_t) * s++ << shift;
			shift += 8;
		      }
		  }
		if (wsize == W32)
		  fp->uidj = add_uid (dwin, uidj,
				      (int) (stack_size / sizeof (uint32_t)),
				      (uint32_t*) stack, link_uid);
		else
		  {
		    // bug 6909545: garbage in 64-bit JAVA_INFO
		    char *nstack = (char*) malloc (stack_size);
		    char *dst = nstack;
		    char *srcmax = stack + stack_size - sizeof (uint64_t);
		    for (char *src = stack; src <= srcmax;)
		      {
			int64_t val = dwin->decode (*(int32_t*) src);
			*(uint64_t*) dst = dwin->decode (val);
			src += sizeof (uint64_t);
			dst += sizeof (uint64_t);
			if (src > srcmax)
			  {
			    fprintf (stderr, "er_print: Experiment::readPacket: Error in data: src=%llx greater than %llx\n",
				     (long long) src, (long long) srcmax);
			    break;
			  }
			*(uint64_t*) dst = *(uint64_t*) src;
			src += sizeof (uint64_t);
			dst += sizeof (uint64_t);
		      }
		    fp->uidj = add_uid (dwin, uidj,
					(int) (stack_size / sizeof (uint64_t)),
					(uint64_t*) nstack, link_uid);
		    free (nstack);
		  }
		break;
	      }
	    case OMP_INFO:
	      fp->omp_state = dwin->decode (((OMP_info*) ptr)->omp_state);
	      break;
	    case OMP2_INFO:
	      {
		uint64_t omp_uid = dwin->decode (((OMP2_info*) ptr)->uid);
		fp->omp_uid = get_uid_node (omp_uid);
		fp->omp_state = dwin->decode (((OMP2_info*) ptr)->omp_state);
		break;
	      }
	    default:
	      break;
	    }
	  ptr += hsize;
	}
      frmpckts->append (fp);
      return size;
    }
  else if (rcptype == UID_PCKT)
    {
      Uid_packet *uidp = (Uid_packet*) rcp;
      uint64_t uid = dwin->decode (uidp->uid);
      char *arr_bytes = (char*) (uidp + 1);
      v16 = (uint16_t) rcp->tsize;
      size_t arr_length = dwin->decode (v16) - sizeof (Uid_packet);
      if (arr_length <= 0)
	return size;
      uint64_t link_uid = (uint64_t) 0;
      if (dwin->decode (uidp->flags) & COMPRESSED_INFO)
	{
	  arr_length -= sizeof (uint64_t);
	  unsigned char *s = (unsigned char*) (arr_bytes + arr_length);
	  int shift = 0;
	  for (size_t i = 0; i<sizeof (link_uid); i++)
	    {
	      link_uid |= (uint64_t) * s++ << shift;
	      shift += 8;
	    }
	}
      if (wsize == W32)
	add_uid (dwin, uid, (int) (arr_length / sizeof (uint32_t)),
		 (uint32_t*) arr_bytes, link_uid);
      else
	add_uid (dwin, uid, (int) (arr_length / sizeof (uint64_t)),
		 (uint64_t*) arr_bytes, link_uid);
      return size;
    }

  PacketDescriptor *pcktDescr = getPacketDescriptor (rcptype);
  if (pcktDescr == NULL)
    return size;
  DataDescriptor *dataDescr = pcktDescr->getDataDescriptor ();
  if (dataDescr == NULL)
    return size;

  /* omazur: TBR START -- old experiment */
  if (rcptype == PROF_PCKT)
    {
      // For backward compatibility with older SS12 experiments
      int numstates = get_params ()->lms_magic_id; // ugly, for old experiments
      if (numstates > LMS_NUM_SOLARIS_MSTATES)
	numstates = LMS_NUM_SOLARIS_MSTATES;
      for (int i = 0; i < numstates; i++)
	if (check_mstate ((char*) rcp, pcktDescr, PROP_UCPU + i))
	  readPacket (dwin, (char*) rcp, pcktDescr, dataDescr, PROP_UCPU + i,
		      size);
    }
  else
    readPacket (dwin, (char*) rcp, pcktDescr, dataDescr, 0, size);
  return size;
}

void
Experiment::readPacket (Data_window *dwin, char *ptr, PacketDescriptor *pDscr,
			DataDescriptor *dDscr, int arg, uint64_t pktsz)
{
  union Value
  {
    uint32_t val32;
    uint64_t val64;
  } *v;

  long recn = dDscr->addRecord ();
  Vector<FieldDescr*> *fields = pDscr->getFields ();
  int sz = fields->size ();
  for (int i = 0; i < sz; i++)
    {
      FieldDescr *field = fields->fetch (i);
      v = (Value*) (ptr + field->offset);
      if (field->propID == arg)
	{
	  dDscr->setValue (PROP_NTICK, recn, dwin->decode (v->val32));
	  dDscr->setValue (PROP_MSTATE, recn, (uint32_t) (field->propID - PROP_UCPU));
	}
      if (field->propID == PROP_THRID || field->propID == PROP_LWPID
	  || field->propID == PROP_CPUID)
	{
	  uint64_t tmp64 = 0;
	  switch (field->vtype)
	    {
	    case TYPE_INT32:
	    case TYPE_UINT32:
	      tmp64 = dwin->decode (v->val32);
	      break;
	    case TYPE_INT64:
	    case TYPE_UINT64:
	      tmp64 = dwin->decode (v->val64);
	      break;
	    case TYPE_STRING:
	    case TYPE_DOUBLE:
	    case TYPE_OBJ:
	    case TYPE_DATE:
	    case TYPE_BOOL:
	    case TYPE_ENUM:
	    case TYPE_LAST:
	    case TYPE_NONE:
	      break;
	    }
	  uint32_t tag = mapTagValue ((Prop_type) field->propID, tmp64);
	  dDscr->setValue (field->propID, recn, tag);
	}
      else
	{
	  switch (field->vtype)
	    {
	    case TYPE_INT32:
	    case TYPE_UINT32:
	      dDscr->setValue (field->propID, recn, dwin->decode (v->val32));
	      break;
	    case TYPE_INT64:
	    case TYPE_UINT64:
	      dDscr->setValue (field->propID, recn, dwin->decode (v->val64));
	      break;
	    case TYPE_STRING:
	      {
		int len = (int) (pktsz - field->offset);
		if ((len > 0) && (ptr[field->offset] != 0))
		  {
		    StringBuilder *sb = new StringBuilder ();
		    sb->append (ptr + field->offset, 0, len);
		    dDscr->setObjValue (field->propID, recn, sb);
		  }
		break;
	      }
	      // ignoring the following cases (why?)
	    case TYPE_DOUBLE:
	    case TYPE_OBJ:
	    case TYPE_DATE:
	    case TYPE_BOOL:
	    case TYPE_ENUM:
	    case TYPE_LAST:
	    case TYPE_NONE:
	      break;
	    }
	}
    }
}

#define PROG_BYTE 102400 // update progress bar every PROG_BYTE bytes

void
Experiment::read_data_file (const char *fname, const char *msg)
{
  Data_window::Span span;
  off64_t total_len, remain_len;
  char *progress_bar_msg;
  int progress_bar_percent = -1;

  char *data_file_name = dbe_sprintf (NTXT ("%s/%s"), expt_name, fname);
  Data_window *dwin = new Data_window (data_file_name);
  // Here we can call stat(data_file_name) to get file size,
  // and call a function to reallocate vectors for clock profiling data
  free (data_file_name);
  if (dwin->not_opened ())
    {
      delete dwin;
      return;
    }
  dwin->need_swap_endian = need_swap_endian;

  span.offset = 0;
  span.length = dwin->get_fsize ();
  total_len = remain_len = span.length;
  progress_bar_msg = dbe_sprintf (NTXT ("%s %s"), NTXT ("  "), msg);
  invalid_packet = 0;
  for (;;)
    {
      uint64_t pcktsz = readPacket (dwin, &span);
      if (pcktsz == 0)
	break;
      // Update progress bar
      if ((span.length <= remain_len) && (remain_len > 0))
	{
	  int percent = (int) (100 * (total_len - remain_len) / total_len);
	  if (percent > progress_bar_percent)
	    {
	      progress_bar_percent += 10;
	      theApplication->set_progress (percent, progress_bar_msg);
	    }
	  remain_len -= PROG_BYTE;
	}
      span.length -= pcktsz;
      span.offset += pcktsz;
    }
  delete dwin;

  if (invalid_packet)
    {
      StringBuilder sb;
      sb.sprintf (GTXT ("WARNING: There are %d invalid packet(s) in the %s file"),
		  invalid_packet, fname);
      Emsg *m = new Emsg (CMSG_WARN, sb);
      warnq->append (m);
    }

  theApplication->set_progress (0, NTXT (""));
  free (progress_bar_msg);
}

int
Experiment::read_overview_file ()
{
  char *data_file_name = dbe_sprintf ("%s/%s", expt_name, SP_OVERVIEW_FILE);
  Data_window *dwin = new Data_window (data_file_name);
  free (data_file_name);
  if (dwin->not_opened ())
    {
      delete dwin;
      return 0;
    }
  dwin->need_swap_endian = need_swap_endian;
  newDataDescriptor (DATA_SAMPLE);

  Data_window::Span span;
  span.offset = 0;
  span.length = dwin->get_fsize ();

  PrUsage *data = NULL, *data_prev = NULL;
  Sample *sample;
  int sample_number = 1;

  int64_t prDataSize;
  if (wsize == W32)
    prDataSize = PrUsage::bind32Size ();
  else
    prDataSize = PrUsage::bind64Size ();

  while (span.length > 0)
    {
      data_prev = data;
      data = new PrUsage ();

      void *dw = dwin->bind (&span, prDataSize);
      if ((dw == NULL) || (prDataSize > span.length))
	{
	  Emsg *m = new Emsg (CMSG_ERROR, GTXT ("Warning: overview data file can't be read"));
	  warnq->append (m);
	  status = FAILURE;
	  delete dwin;
	  return status;
	}

      if (wsize == W32)
	data->bind32 (dw, need_swap_endian);
      else
	data->bind64 (dw, need_swap_endian);
      span.length -= prDataSize;
      span.offset += prDataSize;

      // Skip the first packet
      if (data_prev == NULL)
	continue;
      if (sample_number > samples->size ())
	{ // inconsistent log/overview
	  sample = new Sample (sample_number);
	  char * label = GTXT ("<unknown>");
	  sample->start_label = dbe_strdup (label);
	  sample->end_label = dbe_strdup (label);
	  samples->append (sample);
	}
      else
	sample = samples->fetch (sample_number - 1);
      sample_number++;
      sample->start_time = data_prev->pr_tstamp + 1;
      sample->end_time = data->pr_tstamp;
      sample->prusage = data_prev;

      data_prev->pr_rtime = data->pr_rtime - data_prev->pr_rtime;
      data_prev->pr_utime = data->pr_utime - data_prev->pr_utime;
      data_prev->pr_stime = data->pr_stime - data_prev->pr_stime;
      data_prev->pr_ttime = data->pr_ttime - data_prev->pr_ttime;
      data_prev->pr_tftime = data->pr_tftime - data_prev->pr_tftime;
      data_prev->pr_dftime = data->pr_dftime - data_prev->pr_dftime;
      data_prev->pr_kftime = data->pr_kftime - data_prev->pr_kftime;
      data_prev->pr_ltime = data->pr_ltime - data_prev->pr_ltime;
      data_prev->pr_slptime = data->pr_slptime - data_prev->pr_slptime;
      data_prev->pr_wtime = data->pr_wtime - data_prev->pr_wtime;
      data_prev->pr_stoptime = data->pr_stoptime - data_prev->pr_stoptime;
      data_prev->pr_minf = data->pr_minf - data_prev->pr_minf;
      data_prev->pr_majf = data->pr_majf - data_prev->pr_majf;
      data_prev->pr_nswap = data->pr_nswap - data_prev->pr_nswap;
      data_prev->pr_inblk = data->pr_inblk - data_prev->pr_inblk;
      data_prev->pr_oublk = data->pr_oublk - data_prev->pr_oublk;
      data_prev->pr_msnd = data->pr_msnd - data_prev->pr_msnd;
      data_prev->pr_mrcv = data->pr_mrcv - data_prev->pr_mrcv;
      data_prev->pr_sigs = data->pr_sigs - data_prev->pr_sigs;
      data_prev->pr_vctx = data->pr_vctx - data_prev->pr_vctx;
      data_prev->pr_ictx = data->pr_ictx - data_prev->pr_ictx;
      data_prev->pr_sysc = data->pr_sysc - data_prev->pr_sysc;
      data_prev->pr_ioch = data->pr_ioch - data_prev->pr_ioch;
      sample->get_usage (); // force validation
    }

  for (long smpNum = samples->size (); smpNum >= sample_number; smpNum--)
    {
      // overview file was truncated
      sample = samples->remove (smpNum - 1);
      delete sample;
    }

  if (data)
    {
      // Update last_event so that getEndTime() covers
      // all loadobjects, too.
      update_last_event (data->pr_tstamp);
      delete data;
    }
  delete dwin;
  return SUCCESS;
}

int
Experiment::uidNodeCmp (const void *a, const void *b)
{
  UIDnode *nd1 = *(UIDnode**) a;
  UIDnode *nd2 = *(UIDnode**) b;
  if (nd1->uid == nd2->uid)
    return 0;
  return nd1->uid < nd2->uid ? -1 : 1;
}

static uint64_t
funcAddr (uint32_t val)
{
  if (val == (uint32_t) SP_LEAF_CHECK_MARKER)
    return (uint64_t) SP_LEAF_CHECK_MARKER;
  if (val == (uint32_t) SP_TRUNC_STACK_MARKER)
    return (uint64_t) SP_TRUNC_STACK_MARKER;
  if (val == (uint32_t) SP_FAILED_UNWIND_MARKER)
    return (uint64_t) SP_FAILED_UNWIND_MARKER;
  return val;
}

Experiment::UIDnode *
Experiment::add_uid (Data_window *dwin, uint64_t uid, int size,
		     uint32_t *array, uint64_t link_uid)
{
  if (uid == (uint64_t) 0)
    return NULL;
  uint64_t val = funcAddr (dwin->decode (array[0]));
  UIDnode *node = NULL;
  UIDnode *res = get_uid_node (uid, val);
  UIDnode *next = res;
  for (int i = 0; i < size; i++)
    {
      val = funcAddr (dwin->decode (array[i]));
      if (next == NULL)
	{
	  next = get_uid_node ((uint64_t) 0, val);
	  if (node != NULL)
	    node->next = next;
	}
      node = next;
      next = node->next;
      if (node->val == 0)
	node->val = val;
      else if (node->val != val)   // Algorithmic error (should never happen)
	node->val = (uint64_t) SP_LEAF_CHECK_MARKER;
    }
  if (next == NULL && link_uid != (uint64_t) 0 && node != NULL)
    node->next = get_uid_node (link_uid);
  return res;
}

Experiment::UIDnode *
Experiment::add_uid (Data_window *dwin, uint64_t uid, int size, uint64_t *array, uint64_t link_uid)
{
  if (uid == (uint64_t) 0)
    return NULL;
  UIDnode *node = NULL;
  uint64_t val = dwin->decode (array[0]);
  UIDnode *res = get_uid_node (uid, val);
  UIDnode *next = res;
  for (int i = 0; i < size; i++)
    {
      val = dwin->decode (array[i]);
      if (next == NULL)
	{
	  next = get_uid_node ((uint64_t) 0, val);
	  if (node != NULL)
	    node->next = next;
	}
      node = next;
      next = node->next;
      if (node->val == (uint64_t) 0)
	node->val = val;
      else if (node->val != val)   // Algorithmic error (should never happen)
	node->val = (uint64_t) - 1;
    }
  if (next == NULL && link_uid != (uint64_t) 0 && node != NULL)
    node->next = get_uid_node (link_uid);
  return res;
}

Experiment::UIDnode *
Experiment::new_uid_node (uint64_t uid, uint64_t val)
{
#define NCHUNKSTEP 1024
  if (nnodes >= nchunks * CHUNKSZ)
    {
      // Reallocate Node chunk array
      UIDnode** old_chunks = chunks;
      chunks = new UIDnode*[nchunks + NCHUNKSTEP];
      memcpy (chunks, old_chunks, nchunks * sizeof (UIDnode*));
      nchunks += NCHUNKSTEP;
      delete[] old_chunks;
      // Clean future pointers
      memset (&chunks[nchunks - NCHUNKSTEP], 0, NCHUNKSTEP * sizeof (UIDnode*));
    }

  if (NULL == chunks[nnodes / CHUNKSZ])   // Allocate new chunk for nodes.
    chunks[nnodes / CHUNKSZ] = new UIDnode[CHUNKSZ];
  UIDnode *node = &chunks[nnodes / CHUNKSZ][nnodes % CHUNKSZ];
  node->uid = uid;
  node->val = val;
  node->next = NULL;
  nnodes++;
  return node;
}

Experiment::UIDnode *
Experiment::get_uid_node (uint64_t uid, uint64_t val)
{
  int hash = (((int) uid) >> 4) & (HTableSize - 1);
  if (uid != (uint64_t) 0)
    {
      UIDnode *node = uidHTable[hash];
      if (node && node->uid == uid)
	return node;
    }
  UIDnode *node = new_uid_node (uid, val);
  if (uid != (uint64_t) 0)
    {
      uidHTable[hash] = node;
      uidnodes->append (node);
    }
  return node;
}

Experiment::UIDnode *
Experiment::get_uid_node (uint64_t uid)
{
  if (uid == (uint64_t) 0)
    return NULL;
  int hash = (((int) uid) >> 4) & (HTableSize - 1);
  UIDnode *node = uidHTable[hash];
  if (node && node->uid == uid)
    return node;
  node = new_uid_node (uid, (uint64_t) 0);
  node->next = node;
  return node;
}

Experiment::UIDnode *
Experiment::find_uid_node (uint64_t uid)
{
  int hash = (((int) uid) >> 4) & (HTableSize - 1);
  UIDnode *node = uidHTable[hash];
  if (node && node->uid == uid)
    return node;
  int lt = 0;
  int rt = uidnodes->size () - 1;
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      node = uidnodes->fetch (md);
      if (node->uid < uid)
	lt = md + 1;
      else if (node->uid > uid)
	rt = md - 1;
      else
	{
	  uidHTable[hash] = node;
	  return node;
	}
    }
  return NULL;
}

int
Experiment::frUidCmp (const void *a, const void *b)
{
  RawFramePacket *fp1 = *(RawFramePacket**) a;
  RawFramePacket *fp2 = *(RawFramePacket**) b;
  if (fp1->uid == fp2->uid)
    return 0;
  return fp1->uid < fp2->uid ? -1 : 1;
}

Experiment::RawFramePacket *
Experiment::find_frame_packet (uint64_t uid)
{
  int lt = 0;
  int rt = frmpckts->size () - 1;
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      RawFramePacket *fp = frmpckts->fetch (md);
      if (fp->uid < uid)
	lt = md + 1;
      else if (fp->uid > uid)
	rt = md - 1;
      else
	return fp;
    }

  return NULL;
}

#define FRINFO_CACHEOPT_SIZE_LIMIT  4000000
#define FRINFO_PIPELINE_SIZE_LIMIT  500000
#define FRINFO_PIPELINE_NUM_STAGES  3

// Pipelined execution of resolve_frame_info() and add_stack().
// Since this is the largest time consuming part of loading an experiment (especially
// so for large java experiments) - executing this part as a 3 stage pipeline can
// give significant performance gain - and this concept can be aggressively applied
// to enhance the gain further in future. The three stages are:
// Phase 1:  resolve_frame_info()
// Phase 2:  first part of add_stack() where the native stack is built
// Phase 3:  second part og add_stack() where the java stack is built
// Phase 4:   insert the native and java stacks into the stack map
// The main thread operates in the first Phase and the other stages are
// operated by a ssplib sequential queue - The threads working on the queues run concurrently
// with each other and with the main thread. But within a particular queue, jobs are
// executed sequentially


// This is the second phase of the pipeline of resolve_frame_info and add_stack
//  It works on a chunk of iterations (size CSTCTX_CHUNK_SZ) and invokes add_stack()
// for each one of them

void
Experiment::resolve_frame_info (DataDescriptor *dDscr)
{
  if (!resolveFrameInfo)
    return;
  if (NULL == cstack)
    return;
  dDscr->setResolveFrInfoDone ();

  // Check for TSTAMP
  int propID = dbeSession->getPropIdByName (NTXT ("TSTAMP"));
  Data *dataTStamp = dDscr->getData (propID);
  if (dataTStamp == NULL)
    return;

  propID = dbeSession->getPropIdByName (NTXT ("FRINFO"));
  Data *dataFrinfo = dDscr->getData (propID);

  propID = dbeSession->getPropIdByName (NTXT ("THRID"));
  Data *dataThrId = dDscr->getData (propID);

  // We can get frame info either by FRINFO or by [THRID,STKIDX]
  if (dataFrinfo == NULL)
    return;

  char *propName = NTXT ("MSTACK");
  propID = dbeSession->getPropIdByName (propName);
  PropDescr *prMStack = new PropDescr (propID, propName);
  prMStack->uname = dbe_strdup (GTXT ("Machine Call Stack"));
  prMStack->vtype = TYPE_OBJ;
  dDscr->addProperty (prMStack);

  propName = NTXT ("USTACK");
  propID = dbeSession->getPropIdByName (propName);
  PropDescr *prUStack = new PropDescr (propID, propName);
  prUStack->uname = dbe_strdup (GTXT ("User Call Stack"));
  prUStack->vtype = TYPE_OBJ;
  dDscr->addProperty (prUStack);

  propName = NTXT ("XSTACK");
  propID = dbeSession->getPropIdByName (propName);
  PropDescr *prXStack = new PropDescr (propID, propName);
  prXStack->uname = dbe_strdup (GTXT ("Expert Call Stack"));
  prXStack->vtype = TYPE_OBJ;
  dDscr->addProperty (prXStack);

  propName = NTXT ("HSTACK");
  propID = dbeSession->getPropIdByName (propName);
  PropDescr *prHStack = new PropDescr (propID, propName);
  prHStack->uname = dbe_strdup (GTXT ("ShowHide Call Stack"));
  prHStack->vtype = TYPE_OBJ;
  dDscr->addProperty (prHStack);

  if (has_java)
    {
      propName = NTXT ("JTHREAD");
      propID = dbeSession->getPropIdByName (propName);
      PropDescr *prJThread = new PropDescr (propID, propName);
      prJThread->uname = dbe_strdup (GTXT ("Java Thread"));
      prJThread->vtype = TYPE_OBJ;
      dDscr->addProperty (prJThread);
    }

  if (ompavail)
    {
      PropDescr *prop = new PropDescr (PROP_OMPSTATE, NTXT ("OMPSTATE"));
      prop->uname = dbe_strdup (GTXT ("OpenMP state"));
      prop->vtype = TYPE_UINT32;
      char * stateNames [OMP_LAST_STATE] = OMP_THR_STATE_STRINGS;
      char * stateUNames[OMP_LAST_STATE] = OMP_THR_STATE_USTRINGS;
      for (int ii = 0; ii < OMP_LAST_STATE; ii++)
	prop->addState (ii, stateNames[ii], stateUNames[ii]);
      dDscr->addProperty (prop);

      // add PROP_CPRID to profiling data (not same as omptrace's PROP_CPRID)
      prop = dDscr->getProp (PROP_CPRID);
      if (prop)
	{
	  VType_type type = prop->vtype;
	  assert (type == TYPE_OBJ); //see 7040526
	}
      prop = new PropDescr (PROP_CPRID, NTXT ("CPRID")); //profiling PROP_CPRID
      prop->uname = dbe_strdup (GTXT ("OpenMP parallel region"));
      prop->vtype = TYPE_OBJ;
      dDscr->addProperty (prop);

      // add PROP_TSKID to profiling data (not same as omptrace's PROP_TSKID)
      prop = dDscr->getProp (PROP_TSKID);
      if (prop)
	{
	  VType_type type = prop->vtype;
	  assert (type == TYPE_OBJ); //see 7040526
	}
      prop = new PropDescr (PROP_TSKID, NTXT ("TSKID")); //profiling PROP_TSKID
      prop->uname = dbe_strdup (GTXT ("OpenMP task"));
      prop->vtype = TYPE_OBJ;
      dDscr->addProperty (prop);
    }
  char *progress_bar_msg = dbe_sprintf (NTXT ("%s %s: %s"), NTXT ("  "),
					GTXT ("Processing CallStack Data"),
					get_basename (expt_name));
  int progress_bar_percent = -1;
  long deltaReport = 5000;
  long nextReport = 0;

  long size = dDscr->getSize ();
  //    bool resolve_frinfo_pipelined = size > FRINFO_PIPELINE_SIZE_LIMIT && !ompavail;
  bool resolve_frinfo_pipelined = false;

  Map<uint64_t, uint64_t> *nodeCache = NULL;
  Map<uint64_t, uint64_t> *frameInfoCache = NULL;
  if (size > FRINFO_CACHEOPT_SIZE_LIMIT && dversion == NULL)
    {
      frameInfoCache = new CacheMap<uint64_t, uint64_t>;
      nodeCache = new CacheMap<uint64_t, uint64_t>;
    }

  pushCnt = popCnt = pushCnt3 = popCnt3 = 0;
  nPush = nPop = 0;

  FramePacket *fp = NULL;
  //    DbeThreadPool * threadPool = new DbeThreadPool(5);
  fp = new FramePacket;
  fp->stack = new Vector<Vaddr>;
  fp->jstack = new Vector<Vaddr>;
  fp->ompstack = new Vector<Vaddr>;
  fp->omp_state = 0;
  fp->mpi_state = 0;

  // piggyback on post-processing to calculate exp->last_event
  const hrtime_t _exp_start_time = getStartTime (); // wall clock time
  hrtime_t exp_duration = getLastEvent () == ZERO_TIME ? 0
	  : getLastEvent () - _exp_start_time; // zero-based

  int missed_fi = 0;
  int total_fi = 0;

  for (long i = 0; i < size; i++)
    {
      if (i == nextReport)
	{
	  int percent = (int) (i * 100 / size);
	  if (percent > progress_bar_percent)
	    {
	      progress_bar_percent += 10;
	      theApplication->set_progress (percent, progress_bar_msg);
	    }
	  nextReport += deltaReport;
	}

      uint32_t thrid = (uint32_t) dataThrId->fetchInt (i);
      hrtime_t tstamp = (hrtime_t) dataTStamp->fetchLong (i);

      // piggyback on post-processing to calculate exp->last_event
      {
	hrtime_t relative_timestamp = tstamp - _exp_start_time;
	if (exp_duration < relative_timestamp)
	  exp_duration = relative_timestamp;
      }
      uint64_t frinfo = (uint64_t) dataFrinfo->fetchLong (i);

      RawFramePacket *rfp = NULL;
      if (frinfo)
	{
	  // CacheMap does not work with NULL key
	  if (frameInfoCache != NULL)
	    rfp = (RawFramePacket *) frameInfoCache->get (frinfo);
	}
      if (rfp == 0)
	{
	  rfp = find_frame_packet (frinfo);
	  if (rfp != 0)
	    {
	      if (frameInfoCache != NULL)
		frameInfoCache->put (frinfo, (uint64_t) rfp);
	    }
	  else
	    missed_fi++;
	  total_fi++;
	}

      // Process OpenMP properties
      if (ompavail)
	{
	  fp->omp_state = rfp ? rfp->omp_state : 0;
	  dDscr->setValue (PROP_OMPSTATE, i, fp->omp_state);

	  fp->omp_cprid = mapPRid->get (thrid, tstamp, mapPRid->REL_EQLE);
	  void *omp_preg = mapPReg->get (thrid, tstamp, mapPReg->REL_EQLE);
	  if (!omp_preg)
	    {
	      char *idxname = NTXT ("OMP_preg");
	      int idxtype = dbeSession->findIndexSpaceByName (idxname);
	      if (idxtype != -1)
		{
		  Histable *preg0 = dbeSession->findObjectById (Histable::INDEXOBJ, idxtype, (int64_t) 0);
		  if (preg0)
		    {
		      Vector<Histable*> pregs;
		      pregs.append (preg0);
		      omp_preg = cstack->add_stack (&pregs);
		      mapPReg->put (thrid, tstamp, omp_preg);
		    }
		}
	    }
	  dDscr->setObjValue (PROP_CPRID, i, omp_preg); //profiling PROP_CPRID
	  void *omp_task = mapTask->get (thrid, tstamp, mapTask->REL_EQLE);
	  if (!omp_task)
	    {
	      char *idxname = NTXT ("OMP_task");
	      int idxtype = dbeSession->findIndexSpaceByName (idxname);
	      if (idxtype != -1)
		{
		  Histable *task0 = dbeSession->findObjectById (Histable::INDEXOBJ, idxtype, (int64_t) 0);
		  if (task0)
		    {
		      Vector<Histable*> tasks;
		      tasks.append (task0);
		      omp_task = cstack->add_stack (&tasks);
		      mapTask->put (thrid, tstamp, omp_task);
		    }
		}
	    }
	  dDscr->setObjValue (PROP_TSKID, i, omp_task); //profiling PROP_TSKID
	}
      else
	{
	  fp->omp_state = 0;
	  fp->omp_cprid = 0;
	}

      // Construct the native stack
      fp->stack->reset ();
      Vaddr leafpc = dDscr->getULongValue (PROP_LEAFPC, i);
      if (leafpc)
	fp->stack->append (leafpc);
      UIDnode *node = rfp ? rfp->uidn : NULL;
      while (node)
	{
	  if (node->next == node)
	    // this node contains link_uid
	    node = find_uid_node (node->uid);
	  else
	    {
	      fp->stack->append (node->val);
	      node = node->next;
	    }
	}
      fp->truncated = 0;
      int last = fp->stack->size () - 1;
      if (last >= 0)
	{
	  switch (fp->stack->fetch (last))
	    {
	    case SP_TRUNC_STACK_MARKER:
	      fp->truncated = (Vaddr) SP_TRUNC_STACK_MARKER;
	      fp->stack->remove (last);
	      break;
	    case SP_FAILED_UNWIND_MARKER:
	      fp->truncated = (Vaddr) SP_FAILED_UNWIND_MARKER;
	      fp->stack->remove (last);
	      break;
	    }
	}

      // Construct the Java stack
      fp->jstack->reset ();
      node = rfp ? rfp->uidj : NULL;
      while (node)
	{
	  if (node->next == node)
	    {
	      // this node contains link_uid
	      UIDnode *n = NULL;
	      if (node->uid)
		{
		  // CacheMap does not work with NULL key
		  if (nodeCache != NULL)
		    n = (UIDnode *) nodeCache->get (node->uid);
		}
	      if (n == NULL)
		{
		  n = find_uid_node (node->uid);
		  if (n != NULL)
		    {
		      if (nodeCache != NULL)
			nodeCache->put (node->uid, (uint64_t) n);
		    }
		}
	      node = n;
	    }
	  else
	    {
	      fp->jstack->append (node->val);
	      node = node->next;
	    }
	}
      fp->jtruncated = false;
      last = fp->jstack->size () - 1;
      if (last >= 1 && fp->jstack->fetch (last) == SP_TRUNC_STACK_MARKER)
	{
	  fp->jtruncated = true;
	  fp->jstack->remove (last);
	  fp->jstack->remove (last - 1);
	}

      // Construct the OpenMP stack
      if (ompavail)
	{
	  fp->ompstack->reset ();
	  if (rfp && rfp->omp_uid)
	    {
	      if (leafpc)
		fp->ompstack->append (leafpc);
	      node = rfp->omp_uid;
	      while (node)
		{
		  if (node->next == node)
		    // this node contains link_uid
		    node = find_uid_node (node->uid);
		  else
		    {
		      fp->ompstack->append (node->val);
		      node = node->next;
		    }
		}
	      fp->omptruncated = false;
	      last = fp->ompstack->size () - 1;
	      if (last >= 0)
		{
		  switch (fp->ompstack->fetch (last))
		    {
		    case SP_TRUNC_STACK_MARKER:
		      fp->omptruncated = (Vaddr) SP_TRUNC_STACK_MARKER;
		      fp->ompstack->remove (last);
		      break;
		    case SP_FAILED_UNWIND_MARKER:
		      fp->omptruncated = (Vaddr) SP_FAILED_UNWIND_MARKER;
		      fp->ompstack->remove (last);
		      break;
		    }
		}
	    }
	}
      cstack->add_stack (dDscr, i, fp, NULL);
    }

  // piggyback on post-processing to calculate exp->last_event
  {
    hrtime_t exp_end_time = _exp_start_time + exp_duration;
    update_last_event (exp_end_time);
  }

  if (missed_fi > 0)
    {
      StringBuilder sb;
      sb.sprintf (
		  GTXT ("*** Warning: %d frameinfo packets are missing from total of %d when resolving %s."),
		  missed_fi, total_fi, dDscr->getName ());
      warnq->append (new Emsg (CMSG_WARN, sb));
    }

  //    threadPool->wait_group();
  //    delete threadPool;
  theApplication->set_progress (0, NTXT (""));
  free (progress_bar_msg);
  if (!resolve_frinfo_pipelined && fp != NULL)
    {
      delete fp->ompstack;
      delete fp->jstack;
      delete fp->stack;
      delete fp;
    }
  delete frameInfoCache;
  frameInfoCache = NULL;
  delete nodeCache;
  nodeCache = NULL;
}

void
Experiment::post_process ()
{
  // update non_paused_time after final update to "last_event"
  if (resume_ts != MAX_TIME && last_event)
    {
      hrtime_t ts = last_event - exp_start_time;
      hrtime_t delta = ts - resume_ts;
      non_paused_time += delta;
      resume_ts = MAX_TIME; // collection is paused
    }

  // GC: prune events outside of experiment duration, calculate GC duration, update indices
  int index;
  GCEvent * gcevent;
  gc_duration = ZERO_TIME;
  if (gcevents != NULL)
    {
      // delete events that finish before exp_start_time or start after last_event
      for (int ii = 0; ii < gcevents->size ();)
	{
	  gcevent = gcevents->fetch (ii);
	  if (gcevent->end - exp_start_time < 0
	      || last_event - gcevent->start < 0)
	    delete gcevents->remove (ii);
	  else
	    ii++;
	}
    }
  Vec_loop (GCEvent*, gcevents, index, gcevent)
  {
    gcevent->id = index + 1; // renumber to account for any deleted events
    if (gcevent->start - exp_start_time < 0 || gcevent->start == ZERO_TIME)
      // truncate events that start before experiment start
      gcevent->start = exp_start_time;
    if (last_event - gcevent->end < 0)
      // truncate events that end after experiment end
      gcevent->end = last_event;
    gc_duration += gcevent->end - gcevent->start;
  }
}

Experiment::Exp_status
Experiment::find_expdir (char *path)
{
  // This function checks that the experiment directory
  // is of the proper form, and accessible
  struct stat64 sbuf;

  // Save the name
  expt_name = dbe_strdup (path);

  // Check that the name ends in .er
  size_t i = strlen (path);
  if (i > 0 && path[i - 1] == '/')
    path[--i] = '\0';

  if (i < 4 || strcmp (&path[i - 3], NTXT (".er")) != 0)
    {
      Emsg *m = new Emsg (CMSG_FATAL,
			  GTXT ("*** Error: not a valid experiment name"));
      errorq->append (m);
      status = FAILURE;
      return FAILURE;
    }

  // Check if new directory structure (i.e., no pointer file)
  if (dbe_stat (path, &sbuf))
    {
      Emsg *m = new Emsg (CMSG_FATAL, GTXT ("*** Error: experiment not found"));
      errorq->append (m);
      status = FAILURE;
      return FAILURE;
    }
  if (S_ISDIR (sbuf.st_mode) == 0)
    {
      // ignore pointer-file experiments
      Emsg *m = new Emsg (CMSG_FATAL,
			  GTXT ("*** Error: experiment was recorded with an earlier version, and can not be read"));
      errorq->append (m);
      obsolete = 1;
      status = FAILURE;
      return FAILURE;
    }
  return SUCCESS;
}

void
Experiment::purge ()
{
  // This routine will purge all of the caches of releasable storage.
  for (int i = 0; i < dataDscrs->size (); ++i)
    {
      DataDescriptor *dataDscr = dataDscrs->fetch (i);
      if (dataDscr == NULL)
	continue;
      dataDscr->reset ();
    }
  delete cstack;
  delete cstackShowHide;
  cstack = CallStack::getInstance (this);
  cstackShowHide = CallStack::getInstance (this);
}

void
Experiment::resetShowHideStack ()
{
  delete cstackShowHide;
  cstackShowHide = CallStack::getInstance (this);
}

#define GET_INT_VAL(v, s, len) \
    for (v = len = 0; isdigit(*s); s++, len++) { v = v * 10 + (*s -'0'); }

static int
dir_name_cmp (const void *a, const void *b)
{
  char *s1 = *((char **) a);
  char *s2 = *((char **) b);
  while (*s1)
    {
      if (isdigit (*s1) && isdigit (*s2))
	{
	  int v1, v2, len1, len2;
	  GET_INT_VAL (v1, s1, len1);
	  GET_INT_VAL (v2, s2, len2);
	  if (v1 != v2)
	    return v1 - v2;
	  if (len1 != len2)
	    return len2 - len1;
	  continue;
	}
      if (*s1 != *s2)
	break;
      s1++;
      s2++;
    }
  return *s1 - *s2;
}

Vector<char*> *
Experiment::get_descendants_names ()
{
  char *dir_name = get_expt_name ();
  if (dir_name == NULL)
    return NULL;
  DIR *exp_dir = opendir (dir_name);
  if (exp_dir == NULL)
    return NULL;
  Vector<char*> *exp_names = new Vector<char*>();
  for (struct dirent *entry = readdir (exp_dir); entry;
	  entry = readdir (exp_dir))
    {
      if (entry->d_name[0] == '_' || strncmp (entry->d_name, "M_r", 3) == 0)
	{
	  char *dpath = dbe_sprintf (NTXT ("%s/%s"), dir_name, entry->d_name);
	  struct stat64 sbuf;
	  if (dbe_stat (dpath, &sbuf) == 0 && S_ISDIR (sbuf.st_mode))
	    exp_names->append (dpath);
	  else
	    free (dpath);
	}
    }
  closedir (exp_dir);
  if (exp_names->size () == 0)
    {
      delete exp_names;
      return NULL;
    }
  exp_names->sort (dir_name_cmp);
  return exp_names;
}

bool
Experiment::create_dir (char *dname)
{
  if (mkdir (dname, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0)
    {
      return true;
    }
  struct stat64 sbuf;
  if (dbe_stat (dname, &sbuf) != 0 || S_ISDIR (sbuf.st_mode) == 0)
    {
      char *buf = dbe_sprintf (GTXT ("Unable to create directory `%s'\n"),
			       dname);
      errorq->append (new Emsg (CMSG_ERROR, buf));
      free (buf);
      return false;
    }
  return true;
}

char *
Experiment::get_arch_name ()
{
  if (arch_name == NULL)
    {
      // Determine the master experiment directory.
      // omazur: should do it in a less hacky way. XXXX
      char *ptr = strstr_r (expt_name, DESCENDANT_EXPT_KEY);
      ptr = ptr ? ptr + 3 : expt_name + strlen (expt_name);
      arch_name = dbe_sprintf (NTXT ("%.*s/%s"), (int) (ptr - expt_name),
			       expt_name, SP_ARCHIVES_DIR);
    }
  return arch_name;
}

char *
Experiment::get_fndr_arch_name ()
{
  if (fndr_arch_name == NULL)
    // Determine the founder experiment directory.
    fndr_arch_name = dbe_strdup (get_arch_name ());
  return fndr_arch_name;
}

enum
{
  HASH_NAME_LEN     = 11    // (64 / 6 + 1) = 11
};

static char *
get_hash_string (char buf[HASH_NAME_LEN + 1], uint64_t hash)
{
  static const char *har =
	  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
  for (int i = 0; i < HASH_NAME_LEN; i++)
    {
      buf[i] = har[hash & 0x3f];
      hash = hash >> 6;
    }
  buf[HASH_NAME_LEN] = 0;
  return buf;
}

char *
Experiment::getNameInArchive (const char *fname, bool archiveFile)
{
  char *aname = get_archived_name (fname, archiveFile);
  char *ret = dbe_sprintf (NTXT ("%s/%s"), get_arch_name (), aname);
  free (aname);
  return ret;
}

#define MAX_ARCHIVE_FILENAME_LEN    (256 - HASH_NAME_LEN - 2)

char *
Experiment::get_archived_name (const char *fname, bool archiveFile)
{
  char *bname = get_basename (fname);

  // dirname_hash:
  char dirnameHash[HASH_NAME_LEN + 1];
  // Treat "a.out" and "./a.out" equally
  uint64_t hash = bname != fname ? crc64 (fname, bname - fname)
				 : crc64 (NTXT ("./"), 2);
  get_hash_string (dirnameHash, hash);

  char *ret;
  long bname_len = dbe_sstrlen (bname);
  if (bname_len > MAX_ARCHIVE_FILENAME_LEN)
    {
      char basenameHash[HASH_NAME_LEN + 1];
      hash = crc64 (bname, bname_len);
      get_hash_string (basenameHash, hash);
      ret = dbe_sprintf ("%.*s%c%s_%s",
			 MAX_ARCHIVE_FILENAME_LEN - HASH_NAME_LEN - 1,
			 bname, archiveFile ? '.' : '_',
			 dirnameHash, basenameHash);
    }
  else
    ret = dbe_sprintf ("%s%c%s", bname, archiveFile ? '.' : '_', dirnameHash);
  return ret;
}

char *
Experiment::checkFileInArchive (const char *fname, bool archiveFile)
{
  if (archiveMap)
    {
      char *aname = get_archived_name (fname, archiveFile);
      DbeFile *df = archiveMap->get (aname);
      free (aname);
      if (df)
	return strdup (df->get_location ());
      return NULL;
    }
  if (founder_exp)
    return founder_exp->checkFileInArchive (fname, archiveFile);
  return NULL;
}


// Comparing SegMem

static int
SegMemCmp (const void *a, const void *b)
{
  SegMem *item1 = *((SegMem **) a);
  SegMem *item2 = *((SegMem **) b);
  return item1->unload_time > item2->unload_time ? 1 :
	 item1->unload_time == item2->unload_time ? 0 : -1;
}

SegMem*
Experiment::update_ts_in_maps (Vaddr addr, hrtime_t ts)
{
  Vector<SegMem *> *segMems = (Vector<SegMem *> *) maps->values ();
  if (!segMems->is_sorted ())
    {
      Dprintf (DEBUG_MAPS, NTXT ("update_ts_in_maps: segMems.size=%lld\n"), (long long) segMems->size ());
      segMems->sort (SegMemCmp);
    }
  for (int i = 0, sz = segMems ? segMems->size () : 0; i < sz; i++)
    {
      SegMem *sm = segMems->fetch (i);
      if (ts < sm->unload_time)
	{
	  for (; i < sz; i++)
	    {
	      sm = segMems->fetch (i);
	      if ((addr >= sm->base) && (addr < sm->base + sm->size))
		{
		  Dprintf (DEBUG_MAPS,
			   "update_ts_in_maps: old:%u.%09u -> %u.%09u addr=0x%08llx size=%lld\n",
			   (unsigned) (sm->load_time / NANOSEC),
			   (unsigned) (sm->load_time % NANOSEC),
			   (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
			   (unsigned long long) sm->base, (long long) sm->size);
		  maps->remove (sm->base, sm->load_time);
		  sm->load_time = ts;
		  maps->insert (sm->base, ts, sm);
		  return sm;
		}
	    }
	}
    }
  Dprintf (DEBUG_MAPS, "update_ts_in_maps: NOT FOUND %u.%09u addr=0x%08llx\n",
	   (unsigned) (ts / NANOSEC), (unsigned) (ts % NANOSEC),
	   (unsigned long long) addr);
  return NULL;
}

DbeInstr*
Experiment::map_Vaddr_to_PC (Vaddr addr, hrtime_t ts)
{
  // Look up in the hash table first
  int hash = (((int) addr) >> 8) & (HTableSize - 1);
  SegMem *si = smemHTable[hash];
  if (si == NULL || addr < si->base || addr >= si->base + si->size
      || ts < si->load_time || ts >= si->unload_time)
    {
      // Not in the hash table
      si = (SegMem*) maps->locate (addr, ts);
      if (si == NULL || addr < si->base || addr >= si->base + si->size
	  || ts < si->load_time || ts >= si->unload_time)
	{
	  si = update_ts_in_maps (addr, ts);
	  if (si == NULL)
	    return dbeSession->get_Unknown_Function ()->find_dbeinstr (PCInvlFlag, addr);
	}
      smemHTable[hash] = si;
    }

  // Calculate the file offset of 'addr'
  uint64_t f_offset = si->get_file_offset () + (addr - si->base);

  DbeInstr *instr;
  if (si->obj->get_type () == Histable::LOADOBJECT)
    {
      LoadObject *lo = (LoadObject*) si->obj;
      lo->sync_read_stabs ();
      instr = lo->find_dbeinstr (f_offset);
    }
  else
    {
      int hash2 = ((((int) addr) & 0xFFFC00) | (((int) f_offset) >> 2))
	      & (HTableSize - 1);
      instr = instHTable[hash2];
      if (instr == NULL || instr->func != si->obj || instr->addr != f_offset)
	{
	  // Not in the hash table
	  Function *fp = (Function *) si->obj;
	  instr = fp->find_dbeinstr (0, f_offset);
	  instHTable[hash2] = instr;
	}
    }
  if (!instr->func->isUsed)
    {
      instr->func->isUsed = true;
      instr->func->module->isUsed = true;
      instr->func->module->loadobject->isUsed = true;
    }
  return instr;
}

Sample *
Experiment::map_event_to_Sample (hrtime_t ts)
{
  Sample *sample;
  int index;

  // Check if the last used sample is the right one,
  // if not then find it.
  if (sample_last_used && ts >= sample_last_used->start_time
      && ts <= sample_last_used->end_time)
    return sample_last_used;

  Vec_loop (Sample*, samples, index, sample)
  {
    if ((ts >= sample->start_time) &&
	(ts <= sample->end_time))
      {
	sample_last_used = sample;
	return sample;
      }
  }
  return (Sample*) NULL;
}

GCEvent *
Experiment::map_event_to_GCEvent (hrtime_t ts)
{
  GCEvent *gcevent;
  int index;

  // Check if the last used sample is the right one,
  // if not then find it.
  if (gcevent_last_used && ts >= gcevent_last_used->start
      && ts <= gcevent_last_used->end)
    return gcevent_last_used;
  Vec_loop (GCEvent*, gcevents, index, gcevent)
  {
    if ((ts >= gcevent->start) &&
	(ts <= gcevent->end))
      {
	gcevent_last_used = gcevent;
	return gcevent;
      }
  }
  return (GCEvent*) NULL;
}

DbeInstr*
Experiment::map_jmid_to_PC (Vaddr mid, int bci, hrtime_t ts)
{
  if (mid == 0 || jmaps == NULL)
    // special case: no Java stack was recorded, bci - error code
    return dbeSession->get_JUnknown_Function ()->find_dbeinstr (0, bci);

  JMethod *jmthd = jmidHTable->get (mid);
  if (jmthd == NULL)
    {
      jmthd = (JMethod *) jmaps->locate_exact_match (mid, ts);
      if (jmthd)
	jmidHTable->put (mid, jmthd);
    }
  if (jmthd == NULL || jmthd->get_type () != Histable::FUNCTION)
    return dbeSession->get_JUnknown_Function ()->find_dbeinstr (0, (uint64_t) mid);
  return jmthd->find_dbeinstr (0, bci);
}

Emsg *
Experiment::fetch_comments ()
{
  return commentq->fetch ();
}

Emsg *
Experiment::fetch_runlogq ()
{
  return runlogq->fetch ();
}

Emsg *
Experiment::fetch_errors ()
{
  return errorq->fetch ();
}

Emsg *
Experiment::fetch_warnings ()
{
  return warnq->fetch ();
}

Emsg *
Experiment::fetch_notes ()
{
  return notesq->fetch ();
}

Emsg *
Experiment::fetch_ifreq ()
{
  return ifreqq->fetch ();
}

Emsg *
Experiment::fetch_pprocq ()
{
  return pprocq->fetch ();
}

int
Experiment::read_dyntext_file ()
{
  dyntext_name = dbe_sprintf ("%s/%s", expt_name, SP_DYNTEXT_FILE);
  Data_window *dwin = new Data_window (dyntext_name);
  if (dwin->not_opened ())
    {
      delete dwin;
      return 1;
    }
  dwin->need_swap_endian = need_swap_endian;

  Function *fp = NULL;
  char *progress_msg = NULL; // Message for the progress bar
  for (int64_t offset = 0;;)
    {
      DT_common *cpckt = (DT_common *) dwin->bind (offset, sizeof (DT_common));
      if (cpckt == NULL)
	break;
      size_t cpcktsize = dwin->decode (cpckt->size);
      cpckt = (DT_common *) dwin->bind (offset, cpcktsize);
      if (cpckt == NULL)
	break;
      switch (dwin->decode (cpckt->type))
	{
	case DT_HEADER:
	  {
	    DT_header *hdr = (DT_header*) cpckt;
	    hrtime_t ts = dwin->decode (hdr->time) + exp_start_time;
	    SegMem *si = (SegMem*) maps->locate (dwin->decode (hdr->vaddr), ts);
	    fp = si ? (Function *) si->obj : NULL;
	    if (fp && (fp->get_type () != Histable::FUNCTION
		       || !(fp->flags & FUNC_FLAG_DYNAMIC)))
	      fp = NULL;
	    break;
	  }
	case DT_CODE:
	  if (fp)
	    {
	      fp->img_fname = dyntext_name;
	      fp->img_offset = offset + sizeof (DT_common);
	      if ((platform != Intel) && (platform != Amd64))
		{ //ARCH(SPARC)
		  // Find out 'save' instruction address for SPARC
		  char *ptr = ((char*) cpckt) + sizeof (DT_common);
		  size_t img_size = cpcktsize - sizeof (DT_common);
		  for (size_t i = 0; i < img_size; i += 4)
		    if (ptr[i] == (char) 0x9d && ptr[i + 1] == (char) 0xe3)
		      {
			fp->save_addr = i;
			break;
		      }
		}
	    }
	  break;
	case DT_SRCFILE:
	  if (fp)
	    {
	      char *srcname = dbe_strndup (((char*) cpckt) + sizeof (DT_common),
					   cpcktsize - sizeof (DT_common));
	      LoadObject *ds = fp->module ? fp->module->loadobject : NULL;
	      assert (ds != NULL);
	      Module *mod = dbeSession->createModule (ds, NULL);
	      mod->set_file_name (srcname);
	      //}
	      if (fp->module)
		{
		  // It's most likely (unknown). Remove fp from it.
		  long idx = fp->module->functions->find (fp);
		  if (idx >= 0)
		    fp->module->functions->remove (idx);
		}
	      fp->module = mod;
	      mod->functions->append (fp);
	    }
	  break;
	case DT_LTABLE:
	  if (fp)
	    {
	      DT_lineno *ltab = (DT_lineno*) ((char*) cpckt + sizeof (DT_common));
	      size_t sz = (cpcktsize - sizeof (DT_common)) / sizeof (DT_lineno);
	      if (sz <= 0)
		break;
	      // Take care of the progress bar
	      static int percent = 0;
	      static long deltaReport = sz / 100; // 1000;
	      static long nextReport = 0;
	      static long progress_count = 0;
	      fp->pushSrcFile (fp->getDefSrc (), 0);
	      for (size_t i = 0; i < sz; i++)
		{
		  int lineno = dwin->decode (ltab[i].lineno);
		  if (fp->usrfunc != NULL)
		    {
		      // Update progress bar
		      if (dbeSession->is_interactive ())
			{
			  if (progress_count == nextReport)
			    {
			      if (percent < 99)
				{
				  percent++;
				  if (NULL == progress_msg)
				    {
				      progress_msg = dbe_sprintf (GTXT ("Processing Dynamic Text: %s"),
								  get_basename (expt_name));
				    }
				  theApplication->set_progress (percent, progress_msg);
				  nextReport += deltaReport;
				}
			    }
			  progress_count++;
			}
		      DbeLine *dbeline = fp->usrfunc->mapPCtoLine (lineno, NULL);
		      lineno = dbeline != NULL ? dbeline->lineno : -1;
		    }
		  fp->add_PC_info (dwin->decode (ltab[i].offset), lineno);
		}
	      fp->popSrcFile ();
	    }
	  break;
	default:
	  // skip unknown records
	  break;
	}
      offset += cpcktsize;
    }
  free (progress_msg);
  delete dwin;
  return 0;
}

uint32_t
Experiment::mapTagValue (Prop_type prop, uint64_t value)
{
  Vector<Histable*> *objs = tagObjs->fetch (prop);
  int lt = 0;
  int rt = objs->size () - 1;
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      Other *obj = (Other*) objs->fetch (md);
      if (obj->value64 < value)
	lt = md + 1;
      else if (obj->value64 > value)
	rt = md - 1;
      else
	return obj->tag;
    }

  uint32_t tag;
  if (sparse_threads && (prop == PROP_THRID || prop == PROP_LWPID))
    tag = objs->size () + 1; // "+ 1" related to 7038295
  else
    tag = (int) value; // truncation; See 6788767

  Other *obj = new Other ();
  obj->value64 = value;
  obj->tag = tag;
  if (lt == objs->size ())
    objs->append (obj);
  else
    objs->insert (lt, obj);

  // Update min and max tags
  if (prop == PROP_LWPID)
    {
      if ((uint64_t) tag < min_lwp)
	min_lwp = (uint64_t) tag;
      if ((uint64_t) tag > max_lwp)
	max_lwp = (uint64_t) tag;
      lwp_cnt++;
    }
  else if (prop == PROP_THRID)
    {
      if ((uint64_t) tag < min_thread)
	min_thread = (uint64_t) tag;
      if ((uint64_t) tag > max_thread)
	max_thread = (uint64_t) tag;
      thread_cnt++;
    }
  else if (prop == PROP_CPUID)
    {
      // On Solaris 8, we don't get CPU id -- don't change
      if (value != (uint64_t) - 1)
	{//YXXX is this related only to solaris 8?
	  if ((uint64_t) tag < min_cpu)
	    min_cpu = (uint64_t) tag;
	  if ((uint64_t) tag > max_cpu)
	    max_cpu = (uint64_t) tag;
	}
      cpu_cnt++;
    }
  return obj->tag;
}

Vector<Histable*> *
Experiment::getTagObjs (Prop_type prop)
{
  return tagObjs->fetch (prop);
}

Histable *
Experiment::getTagObj (Prop_type prop, uint32_t tag)
{
  Vector<Histable*> *objs = tagObjs->fetch (prop);
  if (objs == NULL)
    return NULL;
  for (int i = 0; i < objs->size (); i++)
    {
      Other *obj = (Other*) objs->fetch (i);
      if (obj->tag == tag)
	return obj;
    }
  return NULL;
}

JThread *
Experiment::map_pckt_to_Jthread (uint32_t tid, hrtime_t tstamp)
{
  if (!has_java)
    return JTHREAD_DEFAULT;
  int lt = 0;
  int rt = jthreads_idx->size () - 1;
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      JThread *jthread = jthreads_idx->fetch (md);
      if (jthread->tid < tid)
	lt = md + 1;
      else if (jthread->tid > tid)
	rt = md - 1;
      else
	{
	  for (; jthread; jthread = jthread->next)
	    if (tstamp >= jthread->start && tstamp < jthread->end)
	      return jthread;
	  break;
	}
    }

  return JTHREAD_NONE;
}

JThread*
Experiment::get_jthread (uint32_t tid)
{
  if (!has_java)
    return JTHREAD_DEFAULT;
  int lt = 0;
  int rt = jthreads_idx->size () - 1;
  while (lt <= rt)
    {
      int md = (lt + rt) / 2;
      JThread *jthread = jthreads_idx->fetch (md);
      if (jthread->tid < tid)
	lt = md + 1;
      else if (jthread->tid > tid)
	rt = md - 1;
      else
	{
	  JThread *jthread_first = jthread;
	  while ((jthread = jthread->next) != NULL)
	    if (!jthread->is_system () &&
		jthread->jthr_id < jthread_first->jthr_id)
	      jthread_first = jthread;
	  return jthread_first;
	}
    }

  return JTHREAD_NONE;
}

// SS12 experiment
DataDescriptor *
Experiment::newDataDescriptor (int data_id, int flags,
			       DataDescriptor *master_dDscr)
{
  DataDescriptor *dataDscr = NULL;
  if (data_id >= 0 && data_id < dataDscrs->size ())
    {
      dataDscr = dataDscrs->fetch (data_id);
      if (dataDscr != NULL)
	return dataDscr;
    }

  assert (data_id >= 0 && data_id < DATA_LAST);
  const char *nm = get_prof_data_type_name (data_id);
  const char *uname = get_prof_data_type_uname (data_id);

  if (master_dDscr)
    dataDscr = new DataDescriptor (data_id, nm, uname, master_dDscr);
  else
    dataDscr = new DataDescriptor (data_id, nm, uname, flags);
  dataDscrs->store (data_id, dataDscr);
  return dataDscr;
}

Vector<DataDescriptor*> *
Experiment::getDataDescriptors ()
{
  Vector<DataDescriptor*> *result = new Vector<DataDescriptor*>;
  for (int i = 0; i < dataDscrs->size (); ++i)
    {
      DataDescriptor *dd;
      dd = get_raw_events (i); // force data fetch
      if (dd != NULL)
	result->append (dd);
    }
  return result;
}

DataDescriptor *
Experiment::getDataDescriptor (int data_id)
{
  if (data_id < 0 || data_id >= dataDscrs->size ())
    return NULL;
  return dataDscrs->fetch (data_id);
}

PacketDescriptor *
Experiment::newPacketDescriptor (int kind, DataDescriptor *dDscr)
{
  PacketDescriptor *pDscr = new PacketDescriptor (dDscr);
  pcktDscrs->store (kind, pDscr);
  return pDscr;
}

PacketDescriptor *
Experiment::getPacketDescriptor (int kind)
{
  if (kind < 0 || kind >= pcktDscrs->size ())
    return NULL;
  return pcktDscrs->fetch (kind);
}

void
Experiment::set_clock (int clk)
{
  if (clk > 0)
    {
      if (maxclock < clk)
	{
	  maxclock = clk;
	  clock = maxclock;
	}
      if (minclock == 0 || minclock > clk)
	minclock = clk;
    }
}

bool
JThread::is_system ()
{
  if (group_name == NULL)
    return false;
  return strcmp (group_name, NTXT ("system")) == 0;
}

void
Experiment::dump_stacks (FILE *outfile)
{
  cstack->print (outfile);
}

void
Experiment::dump_map (FILE *outfile)
{
  int index;
  SegMem *s;
  fprintf (outfile, GTXT ("Experiment %s\n"), get_expt_name ());
  fprintf (outfile, GTXT ("Address         Size (hex)              Load time     Unload time    Checksum  Name\n"));
  Vec_loop (SegMem*, seg_items, index, s)
  {
    timestruc_t load;
    timestruc_t unload;
    hr2timestruc (&load, (s->load_time - exp_start_time));
    if (load.tv_nsec < 0)
      {
	load.tv_sec--;
	load.tv_nsec += NANOSEC;
      }
    if (s->unload_time == MAX_TIME)
      {
	unload.tv_sec = 0;
	unload.tv_nsec = 0;
      }
    else
      hr2timestruc (&unload, (s->unload_time - exp_start_time));
    if (load.tv_nsec < 0)
      {
	load.tv_sec--;
	load.tv_nsec += NANOSEC;
      }
    fprintf (outfile,
	     "0x%08llx  %8lld (0x%08llx) %5lld.%09lld %5lld.%09lld  \"%s\"\n",
	     (long long) s->base, (long long) s->size, (long long) s->size,
	     (long long) load.tv_sec, (long long) load.tv_nsec,
	     (long long) unload.tv_sec, (long long) unload.tv_nsec,
	     s->obj->get_name ());
  }
  fprintf (outfile, NTXT ("\n"));
}

/**
 * Copy file to archive
 * @param name
 * @param aname
 * @param hide_msg
 * @return 0 - success, 1 - error
 */
int
Experiment::copy_file_to_archive (const char *name, const char *aname, int hide_msg)
{
  errno = 0;
  int fd_w = ::open64 (aname, O_WRONLY | O_CREAT | O_EXCL, 0644);
  if (fd_w == -1)
    {
      if (errno == EEXIST)
	return 0;
      fprintf (stderr, GTXT ("er_archive: unable to copy `%s': %s\n"),
	       name, STR (strerror (errno)));
      return 1;
    }

  if (dbe_stat_file (name, NULL) != 0)
    {
      fprintf (stderr, GTXT ("er_archive: cannot access file `%s': %s\n"),
	       name, STR (strerror (errno)));
      close (fd_w);
      return 1;
    }

  int fd_r = ::open64 (name, O_RDONLY);
  if (fd_r == -1)
    {
      fprintf (stderr, GTXT ("er_archive: unable to open `%s': %s\n"),
	       name, strerror (errno));
      close (fd_w);
      unlink (aname);
      return 1;
    }

  if (!hide_msg)
    fprintf (stderr, GTXT ("Copying `%s'  to `%s'\n"), name, aname);
  bool do_unlink = false;
  for (;;)
    {
      unsigned char buf[65536];
      int n, n1;
      n = (int) read (fd_r, (void *) buf, sizeof (buf));
      if (n <= 0)
	break;
      n1 = (int) write (fd_w, buf, n);
      if (n != n1)
	{
	  fprintf (stderr, GTXT ("er_archive: unable to write %d bytes to `%s': %s\n"),
		   n, aname, STR (strerror (errno)));
	  do_unlink = true;
	  break;
	}
    }
  close (fd_w);

  struct stat64 s_buf;
  if (fstat64 (fd_r, &s_buf) == 0)
    {
      struct utimbuf u_buf;
      u_buf.actime = s_buf.st_atime;
      u_buf.modtime = s_buf.st_mtime;
      utime (aname, &u_buf);
    }
  close (fd_r);
  if (do_unlink)
    {
      if (!hide_msg)
	fprintf (stderr, GTXT ("er_archive: remove %s\n"), aname);
      unlink (aname);
      return 1;
    }
  return 0;
}

/**
 * Copy file to common archive
 * Algorithm:
 * Calculate checksum
 * Generate file name to be created in common archive
 * Check if it is not in common archive yet
 * Copy file to the common archive directory if it is not there yet
 * Create symbolic link: "aname" -> "caname", where "caname" is the name in common archive
 * @param name - original file name
 * @param aname - file name in experiment archive
 * @param common_archive - common archive directory
 * @return 0 - success, 1 - error
 */
int
Experiment::copy_file_to_common_archive (const char *name, const char *aname,
					 int hide_msg,
					 const char *common_archive,
					 int relative_path)
{
  if (!name || !aname || !common_archive)
    {
      if (!name)
	fprintf (stderr, GTXT ("er_archive: Internal error: file name is NULL\n"));
      if (!aname)
	fprintf (stderr, GTXT ("er_archive: Internal error: file name in archive is NULL\n"));
      if (!common_archive)
	fprintf (stderr, GTXT ("er_archive: Internal error: path to common archive is NULL\n"));
      return 1;
    }
  // Check if file is already archived
  if (dbe_stat (aname, NULL) == 0)
    return 0; // File is already archived
  // Generate full path to common archive directory
  char *cad = NULL;
  char *abs_aname = NULL;
  if ((common_archive[0] != '/') || (aname[0] != '/'))
    {
      long size = pathconf (NTXT ("."), _PC_PATH_MAX);
      if (size < 0)
	{
	  fprintf (stderr, GTXT ("er_archive: Fatal error: pathconf(\".\", _PC_PATH_MAX) failed\n"));
	  return 1;
	}
      char *buf = (char *) malloc ((size_t) size);
      if (buf == NULL)
	{
	  fprintf (stderr, GTXT ("er_archive: Fatal error: unable to allocate memory\n"));
	  return 1;
	}
      char *ptr = getcwd (buf, (size_t) size);
      if (ptr == NULL)
	{
	  fprintf (stderr, GTXT ("er_archive: Fatal error: cannot determine current directory\n"));
	  free (buf);
	  return 1;
	}
      if (common_archive[0] != '/')
	cad = dbe_sprintf (NTXT ("%s/%s"), ptr, common_archive);
      else
	cad = dbe_strdup (common_archive);
      if (aname[0] != '/')
	abs_aname = dbe_sprintf (NTXT ("%s/%s"), ptr, aname);
      else
	abs_aname = dbe_strdup (aname);
      free (buf);
    }
  else
    {
      cad = dbe_strdup (common_archive);
      abs_aname = dbe_strdup (aname);
    }
  // Calculate checksum
  char * errmsg = NULL;
  uint32_t crcval = get_cksum (name, &errmsg);
  if (0 == crcval)
    { // error
      free (cad);
      free (abs_aname);
      if (NULL != errmsg)
	{
	  fprintf (stderr, GTXT ("er_archive: Fatal error: %s\n"), errmsg);
	  free (errmsg);
	  return 1;
	}
      fprintf (stderr,
	       GTXT ("er_archive: Fatal error: get_cksum(%s) returned %d\n"),
	       name, crcval);
      return 1;
    }
  // Generate file name to be created in common archive
  char *fname = get_basename (name);
  char *abs_caname = dbe_sprintf (NTXT ("%s/%u_%s"), cad, crcval, fname);
  if (abs_caname == NULL)
    {
      free (cad);
      free (abs_aname);
      fprintf (stderr,
	       GTXT ("er_archive: Fatal error: unable to allocate memory\n"));
      return 1;
    }
  // Check if full name is not too long
  long len = dbe_sstrlen (abs_caname);
  long max = pathconf (cad, _PC_PATH_MAX);
  if ((max < 0) || (len <= 0))
    { // unknown error
      fprintf (stderr, GTXT ("er_archive: Fatal error: pathconf(%s, _PC_PATH_MAX) failed\n"),
	       cad);
      free (abs_caname);
      free (cad);
      free (abs_aname);
      return 1;
    }
  if (len >= max)
    {
      // Try to truncate the name
      if ((len - max) <= dbe_sstrlen (fname))
	{
	  // Yes, we can do it
	  abs_caname[max - 1] = 0;
	  if (!hide_msg)
	    fprintf (stderr, GTXT ("er_archive: file path is too long - truncated:%s\n"),
		     abs_caname);
	}
    }
  // Check if file name is not too long
  char *cafname = get_basename (abs_caname);
  len = dbe_sstrlen (cafname);
  max = pathconf (cad, _PC_NAME_MAX);
  if ((max < 0) || (len <= 0))
    { // unknown error
      fprintf (stderr, GTXT ("er_archive: Fatal error: pathconf(%s, _PC_NAME_MAX) failed\n"),
	       cad);
      free (abs_caname);
      free (cad);
      free (abs_aname);
      return 1;
    }
  if (len >= max)
    {
      // Try to truncate the name
      if ((len - max) <= dbe_sstrlen (fname))
	{
	  // Yes, we can do it
	  cafname[max - 1] = 0;
	  if (!hide_msg)
	    fprintf (stderr, GTXT ("er_archive: file name is too long - truncated:%s\n"),
		     abs_caname);
	}
    }
  // Copy file to the common archive directory if it is not there yet
  int res = 0;
  if (dbe_stat_file (abs_caname, NULL) != 0)
    {
      // Use temporary file to avoid synchronization problems
      char *t = dbe_sprintf ("%s/archive_%llx", cad, (unsigned long long) gethrtime());
      free (cad);
      // Copy file to temporary file
      res = copy_file_to_archive (name, t, hide_msg); // hide messages
      if (res != 0)
	{
	  fprintf (stderr, GTXT ("er_archive: Fatal error: cannot copy file %s to temporary file: %s\n"),
		   name, t);
	  unlink (t);
	  free (t);
	  free (abs_caname);
	  free (abs_aname);
	  return 1;
	}
      // Set read-only permissions
      struct stat64 statbuf;
      if (0 == dbe_stat_file (name, &statbuf))
	{
	  mode_t mask = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
	  mode_t mode = statbuf.st_mode & mask;
	  chmod (t, mode);
	}
      // Try to rename temporary file "t" to "abs_caname"
      // res = link(t, abs_caname); // link() fails on some f/s - use rename()
      res = rename (t, abs_caname);
      if (res != 0)
	{
	  if (errno != EEXIST)
	    {
	      fprintf (stderr, GTXT ("er_archive: Fatal error: rename(%s, %s) returned error: %d\n"),
		       t, abs_caname, res);
	      unlink (t);
	      free (t);
	      free (abs_caname);
	      free (abs_aname);
	      return 1;
	    }
	  // File "abs_caname" is already there - continue
	}
      unlink (t);
      free (t);
    }
  else
    free (cad);
  char *lname = NULL;
  if (relative_path)
    {
      if (common_archive[0] != '/' && aname[0] != '/')
	{
	  // compare one relative path to another and find common beginning
	  char *rel_caname = dbe_sprintf ("%s/%s", common_archive, cafname);
	  if (rel_caname == NULL)
	    {
	      fprintf (stderr, GTXT ("er_archive: Fatal error: unable to allocate memory\n"));
	      return 1;
	    }
	  lname = get_relative_link (rel_caname, aname);
	  free (rel_caname);
	}
      else
	{
	  if (abs_aname == NULL)
	    {
	      fprintf (stderr, GTXT ("er_archive: Fatal error: unable to allocate memory\n"));
	      return 1;
	    }
	  lname = get_relative_link (abs_caname, abs_aname);
	}
    }
  else  // absolute path
    lname = dbe_strdup (abs_caname);
  free (abs_aname);
  if (lname == NULL)
    {
      fprintf (stderr, GTXT ("er_archive: Fatal error: unable to allocate memory\n"));
      return 1;
    }
  // Create symbolic link: aname -> lname
  if (dbe_stat_file (abs_caname, NULL) == 0)
    {
      res = symlink (lname, aname);
      if (res != 0)
	{
	  fprintf (stderr, GTXT ("er_archive: Fatal error: symlink(%s, %s) returned error: %d (errno=%s)\n"),
		   lname, aname, res, strerror (errno));
	  free (abs_caname);
	  free (lname);
	  return 1;
	}
      if (!hide_msg)
	fprintf (stderr, GTXT ("Created symbolic link %s to file in common archive: %s\n"),
		 aname, lname);
    }
  else
    {
      fprintf (stderr, GTXT ("er_archive: Internal error: file does not exist in common archive: %s\n"),
	       abs_caname);
      res = 1;
    }
  free (abs_caname);
  free (lname);
  return res;
}

/**
 * Copy file to archive
 * @param name
 * @param aname
 * @param hide_msg
 * @param common_archive
 * @return 0 - success
 */
int
Experiment::copy_file (char *name, char *aname, int hide_msg, char *common_archive, int relative_path)
{
  if (common_archive)
    {
      if (0 == copy_file_to_common_archive (name, aname, hide_msg,
					    common_archive, relative_path))
	return 0;
      // Error. For now - fatal error. Message is already printed.
      fprintf (stderr, GTXT ("er_archive: Fatal error: cannot copy file %s to common archive %s\n"),
	       name, common_archive);
      return 1;
    }
  return (copy_file_to_archive (name, aname, hide_msg));
}

LoadObject *
Experiment::createLoadObject (const char *path, uint64_t chksum)
{
  LoadObject *lo = dbeSession->createLoadObject (path, chksum);
  if (lo->firstExp == NULL)
    lo->firstExp = this;
  return lo;
}

LoadObject *
Experiment::createLoadObject (const char *path, const char *runTimePath)
{
  DbeFile *df = findFileInArchive (path, runTimePath);
  if (df && (df->get_stat () == NULL))
    df = NULL; // No access to file
  LoadObject *lo = dbeSession->createLoadObject (path, runTimePath, df);
  if (df && (lo->dbeFile->get_location (false) == NULL))
    {
      lo->dbeFile->set_location (df->get_location ());
      lo->dbeFile->inArchive = df->inArchive;
      lo->dbeFile->sbuf = df->sbuf;
      lo->dbeFile->experiment = df->experiment;
      lo->firstExp = df->experiment;
    }
  if (lo->firstExp == NULL)
    {
      lo->firstExp = this;
      lo->dbeFile->experiment = this;
    }
  return lo;
}

SourceFile *
Experiment::get_source (const char *path)
{
  if (founder_exp && (founder_exp != this))
    return founder_exp->get_source (path);
  if (sourcesMap == NULL)
    sourcesMap = new StringMap<SourceFile*>(1024, 1024);
  if (strncmp (path, NTXT ("./"), 2) == 0)
    path += 2;
  SourceFile *sf = sourcesMap->get (path);
  if (sf)
    return sf;
  char *fnm = checkFileInArchive (path, false);
  if (fnm)
    {
      sf = new SourceFile (path);
      dbeSession->append (sf);
      DbeFile *df = sf->dbeFile;
      df->set_location (fnm);
      df->inArchive = true;
      df->check_access (fnm); // init 'sbuf'
      df->sbuf.st_mtime = 0; // Don't check timestamps
      free (fnm);
    }
  else
    sf = dbeSession->createSourceFile (path);
  sourcesMap->put (path, sf);
  return sf;
}

Vector<Histable*> *
Experiment::get_comparable_objs ()
{
  update_comparable_objs ();
  if (comparable_objs || dbeSession->expGroups->size () <= 1)
    return comparable_objs;
  comparable_objs = new Vector<Histable*>(dbeSession->expGroups->size ());
  for (long i = 0, sz = dbeSession->expGroups->size (); i < sz; i++)
    {
      ExpGroup *gr = dbeSession->expGroups->get (i);
      if (groupId == gr->groupId)
	{
	  comparable_objs->append (this);
	  continue;
	}
      Histable *h = NULL;
      for (long i1 = 0, sz1 = gr->exps ? gr->exps->size () : 0; i1 < sz1; i1++)
	{
	  Experiment *exp = gr->exps->get (i1);
	  if ((exp->comparable_objs == NULL) && (dbe_strcmp (utargname, exp->utargname) == 0))
	    {
	      exp->phaseCompareIdx = phaseCompareIdx;
	      h = exp;
	      h->comparable_objs = comparable_objs;
	      break;
	    }
	}
      comparable_objs->append (h);
    }
  dump_comparable_objs ();
  return comparable_objs;
}

DbeFile *
Experiment::findFileInArchive (const char *fname)
{
  if (archiveMap)
    {
      char *aname = get_archived_name (fname);
      DbeFile *df = archiveMap->get (aname);
      free (aname);
      return df;
    }
  if (founder_exp)
    return founder_exp->findFileInArchive (fname);
  return NULL;
}

DbeFile *
Experiment::findFileInArchive (const char *className, const char *runTimePath)
{
  DbeFile *df = NULL;
  if (runTimePath)
    {
      const char *fnm = NULL;
      if (strncmp (runTimePath, NTXT ("zip:"), 4) == 0)
	fnm = runTimePath + 4;
      else if (strncmp (runTimePath, NTXT ("jar:file:"), 9) == 0)
	fnm = runTimePath + 9;
      if (fnm)
	{
	  const char *s = strchr (fnm, '!');
	  if (s)
	    {
	      char *s1 = dbe_strndup (fnm, s - fnm);
	      df = findFileInArchive (s1);
	      free (s1);
	    }
	  else
	    df = findFileInArchive (fnm);
	  if (df)
	    df->filetype |= DbeFile::F_JAR_FILE;
	}
      else if (strncmp (runTimePath, NTXT ("file:"), 5) == 0)
	{
	  fnm = runTimePath + 5;
	  df = findFileInArchive (fnm);
	}
      else
	df = findFileInArchive (runTimePath);
    }
  if (df == NULL)
    df = findFileInArchive (className);
  return df;
}
