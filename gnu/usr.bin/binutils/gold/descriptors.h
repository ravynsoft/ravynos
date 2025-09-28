// descriptors.h -- manage file descriptors for gold   -*- C++ -*-

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

#ifndef GOLD_DESCRIPTORS_H
#define GOLD_DESCRIPTORS_H

#include <vector>

#include "gold-threads.h"

namespace gold
{

// This class manages file descriptors for gold.

class Descriptors
{
 public:
  Descriptors();

  // Get a file descriptor for a file.  The DESCRIPTOR parameter is
  // the descriptor the last time the file was used; this will be -1
  // if this is the first time the file is being opened.  The NAME,
  // FLAGS, and MODE parameters are as for ::open.  NAME must be in
  // permanent storage.  This returns the descriptor to use, which may
  // or may not be the same as DESCRIPTOR.  If there is an error
  // opening the file, this will return -1 with errno set
  // appropriately.
  int
  open(int descriptor, const char* name, int flags, int mode = 0);

  // Release the file descriptor DESCRIPTOR.  If PERMANENT is true, it
  // will be closed, and the caller may not reopen it.  If PERMANENT
  // is false this doesn't necessarily close the descriptor, but it
  // makes it available to be closed; the descriptor must not be used
  // again except as an argument to Descriptor::open.
  void
  release(int descriptor, bool permanent);

  // Close all the descriptors open for reading.
  void
  close_all();

 private:
  // Information kept for a descriptor.
  struct Open_descriptor
  {
    // File name currently associated with descriptor.  This is empty
    // if none.
    const char* name;
    // Index of next descriptor on stack of released descriptors.
    int stack_next;
    // Whether the descriptor is currently in use.
    bool inuse;
    // Whether this is a write descriptor.
    bool is_write;
    // Whether the descriptor is on the stack.
    bool is_on_stack;
  };

  bool
  close_some_descriptor();

  // We need to lock before accessing any fields.
  Lock* lock_;
  // Used to initialize the lock_ field exactly once.
  Initialize_lock initialize_lock_;
  // Information for descriptors.
  std::vector<Open_descriptor> open_descriptors_;
  // Top of stack.
  int stack_top_;
  // The current number of file descriptors open.
  int current_;
  // The maximum number of file descriptors we open.
  int limit_;
};

// File descriptors are a centralized data structure, and we use a
// global variable rather than passing the data structure into every
// routine that does file I/O.

extern Descriptors descriptors;

inline int
open_descriptor(int descriptor, const char* name, int flags, int mode = 0)
{ return descriptors.open(descriptor, name, flags, mode); }

inline void
release_descriptor(int descriptor, bool permanent)
{ descriptors.release(descriptor, permanent); }

inline void
close_all_descriptors()
{ descriptors.close_all(); }

} // End namespace gold.

#endif // !defined(GOLD_DESCRIPTORS_H)
