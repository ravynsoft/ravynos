/*
 * Copyright © 2021 Google, Inc.
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

static const char *hfi_msg_name(unsigned msgid);

/*
 * Decode HFI queues
 */

/* HFI message types */

#define HFI_MSG_CMD 0
#define HFI_MSG_ACK 1
#define HFI_MSG_ACK_V1 2

#define HFI_HEADER_ID(msg) ((msg) & 0xff)
/* Note that header size includes the header itself: */
#define HFI_HEADER_SIZE(msg) (((msg) >> 8) & 0xff)
#define HFI_HEADER_TYPE(msg)   (((msg) >> 16) & 0xf)
#define HFI_HEADER_SEQNUM(msg) (((msg) >> 20) & 0xfff)

struct a6xx_hfi_queue_header {
   uint32_t status;
   uint32_t iova;
   uint32_t type;
   uint32_t size;
   uint32_t msg_size;
   uint32_t dropped;
   uint32_t rx_watermark;
   uint32_t tx_watermark;
   uint32_t rx_request;
   uint32_t tx_request;
   uint32_t read_index;
   uint32_t write_index;
};

struct a6xx_hfi_queue_table_header {
   uint32_t version;
   uint32_t size;               /* Size of the queue table in dwords */
   uint32_t qhdr0_offset;       /* Offset of the first queue header */
   uint32_t qhdr_size;          /* Size of the queue headers */
   uint32_t num_queues;         /* Number of total queues */
   uint32_t active_queues;      /* Number of active queues */
   struct a6xx_hfi_queue_header queue[];
};

/*
 * HFI message definitions:
 */

#define HFI_F2H_MSG_ACK 126

struct a6xx_hfi_msg_response {
   uint32_t header;
   uint32_t ret_header;
   uint32_t error;
   uint32_t payload[16];
};

static void
decode_F2H_MSG_ACK(struct a6xx_hfi_msg_response *msg)
{
   unsigned msgid = HFI_HEADER_ID(msg->ret_header);

   printf("\t\t\t\tret_header: %s  (id=%u, size=%u, type=%u, seqnum=%u)\n",
          hfi_msg_name(msgid), msgid, HFI_HEADER_SIZE(msg->ret_header),
          HFI_HEADER_TYPE(msg->ret_header), HFI_HEADER_SEQNUM(msg->ret_header));
   printf("\t\t\t\terror:      %u\n",     msg->error);
}

#define HFI_F2H_MSG_ERROR 100

struct a6xx_hfi_msg_error {
   uint32_t header;
   uint32_t code;
   uint32_t payload[2];
};

static void
decode_F2H_MSG_ERROR(struct a6xx_hfi_msg_error *msg)
{
   printf("\t\t\t\tcode: %u\n", msg->code);
}

#define HFI_H2F_MSG_INIT 0

struct a6xx_hfi_msg_gmu_init_cmd {
   uint32_t header;
   uint32_t seg_id;
   uint32_t dbg_buffer_addr;
   uint32_t dbg_buffer_size;
   uint32_t boot_state;
};

static void
decode_H2F_MSG_INIT(struct a6xx_hfi_msg_gmu_init_cmd *msg)
{
   printf("\t\t\t\tseg_id:          %u\n",     msg->seg_id);
   printf("\t\t\t\tdbg_buffer_addr: 0x%08x\n", msg->dbg_buffer_addr);
   printf("\t\t\t\tdbg_buffer_size: %u\n",     msg->dbg_buffer_size);
   printf("\t\t\t\tboot_state:      %u\n",     msg->boot_state);
}

#define HFI_H2F_MSG_FW_VERSION 1

struct a6xx_hfi_msg_fw_version {
   uint32_t header;
   uint32_t supported_version;
};

static void
decode_H2F_MSG_FW_VERSION(struct a6xx_hfi_msg_fw_version *msg)
{
   printf("\t\t\t\tsupported_version: 0x%x\n", msg->supported_version);
}

