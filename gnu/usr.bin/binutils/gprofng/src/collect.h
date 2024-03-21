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

#ifndef _COLLECT_H
#define _COLLECT_H

#include <Application.h>

extern "C"
{
  typedef void (*SignalHandler)(int);
}

class Coll_Ctrl;
class Elf;

#define MAXLABELS       10      /* maximum number of -C arguments */
#define STDEBUFSIZE     24000

enum { MAX_LD_PRELOAD_TYPES = 3 };

struct Process
{
  Process (pid_t _pid) : pid (_pid) { }
  pid_t pid;
};

struct DtraceTool
{
  DtraceTool (char*);
  ~DtraceTool ();

  char *name;       // Tool name as specified by -D flag
  char *params;     // Tool parameters
  char *dfile;      // Extracted d-script
  char *mfile;      // Extracted metadata file
  char *ofile;      // Output file
  pid_t pid;
};

// collect object
class collect : Application
{
public:
  collect (int argc, char *argv[], char **envp);
  virtual ~collect ();
  void start (int argc, char *argv[]);
  void  writeStr (int f, const char *buf);

  // the collector control class
  Coll_Ctrl *cc;

private:
  enum Exec_status
  {
    EXEC_OK = 0, // found as runnable executable
    EXEC_ELF_NOSHARE, // found, but built unshared
    EXEC_IS_JAR, // found as a .jar file
    EXEC_IS_CLASS, // found as a .class file but name missing .class
    EXEC_IS_CLASSCLASS, // found as a .class file with explicit .class
    EXEC_OPEN_FAIL, // could not be opened
    EXEC_ELF_LIB, // internal error: bad elf library
    EXEC_ELF_HEADER, // executable, with bad ELF header
    EXEC_ELF_ARCH, // executable, but unrunnable architecture
    EXEC_ISDIR, // a directory, not a file
    EXEC_NOT_EXEC, // a file, but not executable
    EXEC_NOT_FOUND // a directory, not a file
  };

  // override methods in base class
  void usage ();
  void short_usage ();
  void show_hwc_usage ();
  int check_args (int argc, char *argv[]);
  void check_target (int, char **);
  Exec_status check_executable (char *);
  Exec_status check_executable_arch (Elf *);
  char *status_str (Exec_status, char *);
  int do_flag (const char *);
  char *find_java (void);
  char *java_path;
  char *java_how;
  int putenv_libcollector ();
  int putenv_libcollector_ld_audits ();
  int putenv_libcollector_ld_preloads ();
  int putenv_libcollector_ld_misc ();
  void add_ld_preload (const char *lib);
  int putenv_ld_preloads ();
  int putenv_memso ();
  int env_strip (char *env, const char *str);
  int putenv_purged_ld_preloads (const char *var);
  int putenv_append (const char *var, const char *val);
  void get_count_data ();
  void prepare_dbx ();
  int traceme (const char *file, char *const argv[]);
  int checkflagterm (const char *);
  void dupflagseen (char);
  void dupflagseen (const char *);
  void validate_config (int);
  void validate_java (const char *, const char *, int);
  int set_output ();
  void reset_output ();

  /* Logging warning messages */
  char **collect_warnings;
  int collect_warnings_idx;
  void warn_open ();
  void warn_close ();
  void warn_write (const char *format, ...);
  void warn_comment (const char *kind, int num, char *s = NULL, int len = 0);
  char *warnfilename;
  FILE *warn_file;

  /* MPI experiment handling */
  void setup_MPI_expt ();   /* the founder experiment */
  void write_MPI_founder_log ();
  void close_MPI_founder_log (int, int);
  void spawn_MPI_job (); /* run the MPI job */
  void copy_collect_args (char ***);   /* copy collect args for an MPI target */
  int disabled;
  int jseen_global;         /* if -j flag was seen */
  int verbose;
  bool mem_so_me;           /* if T, preload mem.so, not libcollector */
  int origargc;
  char **arglist;
  char **origargv;
  char **origenvp;
  int targ_index;           // index of name of target in origargv
  bool is_64;
  int nargs;
  int njargs;
  char *jargs;
  int nlabels;
  char *label[MAXLABELS];
  char *sp_preload_list[MAX_LD_PRELOAD_TYPES + 1];  // +1 for NULL termination
  char *sp_libpath_list[MAX_LD_PRELOAD_TYPES + 1];  // +1 for NULL termination
};

#endif /* ! _COLLECT_H */
