/*
 * Copyright © 2022 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include "tu_cs.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "tu_device.h"

/* A simple implementations of breadcrumbs tracking of GPU progress
 * intended to be a last resort when debugging unrecoverable hangs.
 *
 * This implementation aims to handle cases where we cannot do anything
 * after the hang, which is achieved by:
 * - On GPU after each breadcrumb we wait until CPU acks it and sends udp
 *    packet to the remote host;
 * - At specified breadcrumb require explicit user input to continue
 *   execution up to the next breadcrumb.
 *
 * For usage see freedreno.rst
 */

struct breadcrumbs_context
{
   char remote_host[64];
   int remote_port;
   uint32_t breadcrumb_breakpoint;
   uint32_t breadcrumb_breakpoint_hits;

   bool thread_stop;
   pthread_t breadcrumbs_thread;

   struct tu_device *device;

   uint32_t breadcrumb_idx;
};

static void *
sync_gpu_with_cpu(void *_job)
{
   struct breadcrumbs_context *ctx = (struct breadcrumbs_context *) _job;
   struct tu6_global *global = ctx->device->global_bo_map;
   uint32_t last_breadcrumb = 0;
   uint32_t breakpoint_hits = 0;

   int s = socket(AF_INET, SOCK_DGRAM, 0);

   if (s < 0) {
      mesa_loge("TU_BREADCRUMBS: Error while creating socket");
      return NULL;
   }

   struct sockaddr_in to_addr;
   to_addr.sin_family = AF_INET;
   to_addr.sin_port = htons(ctx->remote_port);
   to_addr.sin_addr.s_addr = inet_addr(ctx->remote_host);

   /* Run until we know that no more work would be submitted,
    * because each breadcrumb requires an ack from cpu side and without
    * the ack GPU would timeout.
    */
   while (!ctx->thread_stop) {
      uint32_t current_breadcrumb = global->breadcrumb_gpu_sync_seqno;

      if (current_breadcrumb != last_breadcrumb) {
         last_breadcrumb = current_breadcrumb;

         uint32_t data = htonl(last_breadcrumb);
         if (sendto(s, &data, sizeof(data), 0, (struct sockaddr *) &to_addr,
                    sizeof(to_addr)) < 0) {
            mesa_loge("TU_BREADCRUMBS: sendto failed");
            goto fail;
         }

         if (last_breadcrumb >= ctx->breadcrumb_breakpoint &&
             breakpoint_hits >= ctx->breadcrumb_breakpoint_hits) {
            printf("GPU is on breadcrumb %d, continue?", last_breadcrumb);
            while (getchar() != 'y')
               ;
         }

         if (ctx->breadcrumb_breakpoint == last_breadcrumb)
            breakpoint_hits++;

         /* ack that we received the value */
         global->breadcrumb_cpu_sync_seqno = last_breadcrumb;
      }
   }

fail:
   close(s);

   return NULL;
}

/* Same as tu_cs_emit_pkt7 but without instrumentation */
static inline void
emit_pkt7(struct tu_cs *cs, uint8_t opcode, uint16_t cnt)
{
   tu_cs_reserve(cs, cnt + 1);
   tu_cs_emit(cs, pm4_pkt7_hdr(opcode, cnt));
}

void
tu_breadcrumbs_init(struct tu_device *device)
{
   const char *breadcrumbs_opt = NULL;
#ifdef TU_BREADCRUMBS_ENABLED
   breadcrumbs_opt = os_get_option("TU_BREADCRUMBS");
#endif

   device->breadcrumbs_ctx = NULL;
   if (!breadcrumbs_opt) {
      return;
   }

   struct breadcrumbs_context *ctx = (struct breadcrumbs_context *) malloc(
      sizeof(struct breadcrumbs_context));
   ctx->device = device;
   ctx->breadcrumb_idx = 0;
   ctx->thread_stop = false;

   if (sscanf(breadcrumbs_opt, "%[^:]:%d,break=%u:%u", ctx->remote_host,
              &ctx->remote_port, &ctx->breadcrumb_breakpoint,
              &ctx->breadcrumb_breakpoint_hits) != 4) {
      free(ctx);
      mesa_loge("Wrong TU_BREADCRUMBS value");
      return;
   }

   device->breadcrumbs_ctx = ctx;

   struct tu6_global *global = device->global_bo_map;
   global->breadcrumb_cpu_sync_seqno = 0;
   global->breadcrumb_gpu_sync_seqno = 0;

   pthread_create(&ctx->breadcrumbs_thread, NULL, sync_gpu_with_cpu, ctx);
}

