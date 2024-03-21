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

/*
 * The Application class is the base class for all C++ executables
 *	in the Performance Tools Suite
 *
 *	It determines the directory from which the running binary came,
 *	sets up the I18N catalog, the program name, and initializes
 *	an instance of the Settings class to manage all user preferences
 *	and settings.  It also manages usage tracking.
 *
 *	Applications which read experiments are derived from a subclass
 *	named DbeApplication (q.v.)
 */

#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "dbe_types.h"

class Settings;
class Emsg;
class Emsgqueue;

// Application object
class Application
{
public:
  Application (int argc, char *argv[], char *_run_dir = NULL);
  virtual ~Application ();
  void set_name (const char *_name);
  char *get_cur_dir ();

  // Control the settings of a progress bar, used for GUI applications
  // this function also detects cancel requests and returns 1
  // if yes, 0 otherwise
  static int set_progress (int percentage, const char *proc_str);
  static char *get_realpath (const char *_name);

  // queue for messages (from reading er.rc files, ...)
  void queue_comment (Emsg *m); // queue for messages
  Emsg *fetch_comments (void);  // fetch the queue of comment messages
  void delete_comments (void);  // delete the queue of comment messages

  // worker threads (currently used in dbe_stat() for stat() calls)
  int get_number_of_worker_threads ();

  char *get_version ()              { return prog_version; }
  char *get_name ()                 { return prog_name; }
  char *get_run_dir ()              { return run_dir; }
  Emsgqueue *get_comments_queue ()  { return commentq; };

protected: // methods
  void set_run_dir (char *fdhome = NULL);
  typedef int (*ProgressFunc)(int, const char *);

  // Write a usage message; to be defined in derived class
  virtual void usage () = 0;

// Ruud
  // Write a version message; to be defined in derived class
  void print_version_info ();

  // Can be overridden in derived class
  virtual int check_args (int argc, char *argv[]);

  void read_rc ();
  static void set_progress_func (ProgressFunc func) { progress_func = func; }

protected:
  Emsgqueue *commentq;
  Settings *settings;
  char *prog_version;
  char *prog_name;
  char *whoami;
  char *run_dir;
  char *run_dir_with_spaces; // used in case there are spaces
  char *cur_dir;
  int lic_found;
  char *lic_err;

private:
  void set_ut_email (int argc, char *argv[]);
  int number_of_worker_threads;
  static ProgressFunc progress_func;
};

extern Application *theApplication;

#endif /* _APPLICATION_H */