#define HFI_H2F_MSG_PERF_TABLE 4

struct perf_level {
   uint32_t vote;
   uint32_t freq;
};

struct perf_gx_level {
   uint32_t vote;
   uint32_t acd;
   uint32_t freq;
};

struct a6xx_hfi_msg_perf_table_v1 {
   uint32_t header;
   uint32_t num_gpu_levels;
   uint32_t num_gmu_levels;

   struct perf_level gx_votes[16];
   struct perf_level cx_votes[4];
};

struct a6xx_hfi_msg_perf_table {
   uint32_t header;
   uint32_t num_gpu_levels;
   uint32_t num_gmu_levels;

   struct perf_gx_level gx_votes[16];
   struct perf_level cx_votes[4];
};

static void
decode_H2F_MSG_PERF_TABLE(void *_msg)
{
   if (is_gmu_legacy()) {
      struct a6xx_hfi_msg_perf_table_v1 *msg = _msg;
      unsigned i;

      printf("\t\t\t\tnum_gpu_levels: %u\n", msg->num_gpu_levels);
      printf("\t\t\t\tnum_gmu_levels: %u\n", msg->num_gmu_levels);

      assert(msg->num_gpu_levels <= ARRAY_SIZE(msg->gx_votes));
      for (i = 0; i < msg->num_gpu_levels; i++) {
         printf("\t\t\t\tgx_vote[%u]:    vote=%u, freq=%u\n", i,
                msg->gx_votes[i].vote, msg->gx_votes[i].freq);
      }

      for (; i < ARRAY_SIZE(msg->gx_votes); i++) {
         assert(!msg->gx_votes[i].vote);
         assert(!msg->gx_votes[i].freq);
      }

      assert(msg->num_gmu_levels <= ARRAY_SIZE(msg->cx_votes));
      for (i = 0; i < msg->num_gmu_levels; i++) {
         printf("\t\t\t\tcx_vote[%u]:    vote=%u, freq=%u\n", i,
                msg->cx_votes[i].vote, msg->cx_votes[i].freq);
      }

      for (; i < ARRAY_SIZE(msg->cx_votes); i++) {
         assert(!msg->cx_votes[i].vote);
         assert(!msg->cx_votes[i].freq);
      }
   } else {
      struct a6xx_hfi_msg_perf_table *msg = _msg;
      unsigned i;

      printf("\t\t\t\tnum_gpu_levels: %u\n", msg->num_gpu_levels);
      printf("\t\t\t\tnum_gmu_levels: %u\n", msg->num_gmu_levels);

      assert(msg->num_gpu_levels <= ARRAY_SIZE(msg->gx_votes));
      for (i = 0; i < msg->num_gpu_levels; i++) {
         printf("\t\t\t\tgx_vote[%u]:    vote=%u, acd=%u, freq=%u\n", i,
                msg->gx_votes[i].vote, msg->gx_votes[i].acd,
                msg->gx_votes[i].freq);
      }

      for (; i < ARRAY_SIZE(msg->gx_votes); i++) {
         assert(!msg->gx_votes[i].vote);
         assert(!msg->gx_votes[i].acd);
         assert(!msg->gx_votes[i].freq);
      }

      assert(msg->num_gmu_levels <= ARRAY_SIZE(msg->cx_votes));
      for (i = 0; i < msg->num_gmu_levels; i++) {
         printf("\t\t\t\tcx_vote[%u]:    vote=%u, freq=%u\n", i,
                msg->cx_votes[i].vote, msg->cx_votes[i].freq);
      }

      for (; i < ARRAY_SIZE(msg->cx_votes); i++) {
         assert(!msg->cx_votes[i].vote);
         assert(!msg->cx_votes[i].freq);
      }
   }
}

#define HFI_H2F_MSG_BW_TABLE 3