void
tu_breadcrumbs_finish(struct tu_device *device)
{
   struct breadcrumbs_context *ctx = device->breadcrumbs_ctx;
   if (!ctx || ctx->thread_stop)
      return;

   ctx->thread_stop = true;
   pthread_join(ctx->breadcrumbs_thread, NULL);

   free(ctx);
}

void
tu_cs_emit_sync_breadcrumb(struct tu_cs *cs, uint8_t opcode, uint16_t cnt)
{
   /* TODO: we may run out of space if we add breadcrumbs
    * to non-growable CS.
    */
   if (cs->mode != TU_CS_MODE_GROW)
      return;

   struct tu_device *device = cs->device;
   struct breadcrumbs_context *ctx = device->breadcrumbs_ctx;
   if (!ctx || ctx->thread_stop)
      return;

   bool before_packet = (cnt != 0);

   if (before_packet) {
      switch (opcode) {
      case CP_EXEC_CS_INDIRECT:
      case CP_EXEC_CS:
      case CP_DRAW_INDX:
      case CP_DRAW_INDX_OFFSET:
      case CP_DRAW_INDIRECT:
      case CP_DRAW_INDX_INDIRECT:
      case CP_DRAW_INDIRECT_MULTI:
      case CP_DRAW_AUTO:
      case CP_BLIT:
         // case CP_SET_DRAW_STATE:
         // case CP_LOAD_STATE6_FRAG:
         // case CP_LOAD_STATE6_GEOM:
         break;
      default:
         return;
      };
   } else {
      assert(cs->breadcrumb_emit_after == 0);
   }

   uint32_t current_breadcrumb = p_atomic_inc_return(&ctx->breadcrumb_idx);

   if (ctx->breadcrumb_breakpoint != -1 &&
       current_breadcrumb < ctx->breadcrumb_breakpoint)
      return;

   emit_pkt7(cs, CP_WAIT_MEM_WRITES, 0);
   emit_pkt7(cs, CP_WAIT_FOR_IDLE, 0);
   emit_pkt7(cs, CP_WAIT_FOR_ME, 0);

   emit_pkt7(cs, CP_MEM_WRITE, 3);
   tu_cs_emit_qw(
      cs, device->global_bo->iova + gb_offset(breadcrumb_gpu_sync_seqno));
   tu_cs_emit(cs, current_breadcrumb);

   /* Wait until CPU acknowledges the value written by GPU */
   emit_pkt7(cs, CP_WAIT_REG_MEM, 6);
   tu_cs_emit(cs, CP_WAIT_REG_MEM_0_FUNCTION(WRITE_EQ) |
                     CP_WAIT_REG_MEM_0_POLL(POLL_MEMORY));
   tu_cs_emit_qw(
      cs, device->global_bo->iova + gb_offset(breadcrumb_cpu_sync_seqno));
   tu_cs_emit(cs, CP_WAIT_REG_MEM_3_REF(current_breadcrumb));
   tu_cs_emit(cs, CP_WAIT_REG_MEM_4_MASK(~0));
   tu_cs_emit(cs, CP_WAIT_REG_MEM_5_DELAY_LOOP_CYCLES(16));

   if (before_packet)
      cs->breadcrumb_emit_after = cnt;
}
