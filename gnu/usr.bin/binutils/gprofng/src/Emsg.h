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

#ifndef _EMSG_H
#define _EMSG_H

#include "Emsgnum.h"
#include "vec.h"

//
// The Emsg, experiment message, has as objects I18N'd messages
//	in a structure suitable for attaching to and fetching
//	from a queue of such messages.  It is intended to
//	be used for collector errors, collector warnings, parser
//	errors, and er_archive errors that are encountered when
//	reading an experiment

class Emsg;
class Emsgqueue;
class StringBuilder;

typedef enum
{
  CMSG_WARN = 0,
  CMSG_ERROR,
  CMSG_FATAL,
  CMSG_COMMENT,
  CMSG_PARSER,
  CMSG_ARCHIVE
} Cmsg_warn;

class Emsg
{
public:
  friend class Emsgqueue;

  Emsg (Cmsg_warn w, const char *i18n_text);
  Emsg (Cmsg_warn w, StringBuilder& sb);
  Emsg (Cmsg_warn w, int f, const char *param);
  ~Emsg ();

  char *
  get_msg ()
  {
    return text;
  };

  Cmsg_warn
  get_warn ()
  {
    return warn;
  };

  Emsg *next;       // next message in a queue

protected:
  Cmsg_warn warn;   // error/warning/...
  int flavor;       // the message flavor
  char *par;        // the input parameter string
  char *text;       // The I18N text of the message
};

class Emsgqueue
{
public:
  Emsgqueue (char *);
  ~Emsgqueue ();

  void append (Emsg*);
  Emsg *append (Cmsg_warn w, char *msg);
  Emsg *find_msg (Cmsg_warn w, char *msg);
  void appendqueue (Emsgqueue*);
  Emsg *fetch (void);
  void clear (void); // empty the queue
  void mark_clear (void); // mark the queue empty, without touching messages

protected:
  Emsg *first;
  Emsg *last;
  char *qname;
};

class DbeMessages
{
public:
  DbeMessages ();
  ~DbeMessages ();
  Vector<Emsg*> *msgs;
  void remove_msg (Emsg *msg);
  Emsg *get_error ();
  Emsg *append_msg (Cmsg_warn w, const char *fmt, ...);
  void append_msgs (Vector<Emsg*> *lst);
};

#endif  /* _EMSG_H */
