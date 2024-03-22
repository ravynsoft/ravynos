/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2018-2019 Collabora LTD
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

#ifndef SFN_CONDITIONALJUMPTRACKER_H
#define SFN_CONDITIONALJUMPTRACKER_H

#include "gallium/drivers/r600/r600_asm.h"

namespace r600 {

enum JumpType {
   jt_loop,
   jt_if
};

/**
  Class to link the jump locations
*/
class ConditionalJumpTracker {
public:
   ConditionalJumpTracker();
   ~ConditionalJumpTracker();

   /* Mark the start of a loop or a if/else */
   void push(r600_bytecode_cf *start, JumpType type);

   /* Mark the end of a loop or a if/else and fixup the jump sites */
   bool pop(r600_bytecode_cf *final, JumpType type);

   /* Add middle sites to the call frame i.e. continue,
    * break inside loops, and else in if-then-else constructs.
    */
   bool add_mid(r600_bytecode_cf *source, JumpType type);

private:
   struct ConditionalJumpTrackerImpl *impl;
};

} // namespace r600

#endif // SFN_CONDITIONALJUMPTRACKER_H
