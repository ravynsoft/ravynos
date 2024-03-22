/*
 * Copyright © 2022 Google, Inc.
 * Copyright © 2022 Valve Corporation
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

#include "util/macros.h"
#include "crashdec.h"
#include "cffdec.h"

#define MAX_PREFETCH_IBS 4

/* CP_INDIRECT_BUFFER contains an optimization to read ahead and start
 * fetching up to 3 subsequent CP_INDIRECT_BUFFER contents into the ROQ before
 * starting to execute the current IB. This effectively combines them into one
 * CP_INDIRECT_BUFFER. The result is that if the ROQ is fast enough and
 * prefetches some of the extra IBs before the first IB finishes, the ROQ may
 * be in a different IB than the CP is processing. That is, normally we'd have
 * a situation like this:
 *
 *    CP_INDIRECT_BUFFER
 *       ...
 *       CP_FOO <- PFP/SQE is reading from here
 *       ...
 *       CP_BAR <- ROQ has prefetched up to here
 *
 * where CP_IB*_BASE and CP_IB*_REM_SIZE point to CP_BAR and the difference
 * between CP_FOO and CP_BAR is given by CP_ROQ_AVAIL_IBn::REM, but instead we
 * may get a situation like this:
 *
 *   CP_INDIRECT_BUFFER
 *      ...
 *      CP_FOO <- PFP/SQE is reading here
 *      ...
 *   CP_INDIRECT_BUFFER
 *      ...
 *      CP_BAR <- ROQ has prefetched up to here
 *
 * in this case, the "rem" we get with CP_ROQ_AVAIL_IBn::REM added will be
 * larger than the size of the second IB, indicating that we need to back up
 * to the IB before it. This can theoretically even happen recursively with
 * IB2:
 *
 * CP_INDIRECT_BUFFER:
 *    ...
 *    CP_INDIRECT_BUFFER:
 *       ...
 *       CP_FOO <- PFP/SQE IB2 is reading here
 *       ...
 * CP_INDIRECT_BUFFER:
 *    CP_INDIRECT_BUFFER:
 *       ...
 *       CP_BAR <- ROQ IB2 has prefetched up to here
 *       ...
 * CP_BAZ <- PFP/SQE IB1 is reading here
 *
 * Here the ROQ has prefetched the second IB1, then when processing the IB2 at
 * the end of the first IB1 it peeks ahead in ROQ and sees another IB2 right
 * afterward in the second IB1 and starts prefetching that too, so that the
 * ROQ is in a different IB1 *and* IB2 from the CP.
 *
 * To account for this when locating the position that the SQE was at in the
 * cmdstream at the time of the crash, we do a pre-pass scanning the
 * CP_INDIRECT_BUFFER packets, keeping a history of previous IB's so that we
 * can backtrack (because CP_IBn_BASE can be several IB's ahead of SQE).  Once
 * we find the IB1 position that is being read into ROQ, we backtrack until
 * we find the IB1 position that SQE is at, and (roughly) repeat the process
 * in IB2.  This has one calculation in that we need to start scanning for the
 * CP_INDIRECT_BUFFER to IB2 from before the detected IB1 position.
 */

struct ib {
   uint64_t ibaddr;
   uint32_t ibsize;
};

struct prefetch_state {
   struct ib history[MAX_PREFETCH_IBS];
   unsigned num, next;
};

static void
push_ib(struct prefetch_state *s, struct ib *ib)
{
   s->history[s->next++ % ARRAY_SIZE(s->history)] = *ib;
   s->num = MIN2(s->num + 1, ARRAY_SIZE(s->history));
}

static struct ib *
get_ib(struct prefetch_state *s, int n)
{
   if ((n >= s->num) || (n < 0))
      return NULL;
   int idx = s->next - (s->num - n);
   return &s->history[idx % ARRAY_SIZE(s->history)];
}

static void
reset_state(struct prefetch_state *s)
{
   s->num = s->next = 0;
}

/**
 * Once we find the ROQ prefetch position, work backwards to find the SQE
 * position.
 */
static struct ib *
reverse_prefetch(struct prefetch_state *s, int lvl)
{
   unsigned rem = options.ibs[lvl].rem;

   for (int n = s->num - 1; n >= 0; n--) {
      struct ib *ib = get_ib(s, n);
      if (ib->ibsize > rem) {
         options.ibs[lvl].crash_found = 1;
         options.ibs[lvl].base = ib->ibaddr;
         options.ibs[lvl].rem = rem;

         return ib;
      }
      rem -= ib->ibsize;
   }

   return NULL;
}

