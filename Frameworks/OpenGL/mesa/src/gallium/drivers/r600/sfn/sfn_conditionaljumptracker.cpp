/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2019 Collabora LTD
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

#include "sfn_conditionaljumptracker.h"

#include "sfn_debug.h"

#include <iostream>
#include <memory>
#include <stack>
#include <vector>

namespace r600 {

using std::shared_ptr;
using std::stack;
using std::vector;

struct StackFrame {

   StackFrame(r600_bytecode_cf *s, JumpType t):
       type(t),
       start(s)
   {
   }

   virtual ~StackFrame();

   JumpType type;
   r600_bytecode_cf *start;
   vector<r600_bytecode_cf *> mid;

   virtual void fixup_mid(r600_bytecode_cf *cf) = 0;
   virtual void fixup_pop(r600_bytecode_cf *final) = 0;
};

using PStackFrame = shared_ptr<StackFrame>;

struct IfFrame : public StackFrame {
   IfFrame(r600_bytecode_cf *s);
   void fixup_mid(r600_bytecode_cf *cf) override;
   void fixup_pop(r600_bytecode_cf *final) override;
};

struct LoopFrame : public StackFrame {
   LoopFrame(r600_bytecode_cf *s);
   void fixup_mid(r600_bytecode_cf *cf) override;
   void fixup_pop(r600_bytecode_cf *final) override;
};

struct ConditionalJumpTrackerImpl {
   ConditionalJumpTrackerImpl();
   stack<PStackFrame> m_jump_stack;
   stack<PStackFrame> m_loop_stack;
   int m_current_loop_stack_pos;
};

ConditionalJumpTrackerImpl::ConditionalJumpTrackerImpl():
    m_current_loop_stack_pos(0)
{
}

ConditionalJumpTracker::~ConditionalJumpTracker() { delete impl; }

ConditionalJumpTracker::ConditionalJumpTracker()
{
   impl = new ConditionalJumpTrackerImpl();
}

void
ConditionalJumpTracker::push(r600_bytecode_cf *start, JumpType type)
{
   PStackFrame f;
   switch (type) {
   case jt_if:
      f.reset(new IfFrame(start));
      break;
   case jt_loop:
      f.reset(new LoopFrame(start));
      impl->m_loop_stack.push(f);
      break;
   }
   impl->m_jump_stack.push(f);
}

bool
ConditionalJumpTracker::pop(r600_bytecode_cf *final, JumpType type)
{
   if (impl->m_jump_stack.empty())
      return false;

   auto& frame = *impl->m_jump_stack.top();
   if (frame.type != type)
      return false;

   frame.fixup_pop(final);
   if (frame.type == jt_loop)
      impl->m_loop_stack.pop();
   impl->m_jump_stack.pop();
   return true;
}

bool
ConditionalJumpTracker::add_mid(r600_bytecode_cf *source, JumpType type)
{
   if (impl->m_jump_stack.empty()) {
      sfn_log << "Jump stack empty\n";
      return false;
   }

   PStackFrame pframe;
   if (type == jt_loop) {
      if (impl->m_loop_stack.empty()) {
         sfn_log << "Loop jump stack empty\n";
         return false;
      }
      pframe = impl->m_loop_stack.top();
   } else {
      pframe = impl->m_jump_stack.top();
   }

   pframe->mid.push_back(source);
   pframe->fixup_mid(source);
   return true;
}

IfFrame::IfFrame(r600_bytecode_cf *s):
    StackFrame(s, jt_if)
{
}

StackFrame::~StackFrame() {}

void
IfFrame::fixup_mid(r600_bytecode_cf *source)
{
   /* JUMP target is ELSE */
   start->cf_addr = source->id;
}

void
IfFrame::fixup_pop(r600_bytecode_cf *final)
{
   /* JUMP or ELSE target is one past last CF instruction */
   unsigned offset = final->eg_alu_extended ? 4 : 2;
   auto src = mid.empty() ? start : mid[0];
   src->cf_addr = final->id + offset;
   src->pop_count = 1;
}

LoopFrame::LoopFrame(r600_bytecode_cf *s):
    StackFrame(s, jt_loop)
{
}

void
LoopFrame::fixup_mid(UNUSED r600_bytecode_cf *mid)
{
}

void
LoopFrame::fixup_pop(r600_bytecode_cf *final)
{
   /* LOOP END address is past LOOP START */
   final->cf_addr = start->id + 2;

   /* LOOP START address is past LOOP END*/
   start->cf_addr = final->id + 2;

   /* BREAK and CONTINUE point at LOOP END*/
   for (auto m : mid)
      m->cf_addr = final->id;
}

} // namespace r600
