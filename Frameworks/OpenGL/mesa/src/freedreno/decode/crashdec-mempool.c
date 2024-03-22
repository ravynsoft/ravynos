/*
 * Copyright © 2020 Valve Corporation
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


#include "crashdec.h"


static void
dump_mem_pool_reg_write(unsigned reg, uint32_t data, unsigned context,
                        bool pipe)
{
   /* TODO deal better somehow w/ 64b regs: */
   struct regacc r = {
         .rnn = pipe ? rnn_pipe : NULL,
         .regbase = reg,
         .value = data,
   };
   if (pipe) {
      struct rnndecaddrinfo *info = rnn_reginfo(rnn_pipe, reg);
      printf("\t\twrite %s (%02x) pipe\n", info->name, reg);

      if (!strcmp(info->typeinfo->name, "void")) {
         /* registers that ignore their payload */
      } else {
         printf("\t\t\t");
         dump_register(&r);
      }
      rnn_reginfo_free(info);
   } else {
      printf("\t\twrite %s (%05x) context %d\n", regname(reg, 1), reg, context);
      dump_register_val(&r, 2);
   }
}

static void
dump_mem_pool_chunk(const uint32_t *chunk)
{
   struct __attribute__((packed)) {
      bool reg0_enabled : 1;
      bool reg1_enabled : 1;
      uint32_t data0 : 32;
      uint32_t data1 : 32;
      uint32_t reg0 : 18;
      uint32_t reg1 : 18;
      bool reg0_pipe : 1;
      bool reg1_pipe : 1;
      uint32_t reg0_context : 1;
      uint32_t reg1_context : 1;
      uint32_t padding : 22;
   } fields;

   memcpy(&fields, chunk, 4 * sizeof(uint32_t));

   if (fields.reg0_enabled) {
      dump_mem_pool_reg_write(fields.reg0, fields.data0, fields.reg0_context,
                              fields.reg0_pipe);
   }

   if (fields.reg1_enabled) {
      dump_mem_pool_reg_write(fields.reg1, fields.data1, fields.reg1_context,
                              fields.reg1_pipe);
   }
}

