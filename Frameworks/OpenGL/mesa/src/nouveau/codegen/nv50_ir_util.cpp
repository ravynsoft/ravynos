/*
 * Copyright 2011 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nv50_ir_util.h"

namespace nv50_ir {

void DLList::clear()
{
   for (Item *next, *item = head.next; item != &head; item = next) {
      next = item->next;
      delete item;
   }
   head.next = head.prev = &head;
}

void
DLList::Iterator::erase()
{
   Item *rem = pos;

   if (rem == term)
      return;
   pos = pos->next;

   DLLIST_DEL(rem);
   delete rem;
}

void DLList::Iterator::moveToList(DLList& dest)
{
   Item *item = pos;

   assert(term != &dest.head);
   assert(pos != term);

   pos = pos->next;

   DLLIST_DEL(item);
   DLLIST_ADDHEAD(&dest.head, item);
}

bool
DLList::Iterator::insert(void *data)
{
   Item *ins = new Item(data);

   ins->next = pos->next;
   ins->prev = pos;
   pos->next->prev = ins;
   pos->next = ins;

   if (pos == term)
      term = ins;

   return true;
}

void
Stack::moveTo(Stack& that)
{
   unsigned int newSize = this->size + that.size;

   while (newSize > that.limit)
      that.resize();
   memcpy(&that.array[that.size], &array[0], this->size * sizeof(Item));

   that.size = newSize;
   this->size = 0;
}

Interval::Interval(const Interval& that) : head(NULL), tail(NULL)
{
   this->insert(that);
}

Interval::~Interval()
{
   clear();
}

void
Interval::clear()
{
   for (Range *next, *r = head; r; r = next) {
      next = r->next;
      delete r;
   }
   head = tail = NULL;
}

bool
Interval::extend(int a, int b)
{
   Range *r, **nextp = &head;

   // NOTE: we need empty intervals for fixed registers
   // if (a == b)
   //   return false;
   assert(a <= b);

   for (r = head; r; r = r->next) {
      if (b < r->bgn)
         break; // insert before
      if (a > r->end) {
         // insert after
         nextp = &r->next;
         continue;
      }

      // overlap
      if (a < r->bgn) {
         r->bgn = a;
         if (b > r->end)
            r->end = b;
         r->coalesce(&tail);
         return true;
      }
      if (b > r->end) {
         r->end = b;
         r->coalesce(&tail);
         return true;
      }
      assert(a >= r->bgn);
      assert(b <= r->end);
      return true;
   }

   (*nextp) = new Range(a, b);
   (*nextp)->next = r;

   for (r = (*nextp); r->next; r = r->next);
   tail = r;
   return true;
}

bool Interval::contains(int pos) const
{
   for (Range *r = head; r && r->bgn <= pos; r = r->next)
      if (r->end > pos)
         return true;
   return false;
}

bool Interval::overlaps(const Interval &that) const
{
#if 1
   Range *a = this->head;
   Range *b = that.head;

   while (a && b) {
      if (b->bgn < a->end &&
          b->end > a->bgn)
         return true;
      if (a->end <= b->bgn)
         a = a->next;
      else
         b = b->next;
   }
#else
   for (Range *rA = this->head; rA; rA = rA->next)
      for (Range *rB = iv.head; rB; rB = rB->next)
         if (rB->bgn < rA->end &&
             rB->end > rA->bgn)
            return true;
#endif
   return false;
}

void Interval::insert(const Interval &that)
{
   for (Range *r = that.head; r; r = r->next)
      this->extend(r->bgn, r->end);
}

void Interval::unify(Interval &that)
{
   assert(this != &that);
   for (Range *next, *r = that.head; r; r = next) {
      next = r->next;
      this->extend(r->bgn, r->end);
      delete r;
   }
   that.head = NULL;
}

int Interval::length() const
{
   int len = 0;
   for (Range *r = head; r; r = r->next)
      len += r->bgn - r->end;
   return len;
}

void Interval::print() const
{
   if (!head)
      return;
   INFO("[%i %i)", head->bgn, head->end);
   for (const Range *r = head->next; r; r = r->next)
      INFO(" [%i %i)", r->bgn, r->end);
   INFO("\n");
}

void
BitSet::andNot(const BitSet &set)
{
   assert(data && set.data);
   assert(size >= set.size);
   for (unsigned int i = 0; i < (set.size + 31) / 32; ++i)
      data[i] &= ~set.data[i];
}

BitSet& BitSet::operator|=(const BitSet &set)
{
   assert(data && set.data);
   assert(size >= set.size);
   for (unsigned int i = 0; i < (set.size + 31) / 32; ++i)
      data[i] |= set.data[i];
   return *this;
}

bool BitSet::resize(unsigned int nBits)
{
   if (!data || !nBits)
      return allocate(nBits, true);
   const unsigned int p = (size + 31) / 32;
   const unsigned int n = (nBits + 31) / 32;
   if (n == p)
      return true;

   data = (uint32_t *)REALLOC(data, 4 * p, 4 * n);
   if (!data) {
      size = 0;
      return false;
   }
   if (n > p)
      memset(&data[p], 0, (n - p) * 4);
   if (nBits < size && (nBits % 32))
      data[(nBits + 31) / 32 - 1] &= (1 << (nBits % 32)) - 1;

   size = nBits;
   return true;
}

bool BitSet::allocate(unsigned int nBits, bool zero)
{
   if (data && size < nBits) {
      FREE(data);
      data = NULL;
   }
   size = nBits;

   if (!data)
      data = reinterpret_cast<uint32_t *>(CALLOC((size + 31) / 32, 4));

   if (zero)
      memset(data, 0, (size + 7) / 8);
   else
   if (size % 32) // clear unused bits (e.g. for popCount)
      data[(size + 31) / 32 - 1] &= (1 << (size % 32)) - 1;

   return data;
}

unsigned int BitSet::popCount() const
{
   unsigned int count = 0;

   for (unsigned int i = 0; i < (size + 31) / 32; ++i)
      if (data[i])
         count += util_bitcount(data[i]);
   return count;
}

void BitSet::fill(uint32_t val)
{
   unsigned int i;
   for (i = 0; i < (size + 31) / 32; ++i)
      data[i] = val;
   if (val && i)
      data[i - 1] &= (1 << (size % 32)) - 1;
}

void BitSet::setOr(BitSet *pA, BitSet *pB)
{
   if (!pB) {
      *this = *pA;
   } else {
      for (unsigned int i = 0; i < (size + 31) / 32; ++i)
         data[i] = pA->data[i] | pB->data[i];
   }
}

int BitSet::findFreeRange(unsigned int count, unsigned int max) const
{
   const uint32_t m = (1 << count) - 1;
   int pos = max;
   unsigned int i;
   const unsigned int end = (max + 31) / 32;

   if (count == 1) {
      for (i = 0; i < end; ++i) {
         pos = ffs(~data[i]) - 1;
         if (pos >= 0)
            break;
      }
   } else
   if (count == 2) {
      for (i = 0; i < end; ++i) {
         if (data[i] != 0xffffffff) {
            uint32_t b = data[i] | (data[i] >> 1) | 0xaaaaaaaa;
            pos = ffs(~b) - 1;
            if (pos >= 0)
               break;
         }
      }
   } else
   if (count == 4 || count == 3) {
      for (i = 0; i < end; ++i) {
         if (data[i] != 0xffffffff) {
            uint32_t b =
               (data[i] >> 0) | (data[i] >> 1) |
               (data[i] >> 2) | (data[i] >> 3) | 0xeeeeeeee;
            pos = ffs(~b) - 1;
            if (pos >= 0)
               break;
         }
      }
   } else {
      if (count <= 8)
         count = 8;
      else
      if (count <= 16)
         count = 16;
      else
         count = 32;

      for (i = 0; i < end; ++i) {
         if (data[i] != 0xffffffff) {
            for (pos = 0; pos < 32; pos += count)
               if (!(data[i] & (m << pos)))
                  break;
            if (pos < 32)
               break;
         }
      }
   }

   // If we couldn't find a position, we can have a left-over -1 in pos. Make
   // sure to abort in such a case.
   if (pos < 0)
      return -1;

   pos += i * 32;

   return ((pos + count) <= max) ? pos : -1;
}

void BitSet::print() const
{
   unsigned int n = 0;
   INFO("BitSet of size %u:\n", size);
   for (unsigned int i = 0; i < (size + 31) / 32; ++i) {
      uint32_t bits = data[i];
      while (bits) {
         int pos = ffs(bits) - 1;
         bits &= ~(1 << pos);
         INFO(" %i", i * 32 + pos);
         ++n;
         if ((n % 16) == 0)
            INFO("\n");
      }
   }
   if (n % 16)
      INFO("\n");
}

} // namespace nv50_ir
