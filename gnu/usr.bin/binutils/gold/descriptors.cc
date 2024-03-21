// descriptors.cc -- manage file descriptors for gold

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

#include "gold.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>

#include "debug.h"
#include "parameters.h"
#include "options.h"
#include "gold-threads.h"
#include "descriptors.h"
#include "binary-io.h"

// O_CLOEXEC is only available on newer systems.
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

// Very old systems may not define FD_CLOEXEC.
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif

static inline void
set_close_on_exec(int fd ATTRIBUTE_UNUSED)
{
// Mingw does not define F_SETFD.
#ifdef F_SETFD
  fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
}

namespace gold
{

// Class Descriptors.

// The default for limit_ is meant to simply be large.  It gets
// adjusted downward if we run out of file descriptors.

Descriptors::Descriptors()
  : lock_(NULL), initialize_lock_(&this->lock_), open_descriptors_(),
    stack_top_(-1), current_(0), limit_(8192 - 16)
{
  this->open_descriptors_.reserve(128);
}

// Open a file.

int
Descriptors::open(int descriptor, const char* name, int flags, int mode)
{
  // We don't initialize this until we are called, because we can't
  // initialize a Lock until we have parsed the options to find out
  // whether we are running with threads.  We can be called before
  // options are valid when reading a linker script.
  bool lock_initialized = this->initialize_lock_.initialize();

  gold_assert(lock_initialized || descriptor < 0);

  if (is_debugging_enabled(DEBUG_FILES))
    this->limit_ = 8;

  if (descriptor >= 0)
    {
      Hold_lock hl(*this->lock_);

      gold_assert(static_cast<size_t>(descriptor)
		  < this->open_descriptors_.size());
      Open_descriptor* pod = &this->open_descriptors_[descriptor];
      if (pod->name == name
	  || (pod->name != NULL && strcmp(pod->name, name) == 0))
	{
	  gold_assert(!pod->inuse);
	  pod->inuse = true;
	  if (descriptor == this->stack_top_)
	    {
	      this->stack_top_ = pod->stack_next;
	      pod->stack_next = -1;
	      pod->is_on_stack = false;
	    }
	  gold_debug(DEBUG_FILES, "Reused existing descriptor %d for \"%s\"",
		     descriptor, name);
	  return descriptor;
	}
    }

  while (true)
    {
      // We always want to set the close-on-exec flag; we don't
      // require callers to pass it.
      flags |= O_CLOEXEC;

      // Always open the file as a binary file.
      flags |= O_BINARY;

      int new_descriptor = ::open(name, flags, mode);
      if (new_descriptor < 0
	  && errno != ENFILE
	  && errno != EMFILE)
	{
	  if (descriptor >= 0 && errno == ENOENT)
	    {
	      {
		Hold_lock hl(*this->lock_);

		gold_error(_("file %s was removed during the link"), name);
	      }

	      errno = ENOENT;
	    }

	  gold_debug(DEBUG_FILES, "Opened new descriptor %d for \"%s\"",
		     new_descriptor, name);
	  return new_descriptor;
	}

      if (new_descriptor >= 0)
	{
	  // If we have any plugins, we really do need to set the
	  // close-on-exec flag, even if O_CLOEXEC is not defined.
	  // FIXME: In some cases O_CLOEXEC may be defined in the
	  // header file but not supported by the kernel.
	  // Unfortunately there doesn't seem to be any obvious way to
	  // detect that, as unknown flags passed to open are ignored.
	  if (O_CLOEXEC == 0
	      && parameters->options_valid()
	      && parameters->options().has_plugins())
	    set_close_on_exec(new_descriptor);

	  {
	    Hold_optional_lock hl(this->lock_);

	    if (static_cast<size_t>(new_descriptor)
		>= this->open_descriptors_.size())
	      this->open_descriptors_.resize(new_descriptor + 64);

	    Open_descriptor* pod = &this->open_descriptors_[new_descriptor];
	    pod->name = name;
	    pod->stack_next = -1;
	    pod->inuse = true;
	    pod->is_write = (flags & O_ACCMODE) != O_RDONLY;
	    pod->is_on_stack = false;

	    ++this->current_;
	    if (this->current_ >= this->limit_)
	      this->close_some_descriptor();

	    gold_debug(DEBUG_FILES, "Opened new descriptor %d for \"%s\"",
		       new_descriptor, name);
	    return new_descriptor;
	  }
	}

      // We ran out of file descriptors.
      {
	Hold_optional_lock hl(this->lock_);

	this->limit_ = this->current_ - 16;
	if (this->limit_ < 8)
	  this->limit_ = 8;
	if (!this->close_some_descriptor())
	  gold_fatal(_("out of file descriptors and couldn't close any"));
      }
    }
}

// Release a descriptor.

void
Descriptors::release(int descriptor, bool permanent)
{
  Hold_optional_lock hl(this->lock_);

  gold_assert(descriptor >= 0
	      && (static_cast<size_t>(descriptor)
		  < this->open_descriptors_.size()));
  Open_descriptor* pod = &this->open_descriptors_[descriptor];

  if (permanent
      || (this->current_ > this->limit_ && !pod->is_write))
    {
      if (::close(descriptor) < 0)
	gold_warning(_("while closing %s: %s"), pod->name, strerror(errno));
      pod->name = NULL;
      --this->current_;
    }
  else
    {
      pod->inuse = false;
      if (!pod->is_write && !pod->is_on_stack)
	{
	  pod->stack_next = this->stack_top_;
	  this->stack_top_ = descriptor;
	  pod->is_on_stack = true;
	}
    }

  gold_debug(DEBUG_FILES, "Released descriptor %d for \"%s\"",
	     descriptor, pod->name);
}

// Close some descriptor.  The lock is held when this is called.  We
// close the descriptor on the top of the free stack.  Note that this
// is the opposite of an LRU algorithm--we close the most recently
// used descriptor.  That is because the linker tends to cycle through
// all the files; after we release a file, we are unlikely to need it
// again until we have looked at all the other files.  Return true if
// we closed a descriptor.

bool
Descriptors::close_some_descriptor()
{
  int last = -1;
  int i = this->stack_top_;
  while (i >= 0)
    {
      gold_assert(static_cast<size_t>(i) < this->open_descriptors_.size());
      Open_descriptor* pod = &this->open_descriptors_[i];
      if (!pod->inuse && !pod->is_write)
	{
	  if (::close(i) < 0)
	    gold_warning(_("while closing %s: %s"), pod->name, strerror(errno));
	  --this->current_;
	  gold_debug(DEBUG_FILES, "Closed descriptor %d for \"%s\"",
		     i, pod->name);
	  pod->name = NULL;
	  if (last < 0)
	    this->stack_top_ = pod->stack_next;
	  else
	    this->open_descriptors_[last].stack_next = pod->stack_next;
	  pod->stack_next = -1;
	  pod->is_on_stack = false;
	  return true;
	}
      last = i;
      i = pod->stack_next;
    }

  // We couldn't find any descriptors to close.  This is weird but not
  // necessarily an error.
  return false;
}

// Close all the descriptors open for reading.

void
Descriptors::close_all()
{
  Hold_optional_lock hl(this->lock_);

  for (size_t i = 0; i < this->open_descriptors_.size(); i++)
    {
      Open_descriptor* pod = &this->open_descriptors_[i];
      if (pod->name != NULL && !pod->inuse && !pod->is_write)
	{
	  if (::close(i) < 0)
	    gold_warning(_("while closing %s: %s"), pod->name, strerror(errno));
	  gold_debug(DEBUG_FILES, "Closed descriptor %d for \"%s\" (close_all)",
		     static_cast<int>(i), pod->name);
	  pod->name = NULL;
	  pod->stack_next = -1;
	  pod->is_on_stack = false;
	}
    }
  this->stack_top_ = -1;
}

// The single global variable which manages descriptors.

Descriptors descriptors;

} // End namespace gold.