struct a6xx_hfi_msg_bw_table {
   uint32_t header;
   uint32_t bw_level_num;
   uint32_t cnoc_cmds_num;
   uint32_t ddr_cmds_num;
   uint32_t cnoc_wait_bitmask;
   uint32_t ddr_wait_bitmask;
   uint32_t cnoc_cmds_addrs[6];
   uint32_t cnoc_cmds_data[2][6];
   uint32_t ddr_cmds_addrs[8];
   uint32_t ddr_cmds_data[16][8];
};

static void
decode_H2F_MSG_BW_TABLE(struct a6xx_hfi_msg_bw_table *msg)
{
   printf("\t\t\t\tbw_level_num:       %u\n",   msg->bw_level_num);
   printf("\t\t\t\tcnoc_cmds_num:      %u\n",   msg->cnoc_cmds_num);
   printf("\t\t\t\tddr_cmds_num:       %u\n",   msg->ddr_cmds_num);
   printf("\t\t\t\tcnoc_wait_bitmask:  0x%x\n", msg->cnoc_wait_bitmask);
   printf("\t\t\t\tddr_wait_bitmask:   0x%x\n", msg->ddr_wait_bitmask);
   printf("\t\t\t\tcnoc_cmds_addrs:    %08x %08x %08x %08x %08x %08x\n",
          msg->cnoc_cmds_addrs[0], msg->cnoc_cmds_addrs[1], msg->cnoc_cmds_addrs[2],
          msg->cnoc_cmds_addrs[3], msg->cnoc_cmds_addrs[4], msg->cnoc_cmds_addrs[5]);
   for (unsigned i = 0; i < ARRAY_SIZE(msg->cnoc_cmds_data); i++) {
      printf("\t\t\t\tcnoc_cmds_data[%u]:  %08x %08x %08x %08x %08x %08x\n", i,
             msg->cnoc_cmds_data[i][0], msg->cnoc_cmds_data[i][1], msg->cnoc_cmds_data[i][2],
             msg->cnoc_cmds_data[i][3], msg->cnoc_cmds_data[i][4], msg->cnoc_cmds_data[i][5]);
   }
   printf("\t\t\t\tddr_cmds_addrs:     %08x %08x %08x %08x %08x %08x %08x %08x\n",
          msg->ddr_cmds_addrs[0], msg->ddr_cmds_addrs[1], msg->ddr_cmds_addrs[2],
          msg->ddr_cmds_addrs[3], msg->ddr_cmds_addrs[4], msg->ddr_cmds_addrs[5],
          msg->ddr_cmds_addrs[6], msg->ddr_cmds_addrs[7]);
   for (unsigned i = 0; i < ARRAY_SIZE(msg->ddr_cmds_data); i++) {
      printf("\t\t\t\tddr_cmds_data[%u]:   %08x %08x %08x %08x %08x %08x %08x %08x\n", i,
             msg->ddr_cmds_data[i][0], msg->ddr_cmds_data[i][1], msg->ddr_cmds_data[i][2],
             msg->ddr_cmds_data[i][3], msg->ddr_cmds_data[i][4], msg->ddr_cmds_data[i][5],
             msg->ddr_cmds_data[i][6], msg->ddr_cmds_data[i][7]);
   }
}

#define HFI_H2F_MSG_TEST 5

struct a6xx_hfi_msg_test {
   uint32_t header;
};

static void
decode_H2F_MSG_TEST(struct a6xx_hfi_msg_test *msg)
{
}

#define HFI_H2F_MSG_START 10

struct a6xx_hfi_msg_start {
   uint32_t header;
};

static void
decode_H2F_MSG_START(struct a6xx_hfi_msg_start *msg)
{
}

#define HFI_H2F_MSG_CORE_FW_START 14

struct a6xx_hfi_msg_core_fw_start {
   uint32_t header;
   uint32_t handle;
};

static void
decode_H2F_MSG_CORE_FW_START(struct a6xx_hfi_msg_core_fw_start *msg)
{
   printf("\t\t\t\thandle: %u\n", msg->handle);
}

#define HFI_H2F_MSG_GX_BW_PERF_VOTE 30

