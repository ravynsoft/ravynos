/*
 * Copyright 2016 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS, AUTHORS
 * AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "pb_slab.h"

#include "util/u_math.h"
#include "util/u_memory.h"

/* All slab allocations from the same heap and with the same size belong
 * to the same group.
 */
struct pb_slab_group
{
   /* Slabs with allocation candidates. Typically, slabs in this list should
    * have some free entries.
    *
    * However, when the head becomes full we purposefully keep it around
    * until the next allocation attempt, at which time we try a reclaim.
    * The intention is to keep serving allocations from the same slab as long
    * as possible for better locality.
    *
    * Due to a race in new slab allocation, additional slabs in this list
    * can be fully allocated as well.
    */
   struct list_head slabs;
};


static void
pb_slab_reclaim(struct pb_slabs *slabs, struct pb_slab_entry *entry)
{
   struct pb_slab *slab = entry->slab;

   list_del(&entry->head); /* remove from reclaim list */
   list_add(&entry->head, &slab->free);
   slab->num_free++;

   /* Add slab to the group's list if it isn't already linked. */
   if (!list_is_linked(&slab->head)) {
      struct pb_slab_group *group = &slabs->groups[entry->slab->group_index];
      list_addtail(&slab->head, &group->slabs);
   }

   if (slab->num_free >= slab->num_entries) {
      list_del(&slab->head);
      slabs->slab_free(slabs->priv, slab);
   }
}

#define MAX_FAILED_RECLAIMS 2

static unsigned
pb_slabs_reclaim_locked(struct pb_slabs *slabs)
{
   struct pb_slab_entry *entry, *next;
   unsigned num_failed_reclaims = 0;
   unsigned num_reclaims = 0;
   LIST_FOR_EACH_ENTRY_SAFE(entry, next, &slabs->reclaim, head) {
      if (slabs->can_reclaim(slabs->priv, entry)) {
         pb_slab_reclaim(slabs, entry);
         num_reclaims++;
      /* there are typically three possible scenarios when reclaiming:
       * - all entries reclaimed
       * - no entries reclaimed
       * - all but one entry reclaimed
       * in the scenario where a slab contains many (10+) unused entries,
       * the driver should not walk the entire list, as this is likely to
       * result in zero reclaims if the first few entries fail to reclaim
       */
      } else if (++num_failed_reclaims >= MAX_FAILED_RECLAIMS) {
         break;
      }
   }
   return num_reclaims;
}

static unsigned
pb_slabs_reclaim_all_locked(struct pb_slabs *slabs)
{
   struct pb_slab_entry *entry, *next;
   unsigned num_reclaims = 0;
   LIST_FOR_EACH_ENTRY_SAFE(entry, next, &slabs->reclaim, head) {
      if (slabs->can_reclaim(slabs->priv, entry)) {
         pb_slab_reclaim(slabs, entry);
         num_reclaims++;
      }
   }
   return num_reclaims;
}

/* Allocate a slab entry of the given size from the given heap.
 *
 * This will try to re-use entries that have previously been freed. However,
 * if no entries are free (or all free entries are still "in flight" as
 * determined by the can_reclaim fallback function), a new slab will be
 * requested via the slab_alloc callback.
 *
 * Note that slab_free can also be called by this function.
 */
struct pb_slab_entry *
pb_slab_alloc_reclaimed(struct pb_slabs *slabs, unsigned size, unsigned heap, bool reclaim_all)
{
   unsigned order = MAX2(slabs->min_order, util_logbase2_ceil(size));
   unsigned group_index;
   struct pb_slab_group *group;
   struct pb_slab *slab;
   struct pb_slab_entry *entry;
   unsigned entry_size = 1 << order;
   bool three_fourths = false;

   /* If the size is <= 3/4 of the entry size, use a slab with entries using
    * 3/4 sizes to reduce overallocation.
    */
   if (slabs->allow_three_fourths_allocations && size <= entry_size * 3 / 4) {
      entry_size = entry_size * 3 / 4;
      three_fourths = true;
   }

   assert(order < slabs->min_order + slabs->num_orders);
   assert(heap < slabs->num_heaps);

   group_index = (heap * slabs->num_orders + (order - slabs->min_order)) *
                 (1 + slabs->allow_three_fourths_allocations) + three_fourths;
   group = &slabs->groups[group_index];

   simple_mtx_lock(&slabs->mutex);

   /* If there is no candidate slab at all, or the first slab has no free
    * entries, try reclaiming entries.
    */
   if (list_is_empty(&group->slabs) ||
       list_is_empty(&list_entry(group->slabs.next, struct pb_slab, head)->free)) {
      if (reclaim_all)
         pb_slabs_reclaim_all_locked(slabs);
      else
         pb_slabs_reclaim_locked(slabs);
   }

   /* Remove slabs without free entries. */
   while (!list_is_empty(&group->slabs)) {
      slab = list_entry(group->slabs.next, struct pb_slab, head);
      if (!list_is_empty(&slab->free))
         break;

      list_del(&slab->head);
   }

   if (list_is_empty(&group->slabs)) {
      /* Drop the mutex temporarily to prevent a deadlock where the allocation
       * calls back into slab functions (most likely to happen for
       * pb_slab_reclaim if memory is low).
       *
       * There's a chance that racing threads will end up allocating multiple
       * slabs for the same group, but that doesn't hurt correctness.
       */
      simple_mtx_unlock(&slabs->mutex);
      slab = slabs->slab_alloc(slabs->priv, heap, entry_size, group_index);
      if (!slab)
         return NULL;
      simple_mtx_lock(&slabs->mutex);

      list_add(&slab->head, &group->slabs);
   }

   entry = list_entry(slab->free.next, struct pb_slab_entry, head);
   list_del(&entry->head);
   slab->num_free--;

   simple_mtx_unlock(&slabs->mutex);

   return entry;
}

