/*
 * Copyright © 2022 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "io.h"
#include "redump.h"

static void
parse_addr(uint32_t *buf, int sz, unsigned int *len, uint64_t *gpuaddr)
{
   *gpuaddr = buf[0];
   *len = buf[1];
   if (sz > 8)
      *gpuaddr |= ((uint64_t)(buf[2])) << 32;
}

static uint32_t
parse_gpu_id(void *buf)
{
   return *((unsigned int *)buf);
}

static uint64_t
parse_chip_id(void *buf)
{
   return *((uint64_t *)buf);
}

struct rd_parsed_section
{
   void *buf;
   enum rd_sect_type type;
   int sz;
   int ret;
};

static bool
parse_rd_section(struct io *io, struct rd_parsed_section *section)
{
   uint32_t arr[2];
   int ret;

   ret = io_readn(io, arr, 8);
   if (ret <= 0)
      goto end;

   while ((arr[0] == 0xffffffff) && (arr[1] == 0xffffffff)) {
      ret = io_readn(io, arr, 8);
      if (ret <= 0)
         goto end;
   }

   section->type = arr[0];
   section->sz = arr[1];

   if (section->sz < 0) {
      ret = -1;
      goto end;
   }

   free(section->buf);

   section->buf = malloc(section->sz + 1);
   ((char *)section->buf)[section->sz] = '\0';
   ret = io_readn(io, section->buf, section->sz);
   if (ret < 0)
      goto end;

   section->ret = ret;
   return true;

end:
   section->ret = ret;
   return false;
}