/**
 * Scan cmdstream looking for CP_INDIRECT_BUFFER packets, tracking history
 * of consecutive CP_INDIRECT_BUFFER packets, until we find the one that
 * matches CP_IBn_BASE.
 */
static struct ib *
scan_cmdstream(struct prefetch_state *s, int lvl, uint32_t *dwords, uint32_t sizedwords)
{
   int dwords_left = sizedwords;
   uint32_t count = 0; /* dword count including packet header */
   uint32_t val;

   while (dwords_left > 0) {
      if (pkt_is_opcode(dwords[0], &val, &count)) {
         if (!strcmp(pktname(val), "CP_INDIRECT_BUFFER")) {
            uint64_t ibaddr;
            uint32_t ibsize;

            parse_cp_indirect(&dwords[1], count - 1, &ibaddr, &ibsize);
            push_ib(s, &(struct ib){ ibaddr, ibsize });

            /* If we've found the IB indicated by CP_IBn_BASE, then we can
             * search backwards from here to find the SQE position:
             */
            if (ibaddr == options.ibs[lvl].base)
               return reverse_prefetch(s, lvl);

            goto next_pkt;
         }
      } else if (pkt_is_regwrite(dwords[0], &val, &count)) {
      } else {
         count = find_next_packet(dwords, dwords_left);
      }

      /* prefetch only happens across consecutive CP_INDIRECT_BUFFER, so
       * any other packet resets the state:
       */
      reset_state(s);

next_pkt:
      dwords += count;
      dwords_left -= count;
   }

   return NULL;
}

void
handle_prefetch(uint32_t *dwords, uint32_t sizedwords)
{
   struct prefetch_state rb_state = {};
   struct ib *ib1 = scan_cmdstream(&rb_state, 1, dwords, sizedwords);
   if (!ib1)
      return;

   struct prefetch_state ib1_state = {};

   /* Once we find the actual IB1 position, we need to find the IB2 position.
    * But because IB2 prefetch can span IB1 CP_INDIRECT_BUFFER targets.  But
    * there are a limited # of buffers that can be prefetched, and we already
    * have a history of enough  RB->IB1 IB's, so we can simply scan forward
    * from our oldest history entry until we find the IB2 match..
    */
   for (int n = 0; n < rb_state.num; n++) {
      struct ib *ib = get_ib(&rb_state, n);
      uint32_t *ibaddr = hostptr(ib->ibaddr);
      if (!ibaddr)
         break;
      struct ib *ib2 = scan_cmdstream(&ib1_state, 2, ibaddr, ib->ibsize);

      /* If the crash happens in IB2, but IB1 has a sequence of CP_INDIRECT_BUFFER's
       * then IB1 could actually be further ahead than IB2, ie:
       *
       *    IB1:CP_INDIRECT_BUFFER
       *        IB2: .. crash somewhere in here ..
       *    IB1:CP_INDIRECT_BUFFER
       *    IB1:CP_INDIRECT_BUFFER  <-- detected IB1 position
       *
       * Our logic for detecting the correct IB1 position is not incorrect.
       * It is just that SQE has already consumed some additional IB's.  So
       * reset the IB1 crash position back to the oldest RB->IB1 IB that we
       * remember.
       *
       * This isn't *quite* correct, but cffdec will only mark the crash when
       * it finds the location in IB2 if we've determined that the crash is
       * in IB2, but will only consider the address in IB2 if it has seen the
       * IB1 base.
       *
       * The main case we are trying to account for here is GMEM mode crash in
       * IB2 which *isn't* the first bin/tile.  Ie. the crash happens later
       * than the first time we encounter the IB2 crash address.
       *
       * This approach works in practice because there will be some other pkts
       * in IB1 to setup for the next tile, breaking up prefetch.
       */
      if (ib2) {
         assert(options.ibs[2].crash_found);
         struct ib *first_rb_ib = get_ib(&rb_state, 0);

         options.ibs[1].base = first_rb_ib->ibaddr;
         options.ibs[1].rem = first_rb_ib->ibsize;

         break;
      }

      if (ib == ib1)
         break;
   }
}
