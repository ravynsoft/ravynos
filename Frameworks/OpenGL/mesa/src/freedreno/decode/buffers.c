/*
 * Copyright (c) 2012 Rob Clark <robdclark@gmail.com>
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Helper lib to track gpu buffers contents/address, and map between gpu and
 * host address while decoding cmdstream/crashdumps
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/rb_tree.h"
#include "buffers.h"

struct buffer {
   struct rb_node node;
   void *hostptr;
   unsigned int len;
   uint64_t gpuaddr;

   /* for 'once' mode, for buffers containing cmdstream keep track per offset
    * into buffer of which modes it has already been dumped;
    */
   struct {
      unsigned offset;
      unsigned dumped_mask;
   } offsets[256];
   unsigned noffsets;
};

static struct rb_tree buffers;

static int
buffer_insert_cmp(const struct rb_node *n1, const struct rb_node *n2)
{
   const struct buffer *buf1 = (const struct buffer *)n1;
   const struct buffer *buf2 = (const struct buffer *)n2;
   /* Note that gpuaddr comparisions can overflow an int: */
   if (buf1->gpuaddr > buf2->gpuaddr)
      return 1;
   else if (buf1->gpuaddr < buf2->gpuaddr)
      return -1;
   return 0;
}

static int
buffer_search_cmp(const struct rb_node *node, const void *addrptr)
{
   const struct buffer *buf = (const struct buffer *)node;
   uint64_t gpuaddr = *(uint64_t *)addrptr;
   if (buf->gpuaddr + buf->len <= gpuaddr)
      return -1;
   else if (buf->gpuaddr > gpuaddr)
      return 1;
   return 0;
}

static struct buffer *
get_buffer(uint64_t gpuaddr)
{
   if (gpuaddr == 0)
      return NULL;
   return (struct buffer *)rb_tree_search(&buffers, &gpuaddr,
                                          buffer_search_cmp);
}

static int
buffer_contains_hostptr(struct buffer *buf, void *hostptr)
{
   return (buf->hostptr <= hostptr) && (hostptr < (buf->hostptr + buf->len));
}

uint64_t
gpuaddr(void *hostptr)
{
   rb_tree_foreach (struct buffer, buf, &buffers, node) {
      if (buffer_contains_hostptr(buf, hostptr))
         return buf->gpuaddr + (hostptr - buf->hostptr);
   }
   return 0;
}

uint64_t
gpubaseaddr(uint64_t gpuaddr)
{
   struct buffer *buf = get_buffer(gpuaddr);
   if (buf)
      return buf->gpuaddr;
   else
      return 0;
}

void *
hostptr(uint64_t gpuaddr)
{
   struct buffer *buf = get_buffer(gpuaddr);
   if (buf)
      return buf->hostptr + (gpuaddr - buf->gpuaddr);
   else
      return 0;
}

unsigned
hostlen(uint64_t gpuaddr)
{
   struct buffer *buf = get_buffer(gpuaddr);
   if (buf)
      return buf->len + buf->gpuaddr - gpuaddr;
   else
      return 0;
}

bool
has_dumped(uint64_t gpuaddr, unsigned enable_mask)
{
   if (!gpuaddr)
      return false;

   struct buffer *b = get_buffer(gpuaddr);
   if (!b)
      return false;

   assert(gpuaddr >= b->gpuaddr);
   unsigned offset = gpuaddr - b->gpuaddr;

   unsigned n = 0;
   while (n < b->noffsets) {
      if (offset == b->offsets[n].offset)
         break;
      n++;
   }

   /* if needed, allocate a new offset entry: */
   if (n == b->noffsets) {
      b->noffsets++;
      assert(b->noffsets < ARRAY_SIZE(b->offsets));
      b->offsets[n].dumped_mask = 0;
      b->offsets[n].offset = offset;
   }

   if ((b->offsets[n].dumped_mask & enable_mask) == enable_mask)
      return true;

   b->offsets[n].dumped_mask |= enable_mask;

   return false;
}

void
reset_buffers(void)
{
   rb_tree_foreach_safe (struct buffer, buf, &buffers, node) {
      rb_tree_remove(&buffers, &buf->node);
      free(buf->hostptr);
      free(buf);
   }
}

/**
 * Record buffer contents, takes ownership of hostptr (freed in
 * reset_buffers())
 */
void
add_buffer(uint64_t gpuaddr, unsigned int len, void *hostptr)
{
   struct buffer *buf = get_buffer(gpuaddr);

   if (!buf) {
      buf = calloc(sizeof(struct buffer), 1);
      buf->gpuaddr = gpuaddr;
      rb_tree_insert(&buffers, &buf->node, buffer_insert_cmp);
   }

   /* We can end up in scenarios where we capture parts of a buffer that
    * has been suballocated from twice, once as a dumped buffer and once
    * as a cmd.. possibly the kernel should get more clever about this,
    * but we need to tolerate it:
    */
   if (buf->gpuaddr != gpuaddr) {
      assert(gpuaddr > buf->gpuaddr);
      assert((gpuaddr + len) <= (buf->gpuaddr + buf->len));

      void *ptr = ((uint8_t *)buf->hostptr) + (gpuaddr - buf->gpuaddr);
      assert(!memcmp(ptr, hostptr, len));

      return;
   }

   buf->hostptr = hostptr;
   buf->len = len;
}
