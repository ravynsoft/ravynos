/*
 * Copyright © 2016-2018 Intel Corporation
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "aub_mem.h"
#include "util/anon_file.h"

struct bo_map {
   struct list_head link;
   struct intel_batch_decode_bo bo;
   bool unmap_after_use;
   bool ppgtt;
};

struct ggtt_entry {
   struct rb_node node;
   uint64_t virt_addr;
   uint64_t phys_addr;
};

struct phys_mem {
   struct rb_node node;
   uint64_t fd_offset;
   uint64_t phys_addr;
   uint8_t *data;
   const uint8_t *aub_data;
};

static void
add_gtt_bo_map(struct aub_mem *mem, struct intel_batch_decode_bo bo, bool ppgtt, bool unmap_after_use)
{
   struct bo_map *m = calloc(1, sizeof(*m));

   m->ppgtt = ppgtt;
   m->bo = bo;
   m->unmap_after_use = unmap_after_use;
   list_add(&m->link, &mem->maps);
}

void
aub_mem_clear_bo_maps(struct aub_mem *mem)
{
   list_for_each_entry_safe(struct bo_map, i, &mem->maps, link) {
      if (i->unmap_after_use)
         munmap((void *)i->bo.map, i->bo.size);
      list_del(&i->link);
      free(i);
   }
}

static inline struct ggtt_entry *
ggtt_entry_next(struct ggtt_entry *entry)
{
   if (!entry)
      return NULL;
   struct rb_node *node = rb_node_next(&entry->node);
   if (!node)
      return NULL;
   return rb_node_data(struct ggtt_entry, node, node);
}

static inline int
cmp_uint64(uint64_t a, uint64_t b)
{
   if (a < b)
      return 1;
   if (a > b)
      return -1;
   return 0;
}

static inline int
cmp_ggtt_entry(const struct rb_node *node, const void *addr)
{
   struct ggtt_entry *entry = rb_node_data(struct ggtt_entry, node, node);
   return cmp_uint64(entry->virt_addr, *(const uint64_t *)addr);
}

static struct ggtt_entry *
ensure_ggtt_entry(struct aub_mem *mem, uint64_t virt_addr)
{
   struct rb_node *node = rb_tree_search_sloppy(&mem->ggtt, &virt_addr,
                                                cmp_ggtt_entry);
   int cmp = 0;
   if (!node || (cmp = cmp_ggtt_entry(node, &virt_addr))) {
      struct ggtt_entry *new_entry = calloc(1, sizeof(*new_entry));
      new_entry->virt_addr = virt_addr;
      rb_tree_insert_at(&mem->ggtt, node, &new_entry->node, cmp < 0);
      node = &new_entry->node;
   }

   return rb_node_data(struct ggtt_entry, node, node);
}

static struct ggtt_entry *
search_ggtt_entry(struct aub_mem *mem, uint64_t virt_addr)
{
   virt_addr &= ~0xfff;

   struct rb_node *node = rb_tree_search(&mem->ggtt, &virt_addr, cmp_ggtt_entry);

   if (!node)
      return NULL;

   return rb_node_data(struct ggtt_entry, node, node);
}

static inline int
cmp_phys_mem(const struct rb_node *node, const void *addr)
{
   struct phys_mem *mem = rb_node_data(struct phys_mem, node, node);
   return cmp_uint64(mem->phys_addr, *(uint64_t *)addr);
}

static void
check_mmap_result(const void *res)
{
   if (res != MAP_FAILED)
      return;

   if (errno == ENOMEM) {
      fprintf(stderr,
            "Not enough memory available or maximum number of mappings reached. "
            "Consider increasing sysctl vm.max_map_count.\n");
   } else {
      perror("mmap");
   }

   abort();
}

static struct phys_mem *
ensure_phys_mem(struct aub_mem *mem, uint64_t phys_addr)
{
   struct rb_node *node = rb_tree_search_sloppy(&mem->mem, &phys_addr, cmp_phys_mem);
   int cmp = 0;
   if (!node || (cmp = cmp_phys_mem(node, &phys_addr))) {
      struct phys_mem *new_mem = calloc(1, sizeof(*new_mem));
      new_mem->phys_addr = phys_addr;
      new_mem->fd_offset = mem->mem_fd_len;

      ASSERTED int ftruncate_res = ftruncate(mem->mem_fd, mem->mem_fd_len += 4096);
      assert(ftruncate_res == 0);

      new_mem->data = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED,
                           mem->mem_fd, new_mem->fd_offset);
      check_mmap_result(new_mem->data);

      rb_tree_insert_at(&mem->mem, node, &new_mem->node, cmp < 0);
      node = &new_mem->node;
   }

   return rb_node_data(struct phys_mem, node, node);
}

static struct phys_mem *
search_phys_mem(struct aub_mem *mem, uint64_t phys_addr)
{
   phys_addr &= ~0xfff;

   struct rb_node *node = rb_tree_search(&mem->mem, &phys_addr, cmp_phys_mem);

   if (!node)
      return NULL;

   return rb_node_data(struct phys_mem, node, node);
}

void
aub_mem_local_write(void *_mem, uint64_t address,
                    const void *data, uint32_t size)
{
   struct aub_mem *mem = _mem;
   struct intel_batch_decode_bo bo = {
      .map = data,
      .addr = address,
      .size = size,
   };
   add_gtt_bo_map(mem, bo, false, false);
}

void
aub_mem_ggtt_entry_write(void *_mem, uint64_t address,
                         const void *_data, uint32_t _size)
{
   struct aub_mem *mem = _mem;
   uint64_t virt_addr = (address / sizeof(uint64_t)) << 12;
   const uint64_t *data = _data;
   size_t size = _size / sizeof(*data);
   for (const uint64_t *entry = data;
        entry < data + size;
        entry++, virt_addr += 4096) {
      struct ggtt_entry *pt = ensure_ggtt_entry(mem, virt_addr);
      pt->phys_addr = *entry;
   }
}

void
aub_mem_phys_write(void *_mem, uint64_t phys_address,
                   const void *data, uint32_t size)
{
   struct aub_mem *mem = _mem;
   uint32_t to_write = size;
   for (uint64_t page = phys_address & ~0xfff; page < phys_address + size; page += 4096) {
      struct phys_mem *pmem = ensure_phys_mem(mem, page);
      uint64_t offset = MAX2(page, phys_address) - page;
      uint32_t size_this_page = MIN2(to_write, 4096 - offset);
      to_write -= size_this_page;
      memcpy(pmem->data + offset, data, size_this_page);
      pmem->aub_data = data - offset;
      data = (const uint8_t *)data + size_this_page;
   }
}

void
aub_mem_ggtt_write(void *_mem, uint64_t virt_address,
                   const void *data, uint32_t size)
{
   struct aub_mem *mem = _mem;
   uint32_t to_write = size;
   for (uint64_t page = virt_address & ~0xfff; page < virt_address + size; page += 4096) {
      struct ggtt_entry *entry = search_ggtt_entry(mem, page);
      assert(entry && entry->phys_addr & 0x1);

      uint64_t offset = MAX2(page, virt_address) - page;
      uint32_t size_this_page = MIN2(to_write, 4096 - offset);
      to_write -= size_this_page;

      uint64_t phys_page = entry->phys_addr & ~0xfff; /* Clear the validity bits. */
      aub_mem_phys_write(mem, phys_page + offset, data, size_this_page);
      data = (const uint8_t *)data + size_this_page;
   }
}

