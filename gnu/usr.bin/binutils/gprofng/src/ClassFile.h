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

#ifndef _CLASSFILE_H
#define _CLASSFILE_H

#include "Module.h"

class DataInputStream;
class BinaryConstantPool;
class JMethod;
class StringBuilder;
class ByteCodeInfo;

class ClassFile : public Module
{
public:
  ClassFile ();
  virtual ~ClassFile ();
  virtual int readFile ();
  virtual char *get_disasm (uint64_t inst_address, uint64_t end_address,
			    uint64_t start_address, uint64_t f_offset,
			    int64_t &inst_size);
  static char *get_java_file_name (char *clname, bool classSuffix);

private:

  void openFile (const char *fname);
  char *get_opc_name (int op);
  void readAttributes (int count);
  void printConstant (StringBuilder *sb, int index);
  long long printCodeSequence (StringBuilder *sb, uint64_t addr, DataInputStream *in);

  unsigned char *cf_buf;
  int64_t cf_bufsz;
  int blanksCnt;
  DataInputStream *input;
  BinaryConstantPool *bcpool;
  JMethod *cur_jmthd;
  char *class_name;
  char *class_filename;
  char *source_name;
  Vector<ByteCodeInfo *> *byteCodeInfo;
};

#endif
