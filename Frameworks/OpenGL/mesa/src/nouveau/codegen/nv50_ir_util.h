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

#ifndef __NV50_IR_UTIL_H__
#define __NV50_IR_UTIL_H__

#include <new>
#include <assert.h>
#include <stdio.h>
#include <memory>
#include <map>

#ifndef NDEBUG
# include <typeinfo>
#endif

#include "util/u_inlines.h"
#include "util/u_memory.h"

#define ERROR(args...) _debug_printf("ERROR: " args)
#define WARN(args...) _debug_printf("WARNING: " args)
#define INFO(args...) _debug_printf(args)

#define INFO_DBG(m, f, args...)          \
   do {                                  \
      if (m & NV50_IR_DEBUG_##f)         \
         _debug_printf(args);             \
   } while(0)

#define FATAL(args...)          \
   do {                         \
      fprintf(stderr, args);    \
      abort();                  \
   } while(0)


#define NV50_IR_FUNC_ALLOC_OBJ_DEF(obj, f, args...)               \
   new ((f)->getProgram()->mem_##obj.allocate()) obj(f, args)

#define new_Instruction(f, args...)                      \
   NV50_IR_FUNC_ALLOC_OBJ_DEF(Instruction, f, args)
#define new_CmpInstruction(f, args...)                   \
   NV50_IR_FUNC_ALLOC_OBJ_DEF(CmpInstruction, f, args)
#define new_TexInstruction(f, args...)                   \
   NV50_IR_FUNC_ALLOC_OBJ_DEF(TexInstruction, f, args)
#define new_FlowInstruction(f, args...)                  \
   NV50_IR_FUNC_ALLOC_OBJ_DEF(FlowInstruction, f, args)

#define new_LValue(f, args...)                  \
   NV50_IR_FUNC_ALLOC_OBJ_DEF(LValue, f, args)


#define NV50_IR_PROG_ALLOC_OBJ_DEF(obj, p, args...)   \
   new ((p)->mem_##obj.allocate()) obj(p, args)

#define new_Symbol(p, args...)                           \
   NV50_IR_PROG_ALLOC_OBJ_DEF(Symbol, p, args)
#define new_ImmediateValue(p, args...)                   \
   NV50_IR_PROG_ALLOC_OBJ_DEF(ImmediateValue, p, args)


#define delete_Instruction(p, insn) (p)->releaseInstruction(insn)
#define delete_Value(p, val) (p)->releaseValue(val)


namespace nv50_ir {

class Iterator
{
public:
   virtual ~Iterator() { };
   virtual void next() = 0;
   virtual void *get() const = 0;
   virtual bool end() const = 0; // if true, get will return 0
   virtual void reset() { assert(0); } // only for graph iterators
};

typedef std::unique_ptr<Iterator> IteratorRef;

class ManipIterator : public Iterator
{
public:
   virtual bool insert(void *) = 0; // insert after current position
   virtual void erase() = 0;
};

// WARNING: do not use a->prev/next for __item or __list

#define DLLIST_DEL(__item)                      \
   do {                                         \
      (__item)->prev->next = (__item)->next;    \
      (__item)->next->prev = (__item)->prev;    \
      (__item)->next = (__item);                \
      (__item)->prev = (__item);                \
   } while(0)

#define DLLIST_ADDTAIL(__list, __item)          \
   do {                                         \
      (__item)->next = (__list);                \
      (__item)->prev = (__list)->prev;          \
      (__list)->prev->next = (__item);          \
      (__list)->prev = (__item);                \
   } while(0)

#define DLLIST_ADDHEAD(__list, __item)          \
   do {                                         \
      (__item)->prev = (__list);                \
      (__item)->next = (__list)->next;          \
      (__list)->next->prev = (__item);          \
      (__list)->next = (__item);                \
   } while(0)

#define DLLIST_MERGE(__listA, __listB, ty)      \
   do {                                         \
      ty prevB = (__listB)->prev;               \
      (__listA)->prev->next = (__listB);        \
      (__listB)->prev->next = (__listA);        \
      (__listB)->prev = (__listA)->prev;        \
      (__listA)->prev = prevB;                  \
   } while(0)

#define DLLIST_EMPTY(__list) ((__list)->next == (__list))

#define DLLIST_FOR_EACH(list, it) \
   for (DLList::Iterator it = (list)->iterator(); !(it).end(); (it).next())

class DLList
{
public:
   class Item
   {
   public:
      Item(void *priv) : next(this), prev(this), data(priv) { }

   public:
      Item *next;
      Item *prev;
      void *data;
   };

   DLList() : head(0) { }
   ~DLList() { clear(); }

   DLList(const DLList&) = delete;
   DLList& operator=(const DLList&) = delete;

   inline void insertHead(void *data)
   {
      Item *item = new Item(data);

      assert(data);

      item->prev = &head;
      item->next = head.next;
      head.next->prev = item;
      head.next = item;
   }

   inline void insertTail(void *data)
   {
      Item *item = new Item(data);

      assert(data);

      DLLIST_ADDTAIL(&head, item);
   }

   inline void insert(void *data) { insertTail(data); }

   void clear();

   class Iterator : public ManipIterator
   {
   public:
      Iterator(Item *head, bool r) : rev(r), pos(r ? head->prev : head->next),
                                     term(head) { }

      virtual void next() { if (!end()) pos = rev ? pos->prev : pos->next; }
      virtual void *get() const { return pos->data; }
      virtual bool end() const { return pos == term; }

      // caution: if you're at end-2 and erase it, then do next, you're at end
      virtual void erase();
      virtual bool insert(void *data);

      // move item to another list, no consistency with its iterators though
      void moveToList(DLList&);

   private:
      const bool rev;
      Item *pos;
      Item *term;

      friend class DLList;
   };

   inline void erase(Iterator& pos)
   {
      pos.erase();
   }

   Iterator iterator()
   {
      return Iterator(&head, false);
   }

   Iterator revIterator()
   {
      return Iterator(&head, true);
   }

private:
   Item head;
};

class Stack
{
public:
   class Item {
   public:
      union {
         void *p;
         int i;
         unsigned int u;
         float f;
         double d;
      } u;

      Item() { memset(&u, 0, sizeof(u)); }
   };

   Stack() : size(0), limit(0), array(0) { }
   ~Stack() { if (array) FREE(array); }

   Stack(const Stack&) = delete;
   Stack& operator=(const Stack&) = delete;

   inline void push(int i)          { Item data; data.u.i = i; push(data); }
   inline void push(unsigned int u) { Item data; data.u.u = u; push(data); }
   inline void push(void *p)        { Item data; data.u.p = p; push(data); }
   inline void push(float f)        { Item data; data.u.f = f; push(data); }

   inline void push(Item data)
   {
      if (size == limit)
         resize();
      array[size++] = data;
   }

   inline Item pop()
   {
      if (!size) {
         Item data;
         assert(0);
         return data;
      }
      return array[--size];
   }

   inline unsigned int getSize() { return size; }

   inline Item& peek() { assert(size); return array[size - 1]; }

   void clear(bool releaseStorage = false)
   {
      if (releaseStorage && array)
         FREE(array);
      size = limit = 0;
   }

   void moveTo(Stack&); // move all items to target (not like push(pop()))

private:
   void resize()
   {
         unsigned int sizeOld, sizeNew;

         sizeOld = limit * sizeof(Item);
         limit = MAX2(4, limit + limit);
         sizeNew = limit * sizeof(Item);

         array = (Item *)REALLOC(array, sizeOld, sizeNew);
   }

   unsigned int size;
   unsigned int limit;
   Item *array;
};

class DynArray
{
public:
   class Item
   {
   public:
      union {
         uint32_t u32;
         void *p;
      };
   };

   DynArray() : data(NULL), size(0) { }

   ~DynArray() { if (data) FREE(data); }

   DynArray(const DynArray&) = delete;
   DynArray& operator=(const DynArray&) = delete;

   inline Item& operator[](unsigned int i)
   {
      if (i >= size)
         resize(i);
      return data[i];
   }

   inline const Item operator[](unsigned int i) const
   {
      return data[i];
   }

   void resize(unsigned int index)
   {
      const unsigned int oldSize = size * sizeof(Item);

      if (!size)
         size = 8;
      while (size <= index)
         size <<= 1;

      data = (Item *)REALLOC(data, oldSize, size * sizeof(Item));
   }

   void clear()
   {
      FREE(data);
      data = NULL;
      size = 0;
   }

private:
   Item *data;
   unsigned int size;
};

class ArrayList
{
public:
   ArrayList() : size(0) { }

   void insert(void *item, int& id)
   {
      id = ids.getSize() ? ids.pop().u.i : size++;
      data[id].p = item;
   }

   void remove(int& id)
   {
      const unsigned int uid = id;
      assert(uid < size && data[id].p);
      ids.push(uid);
      data[uid].p = NULL;
      id = -1;
   }

   inline unsigned int getSize() const { return size; }

   inline void *get(unsigned int id) { assert(id < size); return data[id].p; }

   class Iterator : public nv50_ir::Iterator
   {
   public:
      Iterator(const ArrayList *array) : pos(0), data(array->data)
      {
         size = array->getSize();
         if (size)
            nextValid();
      }

      void nextValid() { while ((pos < size) && !data[pos].p) ++pos; }

      void next() { if (pos < size) { ++pos; nextValid(); } }
      void *get() const { assert(pos < size); return data[pos].p; }
      bool end() const { return pos >= size; }

   private:
      unsigned int pos;
      unsigned int size;
      const DynArray& data;

      friend class ArrayList;
   };

   Iterator iterator() const { return Iterator(this); }

   void clear()
   {
      data.clear();
      ids.clear(true);
      size = 0;
   }

private:
   DynArray data;
   Stack ids;
   unsigned int size;
};

class Interval
{
public:
   Interval() : head(0), tail(0) { }
   Interval(const Interval&);
   ~Interval();

   Interval& operator=(const Interval&) = delete;

   bool extend(int, int);
   void insert(const Interval&);
   void unify(Interval&); // clears source interval
   void clear();

   inline int begin() const { return head ? head->bgn : -1; }
   inline int end() const { checkTail(); return tail ? tail->end : -1; }
   inline bool isEmpty() const { return !head; }
   bool overlaps(const Interval&) const;
   bool contains(int pos) const;

   inline int extent() const { return end() - begin(); }
   int length() const;

   void print() const;

   inline void checkTail() const;

private:
   class Range
   {
   public:
      Range(int a, int b) : next(0), bgn(a), end(b) { }

      Range *next;
      int bgn;
      int end;

      void coalesce(Range **ptail)
      {
         Range *rnn;

         while (next && end >= next->bgn) {
            assert(bgn <= next->bgn);
            rnn = next->next;
            end = MAX2(end, next->end);
            delete next;
            next = rnn;
         }
         if (!next)
            *ptail = this;
      }
   };

   Range *head;
   Range *tail;
};

class BitSet
{
public:
   BitSet() : marker(false), data(0), size(0) { }
   BitSet(unsigned int nBits, bool zero) : marker(false), data(0), size(0)
   {
      allocate(nBits, zero);
   }
   ~BitSet()
   {
      if (data)
         FREE(data);
   }

   BitSet(const BitSet&) = delete;

   // allocate will keep old data iff size is unchanged
   bool allocate(unsigned int nBits, bool zero);
   bool resize(unsigned int nBits); // keep old data, zero additional bits

   inline unsigned int getSize() const { return size; }

   void fill(uint32_t val);

   void setOr(BitSet *, BitSet *); // second BitSet may be NULL

   inline void set(unsigned int i)
   {
      assert(i < size);
      data[i / 32] |= 1 << (i % 32);
   }
   // NOTE: range may not cross 32 bit boundary (implies n <= 32)
   inline void setRange(unsigned int i, unsigned int n)
   {
      assert((i + n) <= size && (((i % 32) + n) <= 32));
      data[i / 32] |= ((1 << n) - 1) << (i % 32);
   }
   inline void setMask(unsigned int i, uint32_t m)
   {
      assert(i < size);
      data[i / 32] |= m;
   }

   inline void clr(unsigned int i)
   {
      assert(i < size);
      data[i / 32] &= ~(1 << (i % 32));
   }
   // NOTE: range may not cross 32 bit boundary (implies n <= 32)
   inline void clrRange(unsigned int i, unsigned int n)
   {
      assert((i + n) <= size && (((i % 32) + n) <= 32));
      data[i / 32] &= ~(((1 << n) - 1) << (i % 32));
   }

   inline bool test(unsigned int i) const
   {
      assert(i < size);
      return data[i / 32] & (1 << (i % 32));
   }
   // NOTE: range may not cross 32 bit boundary (implies n <= 32)
   inline bool testRange(unsigned int i, unsigned int n) const
   {
      assert((i + n) <= size && (((i % 32) + n) <= 32));
      return data[i / 32] & (((1 << n) - 1) << (i % 32));
   }

   // Find a range of count (<= 32) clear bits aligned to roundup_pow2(count).
   int findFreeRange(unsigned int count, unsigned int max) const;
   inline int findFreeRange(unsigned int count) const {
      return findFreeRange(count, size);
   }

   BitSet& operator|=(const BitSet&);

   BitSet& operator=(const BitSet& set)
   {
      assert(data && set.data);
      assert(size == set.size);
      memcpy(data, set.data, (set.size + 7) / 8);
      return *this;
   }

   void andNot(const BitSet&);

   unsigned int popCount() const;

   void print() const;

public:
   bool marker; // for user

private:
   uint32_t *data;
   unsigned int size;
};

void Interval::checkTail() const
{
#if NV50_DEBUG & NV50_DEBUG_PROG_RA
   Range *r = head;
   while (r->next)
      r = r->next;
   assert(tail == r);
#endif
}

class MemoryPool
{
private:
   inline bool enlargeAllocationsArray(const unsigned int id, unsigned int nr)
   {
      const unsigned int size = sizeof(uint8_t *) * id;
      const unsigned int incr = sizeof(uint8_t *) * nr;

      uint8_t **alloc = (uint8_t **)REALLOC(allocArray, size, size + incr);
      if (!alloc)
         return false;
      allocArray = alloc;
      return true;
   }

   inline bool enlargeCapacity()
   {
      const unsigned int id = count >> objStepLog2;

      uint8_t *const mem = (uint8_t *)MALLOC(objSize << objStepLog2);
      if (!mem)
         return false;

      if (!(id % 32)) {
         if (!enlargeAllocationsArray(id, 32)) {
            FREE(mem);
            return false;
         }
      }
      allocArray[id] = mem;
      return true;
   }

public:
   MemoryPool(unsigned int size, unsigned int incr) : objSize(size),
                                                      objStepLog2(incr)
   {
      allocArray = NULL;
      released = NULL;
      count = 0;
   }

   ~MemoryPool()
   {
      unsigned int allocCount = (count + (1 << objStepLog2) - 1) >> objStepLog2;
      for (unsigned int i = 0; i < allocCount && allocArray[i]; ++i)
         FREE(allocArray[i]);
      if (allocArray)
         FREE(allocArray);
   }

   MemoryPool(const MemoryPool&) = delete;
   MemoryPool& operator=(const MemoryPool&) = delete;

   void *allocate()
   {
      void *ret;
      const unsigned int mask = (1 << objStepLog2) - 1;

      if (released) {
         ret = released;
         released = *(void **)released;
         return ret;
      }

      if (!(count & mask))
         if (!enlargeCapacity())
            return NULL;

      ret = allocArray[count >> objStepLog2] + (count & mask) * objSize;
      ++count;
      return ret;
   }

   void release(void *ptr)
   {
      *(void **)ptr = released;
      released = ptr;
   }

private:
   uint8_t **allocArray; // array (list) of MALLOC allocations

   void *released; // list of released objects

   unsigned int count; // highest allocated object

   const unsigned int objSize;
   const unsigned int objStepLog2;
};

/**
 *  Composite object cloning policy.
 *
 *  Encapsulates how sub-objects are to be handled (if at all) when a
 *  composite object is being cloned.
 */
template<typename C>
class ClonePolicy
{
protected:
   C *c;

public:
   ClonePolicy(C *c) : c(c) {}

   C *context() { return c; }

   template<typename T> T *get(T *obj)
   {
      void *clone = lookup(obj);
      if (!clone)
         clone = obj->clone(*this);
      return reinterpret_cast<T *>(clone);
   }

   template<typename T> void set(const T *obj, T *clone)
   {
      insert(obj, clone);
   }

protected:
   virtual void *lookup(void *obj) = 0;
   virtual void insert(const void *obj, void *clone) = 0;
};

/**
 *  Shallow non-recursive cloning policy.
 *
 *  Objects cloned with the "shallow" policy don't clone their
 *  children recursively, instead, the new copy shares its children
 *  with the original object.
 */
template<typename C>
class ShallowClonePolicy : public ClonePolicy<C>
{
public:
   ShallowClonePolicy(C *c) : ClonePolicy<C>(c) {}

protected:
   virtual void *lookup(void *obj)
   {
      return obj;
   }

   virtual void insert(const void *obj, void *clone)
   {
   }
};

template<typename C, typename T>
inline T *cloneShallow(C *c, T *obj)
{
   ShallowClonePolicy<C> pol(c);
   return obj->clone(pol);
}

/**
 *  Recursive cloning policy.
 *
 *  Objects cloned with the "deep" policy clone their children
 *  recursively, keeping track of what has already been cloned to
 *  avoid making several new copies of the same object.
 */
template<typename C>
class DeepClonePolicy : public ClonePolicy<C>
{
public:
   DeepClonePolicy(C *c) : ClonePolicy<C>(c) {}

private:
   std::map<const void *, void *> map;

protected:
   virtual void *lookup(void *obj)
   {
      return map[obj];
   }

   virtual void insert(const void *obj, void *clone)
   {
      map[obj] = clone;
   }
};

} // namespace nv50_ir

#endif // __NV50_IR_UTIL_H__
