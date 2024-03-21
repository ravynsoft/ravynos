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

#ifndef _DBE_FILE_H
#define _DBE_FILE_H

#include <fcntl.h>

class DbeJarFile;
class Experiment;
template <class ITEM> class Vector;

class DbeFile
{
public:

  enum
  {
    F_NOT_FOUND     = 0,
    F_FICTION       = 1,
    F_LOADOBJ       = 2,
    F_SOURCE        = 4,
    F_JAVACLASS     = 8,
    F_JAVA_SOURCE   = 16,
    F_DOT_O         = 32,
    F_DEBUG_FILE    = 64,
    F_DOT_A_LIB     = 128,
    F_DIR_OR_JAR    = 256,
    F_DIRECTORY     = 512,
    F_FILE          = 1024,
    F_JAR_FILE      = 2048,
    F_UNKNOWN       = 65536
  };

  DbeFile (const char *filename);
  ~DbeFile ();

  char *
  get_name ()
  {
    return name;
  };

  bool
  get_need_refind ()
  {
    return need_refind;
  };

  char *get_location (bool find_needed = true);
  char *getResolvedPath ();
  char *get_location_info ();
  struct stat64 *get_stat ();
  bool compare (DbeFile *df);
  void set_need_refind (bool val);
  void set_location (const char *filename);
  int check_access (const char *filename);
  char *find_file (const char *filename);
  DbeFile *getJarDbeFile (char *fnm, int sym);
  char *find_in_jar_file (const char *filename, DbeJarFile *jfile);
  DbeJarFile *get_jar_file ();

  bool inArchive;
  int filetype;
  struct stat64 sbuf;
  DbeFile *container;
  char *orig_location;
  Experiment *experiment;

protected:
  static bool isJarOrZip (const char *fnm);
  char *find_package_name (const char *filename, const char *dirname);
  char *find_in_directory (const char *filename, const char *dirname);
  bool find_in_pathmap (char *filename);
  void find_in_archives (char *filename);
  void find_in_setpath (char *filename, Vector<char*> *searchPath);
  void find_in_classpath (char *filename, Vector<DbeFile*> *classPath);

  char *name;
  char *location;
  char *location_info;
  bool need_refind;
  DbeJarFile *jarFile;
};

#endif /* _DBE_FILE_H */