struct intel_batch_decode_bo
aub_mem_get_ggtt_bo(void *_mem, uint64_t address)
{
   struct aub_mem *mem = _mem;
   struct intel_batch_decode_bo bo = {0};

   list_for_each_entry(struct bo_map, i, &mem->maps, link)
      if (!i->ppgtt && i->bo.addr <= address && i->bo.addr + i->bo.size > address)
         return i->bo;

   address &= ~0xfff;

   struct ggtt_entry *start =
      (struct ggtt_entry *)rb_tree_search_sloppy(&mem->ggtt, &address,
                                                 cmp_ggtt_entry);
   if (start && start->virt_addr < address)
      start = ggtt_entry_next(start);
   if (!start)
      return bo;

   struct ggtt_entry *last = start;
   for (struct ggtt_entry *i = ggtt_entry_next(last);
        i && last->virt_addr + 4096 == i->virt_addr;
        last = i, i = ggtt_entry_next(last))
      ;

   bo.addr = MIN2(address, start->virt_addr);
   bo.size = last->virt_addr - bo.addr + 4096;
   bo.map = mmap(NULL, bo.size, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   check_mmap_result(bo.map);

   for (struct ggtt_entry *i = start;
        i;
        i = i == last ? NULL : ggtt_entry_next(i)) {
      uint64_t phys_addr = i->phys_addr & ~0xfff;
      struct phys_mem *phys_mem = search_phys_mem(mem, phys_addr);

      if (!phys_mem)
         continue;

      uint32_t map_offset = i->virt_addr - address;
      void *res = mmap((uint8_t *)bo.map + map_offset, 4096, PROT_READ,
                  MAP_SHARED | MAP_FIXED, mem->mem_fd, phys_mem->fd_offset);
      check_mmap_result(res);
   }

   add_gtt_bo_map(mem, bo, false, true);

   return bo;
}

static struct phys_mem *
ppgtt_walk(struct aub_mem *mem, uint64_t pml4, uint64_t address)
{
   uint64_t shift = 39;
   uint64_t addr = pml4;
   for (int level = 4; level > 0; level--) {
      struct phys_mem *table = search_phys_mem(mem, addr);
      if (!table)
         return NULL;
      int index = (address >> shift) & 0x1ff;
      uint64_t entry = ((uint64_t *)table->data)[index];
      if (!(entry & 1))
         return NULL;
      addr = entry & ~0xfff;
      shift -= 9;
   }
   return search_phys_mem(mem, addr);
}

static bool
ppgtt_mapped(struct aub_mem *mem, uint64_t pml4, uint64_t address)
{
   return ppgtt_walk(mem, pml4, address) != NULL;
}

struct intel_batch_decode_bo
aub_mem_get_ppgtt_bo(void *_mem, uint64_t address)
{
   struct aub_mem *mem = _mem;
   struct intel_batch_decode_bo bo = {0};

   list_for_each_entry(struct bo_map, i, &mem->maps, link)
      if (i->ppgtt && i->bo.addr <= address && i->bo.addr + i->bo.size > address)
         return i->bo;

   address &= ~0xfff;

   if (!ppgtt_mapped(mem, mem->pml4, address))
      return bo;

   /* Map everything until the first gap since we don't know how much the
    * decoder actually needs.
    */
   uint64_t end = address;
   while (ppgtt_mapped(mem, mem->pml4, end))
      end += 4096;

   bo.addr = address;
   bo.size = end - address;
   bo.map = mmap(NULL, bo.size, PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   assert(bo.map != MAP_FAILED);

   for (uint64_t page = address; page < end; page += 4096) {
      struct phys_mem *phys_mem = ppgtt_walk(mem, mem->pml4, page);

      void *res = mmap((uint8_t *)bo.map + (page - bo.addr), 4096, PROT_READ,
                  MAP_SHARED | MAP_FIXED, mem->mem_fd, phys_mem->fd_offset);
      check_mmap_result(res);
   }

   add_gtt_bo_map(mem, bo, true, true);

   return bo;
}

bool
aub_mem_init(struct aub_mem *mem)
{
   memset(mem, 0, sizeof(*mem));

   list_inithead(&mem->maps);

   mem->mem_fd = os_create_anonymous_file(0, "phys memory");

   return mem->mem_fd != -1;
}

void
aub_mem_fini(struct aub_mem *mem)
{
   if (mem->mem_fd == -1)
      return;

   aub_mem_clear_bo_maps(mem);


   rb_tree_foreach_safe(struct ggtt_entry, entry, &mem->ggtt, node) {
      rb_tree_remove(&mem->ggtt, &entry->node);
      free(entry);
   }
   rb_tree_foreach_safe(struct phys_mem, entry, &mem->mem, node) {
      rb_tree_remove(&mem->mem, &entry->node);
      free(entry);
   }

   close(mem->mem_fd);
   mem->mem_fd = -1;
}

struct intel_batch_decode_bo
aub_mem_get_phys_addr_data(struct aub_mem *mem, uint64_t phys_addr)
{
   struct phys_mem *page = search_phys_mem(mem, phys_addr);
   return page ?
      (struct intel_batch_decode_bo) { .map = page->data, .addr = page->phys_addr, .size = 4096 } :
      (struct intel_batch_decode_bo) {};
}

struct intel_batch_decode_bo
aub_mem_get_ppgtt_addr_data(struct aub_mem *mem, uint64_t virt_addr)
{
   struct phys_mem *page = ppgtt_walk(mem, mem->pml4, virt_addr);
   return page ?
      (struct intel_batch_decode_bo) { .map = page->data, .addr = virt_addr & ~((1ULL << 12) - 1), .size = 4096 } :
      (struct intel_batch_decode_bo) {};
}

struct intel_batch_decode_bo
aub_mem_get_ppgtt_addr_aub_data(struct aub_mem *mem, uint64_t virt_addr)
{
   struct phys_mem *page = ppgtt_walk(mem, mem->pml4, virt_addr);
   return page ?
      (struct intel_batch_decode_bo) { .map = page->aub_data, .addr = virt_addr & ~((1ULL << 12) - 1), .size = 4096 } :
      (struct intel_batch_decode_bo) {};
}