struct pb_slab_entry *
pb_slab_alloc(struct pb_slabs *slabs, unsigned size, unsigned heap)
{
   return pb_slab_alloc_reclaimed(slabs, size, heap, false);
}

/* Free the given slab entry.
 *
 * The entry may still be in use e.g. by in-flight command submissions. The
 * can_reclaim callback function will be called to determine whether the entry
 * can be handed out again by pb_slab_alloc.
 */
void
pb_slab_free(struct pb_slabs* slabs, struct pb_slab_entry *entry)
{
   simple_mtx_lock(&slabs->mutex);
   list_addtail(&entry->head, &slabs->reclaim);
   simple_mtx_unlock(&slabs->mutex);
}

/* Check if any of the entries handed to pb_slab_free are ready to be re-used.
 *
 * This may end up freeing some slabs and is therefore useful to try to reclaim
 * some no longer used memory. However, calling this function is not strictly
 * required since pb_slab_alloc will eventually do the same thing.
 */
unsigned
pb_slabs_reclaim(struct pb_slabs *slabs)
{
   unsigned num_reclaims;
   simple_mtx_lock(&slabs->mutex);
   num_reclaims = pb_slabs_reclaim_locked(slabs);
   simple_mtx_unlock(&slabs->mutex);
   return num_reclaims;
}

/* Initialize the slabs manager.
 *
 * The minimum and maximum size of slab entries are 2^min_order and
 * 2^max_order, respectively.
 *
 * priv will be passed to the given callback functions.
 */
bool
pb_slabs_init(struct pb_slabs *slabs,
              unsigned min_order, unsigned max_order,
              unsigned num_heaps, bool allow_three_fourth_allocations,
              void *priv,
              slab_can_reclaim_fn *can_reclaim,
              slab_alloc_fn *slab_alloc,
              slab_free_fn *slab_free)
{
   unsigned num_groups;
   unsigned i;

   assert(min_order <= max_order);
   assert(max_order < sizeof(unsigned) * 8 - 1);

   slabs->min_order = min_order;
   slabs->num_orders = max_order - min_order + 1;
   slabs->num_heaps = num_heaps;
   slabs->allow_three_fourths_allocations = allow_three_fourth_allocations;

   slabs->priv = priv;
   slabs->can_reclaim = can_reclaim;
   slabs->slab_alloc = slab_alloc;
   slabs->slab_free = slab_free;

   list_inithead(&slabs->reclaim);

   num_groups = slabs->num_orders * slabs->num_heaps *
                (1 + allow_three_fourth_allocations);
   slabs->groups = CALLOC(num_groups, sizeof(*slabs->groups));
   if (!slabs->groups)
      return false;

   for (i = 0; i < num_groups; ++i) {
      struct pb_slab_group *group = &slabs->groups[i];
      list_inithead(&group->slabs);
   }

   (void) simple_mtx_init(&slabs->mutex, mtx_plain);

   return true;
}

/* Shutdown the slab manager.
 *
 * This will free all allocated slabs and internal structures, even if some
 * of the slab entries are still in flight (i.e. if can_reclaim would return
 * false).
 */
void
pb_slabs_deinit(struct pb_slabs *slabs)
{
   /* Reclaim all slab entries (even those that are still in flight). This
    * implicitly calls slab_free for everything.
    */
   while (!list_is_empty(&slabs->reclaim)) {
      struct pb_slab_entry *entry =
         list_entry(slabs->reclaim.next, struct pb_slab_entry, head);
      pb_slab_reclaim(slabs, entry);
   }

   FREE(slabs->groups);
   simple_mtx_destroy(&slabs->mutex);
}