struct a6xx_hfi_gx_bw_perf_vote_cmd {
   uint32_t header;
   uint32_t ack_type;
   uint32_t freq;
   uint32_t bw;
};

static void
decode_H2F_MSG_GX_BW_PERF_VOTE(struct a6xx_hfi_gx_bw_perf_vote_cmd *msg)
{
   printf("\t\t\t\tack_type: %u\n", msg->ack_type);
   printf("\t\t\t\tfreq:     %u\n", msg->freq);
   printf("\t\t\t\tbw:       %u\n", msg->bw);
}

#define HFI_H2F_MSG_PREPARE_SLUMBER 33

struct a6xx_hfi_prep_slumber_cmd {
   uint32_t header;
   uint32_t bw;
   uint32_t freq;
};

static void
decode_H2F_MSG_PREPARE_SLUMBER(struct a6xx_hfi_prep_slumber_cmd *msg)
{
   printf("\t\t\t\tbw:   %u\n", msg->bw);
   printf("\t\t\t\tfreq: %u\n", msg->freq);
}

static struct {
   const char *name;
   void (*decode)(void *);
} hfi_msgs[] = {
#define HFI_MSG(name) [HFI_ ## name] = { #name, (void (*)(void *))decode_ ## name }
   HFI_MSG(F2H_MSG_ACK),
   HFI_MSG(F2H_MSG_ERROR),
   HFI_MSG(H2F_MSG_INIT),
   HFI_MSG(H2F_MSG_FW_VERSION),
   HFI_MSG(H2F_MSG_PERF_TABLE),
   HFI_MSG(H2F_MSG_BW_TABLE),
   HFI_MSG(H2F_MSG_TEST),
   HFI_MSG(H2F_MSG_START),
   HFI_MSG(H2F_MSG_CORE_FW_START),
   HFI_MSG(H2F_MSG_GX_BW_PERF_VOTE),
   HFI_MSG(H2F_MSG_PREPARE_SLUMBER),
};

static bool
is_valid_msg_type(unsigned type)
{
   switch (type) {
   case HFI_MSG_CMD:
   case HFI_MSG_ACK:
   case HFI_MSG_ACK_V1:
      return true;
   default:
      return false;
   }
}

static const char *
hfi_msg_name(unsigned msgid)
{
   if (msgid < ARRAY_SIZE(hfi_msgs))
      return hfi_msgs[msgid].name;
   return NULL;
}

static bool
is_valid_decode_start(struct a6xx_hfi_state *hfi, unsigned qidx, int32_t read_index)
{
   struct a6xx_hfi_queue_table_header *table = hfi->buf;
   struct a6xx_hfi_queue_header *queue = &table->queue[qidx];
   uint32_t offset = queue->iova - hfi->iova;
   uint32_t *dw = (uint32_t *)(((uint8_t *)hfi->buf) + offset);
   int last_seqno = -1;

   if (read_index < 0)
      return false;

   while (read_index != queue->write_index) {
      uint32_t hdr = dw[read_index];

      if (!is_valid_msg_type(HFI_HEADER_TYPE(hdr)))
         return false;

      if (!hfi_msg_name(HFI_HEADER_ID(hdr)))
         return false;

      /* Header size should be at least 1, and not extend past the write_index: */
      unsigned sz = HFI_HEADER_SIZE(hdr);
      if (!is_gmu_legacy())
         sz = ALIGN_POT(sz, 4);
      int remaining = ((read_index + sz) + (queue->size - 1) -
                       queue->write_index) % queue->size;
      if ((sz == 0) || (remaining < 0))
         return false;

      /* Seqno should be one more than previous seqno: */
      unsigned seqno = HFI_HEADER_SEQNUM(hdr);
      if ((last_seqno != -1) && (((last_seqno + 1) & 0xfff) != seqno))
         return false;

      last_seqno = seqno;

      read_index = (read_index + sz) % queue->size;
   }

   return true;
}

