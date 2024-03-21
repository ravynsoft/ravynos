// cref.h -- cross reference reports for gold   -*- C++ -*-

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#ifndef GOLD_CREF_H
#define GOLD_CREF_H

#include <cstdio>

namespace gold
{

class Object;
class Archive;
class Cref_inputs;

// This class collects data for cross reference and other reporting.

class Cref
{
 public:
  Cref()
    : inputs_(NULL)
  { }

  // Record an input object file.  This is called for each object file
  // in the order in which it is processed.
  void
  add_object(Object*);

  // Start recording an input archive.  This is called for each
  // archive in the order in which it appears on the command line.  A
  // call to add_archive_start precedes calls to add_object for each
  // object included from the archive.
  void
  add_archive_start(Archive*);

  // Finish recording an input archive.  This is called after
  // add_object has been called for each object included from the
  // archive.
  void
  add_archive_stop(Archive*);

  // Print symbol counts.
  void
  print_symbol_counts(const Symbol_table*) const;

  // Print a cross reference table.
  void
  print_cref(const Symbol_table*, FILE*) const;

 private:
  void
  need_inputs();

  Cref_inputs* inputs_;
};

} // End namespace gold.

#endif // !defined(GOLD_CREF_H)
