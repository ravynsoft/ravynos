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

#include "sfn_ra.h"

#include "sfn_alu_defines.h"
#include "sfn_debug.h"

#include <cassert>
#include <queue>

namespace r600 {

void
ComponentInterference::prepare_row(int row)
{
   m_rows.resize(row + 1);
}

void
ComponentInterference::add(size_t idx1, size_t idx2)
{
   assert(idx1 > idx2);
   assert(m_rows.size() > idx1);
   m_rows[idx1].push_back(idx2);
   m_rows[idx2].push_back(idx1);
}

Interference::Interference(LiveRangeMap& map):
    m_map(map)
{
   initialize();
}

void
Interference::initialize()
{
   for (int i = 0; i < 4; ++i) {
      initialize(m_components_maps[i], m_map.component(i));
   }
}

void
Interference::initialize(ComponentInterference& comp_interference,
                         LiveRangeMap::ChannelLiveRange& clr)
{
   for (size_t row = 0; row < clr.size(); ++row) {
      auto& row_entry = clr[row];
      comp_interference.prepare_row(row);
      for (size_t col = 0; col < row; ++col) {
         auto& col_entry = clr[col];
         if (row_entry.m_end >= col_entry.m_start && row_entry.m_start <= col_entry.m_end)
            comp_interference.add(row, col);
      }
   }
}

struct Group {
   int priority;
   std::array<PRegister, 4> channels;
};

static inline bool
operator<(const Group& lhs, const Group& rhs)
{
   return lhs.priority < rhs.priority;
}

using GroupRegisters = std::priority_queue<Group>;

static bool
group_allocation(LiveRangeMap& lrm,
                 const Interference& interference,
                 GroupRegisters& groups)
{
   int color = 0;
   // allocate grouped registers
   while (!groups.empty()) {
      auto group = groups.top();
      groups.pop();

      int start_comp = 0;
      while (!group.channels[start_comp])
         ++start_comp;

      sfn_log << SfnLog::merge << "Color group with " << *group.channels[start_comp]
              << "\n";

      // don't restart registers for exports, we may be able tp merge the
      // export calls, is fthe registers are consecutive
      if (group.priority > 0)
         color = 0;

      while (color < g_registers_end) {
         /* Find the coloring for the first channel */
         bool color_in_use = false;
         int comp = start_comp;

         auto& adjecency = interference.row(start_comp, group.channels[comp]->index());
         auto& regs = lrm.component(comp);

         sfn_log << SfnLog::merge << "Try color " << color;

         for (auto adj : adjecency) {
            if (regs[adj].m_color == color) {
               color_in_use = true;
               sfn_log << SfnLog::merge << " in use\n";
               break;
            }
         }

         if (color_in_use) {
            ++color;
            continue;
         }

         /* First channel color found, check whether it can be used for all
          * channels */
         while (comp < 4) {
            sfn_log << SfnLog::merge << " interference: ";
            if (group.channels[comp]) {
               auto& component_life_ranges = lrm.component(comp);
               auto& adjecencies = interference.row(comp, group.channels[comp]->index());

               for (auto adj_index : adjecencies) {
                  sfn_log << SfnLog::merge << *component_life_ranges[adj_index].m_register
                          << " ";
                  if (component_life_ranges[adj_index].m_color == color) {
                     color_in_use = true;
                     sfn_log << SfnLog::merge << "used";
                     break;
                  }
               }

               if (color_in_use)
                  break;
            }
            ++comp;
         }

         /* We couldn't allocate all channels with this color, so try next */
         if (color_in_use) {
            ++color;
            sfn_log << SfnLog::merge << "\n";
            continue;
         }
         sfn_log << SfnLog::merge << " success\n";

         /* Coloring successful */
         for (auto reg : group.channels) {
            if (reg) {
               auto& vregs = lrm.component(reg->chan());
               auto& vreg_cmp = vregs[reg->index()];
               assert(vreg_cmp.m_start != -1 || vreg_cmp.m_end != -1);
               vreg_cmp.m_color = color;
            }
         }
         break;
      }

      if (color == g_registers_end)
         return false;
   }

   return true;
}

static bool
scalar_allocation(LiveRangeMap& lrm, const Interference& interference)
{
   for (int comp = 0; comp < 4; ++comp) {
      auto& live_ranges = lrm.component(comp);
      for (auto& r : live_ranges) {
         if (r.m_color != -1)
            continue;

         if (r.m_start == -1 && r.m_end == -1)
            continue;

         sfn_log << SfnLog::merge << "Color " << *r.m_register << "\n";

         auto& adjecency = interference.row(comp, r.m_register->index());

         int color = 0;

         while (color < g_registers_end) {
            bool color_in_use = false;
            for (auto adj : adjecency) {
               if (live_ranges[adj].m_color == color) {
                  color_in_use = true;
                  break;
               }
            }

            if (color_in_use) {
               ++color;
               continue;
            }

            r.m_color = color;
            break;
         }
         if (color == g_registers_end)
            return false;
      }
   }
   return true;
}

struct AluRegister {
   int lifetime;
   LiveRangeEntry *lre;
};

static inline bool operator < (const AluRegister& lhs, const AluRegister& rhs)
{
   return lhs.lifetime > rhs.lifetime;
}

using AluClauseRegisters = std::priority_queue<AluRegister>;


static void
scalar_clause_local_allocation (LiveRangeMap& lrm, const Interference&  interference)
{
   for (int comp = 0; comp < 4; ++comp) {
      AluClauseRegisters clause_reg;
      auto& live_ranges = lrm.component(comp);
      for (auto& r : live_ranges) {

         sfn_log << SfnLog::merge << "LR: " << *r.m_register
                 <<  "[ " << r.m_start << ", " << r.m_end
                  << " ], AC: " << r.m_alu_clause_local
                  << " Color; " << r.m_color << "\n";

         if (r.m_color != -1)
            continue;

         if (r.m_start == -1 &&
             r.m_end == -1)
            continue;

         if (!r.m_alu_clause_local)
            continue;

         int len = r.m_end - r.m_start;
         if (len > 1) {
            clause_reg.push({len, &r});
            sfn_log << SfnLog::merge << "Consider " << *r.m_register
                    << " for clause local\n";
         }
      }

      while (!clause_reg.empty()) {
         auto& r = clause_reg.top().lre;
         clause_reg.pop();

         sfn_log << SfnLog::merge << "Color " << *r->m_register << "\n";

         auto& adjecency = interference.row(comp, r->m_register->index());

         int color = g_clause_local_start;

         while (color < g_clause_local_end) {
            bool color_in_use = false;
            for (auto adj : adjecency) {
               if (live_ranges[adj].m_color == color) {
                  color_in_use = true;
                  break;
               }
            }

            if (color_in_use) {
               ++color;
               continue;
            }

            r->m_color = color;
            break;
         }
         if (color == g_clause_local_end)
            break;
      }
   }
}

bool
register_allocation(LiveRangeMap& lrm)
{
   Interference interference(lrm);

   std::map<int, Group> groups;

   // setup fixed colors and group relationships
   for (int i = 0; i < 4; ++i) {
      auto& comp = lrm.component(i);
      for (auto& entry : comp) {
         sfn_log << SfnLog::merge << "Prepare RA for " << *entry.m_register << " ["
                 << entry.m_start << ", " << entry.m_end << "]\n";
         auto pin = entry.m_register->pin();
         if (entry.m_start == -1 && entry.m_end == -1) {
            if (pin == pin_group || pin == pin_chgr)
               entry.m_register->set_chan(7);
            continue;
         }

         auto sel = entry.m_register->sel();
         /* fully pinned registers contain system values with the
          * definite register index, and array values are allocated
          * right after the system registers, so just reuse the IDs (for now) */
         if (pin == pin_fully || pin == pin_array) {
            /* Must set all array element entries */
            sfn_log << SfnLog::merge << "Pin color " << sel << " to " << *entry.m_register
                    << "\n";
            entry.m_color = sel;
         } else if (pin == pin_group || pin == pin_chgr) {
            /* Groups must all have the same sel() value, because they are
             * used as vec4 registers */
            auto igroup = groups.find(sel);
            if (igroup != groups.end()) {
               igroup->second.channels[i] = entry.m_register;
               assert(comp[entry.m_register->index()].m_register->index() ==
                      entry.m_register->index());
            } else {
               int priority = entry.m_use.test(LiveRangeEntry::use_export)
                                 ? -entry.m_end
                                 : entry.m_start;
               Group group{
                  priority, {nullptr, nullptr, nullptr, nullptr}
               };
               group.channels[i] = entry.m_register;
               assert(comp[group.channels[i]->index()].m_register->index() ==
                      entry.m_register->index());
               groups[sel] = group;
            }
         }
      }
   }

   GroupRegisters groups_sorted;
   for (auto& [sel, group] : groups)
      groups_sorted.push(group);

   if (!group_allocation(lrm, interference, groups_sorted))
      return false;

   scalar_clause_local_allocation(lrm, interference);

   if (!scalar_allocation(lrm, interference))
      return false;

   for (int i = 0; i < 4; ++i) {
      auto& comp = lrm.component(i);
      for (auto& entry : comp) {
         sfn_log << SfnLog::merge << "Set " << *entry.m_register << " to ";
         entry.m_register->set_sel(entry.m_color);
         entry.m_register->set_pin(pin_none);
         sfn_log << SfnLog::merge << *entry.m_register << "\n";
      }
   }

   return true;
}

} // namespace r600
