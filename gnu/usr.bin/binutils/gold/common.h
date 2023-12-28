// common.h -- handle common symbols for gold   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

#ifndef GOLD_COMMON_H
#define GOLD_COMMON_H

#include "workqueue.h"

namespace gold
{

class Symbol_table;

// This task is used to allocate the common symbols.

class Allocate_commons_task : public Task
{
 public:
  Allocate_commons_task(Symbol_table* symtab, Layout* layout, Mapfile* mapfile,
			Task_token* blocker)
    : symtab_(symtab), layout_(layout), mapfile_(mapfile), blocker_(blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  { return "Allocate_commons_task"; }

 private:
  Symbol_table* symtab_;
  Layout* layout_;
  Mapfile* mapfile_;
  Task_token* blocker_;
};

} // End namespace gold.

#endif // !defined(GOLD_COMMON_H)
