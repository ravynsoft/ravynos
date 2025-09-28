// gc.cc -- garbage collection of unused sections

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Sriraman Tallam <tmsriram@google.com>.

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
#include "object.h"
#include "gc.h"
#include "symtab.h"

namespace gold
{

// Garbage collection uses a worklist style algorithm to determine the 
// transitive closure of all referenced sections.
void 
Garbage_collection::do_transitive_closure()
{
  while (!this->worklist().empty())
    {
      // Add elements from the work list to the referenced list
      // one by one.
      Section_id entry = this->worklist().back();
      this->worklist().pop_back();
      if (!this->referenced_list().insert(entry).second)
        continue;
      Garbage_collection::Section_ref::iterator find_it = 
                this->section_reloc_map().find(entry);
      if (find_it == this->section_reloc_map().end()) 
          continue;
      const Garbage_collection::Sections_reachable &v = find_it->second;
      // Scan the vector of references for each work_list entry. 
      for (Garbage_collection::Sections_reachable::const_iterator it_v =
               v.begin();
           it_v != v.end();
           ++it_v)
        {
          // Do not add already processed sections to the work_list. 
          if (this->referenced_list().find(*it_v)
              == this->referenced_list().end())
            {
              this->worklist().push_back(*it_v);
            }
        }
    }
  this->worklist_ready();
}

} // End namespace gold.