void
dump_cp_mem_pool(uint32_t *mempool)
{
   /* The mem pool is a shared pool of memory used for storing in-flight
    * register writes. There are 6 different queues, one for each
    * cluster. Writing to $data (or for some special registers, $addr)
    * pushes data onto the appropriate queue, and each queue is pulled
    * from by the appropriate cluster. The queues are thus written to
    * in-order, but may be read out-of-order.
    *
    * The queues are conceptually divided into 128-bit "chunks", and the
    * read and write pointers are in units of chunks.  These chunks are
    * organized internally into 8-chunk "blocks", and memory is allocated
    * dynamically in terms of blocks. Each queue is represented as a
    * singly-linked list of blocks, as well as 3-bit start/end chunk
    * pointers that point within the first/last block.  The next pointers
    * are located in a separate array, rather than inline.
    */

   /* TODO: The firmware CP_MEM_POOL save/restore routines do something
    * like:
    *
    * cread $02, [ $00 + 0 ]
    * and $02, $02, 0x118
    * ...
    * brne $02, 0, #label
    * mov $03, 0x2000
    * mov $03, 0x1000
    * label:
    * ...
    *
    * I think that control register 0 is the GPU version, and some
    * versions have a smaller mem pool. It seems some models have a mem
    * pool that's half the size, and a bunch of offsets are shifted
    * accordingly. Unfortunately the kernel driver's dumping code doesn't
    * seem to take this into account, even the downstream android driver,
    * and we don't know which versions 0x8, 0x10, or 0x100 correspond
    * to. Or maybe we can use CP_DBG_MEM_POOL_SIZE to figure this out?
    */
   bool small_mem_pool = false;

   /* The array of next pointers for each block. */
   const uint32_t *next_pointers =
      small_mem_pool ? &mempool[0x800] : &mempool[0x1000];

   /* Maximum number of blocks in the pool, also the size of the pointers
    * array.
    */
   const int num_blocks = small_mem_pool ? 0x30 : 0x80;

   /* Number of queues */
   const unsigned num_queues = 6;

   /* Unfortunately the per-queue state is a little more complicated than
    * a simple pair of begin/end pointers. Instead of a single beginning
    * block, there are *two*, with the property that either the two are
    * equal or the second is the "next" of the first. Similarly there are
    * two end blocks. Thus the queue either looks like this:
    *
    * A -> B -> ... -> C -> D
    *
    * Or like this, or some combination:
    *
    * A/B -> ... -> C/D
    *
    * However, there's only one beginning/end chunk offset. Now the
    * question is, which of A or B is the actual start? I.e. is the chunk
    * offset an offset inside A or B? It depends. I'll show a typical read
    * cycle, starting here (read pointer marked with a *) with a chunk
    * offset of 0:
    *
    *	  A                    B
    *  _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _
    * |_|_|_|_|_|_|_|_| -> |*|_|_|_|_|_|_|_| -> |_|_|_|_|_|_|_|_|
    *
    * Once the pointer advances far enough, the hardware decides to free
    * A, after which the read-side state looks like:
    *
    *	(free)                A/B
    *  _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _
    * |_|_|_|_|_|_|_|_|    |_|_|_|*|_|_|_|_| -> |_|_|_|_|_|_|_|_|
    *
    * Then after advancing the pointer a bit more, the hardware fetches
    * the "next" pointer for A and stores it in B:
    *
    *	(free)                 A                     B
    *  _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _
    * |_|_|_|_|_|_|_|_|    |_|_|_|_|_|_|_|*| -> |_|_|_|_|_|_|_|_|
    *
    * Then the read pointer advances into B, at which point we've come
    * back to the first state having advanced a whole block:
    *
    *	(free)                 A                     B
    *  _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _      _ _ _ _ _ _ _ _
    * |_|_|_|_|_|_|_|_|    |_|_|_|_|_|_|_|_| -> |*|_|_|_|_|_|_|_|
    *
    *
    * There is a similar cycle for the write pointer. Now, the question
    * is, how do we know which state we're in? We need to know this to
    * know whether the pointer (*) is in A or B if they're different. It
    * seems like there should be some bit somewhere describing this, but
    * after lots of experimentation I've come up empty-handed. For now we
    * assume that if the pointer is in the first half, then we're in
    * either the first or second state and use B, and otherwise we're in
    * the second or third state and use A. So far I haven't seen anything
    * that violates this assumption.
    */

   struct {
      uint32_t unk0;
      uint32_t padding0[7]; /* Mirrors of unk0 */

      struct {
         uint32_t chunk : 3;
         uint32_t first_block : 32 - 3;
      } writer[6];
      uint32_t padding1[2]; /* Mirrors of writer[4], writer[5] */

      uint32_t unk1;
      uint32_t padding2[7]; /* Mirrors of unk1 */

      uint32_t writer_second_block[6];
      uint32_t padding3[2];

      uint32_t unk2[6];
      uint32_t padding4[2];

      struct {
         uint32_t chunk : 3;
         uint32_t first_block : 32 - 3;
      } reader[6];
      uint32_t padding5[2]; /* Mirrors of reader[4], reader[5] */

      uint32_t unk3;
      uint32_t padding6[7]; /* Mirrors of unk3 */

      uint32_t reader_second_block[6];
      uint32_t padding7[2];

      uint32_t block_count[6];
      uint32_t padding[2];

      uint32_t unk4;
      uint32_t padding9[7]; /* Mirrors of unk4 */
   } data1;

   const uint32_t *data1_ptr =
      small_mem_pool ? &mempool[0xc00] : &mempool[0x1800];
   memcpy(&data1, data1_ptr, sizeof(data1));

   /* Based on the kernel, the first dword is the mem pool size (in
    * blocks?) and mirrors CP_MEM_POOL_DBG_SIZE.
    */
   const uint32_t *data2_ptr =
      small_mem_pool ? &mempool[0x1000] : &mempool[0x2000];
   const int data2_size = 0x60;

   /* This seems to be the size of each queue in chunks. */
   const uint32_t *queue_sizes = &data2_ptr[0x18];

   printf("\tdata2:\n");
   dump_hex_ascii(data2_ptr, 4 * data2_size, 1);

   /* These seem to be some kind of counter of allocated/deallocated blocks */
   if (verbose) {
      printf("\tunk0: %x\n", data1.unk0);
      printf("\tunk1: %x\n", data1.unk1);
      printf("\tunk3: %x\n", data1.unk3);
      printf("\tunk4: %x\n\n", data1.unk4);
   }

   for (int queue = 0; queue < num_queues; queue++) {
      const char *cluster_names[6] = {"FE",   "SP_VS", "PC_VS",
                                      "GRAS", "SP_PS", "PS"};
      printf("\tCLUSTER_%s:\n\n", cluster_names[queue]);

      if (verbose) {
         printf("\t\twriter_first_block: 0x%x\n",
                data1.writer[queue].first_block);
         printf("\t\twriter_second_block: 0x%x\n",
                data1.writer_second_block[queue]);
         printf("\t\twriter_chunk: %d\n", data1.writer[queue].chunk);
         printf("\t\treader_first_block: 0x%x\n",
                data1.reader[queue].first_block);
         printf("\t\treader_second_block: 0x%x\n",
                data1.reader_second_block[queue]);
         printf("\t\treader_chunk: %d\n", data1.reader[queue].chunk);
         printf("\t\tblock_count: %d\n", data1.block_count[queue]);
         printf("\t\tunk2: 0x%x\n", data1.unk2[queue]);
         printf("\t\tqueue_size: %d\n\n", queue_sizes[queue]);
      }

      uint32_t cur_chunk = data1.reader[queue].chunk;
      uint32_t cur_block = cur_chunk > 3 ? data1.reader[queue].first_block
                                         : data1.reader_second_block[queue];
      uint32_t last_chunk = data1.writer[queue].chunk;
      uint32_t last_block = last_chunk > 3 ? data1.writer[queue].first_block
                                           : data1.writer_second_block[queue];

      if (verbose)
         printf("\tblock %x\n", cur_block);
      if (cur_block >= num_blocks) {
         fprintf(stderr, "block %x too large\n", cur_block);
         exit(1);
      }
      unsigned calculated_queue_size = 0;
      while (cur_block != last_block || cur_chunk != last_chunk) {
         calculated_queue_size++;
         uint32_t *chunk_ptr = &mempool[cur_block * 0x20 + cur_chunk * 4];

         dump_mem_pool_chunk(chunk_ptr);

         printf("\t%05x: %08x %08x %08x %08x\n",
                4 * (cur_block * 0x20 + cur_chunk + 4), chunk_ptr[0],
                chunk_ptr[1], chunk_ptr[2], chunk_ptr[3]);

         cur_chunk++;
         if (cur_chunk == 8) {
            cur_block = next_pointers[cur_block];
            if (verbose)
               printf("\tblock %x\n", cur_block);
            if (cur_block >= num_blocks) {
               fprintf(stderr, "block %x too large\n", cur_block);
               exit(1);
            }
            cur_chunk = 0;
         }
      }
      if (calculated_queue_size != queue_sizes[queue]) {
         printf("\t\tCALCULATED SIZE %d DOES NOT MATCH!\n",
                calculated_queue_size);
      }
      printf("\n");
   }
}