static void
decode_hfi(struct a6xx_hfi_state *hfi, unsigned qidx, int32_t read_index)
{
   struct a6xx_hfi_queue_table_header *table = hfi->buf;
   struct a6xx_hfi_queue_header *queue = &table->queue[qidx];
   uint32_t offset = queue->iova - hfi->iova;
   uint32_t *dw = (uint32_t *)(((uint8_t *)hfi->buf) + offset);

   while (read_index != queue->write_index) {
      uint32_t hdr = dw[read_index];
      unsigned msgid = HFI_HEADER_ID(hdr);
      unsigned sz    = HFI_HEADER_SIZE(hdr);
      unsigned type  = HFI_HEADER_TYPE(hdr);
      unsigned seqno = HFI_HEADER_SEQNUM(hdr);

      assert(is_valid_msg_type(type));
      assert(hfi_msg_name(msgid));

      printf("\t\t\t------ %s (id=%u, size=%u, type=%u, seqnum=%u)\n",
             hfi_msg_name(msgid), msgid, sz, type, seqno);

      if (!is_gmu_legacy())
         sz = ALIGN_POT(sz, 4);

      uint32_t buf[sz];
      for (unsigned i = 0; i < sz; i++) {
         buf[i] = dw[(read_index + i) % queue->size];
      }

      if (type == HFI_MSG_CMD)
         hfi_msgs[msgid].decode(buf);

      dump_hex_ascii(buf, sz*4, 4);

      read_index = (read_index + sz) % queue->size;
   }
}

/* Search backwards from the most recent (last) history entry to try to
 * find start of the oldest HFI message which has not been overwritten
 * due to ringbuffer wraparound.
 */
static int32_t
find_decode_start(struct a6xx_hfi_state *hfi, unsigned qidx)
{
   int i;

   for (i = ARRAY_SIZE(hfi->history[qidx]) - 1; i >= 0; i--) {
      if (!is_valid_decode_start(hfi, qidx, hfi->history[qidx][i]))
         break;
   }

   /* Last entry was invalid, or we decremented below zero, so advance
    * the index by one:
    */
   i++;

   if (i >= ARRAY_SIZE(hfi->history[qidx]))
      return -1;

   return hfi->history[qidx][i];
}

void
dump_gmu_hfi(struct a6xx_hfi_state *hfi)
{
   struct a6xx_hfi_queue_table_header *table = hfi->buf;

   printf("\tversion:       %u\n", table->version);
   printf("\tsize:          %u\n", table->size);
   printf("\tqhdr0_offset:  %u\n", table->qhdr0_offset);
   printf("\tqhdr_size:     %u\n", table->qhdr_size);
   printf("\tnum_queues:    %u\n", table->num_queues);
   printf("\tactive_queues: %u\n", table->active_queues);

   for (unsigned i = 0; i < table->num_queues; i++) {
      struct a6xx_hfi_queue_header *queue = &table->queue[i];

      printf("\tqueue[%u]:\n", i);
      printf("\t\tstatus:       0x%x\n", queue->status);
      printf("\t\tiova:         0x%x\n", queue->iova);
      printf("\t\ttype:         0x%x\n", queue->type);
      printf("\t\tsize:         %u\n",   queue->size);
      printf("\t\tmsg_size:     %u\n",   queue->msg_size);
      printf("\t\tdropped:      %u\n",   queue->dropped);
      printf("\t\trx_watermark: 0x%x\n", queue->rx_watermark);
      printf("\t\ttx_watermark: 0x%x\n", queue->tx_watermark);
      printf("\t\trx_request:   0x%x\n", queue->rx_request);
      printf("\t\ttx_request:   0x%x\n", queue->tx_request);
      printf("\t\tread_index:   %u\n",   queue->read_index);
      printf("\t\twrite_index:  %u\n",   queue->write_index);

      int32_t read_index = find_decode_start(hfi, i);
      if (read_index >= 0)
         decode_hfi(hfi, i, read_index);
   }
}
