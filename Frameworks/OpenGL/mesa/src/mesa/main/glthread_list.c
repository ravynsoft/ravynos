/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "c99_alloca.h"

#include "main/glthread_marshal.h"
#include "main/dispatch.h"

struct marshal_cmd_CallList
{
   struct marshal_cmd_base cmd_base;
   GLuint num;
   GLuint list[];
};

uint32_t
_mesa_unmarshal_CallList(struct gl_context *ctx,
                         const struct marshal_cmd_CallList *restrict cmd)
{
   const GLuint num = cmd->num;

   if (cmd->cmd_base.cmd_size == sizeof(*cmd) / 8) {
      CALL_CallList(ctx->Dispatch.Current, (num));
   } else {
      CALL_CallLists(ctx->Dispatch.Current, (num, GL_UNSIGNED_INT, cmd->list));
   }

   return cmd->cmd_base.cmd_size;
}

void GLAPIENTRY
_mesa_marshal_CallList(GLuint list)
{
   GET_CURRENT_CONTEXT(ctx);
   struct glthread_state *glthread = &ctx->GLThread;
   struct marshal_cmd_CallList *last = glthread->LastCallList;

   _mesa_glthread_CallList(ctx, list);

   /* If the last call is CallList and there is enough space to append another list... */
   if (_mesa_glthread_call_is_last(glthread, &last->cmd_base) &&
       glthread->used + 1 <= MARSHAL_MAX_CMD_SIZE / 8) {
      STATIC_ASSERT(sizeof(*last) == 8);

      /* Add the list to the last call. */
      if (last->cmd_base.cmd_size > sizeof(*last) / 8) {
         last->list[last->num++] = list;
         if (last->num % 2 == 1) {
            last->cmd_base.cmd_size++;
            glthread->used++;
         }
      } else {
         /* Initially, num contains the first list. After we increase cmd_size,
          * num contains the number of lists and list[] contains the lists.
          */
         last->list[0] = last->num;
         last->list[1] = list;
         last->num = 2;
         last->cmd_base.cmd_size++;
         glthread->used++;
      }
      assert(align(sizeof(*last) + last->num * 4, 8) / 8 == last->cmd_base.cmd_size);
      return;
   }

   int cmd_size = sizeof(struct marshal_cmd_CallList);
   struct marshal_cmd_CallList *cmd;
   cmd = _mesa_glthread_allocate_command(ctx, DISPATCH_CMD_CallList, cmd_size);
   cmd->num = list;

   glthread->LastCallList = cmd;
}
