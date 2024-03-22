/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef INTERFERENCE_H
#define INTERFERENCE_H

#include "sfn_valuefactory.h"

#include <vector>

namespace r600 {

class ComponentInterference {
public:
   using Row = std::vector<int>;

   void prepare_row(int row);

   void add(size_t idx1, size_t idx2);

   auto row(int idx) const -> const Row&
   {
      assert((size_t)idx < m_rows.size());
      return m_rows[idx];
   }

private:
   std::vector<Row> m_rows;
};

class Interference {
public:
   Interference(LiveRangeMap& map);

   const auto& row(int comp, int index) const
   {
      assert(comp < 4);
      return m_components_maps[comp].row(index);
   }

private:
   void initialize();
   void initialize(ComponentInterference& comp, LiveRangeMap::ChannelLiveRange& clr);

   LiveRangeMap& m_map;
   std::array<ComponentInterference, 4> m_components_maps;
};

bool
register_allocation(LiveRangeMap& lrm);

} // namespace r600

#endif // INTERFERENCE_H
