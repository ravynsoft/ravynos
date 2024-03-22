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

#ifndef SFN_LIFERANGEEVALUATOR_HELPERS_H
#define SFN_LIFERANGEEVALUATOR_HELPERS_H

#include "sfn_valuefactory.h"

namespace r600 {

enum ProgramScopeType {
   outer_scope,           /* Outer program scope */
   loop_body,             /* Inside a loop */
   if_branch,             /* Inside if branch */
   else_branch,           /* Inside else branch */
   switch_body,           /* Inside switch statement */
   switch_case_branch,    /* Inside switch case statement */
   switch_default_branch, /* Inside switch default statement */
   undefined_scope
};

class ProgramScope {
public:
   ProgramScope();
   ProgramScope(ProgramScope *parent, ProgramScopeType type, int id, int depth, int begin);

   ProgramScopeType type() const;
   ProgramScope *parent() const;
   int nesting_depth() const;
   int id() const;
   int end() const;
   int begin() const;
   int loop_break_line() const;

   const ProgramScope *in_else_scope() const;
   const ProgramScope *in_ifelse_scope() const;
   const ProgramScope *in_parent_ifelse_scope() const;
   const ProgramScope *innermost_loop() const;
   const ProgramScope *outermost_loop() const;
   const ProgramScope *enclosing_conditional() const;

   bool is_loop() const;
   bool is_in_loop() const;
   bool is_switchcase_scope_in_loop() const;
   bool is_conditional() const;
   bool is_child_of(const ProgramScope *scope) const;
   bool is_child_of_ifelse_id_sibling(const ProgramScope *scope) const;

   bool break_is_for_switchcase() const;
   bool contains_range_of(const ProgramScope& other) const;

   void set_end(int end);
   void set_loop_break_line(int line);

private:
   ProgramScopeType scope_type;
   int scope_id;
   int scope_nesting_depth;
   int scope_begin;
   int scope_end;
   int break_loop_line;
   ProgramScope *parent_scope;
};

/* Class to track the access to a component of a temporary register. */

struct LiveRange;

class RegisterCompAccess {
public:
   RegisterCompAccess();
   RegisterCompAccess(LiveRange range);

   void record_read(int block,int line, ProgramScope *scope, LiveRangeEntry::EUse use);
   void record_write(int block, int line, ProgramScope *scope);

   void update_required_live_range();

   const auto& range() { return m_range; }

   const auto& use_type() { return m_use_type; }

   auto alu_clause_local() { return alu_block_id > block_id_uninitalized;}

private:
   void propagate_live_range_to_dominant_write_scope();
   bool conditional_ifelse_write_in_loop() const;

   void record_ifelse_write(const ProgramScope& scope);
   void record_if_write(const ProgramScope& scope);
   void record_else_write(const ProgramScope& scope);

   ProgramScope *last_read_scope;
   ProgramScope *first_read_scope;
   ProgramScope *first_write_scope;

   int first_write;
   int last_read;
   int last_write;
   int first_read;

   int alu_block_id{block_id_uninitalized};

   /* This member variable tracks the current resolution of conditional writing
    * to this temporary in IF/ELSE clauses.
    *
    * The initial value "conditionality_untouched" indicates that this
    * temporary has not yet been written to within an if clause.
    *
    * A positive (other than "conditionality_untouched") number refers to the
    * last loop id for which the write was resolved as unconditional. With
    * each new loop this value will be overwritten by
    * "conditionality_unresolved" on entering the first IF clause writing this
    * temporary.
    *
    * The value "conditionality_unresolved" indicates that no resolution has
    * been achieved so far. If the variable is set to this value at the end of
    * the processing of the whole shader it also indicates a conditional
    * write.
    *
    * The value "write_is_conditional" marks that the variable is written
    * conditionally (i.e. not in all relevant IF/ELSE code path pairs) in at
    * least one loop.
    */
   int conditionality_in_loop_id;

   /* Helper constants to make the tracking code more readable. */
   static const int write_is_conditional = -1;
   static const int conditionality_unresolved = 0;
   static const int conditionality_untouched;
   static const int write_is_unconditional;
   static const int block_id_not_unique = -1;
   static const int block_id_uninitalized = 0;

   /* A bit field tracking the nexting levels of if-else clauses where the
    * temporary has (so far) been written to in the if branch, but not in the
    * else branch.
    */
   unsigned int if_scope_write_flags;

   int next_ifelse_nesting_depth;
   static const int supported_ifelse_nesting_depth = 32;

   /* Tracks the last if scope in which the temporary was written to
    * without a write in the corresponding else branch. Is also used
    * to track read-before-write in the according scope.
    */
   const ProgramScope *current_unpaired_if_write_scope;

   /* Flag to resolve read-before-write in the else scope. */
   bool was_written_in_current_else_scope;

   LiveRange m_range;

   std::bitset<LiveRangeEntry::use_unspecified> m_use_type;
};

class RegisterAccess {
public:
   using RegisterCompAccessVector = std::vector<RegisterCompAccess>;

   RegisterAccess(const std::array<size_t, 4>& sizes);

   RegisterCompAccess& operator()(const Register& reg);

   auto& component(int i) { return m_access_record[i]; }

private:
   std::array<RegisterCompAccessVector, 4> m_access_record;
};

} // namespace r600
#endif // SFN_LIFERANGEEVALUATOR_HELPERS_H
