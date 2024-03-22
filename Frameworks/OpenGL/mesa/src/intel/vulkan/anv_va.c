/*
 * Copyright © 2023 Intel Corporation
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

#include "anv_private.h"

#include "util/u_math.h"

static uint64_t
va_add(struct anv_va_range *range, uint64_t addr, uint64_t size)
{
   range->addr = addr;
   range->size = size;

   return addr + size;
}

static void
va_at(struct anv_va_range *range, uint64_t addr, uint64_t size)
{
   range->addr = addr;
   range->size = size;
}

static void
anv_device_print_vas(struct anv_physical_device *device)
{
   fprintf(stderr, "Driver heaps:\n");
#define PRINT_HEAP(name) \
   fprintf(stderr, "   0x%016"PRIx64"-0x%016"PRIx64": %s\n", \
           device->va.name.addr, \
           device->va.name.addr + device->va.name.size, \
           #name);
   PRINT_HEAP(general_state_pool);
   PRINT_HEAP(low_heap);
   PRINT_HEAP(dynamic_state_pool);
   PRINT_HEAP(sampler_state_pool);
   PRINT_HEAP(binding_table_pool);
   PRINT_HEAP(internal_surface_state_pool);
   PRINT_HEAP(scratch_surface_state_pool);
   PRINT_HEAP(bindless_surface_state_pool);
   PRINT_HEAP(indirect_descriptor_pool);
   PRINT_HEAP(indirect_push_descriptor_pool);
   PRINT_HEAP(instruction_state_pool);
   PRINT_HEAP(high_heap);
   PRINT_HEAP(trtt);
}

void
anv_physical_device_init_va_ranges(struct anv_physical_device *device)
{
   /* anv Virtual Memory Layout
    * =========================
    *
    * When the anv driver is determining the virtual graphics addresses of
    * memory objects itself using the softpin mechanism, the following memory
    * ranges will be used.
    *
    * Three special considerations to notice:
    *
    * (1) the dynamic state pool is located within the same 4 GiB as the low
    * heap. This is to work around a VF cache issue described in a comment in
    * anv_physical_device_init_heaps.
    *
    * (2) the binding table pool is located at lower addresses than the BT
    * (binding table) surface state pool, within a 4 GiB range which also
    * contains the bindless surface state pool. This allows surface state base
    * addresses to cover both binding tables (16 bit offsets), the internal
    * surface states (32 bit offsets) and the bindless surface states.
    *
    * (3) the last 4 GiB of the address space is withheld from the high heap.
    * Various hardware units will read past the end of an object for various
    * reasons. This healthy margin prevents reads from wrapping around 48-bit
    * addresses.
    */
   uint64_t _1Mb = 1ull * 1024 * 1024;
   uint64_t _1Gb = 1ull * 1024 * 1024 * 1024;
   uint64_t _4Gb = 4ull * 1024 * 1024 * 1024;

   uint64_t address = 0x000000200000ULL; /* 2MiB */

   address = va_add(&device->va.general_state_pool, address,
                    _1Gb - address);

   address = va_add(&device->va.low_heap, address, _1Gb);

   /* The binding table pool has to be located directly in front of the
    * surface states.
    */
   address += _1Gb;
   address = va_add(&device->va.binding_table_pool, address, _1Gb);
   address = va_add(&device->va.internal_surface_state_pool, address, 1 * _1Gb);
   assert(device->va.internal_surface_state_pool.addr ==
          align64(device->va.internal_surface_state_pool.addr, 2 * _1Gb));
   /* Scratch surface state overlaps with the internal surface state */
   va_at(&device->va.scratch_surface_state_pool,
         device->va.internal_surface_state_pool.addr,
         8 * _1Mb);
   address = va_add(&device->va.bindless_surface_state_pool, address, 2 * _1Gb);


   /* PRMs & simulation disagrees on the actual size of this heap. Take the
    * smallest (simulation) so that it works everywhere.
    */
   address = align64(address, _4Gb);
   address = va_add(&device->va.dynamic_state_pool, address, _1Gb);
   address = va_add(&device->va.sampler_state_pool, address, 2 * _1Gb);

   if (device->indirect_descriptors) {
      /* With indirect descriptors, descriptor buffers can go anywhere, they
       * just need to be in a 4Gb aligned range, so all shader accesses can
       * use a relocatable upper dword for the 64bit address.
       */
      address = align64(address, _4Gb);
      address = va_add(&device->va.indirect_descriptor_pool, address, 3 * _1Gb);
      address = va_add(&device->va.indirect_push_descriptor_pool, address, _1Gb);
   }

   /* We use a trick to compute constant data offsets in the shaders to avoid
    * unnecessary 64bit address computations (see lower_load_constant() in
    * anv_nir_apply_pipeline_layout.c). This assumes the instruction pool is
    * located at an address with the lower 32bits at 0.
    */
   address = align64(address, _4Gb);
   address = va_add(&device->va.instruction_state_pool, address, 2 * _1Gb);

   /* What's left to do for us is to set va.high_heap and va.trtt without
    * overlap, but there are a few things to be considered:
    *
    * The TR-TT address space is governed by the GFX_TRTT_VA_RANGE register,
    * which carves out part of the address space for TR-TT and is independent
    * of device->gtt_size. We use 47:44 for gen9+, the values we set here
    * should be in sync with what we write to the register.
    *
    * If we ever gain the capability to use more than 48 bits of address space
    * we'll have to adjust where we put the TR-TT space (and how we set
    * GFX_TRTT_VA_RANGE).
    *
    * We have to leave the last 4GiB out of the high vma range, so that no
    * state base address + size can overflow 48 bits. For more information see
    * the comment about Wa32bitGeneralStateOffset in anv_allocator.c
    *
    * Despite the comment above, before we had TR-TT we were not only avoiding
    * the last 4GiB of the 48bit address space, but also avoiding the last
    * 4GiB from gtt_size, so let's be on the safe side and do the 4GiB
    * avoiding for both the TR-TT space top and the gtt top.
    */
   assert(device->gtt_size <= (1uLL << 48));
   uint64_t trtt_start = 0xFuLL << 44;
   uint64_t trtt_end = (1uLL << 48) - 4 * _1Gb;
   uint64_t addressable_top = MIN2(device->gtt_size, trtt_start) - 4 * _1Gb;

   uint64_t user_heaps_size = addressable_top - address;
   address = va_add(&device->va.high_heap, address, user_heaps_size);
   assert(address <= trtt_start);
   address = va_add(&device->va.trtt, trtt_start, trtt_end - trtt_start);

   if (INTEL_DEBUG(DEBUG_HEAPS))
      anv_device_print_vas(device);
}
